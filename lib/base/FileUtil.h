/**
* FileName:   FileUtil.h
* CreateDate: 2022-03-03 23:03:24
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        文件操作封装
*/
#ifndef SINBACK_FILEUTIL_H
#define SINBACK_FILEUTIL_H

#include <string>
#include <regex>
#include "Base.h"

namespace SinBack
{
    namespace Base
    {

        bool isDir(const Char* path);
        bool isFile(const Char* path);

        // 文件路径
        class Path
        {
        public:
#ifdef OS_WINDOWS
            typedef Char value_type;
            static constexpr value_type preferred_separator = SIN_STR('\\');
#endif
#ifdef OS_LINUX
            typedef Char value_type;
            static constexpr value_type preferred_separator = SIN_STR('/');
#endif
        typedef std::basic_string<value_type> string_type;
        public:
            Path() noexcept;
            Path(const Path& path);
            Path(Path&& path) noexcept;
            Path(string_type&& path);
            Path(const string_type& path);
            ~Path();

            Path& operator = (const Path& path);
            Path& operator = (Path&& path) noexcept ;
            Path& operator = (const string_type& path);
            Path& operator /= (const Path& path);
            Path& operator /= (const string_type& path);
            friend Path operator / (const Path& lhs, const Path& rhs);

            // 清除路径
            void clear();
            // 移除文件名部分
            Path& removeFileName();
            // 更换文件名
            Path& replaceFileName(const Path& filename);
            // 更换文件扩展名
            Path& replaceExtension(const Path& extension);

            string_type str() const;
            string_type native() const;
            operator string_type() const;

            // 返回根
            Path rootName() const;
            // 返回根目录
            Path rootDirectory() const;
            // 返回根+根目录
            Path rootPath() const;
            // 相对于根目录的路径
            Path relativePath() const;
            // 返回亲目录(上级目录)
            Path parentPath() const;
            // 返回文件名
            Path fileName() const;
            // 返回主干路径(除去扩展名)
            Path stem() const;
            // 返回扩展名部分
            Path extension() const;
            // 路径是否为空
            bool empty() const;
            // 检查对于路径是否存在
            bool hasRootName() const;
            bool hasRootDirectory() const;
            bool hasRootPath() const;
            bool hasRelativePath() const;
            bool hasParentPath() const;
            bool hasFilename() const;
            bool hasStem() const;
            bool hasExtension() const;

            // 是否为绝对路径
            bool isAbsolute() const;
            // 是否为相对路径
            bool isRelative() const;

        private:
            void format(const string_type& path);
            std::vector<string_type> split(const string_type& path, value_type ch);
        private:
            string_type tmp_;
            std::vector<string_type> path_;
            value_type cur_ch_;
            bool absolute_;
        };


    }
}

#endif //SINBACK_FILEUTIL_H
