/**
* FileName:   FileUtil.cc
* CreateDate: 2022-03-04 10:40:01
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "FileUtil.h"

using namespace SinBack;

Base::Path::Path() noexcept
    : path_()
    , cur_ch_('/')
    , absolute_(false)
{
}

Base::Path::Path(const Base::Path &path)
    : Path()
{
    this->cur_ch_ = path.cur_ch_;
    this->path_ = path.path_;
    this->absolute_ = path.absolute_;
    this->tmp_ = path.tmp_;
}

Base::Path::Path(Base::Path &&path) noexcept
    : Path()
{
    this->path_ = std::move(path.path_);
    this->cur_ch_ = path.cur_ch_;
    this->absolute_ = path.absolute_;
    this->tmp_ = path.tmp_;
}

Base::Path::Path(const Base::Path::string_type& path)
    : Path()
{
    this->tmp_ = path;
    this->format(path);
}

Base::Path::Path(Base::Path::string_type &&path)
    : Path()
{
    this->tmp_ = path;
    this->format(std::forward<string_type&>(path));
}

Base::Path::~Path()
{
    this->clear();
}

std::vector<Base::Path::string_type> Base::Path::split(const string_type &path, value_type ch)
{
    std::string::size_type lastPos = 0, pos = path.find_first_of(ch, lastPos);
    std::vector<std::string> tokens;

    while (lastPos != std::string::npos) {
        if (pos != lastPos)
            tokens.push_back(path.substr(lastPos, pos - lastPos));
        lastPos = pos;
        if (lastPos == std::string::npos || lastPos + 1 == path.length())
            break;
        pos = path.find_first_of(ch, ++lastPos);
    }

    return tokens;
}

void Base::Path::format(const string_type& path)
{
    bool check = true;
    value_type ch = '/';
    try {
#ifdef OS_WINDOWS
        check = std::regex_match(path, std::basic_regex<value_type>(L"[a-zA-Z]:([\\\\]{2}){1}([\\w \u4e00-\u9fa5]*[\\\\]?)*[.\\w]*"));
        ch = '\\';
        if (!check){
            check = std::regex_match(path, std::basic_regex<value_type>(L"[a-zA-Z]:([/]{1}){1}([\\w \u4e00-\u9fa5]*[//]?)*[.\\w]*"));
            ch = '/';
        }
#else
        check = std::regex_match(path, std::basic_regex<value_type>(SIN_STR("^/([\\w \u4e00-\u9fa5]*[//]?)*[.\\w]*")));
        ch = '/';
#endif
    }catch (std::regex_error& exp){
        check = false;
    }
    this->absolute_ = check;
    std::vector<string_type> paths = this->split(std::forward<const string_type&>(path), ch);
    this->path_.insert(std::end(this->path_), std::begin(paths), std::end(paths));
    this->cur_ch_ = ch;
}

Base::Path &Base::Path::operator=(const Base::Path &path)
{
    if (this == &path){
        return *this;
    }
    this->absolute_ = path.absolute_;
    this->cur_ch_ = path.cur_ch_;
    this->path_ = path.path_;
    this->tmp_ = path.tmp_;
    return *this;
}

Base::Path &Base::Path::operator=(Base::Path &&path) noexcept
{
    if (this == &path){
        return *this;
    }
    this->absolute_ = path.absolute_;
    this->cur_ch_ = path.cur_ch_;
    this->path_ = std::move(path.path_);
    this->tmp_ = std::move(path.tmp_);
    return *this;
}

Base::Path &Base::Path::operator=(const Base::Path::string_type &path)
{
    this->tmp_ = path;
    this->format(path);
    return *this;
}

Base::Path &Base::Path::operator/=(const Base::Path &path)
{
    if (path.absolute_){
        return *this;
    }
    for (auto& it : path.path_){
        this->path_.push_back(it);
    }
    return *this;
}

Base::Path &Base::Path::operator/=(const Base::Path::string_type &path)
{
    Base::Path p = path;
    if (p.absolute_){
        return *this;
    }
    for (auto& it : p.path_){
        this->path_.push_back(it);
    }
    return *this;
}

Base::Path Base::operator/(const Base::Path &lhs, const Base::Path &rhs) {
    Base::Path p;
    p /= lhs;
    p /= rhs;
    return std::move(p);
}

void Base::Path::clear()
{
    this->path_.clear();
    this->tmp_.clear();
}

Base::Path &Base::Path::remove_filename()
{
    if (!has_filename()){
        return *this;
    }
    // 移除扩展名
    if (this->has_extension()){
        this->path_.pop_back();
    }
    // 移除文件名
    if (this->has_stem()){
        this->path_.pop_back();
    }
    return *this;
}

Base::Path &Base::Path::replace_filename(const Base::Path &filename)
{
    if (this == &filename){
        return *this;
    }
    if (!this->has_filename() || !filename.has_filename()){
        return *this;
    }
    if (this->has_filename()){
        this->path_.back() = filename.filename();
    }
    return *this;
}

Base::Path &Base::Path::replace_extension(const Base::Path &extension)
{
    if (this == &extension){
        return *this;
    }
    if (this->has_extension() && extension.has_extension()){
        auto it = this->path_.back().find_last_of('.');
        if (it != string_type::npos){
            string_type str = extension.extension();
            this->path_.back().replace(it, this->path_.size() - it, str.c_str());
        }
    }
    return *this;
}


Base::Path::string_type Base::Path::str() const
{
    return this->native();
}

Base::Path::string_type Base::Path::native() const
{
    string_type str;
#ifndef OS_WINDOWS
    if (this->absolute_) {
        str.push_back('/');
    }
#endif
    Size_t i = 0, len = this->path_.size();
    for (; i < len; ++i){
        str += this->path_[i];
        if (i != len - 1){
            str.push_back(this->cur_ch_);
        }
    }
    return std::move(str);
}

Base::Path::operator string_type() const
{
    string_type str;
    Size_t i = 0, len = this->path_.size();
    for (; i < len; ++i){
        str += this->path_[i];
        if (i != len - 1){
            str.push_back(this->cur_ch_);
        }
    }
    return str;
}

Base::Path Base::Path::root_name() const
{
    Base::Path p;
    if (!this->absolute_){
        return std::move(p);
    }
#ifdef OS_WINDOWS
    p = this->path_.front();
#else
    p = SIN_STR("/");
#endif
    return std::move(p);
}

Base::Path Base::Path::root_directory() const
{
    Base::Path p;
    if (!this->absolute_){
        return std::move(p);
    }
#ifdef OS_WINDOWS
    p = SIN_STR("\\");
#else
    p = SIN_STR("/");
#endif
    return std::move(p);
}

Base::Path Base::Path::root_path() const
{
    Base::Path p;
    if (!this->absolute_){
        return std::move(p);
    }
    p = this->root_name() / this->root_directory();
    return std::move(p);
}

Base::Path Base::Path::relative_path() const
{
    Base::Path p;
    if (!this->absolute_){
        p = *this;
    }else{
        string_type tmp;
        Size_t i = 1, len = this->path_.size();
        for (; i < len; ++i){
            tmp += this->path_[i];
            if (i != len - 1){
                tmp.push_back(this->cur_ch_);
            }
        }
        p = tmp;
    }
    return std::move(p);
}

Base::Path Base::Path::parent_path() const
{
    Base::Path p;
    p.absolute_ = this->absolute_;
    if (this->empty()){
        if (!this->absolute_){
            p.path_.emplace_back(SIN_STR(".."));
        }
    } else {
        Size_t end = this->path_.size() - 1, i = 0;
        for (; i < end; ++i){
            p.path_.push_back(this->path_[i]);
        }
    }
    return std::move(p);
}

Base::Path Base::Path::filename() const
{
    Base::Path p;
    if (this->empty() || !this->has_filename()){
        return std::move(p);
    }
    p = this->path_[this->path_.size() - 1];
    return std::move(p);
}

Base::Path Base::Path::stem() const {
    Base::Path p;
    if (this->empty()){
        return std::move(p);
    }
    string_type tmp = this->filename();
    if (!tmp.empty()){
        auto it = tmp.find_last_of('.');
        if (it != string_type::npos){
            if (it == 0){
                p = tmp;
            }else{
                p = tmp.substr(0, it);
            }
        }else{
            p = tmp;
        }
    }
    return std::move(p);
}

Base::Path Base::Path::extension() const {
    Base::Path p;
    if (this->empty()){
        return std::move(p);
    }
    string_type tmp = this->filename();
    if (!tmp.empty()){
        auto it = tmp.find_last_of('.');
        if (it != string_type::npos && it != 0){
            p = tmp.substr(it + 1);
        }
    }
    return std::move(p);
}

bool Base::Path::empty() const {
    return this->path_.empty();
}

bool Base::Path::has_root_name() const
{
    return this->has_root_path();
}

bool Base::Path::has_root_directory() const
{
    return this->has_root_path();
}

bool Base::Path::has_root_path() const
{
    if (this->tmp_.empty() || !this->absolute_){
        return false;
    }
    try {
#ifdef OS_WINDOWS
        return std::regex_match(this->tmp_, std::basic_regex<value_type>(SIN_STR("^[a-zA-Z]:[\\\\]{2}.*")));
#else
        return std::regex_match(this->tmp_, std::basic_regex<value_type>(SIN_STR("^/.*")));
#endif
    }catch (std::regex_error& exp){
        return false;
    }
}

bool Base::Path::has_relative_path() const
{
    return (!this->absolute_);
}

bool Base::Path::has_parent_path() const
{
    return (!this->parent_path().empty());
}

bool Base::Path::has_filename() const
{
    if (this->tmp_.empty()) return false;
    try{
#ifdef OS_WINDOWS
        return std::regex_match(this->tmp_, std::basic_regex<value_type>(SIN_STR("(.*)\\b([^\\\\]*)")));
#else
        return std::regex_match(this->tmp_, std::basic_regex<value_type>(SIN_STR("(.*)\\b([^/]*)")));
#endif
    }catch (std::regex_error& exp){
        return false;
    }
}

bool Base::Path::has_stem() const
{
    string_type str = this->filename();
    auto it = str.find_last_of('.');
    return (it == string_type::npos || it == 0);
}

bool Base::Path::has_extension() const
{
    string_type str = this->filename();
    auto it = str.find_last_of('.');
    return (it != string_type::npos && it != 0);
}

bool Base::Path::is_absolute() const
{
    return this->absolute_;
}

bool Base::Path::is_relative() const {
    return (!this->absolute_);
}
