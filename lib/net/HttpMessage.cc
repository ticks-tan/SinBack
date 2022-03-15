/**
* FileName:   HttpMessage.cc
* CreateDate: 2022-03-12 18:10:07
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "HttpMessage.h"

using namespace SinBack;

Http::String
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

Http::String
Http::HttpResponse::to_string() const {
    String response;
    response += SIN_STR("HTTP/1.1 ");
    response += std::to_string(this->status_code) + SIN_STR(" ");
    response += this->code_string + SIN_STR("\r\n");
    response += this->header.to_string();
    return response;
}

Http::String
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