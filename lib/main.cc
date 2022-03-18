/**
FileName:   main.cc
CreateDate: 20/1/2022
Author:     ticks
Email:      2938384958@qq.com
Des:        
*/

#include "net/http/HttpServer.h"
#include "tools/Url.h"
#include <sys/signal.h>

using namespace SinBack::Http;
using namespace SinBack;

int main()
{
    signal(SIGPIPE, SIG_IGN);
    HttpServer server;
    HttpService service;
    // 设置http服务器配置
    server.setting().logDir = SIN_STR("/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack/cmake-build-debug");
    // 静态文件根目录
    server.setting().staticFileDir = SIN_STR("/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack");
    server.setting().workThreadNum = 8;
    server.setting().keepAlive = true;
    // 拦截 /test下所有 GET 请求
    service.GET("/api/test", [](HttpContext& context) -> Int {
        Log::logi("我是测试接口 , request url = {} .", context.request().url);
        return context.senText("我是测试接口 !");
    });
    service.GET("/api/getTime", [](HttpContext& context) -> Int{
        return context.senText(Base::getDateTimeNow());
    });
    // 添加到 Test 服务模块
    server.addService("Test", &service);
    // 开始监听2021端口
    server.listen(2022, [](const SinBack::String& err){
        fmt::print("listen error: {}\n", err);
    });
    while (getchar() != '\n');
    server.stop();
    return 0;
}
