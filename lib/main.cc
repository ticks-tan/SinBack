/**
FileName:   main.cc
CreateDate: 20/1/2022
Author:     ticks
Email:      2938384958@qq.com
Des:        
*/

#include <sys/signal.h>
#include "net/TcpServer.h"
#include "net/HttpParser.h"

using namespace SinBack::Net;
using namespace SinBack;

int main()
{

    Http::Http1Parse parser;

    TcpServer server;

    server.on_message = [&parser](const TcpServer::ChannelPtr& io, const TcpServer::StringBuf& buf){
        parser.parse_data(buf.c_str(), buf.length());
        if (parser.need_receive()){
            io->read();
            fmt::print("\nstatus = {}\n", parser.get_status());
        } else{
            fmt::print("request url = {}\n", parser.request.url);
            TcpServer::StringBuf data = "看啥，大傻瓜 -> ";
            data += Base::getdatetimenow();
            TcpServer::StringBuf msg = fmt::format(
                    "HTTP/1.1 200 OK\r\n"
                    "Connection: close\r\n"
                    "Content-Type: text/plain;charset=UTF-8\r\n"
                    "Server: SinBack/0.1\r\n"
                    "Content-Length: {}\r\n\r\n"
                    "{}", data.length(), data.c_str());
            io->write(msg);
        }
        fmt::print("data: \n{}\n. ", parser.request.to_string().c_str());
    };
    server.run(2021);
    server.loop()->add_timer([](const std::weak_ptr<Core::TimerEvent>& ev){
        fmt::print("Hello World.\n");
    }, 5000, 1);
    while (getchar() != '\n');
    return 0;
}
