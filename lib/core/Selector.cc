/**
* FileName:   Selector.cc
* CreateDate: 2022-03-08 23:32:06
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        IO 多路复用
*/
#include "core/Selector.h"
#include "core/EventLoop.h"

using namespace SinBack;

#ifdef OS_WINDOWS
#else

Core::Selector::Selector(Core::EventLoop* loop)
    : error_(false)
    , events_({})
    , loop_(loop)
{
    this->fd_ = epoll_create(Selector::default_selector_size);
    error_ = (this->fd_ == -1);
}

Core::Selector::~Selector()
{
    ::close(this->fd_);
}

bool Core::Selector::add_event(Base::socket_t fd, Int events)
{
    epoll_event ev{};
    std::shared_ptr<Core::IOEvent> io = this->loop_->get_io_event(fd);
    ev.data.fd = fd;
    if (io->evs_ & Core::IO_READ){
        ev.events |= EPOLLIN;
    }
    if (io->evs_ & Core::IO_WRITE){
        ev.events |= EPOLLOUT;
    }
    if (events & Core::IO_READ){
        ev.events |= EPOLLIN;
    }
    if (events & Core::IO_WRITE){
        ev.events |= EPOLLOUT;
    }
    Int opt = (io->evs_ == 0

            ) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    ::epoll_ctl(this->fd_, opt, fd, &ev);
    return true;
}

bool Core::Selector::del_event(Base::socket_t fd, Int events) {
    epoll_event ev{};
    return false;
}

Int Core::Selector::poll_event() {
    return 0;
}

#endif