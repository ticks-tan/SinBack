/**
* FileName:   Buffer.cc
* CreateDate: 2022-03-03 10:49:21
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "Buffer.h"
#include <cstring>

using namespace SinBack;

Base::Buffer::Buffer()
        : begin_(0), end_(0), data_(nullptr), capacity_(0) {}

Base::Buffer::Buffer(const Char *buf)
        : Buffer()
{
    const char* p = buf;
    Size_t len = strlen(buf);
    if (len > 0) {
        capacity_ = len + 1;
        data_.reset(new Char[this->capacity_]);
        std::memcpy(data_.get(), p, len);
        data_.get()[len] = '\0';
        begin_ = 0;
        end_ = len;
    }
}

Base::Buffer::Buffer(const std::vector<Char> &buf)
        : Buffer()
{
    Size_t len = buf.size();
    if (len > 0) {
        if (buf.back() != '\0') {
            len++;
        }
        capacity_ = len;
        data_.reset(new Char[this->capacity_]);
        memcpy(data_.get(), buf.data(), len);
        data_.get()[len] = '\0';
        begin_ = 0;
        end_ = len;
    }
}


Base::Buffer::Buffer(const String &buf)
        :Buffer()
{
    Size_t len = buf.size();
    if (len > 0) {
        capacity_ = len + 1;
        data_.reset(new Char[this->capacity_]);
        memcpy(data_.get(), buf.data(), len);
        data_.get()[len] = '\0';
        begin_ = 0;
        end_ = len;
    }
}

Base::Buffer::Buffer(const Base::Buffer &buf)
        :Buffer()
{
    Size_t len = buf.size();
    if (len > 0) {
        capacity_ = len + 1;
        data_.reset(new Char[this->capacity_]);
        memcpy(data_.get(), buf.data_.get() + buf.begin_, len);
        data_.get()[len] = '\0';
        begin_ = 0;
        end_ = len;
    }
}

SinBack::Base::Buffer::~Buffer()
{
    this->begin_ = this->end_ = 0;
    this->data_.reset();
}


void SinBack::Base::Buffer::clear() {
    this->begin_ = this->end_ = 0;
    if (this->size() > 0) {
        this->data_.get()[0] = CHAR_END;
    }
}

SSize_t Base::Buffer::read(Char *buf, Size_t len)
{
    if (len > this->size()){
        return -1;
    }
    SSize_t read_len = 0;
    Size_t i = this->begin_;
    Char* buffer = buf;

    for (; i < this->end_ && i < len && buffer; ++i){
        *buffer++ = this->data_.get()[this->begin_++];
        ++read_len;
    }
    *buffer = CHAR_END;
    return read_len;
}

SSize_t Base::Buffer::read(String &buf, Size_t len)
{
    if (len > this->size()){
        return -1;
    }
    SSize_t read_len = 0;
    Size_t i = this->begin_;

    for (; i < this->end_ && i < len; ++i){
        buf.push_back(this->data_.get()[this->begin_++]);
        ++read_len;
    }
    return read_len;
}

SSize_t Base::Buffer::read(Base::Buffer &buf, Size_t len)
{
    if (len > this->size()){
        return -1;
    }
    SSize_t read_len = 0;
    Size_t i = this->begin_;

    buf.scan();
    if (buf.end_ + len + 1 > buf.capacity_){
        buf.reserve(buf.end_ + len + 1);
    }

    for (; i < this->end_ && i < len; ++i){
        buf.data_.get()[buf.end_++] = this->data_.get()[this->begin_++];
        ++read_len;
    }
    buf.data_.get()[buf.end_] = CHAR_END;
    return read_len;
}

SSize_t Base::Buffer::write(const Char *buf, Size_t len)
{
    SSize_t write_len = 0;
    Size_t i = 0;
    const Char* p = buf;

    this->scan();
    if (this->end_ + len + 1 > this->capacity_){
        this->reserve(this->capacity() + len + 1);
    }
    for (; i < len ; ++i){
        this->data_.get()[this->end_++] = p[i];
        ++write_len;
    }
    this->data_.get()[this->end_] = CHAR_END;
    return write_len;
}

SSize_t Base::Buffer::write(const String &buf, Size_t len)
{
    if (len > buf.size()){
        return 0;
    }
    return this->write(buf.c_str(), len);
}

SSize_t Base::Buffer::write(const Base::Buffer &buf, Size_t len)
{
    if (len > buf.size()){
        return 0;
    }
    SSize_t write_len = 0;
    SSize_t i = 0;
    this->scan();
    if (this->end_ + len + 1 > this->capacity()){
        this->reserve(this->capacity() + len + 1);
    }

    for (; i < len; ++i){
        this->data_.get()[this->end_++] = buf.data_.get()[buf.begin_ + i];
        ++write_len;
    }
    this->data_.get()[this->end_] = CHAR_END;
    return write_len;
}

SSize_t Base::Buffer::readAll(String &buf)
{
    SSize_t read_len = 0;
    Size_t i = this->begin_;

    for (; i < this->end_; ++i){
        buf.push_back(this->data_.get()[this->begin_++]);
        ++read_len;
    }
    return read_len;
}

SSize_t Base::Buffer::readAll(Base::Buffer &buf)
{
    buf.scan();
    SSize_t read_len = 0;
    Size_t i = this->begin_;

    if (buf.end_ + this->size() + 1 > buf.capacity()){
        buf.reserve(buf.capacity() + this->size() + 1);
    }

    for (; i < this->end_; ++i){
        buf.data_.get()[buf.end_++] = this->data_.get()[this->begin_++];
        ++read_len;
    }
    buf.data_.get()[buf.end_] = CHAR_END;
    return read_len;
}

SSize_t Base::Buffer::writeAll(const String &buf)
{
    return this->write(buf.c_str(), buf.size());
}

SSize_t Base::Buffer::writeAll(const Base::Buffer &buf) {
    return this->write(buf, buf.size());
}

SSize_t Base::Buffer::append(const String &buf)
{
    return this->writeAll(buf);
}

SSize_t Base::Buffer::append(const Base::Buffer &buf)
{
    return this->writeAll(buf);
}

bool Base::Buffer::removeFront(Size_t len)
{
    if (len > this->size()){
        return false;
    }
    this->begin_ += len;
    return true;
}

bool Base::Buffer::removeBack(Size_t len)
{
    if (len > this->size() || len == 0){
        return false;
    }
    this->end_ -= len;
    this->data_.get()[this->end_] = CHAR_END;
    return true;
}

Base::Buffer &Base::Buffer::operator=(const String &buf)
{
    this->begin_ = this->end_ = 0;
    this->reserve(buf.size() + 1);
    Size_t i = 0, len = buf.size();

    for (; i < len; ++i){
        this->data_.get()[this->end_++] = buf[i];
    }
    this->data_.get()[this->end_] = CHAR_END;
    return *this;
}

Base::Buffer &Base::Buffer::operator=(const Base::Buffer &buf)
{
    if (this == &buf){
        return *this;
    }
    this->reserve(buf.capacity());
    Size_t i = 0, len = buf.size();

    for (; i < len; ++i){
        this->data_.get()[this->end_++] = buf.data_.get()[buf.begin_ + i];
    }
    this->data_.get()[this->end_] = CHAR_END;
    return *this;
}

Base::Buffer &Base::Buffer::operator+=(const String &buf)
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

Base::Buffer &Base::Buffer::operator<<(const String &buf)
{
    return this->operator+=(buf);
}

Base::Buffer &Base::Buffer::operator<<(const Base::Buffer &buf)
{
    return this->operator+=(buf);
}

Base::Buffer operator+(const Base::Buffer &buf1, const Base::Buffer &buf2)
{
    Base::Buffer buf;
    buf.append(buf1);
    buf.append(buf2);
    return buf;
}

Base::Buffer &Base::Buffer::operator>>(String &buf) {
    this->readAll(std::forward<String&>(buf));
    return *this;
}

Base::Buffer &Base::Buffer::operator>>(Base::Buffer &buf) {
    if (&buf == this){
        return *this;
    }
    this->readAll(std::forward<Buffer&>(buf));
    return *this;
}

Base::Buffer::operator String() {
    String buf(this->data_.get() + this->begin_, this->size());
    return std::move(buf);
}

void Base::Buffer::reserve(Size_t len)
{
    if (len <= this->capacity()){
        return;
    }
    Size_t old_capacity = this->capacity();
    Size_t new_capacity = this->capacity() * 2;
    if (new_capacity < len){
        new_capacity = len;
    }
    if (!this->data_){
        this->data_.reset(new Char[new_capacity], std::default_delete<Char[]>());
        this->capacity_ = new_capacity;
        return;
    }
    auto old_data = std::move(this->data_);
    this->data_.reset(new Char[new_capacity], std::default_delete<Char[]>());
    std::memcpy(this->data_.get(), old_data.get(), old_capacity);
    this->capacity_ = new_capacity;
}


