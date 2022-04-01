/**
* FileName:   HttpServer_Test.cc
* CreateDate: 2022-03-16 13:44:24
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        HttpServer 测试
*/
#include "net/http/HttpServer.h"
#include "base/System.h"

using namespace SinBack::Http;
using namespace SinBack;

int main()
{
    HttpServer server;
    HttpService service;
    // 设置http服务器配置
    server.setting().logPath = "./Log";
    String cwd = Base::system_get_cwd();
    // 静态文件根目录
    server.setting().staticFileDir = cwd + "/web";
    server.setting().workThreadNum = 4;     // 多线程
    // server.setting().workProcessNum = 4; // 多进程
    server.setting().keepAlive = true;
    // 拦截 /test下所有 GET 请求
    service.GET("/api/test", [](HttpContext& context) -> Int {
        return context.sendText("我是测试接口 !");
    });
    service.GET("/api/getTime", [](HttpContext& context) -> Int{
        return context.sendText(Base::getDateTimeNow());
    });
    service.GET("/api", [](HttpContext& context) -> Int{
        Log::FLogI("/api 下所有请求会过该拦截器");
        // 继续下一个拦截器
        return NEXT;
    });
    // 添加到 Test 服务模块
    server.addService("Test", &service);
    // 开始监听2021端口
    server.listen(2022, [](const SinBack::String& err){
        printf("Listen err: %s\n", err.c_str());
    });
    return 0;
}