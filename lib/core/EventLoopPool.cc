/**
* FileName:   EventLoopPool.cc
* CreateDate: 2022-03-11 14:51:05
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "EventLoopPool.h"


SinBack::Core::EventLoopPool::EventLoopPool(SinBack::UInt count)
    : th_count_(count)
    , loop_index_()
{
    this->loop_index_.store(0);
}

SinBack::Core::EventLoopPool::~EventLoopPool()
{
    this->stop(true);
}

SinBack::SharedPtr<SinBack::Core::EventLoop> SinBack::Core::EventLoopPool::autoLoop()
{
    return this->loop_threads_[(++this->loop_index_) % this->loop_threads_.size()]->loop();
}

bool SinBack::Core::EventLoopPool::start(const Func &begin_func, const Func &end_func)
{
    UInt i = 0;
    for (; i < this->th_count_; ++i){
        this->loop_threads_.emplace_back(new Core::EventLoopThread);
        // this->loop_threads_.push_back(std::make_shared<Core::EventLoopThread>());
        this->loop_threads_.back()->start(
                std::forward<const Func&>(begin_func),
                        std::forward<const Func&>(end_func));
    }
    return true;
}

void SinBack::Core::EventLoopPool::stop(bool is_join)
{
    for (const auto& it : this->loop_threads_){
        it->stop(is_join);
    }
}

void SinBack::Core::EventLoopPool::pause()
{
    for (const auto& it : this->loop_threads_){
        it->pause();
    }
}

void SinBack::Core::EventLoopPool::resume()
{
    for (const auto& it : this->loop_threads_){
        it->resume();
    }
}

void SinBack::Core::EventLoopPool::join()
{
    for (const auto& it : this->loop_threads_){
        it->join();
    }
}

SinBack::SharedPtr<SinBack::Core::EventLoop>
SinBack::Core::EventLoopPool::loop(SinBack::Int index)
{
    if (index < 0){
        return this->autoLoop();
    } else{
        return this->loop_threads_[index]->loop();
    }
}
