/**
FileName:   main.cc
CreateDate: 20/1/2022
Author:     ticks
Email:      2938384958@qq.com
Des:        
*/

#include <sys/signal.h>
#include "net/TcpServer.h"

#ifdef __cplusplus
extern "C"{
#endif
#include "llhttp/llhttp.h"
#ifdef __cplusplus
}
#endif

using namespace SinBack::Net;
using namespace SinBack;

int main()
{
    std::shared_ptr<llhttp_t> parse(new llhttp_t);
    std::shared_ptr<llhttp_settings_t> setting(new llhttp_settings_t);
    llhttp_settings_init(setting.get());
    llhttp_init(parse.get(), HTTP_REQUEST, setting.get());

    setting->on_url = [](llhttp_t* parse, const Char* str, Size_t len) -> Int {
        std::basic_string<Char> url(str, len);
        fmt::print("url = {}", url);
        return 0;
    };
    setting->on_header_field = [](llhttp_t* parse, const Char* str, Size_t len) -> Int{
        std::basic_string<Char> header(str, len);
        fmt::print("header : {}", header.c_str());
        return 0;
    };

    signal(SIGPIPE, SIG_IGN);
    TcpServer server;
    server.on_new_client = [](const TcpServer::ChannelPtr& io){
        fmt::print("新客户端连接, fd = {}.\n", io->get_fd());
        // io->read(256);
    };
    server.on_message = [parse, setting](const TcpServer::ChannelPtr& io, TcpServer::StringBuf& buf){

        auto status = llhttp_execute(parse.get(), buf.c_str(), buf.length());
        if (status == HPE_OK){
            TcpServer::StringBuf data = "看啥，大傻瓜";
            TcpServer::StringBuf msg = fmt::format(
                    "HTTP/1.1 200 OK\r\n"
                    "Connection: close\r\n"
                    "Content-Type: text/plain;charset=UTF-8\r\n"
                    "Server: SinBack/0.1\r\n"
                    "Content-Length: {}\r\n\r\n"
                    "{}", data.length(), data.c_str());
            io->write(msg);
        }else{
            fmt::print("parse error : {}\n", llhttp_errno_name(status));
            io->close();
        }
    };
    server.on_close = [](const TcpServer::ChannelPtr& io){
        fmt::print("客户端关闭，fd = {}.\n", io->get_fd());
    };
    server.run(2021);
    while (getchar() != '\n');
    return 0;
}
