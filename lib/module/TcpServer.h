/**
* FileName:   TcpServer.h
* CreateDate: 2022-03-11 15:43:54
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        TCP 服务端
*/
#ifndef SINBACK_TCPSERVER_H
#define SINBACK_TCPSERVER_H

#include "module/Module.h"
#include "core/Channel.h"

namespace SinBack
{
    namespace Module
    {
        class TcpServer final : public BaseModule
        {
        public:
            using string_type = SinBack::String;
            using ChannelPtr = Core::Channel*;
         public:
            // 新客户端连接时调用
            bool onNewClient(Core::IOEvent &io) override;
            // 新消息回调
            bool onNewMessage(Core::IOEvent& io, const String& read_msg) override;
            // 读取错误
            bool onMessageError(Core::IOEvent& io, const String& err_msg) override;
            // 发送数据回调
            bool onSend(Core::IOEvent& ev, Size_t write_len) override;
            // 发送数据失败
            bool onSendError(Core::IOEvent& ev, const String& err_msg) override;
            // 客户端关闭
            bool onDisconnect(Core::IOEvent& ev) override;

         public:
            void setOnNewClient(std::function<void(const ChannelPtr&)>&& func){
                this->onNewClient_ = std::move(func);
            }
            void setOnNewMessage(std::function<void(const ChannelPtr&, const string_type&)>&& func){
                this->onMessage_ = std::move(func);
            }
            void setOnMessageError(std::function<void(const ChannelPtr&, const string_type&)>&& func){
                this->onErrorMessage_ = std::move(func);
            }
            void setOnSend(std::function<void(const ChannelPtr&, Size_t)>&& func){
                this->onWrite_ = std::move(func);
            }
            void setOnSendError(std::function<void(const ChannelPtr&, const string_type&)>&& func){
                this->onErrorWrite_ = std::move(func);
            }
            void setDisconnect(std::function<void(const ChannelPtr&)>&& func){
                this->onClose_ = std::move(func);
            }
          TcpServer();
          ~TcpServer();

        private:
            // 新客户端连接
            std::function<void(const ChannelPtr&)> onNewClient_;
            // 有新消息
            std::function<void(const ChannelPtr&, const string_type&)> onMessage_;
            // 读取消息错误
            std::function<void(const ChannelPtr&, const string_type&)> onErrorMessage_;
            // 写入消息完成
            std::function<void(const ChannelPtr&, Size_t)> onWrite_;
            // 写入消息错误
            std::function<void(const ChannelPtr&, const string_type&)> onErrorWrite_;
            // 关闭
            std::function<void(const ChannelPtr&)> onClose_;
        };
    }
}



#endif //SINBACK_TCPSERVER_H
