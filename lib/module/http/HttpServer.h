/**
* FileName:   HttpServer.h
* CreateDate: 2022-03-14 17:49:22
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_HTTPSERVER_H
#define SINBACK_HTTPSERVER_H

#include "module/Module.h"
#include "HttpService.h"
#include "HttpContext.h"

namespace SinBack {

    namespace Module {

        namespace Http {

            // Http服务端
            class HttpServer : public Module::BaseModule {
            public:
                using string_type = SinBack::String;
                // Http服务设置
                struct Setting {
                    // 是否启用 keep-alive，默认关闭
                    bool keepAlive;
                    // 静态文件路径, 不指定即为当前工作目录
                    String staticFileDir;
                };
                // 默认 keep-alive 超时时间为60s
                static const UInt default_keep_alive = 60000;
            public:
                HttpServer();

                ~HttpServer();

                // 获取 Setting 引用
                Setting &setting() {
                    return this->setting_;
                }

                // 添加 service,提供对应名字
                bool addService(const String &name, HttpService *service);

                // 获取 services
                std::unordered_map<string_type, HttpService *> &services() {
                    return this->services_;
                }

            private:

                // 设置keepAlive
                static void setIOKeepAlive(const std::weak_ptr<Core::IOEvent> &ev);

                // 有新连接回调
                bool onNewClient(Core::IOEvent &ev) override;

                // 有新消息回调
                bool onNewMessage(Core::IOEvent &ev, const String &read_msg) override;

                // 读取错误
                bool onMessageError(Core::IOEvent &ev, const String &err_msg) override;

                // 发送数据回调
                bool onSend(Core::IOEvent &ev, Size_t write_len) override;

                // 发送数据失败
                bool onSendError(Core::IOEvent &ev, const String &err_msg) override;

                // 客户端关闭
                bool onDisconnect(Core::IOEvent &ev) override;

                // 发送http回应数据
                static void
                sendHttpResponse(const std::shared_ptr<Core::IOEvent> &io, HttpContext *context, Int call_ret);

                // 发送静态文件
                void sendStaticFile(Core::IOEvent &io, HttpContext *context, String &path) const;

            private:
                // 设置
                Setting setting_;
                // 多个服务
                std::unordered_map<string_type, HttpService *> services_;
                // 互斥锁
                std::mutex mutex_;
            };
        }
    }
}


#endif //SINBACK_HTTPSERVER_H
