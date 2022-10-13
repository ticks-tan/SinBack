# SinBack

一个参考[libhv](https://github.com\/ithewei/libhv)，基于**C++11**的**Linux服务端**网络库，事件驱动IO。

---

### 外部依赖库(https -- 目前存在问题，未解决)：
 - [OpenSSL](https://www.openssl.org)

---

### 使用：

Http(s)服务
```cpp
#include "Application.h"
#include "base/System.h"
#include "module/http/HttpServer.h"

using namespace SinBack;
using namespace SinBack::Module;

int main(int argc, char* argv[])
{
    Base::system_signal(SIGPIPE, nullptr);

    // HTTP模块
    auto http = std::make_shared<Http::HttpServer>();
    Http::HttpService service;
    // 添加 Service
    service.GET("/api/test", [](Http::HttpContext& cxt) -> Int {
    printf("url = %s\n", cxt.request().url.c_str());
        return cxt.sendText("我是测试接口");
    });

    // 开启 http 的keep-alive
    http->setting().keepAlive = true;
    // 设置静态文件根目录
    http->setting().staticFileDir = "/run/media/ticks/BigDisk/Codes/vscode/HtmlCode/Ticks/blog";
    http->addService("main", &service);

    Main::Application app;
    // 设置多线程模式
    app.setting().workThreadNum = 4;
    app.setting().listenPort = 2023;
    // 设置日志文件
    app.setting().logPath = "./SinBack";
    // 启用 SSL (默认不启用)
    app.setting().enableSSL = true;
    // 指定证书和私钥
    app.setting().certPath = "/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack/build/cert/localhost+2.pem";
    app.setting().keyPath = "/run/media/ticks/BigDisk/Codes/Clion/Me/SinBack/build/cert/localhost+2-key.pem";
    // 加载HTTP模块
    app.setModule(http);

    // 开始运行(会阻塞)
    app.run([](const String& msg){
        printf("%s\n", msg.c_str());
    });
    return 0;
}


```

TCP服务：
```cpp

```

### 代码结构

----- base :  底层函数封装

----- core : 事件驱动核心代码

----- module : 应用层功能模块

----- tools : 一些工具函数

---

代码问题请 [联系邮箱](ticks.cc@gmail.com)
