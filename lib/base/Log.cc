/**
* FileName:   Log.cc
* CreateDate: 2022-03-03 23:08:32
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "Log.h"
using namespace SinBack;

Log::Logger::Logger(const char *filename, LogLevel level, Size_t max_log_num)
    : flag_(false)
    , max_log_num_(max_log_num)
    , level_(level)
    , th_(nullptr)
    , file_(nullptr)
{
    if (filename){
        this->file_.reset(new Base::File(filename, Base::File::RDWR));
    }else{
        this->file_.reset(new Base::File("SinBack-tmp.log", Base::File::RDWR));
    }
    this->th_.reset(new std::thread(&Logger::RunInBack, this));
}

Log::Logger::~Logger()
{
    this->stop();
    this->cv_.notify_all();
}

void Log::Logger::RunInBack()
{
    if (this->flag_){
        return;
    }
    std::string buf;
    this->flag_ = true;

    while (this->flag_){
        {
            std::unique_lock<std::mutex> lock(this->back_mutex_);
            this->cv_.wait(lock, [this]() -> bool {
                return (!this->flag_ || !this->back_.empty());
            });
            buf = this->back_.front();
            this->front_.pop();
            this->file_->write(buf.c_str(), buf.size(), true);
            if (!this->flag_){
                break;
            }
        }
    }
}

void Log::Logger::swap_buffer()
{
    {
        std::unique_lock<std::mutex> lock(this->back_mutex_);
        this->back_.swap(this->front_);
    }
    this->cv_.notify_one();
}

void Log::Logger::stop()
{
    this->flag_ = false;
    if (this->th_->joinable()){
        this->th_->join();
    }
}
