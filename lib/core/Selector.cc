/**
* FileName:   Selector.cc
* CreateDate: 2022-03-08 23:32:06
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        IO 多路复用
*/
#include "core/Selector.h"

using namespace SinBack;

#ifdef OS_WINDOWS
#else

Core::Selector::Selector()
    : error_(false)
    , events_({})
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
    return false;
}

bool Core::Selector::del_event(Base::socket_t fd, Int events) {
    epoll_event ev{};
    return false;
}

Int Core::Selector::poll_event() {
    return 0;
}

#endif