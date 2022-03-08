/**
* FileName:   TimeUtil.cc
* CreateDate: 2022-03-07 23:03:54
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "base/TimeUtil.h"

// 获取时间
void SinBack::Base::gettimeofday(timeval* tv){
    auto time_now = std::chrono::system_clock::now();
    auto tv_s = std::chrono::duration_cast<std::chrono::seconds>(time_now.time_since_epoch()).count();
    auto tv_us = std::chrono::duration_cast<std::chrono::microseconds>(time_now.time_since_epoch()).count();
    tv->tv_sec = tv_s;
    tv->tv_usec = tv_us;
}
// 获取距离1900过去多少毫秒
SinBack::SSize_t SinBack::Base::gettimeofday_ms(){
    timeval time{};
    SinBack::Base::gettimeofday(&time);
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}
// 获取距离1900过去多少微妙
SinBack::SSize_t SinBack::Base::gettimeofday_us(){
    timeval time{};
    SinBack::Base::gettimeofday(&time);
    return (time.tv_sec * 1000000) + time.tv_usec;
}
// 获取当前日期时间
bool SinBack::Base::getdatetimenow(DateTime* dt){
    auto now_time = std::chrono::system_clock::now();
    time_t time = std::chrono::system_clock::to_time_t(now_time);
    tm t{};
    localtime_r(&time, &t);
    dt->year = t.tm_year + 1900;
    dt->month = t.tm_mon + 1;
    dt->day = t.tm_mday;
    dt->weekday = t.tm_wday;
    dt->hour = t.tm_hour;
    dt->min = t.tm_min;
    dt->sec = t.tm_sec;
    return true;
}
// 获取当前时间并格式化为指定格式
std::string SinBack::Base::getdatetimenow(const std::string& format){
    auto now_time = std::chrono::system_clock::now();
    time_t time = std::chrono::system_clock::to_time_t(now_time);
    tm* t = gmtime(&time);
    char str[21] = {0};
    if (t) {
        strftime(str, 20, format.c_str(), t);
    }
    return str;
}

// time_t转换为 DateTime
SinBack::Base::DateTime SinBack::Base::todatetime(Long time){
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
SinBack::Long SinBack::Base::datetimeto(DateTime date){
    tm time{};
    time.tm_year = date.year - 1900;
    time.tm_mon = date.month - 1;
    time.tm_mday = date.day;
    time.tm_hour = date.hour;
    time.tm_min = date.min;
    time.tm_sec = date.sec;
    return mktime(&time);
}