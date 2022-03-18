/**
* FileName:   UrlUtil.h
* CreateDate: 2022-03-18 13:44:37
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_URL_H
#define SINBACK_URL_H

#include "base/Base.h"

namespace SinBack
{
    namespace Tools
    {
        // url编码
        std::string url_encode(const std::string& url);
        // url解码
        std::string url_decode(const std::string& url);
    }
}

#endif //SINBACK_URL_H
