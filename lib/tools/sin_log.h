/**
* FileName:   sin_log.h
* CreateDate: 2022-03-02 12:35:29
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SIN_BACK_SIN_LOG_H
#define SIN_BACK_SIN_LOG_H

#include <mutex>
#include "fmt/os.h"


namespace SinBack {

    namespace Log {
        // 日志类型
        enum LoggerType {
            ConsoleLog = 0x00,
            FileLog = 0x01
        };

        // 日志类型
        enum LoggerLevel {
            Info = 0x01,
            Warn = 0x02,
            Error = 0x03,
        };

        template<LoggerType Type>
        class Logger {
        public:
        };

        template<>
        class Logger<ConsoleLog> {
        public:
            explicit Logger(const fmt::string_view& name = "", LoggerLevel level = LoggerLevel::Info){
                name_ = name;
                level_ = level;
            }
            ~Logger() = default;
            void info(bool is_lock, const fmt::string_view& format, ...){
                if (this->level_ > LoggerLevel::Info){
                    return;
                }
                if (is_lock){
                    std::unique_lock<std::mutex> ul(this->mutex_);
                }
                fmt::string_view f = "[Info ]";
            }
            void info(const fmt::string_view& format, ...){

            }
            void warn(bool is_lock, const fmt::string_view& format, ...){

            }
            void warn(const fmt::string_view& format, ...){

            }
            void error(bool is_lock, const fmt::string_view& format, ...){

            }
            void error(const fmt::string_view& format, ...){

            }
        private:
            // 日志级别
            LoggerLevel level_;
            // 日志记录器名称
            fmt::string_view name_;
            // 日志输出锁
            std::mutex mutex_;
        };
    }
}

#endif //SIN_BACK_SIN_LOG_H
