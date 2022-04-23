/**
* FileName:   TcpServer.cc
* CreateDate: 2022-03-11 15:43:54
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include <cstring>
#include "TcpServer.h"

using namespace SinBack;

Module::TcpServer::TcpServer() = default;

Module::TcpServer::~TcpServer() = default;

bool
Module::TcpServer::onNewClient(Core::IOEvent& io)
{
  auto* server = this;
  auto channel = new Core::Channel(io.shared_from_this());
  if (!io.context_){
    io.context_ = channel;
    if (server->onNewClient_){
          server->onNewClient_(channel);
    }
  }
  io.read();
  return true;
}

bool Module::TcpServer::onNewMessage(Core::IOEvent &io, const String &read_msg)
{
    auto* channel = static_cast<ChannelPtr>(io.context_);
    if (channel && this->onMessage_) {
        this->onMessage_(channel, read_msg);
        return true;
    }
    return false;
}

bool Module::TcpServer::onMessageError(Core::IOEvent &io, const String &err_msg) {
    auto* channel = static_cast<ChannelPtr>(io.context_);
    if (channel && this->onErrorMessage_) {
        this->onErrorMessage_(channel, err_msg);
        return true;
    }
    return false;
}

bool Module::TcpServer::onSend(Core::IOEvent &io, Size_t write_len)
{
    auto* channel = static_cast<ChannelPtr>(io.context_);
    if (channel && this->onWrite_) {
        this->onWrite_(channel, write_len);
        return true;
    }
    return false;
}

bool Module::TcpServer::onSendError(Core::IOEvent &io, const String &err_msg)
{
    auto* channel = static_cast<ChannelPtr>(io.context_);
    if (channel && this->onErrorWrite_) {
        this->onErrorWrite_(channel, err_msg);
        return true;
    }
    return false;
}

bool Module::TcpServer::onDisconnect(Core::IOEvent &io)
{
    auto* channel = static_cast<ChannelPtr>(io.context_);
    if (channel) {
        if (this->onClose_){
            this->onClose_(channel);
        }
        delete channel;
        io.context_ = nullptr;
        return true;
    }
    return false;
}
