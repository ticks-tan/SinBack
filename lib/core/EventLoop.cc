/**
* FileName:   EventLoop.cc
* CreateDate: 2022-03-09 09:56:06
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "EventLoop.h"

using namespace SinBack;

/* 全局事件ID */
static std::atomic<ULong> g_event_loop_id_count;

/**
 * 获取新的事件id
 * @return
 */
ULong Core::EventLoop::event_loop_next_id() {
    ++g_event_loop_id_count;
    return g_event_loop_id_count;
}

Core::EventLoop::EventLoop()
    : pid_(0), id_(0)
    , status_(Stop)
    , start_time_(0), end_time_(0), cur_time_(0)
    , loop_count_(0), actives_count_(0)
    , pending_count_(0), io_count_(0), custom_count_(0), idle_count_(0), timer_count_(0)
    , thread_pool_(default_thread_pool_count)
{
    init_loop();
}

Core::EventLoop::~EventLoop()
{
    for (auto& it : io_evs_){
        selector_->del_event(it.first, Core::IO_RDWR);
        Base::close_socket(it.first);
    }
    io_evs_.clear();
}

/**
 * 初始化事件循环
 */
void Core::EventLoop::init_loop()
{
    this->id_ = event_loop_next_id();
    this->selector_ = std::make_shared<Core::Selector>(this);
    std::basic_string<Char> logger_name = SIN_STR("SinBack_EventLoop_");
    logger_name += std::to_string(this->id_) + SIN_STR("_");
    logger_name += std::to_string(this->pid_);
    this->logger_ = std::make_shared<Log::Logger>(Log::LoggerType::Rolling, logger_name);
    this->start_time_ = this->end_time_ = this->cur_time_ = 0;
}

/**
 * 开始运行循环
 * @return
 */
bool Core::EventLoop::run()
{
    if (this->status_ == Running){
        return false;
    }
    this->status_ = Running;
    this->pid_ = getpid();
    this->start_time_ = Base::gettimeofday_ms();

    while (this->status_ != Stop){
        if (this->status_ == Pause){
            // 休眠一段时间
            Base::sleep_ms(default_event_loop_pause_timeout);
            update_time();
            continue;
        }
        ++this->loop_count_;
        if (this->loop_count_ == 100000000){
            this->logger_->debug("loop count = {}, reset the count.", this->loop_count_);
            this->loop_count_.store(0);
        }
        // 处理事件
        this->process_events();
        update_time();
    }
    this->status_ = Stop;
    this->end_time_ = Base::gettimeofday_ms();
    return true;
}

/**
 * 停止循环
 * @return
 */
bool Core::EventLoop::stop()
{
    if (this->status_ == Running){
        this->status_ = Stop;
        return true;
    }
    return false;
}

/**
 * 暂停循环
 * @return
 */
bool Core::EventLoop::pause()
{
    if (this->status_ == Running){
        this->status_ = Pause;
        return true;
    }
    return false;
}

/**
 * 恢复循环
 * @return
 */
bool Core::EventLoop::resume()
{
    if (this->status_ == Pause){
        this->status_ == Running;
        return true;
    }
    return false;
}

/**
 * 根据套接字获取 IO 事件，没有则创建
 * @param fd
 * @return
 */
std::shared_ptr<Core::IOEvent> Core::EventLoop::get_io_event(Base::socket_t fd)
{
    std::shared_ptr<IOEvent> ptr;
    auto it = this->io_evs_.find(fd);
    if (it != this->io_evs_.end()){
        if (it->second->destroy_){
            ptr = std::make_shared<IOEvent>();
            ptr->init();
            ptr->loop_ = this;
            ptr->fd_ = fd;
            ptr->type_ = Event_Type_IO;
            it->second = ptr;
        } else{
            ptr = it->second;
        }
    }else{
        ptr.reset(new IOEvent);
        ptr->init();
        ptr->loop_ = this;
        ptr->fd_ = fd;
        ptr->type_ = Event_Type_IO;
        this->io_evs_[fd] = ptr;
    }
    return ptr;
}

/**
 * 处理事件循环
 * @return
 */
