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
    server.on_new_client = [](const TcpServer::ChannelPtr& io) {
        fmt::print("有新客户端连接, fd = {}\n", io->getFd());
    };
    // 有新数据读取后会调用该函数
    server.on_message = [](const TcpServer::ChannelPtr& io, const String& read_msg) {
        // 像客户端写入发来的数据
        io->write(read_msg);
    };
    // 服务端读取数据错误会调用该函数
    server.on_error_message = [](const TcpServer::ChannelPtr& io, const String& err_msg) {
        fmt::print("读取错误，错误信息: {}\n", err_msg);
        // 这里不用调用 io->close ，读取错误会自动关闭
    };
    // 数据成功写入会调用该函数
    server.on_write = [](const TcpServer::ChannelPtr& io, Size_t len) {
        // len 为写入数据大小
        fmt::print("成功写入{}字节数据\n", len);
    };
    // 服务端写入时错误会调用该函数
    server.on_error_write = [](const TcpServer::ChannelPtr& io, const String& err_msg) {
        fmt::print("写入错误，错误信息: {}\n", err_msg);
        // 读取和写入错误时，IO句柄对应 loop 的日志记录器都会记录该错误
    };
    // IO关闭后调用该函数
    server.on_close = [](const TcpServer::ChannelPtr& io) {
        // 这里不应该再调用 read 和 write，因为此时 IO 已经关闭，不会处理读取和写入事件
        fmt::print("IO关闭，fd = {}\n", io->getFd());
    };

    // 开始运行，并监听 2022 端口
    server.run(2022);

    while (getchar() != '\n');
    return 0;
}