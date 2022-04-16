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
#include <thread>
#include "Base.h"

namespace SinBack
{
    namespace Base
    {
        // 时间类型
        enum TimeType{
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
        void getTimeOfDay(timeval* tv);
        // 获取距离1900过去多少毫秒
        SSize_t getTimeOfDayMs();
        // 获取距离1900过去多少微妙
        SSize_t getTimeOfDayUs();
        // 获取当前日期时间
        bool getDateTimeNow(DateTime* dt);
        // 获取当前时间并格式化为指定格式
        String getDateTimeNow(const String& format = "%Y-%m-%d %H:%M:%S");

        // 获取时间点后多少时间对应 time_t
        template <TimeType Type> static Long getTimeAfter(Long time, Long after){
            return 0;
        }
        template <> Long getTimeAfter<MSecond>(Long time, Long after){
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(time) + std::chrono::microseconds(after));
        }
        template <> Long getTimeAfter<Second>(Long time, Long after){
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(time) + std::chrono::seconds(after));
        }
        template <> Long getTimeAfter<Minute>(Long time, Long after){
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(time) + std::chrono::minutes(after));
        }
        template <> Long getTimeAfter<Hour>(Long time, Long after){
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(time) + std::chrono::hours(after));
        }
        template <> Long getTimeAfter<Day>(Long time, Long after){
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(time) + std::chrono::hours(24 * after));
        }
        template <> Long getTimeAfter<Week>(Long time, Long after){
            return std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(time) + std::chrono::hours( 24 * 7 * after));
        }

        // time_t转换为 DateTime
        DateTime toDateTime(Long time);
        // DateTime转换为 time_t
        Long dateTimeTo(DateTime date);
        // 线程休眠
        inline void sleepMs(Int time){
            std::this_thread::sleep_for(std::chrono::milliseconds(time));
        }
        inline void sleepUs(Int time){
            std::this_thread::sleep_for(std::chrono::microseconds(time));
        }

    }
}

#endif //SINBACK_TIMEUTIL_H
