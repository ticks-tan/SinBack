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
#include <deque>
#include <atomic>
#include <mutex>
#include <memory>
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
            // 空闲事件
            Event_Type_Idle     = 4
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
        // 空闲事件
        struct IdleEvent;

        // 回调函数
        using EventCB = std::function<void(const std::weak_ptr<Event>&)>;
        using IdleEventCB = std::function<void(const std::weak_ptr<IdleEvent>&)>;
        using IOEventCB = std::function<void(const std::weak_ptr<IOEvent>&)>;
        using TimerEventCB = std::function<void(const std::weak_ptr<TimerEvent>&)>;
        using IOAcceptCB = std::function<void(const std::weak_ptr<IOEvent>&)>;
        using IOReadCB = std::function<void(const std::weak_ptr<IOEvent>&, const std::basic_string<Char>&)>;
        using IOReadErrCB = std::function<void(const std::weak_ptr<IOEvent>&, const std::basic_string<Char>&)>;
        using IOWriteCB = std::function<void(const std::weak_ptr<IOEvent>&, Size_t)>;
        using IOWriteErrCB = std::function<void(const std::weak_ptr<IOEvent>&, const std::basic_string<Char>&)>;
        using IOCloseCB = std::function<void(const std::weak_ptr<IOEvent>&)>;

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
            EventType type_ = Event_Type_None;
            // 事件优先级
            EventPriority priority_ = Event_Priority_Lowest;
            // EventLoop
            EventLoopPtr loop_ = nullptr;
            // 是否活跃
            bool active_ = false;
            // 是否为待处理状态
            bool pending_ = false;
            // 回调函数
            EventCB cb_ = nullptr;
            // 自定义数据指针
            void* context_ = nullptr;

            // 初始化
            virtual void init(){
                pid_ = getpid();
                id_ = event_next_id();
                cb_ = nullptr;
                type_ = EventType::Event_Type_Custom;
                active_ = pending_ = false;
                priority_ = Event_Priority_Lowest;
                context_ = nullptr;
                init_ = true;
                loop_ = nullptr;
            }
        };

        // IO事件
        struct IOEvent : public Event, public std::enable_shared_from_this<IOEvent>
        {
            using string_type = String;
            static const Size_t default_keep_alive_timeout = 1000;
            // 监听事件
            Int evs_ = 0;
            // 事件回调
            IOEventCB cb_ = nullptr;
            // 错误
            Int error = 0;
            // 活动事件
            Int active_evs_ = 0;
            // socket_t
            Base::socket_t fd_ = -1;
            // 是否就绪
            bool ready_ = false;
            // 是否 accept
            bool accept_ = false;
            // 是否关闭
            bool closed = false;
            // accept 回调
            IOAcceptCB accept_cb_ = nullptr;
            // read 回调
            IOReadCB read_cb_ = nullptr;
            // read 错误回调
            IOReadErrCB read_err_cb_ = nullptr;
            // write 回调
            IOWriteCB write_cb_ = nullptr;
            // write错误回调
            IOWriteErrCB write_err_cb_ = nullptr;
            // close 回调
            IOCloseCB close_cb_ = nullptr;
            // 上次读取事件
            ULong last_read_time_ = 0;
            // 上次写入时间
            ULong last_write_time_ = 0;
            // 保持活跃时间
            Size_t keep_alive_ms_ = 0;
            // 读取缓冲区
            string_type read_buf_;
            // 读取长度
            Size_t read_len_ = 0;
            // 写入缓冲队列
            std::deque<string_type> write_queue_;
            // 写入锁
            std::mutex write_mutex_;

            // 初始化
            void init() override{
                // 父类初始化
                Event::init();
                Event::type_ = EventType::Event_Type_IO;
                Event::priority_ = Event_Priority_Normal;
                evs_ = active_evs_ = 0;
                fd_ = Base::socket_t(-1);
                ready_ = accept_ = false;
                closed = false;
                error = 0;
                read_len_ = 0;
                last_read_time_ = 0;
                last_write_time_ = 0;
                keep_alive_ms_ = 0;
            }
            // 开始准备
            void ready();
            // 准备关闭
            void clean();
            // 接收连接
            Int accept();
            // 读取数据
            Int read();
            Int read(Size_t len);
            // 写入数据
            Int write(const void* buf, Size_t len);
            // 关闭连接
            Int close(bool timer = true);
            // 设置keep-alive时间
            bool setKeepAlive(Size_t timout_ms);
        };

        struct TimerEvent : public Event
        {
            static const Int infinity = -1;

            // 定时器执行次数
            Int repeat_ = 0;
            // 超时时间
            ULong timeout_ = 0;
            // 下次超时时间
            ULong next_time_ = 0;
            // 回调事件
            TimerEventCB cb_ = nullptr;

            // 初始化
            void init() override{
                Event::init();
                timeout_ = next_time_ = repeat_ = 0;
                Event::type_ = Event_Type_Timer;
                Event::priority_ = Event_Priority_Highest;
            }
        };

        // 空闲事件
        struct IdleEvent : Event
        {
            static const Int infinity = -1;
            // 回调函数
            IdleEventCB cb_ = nullptr;
            // 执行次数
            Int repeat_ = 0;

            // 初始化
            void init() override {
                Event::init();
                Event::type_ = Event_Type_Idle;
                repeat_ = 0;
                cb_ = nullptr;
            }
        };
    }
}


#endif //SINBACK_EVENT_H
