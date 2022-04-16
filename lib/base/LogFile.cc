/**
* FileName:   File.cc
* CreateDate: 2022-03-03 22:24:31
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "LogFile.h"
#include "TimeUtil.h"

using namespace SinBack;

Base::LogFile::LogFile()
    : fp_(nullptr)
    , close_(false)
    , last_write_time_(0)
{
}

/**
 * 构造函数
 * @param filename: 文件名字(不包括扩展名)
 */
Base::LogFile::LogFile(const Char *filename)
    : LogFile()
{
    this->filename_ = filename;
    this->filename_ += ".log";
    this->fp_ = ::fopen(this->filename_.data(), "w+");
    this->buffer_.reserve(LOGFILE_MAX_BUFFER_LEN);
}

Base::LogFile::~LogFile()
{
    this->close();
}

// 刷新文件输出缓冲区
void Base::LogFile::flush()
{
    if (this->fp_){
        ::fflush(this->fp_);
    }
}

// 关闭文件
void Base::LogFile::close()
{
    // 文件即将关闭，写入缓冲区剩余内容
    if (!this->buffer_.empty() && this->fp_){
        this->close_ = true;
        this->write("");
        this->flush();
        this->close_ = false;
    }
    if (this->fp_){
        ::fclose(this->fp_);
        this->fp_ = nullptr;
    }
}

/**
 * 写入文件
 * @param buf: 待写入内容
 * @return
 */
Size_t Base::LogFile::write(const string_type& buf)
{
    if (!buf.empty()) {
        this->buffer_.append(buf);
    }
    Size_t now_time = Base::getTimeOfDayMs();
    // 缓冲区未满，不进行真正的写入操作
    // 如果 close 标志为 true, 说明文件即将关闭，强制写入缓冲区内容
    if (!this->fileFp()
    || (now_time - this->lastWriteTime() < LogFile::max_free_time &&
    this->buffer_.size() < LOGFILE_MAX_BUFFER_LEN && !this->isClose())){
        return 0;
    }
    // 更新最后写入时间
    this->last_write_time_ = now_time;
    // 缓冲区满，执行写入操作
    Size_t total_len = 0, buf_len = this->buffer_.size();
    Int err_count = 0;
    Size_t write_len;
    // 确保写入全部内容
    while (total_len < buf_len){
        write_len = ::fwrite(this->buffer_.data() + total_len, 1, sizeof(Char) * (buf_len - total_len), this->fp_);
        if (write_len > 0){
            total_len += write_len
;        }
        if (total_len < buf_len){
            // 判断是否为写入出错，出错重试次数最多为 3
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
        this->buffer_.removeFront(total_len);
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

/**
 * 写入内容
 * @param buf： 待写入内容
 * @return
 */
Size_t Base::RollLogFile::write(const string_type& buf)
{
    Size_t write_len = LogFile::write(buf);
    // 真正执行写入操作才更新文件大小，并判断是否需要滚动
    if (write_len > 0){
        refreshFileSize();
        if (this->file_size_ > this->max_len_){
            this->rollFile();
        }
    }
    return write_len;
}

// 滚动日志
void Base::RollLogFile::rollFile()
{
    ++this->roll_count_;
    auto it = this->base_name_.rfind('.');
    if (it != LogFile::string_type::npos){
        string_type name = this->base_name_;
        // 根据滚动次数重命名文件
        name.insert(it, std::to_string(this->roll_count_));
        this->name() = name;
        this->close();
        this->fileFp() = ::fopen(this->name().data(), "w+");
    }
}

// 刷新文件大小
void Base::RollLogFile::refreshFileSize()
{
    FILE* fp = this->fileFp();
    Size_t cur_seek = ::ftell(fp);
    ::fseek(fp, 0, SEEK_END);
    this->file_size_ = ::ftell(fp);
    ::fseek(fp, (Long)cur_seek, SEEK_SET);
}
