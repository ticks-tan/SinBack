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

Module::TcpServer::TcpServer()
{
}

Module::TcpServer::~TcpServer()
{
    this->channels_.clear();
}

Module::TcpServer::ChannelPtr
Module::TcpServer::addChannel(const std::weak_ptr<Core::IOEvent>& ev) {
    std::shared_ptr<Core::IOEvent> io = ev.lock();
    UInt id = io->id_;
    auto channel = std::make_shared<Core::Channel>(io);
    std::unique_lock<std::mutex> lock(mutex_);
    this->channels_[id] = channel;
    return this->channels_[id];
}

void
Module::TcpServer::removeChannel(const TcpServer::ChannelPtr &channel)
{
    auto id = channel->getId();
    std::unique_lock<std::mutex> lock(this->mutex_);
    this->channels_.erase(id);
}

Module::TcpServer::ChannelPtr
Module::TcpServer::getChannel(SinBack::Base::socket_t id) {
    auto it = this->channels_.find(id);
    std::unique_lock<std::mutex> lock(this->mutex_);
    return (it != this->channels_.end()) ? it->second : nullptr;
}

bool
Module::TcpServer::onNewClient(Core::IOEvent& io)
{
    auto* server = static_cast<TcpServer*>(io.context_);
    if (server) {
        const ChannelPtr channel = server->addChannel(io.shared_from_this());

        if (server->onNewClient_) {
            server->onNewClient_(channel);
        }
        channel->read();
        return true;
    }
    return false;
}

bool Module::TcpServer::onNewMessage(Core::IOEvent &io, const String &read_msg)
{
    auto* server = static_cast<TcpServer*>(io.context_);
    if (server) {
        auto channel = server->getChannel(io.fd_);
        if (channel && server->onMessage_) {
            server->onMessage_(channel, read_msg);
        }
        return true;
    }
    return false;
}

bool Module::TcpServer::onMessageError(Core::IOEvent &io, const String &err_msg) {
    auto* server = static_cast<TcpServer*>(io.context_);
    if (server) {
        auto channel = server->getChannel(io.fd_);
        if (channel && server->onErrorMessage_) {
            server->onErrorMessage_(channel, err_msg);
        }
        return true;
    }
    return false;
}

bool Module::TcpServer::onSend(Core::IOEvent &io, Size_t write_len)
{
    auto* server = static_cast<TcpServer*>(io.context_);
    if (server) {
        auto channel = server->getChannel(io.fd_);
        if (channel && server->onWrite_) {
            server->onWrite_(channel, write_len);
        }
        return true;
    }
    return false;
}

bool Module::TcpServer::onSendError(Core::IOEvent &io, const String &err_msg)
{
    auto* server = static_cast<TcpServer*>(io.context_);
    if (server) {
        auto channel = server->getChannel(io.fd_);
        if (channel && server->onErrorWrite_) {
            server->onErrorWrite_(channel, err_msg);
        }
        return true;
    }
    return false;
}

bool Module::TcpServer::onDisconnect(Core::IOEvent &io)
{
    auto* server = static_cast<TcpServer*>(io.context_);
    if (server) {
        auto channel = server->getChannel(io.fd_);
        if (channel && server->onClose_) {
            server->onClose_(channel);
        }
        server->removeChannel(channel);
        return true;
    }
    return false;
}
