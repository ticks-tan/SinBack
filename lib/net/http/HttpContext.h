/**
* FileName:   HttpContext.h
* CreateDate: 2022-03-14 16:19:25
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_HTTPCONTEXT_H
#define SINBACK_HTTPCONTEXT_H

#include "base/File.h"
#include "HttpParser.h"

namespace SinBack
{
    namespace Http
    {
        class HttpServer;
        class HttpContext
        {
        public:

            HttpContext(HttpServer* server);
            ~HttpContext();

            // 发送文本数据
            Int sen_text(const SinBack::String& text);
            // 发送文件
            Int sen_file(const SinBack::String& file_name);
            // 解析Url
            bool parse_url();
            // 解析请求数据
            bool parse_body();
            // 初始化
            bool init();
            // 获取请求
            HttpRequest& request(){
                return this->request_;
            }
            // 获取回应
            HttpResponse& response(){
                return this->response_;
            }
            // 获取解析器
            HttpParser* parser(){
                return this->parser_.get();
            }

            HttpServer* server(){
                return this->server_;
            }
        private:
            // Http解析器
            std::shared_ptr<HttpParser> parser_;
            // Http请求
            HttpRequest request_;
            // Http回应
            HttpResponse response_;
            // 对应 HttpServer 指针
            HttpServer* server_;
        };
    }
}


#endif //SINBACK_HTTPCONTEXT_H
