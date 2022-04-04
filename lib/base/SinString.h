/**
* FileName:   SinString.h
* CreateDate: 2022-04-03 00:16:40
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        自定义简单字符串类
*/
#ifndef SINBACK_TEST_SINSTRING_H
#define SINBACK_TEST_SINSTRING_H

#include <cstring>
#include "base/Base.h"

namespace SinBack
{
    namespace Base
    {

        class SinString final
        {
        public:
            static const SSize_t npos = -1;
            class Iterator
            {
            public:
                typedef Size_t difference_type;
                typedef Char value_type;
                typedef Char* pointer;
                typedef Char& reference;
                typedef std::random_access_iterator_tag iterator_category;
                Iterator() = default;
                explicit Iterator(Char* ptr) : ptr_(ptr) {}
                ~Iterator() = default;
                Iterator& operator++() { ++ptr_; return *this; }
                Iterator& operator--() { --ptr_; return *this; }
                Iterator& operator+=(SSize_t n) { ptr_ += n; return *this; }
                Iterator& operator-=(SSize_t n) { ptr_ -= n; return *this; }
                Iterator operator+(SSize_t n) { return Iterator(ptr_ + n); }
                Iterator operator-(SSize_t n) { return Iterator(ptr_ - n); }
                Size_t operator-(Iterator other) { return ptr_ - other.ptr_; }
                Char& operator*() { return *ptr_; }
                Char operator*() const { return *ptr_; }
                bool operator==(const Iterator& other) { return ptr_ == other.ptr_; }
                bool operator!=(const Iterator& other) { return ptr_ != other.ptr_; }
                bool operator<(const Iterator& other) { return ptr_ < other.ptr_; }
                bool operator>(const Iterator& other) { return ptr_ > other.ptr_; }
                bool operator<=(const Iterator& other) { return ptr_ <= other.ptr_; }
                bool operator>=(const Iterator& other) { return ptr_ >= other.ptr_; }
            private:
                Char* ptr_;
            };

            class ConstIterator
            {
            public:
                typedef Size_t difference_type;
                typedef Char value_type;
                typedef Char* pointer;
                typedef Char& reference;
                typedef std::random_access_iterator_tag iterator_category;
                ConstIterator() = default;
                explicit ConstIterator(Char* ptr) : ptr_(ptr) {}
                ~ConstIterator() = default;
                ConstIterator& operator++() { ++ptr_; return *this; }
                ConstIterator& operator--() { --ptr_; return *this; }
                ConstIterator& operator+=(SSize_t n) { ptr_ += n; return *this; }
                ConstIterator& operator-=(SSize_t n) { ptr_ -= n; return *this; }
                ConstIterator operator+(SSize_t n) { return ConstIterator(ptr_ + n); }
                ConstIterator operator-(SSize_t n) { return ConstIterator(ptr_ - n); }
                Size_t operator-(ConstIterator other) { return ptr_ - other.ptr_; }
                const Char& operator*() const { return *ptr_; }
                bool operator==(const ConstIterator& other) { return ptr_ == other.ptr_; }
                bool operator!=(const ConstIterator& other) { return ptr_ != other.ptr_; }
                bool operator<(const ConstIterator& other) { return ptr_ < other.ptr_; }
                bool operator>(const ConstIterator& other) { return ptr_ > other.ptr_; }
                bool operator<=(const ConstIterator& other) { return ptr_ <= other.ptr_; }
                bool operator>=(const ConstIterator& other) { return ptr_ >= other.ptr_; }
            private:
                Char* ptr_;
            };

            // 构造函数
            explicit SinString();
            SinString(const Char *str);
            SinString(const Char* str, Size_t len);
            SinString(Size_t len, Char ch);
            explicit SinString(Size_t len);
            SinString(const SinString &str);
            SinString(SinString &&str) noexcept ;
            // 析构函数
            ~SinString();

            // 赋值运算符
            SinString &operator=(const SinString &str);
            SinString &operator=(SinString &&str) noexcept;
            SinString &operator=(const Char *str);

            // 添加字符串
            SinString &operator+=(const SinString &str);
            SinString &operator+=(const Char *str);
            SinString &operator+=(Char ch);

            // 访问字符
            char &operator[](int index);
            const char &operator[](int index) const;

            // 流式输入
            SinString& operator<<(const SinString &str);
            SinString& operator<<(const Char *str);
            SinString& operator<<(Char ch);

            // 比较运算符
            bool operator<(const SinString &str) const;
            bool operator>(const SinString &str) const;

            // 比较字符串
            bool operator==(const SinString &str) const;
            bool operator==(const Char *str) const;

            // 字符串是否以指定字符串开头
            bool startsWith(const SinString &str) const;
            bool startsWith(const Char *str) const;
            // 字符串是否以指定字符串结尾
            bool endsWith(const SinString &str) const;
            bool endsWith(const Char *str) const;
            // 字符串是否包含指定字符串
            bool contains(const SinString &str) const;
            bool contains(const Char *str) const;

            // 获取字符串长度
            Size_t length() const;
            // 字符串是否为空
            bool empty() const;
            // 清空字符串
            void clear();
            // 获取字符串首部指针
            Char* data();
            // 获取 const 类型 字符串指针
            const Char* const_data() const;
            // 分配最大字符串空间
            Size_t reserve(Size_t new_capacity);
            // 重新设置字符串长度
            Size_t resize(Size_t new_size, Char ch = CHAR_END);
            Size_t size() const{
                return this->length_;
            }
            Size_t capacity() const{
                return this->capacity_;
            }
            void push_back(Char ch);
            void pop_back();

            // 返回最开始迭代器
            Iterator begin();
            ConstIterator cbegin() const;
            // 返回最后迭代器
            Iterator end();
            ConstIterator cend() const;

            // 添加字符串到尾部
            SinString &append(const SinString &str);
            SinString &append(const Char *str, Size_t len);
            SinString &append(Char ch);

            // 插入字符串
            SinString &insert(Size_t index, const SinString &str);
            SinString& insert(Size_t index, const Char *str);
            SinString &insert(Size_t index, const Char *str, Size_t len);
            SinString &insert(Size_t index, Char ch);
            // 移除字符串
            SinString &erase(Size_t index, Size_t count = 1);
            // 替换字符串
            SinString &replace(int index, int count, const SinString &str);
            SinString &replace(int index, int count, const Char *str, Size_t len);
            SinString &replace(int index, int count, Char ch);

            // 获取字符串子串
            SinString substr(Size_t index, SSize_t count = -1) const;

            // 查找字符串
            Size_t find(const SinString &str, Size_t index = 0) const;
            Size_t find(const Char *str, Size_t index = 0) const;
            Size_t find(Char ch, Size_t index = 0) const;
            // 查找最后一个字符串
            Size_t rfind(const SinString &str, Size_t index = 0) const;
            Size_t rfind(const Char *str, Size_t index = 0) const;
            Size_t rfind(Char ch, Size_t index = 0) const;

        private:
            // 存储字符串数据
            UniquePtr<Char> data_;
            // 存储字符串长度
            Size_t length_;
            // 存储最大字符串容量
            Size_t capacity_;
        };
    }
}

#endif //SINBACK_TEST_SINSTRING_H
