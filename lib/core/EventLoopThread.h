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
            using Func = std::function<void(void)>;
        public:
            EventLoopThread();
            ~EventLoopThread();

            bool start(const Func& begin_func = nullptr, const Func& end_func = nullptr);
            void stop(bool is_join = false);
            void pause();
            void resume();
            void join();

            bool hasSSL() {
                return this->loop_->getSSL() != nullptr;
            }

            SharedPtr<Core::EventLoop> loop();

        private:
            void threadFunc(const Func& begin_func, const Func& end_func);
        private:
            // 线程
            SharedPtr<std::thread> th_;
            // 事件循环
            SharedPtr<EventLoop> loop_;
            // 线程运行标识
            bool running_;
        };
    }
}

#endif //SINBACK_EVENTLOOPTHREAD_H
