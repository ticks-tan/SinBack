# SinBack

一个参考[libhv](https://github.com\/ithewei/libhv)，基于**C++11**的**Linux服务端**网络库，事件驱动IO， 欢迎使用。

---

### 外部依赖库：
 - [llhttp](https://github.com/nodejs/llhttp) : nodejs 官方 http1 请求响应解析库 。
 - [OpenSSL](https://www.openssl.org)

---

### 使用：

Http服务
```cpp
#include "Application.h"
#include "base/System.h"
#include "module/http/HttpServer.h"

using namespace SinBack;
using namespace SinBack::Module;

int main(int argc, char* argv[])
{
Base::system_signal(SIGPIPE, nullptr);

auto module = std::make_shared<Http::HttpServer>();
Http::HttpService service;
// 添加 Service
service.GET("/api/test", [](Http::HttpContext& cxt) -> Int {
return cxt.sendText("我是测试接口");
});

module->setting().keepAlive = false;
module->setting().staticFileDir = "../web";
module->addService("main", &service);

Main::Application app;
app.setting().workThreadNum = 4;
app.setting().listenPort = 2022;
app.setting().logPath = "./SinBack";
app.setModule(module);

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

代码问题请发邮件：**tw2938384958@gmail.com** 、 **2938384958@qq.com**