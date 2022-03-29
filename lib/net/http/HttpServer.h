/**
* FileName:   HttpServer.h
* CreateDate: 2022-03-14 17:49:22
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_HTTPSERVER_H
#define SINBACK_HTTPSERVER_H

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
                // 工作进程数量（线程和进程同时指定会优先选择进程模式）
                Size_t workProcessNum;
                // 日志文件存放目录，默认执行文件所在目录
                String logPath;
                // 最大连接客户端数量， 默认 4096
                Size_t maxAcceptCnt;
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

#ifdef OS_LINUX
            // 子进程退出信号处理
            static void processExitCall(Int sig);
#endif
            // 创建监听套接字
            static Base::socket_t createListenSocketV4(UInt port);
            // 设置keepAlive
            static void setIOKeepAlive(const std::weak_ptr<Core::IOEvent>& ev);
            // 开始运行
            void start();
            // 开始 accept
            void startListenAccept(Core::EventLoopPtr loop);
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
            static void sendHttpResponse(const std::shared_ptr<Core::IOEvent>& io, HttpContext* context, Int call_ret);
            // 发送静态文件
            void sendStaticFile(const std::shared_ptr<Core::IOEvent>& io, HttpContext* context, String& path) const;
        private:
            // 线程执行函数
            void runThreadFunc();
            // 进程执行函数
            void runProcessFunc();
            // 设置
            Setting setting_;
            // 多个服务
            std::unordered_map<String, HttpService*> services_;
            // accept 线程
            std::shared_ptr<Core::EventLoopThread> accept_th_;
            // 工作线程
            std::vector<std::shared_ptr<Core::EventLoopThread>> work_th_;
            // 监听端口
            Size_t listen_port_;
            // 是否运行
            bool running_;
            // 记录当前连接客户端数量
            std::atomic<Size_t> connect_cnt_{};
            // 互斥锁
            std::mutex mutex_;
            // 与子进程通信管道
            std::vector<Base::socket_t> pipes_;
            // 线程tid
            pid_t tid_;
        };
    }
}


#endif //SINBACK_HTTPSERVER_H
