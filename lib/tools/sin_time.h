/**
* @Author : Ticks
* @File   : SinBack.h
* @Date   : 2022-02-21 22:41
* @Email  : 2938384958@qq.com
* Des     :   
**/
#ifndef SIN_BACK_SIN_TIME_H
#define SIN_BACK_SIN_TIME_H

#include <sys/time.h>
#include "fmt/format.h"
#include "fmt/chrono.h"

namespace SinBack
{
    namespace TimeTool {
        static unsigned long long gettimeofday_ms() {
            struct timeval tv{};
            gettimeofday(&tv, nullptr);
            return tv.tv_sec * static_cast<unsigned long long>(1000) + tv.tv_usec / 1000;
        }

        static unsigned long long gettimeofday_us() {
            struct timeval tv{};
            gettimeofday(&tv, nullptr);
            return tv.tv_sec * static_cast<unsigned long long>(1000000) + tv.tv_usec;
        }

        // 获取当前时间
        static fmt::string_view datetime_now(){
            auto time = std::chrono::system_clock::now();

            return fmt::format("{:%F}", std::chrono::time_point_cast<std::chrono::seconds>(time));
        }
    }
}

#endif //SIN_BACK_SIN_TIME_H
