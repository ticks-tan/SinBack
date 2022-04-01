/**
* FileName:   ThreadPool.h
* CreateDate: 2022-03-08 21:47:42
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        线程池
*/
#ifndef SINBACK_THREADPOOL_H
#define SINBACK_THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <stdexcept>
#include "base/Base.h"

namespace SinBack {
    namespace Base {
        class ThreadPool {
        public:
            explicit ThreadPool(Size_t);

            template<class F, class... Args>
            auto enqueue(F &&f, Args &&... args)
            -> std::future<typename std::result_of<F(Args...)>::type>;

            void stop();
            ~ThreadPool();

        private:
            // need to keep track of threads so we can join them
            std::vector<std::thread> workers_;
            // the task queue
            std::queue<std::function<void()> > tasks_;

            // synchronization
            std::mutex queue_mutex_;
            std::condition_variable cv_;
            bool stop_;
        };
    }
}

// the constructor just launches some amount of workers_
inline SinBack::Base::ThreadPool::ThreadPool(SinBack::Size_t threads)
        : stop_(false)
{
    for(Size_t i = 0;i<threads;++i)
        workers_.emplace_back(
                [this]
                {
                    for(;;)
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex_);
                            this->cv_.wait(lock,
                                           [this]{ return this->stop_ || !this->tasks_.empty(); });
                            if(this->stop_ && this->tasks_.empty())
                                return;
                            task = std::move(this->tasks_.front());
                            this->tasks_.pop();
                        }
                        task();
                    }
                }
        );
}

// add new work item to the pool
template<class F, class... Args>
auto SinBack::Base::ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        // don't allow enqueueing after stopping the pool
        if(stop_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks_.emplace([task](){ (*task)(); });
    }
    cv_.notify_one();
    return res;
}

inline void SinBack::Base::ThreadPool::stop()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    cv_.notify_all();
}

// the destructor joins all threads
inline SinBack::Base::ThreadPool::~ThreadPool()
{
    this->stop();
    for(std::thread &worker: workers_) {
        if (worker.joinable())
            worker.join();
    }
}

#endif //SINBACK_THREADPOOL_H