Int Core::EventLoop::process_events()
{
    Int ios_cnt = 0, timer_cnt = 0, idle_cnt = 0, pending_cnt = 0;
    Int block_time = default_event_loop_wait_timeout;
    // 处理定时器任务
    if (!this->timer_.empty()){
        update_time();
        ULong min_next_timeout = (*(this->timer_.begin())).second->next_time_;
        Long block_us = (Long)(min_next_timeout - this->cur_time_);
        if (block_us <= 0){
            // 有超时事件
            goto PROCESS_TIMER;
        }
        ++block_us;
        block_time = (block_us < default_event_loop_wait_timeout * 1000) ? (Int)block_us : default_event_loop_wait_timeout * 1000;
    }
    if (this->io_count_ > 0){
        // 处理IO事件
        ios_cnt = this->process_io(block_time);
    }else{
        Base::sleep_us(block_time);
    }
    this->update_time();
    if (this->status_ == Stop){
        return 0;
    }
    PROCESS_TIMER:
    if (this->timer_count_ > 0){
        // 处理定时事件
        timer_cnt = process_timer();
    }
    pending_cnt = this->pending_count_;
    if (pending_cnt == 0){
        if (this->idle_count_ > 0){
            pending_cnt = this->process_idle();
        }
    }
    pending_cnt = this->process_pending();
    return pending_cnt;
}

/**
 * 处理空闲事件
 * @return
 */
Int Core::EventLoop::process_idle()
{
    Int idle_cnt = 0;
    std::shared_ptr<Core::IdleEvent> item;
    auto it = this->idle_evs_.begin();
    for (; it != this->idle_evs_.end();){
        item = *it;
        if (item->repeat_ != Core::IdleEvent::infinity){
            --item->repeat_;
        }
        EventLoop::loop_event_pending(item);
        ++idle_cnt;

        if (item->repeat_ == 0){
            item->destroy_ = true;
            item->loop_->idle_count_--;
            it = this->idle_evs_.erase(it);
            continue;
        }
        ++it;
    }
    return idle_cnt;
}

/**
 * 待处理事件
 * @return
 */
Int Core::EventLoop::process_pending()
{
    if (this->pending_count_ == 0){
        return 0;
    }
    std::shared_ptr<Event> item;
    // 遍历优先级数组 (高 -> 低)
    Int pending_cnt = 0, i = Core::EventPriority::Event_Priority_Highest;
    for (; i >= 0; --i){
        auto& list = this->pending_evs_[i];
        for (auto& it : list){
            item = it;
            // 根据事件类型转换后调用对应回调函数
            if (item){
                if (item->pending_){
                    // 执行回调函数
                    if (item->active_){
                        if (item->type_ == Core::Event_Type_Custom) {
                            if (item->cb_) {
                                item->cb_(item);
                            }
                        }else if (item->type_ == Core::Event_Type_IO){
                            auto io = std::dynamic_pointer_cast<Core::IOEvent>(item);
                            if (io->cb_) {
                                io->cb_(io);
                            }
                        }else if (item->type_ == Core::Event_Type_Idle){
                            auto idle = std::dynamic_pointer_cast<Core::IdleEvent>(item);
                            if (idle->cb_) {
                                idle->cb_(idle);
                            }
                        }else if (item->type_ == Core::Event_Type_Timer){
                            // this->logger_->debug("Timer Event {} .", item->id_);
                            auto timer = std::dynamic_pointer_cast<Core::TimerEvent>(item);
                            if (timer->cb_) {
                                timer->cb_(timer);
                            }
                        }
                        ++pending_cnt;
                    }
                    item->pending_ = false;
                    if (item->destroy_){
                        loop_event_inactive(item);
                    }
                }
            }else{
                fmt::print("event error\n");
            }
        }
        // 清理待执行链表
        list.clear();
    }
    this->pending_count_ = 0;
    return pending_cnt;
}

/**
 * 处理定时器事件
 * @return
 */
