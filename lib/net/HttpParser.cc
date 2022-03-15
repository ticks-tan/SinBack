/**
* FileName:   HttpParser.cc
* CreateDate: 2022-03-13 15:04:35
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "HttpParser.h"

using namespace SinBack;

Int on_url(llhttp_t* parser, const Char* data, Size_t len);
Int on_status(llhttp_t* parser, const Char* data, Size_t len);
Int on_header_field(llhttp_t* parser, const Char* data, Size_t len);
Int on_header_value(llhttp_t* parser, const Char* data, Size_t len);
Int on_body(llhttp_t* parser, const Char* data, Size_t len);

Int on_message_begin(llhttp_t* parse);
Int on_headers_complete(llhttp_t* parser);
Int on_message_complete(llhttp_t* parser);
Int on_chunk_header(llhttp_t* parser);
Int on_chunk_complete(llhttp_t* parser);


Int on_url(llhttp_t* parser, const Char* data, Size_t len)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->request_->url.append(data, len);
        hp->set_status(Http::HP_PARSE_URL);
        return 0;
    }
    return 1;
}
Int on_status(llhttp_t* parser, const Char* data, Size_t len)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->set_status(Http::HP_PARSE_STATUS);
        return 0;
    }
    return 1;
}
Int on_header_field(llhttp_t* parser, const Char* data, Size_t len)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->header_field.append(data, len);
        if (!hp->header_field.empty() && !hp->header_value.empty()){
            hp->request_->header.set_head(hp->header_field, hp->header_value);
            hp->header_field.clear();
            hp->header_value.clear();
        }
        hp->set_status(Http::HP_PARSE_HEADER_FIELD);
        return 0;
    }
    return 1;
}
Int on_header_value(llhttp_t* parser, const Char* data, Size_t len)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->header_value.append(data, len);
        if (!hp->header_field.empty() && !hp->header_value.empty()){
            hp->request_->header.set_head(hp->header_field, hp->header_value);
            hp->header_field.clear();
            hp->header_value.clear();
        }
        hp->set_status(Http::HP_PARSE_HEADER_VALUE);
        return 0;
    }
    return 1;
}
Int on_body(llhttp_t* parser, const Char* data, Size_t len)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->set_status(Http::HP_PARSE_BODY);
        hp->request_->content.data().append(data, len);
        return 0;
    }
    return 1;
}

Int on_message_begin(llhttp_t* parser)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->set_status(Http::HP_PARSE_BEGIN);
        return 0;
    }
    return 1;
}
Int on_headers_complete(llhttp_t* parser)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->set_status(Http::HP_PARSE_HEADERS_END);
        return 0;
    }
    return 1;
}
Int on_message_complete(llhttp_t* parser)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->request_->method = (Http::HttpMethod)parser->method;
        hp->set_status(Http::HP_PARSE_END);
        return 0;
    }
    return 1;
}
Int on_chunk_header(llhttp_t* parser)
{
    auto* hp = (Http::Http1Parse*)(parser->data);
    if (hp){
        hp->set_status(Http::HP_PARSE_CHUNK_HEADER);
        return 0;
    }
    return 1;
}
Int on_chunk_complete(llhttp_t* parser)
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
    llhttp_settings_init(&this->setting_);
    llhttp_init(&this->parser_, HTTP_REQUEST, &this->setting_);
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

Int Http::Http1Parse::parse_data(const Char* data, Size_t len)
{
    return llhttp_execute(&this->parser_, data, len);
}

Int Http::Http1Parse::get_status()
{
    return this->status_;
}

bool Http::Http1Parse::need_receive()
{
    return (this->status_ != HP_PARSE_END);
}

bool Http::Http1Parse::need_send()
{
    return (this->status_ == HP_PARSE_END);
}

Int Http::Http1Parse::init_request()
{
    return 0;
}

Int Http::Http1Parse::init_response() {
    return 0;
}
