/**
* FileName:   File.cc
* CreateDate: 2022-03-15 23:52:31
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include <vector>
#include "File.h"
#include "base/FileUtil.h"

using namespace SinBack;

Base::File::File()
    : name_()
    , mode_(ReadOnly)
    , file_(nullptr)
{
}

Base::File::File(const string_type &name, Base::OpenFileMode mode)
    : name_(name)
    , mode_(mode)
    , file_(nullptr)
{
    switch (mode) {
        case ReadOnly:
            this->file_ = fopen(name.data(), "r");
            break;
        case WriteOnly:
            this->file_ = fopen(name.data(), "w");
            break;
        case ReadWrite:
            this->file_ = fopen(name.data(), "w+");
            break;
        case Append:
            this->file_ = fopen(name.data(), "a+");
            break;
        default:
            this->file_ = fopen(name.data(), "w+");
            break;
    }
}

Base::File::~File()
{
    if (this->file_){
        this->close();
    }
    this->file_ = nullptr;
}

/**
 * @brief 重新打开文件
 *
 * @param name : 文件名
 * @param mode : 文件打开模式
 * @return true
 * @return false
 */
bool Base::File::reOpen(const string_type &name, Base::OpenFileMode mode)
{
    if (name.empty() || this->name_ == name && this->mode_ == mode) return true;
    if (Base::isDir(name.data())) return false;
    this->close();
    this->name_ = name;
    this->mode_ = mode;
    switch (mode) {
        case ReadOnly:
            this->file_ = fopen(name.data(), "r");
            break;
        case WriteOnly:
            this->file_ = fopen(name.data(), "w");
            break;
        case ReadWrite:
            this->file_ = fopen(name.data(), "w+");
            break;
        case Append:
            this->file_ = fopen(name.data(), "a+");
            break;
        default:
            this->file_ = fopen(name.data(), "w+");
            break;
    }
    return (this->file_ != nullptr);
}

/**
 * @brief 读取文件
 * @param buf : 读取缓冲区
 * @param len : 读取长度
 * @return
 */
Long Base::File::read(void *buf, Size_t len)
{
    if (this->file_ && len > 0){
        Size_t read_len = 0, tmp_len;
        Int err = 0;

        while (read_len < len){
            tmp_len = ::fread(buf, 1, sizeof(Char) * (len - read_len), this->file_);
            if (tmp_len > 0){
                read_len += tmp_len;
            }
            if (read_len < len){
                if (::ferror(this->file_)){
                    if (err > 3) break;
                    ::clearerr(this->file_);
                    ++err;
                    continue;
                }
                if (::feof(this->file_)){
                    break;
                }
            }
        }
        return (Long)read_len;
    }
    return -1;
}

/**
 * @brief 读取文件到字符串
 * @param len : 读取长度
 * @return : 读取的字符串
 */
Base::File::string_type Base::File::read(Size_t len)
{
    if (this->file_ && len > 0) {
        std::vector<Char> buf;
        buf.reserve(len);
        this->read(buf.data(), len);
        return buf.data();
    }
    return "";
}

/**
 * @brief 读取文件到字符串读取长度
 * @return : 读取的字符串
 */
Base::File::string_type Base::File::readAll()
{
    string_type buffer;
    if (this->file_ && !this->name_.empty()){
        std::vector<Char> buf;
        buf.reserve(128);
        Size_t need_read_len = 0;
        Long read_len = 0;
        while (!::feof(this->file_)){
            need_read_len = buf.capacity() - buf.size();
            read_len = this->read((void*)buf.data() , need_read_len);
            buffer.append(buf.data(), read_len);
            buf.clear();
        }
        return std::move(buffer);
    }
    return std::move(buffer);
}

/**
 * @brief 写入文件
 * @param buf : 写入缓冲区
 * @param len : 写入长度
 * @return : 写入的长度
 */
Long Base::File::write(const void *buf, Size_t len)
{
    if (this->file_ && len > 0){
        Size_t write_len = 0, tmp_len;
        Int err = 0;

        while (write_len < len){
            tmp_len = ::fwrite(buf, 1, sizeof(Char) * (len - write_len), this->file_);
            if (tmp_len > 0){
                write_len += tmp_len;
            }
            if (write_len < len){
                if (::ferror(this->file_)){
                    if (err > 3) break;
                    ::clearerr(this->file_);
                    ++err;
                }
                if (::feof(this->file_)){
                    break;
                }
            }
        }
        return (Long)write_len;
    }
    return -1;
}

/**
 * @brief 写入字符串到文件
 * @param buf : 写入字符串
 * @return : 写入的长度
 */
Long Base::File::write(const string_type &buf)
{
    return this->write(buf.data(), buf.length());
}

/**
 * @brief 流式写入字符串到文件
 * @param buf : 写入字符串
 * @return : File&
 */
Base::File &Base::File::operator<<(const string_type &buf)
{
    this->write(buf);
    return *this;
}

/**
 * @brief 流式读取文件到字符串
 * @param buf : 要读取到的字符串
 * @return : File&
 */
Base::File &Base::File::operator>>(string_type &buf)
{
    buf = std::move(this->readAll());
    return *this;
}

/**
 * @brief 关闭文件
 * @return : 是否关闭成功
 */
void Base::File::close()
{
    if (this->file_ != nullptr){
        ::fclose(this->file_);
    }
    this->file_ = nullptr;
    this->name_.clear();
}

/**
 * @brief 获取文件大小
 */
Size_t Base::File::size()
{
    if (this->file_) {
        Size_t size = 0;
        auto cur_seek = ftell(this->file_);
        fseek(this->file_, 0, SEEK_END);
        size = ftell(this->file_);
        fseek(this->file_, cur_seek, SEEK_SET);
        return size;
    }
    return 0;
}

/**
 * @brief 清空文件
 */
bool Base::File::clear()
{
    if (this->file_){
        return (::ftruncate(fileno(this->file_), 0) == 0);
    }
    return false;
}
