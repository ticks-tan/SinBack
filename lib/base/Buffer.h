/**
* FileName:   Buffer.h
* CreateDate: 2022-03-03 10:31:11
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SIN_BACK_BUFFER_H
#define SIN_BACK_BUFFER_H

#include <vector>
#include "Base.h"

namespace SinBack
{
    namespace Base{
        // Buffer类设计
        class Buffer{
        public:
            explicit Buffer();
            explicit Buffer(const String& buf);
            explicit Buffer(const Char* buf);
            explicit Buffer(const std::vector<Char>& buf);
            Buffer(const Buffer& buf);
            ~Buffer();

            // 返回字符串可读取长度
            inline Size_t size() const;
            // 返回最大可存储字符数量
            inline Size_t capacity() const;
            // 重新分配大小
            inline void reserve(Size_t len);
            // 清空缓存
            void clear();
            // 字符串开始内存地址
            char* data(){
                return this->data_.data() + this->begin_;
            }
            // 是否为空
            bool empty() const
            {
                return (this->size() == 0);
            }
            void scan(){
                if (this->begin_ > 0){
                    Size_t begin = this->begin_;
                    std::move(this->data_.begin() + (Long)this->begin_,
                              this->data_.begin() + (Long)this->end_,
                              this->data_.begin());
                    this->begin_ = 0;
                    this->end_ -= begin;
                }
            }
            SSize_t read(void* buf, Size_t len);
            SSize_t read(std::string& buf, Size_t len);
            SSize_t read(Buffer& buf, Size_t len);
            SSize_t write(const void* buf, Size_t len);
            SSize_t write(const std::string& buf, Size_t len);
            SSize_t write(const Buffer& buf, Size_t len);
            SSize_t readAll(std::string& buf);
            SSize_t readAll(Buffer& buf);
            SSize_t writeAll(const std::string& buf);
            SSize_t writeAll(const Buffer& buf);

            SSize_t append(const std::string& buf);
            SSize_t append(const Buffer& buf);
            bool remove(Size_t len);

            Buffer& operator = (const std::string& buf);
            Buffer& operator = (const Buffer& buf);
            friend Buffer operator + (const Buffer& buf1, const Buffer& buf2);
            Buffer& operator += (const std::string& buf);
            Buffer& operator += (const Buffer& buf);
            Buffer& operator << (const std::string& buf);
            Buffer& operator << (const Buffer& buf);
            Buffer& operator >> (String& buf);
            Buffer& operator >> (Buffer& buf);
            explicit operator std::basic_string<Char>();

        private:
            // 存储实际内容
            std::vector<char> data_;
            // 开始下标
            Size_t begin_;
            // 结束下标
            Size_t end_;
        };
    }
}

#endif //SIN_BACK_BUFFER_H