Int Core::EventLoop::process_timer()
{
    Int timer_cnt = 0;
    ULong now_time = this->cur_time_;
    std::shared_ptr<Core::TimerEvent> item;
    if (!this->timer_.empty()){
        auto it = this->timer_.begin();
        for (; it != this->timer_.end();){
            item = it->second;
            if (item->next_time_ > now_time){
                break;
            }
            if (item->repeat_ != Core::TimerEvent::infinity){
                --item->repeat_;
            }
            EventLoop::loop_event_pending(item);
            ++timer_cnt;
            this->update_time();
            if (item->repeat_ == 0){
                // 次数用尽，删除定时器
                item->loop_->timer_count_--;
                item->destroy_ = true;
                it = this->timer_.erase(it);
            } else{
                item->id_ = Core::event_next_id();
                item->next_time_ = this->cur_time_ + item->timeout_ * 1000;
                this->timer_[item->next_time_] = item;
                it = this->timer_.erase(it);
            }
        }
    }
    return timer_cnt;
}

/**
 * 处理IO事件
 * @param timeout
 * @return
 */
Int Core::EventLoop::process_io(Int timeout)
{
    Int io_cnt = this->selector_->poll_event(timeout);
    return (io_cnt < 0) ? 0 : io_cnt;
}

/**
 * 更新事件循环时间
 */
void Core::EventLoop::update_time()
{
    this->cur_time_ = Base::gettimeofday_us();
}

/**
 * 添加IO事件
 * @param ev：事件指针
 * @param cb：对应事件回调函数
 * @param events：待添加事件
 * @return
 */
Int Core::EventLoop::add_io_event(const std::weak_ptr<Core::IOEvent> &ev, const Core::IOEventCB &cb, Int events)
{
    std::shared_ptr<Core::IOEvent> io = ev.lock();
    if (io){
        // 事件活跃
        if (!io->active_){
            EventLoop::loop_event_active(io);
            ++io->loop_->io_count_;
        }
        if (cb){
            io->cb_ = cb;
        }
        // 将事件添加到Selector
        if (!(io->evs_ & events)){
            io->loop_->selector_->add_event(io->fd_, events);
            io->evs_ |= events;
        }
    }
    return 0;
}

/**
 * 删除 IO 事件
 * @param ev：事件指针
 * @param events ：待删除事件
 * @return
 */
Int Core::EventLoop::delete_io_event(const std::weak_ptr<Core::IOEvent> &ev, Int events)
{
    std::shared_ptr<Core::IOEvent> io = ev.lock();
    if (io){
#ifdef OS_WINDOWS
        if (io->fd_ < 3) return -1;
#else
        if (!io->active_) return -1;
#endif
        if (io->evs_ & events){
            io->loop_->selector_->del_event(io->fd_, events);
            io->evs_ &= ~events;
        }
        // 事件清空，事件不活跃
        if (io->evs_ == 0){
            io->loop_->io_count_--;
            EventLoop::loop_event_inactive(io);
        }
    }
    return 0;
}

/**
 * IO事件 -- 接受连接
 * @param listen_fd：监听套接字
 * @param cb：监听回调
 * @return
 */
std::shared_ptr<Core::IOEvent>
Core::EventLoop::accept_io(Base::socket_t listen_fd, const Core::IOAcceptCB &cb)
{
    std::shared_ptr<Core::IOEvent> io = this->get_io_event(listen_fd);
    if (cb){
        io->accept_cb_ = cb;
    }
    io->accept();
    return io;
}

/**
 * IO事件 -- 读取事件
 * @param fd ：要读取的套接字
 * @param read_len ：读取数据长度
 * @param cb ：读取回调
 * @return
 */
std::shared_ptr<Core::IOEvent>
Core::EventLoop::read_io(Base::socket_t fd, Size_t read_len, const Core::IOReadCB &cb)
{
    std::shared_ptr<Core::IOEvent> io = this->get_io_event(fd);
    if (read_len > 0){
        io->read_buf_.reserve(io->read_buf_.capacity() + read_len);
    }
    if (cb){
        io->read_cb_ = cb;
    }
    io->read();
    return io;
}

/**
 * IO事件 -- 写入
 * @param fd ：要写入的套接字
 * @param buf ：缓冲区
 * @param len ：数据长度
 * @param cb：写入回调
 * @return
 */
std::shared_ptr<Core::IOEvent>
Core::EventLoop::write_io(Base::socket_t fd, const void *buf, Size_t len, const Core::IOWriteCB &cb)
{
    std::shared_ptr<Core::IOEvent> io = this->get_io_event(fd);
    if (cb){
        io->write_cb_ = cb;
    }
    if (buf && len > 0){
        io->write(buf, len);
    }
    return io;
}

