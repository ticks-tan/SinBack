/**
FileName:   main.cc
CreateDate: 20/1/2022
Author:     ticks
Email:      2938384958@qq.com
Des:        
*/

#include "net/HttpServer.h"

using namespace SinBack::Http;
using namespace SinBack;

int main()
{
    HttpServer server;
    HttpService service;
    // 设置http服务器配置
    server.setting().log_dir = "./LogDir";
    server.setting().work_thread_num = 4;
    server.setting().keep_alive = false;
    // 设置 service
    // 拦截 /test下所有 GET 请求
    service.GET("/test/**", [](HttpContext& context) -> Int {
        return context.sen_text("我是测试接口 !");
    });
    // 添加到 Test 服务模块
    server.add_service("Test", &service);
    // 开始监听2021端口
    server.listen(2022, [](const SinBack::String& err){
        fmt::print("listen error: {}\n", err);
    });
    while (getchar() != '\n');
    server.stop();
    return 0;
}
