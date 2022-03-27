/**
* FileName:   FileUtil.h
* CreateDate: 2022-03-03 23:03:24
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        文件操作封装
*/
#ifndef SINBACK_FILEUTIL_H
#define SINBACK_FILEUTIL_H

#include <regex>
#include "Base.h"

namespace SinBack
{
    namespace Base
    {
        bool isDir(const Char* path);
        bool isFile(const Char* path);
    }
}

#endif //SINBACK_FILEUTIL_H
