/**
FileName:   main.cc
CreateDate: 20/1/2022
Author:     ticks
Email:      2938384958@qq.com
Des:        
*/
#include "Application.h"
#include "base/System.h"
#include "module/http/HttpServer.h"

using namespace SinBack;
using namespace SinBack::Module;

int main(int argc, char* argv[])
{
    Base::system_signal(SIGPIPE, nullptr);

    // HTTP模块
    auto http = std::make_shared<Http::HttpServer>();
    Http::HttpService service;
    // 添加 Service
    service.GET("/api/test", [](Http::HttpContext& cxt) -> Int {
        printf("url = %s\n", cxt.request().url.c_str());
        return cxt.sendText("我是测试接口");
    });

    // 开启 http 的keep-alive
    http->setting().keepAlive = true;
    // 设置静态文件根目录
    http->setting().staticFileDir = "/run/media/ticks/BigDisk/Codes/vscode/HtmlCode/Ticks/blog";
    http->addService("main", &service);

    Main::Application app;
    app.setting().workThreadNum = 4;
    app.setting().listenPort = 2023;
    app.setting().logPath = "./SinBack1";
    // 启用 SSL
    app.setting().enableSSL = true;
    // 指定证书和私钥
    app.setting().certPath = "/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack/build/cert/localhost+2.pem";
    app.setting().keyPath = "/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack/build/cert/localhost+2-key.pem";
    // 加载HTTP模块
    app.setModule(http);

    app.run([](const String& msg){
        printf("%s\n", msg.c_str());
    });
    return 0;
}

