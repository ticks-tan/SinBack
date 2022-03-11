/**
* FileName:   Event.h
* CreateDate: 2022-03-08 21:52:29
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        事件定义
*/
#ifndef SINBACK_EVENT_H
#define SINBACK_EVENT_H

#include <functional>
#include <memory>
#include <string>
#include <deque>
#include <atomic>
#include <mutex>
#include "base/SocketUtil.h"

namespace SinBack
{
    namespace Core
    {
        static std::atomic<Size_t> g_event_id;
        static inline Size_t event_next_id(){
            return ++g_event_id;
        }
        // 事件类型
        enum EventType : Int
        {
            // 未知类型
            Event_Type_None     = 0,
            // 自定义事件
            Event_Type_Custom   = 1,
            // IO事件
            Event_Type_IO       = 2,
            // 定时事件
            Event_Type_Timer    = 3,
            // 超时事件
            Event_Type_TimeOut  = 4,
            // 空闲事件
            Event_Type_Idle     = 5
        };

        enum EventPriority : Int
        {
            Event_Priority_Lowest = 0,
            Event_Priority_Low = 1,
            Event_Priority_Normal = 2,
            Event_Priority_High = 3,
            Event_Priority_Highest = 4
        };
        static constexpr Int event_priority_size(){
            return (Event_Priority_Highest - Event_Priority_Lowest + 1);
        }

        // 前置声明
        class EventLoop;
        // 事件基类
        struct Event;
        // IO事件
        struct IOEvent;
        // 定时事件
        struct TimerEvent;
        // 超时事件
        struct TimeOutEvent;
        // 空闲事件
        struct IdleEvent;

        // 回调函数
        using EventCB = std::function<void(const std::weak_ptr<Event>&)>;
        using IdleEventCB = std::function<void(const std::weak_ptr<IdleEvent>&)>;
        using IOEventCB = std::function<void(const std::weak_ptr<IOEvent>&)>;
        using TimerEventCB = std::function<void(const std::weak_ptr<TimerEvent>&)>;
        using IOAcceptCB = std::function<void(IOEvent*)>;
        using IOReadCB = std::function<void(IOEvent*, std::basic_string<Char>&)>;
        using IOWriteCB = std::function<void(IOEvent*, const std::basic_string<Char>&)>;
        using IOCloseCB = std::function<void(IOEvent*)>;

        using EventLoopPtr = EventLoop*;
        // 事件基类
    struct Event{
            // pid
            pid_t pid_ = 0;
            //  事件id
            Size_t id_ = 0;
            // 是否初始化
            bool init_ = false;
            // 事件类型
            EventType type_;
            // 事件优先级
            EventPriority priority_;
            // EventLoop
            EventLoopPtr loop_;
            // 是否活跃
            bool active_ = false;
            // 是否需要销毁
            bool destroy_ = false;
            // 是否为待处理状态
            bool pending_ = false;
            // 回调函数
            EventCB cb_;
            // 自定义数据指针
            void* context_ = nullptr;

            // 初始化
            virtual void init(){
                pid_ = getpid();
                id_ = event_next_id();
                type_ = EventType::Event_Type_Custom;
                active_ = destroy_ = pending_ = false;
                priority_ = Event_Priority_Lowest;
                context_ = nullptr;
                init_ = true;
            }
        };

        // IO事件
        struct IOEvent : public Event, public std::enable_shared_from_this<IOEvent>
        {
            using string_type = std::basic_string<Char>;
            // 监听事件
            Int evs_ = 0;
            // 事件回调
            IOEventCB cb_;
            // 活动事件
            Int active_evs_ = 0;
            // socket_t
            Base::socket_t fd_;
            // 是否就绪
            bool ready_ = false;
            // 是否 accept
            bool accept_ = false;
            // 是否关闭
            bool closed = false;
            // 各事件回调
            IOAcceptCB accept_cb_;
            IOReadCB read_cb_;
            IOWriteCB write_cb_;
            IOCloseCB close_cb_;
            // 读取缓冲区
            string_type read_buf_;
            // 写入队列
            std::deque<string_type> write_queue_;
            // 写入锁
            std::mutex write_mutex_;

            // 初始化
            void init() override{
                // 父类初始化
                Event::init();
                type_ = EventType::Event_Type_IO;
                evs_ = active_evs_ = 0;
                fd_ = Base::socket_t(-1);
                ready_ = accept_ = false;
                closed = false;
            }

            // 接收连接
            Int accept();
            // 读取数据
            Int read();
            // 写入数据
            Int write(const void* buf, Size_t len);
            // 关闭连接
            Int close();
        };

        struct TimerEvent : public Event
        {
            static const Int infinity = -1;

            // 定时器执行次数
            Int repeat_ = 0;
            // 超时时间
            Long timeout_ = 0;
            // 下次超时时间
            ULong next_time_ = 0;
            // 回调事件
            TimerEventCB cb_;

            // 初始化
            void init() override{
                Event::init();
                timeout_ = next_time_ = repeat_ = 0;
            }
        };

        // 空闲事件
        struct IdleEvent : Event
        {
            static const Int infinity = -1;
            // 回调函数
            IdleEventCB cb_;
            // 执行次数
            Int repeat_ = 0;
        };
    }
}


#endif //SINBACK_EVENT_H
