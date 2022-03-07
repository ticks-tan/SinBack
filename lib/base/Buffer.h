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
#include <string>
#include "Base.h"

namespace SinBack
{
    namespace Base{
        // Buffer类设计
        class Buffer{
        public:
            explicit Buffer();
            explicit Buffer(const std::string& buf);
            Buffer(const Buffer& buf);
            ~Buffer();

            inline Size_t size() const;
            inline Size_t capacity() const;
            void clear();
            char* data(){
                return this->data_.data() + this->begin_;
            }
            void scan(){
                if (this->begin_ > 0){
                    auto it = this->data_.begin();
                    this->data_.erase(it, it + (Int)this->begin_);
                    this->begin_ = 0;
                }
            }
            SSize_t out(void* buf, Size_t len);
            SSize_t out(std::string& buf, Size_t len);
            SSize_t out(Buffer& buf, Size_t len);
            SSize_t in(const void* buf, Size_t len);
            SSize_t in(const std::string& buf, Size_t len);
            SSize_t in(const Buffer& buf, Size_t len);
            SSize_t outall(std::string& buf);
            SSize_t outall(Buffer& buf);
            SSize_t inall(const std::string& buf);
            SSize_t inall(const Buffer& buf);

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
            explicit operator std::string();

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
