/**
* FileName:   UrlUtil.h
* CreateDate: 2022-03-18 13:44:37
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_URL_H
#define SINBACK_URL_H

#include "Base.h"

namespace SinBack
{
    namespace Tools
    {
        // url编码
        SinBack::String url_encode(const SinBack::String& url);
        // url解码
        SinBack::String url_decode(const SinBack::String& url);
    }
}

#endif //SINBACK_URL_H
