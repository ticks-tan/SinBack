/**
* FileName:   TcpServer.cc
* CreateDate: 2022-03-11 15:43:54
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "TcpServer.h"

using namespace SinBack;

SinBack::Net::TcpServer::TcpServer()
    : work_thread_cnt(std::thread::hardware_concurrency())
    , listen_fd_(-1)
    , port_(0)
{
}

SinBack::Net::TcpServer::~TcpServer()
{
    this->accept_loop_.stop(true);
    if (this->work_thread_cnt > 0){
        this->work_loops_.stop(true);
    }
}

SinBack::Net::TcpServer::ChannelPtr
SinBack::Net::TcpServer::add_channel(const std::weak_ptr<Core::IOEvent>& ev) {
    std::shared_ptr<Core::IOEvent> io = ev.lock();
    UInt id = io->id_;
    auto channel = std::make_shared<Core::Channel>(io);
    std::unique_lock<std::mutex> lock(mutex);
    this->channels_[id] = channel;
    return this->channels_[id];
}

void
SinBack::Net::TcpServer::remove_channel(const SinBack::Net::TcpServer::ChannelPtr &channel)
{
    auto id = channel->get_id();
    std::unique_lock<std::mutex> lock(this->mutex);
    this->channels_.erase(id);
}

SinBack::Net::TcpServer::ChannelPtr
SinBack::Net::TcpServer::get_channel(SinBack::Base::socket_t id) {
    auto it = this->channels_.find(id);
    std::unique_lock<std::mutex> lock(this->mutex);
    return (it != this->channels_.end()) ? it->second : nullptr;
}

bool
SinBack::Net::TcpServer::run(UInt port)
{
    this->listen_fd_ = create_listen_fd(port);
    if (this->listen_fd_ == -1){
        fmt::print("Run tcp_server error !\n");
        return false;
    }
    Base::set_socket_nonblock(this->listen_fd_);
    if (this->work_thread_cnt > 0){
        this->work_loops_.start();
    }
    this->accept_loop_.start(std::bind(&TcpServer::start_accept, this), nullptr);
    return true;
}

SinBack::Net::TcpServer::EventLoopPtr
SinBack::Net::TcpServer::loop(Int index) {
    return this->work_loops_.loop(index);
}

void
SinBack::Net::TcpServer::start_accept()
{
    auto acceptor = this->accept_loop_.loop()->accept_io(this->listen_fd_, new_client);
    acceptor->context_ = this;
}

void
SinBack::Net::TcpServer::new_client(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io){
        auto* server = (TcpServer*)io->context_;
        if (server) {
            if (server->accept_count() > default_max_accept_count) {
                server->accept_loop_.loop()->logger().warn("accept count over max !");
                io->loop_->close_io(io->fd_);
                return;
            }
            EventLoopPtr work_loop = server->loop();
            // 由于work loop与 accept loop不同，需要对 io事件设置新的 loop
            Core::EventLoop::change_io_loop(io, work_loop.get());
            const ChannelPtr channel = server->add_channel(io);
            Base::set_socket_nonblock(channel->get_fd());
            // 设置读取回调
            channel->set_read_cb([server, &channel](const StringBuf &buf) {
                if (server->on_message) {
                    server->on_message(channel, buf);
                }
            });
            // 设置读取错误回调
            channel->set_read_err_cb([server, &channel](const StringBuf& err){
                if (server->on_error_message){
                    server->on_error_message(channel, err);
                }
            });
            // 设置写入回调
            channel->set_write_cb([server, &channel](Size_t len) {
                if (server->on_write) {
                    server->on_write(channel, len);
                }
            });
            // 设置写入错误回调
            channel->set_write_err_cb([server, &channel](const StringBuf& err){
                if (server->on_error_write){
                    server->on_error_write(channel, err);
                }
            });
            // 设置关闭回调
            channel->set_close_cb([server, &channel]() {
                if (server->on_close) {
                    server->on_close(channel);
                }
                server->remove_channel(channel);
            });

            if (server->on_new_client) {
                server->on_new_client(channel);
            }
            channel->read();
        }
    }
}

Base::socket_t Net::TcpServer::create_listen_fd(UInt port)
{
    this->port_ = port;
    Base::socket_t sock = Base::creat_socket(Base::IP_4, Base::S_TCP, true);
    if (sock < 0){
        this->accept_loop_.loop()->logger().error("create socket error !");
        perror("socket()");
        return -1;
    }
    Base::socket_reuse_address(sock);
    if (!Base::bind_socket(sock, Base::IP_4, nullptr, (Int)port)){
        this->accept_loop_.loop()->logger().error("bind socket error !");
        perror("bind()");
        return -1;
    }
    if (!Base::listen_socket(sock)){
        this->accept_loop_.loop()->logger().error("listen socket error !");
        perror("listen()");
        return -1;
    }
    return sock;
}
