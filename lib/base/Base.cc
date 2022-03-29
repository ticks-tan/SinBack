/**
* FileName:   Base.cc
* CreateDate: 2022-03-03 10:25:17
* Author:     ticks
* Email:      2938384958@qq.com
* Des:
*/

#include "base/Base.h"

using namespace SinBack;

bool SinBack::startWith(const String& str1, const String& str2){
    if (str1.length() > str2.length()) return false;
    Size_t pos = 0, len1 = str1.length();
    for (; pos < len1; ++pos){
        if (str1[pos] != str2[pos]){
            return false;
        }
    }
    return true;
}

bool SinBack::endWith(const String& str1, const String& str2){
    if (str1.length() > str2.length()) return false;
    Size_t pos = 0, len1 = str1.length(), len2 = str2.length();
    for (; pos < len1; ++pos){
        if (str1[len1 - pos - 1] != str2[len2 - pos - 1]){
            return false;
        }
    }
    return true;
}