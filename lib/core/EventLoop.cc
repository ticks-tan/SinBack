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
    initLoop();
}

Core::EventLoop::~EventLoop()
{
    this->stop();
    this->thread_pool_.stop();
    this->io_evs_.clear();
    this->timer_.clear();
    Int i = Core::Event_Priority_Highest;
    for (; i >= Core::Event_Priority_Lowest; --i){
        this->pending_evs_[i].clear();
    }
    this->idle_evs_.clear();
    while (!this->custom_evs_.empty()) {
        this->custom_evs_.pop();
    }
    this->io_count_ = this->custom_count_ = this->idle_count_ = this->timer_count_ = 0;
}

/**
 * 初始化事件循环
 */
void Core::EventLoop::initLoop()
{
    this->id_ = event_loop_next_id();
    this->selector_ = std::make_shared<Core::Selector>(this);
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
    this->start_time_ = Base::getTimeOfDayMs();

    while (this->status_ != Stop){
        if (this->status_ == Pause){
            // 休眠一段时间
            Base::sleepMs(default_event_loop_pause_timeout);
            updateTime();
            continue;
        }
        ++this->loop_count_;
        if (this->loop_count_ == 100000000){
            Log::logd("loop count = {}, reset the count.", this->loop_count_);
            this->loop_count_.store(0);
        }
        // 处理事件
        this->processEvents();
        updateTime();
    }
    this->status_ = Stop;
    this->end_time_ = Base::getTimeOfDayMs();
    return true;
}

/**
 * 停止循环
 * @return
 */
