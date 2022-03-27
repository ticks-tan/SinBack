/**
* FileName:   FileUtil.cc
* CreateDate: 2022-03-04 10:40:01
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include <sys/stat.h>
#include "FileUtil.h"

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
