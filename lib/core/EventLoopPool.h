/**
* FileName:   EventLoopPool.h
* CreateDate: 2022-03-11 14:51:05
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        事件循环线程池
*/
#ifndef SINBACK_EVENTLOOPPOOL_H
#define SINBACK_EVENTLOOPPOOL_H

#include "core/EventLoopThread.h"

namespace SinBack
{
    namespace Core
    {
        class EventLoopPool : noncopyable
        {
        public:
            using EventLoopThreadPtr = std::shared_ptr<EventLoopThread>;
            using Func = std::function<void(void)>;
        public:
            explicit EventLoopPool(UInt count = std::thread::hardware_concurrency());
            ~EventLoopPool();

            bool start(const Func& begin_func = nullptr, const Func& end_func = nullptr);
            void stop(bool is_join = false);
            void pause();
            void resume();
            void join();

            SharedPtr<Core::EventLoop> loop(Int index = -1);

        private:
            SharedPtr<Core::EventLoop> autoLoop();
        private:
            // 事件循环线程数组
            std::vector<EventLoopThreadPtr> loop_threads_;
            // 事件循环线程数量
            UInt th_count_;
            // 获取事件循环的下标
            std::atomic<UInt> loop_index_;
        };
    }
}


#endif //SINBACK_EVENTLOOPPOOL_H
