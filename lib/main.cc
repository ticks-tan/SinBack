/**
FileName:   main.cc
CreateDate: 20/1/2022
Author:     ticks
Email:      2938384958@qq.com
Des:        
*/

#include "net/http/HttpServer.h"
#include "tools/Url.h"
#include "base/System.h"
#include "base/Args.h"

using namespace SinBack::Http;
using namespace SinBack;

int main(int argc, char* argv[])
{
    signal(SIGPIPE, SIG_IGN);
    HttpServer server;
    HttpService service;
    // 设置http服务器配置
    server.setting().logPath = "/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack/build/SinBack";
    // 静态文件根目录
    server.setting().staticFileDir = "/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack/web";
    // 设置进程数量
    server.setting().workProcessNum = 4;
    server.setting().keepAlive = false;
    // 拦截 /test下所有 GET 请求
    service.GET("/api/test", [](HttpContext& context) -> Int {
        Log::logi("我是测试接口 , request url = %s .", context.request().url.c_str());
        return context.sendText("我是测试接口 !");
    });
    service.GET("/api/getTime", [](HttpContext& context) -> Int{
        if (context.parseUrl()){
            auto& params = context.urlParams();
            if (params.find("format") != params.end()){
                return context.sendText(Base::getDateTimeNow(params["format"]));
            }
            return context.sendText("请求参数错误");
        }
        return context.error();
    });
    // 添加到 Test 服务模块
    server.addService("Test", &service);
    // 开始监听2021端口
    server.listen(2022, [](const SinBack::String& err){
        printf("listen error: %s\n", err.c_str());
    });
}
