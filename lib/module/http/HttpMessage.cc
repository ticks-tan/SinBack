/**
* FileName:   HttpMessage.cc
* CreateDate: 2022-03-12 18:10:07
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "HttpMessage.h"

using namespace SinBack;
using namespace SinBack::Module;

Http::HttpRequest::string_type
Http::HttpRequest::toString() const
{
    string_type request;
    request += Http::get_http_method_name(this->method) + " ";
    request += this->url + " ";
    switch (this->http_version) {
        case HTTP_1_0:
            request += "HTTP/1.0\r\n";
            break;
        default:
            request += "HTTP/1.1\r\n";
            break;
    }
    request += this->header.toString();
    return request;
}

void Http::HttpRequest::clear()
{
    this->header.clear();
    this->content.clear();
    this->url.clear();
    this->method = HTTP_GET;
}

Http::HttpRequest::string_type
Http::HttpResponse::toString() {
    string_type response;
    switch (this->http_version) {
        case HTTP_1_0:
            response += "HTTP/1.0\r\n";
            break;
        default:
            response += "HTTP/1.1\r\n";
            break;
    }
    response += std::to_string(this->status_code) + " ";
    response += Http::get_http_status_code_name(this->status_code) + "\r\n";
    if (this->header.getHead("Content-Length").empty()){
        this->header.setHead("Content-Length", std::to_string(this->content.data().length()));
    }
    response += this->header.toString();
    return response;
}

void Http::HttpResponse::clear()
{
    this->header.clear();
    this->content.clear();
    this->status_code = 0;
}

SinBack::String
Http::get_http_method_name(Http::HttpMethod method)
{
#define HTTP_GET_METHOD_NAME(NUM, NAME, STR) case HTTP_##NAME:return STR;break;
    switch (method) {
        HTTP_METHOD_DEFINE(HTTP_GET_METHOD_NAME)
        default:
            return "";
            break;
    }
#undef HTTP_GET_METHOD_NAME
    return "";
}

SinBack::String
Http::get_http_content_type(const String& suffix)
{
#define GET_HTTP_CONTENT_TYPE(NUM, SUFFIX, TYPE)    if (suffix == SUFFIX) return TYPE;
    HTTP_CONTENT_TYPE_MAP(GET_HTTP_CONTENT_TYPE)
#undef GET_HTTP_CONTENT_TYPE
    return "application/octet-stream";
}

SinBack::String
Http::get_http_status_code_name(UInt code)
{
#define GET_HTTP_STATUS_CODE_NAME(NUM, CODE, STR)   case CODE: return STR;
    switch (code) {
        HTTP_STATUS_CODE_MAP(GET_HTTP_STATUS_CODE_NAME)
        default:
            return "None";
    }
#undef GET_HTTP_STATUS_CODE_NAME
}