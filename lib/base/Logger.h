/**
* FileName:   Log.h
* CreateDate: 2022-03-03 21:13:57
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_LOGGER_H
#define SINBACK_LOGGER_H

#include <vector>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include "libfmt/format.h"
#include "base/TimeUtil.h"
#include "base/LogFile.h"

namespace SinBack
{
    namespace Log {
        // 日志级别
        enum LogLevel {
            Debug = 0,
            Info = 1,
            Warn = 2,
            Error = 3
        };

        enum LoggerType {
            Normal,
            Rolling
        };

        // 日志类
        class Logger : noncopyable {
        public:
            typedef std::basic_string<Char> string_type;
            typedef std::queue<string_type> queue_type;
        public:
            explicit Logger(LoggerType type, const string_type &file_name = "", Size_t thread_num = 1);

            ~Logger();
            void set_loglevel(LogLevel level){
                this->level_ = level;
            }

            template<typename... Type>
            Logger &debug(fmt::format_string<Type...> format, Type &&... args) {
                this->log(Debug, format, args...);
                return *this;
            }

            template<typename... Type>
            Logger &info(fmt::format_string<Type...> format, Type &&... args) {
                this->log(Info, format, args...);
                return *this;
            }

            template<typename... Type>
            Logger &warn(fmt::format_string<Type...> format, Type &&... args) {
                this->log(Warn, format, args...);
                return *this;
            }

            template<typename... Type>
            Logger &error(fmt::format_string<Type...> format, Type &&... args) {
                this->log(Error, format, args...);
                return *this;
            }

            string_type fileName() const {
                return log_->name();
            }

        private:
            template<typename... Type>
            void log(LogLevel msg_level, fmt::format_string<Type...> format, Type &&...args) {
                if (msg_level < this->level_) {
                    return;
                }
                // 前端日志时间尽量少，避免阻塞
                std::string msg = SIN_STR("[");
                if (msg_level == Debug) {
                    msg += SIN_STR("debug ");
                } else if (msg_level == Info) {
                    msg += SIN_STR("info ");
                } else if (msg_level == Warn) {
                    msg += SIN_STR("warn ");
                } else if (msg_level == Error) {
                    msg += SIN_STR("error ");
                }
                timeval tv{};
                Base::getTimeOfDay(&tv);
                {
                    std::unique_lock<std::mutex> lock(this->format_mutex_);
                    if (this->sec_  != tv.tv_sec % 60) {
                        this->sec_ = Int(tv.tv_sec) % 60;
                        if ((this->sec_ / 60) - this->min_ >= 1) {
                            Base::getDateTimeNow(&this->datetime_);
                            formatTime();
                            this->min_ = (this->sec_ / 60);
                        } else {
                            formatTimeSec();
                        }
                    }
                }
                msg += this->time_str_;
                msg += SIN_STR("]: ");
                msg += fmt::format(format, args...);
                msg.push_back(STR_CTL);
                std::unique_lock<std::mutex> lock(this->front_mutex_);
                this->front_buf_->push(std::move(msg));
            }
            // 线程执行函数
            void thread_run_func();
            // 解析时间
            void formatTime();
            // 解析时间秒部分
            void formatTimeSec();
        private:
            // 日志类型
            LoggerType type_;
            // 日志文件
            std::shared_ptr<Base::LogFile> log_;
            // 日志记录器级别
            LogLevel level_;
            // 线程数量
            Size_t th_num_;
            // 线程
            std::vector<std::thread> ths_;
            // 前端缓冲区
            std::unique_ptr<queue_type> front_buf_;
            // 后端缓冲区
            std::unique_ptr<queue_type> back_buf_;
            // 前端锁
            std::mutex front_mutex_;
            // 后端锁
            std::mutex back_mutex_;
            // 格式化时间锁
            std::mutex format_mutex_;
            // 条件变量
            std::condition_variable cv_;
            Base::DateTime datetime_;
            // 缓存时间字符串
            Char time_str_[20];
            Int sec_;
            Int min_;
            // 停止标志
            bool stop_;
        };

        static String& default_logger_path(){
            static String $_default_logger_path_$;
            return $_default_logger_path_$;
        }
        static void set_default_logger_path(const String& path){
            default_logger_path() = path;
        }

        static LogLevel& default_logger_level(){
            static LogLevel $_default_logger_level_$;
            return $_default_logger_level_$;
        }
        static void set_default_logger_level(LogLevel level){
            default_logger_level() = level;
        }

        static UInt& default_logger_thread_num(){
            static UInt $_default_logger_thread_num_$;
            return $_default_logger_thread_num_$;
        }
        static void set_default_logger_thread_num(UInt num){
            default_logger_thread_num() = num;
        }

        static Logger* default_logger(){
            // 默认日志记录器
            static Logger $_default_logger_$(LoggerType::Rolling, default_logger_path(), default_logger_thread_num());
            $_default_logger_$.set_loglevel(default_logger_level());
            return &($_default_logger_$);
        }

        template <typename... T>
        void file_log_print(Logger* logger, LogLevel level, fmt::format_string<T...> format, T&&...args){
            if (!logger){
                return;
            }
            switch (level) {
                case Debug:
                    logger->debug(format, args...);
                    break;
                case Info:
                    logger->info(format, args...);
                    break;
                case Warn:
                    logger->warn(format, args...);
                    break;
                case Error:
                    logger->error(format, args...);
                    break;
                default:
                    break;
            }
        }
        template <typename... T>
        void logd(fmt::format_string<T...> format, T&&...args){
            file_log_print(default_logger(), LogLevel::Debug, format, args...);
        }
        template <typename... T>
        void logi(fmt::format_string<T...> format, T&&...args){
            file_log_print(default_logger(), LogLevel::Info, format, args...);
        }
        template <typename... T>
        void logw(fmt::format_string<T...> format, T&&...args){
            file_log_print(default_logger(), LogLevel::Warn, format, args...);
        }
        template <typename... T>
        void loge(fmt::format_string<T...> format, T&&...args){
            file_log_print(default_logger(), LogLevel::Error, format, args...);
        }
    }
}

#endif //SINBACK_LOGGER_H
