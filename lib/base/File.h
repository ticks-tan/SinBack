/**
* FileName:   File.h
* CreateDate: 2022-03-15 23:52:31
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_FILE_H
#define SINBACK_FILE_H

#include <cstdio>
#include <vector>
#include <unistd.h>
#include "base/noncopyable.h"
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
            File(const String& name, OpenFileMode mode);
            ~File();
            // 重新打开一个文件
            bool reopen(const String& name, OpenFileMode mode);
            // 读取数据
            Long read(void* buf, Size_t len);
            String read(Size_t len);
            String readAll();
            // 写入数据
            Long write(const void* buf, Size_t len);
            Long write(const String& buf);

            File& operator << (const String& buf);
            File& operator >> (String& buf);

            // 获取文件大小
            Size_t size();
            // 清空文件
            bool clear();

            String& fileName(){
                return this->name_;
            }
            bool exist() const{
                return (this->file_ != nullptr);
            }

            String suffix(){
                auto it = this->name_.find_last_of('.');
                if (it != String::npos){
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
            String name_;
            // 文件打开模式
            OpenFileMode mode_;
        };
    }
}


#endif //SINBACK_FILE_H