/**
 * IO事件 -- 关闭套接字
 * @param fd：要关闭的套接字
 */
void
Core::EventLoop::close_io(Base::socket_t fd)
{
    std::shared_ptr<Core::IOEvent> io = this->get_io_event(fd);
    io->close();
}

/**
 * 添加定时器
 * @param cb：定时函数
 * @param timeout：超时时间 (ms)
 * @param repeat：重复次数
 * @return
 */
std::shared_ptr<Core::TimerEvent>
Core::EventLoop::add_timer(const Core::TimerEventCB &cb, UInt timeout, Int repeat)
{
    std::shared_ptr<Core::TimerEvent> timer(new TimerEvent);
    timer->init();
    timer->type_ = Core::Event_Type_Timer;
    timer->priority_ = Core::Event_Priority_Highest;
    timer->timeout_ = timeout;
    timer->repeat_ = repeat;
    this->update_time();
    timer->next_time_ = this->cur_time_ + timeout * 1000;
    if (timeout > 1000 && timeout % 100 == 0){
        timer->next_time_ = timer->next_time_ / 100000 * 100000;
    }
    this->timer_[timer->next_time_] = timer;
    // add the event
    timer->loop_ = this;
    timer->cb_ = cb;
    EventLoop::loop_event_active(timer);
    ++this->timer_count_;
    return timer;
}

/**
 * 删除定时器
 * @param ev：待删除定时器
 */
void
Core::EventLoop::delete_timer(const std::weak_ptr<Core::TimerEvent>& ev)
{
    std::shared_ptr<TimerEvent> timer = ev.lock();
    if (timer) {
        if (!timer->active_) return;
        auto it = this->timer_.find(timer->next_time_);
        if (it != this->timer_.end()) {
            this->timer_count_--;
            timer->destroy_ = true;
            EventLoop::loop_event_inactive(it->second);
            it = this->timer_.erase(it);
        }
    }
}

/**
 * 添加待处理事件
 * @param cb ：待执行函数
 * @param repeat ：重复次数
 * @return
 */
std::shared_ptr<Core::IdleEvent>
Core::EventLoop::add_idle(const Core::IdleEventCB &cb, Int repeat)
{
    std::shared_ptr<Core::IdleEvent> idle(new Core::IdleEvent);
    // 初始化事件
    idle->init();
    // 制定事件类型
    idle->type_ = Core::Event_Type_Idle;
    idle->repeat_ = repeat;
    // 事件优先级最低
    idle->priority_ = Core::Event_Priority_Lowest;

    this->idle_evs_.push_back(idle);
    idle->loop_ = this;
    idle->cb_ = cb;
    EventLoop::loop_event_active(idle);
    ++this->idle_count_;

    return idle;
}

/**
 * 删除空闲事件
 * @param ev：待删除空闲事件
 */
void
Core::EventLoop::delete_idle(const std::weak_ptr<Core::IdleEvent>& ev)
{
    std::shared_ptr<Core::IdleEvent> idle = ev.lock();
    if (idle) {
        if (!idle->active_) return;
        idle->destroy_ = true;
        auto it = std::find(this->idle_evs_.begin(), this->idle_evs_.end(), idle);
        if (it != this->idle_evs_.end()){
            this->idle_count_--;
            EventLoop::loop_event_inactive(idle);
            it = this->idle_evs_.erase(it);
        }
    }
}

/**
 * 添加自定义事件
 * @param cb：自定义事件回调
 * @return
 */
std::shared_ptr<Core::Event>
Core::EventLoop::add_custom(const Core::EventCB &cb) {
    std::shared_ptr<Core::Event> custom(new Event);
    custom->init();
    custom->type_ = Event_Type_Custom;
    custom->priority_ = Core::Event_Priority_Lowest;
    ++this->custom_count_;
    custom->loop_ = this;
    custom->cb_ = cb;
    this->thread_pool_.enqueue([&custom](){
        if (custom->cb_){
            custom->cb_(custom);
        }
        --custom->loop_->custom_count_;
    });
    return custom;
}

