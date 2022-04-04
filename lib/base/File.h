/**
* FileName:   File.h
* CreateDate: 2022-03-15 23:52:31
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        简单文件类封装
*/
#ifndef SINBACK_FILE_H
#define SINBACK_FILE_H

#include <cstdio>
#include <unistd.h>
#include "base/Base.h"

namespace SinBack {
    namespace Base {

        // 打开文件模式
        enum OpenFileMode
        {
            ReadOnly = 1,
            WriteOnly = 2,
            ReadWrite = 3,
            Append = 4
        };

        // 通用文件类
        class File : public noncopyable
        {
        public:
            using string_type = String;
            File();
            File(const string_type & name, OpenFileMode mode);
            ~File();
            // 重新打开一个文件
            bool reOpen(const string_type & name, OpenFileMode mode);
            // 读取数据
            Long read(void* buf, Size_t len);
            string_type read(Size_t len);
            string_type readAll();
            // 写入数据
            Long write(const void* buf, Size_t len);
            Long write(const string_type & buf);

            File& operator << (const string_type & buf);
            File& operator >> (string_type & buf);

            // 获取文件大小
            Size_t size();
            // 清空文件
            bool clear();

            string_type& fileName(){
                return this->name_;
            }
            bool exist() const{
                return ( !this->name_.empty() && (::access(this->name_.c_str(), F_OK | R_OK) == 0));
            }

            /**
             * 获取文件扩展名
             * @return
             */
            string_type suffix(){
                auto it = this->name_.find_last_of('.');
                if (it != string_type::npos){
                    return this->name_.substr(it);
                }
                return "";
            }
        private:
            void close();

        private:
            // 文件指针
            FILE* file_;
            // 文件名
            string_type name_;
            // 文件打开模式
            OpenFileMode mode_;
        };
    }
}


#endif //SINBACK_FILE_H
