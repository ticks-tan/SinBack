/**
* FileName:   EventLoopThread.h
* CreateDate: 2022-03-11 13:41:44
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        事件循环线程
*/
#ifndef SINBACK_EVENTLOOPTHREAD_H
#define SINBACK_EVENTLOOPTHREAD_H

#include "core/EventLoop.h"

namespace SinBack
{
    namespace Core {
        // 事件循环线程
        class EventLoopThread : SinBack::noncopyable
        {
        public:
            using EventLoopPtr = std::shared_ptr<EventLoop>;
            using Func = std::function<void(void)>;
        public:
            EventLoopThread();
            ~EventLoopThread();

            bool start(const Func& begin_func = nullptr, const Func& end_func = nullptr);
            void stop(bool is_join = false);
            void pause();
            void resume();
            void join();

            EventLoopPtr& loop();

        private:
            void thread_func(const Func& begin_func, const Func& end_func);
        private:
            // 线程
            std::shared_ptr<std::thread> th_;
            // 事件循环
            std::shared_ptr<EventLoop> loop_;
            // 线程运行标识
            bool stop_;
            bool running_;
        };
    }
}

#endif //SINBACK_EVENTLOOPTHREAD_H
