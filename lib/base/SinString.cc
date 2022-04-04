/**
* FileName:   SinString.cc
* CreateDate: 2022-04-03 00:27:38
* Author:     ticks
* Email:      2938384958@qq.com
* Des:
*/
#include <stdexcept>
#include "SinString.h"

using namespace SinBack;


SinBack::Base::SinString::SinString()
    : data_(new Char[1])
    , length_(0)
    , capacity_(1)
{
    data_.get()[0] = CHAR_END;
}

SinBack::Base::SinString::SinString(const SinBack::Char *str)
    : length_(0)
    , capacity_(0)
{
    this->capacity_ = strlen(str) + 1;
    this->data_.reset(new Char[this->capacity_]);
    Size_t pos = 0;
    for (; str[pos] != CHAR_END; ++pos) {
        this->data_.get()[this->length_++] = str[pos];
    }
    this->data_.get()[this->length_] = CHAR_END;
}

Base::SinString::SinString(const Char *str, Size_t len)
    : length_(0)
    , capacity_(0)
{
    this->capacity_ = len + 1;
    this->data_.reset(new Char[this->capacity_]);
    Size_t pos = 0;
    for (; pos < len; ++pos) {
        this->data_.get()[this->length_++] = str[pos];
    }
    this->data_.get()[this->length_] = CHAR_END;
}

Base::SinString::SinString(Size_t len, Char ch)
    : length_(len)
    , capacity_(len + 1)
{
    if (len == 0) {
        this->data_.reset(new Char[1]);
        this->data_.get()[0] = CHAR_END;
    } else {
        this->data_.reset(new Char[this->capacity_]);
        for (Size_t i = 0; i < len; ++i) {
            this->data_.get()[i] = ch;
        }
        this->data_.get()[len] = CHAR_END;
    }
}

Base::SinString::SinString(Size_t len)
    : length_(len)
    , capacity_(len + 1)
{
    if (len == 0) {
        this->data_.reset(new Char[1]);
        this->data_.get()[0] = CHAR_END;
    } else {
        this->data_.reset(new Char[this->capacity_]);
        for (Size_t i = 0; i < len; ++i) {
            this->data_.get()[i] = CHAR_END;
        }
        this->data_.get()[len] = CHAR_END;
    }
}


SinBack::Base::SinString::SinString(const SinBack::Base::SinString &str)
    : length_(str.length_)
    , capacity_(str.capacity_)
{
    this->data_.reset(new Char[this->capacity_]);
    std::strncpy(this->data_.get(), str.data_.get(), this->length_);
}

SinBack::Base::SinString::SinString(SinBack::Base::SinString &&str) noexcept
    : data_(std::move(str.data_))
    , length_(str.length_)
    , capacity_(str.capacity_)
{
    str.length_ = 0;
    str.capacity_ = 0;
}

SinBack::Base::SinString::~SinString()
{
    if (this->capacity_ > 0) {
        this->data_.reset();
    }
    this->length_ = this->capacity_ = 0;
}

SinBack::Base::SinString &SinBack::Base::SinString::operator=(const SinBack::Base::SinString &str)
{
    if (this == &str) {
        return *this;
    }
    this->length_ = str.length_;
    if (this->capacity_ < this->length_ + 1) {
        this->capacity_ = this->length_ + 1;
        this->data_.reset(new Char[this->capacity_]);
    }
    std::strncpy(this->data_.get(), str.data_.get(), this->length_);
    this->data_.get()[this->length_] = CHAR_END;
    return *this;
}

SinBack::Base::SinString &SinBack::Base::SinString::operator=(SinBack::Base::SinString &&str)
 noexcept {
    if (this == &str) {
        return *this;
    }
    this->data_ = std::move(str.data_);
    this->length_ = str.length_;
    this->capacity_ = str.capacity_;
    str.length_ = 0;
    str.capacity_ = 0;
    return *this;
}

SinBack::Base::SinString &SinBack::Base::SinString::operator=(const SinBack::Char *str)
{
    this->length_ = strlen(str);
    if (this->capacity_ < this->length_ + 1) {
        this->capacity_ = this->length_ + 1;
        this->data_.reset(new Char[this->capacity_]);
    }
    std::strncpy(this->data_.get(), str, this->length_);
    this->data_.get()[this->length_] = CHAR_END;
    return *this;
}

