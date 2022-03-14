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
    TcpServer server;
    std::unordered_map<UInt, std::shared_ptr<Http::Http1Parse>> maps;

    server.on_message = [&maps](const TcpServer::ChannelPtr& io, const TcpServer::StringBuf& buf){
        auto it = maps.find(io->get_id());
        if (it == maps.end()){
            maps[io->get_id()] = std::make_shared<Http::Http1Parse>();
        }
        std::shared_ptr<Http::Http1Parse> parser = maps[io->get_id()];
        parser->parse_data(buf.c_str(), buf.length());
        if (parser->need_receive()){
            io->read();
        } else{
            fmt::print("request url = {}\n", parser->request.url);
            TcpServer::StringBuf data = "看啥，大傻瓜 -> ";
            data += Base::getdatetimenow();
            TcpServer::StringBuf msg = fmt::format(
                    "HTTP/1.1 200 OK\r\n"
                    "Connection: close\r\n"
                    "Content-Type: text/plain;charset=UTF-8\r\n"
                    "Server: SinBack/0.1\r\n"
                    "Content-Length: {}\r\n\r\n"
                    "{}\n", data.length(), data.c_str());
            io->write(msg);
        }
        fmt::print("\n{}\n", buf.c_str());
    };
    server.run(2021);
    while (getchar() != '\n');
    return 0;
}
