/**
* FileName:   noncopyable.h
* CreateDate: 2022-03-03 22:15:52
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        禁止拷贝函数
*/
#ifndef SINBACK_NONCOPYABLE_H
#define SINBACK_NONCOPYABLE_H

namespace SinBack {

    class noncopyable {
    public:
        noncopyable(const noncopyable &) = delete;

        noncopyable &operator=(const noncopyable &) = delete;

    protected:
        noncopyable() = default;

        ~noncopyable() = default;
    };
}

#endif //SINBACK_NONCOPYABLE_H
