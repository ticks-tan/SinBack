/**
* FileName:   FileUtil.cc
* CreateDate: 2022-03-04 10:40:01
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "FileUtil.h"

#ifdef OS_WINDOWS
#else
#include <sys/stat.h>
#endif

using namespace SinBack;


bool Base::isDir(const Char* path)
{
    struct stat s_buf{};

    stat(path, &s_buf);
    if (S_ISDIR(s_buf.st_mode)){
        return true;
    }
    return false;
}

bool Base::isFile(const Char* path)
{
    struct stat s_buf{};

    stat(path, &s_buf);
    if (S_ISREG(s_buf.st_mode)){
        return true;
    }
    return false;
}


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

Base::Path operator/(const Base::Path &lhs, const Base::Path &rhs) {
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

Base::Path &Base::Path::removeFileName()
{
    if (!hasFilename()){
        return *this;
    }
    // 移除扩展名
    if (this->hasExtension()){
        this->path_.pop_back();
    }
    // 移除文件名
    if (this->hasStem()){
        this->path_.pop_back();
    }
    return *this;
}

Base::Path &Base::Path::replaceFileName(const Base::Path &filename)
{
    if (this == &filename){
        return *this;
    }
    if (!this->hasFilename() || !filename.hasFilename()){
        return *this;
    }
    if (this->hasFilename()){
        this->path_.back() = filename.fileName();
    }
    return *this;
}

Base::Path &Base::Path::replaceExtension(const Base::Path &extension)
{
    if (this == &extension){
        return *this;
    }
    if (this->hasExtension() && extension.hasExtension()){
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

Base::Path Base::Path::rootName() const
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

Base::Path Base::Path::rootDirectory() const
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

Base::Path Base::Path::rootPath() const
{
    Base::Path p;
    if (!this->absolute_){
        return std::move(p);
    }
    p /= this->rootName();
    p /= this->rootDirectory();
    return std::move(p);
}

Base::Path Base::Path::relativePath() const
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

Base::Path Base::Path::parentPath() const
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

Base::Path Base::Path::fileName() const
{
    Base::Path p;
    if (this->empty() || !this->hasFilename()){
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
    string_type tmp = this->fileName();
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
    string_type tmp = this->fileName();
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

bool Base::Path::hasRootName() const
{
    return this->hasRootPath();
}

bool Base::Path::hasRootDirectory() const
{
    return this->hasRootPath();
}

bool Base::Path::hasRootPath() const
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

bool Base::Path::hasRelativePath() const
{
    return (!this->absolute_);
}

bool Base::Path::hasParentPath() const
{
    return (!this->parentPath().empty());
}

bool Base::Path::hasFilename() const
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

bool Base::Path::hasStem() const
{
    string_type str = this->fileName();
    auto it = str.find_last_of('.');
    return (it == string_type::npos || it == 0);
}

bool Base::Path::hasExtension() const
{
    string_type str = this->fileName();
    auto it = str.find_last_of('.');
    return (it != string_type::npos && it != 0);
}

bool Base::Path::isAbsolute() const
{
    return this->absolute_;
}

bool Base::Path::isRelative() const {
    return (!this->absolute_);
}
