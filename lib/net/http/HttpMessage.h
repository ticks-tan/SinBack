/**
* FileName:   HttpMessage.h
* CreateDate: 2022-03-12 18:10:07
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_HTTPMESSAGE_H
#define SINBACK_HTTPMESSAGE_H

#include <unordered_map>
#include "base/Base.h"


namespace SinBack
{
    namespace Http
    {
        // HTTP头部信息
        class HttpHeader
        {
        public:
            using string_type = SinBack::String;
            using Map = std::unordered_map<String, String>;
            using Iterator = Map::iterator;
        public:
            HttpHeader() = default;

            void setHead(const string_type& key, const string_type& value){
                if (!key.empty() && !value.empty()) {
                    this->data_[key] = value;
                }
            }
            String getHead(const string_type& key) const{
                auto it = this->data_.find(key);
                if (it != this->data_.end()){
                    return it->second;
                }
                return "";
            }
            String toString() const{
                String str;
                for (auto& it : this->data_){
                    str += it.first;
                    str += ": ";
                    str += it.second;
                    str += "\r\n";
                }
                str += "\r\n";
                return str;
            }

            String operator [](const string_type& key) const{
                return getHead(key);
            }
            String& operator [](const string_type& key){
                return this->data_[key];
            }
            void clear() {
                this->data_.clear();
            }
        private:
            Map data_;
        };

        // HTTP内容
        class HttpContent
        {
        public:
            using string_type = SinBack::String;
            HttpContent() = default;

            String& data(){
                return data_;
            }
            Size_t size() const {
                return this->data_.size();
            }
            bool empty() const {
                return this->data_.empty();
            }
            void clear(){
                this->data_.clear();
            }
        private:
            string_type data_;
        };

        // Http请求方法
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

        // Http请求方法对应字符串
#define HTTP_METHOD_DEFINE(XX) \
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
  XX(45, FLUSH, "FLUSH")

  // 常见文件类型对应 Content-Type
#define HTTP_CONTENT_TYPE_MAP(XX)       \
    XX(0,   ".html",    "text/html")    \
    XX(1,   ".hml",     "text/html")    \
    XX(2,   ".txt",     "text/plain")   \
    XX(3,   ".xml",     "text/xml")     \
    XX(4,   ".gif",     "image/gif")    \
    XX(5,   ".jpg",     "image/x-jpg")  \
    XX(6,   ".jpeg",    "image/jpeg")   \
    XX(7,   ".png",     "image/x-png")  \
    XX(8,   ".xhtml",   "application/xhtml+xml")    \
    XX(9,   ".json",    "application/json")         \
    XX(10,  ".pdf",     "application/pdf")          \
    XX(11,  ".word",    "application/msword")       \
    XX(12,  ".doc",     "application/msword")       \
    XX(13,  ".docx",    "application/msword")       \
    XX(14,  ".bmp",     "application/x-bmp")        \
    XX(15,  ".jpe",     "image/jpeg")           \
    XX(16,  ".mp4",     "video/mpeg4")          \
    XX(17,  ".ppt",     "application/ppt")      \
    XX(18,  ".wav",     "audio/wav")            \
    XX(19,  ".xls",     "application/x-xls")    \
    XX(20,  ".pdf",     "application/pdf")      \
    XX(21,  ".css",     "text/css")             \
    XX(22,  ".js",      "text/javascript")


    // Http 回应代码对应字符
#define HTTP_STATUS_CODE_MAP(XX) \
    XX(0, 100, "Continue")          \
    XX(1, 101, "Switching Protocol")\
    XX(2, 102, "Processing")        \
    XX(3, 103, "Early Hints")     \
    XX(4, 200, "OK")             \
    XX(5, 201, "Created")        \
    XX(6, 202, "Accepted")       \
    XX(7, 203, "Non-Authoritative Information") \
    XX(8, 204, "No Content")     \
    XX(9, 205, "Reset Content")  \
    XX(10, 207, "Multi-Status")  \
    XX(11, 208, "Already Reported") \
    XX(12, 226, "IM Used")       \
    XX(13, 300, "Multiple Choice")  \
    XX(14, 301, "Moved Permanently")\
    XX(15, 302, "Found")         \
    XX(16, 303, "See Other")     \
    XX(17, 304, "Not modified")  \
    XX(18, 307, "Temporary Redirect")           \
    XX(19, 308, "Permanent Redirect")           \
    XX(20, 400, "Bad Request")   \
    XX(21, 401, "Unauthorized")  \
    XX(22, 402, "Payment Required") \
    XX(23, 403, "Forbidden")     \
    XX(24, 404, "Not Found")     \
    XX(25, 405, "Method Not Allowed")           \
    XX(26, 406, "Not Acceptable")\
    XX(27, 407, "Proxy Authentication Required")\
    XX(28, 408, "Request Timeout")  \
    XX(29, 409, "Conflict")      \
    XX(30, 410, "Gone")          \
    XX(31, 411, "Length Required")  \
    XX(32, 412, "Precondition Failed")          \
    XX(33, 413, "Payload Too Large")\
    XX(34, 414, "Url Too Long")  \
    XX(35, 415, "Unsupported Media Type")       \
    XX(36, 416, "Range Not Satisfiable")        \
    XX(37, 417, "Expectation Failed")           \
    XX(38, 418, "I'm a teapot")  \
    XX(39, 421, "Misdirected Request")          \
    XX(40, 422, "Unprocessable Entity")         \
    XX(41, 424, "Failed Dependency")\
    XX(42, 425, "Too Early")     \
    XX(43, 426, "Upgrade Required") \
    XX(44, 428, "Precondition Required")        \
    XX(45, 429, "Too Many Requests")\
    XX(46, 431, "Request Header Fields Too large") \
    XX(47, 451, "Unavailable For Legal Reasons")\
    XX(48, 500, "Internal Server Error")        \
    XX(49, 501, "Not Implemented")  \
    XX(50, 502, "Bad Gateway")   \
    XX(51, 503, "Service Unavailable")          \
    XX(52, 504, "Gateway Timeout")  \
    XX(53, 505, "HTTP Version Not Supported")   \
    XX(54, 506, "Variant Also Negotiates")      \
    XX(55, 507, "Insufficient Storage")         \
    XX(56, 508, "Loop Detected") \
    XX(57, 510, "Not Extended")  \
    XX(58, 511, "NetWork Authentication Required")


        // 根据 HTTP 状态码获取对应字符串
        String get_http_status_code_name(UInt code);

        // 获取HTTP方法名称
        String get_http_method_name(HttpMethod method);

        // 获取主要文件扩展名对应 Content-Type
        String get_http_content_type(const String& suffix);

        // Http 版本
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
            using string_type = SinBack::String;
            HttpRequest(){
                url = "";
                method = HttpMethod::HTTP_GET;
                http_version = HTTP_1_1;
            }

            // 请求URL
            string_type url;
            // 请求方法
            HttpMethod method;
            // 协议版本
            HttpVersion http_version;
            // 头部
            HttpHeader header;
            // 内容
            HttpContent content;
            // 解析为字符串格式
            string_type toString() const;
            // 清空内容
            void clear();
        };
        // Http回应
        class HttpResponse
        {
        public:
            using string_type = SinBack::String;
            HttpResponse(){
                status_code = 200;
                http_version = HTTP_1_1;
            }

            // Http版本
            HttpVersion http_version = HTTP_1_1;
            // 状态代码
            Int status_code = 0;
            // 头部
            HttpHeader header;
            // 内容
            HttpContent content;

            // 解析为字符串格式
            string_type toString();
            // 清空内容
            void clear();
        };
    }
}


#endif //SINBACK_HTTPMESSAGE_H
