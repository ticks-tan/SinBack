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

    auto module = std::make_shared<Http::HttpServer>();
    Http::HttpService service;
    // 添加 Service
    service.GET("/api/test", [](Http::HttpContext& cxt) -> Int {
        return cxt.sendText("我是测试接口");
    });

    module->setting().keepAlive = false;
    module->setting().staticFileDir = "../web";
    module->addService("main", &service);

    Main::Application app;
    app.setting().workThreadNum = 4;
    app.setting().listenPort = 2022;
    app.setting().logPath = "./SinBack";
    app.setModule(module);

    app.run([](const String& msg){
        printf("%s\n", msg.c_str());
    });

    return 0;
}
