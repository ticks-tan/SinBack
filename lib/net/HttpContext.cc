/**
* FileName:   HttpContext.cc
* CreateDate: 2022-03-14 16:19:25
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "HttpContext.h"

using namespace SinBack;

Http::HttpContext::HttpContext(Http::HttpServer* server)
    : parser_()
    , server_(server)
{
    this->parser_.reset(new Http::Http1Parse(&request_, &response_));
}

Http::HttpContext::~HttpContext()
{
}

Int Http::HttpContext::sen_text(const String &text)
{
    this->response_.status_code = 200;
    this->response_.header.set_head(SIN_STR("Content-Type"), SIN_STR("text/plain;charset=UTF-8"));
    this->response_.content.data().append(text);
    return 1;
}

Int Http::HttpContext::sen_file(const String &file_name)
{
    this->response_.status_code = 200;
    return 1;
}

bool Http::HttpContext::parse_url()
{
    return false;
}

bool Http::HttpContext::parse_body()
{
    return false;
}

bool Http::HttpContext::init()
{
    this->response_.http_version = this->request_.http_version;
    this->response_.header.set_head(SIN_STR("Server"), SIN_STR("SinBack"));
    if (!this->request_.header.get_head(SIN_STR("Connection")).empty()){
        this->response_.header[SIN_STR("Connection")] = this->request_.header[SIN_STR("Connection")];
    }
    return true;
}
