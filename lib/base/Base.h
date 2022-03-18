/**
* FileName:   Base.h
* CreateDate: 2022-03-03 10:25:17
* Author:     ticks
* Email:      2938384958@qq.com
* Des:
*/
#ifndef SIN_BACK_BASE_H
#define SIN_BACK_BASE_H

#include <algorithm>
#include <string>
#include "noncopyable.h"

// platform
// Windows平台
#ifdef _WIN32
#define OS_WINDOWS
#include <windows.h>
#endif

// Linux平台
#ifdef __linux__
#define OS_LINUX
#endif

namespace SinBack {
    // 类型定义
typedef unsigned long Size_t;
typedef long SSize_t;
typedef int Int;
typedef unsigned int UInt;
typedef long Long;
typedef long long LLong;
typedef unsigned long ULong;
typedef unsigned long long ULLong;
typedef char Char;
typedef unsigned char UChar;

    using String = std::basic_string<Char>;
    // 字符串结尾
#define CHAR_END    ('\0')
// 字符换行
#define STR_CTL     ('\n')

// 字符串
#ifdef OS_WINDOWS
#define SIN_STR(str)    (L##str)
#else
#define SIN_STR(str)    (str)
#endif

}

#endif //SIN_BACK_BASE_H
