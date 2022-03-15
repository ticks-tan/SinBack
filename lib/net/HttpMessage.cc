/**
* FileName:   HttpMessage.cc
* CreateDate: 2022-03-12 18:10:07
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "HttpMessage.h"

using namespace SinBack;

SinBack::String
Http::HttpRequest::to_string() const
{
    String request;
    request += Http::get_http_method_name(this->method) + SIN_STR(" ");
    request += this->url + SIN_STR(" ");
    switch (this->http_version) {
        case HTTP_1_0:
            request += SIN_STR("HTTP/1.0\r\n");
            break;
        default:
            request += SIN_STR("HTTP/1.1\r\n");
            break;
    }
    request += this->header.to_string();
    return request;
}

SinBack::String
Http::HttpResponse::to_string() {
    String response;
    switch (this->http_version) {
        case HTTP_1_0:
            response += SIN_STR("HTTP/1.0\r\n");
            break;
        default:
            response += SIN_STR("HTTP/1.1\r\n");
            break;
    }
    response += std::to_string(this->status_code) + SIN_STR(" ");
    response += Http::get_http_status_code_name(this->status_code) + SIN_STR("\r\n");
    if (!this->content.data().empty()){
        this->header.set_head(SIN_STR("Content-Length"), std::to_string(this->content.data().length()));
    }
    response += this->header.to_string();
    return response;
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