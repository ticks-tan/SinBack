/**
* FileName:   Log.h
* CreateDate: 2022-03-03 21:13:57
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_LOG_H
#define SINBACK_LOG_H

#include <queue>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "libfmt/format.h"
#include "base/TimeUtil.h"
#include "base/File.h"

namespace SinBack
{
    namespace Log
    {
        // 日志级别
        enum LogLevel{
            Info = 0,
            Warn = 1,
            Error = 2
        };

        // 日志类
        class Logger
        {
        public:
            Logger(const char* filename, LogLevel level = Info,Size_t max_log_num = 30);
            ~Logger();
            // 写入 info 级别日志
            template<typename ... Args>
            void info(fmt::format_string<Args...> format, const Args&... args){
                if (this->level_ > Info || this->stop_){
                    return;
                }
                std::string buf = fmt::format("[Info ");
                this->format_time(buf);
                buf += fmt::format(format, args...);
                buf.push_back(STR_CTL);
                this->push_log(buf);
            }
            // 写入 warn 级别日志
            template<typename ... Args>
            void warn(fmt::format_string<Args...> format, const Args&... args){
                if (this->level_ > Warn || this->stop_){
                    return;
                }
                std::string buf = fmt::format("[Warn ");
                this->format_time(buf);
                buf += fmt::format(format, args...);
                buf.push_back(STR_CTL);
                this->push_log(buf);
            }
            // 写入 error 级别日志
            template<typename ... Args>
            void error(fmt::format_string<Args...> format, const Args&...args){
                if (this->level_ > Error || this->stop_){
                    return;
                }
                std::string buf = fmt::format("[Error ");
                this->format_time(buf);
                buf += fmt::format(format, args...);
                buf.push_back(STR_CTL);
                this->push_log(buf);
            }
            void stop();
        private:
            void RunInBack();
            // 交换前后端buffer
            void swap_buffer();
            void push_log(const std::string& buf){
                std::unique_lock<std::mutex> lock(this->front_mutex_);
                this->front_->push(buf);
                if (this->front_->size() > this->max_log_num_){
                    this->swap_buffer();
                }
            }
            void format_time(std::string& buf){
                Long now = clock();
                if (now - last_time_ > CLOCKS_PER_SEC * 1){
                    Base::getdatetimenow(&this->time_);
                }
                buf += fmt::format("{}-{}-{} {}:{}:{} ]: ",
                                   this->time_.year, this->time_.month, this->time_.day,
                                   this->time_.hour, this->time_.min, this->time_.sec);
            }
        private:
            // 运行标志
            bool stop_ = true;
            // 前端队列最大元素个数
            Size_t max_log_num_;
            // 日志记录器记录级别
            LogLevel level_ = Info;
            // 前端数据队列
            std::unique_ptr<std::queue<std::string>> front_;
            // 后端数据队列
            std::unique_ptr<std::queue<std::string>> back_;
            // 后端写入锁
            std::mutex back_mutex_;
            // 前端写入锁
            std::mutex front_mutex_;
            // 条件变量，用于唤醒后端线程
            std::condition_variable cv_;
            // 后端写入线程
            std::unique_ptr<std::thread> th_;
            // 日志文件
            std::unique_ptr<Base::File> file_;
            // 上次时间
            Long last_time_;
            // 时间
            Base::DateTime time_;
        };

    }
}

#endif //SINBACK_LOG_H
