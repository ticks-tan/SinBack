/**
* FileName:   File.cc
* CreateDate: 2022-03-03 22:24:31
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "File.h"
#include "Base.h"
#include <cstring>
#ifdef OS_LINUX
#include <sys/stat.h>
#endif

using namespace SinBack;

Base::File::File(const char *filename, int mode)
    : mode_(0)
    , fp_(nullptr)
    , file_name_()
{
    bool success = true;
    if (mode & RDONLY){
        this->fp_ = fopen(filename, "r");
        this->mode_ |= RDONLY;
    }
    if (mode & WRONLY){
        if (mode & RDONLY){
            success = false;
            goto ERROR;
        }
        this->fp_ = fopen(filename, "w");
        this->mode_ |= WRONLY;
    }
    if (mode & RDWR){
        if (mode & RDONLY || mode & WRONLY){
            success = false;
            goto ERROR;
        }
        this->mode_ |= RDWR;
        if (mode & EXIST){
            this->fp_ = fopen(filename, "r+");
            this->mode_ |= EXIST;
        }else{
            this->fp_ = fopen(filename, "w+");
        }
    }else if (mode & APPEND){
        if (mode & RDONLY || mode & WRONLY){
            success = false;
            goto ERROR;
        }
        this->fp_ = fopen(filename, "a+");
        this->mode_ |= APPEND;
    }
    std::strcpy(this->file_name_, filename);
    ERROR:
    return;
}

Base::File::~File()
{
    if (this->fp_){
        ::fflush(this->fp_);
        fclose(this->fp_);
    }
}

SSize_t Base::File::size()
{
    SSize_t file_size = 0;
#ifdef OS_WINDOWS
    FILE* fp = fopen(this->file_name_, "r");
    if (!fp) return SSize_t(-1);
    fseek(fp, 0, SEEK_END);
    file_size = SSize_t(ftell(fp));
    fclose(fp);
#endif
#ifdef OS_LINUX
    struct stat s{};
    stat(this->file_name_, &s);
    file_size = s.st_size;
#endif
    return file_size;
}

SSize_t Base::File::read(void *buf, Size_t len)
{
    if (!this->fp_){
        return -1;
    }
    return (SSize_t)fread(buf, len, 1, this->fp_);
}

SSize_t Base::File::write(const void *buf, Size_t len, bool flush)
{
    if (!this->fp_){
        return -1;
    }
    auto write_len = (SSize_t) fwrite(buf, len, 1, this->fp_);
    if (flush){
        ::fflush(this->fp_);
    }
    return write_len;
}

void Base::File::flush()
{
    if (this->fp_){
        ::fflush(this->fp_);
    }
}

SSize_t Base::File::seek(SSize_t offset, int mode)
{
    if (!this->fp_){
        return -1;
    }
    return fseek(this->fp_, offset, mode);
}

void Base::File::clear()
{
    if (this->fp_){
        if (this->mode_ & RDONLY){
            return;
        }
    }
}
