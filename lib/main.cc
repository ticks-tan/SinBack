/**
FileName:   main.cc
CreateDate: 20/1/2022
Author:     ticks
Email:      2938384958@qq.com
Des:        
*/

#include <random>
#include "core/EventLoop.h"
using namespace SinBack;

int main()
{
    // 创建监听套接字
    Base::socket_t server = Base::creat_socket(Base::IP_4, Base::S_TCP, true);
    if (server < 0){
        Log::logd("creat the socket error !");
        return 1;
    }
    // 复用端口
    Base::socket_reuse_address(server);
    //  绑定
    if (!Base::bind_socket(server, Base::IP_4, nullptr, 2022)){
        Log::logd("bind socket error !");
        perror("bind: ");
        return 1;
    }
    if (!Base::listen_socket(server)){
        Log::logd("listen socket error !");
        return 1;
    }
    // 事件循环
    Core::EventLoop loop;
    auto acceptor = loop.accept_io(server, [](Core::IOEvent* io){
        Core::EventLoop* loop = io->loop_;
        fmt::print("new client , fd = {} .\n", io->fd_);
        loop->read_io(io->fd_, 256, [](Core::IOEvent* io, std::basic_string<Char>& buf){
            fmt::print("receive message:\n{}", buf.c_str());
            std::basic_string<Char> msg1 = "你好呀，大傻比";
            std::basic_string<Char> msg = fmt::format("HTTP/1.1 200 OK\r\n"
                                                      "Content-Length: {}\r\n"
                                                      "Connection: close\r\n"
                                                      "Content-Type: text/plain;charset=UTF-8\r\n\r\n"
                                                      "{}"
                                                      , msg1.length(), msg1.c_str());
            io->loop_->write_io(io->fd_, msg.data(), msg.length(), nullptr);
        });
    });
    loop.add_timer([](const std::weak_ptr<Core::TimerEvent>& ev){
        auto timer = ev.lock();
        if (timer){
            timer->loop_->stop();
        }
    }, 15000, 1);
    loop.queue_func([](){
        fmt::print("Hello, I am custom event !\n");
    });
    loop.run();
    return 0;
}
