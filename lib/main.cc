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
#include "base/Process.h"
#include "base/Args.h"

using namespace SinBack::Http;
using namespace SinBack;

int main(int argc, char* argv[])
{
    Base::Args args(argc, argv);

    if (args.hasArg(SIN_STR("help"))){
        fmt::print("help is none!\n");
    }
    if (args.hasArg("--worknum")){
        fmt::print("worknums = {}\n", args.getArg(SIN_STR("--worknum")));
    }

    signal(SIGPIPE, SIG_IGN);
    HttpServer server;
    HttpService service;
    // 设置http服务器配置
    server.setting().logPath = SIN_STR("/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack/build/SinBack");
    // 静态文件根目录
    server.setting().staticFileDir = SIN_STR("/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack/web");
    server.setting().workThreadNum = 8;
    server.setting().keepAlive = true;
    // 拦截 /test下所有 GET 请求
    service.GET("/api/test", [](HttpContext& context) -> Int {
        Log::logi("我是测试接口 , request url = {} .", context.request().url);
        return context.sendText("我是测试接口 !");
    });
    service.GET("/api/getTime", [](HttpContext& context) -> Int{
        if (context.parseUrl()){
            auto& params = context.urlParams();
            if (params.find(SIN_STR("format")) != params.end()){
                return context.sendText(Base::getDateTimeNow(params[SIN_STR("format")]));
            }
            return context.sendText(SIN_STR("请求参数错误"));
        }
        return context.error();
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
