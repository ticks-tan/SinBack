/**
* FileName:   FileUtil.h
* CreateDate: 2022-03-03 23:03:24
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        文件操作封装
*/
#ifndef SINBACK_FILEUTIL_H
#define SINBACK_FILEUTIL_H

#include "base/Base.h"

namespace SinBack
{
    namespace Base
    {
        // 判断是否为目录
        bool isDir(const Char* path);
        // 判断是否为文件
        bool isFile(const Char* path);
        // 获取文件大小
        SSize_t getFileSize(const Char* path);
        // 清空文件
        bool clearFile(const Char* path);
        // 获取文件最后修改时间
        SSize_t getFileModifyTime(const Char* path);
        // 创建目录
        bool createDir(const SinBack::String& path, Int mode = 0755, bool recursive = false);
    }
}

#endif //SINBACK_FILEUTIL_H
