/**
* FileName:   File.h
* CreateDate: 2022-03-03 22:05:39
* Author:     ticks
* Email:      2938384958@qq.com
* Des:
*/
#ifndef SINBACK_LOGFILE_H
#define SINBACK_LOGFILE_H

#include <cstdio>
#include <string>
#include "Base.h"
#include "noncopyable.h"

namespace SinBack
{
    namespace Base
    {
        // 普通日志文件
        class LogFile : noncopyable
        {
            // 默认缓冲区大小
            static const Int LOGFILE_MAX_BUFFER_LEN = 512;
        public:
            using string_type = std::basic_string<Char>;
            LogFile();
            explicit LogFile(const Char* filename);
            virtual ~LogFile();

            string_type& name(){
                return this->filename_;
            }
            FILE*& file_fp(){
                return this->fp_;
            }
            bool& is_close(){
                return this->close_;
            }

            void flush();
            void close();
            virtual Size_t write(const string_type& buf);
        private:
            // 文件指针
            FILE* fp_;
            // 文件名
            string_type filename_;
            // 缓冲区，只有缓冲区满或者关闭时真正调用write写入
            string_type buffer_;
            // 是否关闭
            bool close_;
        };

        // 滚动日志文件
        class RollLogFile : public LogFile
        {
            // 默认滚动日志大小为 10M
            static const Size_t DEFAULT_ROLL_SIZE = 1024 * 1024 * 30;
        public:
            RollLogFile();
            explicit RollLogFile(const Char* filename, Size_t max_len = DEFAULT_ROLL_SIZE);
            ~RollLogFile() override;

            Size_t write(const string_type& buf) override;

        private:
            void roll_file();
            void refresh_file_size();
        private:
            // 基础文件名
            string_type base_name_;
            // 每一个文件最大大小(字节)
            Size_t max_len_;
            // 当前文件大小
            Size_t file_size_;
            // 滚动次数
            Size_t roll_count_;
        };
    }
}


#endif //SINBACK_LOGFILE_H
