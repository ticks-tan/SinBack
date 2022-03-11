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

struct Pair
{
    Int first;
    Int second;
};

#define MAX_STACK_SIZE 1024

static Pair g_stack[MAX_STACK_SIZE];
static Int g_stack_top = 0;

bool stack_empty(){
    return (g_stack_top == 0);
}
bool stack_full(){
    return (g_stack_top == MAX_STACK_SIZE);
}
bool stack_push(Pair pair){
    if (stack_full()){
        return false;
    }
    g_stack[g_stack_top++] = pair;
    return true;
}
bool stack_pop(){
    if (stack_empty()){
        return false;
    }
    --g_stack_top;
    return true;
}
Pair stack_top(){
    return g_stack[g_stack_top - 1];
}

void stack_clear(){
    g_stack_top = 0;
}

void quick_sort(Int array[], Int start, Int end)
{
    if (start >= end){
        return;
    }
    stack_push({start, end});
    Pair top{};
    while (!stack_empty()){
        top = stack_top();
        Int front = top.first, back = top.second;
        Int tmp = array[front];
        while (front < back){
            while (front < back && array[back] > tmp){
                --back;
            }
            if (front < back) {
                array[front++] = array[back];
            }
            while (front < back && array[front] <= tmp){
                ++front;
            }
            if (front < back){
                array[back--] = array[front];
            }
        }
        array[front] = tmp;
        stack_pop();
        if (top.first < front - 1){
            stack_push({top.first, front - 1});
        }
        if (front + 1 < top.second){
            stack_push({front + 1, top.second});
        }
    }
}

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
    loop.run();
    return 0;
}
