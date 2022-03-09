/**
* FileName:   Selector.h
* CreateDate: 2022-03-08 23:21:22
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        IO多路复用
*/
#ifndef SINBACK_SELECTOR_H
#define SINBACK_SELECTOR_H

#ifdef OS_WINDOWS
#else
#include <sys/epoll.h>
#include <sys/types.h>
#endif

#include "base/noncopyable.h"
#include "core/Event.h"

namespace SinBack
{
    namespace Core
    {
        enum SelectorEventType {
            IO_READ = 0x001,
            IO_WRITE = 0x004,
            IO_RDWR = (IO_READ | IO_WRITE)
        };
#ifdef OS_WINDOWS
#else
        // Epoll
        class Selector : noncopyable
        {
        public:
            static const Int default_selector_size = 256;
            friend class EventLoop;
        public:
            Selector();
            ~Selector();
            // 添加事件
            bool add_event(Base::socket_t fd, Int events);
            // 删除事件
            bool del_event(Base::socket_t fd, Int events);
            // 处理事件
            Int poll_event();
        private:
            // epoll 套接字
            Int fd_;
            // 临时事件数组
            std::array<epoll_event, default_selector_size> events_;
            // EventLoop
            std::shared_ptr<EventLoop> loop_;
            // error
            bool error_;
        };
#endif
    }
}

#endif //SINBACK_SELECTOR_H
