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
#include "Json.h"

namespace SinBack
{
    namespace Module {
        namespace Http {
            class HttpServer;

            class HttpContext {
            public:
                using Json = nlohmann::json;
                using string_type = SinBack::String;
            public:
                static const string_type notfound_str;
                static const string_type error_str;
            public:
                explicit HttpContext(HttpServer *server);

                ~HttpContext();
                // 发送文本数据
                Int sendText(const string_type &text);
                // 发送文件
                Int sendFile(const string_type &file_name);
                // 回应404
                Int notFound();
                // 回应错误
                Int error();
                // 解析Url
                bool parseUrl();
                // 初始化
                bool init();

                // 解析 Json
                bool parseJson();

                // 获取Json
                Json& json(){
                    if (!this->json_){
                        this->json_ = std::make_shared<Json>();
                    }
                    return *this->json_;
                }

                bool isInit() const {
                    return this->init_;
                }
                HttpMethod getMethod() const {
                    return this->request_.method;
                }

                Int getStatusCode() const {
                    return this->response_.status_code;
                }
                void setStatusCode(Int code) {
                    if (code > 0) {
                        this->response_.status_code = code;
                    }
                }

                // 获取请求
                HttpRequest &request() {
                    return this->request_;
                }

                // 获取回应
                HttpResponse &response() {
                    return this->response_;
                }

                // 获取解析器
                HttpParser *parser() {
                    return this->parser_.get();
                }

                HttpServer *server() {
                    return this->server_;
                }

                void clear() {
                    this->request_.clear();
                    this->response_.clear();
                    if (this->url_params_) {
                        this->url_params_->clear();
                    }
                    this->init_ = false;
                }
                // 获取 URL 请求参数
                std::unordered_map<string_type, string_type> &urlParams() {
                    if (this->url_params_ == nullptr) {
                        this->url_params_ = std::make_shared<std::unordered_map<string_type, string_type>>();
                    }
                    return *this->url_params_;
                }

            public:
                // Cache File
                std::shared_ptr<Base::File> cache_file_;
            private:
                // Json
                std::shared_ptr<Json> json_;
                // Http解析器
                std::shared_ptr<HttpParser> parser_;
                // Http请求
                HttpRequest request_;
                // Http回应
                HttpResponse response_;
                // 对应 HttpServer 指针
                HttpServer *server_;
                // URL 解析结果
                std::shared_ptr<std::unordered_map<string_type, string_type>> url_params_;
                // 是否初始化
                bool init_;
            };
        }
    }
}


#endif //SINBACK_HTTPCONTEXT_H
