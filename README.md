# SinBack

一个参考[libhv](https://github.com)实现，基于C++11的网络库，事件驱动IO， 欢迎使用。

---

### 外部依赖库：
 - [libfmt]() : 一个方便而高效的C++字符串解析库，类似于C++20 format 。
 - [llhttp]() : nodejs 官方 http1 请求响应解析库 。

---

### 使用：

Http服务
```cpp
#include "net/HttpServer.h"

using namespace SinBack::Http;
using namespace SinBack;

int main()
{
    HttpServer server;
    HttpService service;
    // http服务器配置
    // 设置日志目录
    server.setting().log_dir = SIN_STR("./LogDir");
    // Http 静态服务根目录，不设置则不开启静态文件服务 -- 支持异步读取文件
    server.setting().static_file_dir = SIN_STR("/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack");
    server.setting().work_thread_num = 4;
    server.setting().keep_alive = true;
    // 拦截 /api/test GET 请求
    service.GET("/api/test", [](HttpContext& context) -> Int {
        return context.sen_text("我是测试接口 !");
    });
    service.GET("/api/getTime", [](HttpContext& context) -> Int{
        return context.sen_text(Base::getdatetimenow());
    });
    // 拦截 /api/ 下所有 GET 请求
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
    // 停止服务
    server.stop();
    return 0;
}
```

TCP服务：
```cpp
#include "net/TcpServer.h"

using namespace SinBack;
using namespace SinBack::Net;

int main()
{
    TcpServer server;

    // 有新客户端连接会调用该函数
    server.on_new_client = [](const TcpServer::ChannelPtr& io) {
        fmt::print("有新客户端连接, fd = {}\n", io->get_fd());
    };
    // 有新数据读取后会调用该函数
    server.on_message = [](const TcpServer::ChannelPtr& io, const String& read_msg) {
        // 像客户端写入发来的数据
        io->write(read_msg);
    };
    // 服务端读取数据错误会调用该函数
    server.on_error_message = [](const TcpServer::ChannelPtr& io, const String& err_msg) {
        fmt::print("读取错误，错误信息: {}\n", err_msg);
        // 这里不用调用 io->close ，读取错误会自动关闭
    };
    // 数据成功写入会调用该函数
    server.on_write = [](const TcpServer::ChannelPtr& io, Size_t len) {
        // len 为写入数据大小
        fmt::print("成功写入{}字节数据\n", len);
    };
    // 服务端写入时错误会调用该函数
    server.on_error_write = [](const TcpServer::ChannelPtr& io, const String& err_msg) {
        fmt::print("写入错误，错误信息: {}\n", err_msg);
        // 读取和写入错误时，IO句柄对应 loop 的日志记录器都会记录该错误
    };
    // IO关闭后调用该函数
    server.on_close = [](const TcpServer::ChannelPtr& io) {
        // 这里不应该再调用 read 和 write，因为此时 IO 已经关闭，不会处理读取和写入事件
        fmt::print("IO关闭，fd = {}\n", io->get_fd());
    };

    // 开始运行，并监听 2022 端口
    server.run(2022);

    while (getchar() != '\n');
    return 0;
}
```

---

### 后续功能
- 详细代码注释
- Windows平台支持
- UDP服务
- epoll支持边缘触发
- 添加Http2协议
- 添加WebSocket协议
- lua脚本
- 完善Http服务
- Mongodb和Mysql连接
- SSL支持
- ...

---

后期会新建一个分支用于详细代码注释，目前代码也有关键代码注释，
基于C++11实现，没有使用到C++的很多新特性，方便初学者学习。
如果这个网络库帮助到你，希望您可以动动小手指点一个star ^_^

代码问题请发邮件：**tw2938384958@gmail.com** 、 **2938384958@qq.com**