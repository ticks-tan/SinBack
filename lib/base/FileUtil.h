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
            Path& remove_filename();
            // 更换文件名
            Path& replace_filename(const Path& filename);
            // 更换文件扩展名
            Path& replace_extension(const Path& extension);

            string_type str() const;
            string_type native() const;
            operator string_type() const;

            // 返回根
            Path root_name() const;
            // 返回根目录
            Path root_directory() const;
            // 返回根+根目录
            Path root_path() const;
            // 相对于根目录的路径
            Path relative_path() const;
            // 返回亲目录(上级目录)
            Path parent_path() const;
            // 返回文件名
            Path filename() const;
            // 返回主干路径(除去扩展名)
            Path stem() const;
            // 返回扩展名部分
            Path extension() const;
            // 路径是否为空
            bool empty() const;
            // 检查对于路径是否存在
            bool has_root_name() const;
            bool has_root_directory() const;
            bool has_root_path() const;
            bool has_relative_path() const;
            bool has_parent_path() const;
            bool has_filename() const;
            bool has_stem() const;
            bool has_extension() const;

            // 是否为绝对路径
            bool is_absolute() const;
            // 是否为相对路径
            bool is_relative() const;

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