SinBack::Base::SinString &SinBack::Base::SinString::operator+=(const SinBack::Base::SinString &str)
{
    // 1.5倍扩容
    if (this->capacity_ < this->length_ + str.length_ + 1) {
        this->reserve(std::max(this->length_ + str.length_ + 1, this->capacity_ + this->capacity_ / 2));
    }
    std::strncpy(this->data_.get() + this->length_, str.data_.get(), str.length_);
    this->length_ += str.length_;
    this->data_.get()[this->length_] = CHAR_END;
    return *this;
}

SinBack::Base::SinString &SinBack::Base::SinString::operator+=(const SinBack::Char *str)
{
    Size_t len = strlen(str);
    if (this->capacity_ < this->length_ + len + 1) {
        this->reserve(std::max(this->length_ + len + 1, this->capacity_ + this->capacity_ / 2));
    }
    std::strncpy(this->data_.get() + this->length_, str, len);
    this->length_ += len;
    this->data_.get()[this->length_] = CHAR_END;
    return *this;
}

SinBack::Base::SinString &SinBack::Base::SinString::operator+=(SinBack::Char ch)
{
    if (this->capacity_ < this->length_ + 2) {
        this->reserve(std::max(this->length_ + 2, this->capacity_ + this->capacity_ / 2));
    }
    this->data_.get()[this->length_++] = ch;
    this->data_.get()[this->length_] = CHAR_END;
    return *this;
}

char &SinBack::Base::SinString::operator[](int index)
{
    return this->data_.get()[index];
}

const char &SinBack::Base::SinString::operator[](int index) const {
    return this->data_.get()[index];
}

SinBack::Base::SinString &SinBack::Base::SinString::operator<<(const SinBack::Base::SinString &str) {
    return this->operator+=(str);
}

SinBack::Base::SinString &SinBack::Base::SinString::operator<<(const SinBack::Char *str) {
    return this->operator+=(str);
}

SinBack::Base::SinString &SinBack::Base::SinString::operator<<(SinBack::Char ch) {
    return this->operator+=(ch);
}

bool SinBack::Base::SinString::operator<(const SinBack::Base::SinString &str) const {
    return std::strncmp(this->data_.get(), str.data_.get(), std::min(this->length_, str.length_)) < 0;
}

bool SinBack::Base::SinString::operator>(const SinBack::Base::SinString &str) const {
    return std::strncmp(this->data_.get(), str.data_.get(), std::min(this->length_, str.length_)) > 0;
}

bool Base::SinString::operator==(const Base::SinString &str) const {
    if (this->length_ != str.length_) {
        return false;
    }
    return std::strncmp(this->data_.get(), str.data_.get(), this->length_) == 0;
}

bool Base::SinString::operator==(const Char *str) const {
    Size_t len = std::strlen(str);
    if (this->length_ != len) {
        return false;
    }
    return std::strncmp(this->data_.get(), str, this->length_) == 0;
}

bool Base::SinString::startsWith(const Base::SinString &str) const
{
    if (this->length_ < str.length_) {
        return false;
    }
    return std::strncmp(this->data_.get(), str.data_.get(), str.length_) == 0;
}

bool Base::SinString::startsWith(const Char *str) const
{
    Size_t len = std::strlen(str);
    if (this->length_ < len) {
        return false;
    }
    return std::strncmp(this->data_.get(), str, len) == 0;
}

bool Base::SinString::endsWith(const Base::SinString &str) const
{
    if (this->length_ < str.length_) {
        return false;
    }
    return std::strncmp(this->data_.get() + this->length_ - str.length_, str.data_.get(), str.length_) == 0;
}

bool Base::SinString::endsWith(const Char *str) const
{
    Size_t len = std::strlen(str);
    if (this->length_ < len) {
        return false;
    }
    return std::strncmp(this->data_.get() + this->length_ - len, str, len) == 0;
}

bool Base::SinString::contains(const Base::SinString &str) const
{
    if (this->length_ < str.length_) {
        return false;
    }
    return std::strstr(this->data_.get(), str.data_.get()) != nullptr;
}

bool Base::SinString::contains(const Char *str) const
{
    Size_t len = std::strlen(str);
    if (this->length_ < len) {
        return false;
    }
    return std::strstr(this->data_.get(), str) != nullptr;
}

Size_t Base::SinString::length() const {
    return this->length_;
}

bool Base::SinString::empty() const {
    return this->length_ == 0;
}

void Base::SinString::clear()
{
    this->length_ = 0;
    if (this->capacity_ > 0) {
        this->data_.get()[0] = CHAR_END;
    }
}

Char *Base::SinString::data() {
    return this->data_.get();
}

const Char *Base::SinString::const_data() const {
    if (this->empty()){
        return nullptr;
    }
    return this->data_.get();
}

