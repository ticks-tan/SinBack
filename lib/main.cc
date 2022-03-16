/**
FileName:   main.cc
CreateDate: 20/1/2022
Author:     ticks
Email:      2938384958@qq.com
Des:        
*/

#include "net/http/HttpServer.h"

using namespace SinBack::Http;
using namespace SinBack;

int main()
{
    HttpServer server;
    HttpService service;
    // 设置http服务器配置
    server.setting().log_dir = SIN_STR("./LogDir/");
    // 静态文件根目录
    server.setting().static_file_dir = SIN_STR("/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack");
    server.setting().work_thread_num = 4;
    server.setting().keep_alive = false;
    // 拦截 /test下所有 GET 请求
    service.GET("/api/test", [](HttpContext& context) -> Int {
        return context.sen_text("我是测试接口 !");
    });
    service.GET("/api/getTime", [](HttpContext& context) -> Int{
        return context.sen_text(Base::getdatetimenow());
    });
    service.GET("/api/.*", [](HttpContext& context) -> Int{
        fmt::print("api拦截成功!\n");
        // 继续下一个 Service
        return NEXT;
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
