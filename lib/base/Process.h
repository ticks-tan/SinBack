/**
* FileName:   Process.h
* CreateDate: 2022-03-18 15:42:31
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        进程相关函数
*/
#ifndef SINBACK_PROCESS_H
#define SINBACK_PROCESS_H

#include <cstdlib>
#include <functional>

#ifdef OS_LINUX

namespace SinBack
{
    namespace Base
    {
        // 注册终止处理函数，注册一次就会被执行一次，即使函数一样
        bool register_func_exit(void (*func)()){
            return (std::atexit(func) == 0);
        }
    }
}

#endif

#endif //SINBACK_PROCESS_H
