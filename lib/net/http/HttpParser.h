/**
* FileName:   HttpParser.h
* CreateDate: 2022-03-13 15:04:35
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_HTTPPARSER_H
#define SINBACK_HTTPPARSER_H

#include <memory>
#include "llhttp/llhttp.h"
#include "HttpMessage.h"

namespace SinBack
{
    namespace Http
    {
        enum HttpParserStatus
        {
            HP_NULL = 0,
            HP_PARSE_BEGIN = 1,
            HP_PARSE_URL = 2,
            HP_PARSE_STATUS = 3,
            HP_PARSE_HEADER_FIELD = 4,
            HP_PARSE_HEADER_VALUE = 5,
            HP_PARSE_HEADERS_END = 6,
            HP_PARSE_CHUNK_HEADER = 7,
            HP_PARSE_BODY = 8,
            HP_PARSE_CHUNK_END = 9,
            HP_PARSE_END = 10
        };
        class HttpParser {
        public:
            HttpVersion version = HTTP_1_1;
            // static std::shared_ptr<HttpParser> make_http_parser(HttpVersion version = HTTP_1_1);

            virtual ~HttpParser() = default;
            // 解析数据
            virtual Int parse_data(const Char* data, Size_t len) = 0;
            // 重置解析器
            virtual void reset_parser() = 0;
            // 获取状态
            virtual Int get_status() = 0;
            // 是否需要获取数据
            virtual bool need_receive() = 0;
            // 是否需要发送数据
            virtual bool need_send() = 0;

            virtual Int init_request() = 0;
            virtual Int init_response() = 0;

        };

        class Http1Parse : public HttpParser
        {
        public:

            Http1Parse(HttpRequest* req, HttpResponse* resp);
            Http1Parse() = delete;
            ~Http1Parse() override;

            Int parse_data(const Char* data, Size_t len) override;
            void reset_parser() override;
            Int get_status() override;
            bool need_receive() override;
            bool need_send() override;

            Int init_request() override;
            Int init_response() override;

            void set_status(HttpParserStatus status){
                this->status_ = status;
            }

        public:
            HttpRequest* request_;
            HttpResponse* response_;

            String header_field;
            String header_value;
        private:
            llhttp_t parser_;
            HttpParserStatus status_;
            llhttp_settings_t setting_;
        };
    }
}



#endif //SINBACK_HTTPPARSER_H
