/**
* FileName:   HttpServer.h
* CreateDate: 2022-03-14 17:49:22
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_HTTPSERVER_H
#define SINBACK_HTTPSERVER_H

#include "base/noncopyable.h"
#include "core/EventLoopPool.h"
#include "HttpService.h"
#include "HttpContext.h"

namespace SinBack {

    namespace Http {

        // Http服务端
        class HttpServer : noncopyable
        {
        public:
            // Http服务设置
            struct Setting
            {
                // 工作线程数量，默认为 核心数 * 2
                Size_t workThreadNum;
                // 日志文件存放目录，默认执行文件所在目录
                String logPath;
                // 最大连接客户端数量， 默认4096
                Size_t maxAcceptCnt;
                // 是否启用 keep-alive，默认关闭
                bool keepAlive;
                // 静态文件路径, 不指定即为当前工作目录
                String staticFileDir;
            };
            static const UInt default_keep_alive = 60000;
        public:
            HttpServer();
            ~HttpServer();

            // 获取 Setting 引用
            Setting& setting(){
                return this->setting_;
            }
            // 添加 service,提供对应名字
            bool addService(const String& name, HttpService* service);

            // 开始监听
            bool listen(UInt port, const std::function<void(const String& err)>& err_callback = nullptr);

            // 停止 HttpServer
            void stop();

            // 获取当前连接客户端数量
            Size_t getConnectCount() const{
                return this->connect_cnt_;
            }

            // 获取 services
            std::unordered_map<SinBack::String, HttpService*>& services(){
                return this->services_;
            }

            // 获取一个工作 loop
            Core::EventLoopPtr loop(Int index = -1){
                std::unique_lock<std::mutex> lock(this->mutex_);
                Core::EventLoopPtr ptr = this->work_th_->loop(index).get();
                lock.unlock();
                return ptr;
            }

            // 当前连接客户端数量 +1
            void incConnectCount(){
                ++this->connect_cnt_;
            }
            // 当前连接客户端数量 -1
            void decConnectCount(){
                --this->connect_cnt_;
            }

        private:
            // 初始化 HttpServer
            void init();
            // 创建监听套接字
            bool createListenSocketV4(UInt port);

            // 开始运行
            void start();

            // 设置keepAlive
            static void setIOKeepAlive(const std::weak_ptr<Core::IOEvent>& ev);

            // 开始 accept
            void startAccept();
            // 有新连接回调
            void onNewClient(const std::weak_ptr<Core::IOEvent>& ev);
            // 有新消息回调
            void onNewMessage(const std::weak_ptr<Core::IOEvent>& ev, const String& read_msg);
            // 读取错误
            void onMessageError(const std::weak_ptr<Core::IOEvent>& ev, const String& err_msg);
            // 发送数据回调
            void onSend(const std::weak_ptr<Core::IOEvent>& ev, Size_t write_len);
            // 发送数据失败
            void onSendError(const std::weak_ptr<Core::IOEvent>& ev, const String& err_msg);
            // 客户端关闭
            void onDisconnect(const std::weak_ptr<Core::IOEvent>& ev);
            // 发送http回应数据
            void sendHttpResponse(const std::shared_ptr<Core::IOEvent>& io, HttpContext* context, Int call_ret);

            // 静态文件
            void sendStaticFile(const std::shared_ptr<Core::IOEvent>& io, HttpContext* context, String& path) const;
        private:
            // 设置
            Setting setting_;
            // 多个服务
            std::unordered_map<String, HttpService*> services_;
            // accept 线程
            std::shared_ptr<Core::EventLoopThread> accept_th_;
            // 工作线程
            std::shared_ptr<Core::EventLoopPool> work_th_;
            // 监听套接字
            Base::socket_t listen_fd_;
            // 是否运行
            bool running_;
            // 记录当前连接客户端数量
            std::atomic<Size_t> connect_cnt_{};
            std::mutex mutex_;
        };
    }
}


#endif //SINBACK_HTTPSERVER_H
