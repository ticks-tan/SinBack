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
#include <functional>
#include <memory>
#include <string>
#include <cstring>
#include "base/noncopyable.h"

// Linux平台
#ifdef __linux__
#define OS_LINUX
#endif

#define SINBACK_OPENSSL

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
    typedef unsigned char Byte;
    typedef float Float;
    typedef double Double;

    // shared_ptr
    template <typename T> using SharedPtr = std::shared_ptr<T>;
    // unique_ptr
    template <typename T> using UniquePtr = std::unique_ptr<T>;
    // function
    template <typename T> using Function = std::function<T>;
    // string
    typedef std::basic_string<Char> String;

    // 字符串结尾
#define CHAR_END    ('\0')
    // 字符换行
#define STR_CTL     ('\n')

    template <typename... Types> struct TypeSize;
    template <typename Only>
    struct TypeSize<Only>
    {
        Size_t size = 0;
        explicit TypeSize(Only&& t)
        {
            size = sizeof(t);
        }
    };
    template <typename First, typename... Other>
    struct TypeSize<First, Other...>
    {
        Size_t size = 0;
        explicit TypeSize(First&& first, Other&&... other){
            size = TypeSize<First&&>(first).size + TypeSize<Other&&...>(other...).size;
        }
    };
    template<> struct TypeSize<>
    {
        Size_t size = 0;
        explicit TypeSize(){
            size = 0;
        }
    };

    // 格式化字符串
    template<typename... T> static String format(const String& fmt, T&&... args)
    {
        TypeSize<T&&...> type_size(args...);
        Size_t str_len = fmt.size() + type_size.size;
        UniquePtr<Char[]> str(new Char[str_len + 1]);
        std::snprintf(str.get(), str_len, fmt.c_str(), args...);
        return str.get();
    }

    // 字符串是否以指定字符串开头
    static bool startsWith(const String &str, const String& prefix)
    {
        if (str.size() < prefix.size())
            return false;
        return str.compare(0, prefix.size(), prefix) == 0;
    }

    static bool startsWith(const Char *str, const Char *prefix)
    {
        if (str == nullptr || prefix == nullptr)
            return false;
        Size_t str_len = std::strlen(str);
        Size_t prefix_len = std::strlen(prefix);
        if (str_len < prefix_len)
            return false;
        return std::strncmp(str, prefix, prefix_len) == 0;
    }
    // 字符串是否以指定字符串结尾
    static bool endsWith(const String &str, const String& suffix)
    {
        if (str.size() < suffix.size())
            return false;
        return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
    static bool endsWith(const Char *str, const Char *suffix)
    {
        if (str == nullptr || suffix == nullptr)
            return false;
        Size_t str_len = std::strlen(str);
        Size_t suffix_len = std::strlen(suffix);
        if (str_len < suffix_len)
            return false;
        return std::strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
    }

}

#endif //SIN_BACK_BASE_H
