/**
* FileName:   TcpServer.cc
* CreateDate: 2022-03-11 15:43:54
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include <cstring>
#include "TcpServer.h"
#include "base/System.h"

using namespace SinBack;

SinBack::Net::TcpServer::TcpServer()
    : work_thread_cnt(std::thread::hardware_concurrency())
    , listen_fd_(-1)
    , port_(0), running_(false)
{
}

SinBack::Net::TcpServer::~TcpServer()
{
    if (this->running_) {
        this->stop(true);
    }
}

void Net::TcpServer::stop(bool join)
{
    if (this->running_) {
        this->accept_loop_.stop(join);
        if (this->work_thread_cnt > 0) {
            this->work_loops_.stop(join);
        }
        this->cv_.notify_all();
    }
}

// 处理退出信号
void Net::TcpServer::sigStop(Int sig)
{
    if (this->running_){
        this->stop(true);
    }
}


SinBack::Net::TcpServer::ChannelPtr
SinBack::Net::TcpServer::addChannel(const std::weak_ptr<Core::IOEvent>& ev) {
    std::shared_ptr<Core::IOEvent> io = ev.lock();
    UInt id = io->id_;
    auto channel = std::make_shared<Core::Channel>(io);
    std::unique_lock<std::mutex> lock(mutex);
    this->channels_[id] = channel;
    return this->channels_[id];
}

void
SinBack::Net::TcpServer::removeChannel(const SinBack::Net::TcpServer::ChannelPtr &channel)
{
    auto id = channel->getId();
    std::unique_lock<std::mutex> lock(this->mutex);
    this->channels_.erase(id);
}

SinBack::Net::TcpServer::ChannelPtr
SinBack::Net::TcpServer::getChannel(SinBack::Base::socket_t id) {
    auto it = this->channels_.find(id);
    std::unique_lock<std::mutex> lock(this->mutex);
    return (it != this->channels_.end()) ? it->second : nullptr;
}

bool
SinBack::Net::TcpServer::run(UInt port)
{
    if (this->running_) return false;
    this->running_ = true;

    // 屏蔽 Ctrl + C 信号
    Base::system_ignore_sig(SIGINT);
    // 屏蔽 PIPE 错误
    Base::system_ignore_sig(SIGPIPE);
    // 设置 TERM 信号处理， 优雅退出
    Base::system_signal(SIGTERM, std::bind(&TcpServer::sigStop, this, std::placeholders::_1));

    this->listen_fd_ = createListenFd(port);
    if (this->listen_fd_ == -1){
        perror("create listen socket");
        return false;
    }

    if (this->work_thread_cnt > 0){
        this->work_loops_.start();
    }
    this->accept_loop_.start(std::bind(&TcpServer::startAccept, this), nullptr);

    // 等待程序退出
    std::unique_lock<std::mutex> lock(this->mutex);
    this->cv_.wait(lock, [this](){
        return !this->running_;
    });

    return true;
}

SinBack::Net::TcpServer::EventLoopPtr
SinBack::Net::TcpServer::loop(Int index) {
    return this->work_loops_.loop(index);
}

void
SinBack::Net::TcpServer::startAccept()
{
    auto acceptor = this->accept_loop_.loop()->acceptIo(this->listen_fd_, newClient);
    acceptor->context_ = this;
}

void
SinBack::Net::TcpServer::newClient(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io){
        auto* server = (TcpServer*)io->context_;
        if (server) {
            if (server->acceptCount() > default_max_accept_count) {
                Log::FLogW("accept count over max !");
                io->loop_->closeIo(io->fd_);
                return;
            }
            EventLoopPtr work_loop = server->loop();
            if (work_loop) {
                // 由于work loop与 accept loop不同，需要对 io事件设置新的 loop
                Core::EventLoop::changeIoLoop(io, work_loop.get());
            }
            const ChannelPtr channel = server->addChannel(io);
            Base::setSocketNonblock(channel->getFd());
            // 设置读取回调
            channel->setReadCb([server, &channel](const string_type &buf) {
                if (server->onMessage) {
                    server->onMessage(channel, buf);
                }
            });
            // 设置读取错误回调
            channel->setReadErrCb([server, &channel](const string_type &err) {
                if (server->onErrorMessage) {
                    server->onErrorMessage(channel, err);
                }
            });
            // 设置写入回调
            channel->setWriteCb([server, &channel](Size_t len) {
                if (server->onWrite) {
                    server->onWrite(channel, len);
                }
            });
            // 设置写入错误回调
            channel->setWriteErrCb([server, &channel](const string_type &err) {
                if (server->onErrorWrite) {
                    server->onErrorWrite(channel, err);
                }
            });
            // 设置关闭回调
            channel->setCloseCb([server, &channel]() {
                if (server->onClose) {
                    server->onClose(channel);
                }
                server->removeChannel(channel);
            });

            if (server->onNewClient) {
                server->onNewClient(channel);
            }
            channel->read();
        }
    }
}

Base::socket_t Net::TcpServer::createListenFd(UInt port)
{
    this->port_ = port;
    Base::socket_t sock = Base::creatSocket(Base::IP_4, Base::S_TCP, true);
    if (sock < 0){
        Log::FLogE("create socket error -- %s .", strerror(errno));
        perror("socket()");
        return -1;
    }
    // 设置套接字非阻塞
    Base::setSocketNonblock(this->listen_fd_);
    // 设置套接字复用地址
    Base::setSocketReuseAddress(this->listen_fd_);

    if (!Base::bindSocket(sock, Base::IP_4, nullptr, (Int) port)){
        Log::FLogE("bind socket error -- %s .", strerror(errno));
        perror("bind()");
        return -1;
    }
    if (!Base::listenSocket(sock)){
        Log::FLogE("listen socket error -- {}", strerror(errno));
        perror("listen()");
        return -1;
    }
    return sock;
}
