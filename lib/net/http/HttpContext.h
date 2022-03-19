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
            static const String notfound_str;
            static const String error_str;
        public:
            explicit HttpContext(HttpServer* server);
            ~HttpContext();

            // 发送文本数据
            Int sendText(const SinBack::String& text);
            // 发送文件
            Int sendFile(const SinBack::String& file_name);
            // 回应404
            Int notFound();
            // 回应错误
            Int error();
            // 解析Url
            bool parseUrl();
            // 解析请求数据
            bool parseBody();
            // 初始化
            bool init();

            HttpMethod getMethod() const{
                return this->request_.method;
            }
            Int getStatusCode() const{
                return this->response_.status_code;
            }
            void setStatusCode(Int code){
                if (code > 0) {
                    this->response_.status_code = code;
                }
            }
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

            void clear() {
                this->request_.clear();
                this->response_.clear();
            }

            std::unordered_map<String, String>& urlParams(){
                if (this->url_params_ == nullptr){
                    this->url_params_ = new std::unordered_map<String, String>;
                }
                return *this->url_params_;
            }

        public:
            // Cache File
            std::shared_ptr<Base::File> cache_file_;
        private:
            // Http解析器
            std::shared_ptr<HttpParser> parser_;
            // Http请求
            HttpRequest request_;
            // Http回应
            HttpResponse response_;
            // 对应 HttpServer 指针
            HttpServer* server_;
            // URL 解析结果
            std::unordered_map<String, String>* url_params_;
        };
    }
}


#endif //SINBACK_HTTPCONTEXT_H
