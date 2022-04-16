/**
* FileName:   EventLoopThread.cpp
* CreateDate: 2022-03-11 14:21:27
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "EventLoopThread.h"

SinBack::Core::EventLoopThread::EventLoopThread()
    : running_(false)
{
    this->loop_.reset(new EventLoop);
}

SinBack::Core::EventLoopThread::~EventLoopThread()
{
    this->stop(true);
}

SinBack::SharedPtr<SinBack::Core::EventLoop>
SinBack::Core::EventLoopThread::loop()
{
    return this->loop_;
}

bool
SinBack::Core::EventLoopThread::start(const SinBack::Core::EventLoopThread::Func &begin_func,
                                           const SinBack::Core::EventLoopThread::Func &end_func){
    if (this->running_){
        return false;
    }
    this->th_.reset(new std::thread(std::bind(&EventLoopThread::threadFunc, this, begin_func, end_func)));
    return true;
}

void
SinBack::Core::EventLoopThread::stop(bool is_join)
{
    if (!this->running_){
        return;
    }
    this->loop_->stop();
    if (is_join){
        this->join();
    }
}

void
SinBack::Core::EventLoopThread::pause()
{
    if (!this->running_){
        return;
    }
    this->loop_->pause();
}

void
SinBack::Core::EventLoopThread::resume()
{
    if (!this->running_){
        return;
    }
    this->loop_->resume();
}

void
SinBack::Core::EventLoopThread::join()
{
    if (this->th_->joinable()){
        this->th_->join();
        return;
    }
}

void
SinBack::Core::EventLoopThread::threadFunc(const Func& begin_func, const Func& end_func)
{
    this->running_ = true;
    if (begin_func){
        this->loop_->addCustom([&begin_func](const std::weak_ptr<Core::Event> &ev) {
            if (begin_func) {
                begin_func();
            }
        });
    }
    this->loop_->run();
    this->running_ = false;
    if (end_func){
        end_func();
    }
}
