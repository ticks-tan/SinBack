/**
* FileName:   TcpServer_Test.cc
* CreateDate: 2022-03-16 13:45:06
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        TcpServer 测试
*/

#include "net/TcpServer.h"

using namespace SinBack;
using namespace SinBack::Net;

int main()
{
    TcpServer server;

    // 有新客户端连接会调用该函数
    server.onNewClient = [](const TcpServer::ChannelPtr& io) {
        printf("有新客户端连接, fd = %d\n", io->getFd());
    };
    // 有新数据读取后会调用该函数
    server.onMessage = [](const TcpServer::ChannelPtr& io, const String& read_msg) {
        // 像客户端写入发来的数据
        io->write(read_msg);
    };
    // 服务端读取数据错误会调用该函数
    server.onErrorMessage = [](const TcpServer::ChannelPtr& io, const String& err_msg) {
        printf("读取错误，错误信息: %s\n", err_msg.c_str());
        // 这里不用调用 io->close ，读取错误会自动关闭
    };
    // 数据成功写入会调用该函数
    server.onWrite = [](const TcpServer::ChannelPtr& io, Size_t len) {
        // len 为写入数据大小
        printf("成功写入%ld字节数据\n", len);
    };
    // 服务端写入时错误会调用该函数
    server.onErrorWrite = [](const TcpServer::ChannelPtr& io, const String& err_msg) {
        printf("写入错误，错误信息: %s\n", err_msg.c_str());
        // 读取和写入错误时，IO句柄对应 loop 的日志记录器都会记录该错误
    };
    // IO关闭后调用该函数
    server.onClose = [](const TcpServer::ChannelPtr& io) {
        // 这里不应该再调用 read 和 read，因为此时 IO 已经关闭，不会处理读取和写入事件
        printf("IO关闭，fd = %d\n", io->getFd());
    };

    // 开始运行，并监听 2022 端口
    server.run(2022);
    return 0;
}