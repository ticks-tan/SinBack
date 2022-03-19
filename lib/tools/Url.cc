/**
* FileName:   Url.cc
* CreateDate: 2022-03-18 14:43:31
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "Url.h"

using namespace SinBack;

constexpr static const UChar url_enc_code[] = "0123456789ABCDEF";

static inline unsigned char hex2i(Char hex) {
    return hex <= '9' ? hex - '0' :
           hex <= 'F' ? hex - 'A' + 10 : hex - 'a' + 10;
}

static inline bool is_hex(char ch){
    return std::isalnum(ch) || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}


std::string SinBack::Tools::url_encode(const std::string& url){
    auto len = url.length();
    String str;
    Char tmp[4] = "%00";
    Char ch;

    Int i;
    for (i = 0; i < len; ++i) {
        ch = url[i];
        if (!std::isalnum(ch) && (ch != '.' && ch != '-' && ch != '_' && ch != '~')){
            tmp[1] = (Char)url_enc_code[(UChar)ch >> 4];
            tmp[2] = (Char)url_enc_code[(UChar)ch & 0X0F];
            str += tmp;
        }else{
            str += url[i];
        }
    }
    return str;
}

std::string SinBack::Tools::url_decode(const std::string& url) {
    auto len = url.length();
    String str;
    Char ch;
    Int i = 0;

    for (; i < len;) {
        ch = url[i];
        if (ch == '%' && is_hex(url[i + 1]) && is_hex(url[i + 2])){
            str += ((hex2i(url[i + 1]) << 4) | hex2i(url[i + 2]));
            i += 3;
        }else{
            str.push_back(ch);
            ++i;
        }
    }
    return str;
}