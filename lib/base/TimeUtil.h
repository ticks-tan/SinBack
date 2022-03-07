/**
* FileName:   TimeApi.h
* CreateDate: 2022-03-03 12:40:49
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        封装与时间相关函数
*/
#ifndef SINBACK_TIMEUTIL_H
#define SINBACK_TIMEUTIL_H

#include <chrono>
#include <string>
#include "Base.h"

#ifdef OS_WINDOWS
struct timeval{
    SinBack::LLong tv_sec;
    SinBack::LLong tv_usec;
};
#endif

namespace SinBack
{
    namespace Base
    {
        // 时间类型
        enum TimeType{
            Month   = 0x01,
            Day     = 0x02,
            Week    = 0x03,
            Hour    = 0x04,
            Minute  = 0x05,
            Second  = 0x06,
            MSecond = 0x07
        };
        // 存储时间日期
        struct DateTime{
            Int year;
            Int month;
            Int day;
            Int weekday;
            Int hour;
            Int min;
            Int sec;
            Int ms;
        };
        // 获取时间
        void gettimeofday(timeval* tv){
            auto time_now = std::chrono::system_clock::now();
            auto tv_s = std::chrono::duration_cast<std::chrono::seconds>(time_now.time_since_epoch()).count();
            auto tv_us = std::chrono::duration_cast<std::chrono::microseconds>(time_now.time_since_epoch()).count();
            tv->tv_sec = tv_s;
            tv->tv_usec = tv_us;
        }
        // 获取距离1900过去多少毫秒
        SSize_t gettimeofday_ms(){
            timeval time{};
            gettimeofday(&time);
            return (time.tv_sec * 1000) + (time.tv_usec / 1000);
        }
        // 获取距离1900过去多少微妙
        SSize_t gettimeofday_us(){
            timeval time{};
            gettimeofday(&time);
            return (time.tv_sec * 1000000) + time.tv_usec;
        }
        // 获取当前日期时间
        bool getdatetimenow(DateTime* dt){
            auto now_time = std::chrono::system_clock::now();
            time_t time = std::chrono::system_clock::to_time_t(now_time);
            tm* t = gmtime(&time);
            if (t){
                dt->year = t->tm_year + 1900;
                dt->month = t->tm_mon + 1;
                dt->day = t->tm_mday;
                dt->weekday = t->tm_wday;
                dt->hour = t->tm_hour;
                dt->min = t->tm_min;
                dt->sec = t->tm_sec;
                return true;
            }
            return false;
        }
        // 获取当前时间并格式化为指定格式
        std::string getdatetimenow(const std::string& format = "%Y-%m-%d %H:%M:%S"){
            auto now_time = std::chrono::system_clock::now();
            time_t time = std::chrono::system_clock::to_time_t(now_time);
            tm* t = gmtime(&time);
            char str[21] = {0};
            if (t) {
                strftime(str, 20, format.c_str(), t);
            }
            return str;
        }

        // 获取时间点后多少时间对应 time_t
        template <TimeType Type> Long gettimeafter(Long time, Long after){
        }
        template <> Long gettimeafter<MSecond>(Long time, Long after){
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(time) + std::chrono::microseconds(after));
        }
        template <> Long gettimeafter<Second>(Long time, Long after){
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(time) + std::chrono::seconds(after));
        }
        template <> Long gettimeafter<Minute>(Long time, Long after){
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(time) + std::chrono::minutes(after));
        }
        template <> Long gettimeafter<Hour>(Long time, Long after){
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(time) + std::chrono::hours(after));
        }
        template <> Long gettimeafter<Day>(Long time, Long after){
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(time) + std::chrono::hours(24 * after));
        }
        template <> Long gettimeafter<Week>(Long time, Long after){
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(time) + std::chrono::hours( 24 * 7 * after));
        }

        // time_t转换为 DateTime
        DateTime todatetime(Long time){
            DateTime dt{};
            tm* t = gmtime(&time);
            if (t){
                dt.year = t->tm_year + 1900;
                dt.month = t->tm_mon + 1;
                dt.day = t->tm_mday;
                dt.weekday = t->tm_wday;
                dt.hour = t->tm_hour;
                dt.min = t->tm_min;
                dt.sec = t->tm_sec;
            }
            return dt;
        }
        // DateTime转换为 time_t
        Long datetimeto(DateTime date){
            tm time{};
            time.tm_year = date.year - 1900;
            time.tm_mon = date.month - 1;
            time.tm_mday = date.day;
            time.tm_hour = date.hour;
            time.tm_min = date.min;
            time.tm_sec = date.sec;
            return mktime(&time);
        }

    }
}

#endif //SINBACK_TIMEUTIL_H
