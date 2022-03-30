/**
* FileName:   Selector.h
* CreateDate: 2022-03-08 23:21:22
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        IO多路复用
*/
#ifndef SINBACK_SELECTOR_H
#define SINBACK_SELECTOR_H

#include <sys/epoll.h>
#include <sys/types.h>
#include "core/Event.h"

namespace SinBack
{
    namespace Core
    {
        enum SelectorEventType : UInt{
            IO_READ = 0x001,
            IO_WRITE = 0x004,
            IO_RDWR = (IO_READ | IO_WRITE),
            IO_TYPE_ET = 1u << 31
        };
        // Epoll
        class Selector : noncopyable
        {
        public:
            static const Int default_selector_size = 256;
            friend class EventLoop;
        public:
            explicit Selector(EventLoop* loop);
            ~Selector();
            // 添加事件
            bool addEvent(Base::socket_t fd, Int events);
            // 删除事件
            bool delEvent(Base::socket_t fd, Int events);
            // 处理事件
            Int pollEvent(Int timeout);
        private:
            // epoll 套接字
            Int fd_;
            // 临时事件数组
            std::array<epoll_event, default_selector_size> events_;
            // EventLoop
            EventLoop* loop_;
            // error
            bool error_;
        };
    }
}

#endif //SINBACK_SELECTOR_H