Base::SinString &Base::SinString::append(const Base::SinString &str) {
    return this->operator+=(str);
}

Base::SinString &Base::SinString::append(const Char *str, Size_t len)
{
    if (len == 0) {
        return *this;
    }
    if (this->capacity_ < this->length_ + len + 1) {
        this->reserve(std::max(this->length_ + len + 1, this->capacity_ + this->capacity_ / 2));
    }
    std::strncpy(this->data_.get() + this->length_, str, len);
    this->length_ += len;
    this->data_.get()[this->length_] = CHAR_END;
    return *this;
}

Base::SinString &Base::SinString::append(Char ch) {
    return this->operator+=(ch);
}

Base::SinString &Base::SinString::insert(Size_t index, const Base::SinString &str) {
    return this->insert(index, str.data_.get(), str.length_);
}

Base::SinString &Base::SinString::insert(Size_t index, const Char *str) {
    return this->insert(index, str, std::strlen(str));
}

Base::SinString &Base::SinString::insert(Size_t index, const Char *str, Size_t len)
{
    if (index < 0 || index > this->length_) {
        throw std::out_of_range("index out of range");
    }
    if (len == 0) {
        return *this;
    }
    if (this->capacity_ < this->length_ + len + 1) {
        this->reserve(std::max(this->length_ + len + 1, this->capacity_ + this->capacity_ / 2));
    }
    // 从后向前移动，否则会覆盖数据
    std::move_backward(this->data_.get() + index, this->data_.get() + this->length_,
                       this->data_.get() + this->length_ + len);
    // 插入数据
    std::strncpy(this->data_.get() + index, str, len);
    this->length_ += len;
    this->data_.get()[this->length_] = CHAR_END;
    return *this;
}

Base::SinString &Base::SinString::insert(Size_t index, Char ch)
{
    if (index < 0 || index > this->length_) {
        throw std::out_of_range("index out of range");
    }
    if (this->capacity_ < this->length_ + 2) {
        this->reserve(std::max(this->length_ + 2, this->capacity_ + this->capacity_ / 2));
    }
    // 从后向前移动，否则会覆盖数据
    std::move_backward(this->data_.get() + index, this->data_.get() + this->length_,
                       this->data_.get() + this->length_ + 1);
    // 插入数据
    this->data_.get()[index] = ch;
    this->length_ += 1;
    this->data_.get()[this->length_] = CHAR_END;
    return *this;
}

Base::SinString &Base::SinString::erase(Size_t index, Size_t count)
{
    if (index < 0 || index > this->length_) {
        throw std::out_of_range("index out of range");
    }
    if (count < 0 || count > this->length_ - index) {
        throw std::out_of_range("count out of range");
    }
    if (count == 0) {
        return *this;
    }
    std::move(this->data_.get() + index + count, this->data_.get() + this->length_,
              this->data_.get() + index);
    this->length_ -= count;
    this->data_.get()[this->length_] = CHAR_END;
    return *this;
}

Base::SinString &Base::SinString::replace(int index, int count, const Base::SinString &str) {
    return this->replace(index, count, str.data_.get(), str.length_);
}

Base::SinString &Base::SinString::replace(int index, int count, const Char *str, Size_t len)
{
    if (index < 0 || index > this->length_) {
        throw std::out_of_range("index out of range");
    }
    if (count < 0 || count > this->length_ - index) {
        throw std::out_of_range("count out of range");
    }
    if (count == 0) {
        return *this;
    }
    if (this->capacity_ < this->length_ + len - count + 1) {
        this->reserve(std::max(this->length_ + len - count + 1, this->capacity_ + this->capacity_ / 2));
    }
    // 移动需要删除的数据后面的数据，然后直接拷贝数据到指定位置
    std::move_backward(this->data_.get() + index + count, this->data_.get() + this->length_,
                       this->data_.get() + this->length_ + len - count);
    std::strncpy(this->data_.get() + index, str, len);
    this->length_ += len - count;
    this->data_.get()[this->length_] = CHAR_END;
    return *this;
}

Base::SinString &Base::SinString::replace(int index, int count, Char ch)
{
    if (index < 0 || index > this->length_) {
        throw std::out_of_range("index out of range");
    }
    if (count < 0 || count > this->length_ - index) {
        throw std::out_of_range("count out of range");
    }
    if (count == 0) {
        return *this;
    }
    if (this->capacity_ < this->length_ + 1 - count + 1) {
        this->reserve(std::max(this->length_ + 1 - count + 1, this->capacity_ + this->capacity_ / 2));
    }
    // 移动需要删除的数据后面的数据，然后直接拷贝数据到指定位置
    std::move_backward(this->data_.get() + index + count, this->data_.get() + this->length_,
                       this->data_.get() + this->length_ + 1 - count);
    this->data_.get()[index] = ch;
    this->length_ += 1 - count;
    this->data_.get()[this->length_] = CHAR_END;
    return *this;
}

