/**
* FileName:   Buffer.cc
* CreateDate: 2022-03-03 10:49:21
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "Buffer.h"

using namespace SinBack;

Base::Buffer::Buffer()
    : begin_(0), end_(0)
{
}

Base::Buffer::Buffer(const std::string &buf)
    :Buffer()
{
    this->data_.reserve(buf.capacity());
    Size_t i = 0, len = buf.size();
    for (; i < len; ++i){
        this->data_[this->end_++] = buf[i];
    }
    this->data_[this->end_] = CHAR_END;
}

Base::Buffer::Buffer(const Base::Buffer &buf)
    :Buffer()
{
    this->data_.reserve(buf.capacity());
    Size_t i = buf.begin_;
    for (; i < buf.end_; ++i){
        this->data_[this->end_++] = buf.data_[i];
    }
    this->data_[this->end_] = CHAR_END;
}

SinBack::Base::Buffer::~Buffer()
{
    this->begin_ = this->end_ = 0;
    this->data_.clear();
}

Size_t Base::Buffer::size() const{
    return (this->end_ - this->begin_);
}

Size_t Base::Buffer::capacity() const
{
    return this->data_.capacity();
}

void SinBack::Base::Buffer::clear() {
    this->begin_ = this->end_ = 0;
    this->data_.clear();
}

SSize_t Base::Buffer::get(void *buf, Size_t len)
{
    if (len > this->size()){
        return -1;
    }
    SSize_t read_len = 0;
    Size_t i = this->begin_;
    char* buffer = (char*)buf;

    for (; i < this->end_ && buffer; ++i){
        *buffer++ = this->data_[this->begin_++];
        ++read_len;
    }
    *buffer = CHAR_END;
    return read_len;
}

SSize_t Base::Buffer::get(std::string &buf, Size_t len)
{
    if (len > this->size()){
        return -1;
    }
    SSize_t read_len = 0;
    Size_t i = this->begin_;

    for (; i < this->end_; ++i){
        buf.push_back(this->data_[this->begin_++]);
        ++read_len;
    }
    return read_len;
}

SSize_t Base::Buffer::get(Base::Buffer &buf, Size_t len)
{
    if (len > this->size()){
        return -1;
    }
    SSize_t read_len = 0;
    Size_t i = this->begin_;

    buf.scan();

    if (len + buf.size() >= buf.capacity()){
        buf.data_.reserve(buf.capacity() + len);
    }

    for (; i < this->end_; ++i){
        buf.data_[buf.end_++] = this->data_[this->begin_++];
        ++read_len;
    }
    buf.data_[buf.end_] = CHAR_END;
    return read_len;
}

SSize_t Base::Buffer::in(const void *buf, Size_t len)
{
    std::string buffer((const char*)buf);
    return this->in(buffer, len);
}

SSize_t Base::Buffer::in(const std::string &buf, Size_t len)
{
    if (len > buf.size()){
        return -1;
    }
    SSize_t write_len = 0;
    Size_t i = 0;

    this->scan();

    if (this->size() + len >= this->capacity()){
        this->data_.reserve(this->data_.capacity() + len);
    }

    for (; i < len; ++i){
        this->data_[this->end_++] = buf[i];
        ++write_len;
    }
    this->data_[this->end_] = CHAR_END;
    return write_len;
}

SSize_t Base::Buffer::in(const Base::Buffer &buf, Size_t len)
{
    if (len > buf.size()){
        return -1;
    }
    SSize_t write_len = 0;
    SSize_t i = 0;

    this->scan();

    if (this->size() + len >= this->capacity()){
        this->data_.reserve(this->data_.capacity() + len);
    }

    for (; i < len; ++i){
        this->data_[this->end_++] = buf.data_[buf.begin_ + i];
        ++write_len;
    }
    this->data_[this->end_] = CHAR_END;
    return write_len;
}

SSize_t Base::Buffer::getAll(std::string &buf)
{
    SSize_t read_len = 0;
    Size_t i = this->begin_;

    for (; i < this->end_; ++i){
        buf.push_back(this->data_[this->begin_++]);
        ++read_len;
    }
    return read_len;
}

SSize_t Base::Buffer::getAll(Base::Buffer &buf)
{
    buf.scan();

    if (buf.size() + this->size() >= buf.capacity()){
        buf.data_.reserve(buf.data_.capacity() + this->size());
    }

    SSize_t read_len = 0;
    Size_t i = this->begin_;

    for (; i < this->end_; ++i){
        buf.data_[buf.end_++] = this->data_[this->begin_++];
        ++read_len;
    }
    buf.data_[buf.end_] = CHAR_END;
    return read_len;
}

SSize_t Base::Buffer::inAll(const std::string &buf)
{
    return this->in(buf, buf.size());
}

SSize_t Base::Buffer::inAll(const Base::Buffer &buf) {
    return this->in(buf, buf.size());
}

SSize_t Base::Buffer::append(const std::string &buf)
{
    return this->inAll(buf);
}

SSize_t Base::Buffer::append(const Base::Buffer &buf)
{
    return this->inAll(buf);
}

bool Base::Buffer::remove(Size_t len)
{
    if (len > this->size()){
        return false;
    }
    this->end_ -= len;
    this->data_[this->end_] = CHAR_END;
    return true;
}

Base::Buffer &Base::Buffer::operator=(const std::string &buf)
{
    this->begin_ = this->end_ = 0;
    this->data_.reserve(buf.capacity());
    Size_t i = 0, len = buf.length();

    for (; i < len; ++i){
        this->data_[this->end_++] = buf[i];
    }
    this->data_[this->end_] = CHAR_END;
    return *this;
}

Base::Buffer &Base::Buffer::operator=(const Base::Buffer &buf)
{
    if (this == &buf){
        return *this;
    }
    this->data_.reserve(buf.capacity());
    Size_t i = 0, len = buf.size();

    for (; i < len; ++i){
        this->data_[this->end_++] = buf.data_[buf.begin_ + i];
    }
    this->data_[this->end_] = CHAR_END;
    return *this;
}

Base::Buffer &Base::Buffer::operator+=(const std::string &buf)
{
    this->append(buf);
    return *this;
}

Base::Buffer &Base::Buffer::operator+=(const Base::Buffer &buf)
{
    if (this == &buf){
        return *this;
    }
    this->append(buf);
    return *this;
}

Base::Buffer &Base::Buffer::operator<<(const std::string &buf)
{
    return this->operator=(buf);
}

Base::Buffer &Base::Buffer::operator<<(const Base::Buffer &buf)
{
    return this->operator=(buf);
}

Base::Buffer operator+(const Base::Buffer &buf1, const Base::Buffer &buf2)
{
    Base::Buffer buf;
    buf.append(buf1);
    buf.append(buf2);
    return buf;
}

Base::Buffer::operator std::string() {
    std::string buf;
    buf.reserve(this->data_.capacity());
    buf.append(this->data_.data() + this->begin_, this->size());
    return buf;
}
