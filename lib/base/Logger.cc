/**
* FileName:   Log.cc
* CreateDate: 2022-03-03 23:08:32
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "Logger.h"
#include <memory>
using namespace SinBack;

Log::Logger::Logger(Log::LoggerType type, const Log::Logger::string_type &file_name, Size_t thread_num)
        : datetime_()
        , th_num_(thread_num)
        , level_(Debug)
        , stop_(false)
        , time_str_{0}
        , sec_(0)
        , type_(type)
{
    this->front_buf_.reset(new queue_type);
    this->back_buf_.reset(new queue_type);
    Base::getdatetimenow(&this->datetime_);
    format_time();
    this->sec_ = datetime_.sec;

    string_type name = file_name;
    if (name.empty()){
        name = SIN_STR("SinBack");
    }
    if (type_ == Normal){
        this->log_.reset(new Base::LogFile(name.c_str()));
    }else if (type_ == Rolling){
        this->log_.reset(new Base::RollLogFile(name.c_str()));
    }
    Size_t i = 0;
    for (; i < this->th_num_; ++i){
        this->ths_.emplace_back(&Logger::thread_run_func, this);
    }
}

void Log::Logger::thread_run_func()
{
    string_type buf;
    while (true){
        {
            {
                std::unique_lock<std::mutex> lock(this->back_mutex_);
                // 如果后端缓冲区为空，则休眠超时 3s
                if (this->back_buf_->empty()) {
                    this->cv_.wait_for(lock, std::chrono::seconds(1), [this] {
                        return (this->stop_) || (!this->back_buf_->empty());
                    });
                }
                // 如果为超时唤醒，则交换缓冲区
                if (!this->stop_ && this->back_buf_->empty()) {
                    std::unique_lock<std::mutex> lock1(this->front_mutex_);
                    this->back_buf_.swap(this->front_buf_);
                    fmt::print("swap. . .\n");
                }
                // 全部数据都处理完成，则退出
                if (this->stop_ && this->back_buf_->empty() && this->front_buf_->empty()) {
                    break;
                }
                // 写入文件
                if (!this->back_buf_->empty()) {
                    buf = std::move(this->back_buf_->front());
                    this->back_buf_->pop();
                    if (this->type_ == Normal) {
                        this->log_->write(buf);
                    } else if (this->type_ == Rolling) {
                        std::dynamic_pointer_cast<Base::RollLogFile>(this->log_)->write(buf);
                    }
                }
            }
            // 写入前端缓冲区内容
            {
                std::unique_lock<std::mutex> lock(this->front_mutex_);
                if (this->stop_ && !this->front_buf_->empty()) {
                    buf = std::move(this->front_buf_->front());
                    this->front_buf_->pop();
                    if (this->type_ == Normal) {
                        this->log_->write(buf);
                    } else if (this->type_ == Rolling) {
                        std::dynamic_pointer_cast<Base::RollLogFile>(this->log_)->write(buf);
                    }
                }
            }
        }
    }
}

void Log::Logger::format_time()
{
    snprintf(time_str_, 20, "%04d-%02d-%02d %02d:%02d:%02d",
             datetime_.year, datetime_.month, datetime_.day, datetime_.hour, datetime_.min, datetime_.sec);
}

void Log::Logger::format_time_sec()
{
    snprintf(time_str_ + 17, 3, "%02d", this->sec_);
}

Log::Logger::~Logger(){
    this->stop_ = true;
    for (auto& it : this->ths_){
        if (it.joinable()){
            it.join();
        }
    }
}