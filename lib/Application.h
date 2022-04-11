/**
* FileName:   Application.h
* CreateDate: 2022-04-09 13:53:57
* Author:     ticks
* Email:      2938384958@qq.com
* Des:
*/
#ifndef SINBACK_TEST_APPLICATION_H
#define SINBACK_TEST_APPLICATION_H

#include "core/EventLoopPool.h"
#include "base/System.h"
#include "module/Module.h"

namespace SinBack
{
    namespace Main
    {
        // App类
        class Application final : noncopyable
        {
        public:
            using string_type = SinBack::String;
            struct Setting{
                // 工作线程数量，默认为 核心数 * 2
                Size_t workThreadNum;
                // 工作进程数量（线程和进程同时指定会优先选择进程模式）
                Size_t workProcessNum;
                // 最大连接客户端数量， 默认 4096
                Size_t maxConnectCnt;
                // 日志文件存放目录，默认执行文件所在目录
                string_type logPath;
                // 监听端口
                Size_t listenPort;
                // 启用SSL
                bool enableSSL;
                // 证书和私钥路径
                string_type certPath;
                string_type keyPath;
                //

            };
        public:
            Application();
            ~Application();

            // 获取 setting
            Setting& setting(){
                return setting_;
            }
            // 开始运行
            bool run(const std::function<void(const String &)> &err_callback = nullptr);
            // 停止
            void stop();

            // 设置模块
            void setModule(std::shared_ptr<Module::BaseModule>&& module){
                this->module_ = std::move(module);
            }

        private:
            // 创建 IPV4 套接字并监听
            static Base::socket_t createListenSocketV4(UInt port, bool reuse_port = false);

            // 初始化SSL
            bool initSSL();
            // 内部运行函数
            void start();
            // 开始监听并 accept
            void startListenAccept(Core::EventLoopPtr loop, bool reuse_port = false);

            // 有新连接回调
            void onNewClient(const std::weak_ptr<Core::IOEvent>& ev);
            // 有新消息回调
            void onNewMessage(const std::weak_ptr<Core::IOEvent>& ev, const string_type& read_msg);
            // 读取错误
            void onMessageError(const std::weak_ptr<Core::IOEvent>& ev, const string_type& err_msg);
            // 发送数据回调
            void onSend(const std::weak_ptr<Core::IOEvent>& ev, Size_t write_len);
            // 发送数据失败
            void onSendError(const std::weak_ptr<Core::IOEvent>& ev, const string_type& err_msg);
            // 客户端关闭
            void onDisconnect(const std::weak_ptr<Core::IOEvent>& ev);

            void incConnectCount(){
                ++this->connect_cnt_;
            }
            void decConnectCount(){
                --this->connect_cnt_;
            }
            Size_t getConnectCount() const{
                return this->connect_cnt_;
            }
            // 线程模式
            void runThreadMode();
            // 进程模式
            void runProcessMode();

            // 处理子进程退出
            void sigWaitChild(int sig);
            // 处理退出信号(优雅退出)
            void sigHandleExit(int sig);
        private:
            // 是否运行
            bool running_;
            // 设置
            Setting setting_;
            // 当前连接数量
            std::atomic<Size_t> connect_cnt_;
            // 存储各个子模块
            SharedPtr<Module::BaseModule> module_;
            // accept 线程
            std::shared_ptr<Core::EventLoopThread> accept_th_;
            // 工作线程
            std::shared_ptr<Core::EventLoopPool> work_th_;
            // 互斥锁
            std::mutex mutex_;
            // 与子进程通信管道
            std::vector<pid_t> child_pid_;
            // 线程tid
            pid_t tid_;
            // 条件变量，用于阻塞进程
            std::condition_variable wait_cv_;
        };
    }
}

#endif //SINBACK_TEST_APPLICATION_H
