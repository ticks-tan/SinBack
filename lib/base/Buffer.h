/**
* FileName:   Buffer.h
* CreateDate: 2022-03-03 10:31:11
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        缓冲区封装
*/
#ifndef SIN_BACK_BUFFER_H
#define SIN_BACK_BUFFER_H

#include <vector>
#include "base/SinString.h"

namespace SinBack
{
    namespace Base{
        // Buffer类设计
        class Buffer{
        public:
            using string_type = String;
            explicit Buffer();
            explicit Buffer(const string_type & buf);
            explicit Buffer(const Char* buf);
            explicit Buffer(const std::vector<Char>& buf);
            Buffer(const Buffer& buf);
            ~Buffer();

            // 返回字符串可读取长度
            Size_t size() const{
                return (this->end_ - this->begin_);
            }
            // 返回最大可存储字符数量
            Size_t capacity() const{
                return this->capacity_;
            }
            // 重新分配大小
            void reserve(Size_t len);
            // 清空缓存
            void clear();
            // 字符串开始内存地址
            char* data(){
                return this->data_.get() + this->begin_;
            }
            // 是否为空
            bool empty() const{
                return (this->size() == 0);
            }
            void scan(){
                if (this->begin_ > 0){
                    Size_t begin = this->begin_;
                    std::move(this->data_.get() + (Long)this->begin_,
                              this->data_.get() + (Long)this->end_,
                              this->data_.get());
                    this->begin_ = 0;
                    this->end_ -= begin;
                }
            }
            // 从缓存中读取字符串
            SSize_t read(Char* buf, Size_t len);
            SSize_t read(string_type & buf, Size_t len);
            SSize_t read(Buffer& buf, Size_t len);
            // 向缓存中写入字符串
            SSize_t write(const Char* buf, Size_t len);
            SSize_t write(const string_type & buf, Size_t len);
            SSize_t write(const Buffer& buf, Size_t len);
            // 从缓存读取所有字符
            SSize_t readAll(string_type & buf);
            SSize_t readAll(Buffer& buf);
            // 向缓存中写入所有字符
            SSize_t writeAll(const string_type & buf);
            SSize_t writeAll(const Buffer& buf);
            // 向缓存中写入字符串
            SSize_t append(const string_type & buf);
            SSize_t append(const Buffer& buf);
            // 移除缓存中的字符串
            bool removeFront(Size_t len);
            bool removeBack(Size_t len);

            Buffer& operator = (const string_type & buf);
            Buffer& operator = (const Buffer& buf);
            friend Buffer operator + (const Buffer& buf1, const Buffer& buf2);
            Buffer& operator += (const string_type & buf);
            Buffer& operator += (const Buffer& buf);
            Buffer& operator << (const string_type & buf);
            Buffer& operator << (const Buffer& buf);
            Buffer& operator >> (string_type & buf);
            Buffer& operator >> (Buffer& buf);
            explicit operator string_type();

        private:
            // 存储实际内容
            SharedPtr<Char> data_;
            // 开始下标
            Size_t begin_;
            // 结束下标
            Size_t end_;
            // 分配内存大小
            Size_t capacity_;
        };
    }
}

#endif //SIN_BACK_BUFFER_H
