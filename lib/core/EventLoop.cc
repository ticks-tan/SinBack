/**
* FileName:   EventLoop.cc
* CreateDate: 2022-03-09 09:56:06
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "EventLoop.h"

using namespace SinBack;

static std::atomic<ULong> g_event_loop_id_count;

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
        // 处理事件
        this->process_events();
        update_time();
    }
    this->status_ = Stop;
    this->end_time_ = Base::gettimeofday_ms();
    return true;
}

bool Core::EventLoop::stop()
{
    if (this->status_ == Running){
        this->status_ = Stop;
        return true;
    }
    return false;
}

bool Core::EventLoop::pause()
{
    if (this->status_ == Running){
        this->status_ = Pause;
        return true;
    }
    return false;
}

bool Core::EventLoop::resume()
{
    if (this->status_ == Pause){
        this->status_ == Running;
        return true;
    }
    return false;
}

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

Int Core::EventLoop::process_events()
{
    Int ios_cnt = 0, timer_cnt = 0, idle_cnt = 0, pending_cnt = 0;
    Int block_time = default_event_loop_wait_timeout;
    // 处理定时器任务
    if (!this->timer_.empty()){
        update_time();
        ULong min_next_timeout = (*(this->timer_.begin()))->next_time_;
        Long block_ms = (Long)min_next_timeout - Base::gettimeofday_ms();
        if (block_ms <= 0){
            // 有超时事件
            goto PROCESS_TIMER;
        }
        ++block_ms;
        block_time = (block_ms < default_event_loop_wait_timeout) ? (Int)block_ms : default_event_loop_wait_timeout;
    }
    if (this->io_count_ > 0){
        // 处理IO事件
        ios_cnt = this->process_io(block_time);
    }else{
        Base::sleep_ms(block_time);
    }
    update_time();
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
            it = this->idle_evs_.erase(it);
            continue;
        }
        ++it;
    }
    return idle_cnt;
}

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
            if (item){
                if (item->pending_){
                    // 执行回调函数
                    if (item->active_ && item->cb_){
                        item->cb_(item.get());
                        ++pending_cnt;
                    }
                    item->pending_ = false;
                    if (item->destroy_){
                        loop_event_inactive(item);
                    }
                }
            }
        }
        // 清理待执行链表
        list.clear();
    }
    this->pending_count_ = 0;
    return pending_cnt;
}

Int Core::EventLoop::process_timer()
{
    Int timer_cnt = 0;
    ULong now_time = this->cur_time_;
    std::shared_ptr<Core::TimerEvent> item;
    if (!this->timer_.empty()){
        auto it = this->timer_.begin();
        for (; it != this->timer_.end();){
            item = *it;
            if (item->next_time_ > now_time){
                break;
            }
            if (item->repeat_ != Core::TimerEvent::infinity){
                --item->repeat_;
            }
            EventLoop::loop_event_pending(item);
            ++timer_cnt;

            if (item->repeat_ == 0){
                // 次数用尽，删除定时器
                item->loop_->timer_count_--;
                item->destroy_ = true;
                it = this->timer_.erase(it);
            } else{
                while (item->next_time_ <= now_time){
                    item->next_time_ += item->timeout_;
                }
                this->timer_.insert(item);
                it = this->timer_.erase(it);
            }
        }
    }
    return timer_cnt;
}

Int Core::EventLoop::process_io(Int timeout)
{
    Int io_cnt = this->selector_->poll_event();
    return (io_cnt < 0) ? 0 : io_cnt;
}

