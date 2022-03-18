/**
* FileName:   File.cc
* CreateDate: 2022-03-15 23:52:31
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "File.h"
#include "base/FileUtil.h"

using namespace SinBack;

Base::File::File()
    : name_()
    , mode_(ReadOnly)
    , file_(nullptr)
{
}

Base::File::File(const String &name, Base::OpenFileMode mode)
    : name_(name)
    , mode_(mode)
    , file_(nullptr)
{
    switch (mode) {
        case ReadOnly:
            this->file_ = fopen(name.c_str(), "r");
            break;
        case WriteOnly:
            this->file_ = fopen(name.c_str(), "w");
            break;
        case ReadWrite:
            this->file_ = fopen(name.c_str(), "w+");
            break;
        case Append:
            this->file_ = fopen(name.c_str(), "a+");
            break;
        default:
            this->file_ = fopen(name.c_str(), "w+");
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

bool Base::File::reOpen(const String &name, Base::OpenFileMode mode)
{
    if (name.empty() || this->name_ == name && this->mode_ == mode) return true;
    if (Base::isDir(name.c_str())) return false;
    this->close();
    this->name_ = name;
    this->mode_ = mode;
    switch (mode) {
        case ReadOnly:
            this->file_ = fopen(name.c_str(), "r");
            break;
        case WriteOnly:
            this->file_ = fopen(name.c_str(), "w");
            break;
        case ReadWrite:
            this->file_ = fopen(name.c_str(), "w+");
            break;
        case Append:
            this->file_ = fopen(name.c_str(), "a+");
            break;
        default:
            this->file_ = fopen(name.c_str(), "w+");
            break;
    }
    return (this->file_ != nullptr);
}

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

String Base::File::read(Size_t len)
{
    if (this->file_ && len > 0) {
        std::vector<Char> buf;
        buf.reserve(len);
        this->read(buf.data(), len);
        return buf.data();
    }
    return "";
}

String Base::File::readAll()
{
    String buffer;
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

Long Base::File::write(const String &buf)
{
    return this->write(buf.c_str(), buf.length());
}

Base::File &Base::File::operator<<(const String &buf)
{
    this->write(buf);
    return *this;
}

Base::File &Base::File::operator>>(String &buf)
{
    buf = std::move(this->readAll());
    return *this;
}

void Base::File::close()
{
    if (this->file_ != nullptr){
        ::fclose(this->file_);
    }
    this->file_ = nullptr;
    this->name_.clear();
}

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

bool Base::File::clear()
{
    if (this->file_){
        return (::ftruncate(fileno(this->file_), 0) == 0);
    }
    return false;
}
