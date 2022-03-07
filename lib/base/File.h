/**
* FileName:   File.h
* CreateDate: 2022-03-03 22:05:39
* Author:     ticks
* Email:      2938384958@qq.com
* Des:
*/
#ifndef SINBACK_FILE_H
#define SINBACK_FILE_H

#include <cstdio>
#include "Base.h"
#include "noncopyable.h"

namespace SinBack
{
    namespace Base
    {
        // 文件类
        class File : SinBack::noncopyable
        {
        public:
            enum FileMode
            {
                RDONLY = 00,
                WRONLY = 01,
                RDWR   = 02,
                APPEND = 02000,
                EXIST  = 0200
            };
        public:
            explicit File(const char* filename, int mode = RDWR);
            ~File();
            SSize_t size();
            SSize_t read(void* buf, Size_t len);
            SSize_t write(const void* buf, Size_t len, bool flush);
            void flush();
            SSize_t seek(SSize_t offset, int mode);
            void clear();
        private:
            FILE* fp_;
            int mode_;
            char file_name_[FILENAME_MAX];
        };
    }
}

#endif //SINBACK_FILE_H