Size_t Base::SinString::reserve(Size_t size)
{
    if (size <= this->capacity_) {
        return this->capacity_;
    }
    if (size < this->length_ + 1) {
        throw std::length_error("size is too small");
    }
    this->capacity_ = size;
    // 开辟新空间拷贝数据然后利用std::move将新数据移动到原数据
    UniquePtr<Char> new_data(new Char[this->capacity_ + 1]);
    std::strncpy(new_data.get(), this->data_.get(), this->length_);
    this->data_ = std::move(new_data);
    this->data_.get()[this->length_] = CHAR_END;
    return this->capacity_;
}

Size_t Base::SinString::resize(Size_t new_size, Char ch)
{
    if (new_size < this->length_) {
        this->length_ = new_size;
        this->data_.get()[this->length_] = CHAR_END;
        return this->length_;
    }
    if (new_size == this->length_) {
        return this->length_;
    }
    if (new_size > this->capacity_ - 1) {
        this->reserve(std::max(new_size, this->capacity_ + this->capacity_ / 2));
    }
    std::memset(this->data_.get() + this->length_, ch, sizeof(Char) * (new_size - this->length_));
    this->length_ = new_size;
    this->data_.get()[this->length_] = CHAR_END;
    return this->length_;
}

Base::SinString Base::SinString::substr(Size_t index, SSize_t count) const
{
    SinString str;
    if (index < 0 || index > this->length_) {
        throw std::out_of_range("index out of range");
    }
    if (count == SinString::npos) {
        count = SSize_t(this->length_ - index);
    }
    if (count < 0 || count > this->length_ - index) {
        throw std::out_of_range("count out of range");
    }
    str.reserve(count + 1);
    str.append(this->data_.get() + index, count);
    return std::move(str);
}

Base::SinString::Iterator Base::SinString::begin() {
    return Iterator(this->data_.get());
}

Base::SinString::ConstIterator Base::SinString::cbegin() const {
    return ConstIterator(this->data_.get());
}

Base::SinString::Iterator Base::SinString::end() {
    return Iterator(this->data_.get() + this->length_);
}

Base::SinString::ConstIterator Base::SinString::cend() const {
    return ConstIterator(this->data_.get() + this->length_);
}

Size_t Base::SinString::find(const Base::SinString &str, Size_t index) const {
    return this->find(str.const_data(), index);
}

Size_t Base::SinString::find(const Char *str, Size_t index) const
{
    Size_t len = std::strlen(str);
    if (index < 0 || index > this->length_) {
        throw std::out_of_range("index out of range");
    }
    if (len == 0) {
        return SinString::npos;
    }
    if (len > this->length_ - index) {
        return SinString::npos;
    }
    const Char *p = std::strstr(this->data_.get() + index, str);
    if (p == nullptr) {
        return SinString::npos;
    }
    return Size_t(p - this->data_.get());
}

Size_t Base::SinString::find(Char ch, Size_t index) const
{
    if (index < 0 || index > this->length_) {
        throw std::out_of_range("index out of range");
    }
    const Char *p = std::strchr(this->data_.get() + index, ch);
    if (p == nullptr) {
        return SinString::npos;
    }
    return Size_t(p - this->data_.get());
}

Size_t Base::SinString::rfind(const Base::SinString &str, Size_t index) const {
    return this->rfind(str.const_data(), index);
}

Size_t Base::SinString::rfind(const Char *str, Size_t index) const
{
    SinString str_tmp(this->data_.get());
    std::reverse(str_tmp.begin(), str_tmp.end());
    return str_tmp.find(str, index);
}

Size_t Base::SinString::rfind(Char ch, Size_t index) const
{
    if (index < 0 || index > this->length_) {
        return SinString::npos;
    }
    auto i = (SSize_t)index;
    while (i >= 0) {
        if (this->data_.get()[i] == ch) {
            return i;
        }
        i--;
    }
    return SinString::npos;
}

void Base::SinString::push_back(Char ch) {
    this->append(ch);
}

void Base::SinString::pop_back(){
    if (this->length_ == 0) {
        throw std::out_of_range("pop_back: string is empty");
    }
    this->length_--;
    this->data_.get()[this->length_] = CHAR_END;
}




