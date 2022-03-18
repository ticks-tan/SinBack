/**
* FileName:   HttpService.h
* CreateDate: 2022-03-14 16:22:58
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_HTTPSERVICE_H
#define SINBACK_HTTPSERVICE_H

#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include <unordered_map>
#include "base/Base.h"

namespace SinBack {
    namespace Http {
        // 前置声明
        class HttpContext;

        enum ServiceCallBackRet
        {
            NEXT = 0,
            END = 1
        };

        struct HttpServiceCall
        {
            using Call = std::function<Int(HttpContext&)>;
            // 请求方法
            Int method;
            // 回调函数
            Call callback;

            HttpServiceCall(Int method_, Call  call_)
                : method(method_)
                , callback(std::move(call_)){}
        };
        class HttpService
        {
        public:
            // 拦截get请求
            void GET(const String& path, const HttpServiceCall::Call& call);
            // 拦截post请求
            void POST(const String& path, const HttpServiceCall::Call& call);
            // 拦截post请求
            void DELETE(const String& path, const HttpServiceCall::Call& call);
            // 拦截post请求
            void HEAD(const String& path, const HttpServiceCall::Call& call);
            // 拦截post请求
            void PUT(const String& path, const HttpServiceCall::Call& call);
            // 拦截其他请求
            void On(Int method, const String& path, const HttpServiceCall::Call& call);
            // 匹配服务
            std::vector<std::shared_ptr<HttpServiceCall>> matchService(const String& path, Int method);
            // 清空服务
            void clearService();
        private:
            // 添加服务
            void addService(const String& path, Int method, const HttpServiceCall::Call& call);
            // 删除服务
            void removeService(const String& path, Int method);

        private:
            std::unordered_map<String, std::shared_ptr<HttpServiceCall>> services_;
        };
    }
}


#endif //SINBACK_HTTPSERVICE_H