bool Core::EventLoop::stop()
{
    if (this->status_ != Stop){
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
std::shared_ptr<Core::IOEvent> Core::EventLoop::getIoEvent(Base::socket_t fd)
{
    std::shared_ptr<IOEvent> ptr;
    auto it = this->io_evs_.find(fd);
    if (it != this->io_evs_.end()){
        if (it->second->destroy_ || !it->second->active_){
            ptr = std::make_shared<IOEvent>();
            ptr->init();
            ptr->loop_ = this;
            ptr->fd_ = fd;
            ptr->type_ = Event_Type_IO;
            this->io_evs_[fd] = ptr;
        } else{
            ptr = it->second;
        }
    }else{
        ptr = std::make_shared<IOEvent>();
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
Int Core::EventLoop::processEvents()
{
    if (this->status_ == Stop) return 0;
    Int ios_cnt = 0, timer_cnt = 0, idle_cnt = 0, pending_cnt = 0;
    Int block_time = default_event_loop_wait_timeout;
    // 处理定时器任务
    if (!this->timer_.empty()){
        updateTime();
        ULong min_next_timeout = (*(this->timer_.begin())).second->next_time_;
        Long block_us = (Long)(min_next_timeout - this->cur_time_);
        if (block_us <= 0){
            // 有超时事件
            goto PROCESS_TIMER;
        }
        ++block_us;
        block_time = (block_us < default_event_loop_wait_timeout * 1000) ?
                (Int)block_us : default_event_loop_wait_timeout * 1000;
    }
    if (this->io_count_ > 0){
        // 处理IO事件
        ios_cnt = this->processIo(block_time / 1000);
    }else{
        Base::sleepUs(block_time);
    }
    this->updateTime();
    if (this->status_ == Stop){
        return 0;
    }
    PROCESS_TIMER:
    if (this->timer_count_ > 0){
        // 处理定时事件
        timer_cnt = processTimer();
    }
    pending_cnt = this->pending_count_;
    if (pending_cnt == 0){
        if (this->idle_count_ > 0){
            pending_cnt = this->processIdle();
        }
    }
    pending_cnt = this->processPending();
    return pending_cnt;
}

/**
 * 处理空闲事件
 * @return
 */
Int Core::EventLoop::processIdle()
{
    if (this->status_ == Stop) return 0;
    Int idle_cnt = 0;
    std::shared_ptr<Core::IdleEvent> item;
    auto it = this->idle_evs_.begin();
    for (; it != this->idle_evs_.end();){
        if (this->status_ == Stop) return 0;
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
Int Core::EventLoop::processPending()
{
    if (this->pending_count_ == 0){
        return 0;
    }
    std::shared_ptr<Event> item;
    // 遍历优先级数组 (高 -> 低)
    Int pending_cnt = 0, i = Core::EventPriority::Event_Priority_Highest;
    for (; i >= 0; --i){
        if (this->status_ == Stop) return pending_cnt;
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
                        item->cb_ = nullptr;
                    }
                }
            }else{
                Log::logd("event pending error!");
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
Int Core::EventLoop::processTimer()
{
    if (this->status_ == Stop) return 0;
    Int timer_cnt = 0;
    ULong now_time = this->cur_time_;
    std::shared_ptr<Core::TimerEvent> item;
    if (!this->timer_.empty()){
        auto it = this->timer_.begin();
        for (; it != this->timer_.end();){
            // 检查 EventLoop 是否停止
            if (this->status_ == Stop) return timer_cnt;
            item = it->second;
            if (item->next_time_ > now_time){
                break;
            }
            if (item->repeat_ != Core::TimerEvent::infinity){
                --item->repeat_;
            }
            EventLoop::loop_event_pending(item);
            ++timer_cnt;
            this->updateTime();
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
Int Core::EventLoop::processIo(Int timeout)
{
    if (this->status_ != Running) return 0;
    Int io_cnt = this->selector_->pollEvent(timeout);
    return (io_cnt < 0) ? 0 : io_cnt;
}

/**
 * 更新事件循环时间
 */
void Core::EventLoop::updateTime()
{
    this->cur_time_ = Base::getTimeOfDayUs();
}

/**
 * 添加IO事件
 * @param ev：事件指针
 * @param cb：对应事件回调函数
 * @param events：待添加事件
 * @return
 */
Int Core::EventLoop::addIoEvent(const std::weak_ptr<Core::IOEvent> &ev, const Core::IOEventCB &cb, Int events)
{
    std::shared_ptr<Core::IOEvent> io = ev.lock();
    if (io){
        // 事件活跃
        if (!io->active_){
            EventLoop::loop_event_active(io);
            ++io->loop_->io_count_;
        }
        if (!io->ready_){
            io->ready_ = true;
        }
        if (cb){
            io->cb_ = cb;
        }
        // 将事件添加到Selector
        if (!(io->evs_ & events)){
            io->loop_->selector_->addEvent(io->fd_, events);
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
Int Core::EventLoop::removeIoEvent(const std::weak_ptr<Core::IOEvent> &ev, Int events)
{
    std::shared_ptr<Core::IOEvent> io = ev.lock();
    if (io){
#ifdef OS_WINDOWS
        if (io->fd_ < 3) return -1;
#else
        if (io->destroy_ || !io->active_) return -1;
#endif
        if (io->evs_ & events){
            io->loop_->selector_->delEvent(io->fd_, events);
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
Core::EventLoop::acceptIo(Base::socket_t listen_fd, const Core::IOAcceptCB &cb)
{
    std::shared_ptr<Core::IOEvent> io = this->getIoEvent(listen_fd);
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
Core::EventLoop::readIo(Base::socket_t fd, Size_t read_len,
                        const Core::IOReadCB &cb, const Core::IOReadErrCB& err_cb)
{
    std::shared_ptr<Core::IOEvent> io = this->getIoEvent(fd);
    if (cb){
        io->read_cb_ = cb;
    }
    if (read_len > 0){
        io->read_len_ = read_len;
        io->read_buf_.reserve(io->read_buf_.capacity() + read_len);
        if (read_len <= io->read_buf_.length()){
            // 需要读取的数据已经在缓冲区中，直接调用回调
            if (io->read_cb_){
                std::basic_string<Char> buffer(io->read_buf_, read_len);
                io->read_cb_(io, buffer);
                io->read_buf_.erase(0, read_len);
            }
            return io;
        }
    }
    if (err_cb){
        io->read_err_cb_ = err_cb;
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
Core::EventLoop::writeIo(Base::socket_t fd, const void *buf, Size_t len,
                         const Core::IOWriteCB &cb, const Core::IOWriteErrCB& err_cb)
{
    std::shared_ptr<Core::IOEvent> io = this->getIoEvent(fd);
    if (cb){
        io->write_cb_ = cb;
    }
    if (err_cb){
        io->write_err_cb_ = err_cb;
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
Core::EventLoop::closeIo(Base::socket_t fd)
{
    std::shared_ptr<Core::IOEvent> io = this->getIoEvent(fd);
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
Core::EventLoop::addTimer(const Core::TimerEventCB &cb, UInt timeout, Int repeat)
{
    std::shared_ptr<Core::TimerEvent> timer(new TimerEvent);
    timer->init();
    timer->type_ = Core::Event_Type_Timer;
    timer->priority_ = Core::Event_Priority_Highest;
    timer->timeout_ = timeout;
    timer->repeat_ = repeat;
    this->updateTime();
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
Core::EventLoop::removeTimer(const std::weak_ptr<Core::TimerEvent>& ev)
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
Core::EventLoop::addIdle(const Core::IdleEventCB &cb, Int repeat)
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
Core::EventLoop::removeIdle(const std::weak_ptr<Core::IdleEvent>& ev)
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
Core::EventLoop::addCustom(const Core::EventCB &cb) {
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

void
Core::EventLoop::changeIoLoop(const std::weak_ptr<Core::IOEvent> &ev, Core::EventLoopPtr loop)
{
    auto io = ev.lock();
    if (io) {
        Base::socket_t fd = io->fd_;
        Core::EventLoopPtr old_loop = io->loop_;
        // 判断新loop里面是否有重复io
        auto it = old_loop->io_evs_.find(fd);
        if (it != old_loop->io_evs_.end()) {
            old_loop->io_evs_.erase(it);
        }
        io->loop_ = loop;
        loop->io_evs_[fd] = io;
    }
}

