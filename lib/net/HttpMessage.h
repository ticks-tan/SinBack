/**
* FileName:   HttpMessage.h
* CreateDate: 2022-03-12 18:10:07
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_HTTPMESSAGE_H
#define SINBACK_HTTPMESSAGE_H

#include <string>
#include <unordered_map>
#include "base/Base.h"


namespace SinBack
{
    namespace Http
    {
        using String = std::basic_string<Char>;
        // HTTP头部信息
        class HttpHeader
        {
        public:
            using Map = std::unordered_map<String, String>;
            using Iterator = Map::iterator;
        public:
            void set_head(const String& key, const String& value){
                if (!key.empty() && !value.empty()) {
                    this->data_[key] = value;
                }
            }
            String get_head(const String& key) const{
                auto it = this->data_.find(key);
                if (it != this->data_.end()){
                    return it->second;
                }
                return "";
            }
            String to_string() const{
                String str;
                for (auto& it : this->data_){
                    str += it.first;
                    str += SIN_STR(":\t");
                    str += it.second;
                    str += SIN_STR("\r\n");
                }
                str += SIN_STR("\r\n");
                return str;
            }

            String operator [](const String& key) const{
                return get_head(key);
            }
            String& operator [](const String& key){
                return this->data_[key];
            }
        private:
            Map data_;
        };

        // HTTP内容
        class HttpContent
        {
        public:
            String& data(){
                return data_;
            }
        private:
            String data_;
        };

        enum HttpMethod {
            HTTP_DELETE = 0,
            HTTP_GET = 1,
            HTTP_HEAD = 2,
            HTTP_POST = 3,
            HTTP_PUT = 4,
            HTTP_CONNECT = 5,
            HTTP_OPTIONS = 6,
            HTTP_TRACE = 7,
            HTTP_COPY = 8,
            HTTP_LOCK = 9,
            HTTP_MKCOL = 10,
            HTTP_MOVE = 11,
            HTTP_PROPFIND = 12,
            HTTP_PROPPATCH = 13,
            HTTP_SEARCH = 14,
            HTTP_UNLOCK = 15,
            HTTP_BIND = 16,
            HTTP_REBIND = 17,
            HTTP_UNBIND = 18,
            HTTP_ACL = 19,
            HTTP_REPORT = 20,
            HTTP_MKACTIVITY = 21,
            HTTP_CHECKOUT = 22,
            HTTP_MERGE = 23,
            HTTP_MSEARCH = 24,
            HTTP_NOTIFY = 25,
            HTTP_SUBSCRIBE = 26,
            HTTP_UNSUBSCRIBE = 27,
            HTTP_PATCH = 28,
            HTTP_PURGE = 29,
            HTTP_MKCALENDAR = 30,
            HTTP_LINK = 31,
            HTTP_UNLINK = 32,
            HTTP_SOURCE = 33,
            HTTP_PRI = 34,
            HTTP_DESCRIBE = 35,
            HTTP_ANNOUNCE = 36,
            HTTP_SETUP = 37,
            HTTP_PLAY = 38,
            HTTP_PAUSE = 39,
            HTTP_TEARDOWN = 40,
            HTTP_GET_PARAMETER = 41,
            HTTP_SET_PARAMETER = 42,
            HTTP_REDIRECT = 43,
            HTTP_RECORD = 44,
            HTTP_FLUSH = 45
        };

#define HTTP_METHOD_MAP(XX) \
  XX(0, DELETE, "DELETE") \
  XX(1, GET, "GET") \
  XX(2, HEAD, "HEAD") \
  XX(3, POST, "POST") \
  XX(4, PUT, "PUT") \
  XX(5, CONNECT, "CONNECT") \
  XX(6, OPTIONS, "OPTIONS") \
  XX(7, TRACE, "TRACE") \
  XX(8, COPY, "COPY") \
  XX(9, LOCK, "LOCK") \
  XX(10, MKCOL, "MKCOL") \
  XX(11, MOVE, "MOVE") \
  XX(12, PROPFIND, "PROPFIND") \
  XX(13, PROPPATCH, "PROPPATCH") \
  XX(14, SEARCH, "SEARCH") \
  XX(15, UNLOCK, "UNLOCK") \
  XX(16, BIND, "BIND") \
  XX(17, REBIND, "REBIND") \
  XX(18, UNBIND, "UNBIND") \
  XX(19, ACL, "ACL") \
  XX(20, REPORT, "REPORT") \
  XX(21, MKACTIVITY, "MKACTIVITY") \
  XX(22, CHECKOUT, "CHECKOUT") \
  XX(23, MERGE, "MERGE") \
  XX(24, MSEARCH, "M-SEARCH") \
  XX(25, NOTIFY, "NOTIFY") \
  XX(26, SUBSCRIBE, "SUBSCRIBE") \
  XX(27, UNSUBSCRIBE, "UNSUBSCRIBE") \
  XX(28, PATCH, "PATCH") \
  XX(29, PURGE, "PURGE") \
  XX(30, MKCALENDAR, "MKCALENDAR") \
  XX(31, LINK, "LINK") \
  XX(32, UNLINK, "UNLINK") \
  XX(33, SOURCE, "SOURCE") \
  XX(34, PRI, "PRI") \
  XX(35, DESCRIBE, "DESCRIBE") \
  XX(36, ANNOUNCE, "ANNOUNCE") \
  XX(37, SETUP, "SETUP") \
  XX(38, PLAY, "PLAY") \
  XX(39, PAUSE, "PAUSE") \
  XX(40, TEARDOWN, "TEARDOWN") \
  XX(41, GET_PARAMETER, "GET_PARAMETER") \
  XX(42, SET_PARAMETER, "SET_PARAMETER") \
  XX(43, REDIRECT, "REDIRECT") \
  XX(44, RECORD, "RECORD") \
  XX(45, FLUSH, "FLUSH")    \

        // 获取HTTP方法名称
        String get_http_method_name(HttpMethod method);

        enum HttpVersion
        {
            HTTP_1_0 = 0x10,
            HTTP_1_1 = 0x11,
            HTTP_2 = 0x020
        };

        // Http请求
        class HttpRequest
        {
        public:
            // 请求URL
            String url;
            // 请求方法
            HttpMethod method;
            // 协议版本
            HttpVersion http_version;
            // 头部
            HttpHeader header;
            // 内容
            HttpContent content;
            // 解析为字符串格式
            String to_string() const;
        };
        // Http回应
        class HttpResponse
        {
        public:
            // Http版本
            HttpVersion http_version = HTTP_1_1;
            // Http方法
            HttpMethod method;
            // 状态代码
            Int status_code;
            // 状态码解释
            String code_string;
            // 头部
            HttpHeader header;
            // 内容
            HttpContent content;

            // 解析为字符串格式
            String to_string() const;
        };
    }
}


#endif //SINBACK_HTTPMESSAGE_H
