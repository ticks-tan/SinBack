/**
* FileName:   File.cc
* CreateDate: 2022-03-03 22:24:31
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "LogFile.h"

using namespace SinBack;

Base::LogFile::LogFile()
    : fp_(nullptr)
    , close_(false)
{
}

Base::LogFile::LogFile(const Char *filename)
    : LogFile()
{
    this->filename_ = filename;
    this->filename_ += SIN_STR(".log");
    this->fp_ = ::fopen(this->filename_.c_str(), SIN_STR("w+"));
    this->buffer_.reserve(LOGFILE_MAX_BUFFER_LEN);
}

Base::LogFile::~LogFile()
{
    this->close();
}

void Base::LogFile::flush()
{
    if (this->fp_){
        ::fflush(this->fp_);
    }
}

void Base::LogFile::close()
{
    // 写入缓冲区内容
    if (!this->buffer_.empty() && this->fp_){
        this->close_ = true;
        this->write("");
        ::fflush(this->fp_);
        this->close_ = false;
    }
    if (this->fp_){
        ::fclose(this->fp_);
        this->fp_ = nullptr;
    }
}

Size_t Base::LogFile::write(const string_type& buf)
{
    if (!buf.empty()) {
        this->buffer_ += buf;
    }
    // 缓冲区没满，不进行真正的写入操作
    if (this->buffer_.length() < LOGFILE_MAX_BUFFER_LEN && !this->close_){
        return 0;
    }
    if (!this->fp_){
        return 0;
    }
    Size_t total_len = 0, buf_len = this->buffer_.length();
    Int err_count = 0;
    Size_t write_len;
    while (total_len < buf_len){
        write_len = ::fwrite(this->buffer_.data() + total_len, 1, sizeof(Char) * (buf_len - total_len), this->fp_);
        if (write_len > 0){
            total_len += write_len
;        }
        if (total_len < buf_len){
            if (::ferror(this->fp_)){
                if (err_count > 3){
                    break;
                }
                ::clearerr(this->fp_);
                ++err_count;
            }
        }
    }
    // 清除已经写入的内容
    if (total_len == buf_len) {
        this->buffer_.clear();
    } else{
        this->buffer_.erase(0, total_len);
    }
    return total_len;
}

Base::RollLogFile::RollLogFile()
    : LogFile()
    , max_len_(DEFAULT_ROLL_SIZE)
    , file_size_(0)
    , roll_count_(0)
{
}

Base::RollLogFile::RollLogFile(const Char *filename, Size_t max_len)
    : LogFile(filename)
    , max_len_(max_len)
    , file_size_(0)
    , roll_count_(0)
{
    this->base_name_ = this->name();
}

Base::RollLogFile::~RollLogFile() = default;

Size_t Base::RollLogFile::write(const string_type& buf)
{
    Size_t write_len = LogFile::write(buf);
    // 真正执行写入操作时更新文件大小，并判断是否需要滚动
    if (write_len > 0){
        refresh_file_size();
        if (this->file_size_ > this->max_len_){
            this->roll_file();
        }
    }
    return write_len;
}

void Base::RollLogFile::roll_file()
{
    ++this->roll_count_;
    auto it = this->base_name_.find_last_of('.');
    if (it != LogFile::string_type::npos){
        string_type name = this->base_name_;
        name.insert(it, std::to_string(this->roll_count_));
        this->name() = name;
        this->close();
        this->file_fp() = ::fopen(this->name().c_str(), SIN_STR("w+"));
    }
}

void Base::RollLogFile::refresh_file_size()
{
    FILE* fp = this->file_fp();
    Size_t cur_seek = ::ftell(fp);
    ::fseek(fp, 0, SEEK_END);
    this->file_size_ = ::ftell(fp);
    ::fseek(fp, (Long)cur_seek, SEEK_SET);
}