/**
* FileName:   Selector.cc
* CreateDate: 2022-03-08 23:32:06
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        IO 多路复用
*/
#include "Selector.h"
#include "EventLoop.h"

using namespace SinBack;

Core::Selector::Selector(Core::EventLoop* loop)
    : error_(false)
    , events_({})
    , loop_(loop)
{
    this->fd_ = epoll_create(Selector::default_selector_size);
    error_ = (this->fd_ == -1);
    if (error_){
        Log::FLogE("epoll create error_!");
    }
}

Core::Selector::~Selector()
{
    ::close(this->fd_);
}

/**
 * 添加（监听）事件
 * @param fd : 要监听的套接字
 * @param events : 需要监听的事件
 * @return
 */
bool Core::Selector::addEvent(Base::socket_t fd, Int events)
{
    epoll_event ev{};
    std::shared_ptr<Core::IOEvent> io = this->loop_->getIoEvent(fd);
    ev.data.fd = fd;
    if (io->evs_ & Core::IO_READ){
        ev.events |= EPOLLIN;
    }
    if (io->evs_ & Core::IO_WRITE){
        ev.events |= EPOLLOUT;
    }
    if (io->evs_ & Core::IO_TYPE_ET){
        ev.events |= EPOLLET;
    }
    if (events & Core::IO_READ){
        ev.events |= EPOLLIN;
    }
    if (events & Core::IO_WRITE){
        ev.events |= EPOLLOUT;
    }
    if (events & Core::IO_TYPE_ET){
        ev.events |= EPOLLET;
    }
    Int opt = (io->evs_ == 0) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    opt = ::epoll_ctl(this->fd_, opt, fd, &ev);
    return (opt != -1);
}

/**
 * 删除（取消监听）事件
 * @param fd : 套接字
 * @param events : 事件
 * @return
 */
bool Core::Selector::delEvent(Base::socket_t fd, Int events)
{
    epoll_event ev{};
    ev.data.fd = fd;
    std::shared_ptr<Core::IOEvent> io = this->loop_->getIoEvent(fd);
    if (io->evs_ & Core::IO_READ){
        ev.events |= EPOLLIN;
    }
    if (io->evs_ & Core::IO_WRITE){
        ev.events |= EPOLLOUT;
    }
    if (io->evs_ & Core::IO_TYPE_ET){
        ev.events |= EPOLLET;
    }
    if (events & Core::IO_READ){
        ev.events &= ~EPOLLIN;
    }
    if (events & Core::IO_WRITE){
        ev.events &= ~EPOLLOUT;
    }
    if (events & Core::IO_TYPE_ET){
        ev.events &= ~EPOLLET;
    }
    Int opt = (ev.events == 0) ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    opt = ::epoll_ctl(this->fd_, opt, fd, &ev);
    return (opt != -1);
}

/**
 * 向 Epoll 询问事件
 * @param timeout : Epoll 超时时间
 * @return
 */
Int Core::Selector::pollEvent(Int timeout)
{
    Int ep_cnt = ::epoll_wait(this->fd_, this->events_.data(), (Int)this->events_.size(), timeout);
    if (ep_cnt < 0){
        // 被信号中断
        if (errno == EINTR){
            return 0;
        }
        Log::FLogE("epoll_wait error_, return code is %d", ep_cnt);
        return ep_cnt;
    }
    if (ep_cnt == 0){
        // 超时
        return 0;
    }
    Int ev_cnt = 0, i = 0, fd;
    UInt events;
    epoll_event* ev_ptr;

    for (; i < ep_cnt; ++i){
        ev_ptr = &this->events_[i];
        fd = ev_ptr->data.fd;
        events = ev_ptr->events;
        if (events){
            ++ev_cnt;
            // 获取对于套接字，并设置套接字活动事件
            std::shared_ptr<Core::IOEvent> io = this->loop_->getIoEvent(fd);
            if (io){
                if (events & (EPOLLIN | EPOLLERR | EPOLLHUP)){
                    io->active_evs_ |= Core::IO_READ;
                }
                if (events & (EPOLLOUT | EPOLLERR | EPOLLHUP)){
                    io->active_evs_ |= Core::IO_WRITE;
                }
                EventLoop::loop_event_pending(io);
            }
        }
        if (ev_cnt == ep_cnt) break;
    }
    return ev_cnt;
}