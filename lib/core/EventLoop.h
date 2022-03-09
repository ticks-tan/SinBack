/**
* FileName:   EventLoop.h
* CreateDate: 2022-03-09 09:56:06
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_EVENTLOOP_H
#define SINBACK_EVENTLOOP_H

#include <forward_list>
#include <unordered_map>
#include "core/Selector.h"

namespace SinBack
{
    namespace Core
    {
        class EventLoop : noncopyable
        {
            using EventPtr = std::weak_ptr<Event>;
            using EventPtrList = std::forward_list<EventPtr>;
        private:
            // 进程pid
            pid_t pid_;
            // 开始时间
            ULong start_time_;
            // 结束时间
            ULong end_time_;
            // 循环次数
            LLong loop_count_;

            // Selector
            std::shared_ptr<Selector> selector_;

            // 待处理事件队列
            std::array<EventPtrList , event_priority_size()> pending_evs_;
            // 待处理事件数量
            Int pending_count_;

            // IO 事件集合
            std::unordered_map<Base::socket_t, std::shared_ptr<IOEvent>> io_evs_;
            // IO事件数量
            Int io_count_;

            // 自定义事件队列
            std::forward_list<std::shared_ptr<Event>> custom_evs_;
            Int custom_count_;

        };
    }
}


#endif //SINBACK_EVENTLOOP_H
