/**
* FileName:   TcpServer.h
* CreateDate: 2022-03-11 15:43:54
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        TCP 服务端
*/
#ifndef SINBACK_TCPSERVER_H
#define SINBACK_TCPSERVER_H

#include "core/EventLoopPool.h"
#include "core/Channel.h"

namespace SinBack
{
    namespace Net
    {
        class TcpServer
        {
        public:
            using EventLoopPtr = std::shared_ptr<SinBack::Core::EventLoop>;
            using ChannelPtr = std::shared_ptr<SinBack::Core::Channel>;
            static const UInt default_max_accept_count = 4096;

            // 新客户端连接
            std::function<void(const ChannelPtr&)> on_new_client;
            // 有新消息
            std::function<void(const ChannelPtr&, const String&)> on_message;
            // 读取消息错误
            std::function<void(const ChannelPtr&, const String&)> on_error_message;
            // 写入消息完成
            std::function<void(const ChannelPtr&, Size_t)> on_write;
            // 写入消息错误
            std::function<void(const ChannelPtr&, const String&)> on_error_write;
            // 关闭
            std::function<void(const ChannelPtr&)> on_close;
        public:
            TcpServer();
            ~TcpServer();
            // 添加channel
            ChannelPtr add_channel(const std::weak_ptr<Core::IOEvent>& io);
            // 移除channel
            void remove_channel(const ChannelPtr& channel);
            // 获取channel
            ChannelPtr get_channel(Base::socket_t id);

            // 开始运行
            bool run(UInt port);
            // 获取事件循环
            EventLoopPtr loop(Int index = -1);

            Size_t accept_count() const{
                return this->channels_.size();
            }
        private:
            Base::socket_t create_listen_fd(UInt port);
            // 开始接受连接
            void start_accept();
            // 新客户端连接
            static void new_client(const std::weak_ptr<Core::IOEvent>& io);
        private:
            // 用于接受新连接
            Core::EventLoopThread accept_loop_;
            // 监听套接字
            Base::socket_t listen_fd_;
            // 监听端口
            UInt port_;
            // 用于其他网络IO
            Core::EventLoopPool work_loops_;
            // 工作线程数量
            UInt work_thread_cnt;
            // 存储Channel
            std::unordered_map<UInt, ChannelPtr> channels_;
            // 互斥锁
            std::mutex mutex;
        };
    }
}



#endif //SINBACK_TCPSERVER_H
