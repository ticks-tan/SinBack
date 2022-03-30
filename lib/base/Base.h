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
#include <functional>
#include <memory>
#include <vector>
#include "base/noncopyable.h"

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

    // string
    using String = std::basic_string<Char>;
    // shared_ptr
    template <typename T> using SharedPtr = std::shared_ptr<T>;
    // unique_ptr
    template <typename T> using UniquePtr = std::unique_ptr<T>;
    // function
    template <typename T> using Function = std::function<T>;

    // 字符串结尾
#define CHAR_END    ('\0')
    // 字符换行
#define STR_CTL     ('\n')

// str2 是否以 str1 开头
    static bool startWith(const String& str1, const String& str2)
    {
        if (str1.length() > str2.length()) return false;
        Size_t pos = 0, len1 = str1.length();
        for (; pos < len1; ++pos){
            if (str1[pos] != str2[pos]){
                return false;
            }
        }
        return true;
    }
    // str2 是否以 str1 结尾
    static bool endWith(const String& str1, const String& str2)
    {
        if (str1.length() > str2.length()) return false;
        Size_t pos = 0, len1 = str1.length(), len2 = str2.length();
        for (; pos < len1; ++pos){
            if (str1[len1 - pos - 1] != str2[len2 - pos - 1]){
                return false;
            }
        }
        return true;
    }

    // 获取多个类型大小
    template <typename... Types> struct TypeSize;
    template <typename Only>
    struct TypeSize<Only> :
            std::integral_constant<Size_t, sizeof(Only)>
    {};
    template <typename First, typename... Other>
    struct TypeSize<First, Other...> :
            std::integral_constant<Size_t, TypeSize<First>::value + TypeSize<Other...>::value>
    {};
    template<> struct TypeSize<> : std::integral_constant<Size_t, 0>
    {};

    // 格式化字符串
    template <typename... Args>
    String formatString(const String& format, Args&&... args)
    {
        std::vector<Char> str;
        // 根据可变参数大小分配内存
        str.reserve(format.size() + TypeSize<Args...>::value);
        ::sprintf(str.data(), format.c_str(), args...);
        return str.data();
    }

}

#endif //SIN_BACK_BASE_H
