/**
FileName:   main.cc
CreateDate: 20/1/2022
Author:     ticks
Email:      2938384958@qq.com
Des:        
*/

#include <sys/signal.h>
#include "net/TcpServer.h"

using namespace SinBack::Net;

int main()
{
    signal(SIGPIPE, SIG_IGN);
    TcpServer server;
    server.on_new_client = [](const TcpServer::ChannelPtr& io){
        fmt::print("新客户端连接, fd = {}.\n", io->get_fd());
    };
    server.on_message = [](const TcpServer::ChannelPtr& io, TcpServer::StringBuf& buf){
        fmt::print("接收的数据: \n{}\n", buf.c_str());
        TcpServer::StringBuf data = "看啥，大傻瓜";
        TcpServer::StringBuf msg = fmt::format(
                "HTTP/1.1 200 OK\r\n"
                "Connection: close\r\n"
                "Content-Type: text/plain;charset=UTF-8\r\n"
                "Server: SinBack/0.1\r\n"
                "Content-Length: {}\r\n\r\n"
                "{}", data.length(), data.c_str());
        io->write(msg);
    };
    server.on_close = [](const TcpServer::ChannelPtr& io){
        fmt::print("客户端关闭，fd = {}.\n", io->get_fd());
    };
    server.run(2021);
    while (getchar() != '\n');
    return 0;
}
