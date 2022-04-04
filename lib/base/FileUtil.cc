/**
* FileName:   FileUtil.cc
* CreateDate: 2022-03-04 10:40:01
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include <sys/stat.h>
#include <cstdio>
#include "FileUtil.h"

using namespace SinBack;


/**
 * @brief 判断文件是否存在
 * @param path
 * @return
 */
bool Base::isDir(const Char* path)
{
    struct stat s_buf{};

    stat(path, &s_buf);
    if (S_ISDIR(s_buf.st_mode)){
        return true;
    }
    return false;
}

/**
 * @brief 判断文件是否存在
 * @param path
 * @return
 */
bool Base::isFile(const Char* path)
{
    struct stat s_buf{};

    stat(path, &s_buf);
    if (S_ISREG(s_buf.st_mode)){
        return true;
    }
    return false;
}

/**
 * @brief 获取文件大小
 * @param path
 * @return
 */
SSize_t Base::getFileSize(const Char* path)
{
    struct stat s_buf{};

    stat(path, &s_buf);
    return s_buf.st_size;
}

/**
 * @brief 清空文件
 * @param path
 * @return
 */
bool Base::clearFile(const Char *path) {
    FILE *fp = std::fopen(path, "w");
    if (fp == nullptr) {
        return false;
    }
    std::fclose(fp);
    return true;
}

/**
 * @brief 获取文件最后修改时间
 * @param path
 * @return
 */
SSize_t Base::getFileModifyTime(const Char *path) {
    struct stat s_buf{};

    stat(path, &s_buf);
    return s_buf.st_mtime;
}

/**
 * @brief 创建目录
 * @param path
 * @param mode
 * @param recursive
 * @return
 */
bool Base::createDir(const SinBack::String &path, Int mode, bool recursive) {
    if (isDir(path.data())) {
        return true;
    }
    if (recursive) {
        SinBack::String parent = path.substr(0, path.rfind('/'));
        if (!createDir(parent, mode, true)) {
            return false;
        }
    }
    if (mkdir(path.data(), mode) != 0) {
        return false;
    }
    return true;
}