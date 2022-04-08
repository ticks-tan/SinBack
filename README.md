# SinBack

一个参考[libhv](https://github.com\/ithewei/libhv)，基于**C++11**的**Linux服务端**网络库，事件驱动IO， 欢迎使用。

---

### 外部依赖库(已添加至项目，无需手动添加)：
 - [llhttp](https://github.com/nodejs/llhttp) : nodejs 官方 http1 请求响应解析库 。

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
    server.setting().logDir = "./LogDir";
    // Http 静态服务根目录，不设置则不开启静态文件服务 -- 支持异步读取文件
    server.setting().staticFileDir = "/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack";
    server.setting().workThreadNum = 4;
    server.setting().keepAlive = true;
    // 拦截 /api/test GET 请求
    service.GET("/api/test", [](HttpContext& context) -> Int {
        return context.sendText("我是测试接口 !");
    });
    service.GET("/api/getTime", [](HttpContext& context) -> Int{
        return context.sendText(Base::getDateTimeNow());
    });
    // 添加到 Test 服务模块
    server.addService("Test", &service);
    // 开始监听2021端口
    server.listen(2022, [](const SinBack::SinString& err){
        fmt::print("listen error_: %s\n", err);
    });
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
    server.onNewClient = [](const TcpServer::ChannelPtr& io) {
        fmt::print("有新客户端连接, fd = %d\n", io->getFd());
    };
    // 有新数据读取后会调用该函数
    server.onMessage = [](const TcpServer::ChannelPtr& io, const SinString& read_msg) {
        // 像客户端写入发来的数据
        io->read(read_msg);
    };
    // 服务端读取数据错误会调用该函数
    server.onErrorMessage = [](const TcpServer::ChannelPtr& io, const SinString& err_msg) {
        fmt::print("读取错误，错误信息: %s\n", err_msg);
        // 这里不用调用 io->close ，读取错误会自动关闭
    };
    // 数据成功写入会调用该函数
    server.onWrite = [](const TcpServer::ChannelPtr& io, Size_t len) {
        // len 为写入数据大小
        fmt::print("成功写入%ld字节数据\n", len);
    };
    // 服务端写入时错误会调用该函数
    server.onErrorWrite = [](const TcpServer::ChannelPtr& io, const SinString& err_msg) {
        fmt::print("写入错误，错误信息: %s\n", err_msg);
        // 读取和写入错误时，IO句柄对应 loop 的日志记录器都会记录该错误
    };
    // IO关闭后调用该函数
    server.onClose = [](const TcpServer::ChannelPtr& io) {
        // 这里不应该再调用 read 和 read，因为此时 IO 已经关闭，不会处理读取和写入事件
        fmt::print("IO关闭，fd = %d\n", io->getFd());
    };

    // 开始运行，并监听 2022 端口
    server.run(2022);
    return 0;
}
```

---

### 后续预计添加功能
- 详细代码注释
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

### 代码结构

----- base :  底层函数封装

----- core : 事件驱动核心代码

----- net : 顶层网络封装

----- tools : 一些工具函数

---


代码问题请发邮件：**tw2938384958@gmail.com** 、 **2938384958@qq.com**