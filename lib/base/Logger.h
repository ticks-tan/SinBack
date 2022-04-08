/**
* FileName:   Log.h
* CreateDate: 2022-03-03 21:13:57
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        单例模式日志记录器，支持多线程写入，字符串格式化
*/
#ifndef SINBACK_LOGGER_H
#define SINBACK_LOGGER_H

#include <mutex>
#include <thread>
#include <queue>
#include <unordered_map>
#include <condition_variable>
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
    class Logger : noncopyable{
        public:
            // 字符串
        using string_type = SinBack::String;
            // 队列
            typedef std::queue<string_type> queue_type;

            // 获取日志类
            static Logger* getLogger(const string_type& name);
            // 注册日志类
            static bool registerLogger(const string_type& name = "", const string_type& log_dir = "",
                                       LoggerType type = Normal, LogLevel level = Info, Size_t thread_num = 1);
            // 取消注册日志类
            static bool unregisterLogger(const string_type& name);
            // 取消注册所有日志类
            static void unregisterAllLogger();

        private:
            // 静态成员 LoggerName - Logger MAP
            static std::unordered_map<string_type, Logger*> logger_map;
            // 全局锁，防止多线程注册取消注册日志访问冲突
            static std::mutex logger_mutex;

            // 前端日志队列最大值，超过这个值会唤醒后端日志线程
            static const Size_t front_queue_max_size = 2000;

            explicit Logger(LoggerType type, const string_type &file_name = "", Size_t thread_num = 2);

            ~Logger();
            // 设置日志级别
            void setLogLevel(LogLevel level){
                this->level_ = level;
            }

        public:
            template<typename... Type>
            Logger &debug(const string_type &format, Type &&... args) {
                this->log(Debug, format, args...);
                return *this;
            }

            template<typename... Type>
            Logger &info(const string_type &format, Type &&... args) {
                this->log(Info, format, args...);
                return *this;
            }

            template<typename... Type>
            Logger &warn(const string_type &format, Type &&... args) {
                this->log(Warn, format, args...);
                return *this;
            }

            template<typename... Type>
            Logger &error(const string_type &format, Type &&... args) {
                this->log(Error, format, args...);
                return *this;
            }
            // 获取日志文件名
            string_type& fileName() const {
                return log_->name();
            }

        private:
            template<typename... Type>
            void log(LogLevel msg_level, const string_type&format, Type&&...args) {
                if (msg_level < this->level_) {
                    return;
                }
                // 前端日志时间尽量少，避免阻塞
                Base::Buffer msg;
                msg << "[";
                if (msg_level == Debug) {
                    msg << "debug ";
                } else if (msg_level == Info) {
                    msg << "info ";
                } else if (msg_level == Warn) {
                    msg << "warn ";
                } else if (msg_level == Error) {
                    msg << "error_ ";
                }
                timeval tv{};
                Base::getTimeOfDay(&tv);
                {
                    std::unique_lock<std::mutex> lock(this->format_mutex_);
                    if (this->sec_  != tv.tv_sec % 60) {
                        this->sec_ = Int((tv.tv_sec) % 60);
                        if ((tv.tv_sec / 60) % 60 != this->min_) {
                            Base::getDateTimeNow(&this->datetime_);
                            formatTime();
                            this->min_ = (Int)((tv.tv_sec / 60) % 60);
                        } else {
                            formatTimeSec();
                        }
                    }
                }
                // 格式化字符串
                msg << this->time_str_
                    << "]: "
                    << std::move(SinBack::format(format, std::forward<Type&&>(args)...))
                    << "\n";
                // 最后入队操作，需要加锁
                std::unique_lock<std::mutex> lock(this->front_mutex_);
                this->front_buf_->push(msg.data());
                // 前端队列长度很大了，通知后端线程快写日志
                if (this->front_buf_->size() >= front_queue_max_size) {
                    lock.unlock();
                    this->cv_.notify_one();
                }
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
            // 缓存的秒数
            Int sec_;
            // 缓存的分钟数
            Int min_;
            // 停止标志
            bool stop_;
        };

        template <typename... T>
        void file_log_print(Logger* logger, LogLevel level, const Logger::string_type &format, T&&...args){
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
        void FLogD(const Logger::string_type &format, T&&...args){
            auto logger = Logger::getLogger("SinBackDefault");
            if (logger) {
                file_log_print(logger, LogLevel::Debug, format, args...);
            }
        }
        template <typename... T>
        void FLogI(const Logger::string_type &format, T&&...args){
            auto logger = Logger::getLogger("SinBackDefault");
            if (logger) {
                file_log_print(logger, LogLevel::Info, format, args...);
            }
        }
        template <typename... T>
        void FLogW(const Logger::string_type &format, T&&...args){
            auto logger = Logger::getLogger("SinBackDefault");
            if (logger) {
                file_log_print(logger, LogLevel::Warn, format, args...);
            }
        }
        template <typename... T>
        void FLogE(const Logger::string_type &format, T&&...args){
            auto logger = Logger::getLogger("SinBackDefault");
            if (logger) {
                file_log_print(logger, LogLevel::Error, format, args...);
            }
        }
    }
}

#endif //SINBACK_LOGGER_H
