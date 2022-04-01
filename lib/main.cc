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

    String cwd = Base::system_get_cwd();
    printf("cwd: %s\n", cwd.c_str());
    // 设置http服务器配置
    server.setting().logPath = cwd + "/SinBack";
    // 静态文件根目录
    server.setting().staticFileDir = cwd + "/web";
    // 设置线程数量
    server.setting().workThreadNum = 4;
    // 默认允许客户端 keep-alive，不用设置

    // 拦截 /test下所有 GET 请求
    service.GET("/api/test", [](HttpContext& context) -> Int {
        Log::FLogI("我是测试接口 , request url = %s .", context.request().url.c_str());
        return context.sendText("我是测试接口 !");
    });
    service.GET("/", [](HttpContext& context) -> Int {
        printf("所有请求都会走这里 -- %s\n", context.request().url.c_str());
        // 返回 NEXT 时，会继续执行下一个拦截器
        return NEXT;
    });
    // 添加到 Test 服务模块
    server.addService("Test", &service);
    // 开始监听2021端口
    server.listen(2022, [](const SinBack::String& err){
        printf("listen error: %s\n", err.c_str());
    });
    return 0;
}
