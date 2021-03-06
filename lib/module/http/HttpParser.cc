/**
* FileName:   HttpParser.cc
* CreateDate: 2022-03-13 15:04:35
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "HttpParser.h"
#include "base/Url.h"

using namespace SinBack;
using namespace SinBack::Module;

Int on_url(http_parser* parser, const Char* data, Size_t len);
Int on_status(http_parser* parser, const Char* data, Size_t len);
Int on_header_field(http_parser* parser, const Char* data, Size_t len);
Int on_header_value(http_parser* parser, const Char* data, Size_t len);
Int on_body(http_parser* parser, const Char* data, Size_t len);

Int on_message_begin(http_parser* parse);
Int on_headers_complete(http_parser* parser);
Int on_message_complete(http_parser* parser);
Int on_chunk_header(http_parser* parser);
Int on_chunk_complete(http_parser* parser);


Int on_url(http_parser* parser, const Char* data, Size_t len)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->request_->url.clear();
        hp->request_->url.append(data, len);
        // url解码
        hp->request_->url = std::move(SinBack::Tools::url_decode(hp->request_->url));
        hp->set_status(Http::HP_PARSE_URL);
        return 0;
    }
    return 1;
}

Int on_status(http_parser* parser, const Char* data, Size_t len)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->set_status(Http::HP_PARSE_STATUS);
        return 0;
    }
    return 1;
}
Int on_header_field(http_parser* parser, const Char* data, Size_t len)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->header_field.append(data, len);
        hp->set_status(Http::HP_PARSE_HEADER_FIELD);
        return 0;
    }
    return 1;
}
Int on_header_value(http_parser* parser, const Char* data, Size_t len)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->header_value.append(data, len);
        if (!hp->header_field.empty() && !hp->header_value.empty()){
            hp->request_->header.setHead(hp->header_field, hp->header_value);
            hp->header_field.clear();
            hp->header_value.clear();
        }
        hp->set_status(Http::HP_PARSE_HEADER_VALUE);
        return 0;
    }
    return 1;
}
Int on_body(http_parser* parser, const Char* data, Size_t len)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->set_status(Http::HP_PARSE_BODY);
        hp->request_->content.data().append(data, len);
        return 0;
    }
    return 1;
}

Int on_message_begin(http_parser* parser)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->set_status(Http::HP_PARSE_BEGIN);
        return 0;
    }
    return 1;
}
Int on_headers_complete(http_parser* parser)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->set_status(Http::HP_PARSE_HEADERS_END);
        return 0;
    }
    return 1;
}
Int on_message_complete(http_parser* parser)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->request_->method = (Http::HttpMethod)parser->method;
        hp->set_status(Http::HP_PARSE_END);
        return 0;
    }
    return 1;
}
Int on_chunk_header(http_parser* parser)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->set_status(Http::HP_PARSE_CHUNK_HEADER);
        return 0;
    }
    return 1;
}
Int on_chunk_complete(http_parser* parser)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->set_status(Http::HP_PARSE_CHUNK_END);
        return 0;
    }
    return 1;
}


Http::Http1Parse::Http1Parse(HttpRequest* req, HttpResponse* resp)
    : setting_()
    , parser_()
    , status_(HP_NULL)
    , request_(req)
    , response_(resp)
{
    http_parser_settings_init(&this->setting_);
    http_parser_init(&this->parser_, HTTP_REQUEST);
    this->parser_.data = this;

    this->setting_.on_message_begin = on_message_begin;
    this->setting_.on_url = on_url;
    this->setting_.on_status = on_status;
    this->setting_.on_header_field = on_header_field;
    this->setting_.on_header_value = on_header_value;
    this->setting_.on_headers_complete = on_headers_complete;
    this->setting_.on_body = on_body;
    this->setting_.on_message_complete = on_message_complete;
    this->setting_.on_chunk_header = on_chunk_header;
    this->setting_.on_chunk_complete = on_chunk_complete;
}

Http::Http1Parse::~Http1Parse()
{
    this->header_value.clear();
    this->header_field.clear();
}

bool Http::Http1Parse::parseData(const Char* data, Size_t len)
{
    auto parse_len = http_parser_execute(&this->parser_, &this->setting_, data, len);
    if (HTTP_PARSER_ERRNO(&this->parser_) == HPE_OK){
            return true;
    }
    return false;
}

Int Http::Http1Parse::getStatus()
{
    return this->status_;
}

bool Http::Http1Parse::needReceive()
{
    return (this->status_ != HP_PARSE_END);
}

bool Http::Http1Parse::needSend()
{
    return (this->status_ == HP_PARSE_END);
}

Int Http::Http1Parse::initRequest()
{
    return 0;
}

Int Http::Http1Parse::initResponse() {
    return 0;
}

void Http::Http1Parse::resetParser()
{
    std::memset(&this->parser_, 0, sizeof(this->parser_));
}
