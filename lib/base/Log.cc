/**
* FileName:   Log.cc
* CreateDate: 2022-03-03 23:08:32
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "Log.h"

#include <memory>
using namespace SinBack;

Log::Logger::Logger(const char *filename, LogLevel level, Size_t max_log_num)
    : stop_(false)
    , last_time_(clock())
    , time_()
    , max_log_num_(max_log_num)
    , level_(level)
    , th_(nullptr)
    , file_(nullptr)
    , front_(std::make_unique<std::queue<std::string>>())
    , back_(std::make_unique<std::queue<std::string>>())
{
    if (filename){
        this->file_ = std::make_unique<Base::File>(filename, Base::File::RDWR);
    }else{
        this->file_ = std::make_unique<Base::File>("SinBack-tmp.log", Base::File::RDWR);
    }
    this->th_ = std::make_unique<std::thread>(&Logger::RunInBack, this);
    Base::getdatetimenow(&this->time_);
}

Log::Logger::~Logger()
{
    this->stop();
    this->cv_.notify_all();
}

void Log::Logger::RunInBack()
{
    std::string buf;
    for (;;){
        {
            std::unique_lock<std::mutex> lock(this->back_mutex_);
            this->cv_.wait(lock, [this]{
                return (this->stop_ || !this->back_->empty());
            });
            if (this->stop_ && this->back_->empty()){
                return;
            }
            if (!this->back_->empty()) {
                buf = std::move(this->back_->front());
                this->back_->pop();
                this->file_->write(buf.c_str(), buf.size(), true);
            }
        }
    }
}

void Log::Logger::swap_buffer()
{
    {
        std::unique_lock<std::mutex> lock(this->back_mutex_);
        while(!this->back_->empty()){
            this->front_->push(std::move(this->back_->front()));
            this->back_->pop();
        }
        this->back_.swap(this->front_);
    }
    this->cv_.notify_one();
}

void Log::Logger::stop()
{
    this->stop_ = true;
    this->swap_buffer();
    if (this->th_->joinable()){
        this->th_->join();
    }
}
