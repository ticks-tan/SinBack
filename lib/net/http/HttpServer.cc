/**
* FileName:   HttpServer.cc
* CreateDate: 2022-03-14 17:49:22
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "HttpServer.h"
#include "base/System.h"

#ifdef OS_LINUX
#include <sys/wait.h>
#endif

using namespace SinBack;

Http::HttpServer::HttpServer()
    : setting_()
    , running_(false)
    , listen_port_(0)
    , accept_th_(nullptr)
    , work_th_()
    , tid_(0)
{
    // 初始化工作
    init();
}

Http::HttpServer::~HttpServer()
{
    if (this->running_){
        this->stop();
    }
}

/**
 * 收到进程退出信号
 * @param sig
 */
void Http::HttpServer::sigStop(int sig)
{
    if (this->running_) {
        this->stop();
    }
    // 取消所有日志记录器
    Log::Logger::unregisterAllLogger();
    exit(0);
}

/**
 * 初始化 Setting
 */
void Http::HttpServer::init()
{
    // 默认设置
    this->setting_.keepAlive = false;
    this->setting_.logPath = "./";
    // 静态文件默认开启，手动指定静态文件根目录才会开启
    this->setting_.staticFileDir = "";
    this->setting_.maxAcceptCnt = 4096;
    // 默认为多线程模式，指定进程数量后采用多进程模式
    this->setting_.workProcessNum = 0;
    // 默认线程数量为当前核心数两倍
    this->setting_.workThreadNum = std::thread::hardware_concurrency();
    // 默认不启用 SSL
    this->setting_.enableSSL = false;
}

bool Http::HttpServer::addService(const string_type &name, Http::HttpService *service)
{
    auto it = this->services_.find(name);
    std::unique_lock<std::mutex> lock(this->mutex_);
    if (it == this->services_.end()){
        this->services_[name] = service;
        return true;
    }
    return false;
}

bool Http::HttpServer::listen(UInt port, const std::function<void(const String &)> &err_callback)
{
    if (this->running_){
        if (err_callback){
            err_callback("HttpServer is listening !");
        }
        return false;
    }
    this->tid_ = gettid();
    this->running_ = true;
    this->listen_port_ = port;

    // 忽略 Ctrl + C 信号
    Base::system_ignore_sig(SIGINT);
    // 忽略 PIPE 错误信号
    Base::system_ignore_sig(SIGPIPE);

    // 开始运行
    start();
    // 使用条件变量等待程序退出
    std::unique_lock<std::mutex> lock(this->mutex_);
    this->wait_cv_.wait(lock, [this](){
        return !this->running_;
    });
    // 服务停止
    Log::FLogI("Http Server stopped, pid = %ld, tid = %ld .", getpid(), gettid());
    return true;
}

void Http::HttpServer::stop()
{
    // 停止运行
    if (this->running_){
        this->running_ = false;
        if (this->setting_.workProcessNum == 0){
            // 线程模式
            this->accept_th_->stop(true);
            this->work_th_->stop(true);
        }else {
            // 多进程模式
            this->accept_th_->stop(true);
            for (auto pid : this->child_pid_){
                Base::system_send_sig(pid, SIGTERM);
            }
            this->child_pid_.clear();
        }
        // 唤醒主线程，取消睡眠
        this->wait_cv_.notify_all();
    }
}

/**
 * 新客户端连接后会调用该函数
 * @param ev : 新连接的客户端对应 IO 事件句柄
 */
void Http::HttpServer::onNewClient(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io){
        // 判断是否超过最大允许的连接数量
        Size_t cnt = this->getConnectCount();
        if (cnt >= this->setting().maxAcceptCnt){
            // 超过最大连接数量
            Log::FLogE("HttpServer over the max allow connect count : %ld, pid = %ld, tid = %ld !",
                       cnt, getpid(), gettid());
            io->close(false);
            return;
        }

        if (this->setting_.workThreadNum > 0) {
            // 多线程模式
            auto loop = this->work_th_->loop();
            if (loop) {
                Core::EventLoop::changeIoLoop(io, loop.get());
            }
        }
        // 设置事件回调
        io->read_cb_ = std::bind(&HttpServer::onNewMessage, this, std::placeholders::_1, std::placeholders::_2);
        io->read_err_cb_ = std::bind(&HttpServer::onMessageError, this, std::placeholders::_1, std::placeholders::_2);
        io->write_cb_ = std::bind(&HttpServer::onSend, this, std::placeholders::_1, std::placeholders::_2);
        io->write_err_cb_ = std::bind(&HttpServer::onSendError, this, std::placeholders::_1, std::placeholders::_2);
        io->close_cb_ = std::bind(&HttpServer::onDisconnect, this, std::placeholders::_1);
        if (io->context_ == nullptr) {
            auto *http_context = new Http::HttpContext(this);
            // 将 HttpContext与IO事件关联
            io->context_ = http_context;
        }
        // 当前连接客户端 +1
        this->incConnectCount();
        // 读取数据 (主要作用是将可读事件注册到 Loop)
        io->read();
    }
}

/**
 * 有新数据时会调用该函数 -- 核心处理函数
 * @param ev : 读取数据的IO事件句柄
 * @param read_msg : 本次读取的数据
 */
void Http::HttpServer::onNewMessage(const std::weak_ptr<Core::IOEvent>& ev, const String& read_msg)
{
    // 有新数据到来：解析数据 -> 匹配路由并执行对应回调 -> 解析返回信息 -> 调用write写入数据
    auto io = ev.lock();
    if (io){
        // 取出 HttpContext
        auto http_context = (HttpContext*)(io->context_);

        if (http_context){
            // 取出 Http Parser 解析 Http 数据
            auto* parser = http_context->parser();
            // 解析数据
            Int ret = parser->parseData(read_msg.c_str(), read_msg.length());
            // 解析失败，关闭连接
            if (ret != HPE_OK){
                io->close(false);
                return;
            }
            // 判断请求是否完成，是否还需要接受数据
            if (parser->needReceive()){
                // 需要继续读取数据
                return;
            } else {
                // 重置解析器
                parser->resetParser();
                // 解析完成, 初始化，初始化失败关闭连接
                if (!http_context->init()){
                    io->close(false);
                    return;
                }

                Int method = http_context->request().method;
                string_type url = http_context->request().url;
                bool keep_alive = http_context->request().header.getHead("Connection") == "keep-alive";

                if (!this->setting_.keepAlive && keep_alive){
                    keep_alive = false;
                    // 服务器配置为不开启保持连接，指定消息头通知客户端主动关闭连接
                    http_context->response().header.setHead("Connection", "close");
                }

                Int call_ret = 0;

                // 查找匹配的 service
                for (auto &service: this->services()) {
                    auto routers = service.second->matchService(url, method);
                    // 依次执行 service 回调
                    for (auto& call : routers){
                        // 如果方法匹配，调用回调
                        if (call->method & method){
                            if (call->callback){
                                call_ret = call->callback(*http_context);
                            }
                            // 如果不为 NEXT, 后续回调被忽略
                            if (call_ret != Http::ServiceCallBackRet::NEXT){
                                goto SERVICE_END;
                            }
                        }
                    }
                }
                // 没有执行回调 (请求没有被拦截，或者拦截后没有需要返回的数据)
                if (http_context->response().content.empty()){
                    // 如果启用了静态文件服务，则返回对应文件
                    if (!this->setting_.staticFileDir.empty()){
                        sendStaticFile(io, http_context, url);
                        if (keep_alive){
                            HttpServer::setIOKeepAlive(io);
                        }
                        return;
                    }
                    // 没有启用，返回 405 请求不支持
                    http_context->setStatusCode(405);
                    http_context->response().header.setHead("Content-Type",
                                                            "text/plain;charset=UTF-8");
                    http_context->response().content.data() = HttpContext::error_str;
                    url = http_context->response().toString();
                    url += http_context->response().content.data();
                    io->write(url.c_str(), url.length());
                    goto END;
                }
                SERVICE_END:
                // 回应数据
                HttpServer::sendHttpResponse(io, http_context, call_ret);
                // 清空解析器数据，为下一次请求解析准备
                http_context->clear();
                END:
                if (!keep_alive){
                    // 不保持连接，数据发送完成关闭
                    io->close();
                } else {
                    // 保持连接 1 min
                    SinBack::Http::HttpServer::setIOKeepAlive(io);
                }
                return;
            }
        }
    }
}

void Http::HttpServer::onSend(const std::weak_ptr<Core::IOEvent>& ev, Size_t write_len)
{
}

void Http::HttpServer::onMessageError(const std::weak_ptr<Core::IOEvent> &ev, const SinBack::String &err_msg)
{
    auto io = ev.lock();
    if (io){
        Log::FLogE("HttpServer receive message error_, loop_id = %ld, io_id = %ld, err_msg = %s .",
                   io->loop_->getId(), io->id_, err_msg.c_str());
    }
}

void Http::HttpServer::onSendError(const std::weak_ptr<Core::IOEvent> &ev, const SinBack::String &err_msg)
{
    auto io = ev.lock();
    if (io){
        Log::FLogE("HttpServer send message error_, loop_id = %ld, io_id = %ld, err_msg = %s .",
                   io->loop_->getId(), io->id_, err_msg.c_str());
    }
}

void Http::HttpServer::onDisconnect(const std::weak_ptr<Core::IOEvent> &ev)
{
    this->decConnectCount();
    auto io = ev.lock();
    if (io){
        auto http_context = (Http::HttpContext*)(io->context_);
        io->context_ = nullptr;
        delete http_context;
    }
}

Base::socket_t Http::HttpServer::createListenSocketV4(UInt port, bool reuse_port)
{
    // 创建套接字
    Base::socket_t listen_fd_ = Base::creatSocket(Base::IP_4, Base::S_TCP, true);
    if (listen_fd_ < 0){
        return -1;
    }
    // 设置非阻塞
    Base::setSocketNonblock(listen_fd_);
    // 设置端口复用
    Base::setSocketReuseAddress(listen_fd_);
    // 设置多个进程监听端口
    if (reuse_port) {
        Base::setSocketReusePort(listen_fd_);
    }

    // 绑定
    if (!Base::bindSocket(listen_fd_, Base::IP_4, nullptr, Int(port))){
        goto ERR;
    }
    if (!Base::listenSocket(listen_fd_)){
        goto ERR;
    }
    return listen_fd_;
    ERR:
    Base::closeSocket(listen_fd_);
    return -1;
}

/**
 * 开始运行
 */
void Http::HttpServer::start()
{
    // 注册日志记录器
    Log::Logger::registerLogger( "SinBackDefault",this->setting_.logPath,
                                 Log::LoggerType::Rolling, Log::Info, 2);
    // 优先选择多进程
    if (this->setting_.workProcessNum > 0){
        // 多进程运行模式
        this->runProcessFunc();
        return;
    }
    if (this->setting_.workThreadNum > 0){
        // 多线程运行模式
        this->runThreadFunc();
    }
}

void Http::HttpServer::startListenAccept(Core::EventLoopPtr loop, bool reuse_port)
{
    if (loop) {
        Base::socket_t listen_fd = HttpServer::createListenSocketV4(this->listen_port_, reuse_port);
        // 监听
        if (listen_fd < 0){
            Log::FLogE("Http Server startListenAccept error_ -> create listen socket error_ !");
            loop->stop();
        }
        // 注册 accept 事件并设置回调
        loop->acceptIo(listen_fd, std::bind(&HttpServer::onNewClient, this, std::placeholders::_1));
    }
}

void Http::HttpServer::setIOKeepAlive(const std::weak_ptr<Core::IOEvent> &ev)
{
    auto io = ev.lock();
    if (io){
        if (io->loop_){
            ULong id = io->id_;
            io->loop_->addTimer([id, io](const std::weak_ptr<Core::TimerEvent>& ev){
                if (io && io->id_ == id){
                    io->close();
                }
            }, HttpServer::default_keep_alive, 1);
        }
    }
}

/**
 * 发送 Http 响应
 * @param io
 * @param context
 * @param call_ret
 */
void Http::HttpServer::sendHttpResponse(const std::shared_ptr<Core::IOEvent>& io, Http::HttpContext *context, Int call_ret)
{
    if (context->response().status_code == 0){
        context->response().status_code = 200;
    }
    if (call_ret > 1){
        context->response().status_code = call_ret;
    }

    SinBack::String buf = context->response().toString();
    buf += context->response().content.data();
    io->write(buf.c_str(), buf.length());
}

/**
 * 发送Http静态文件
 * @param io
 * @param context
 * @param path
 */
void Http::HttpServer::sendStaticFile(const std::shared_ptr<Core::IOEvent> &io, Http::HttpContext *context, String& path) const
{
    path.insert(0, this->setting_.staticFileDir);
    if (context->request().url == "/"){
        // 添加index.html
        path += "index.html";
    }
    if (!context->cache_file_->reOpen(path, Base::ReadOnly)){
        // 文件不存在
        context->notFound();
        path = context->response().toString();
        path += context->response().content.data();
        io->write(path.c_str(), path.length());
        // 发生完成，清空解析数据
        context->clear();
    } else {
        // 文件操作。放到线程池中执行
        auto result = io->loop_->queueFunc([](const std::shared_ptr<Core::IOEvent> &io, const String &path, ULong id) {
            auto* context = (HttpContext*)io->context_;
            if (io->id_ != id || io->closed_ || context == nullptr) return ;
            if (!context->isInit()){
                context->init();
            }
            context->response().status_code = 200;
            context->response().header.setHead("Content-Type",
                                               Http::get_http_content_type(context->cache_file_->suffix()));
            context->response().content.data() = std::move(context->cache_file_->readAll());
            string_type buf = context->response().toString();
            buf += context->response().content.data();
            io->write(buf.c_str(), buf.length());
            // 清空数据
            context->clear();
            io->close();
        }, io, path, io->id_);
        // 等待线程函数执行完成
        result.get();
    }
}

/**
 * 多进程模式
 */
void Http::HttpServer::runProcessFunc()
{
    if (this->setting_.workProcessNum > 0){
        Size_t i = 0;
        Int pid = 1;

        // 注册子进程退出信号
        Base::system_signal(SIGCHLD, HttpServer::processExitCall);
        // 注册 TERM 信号处理函数
        Base::system_signal(SIGTERM, std::bind(&HttpServer::sigStop, this, std::placeholders::_1));
        Base::system_signal(SIGSEGV, std::bind(&HttpServer::sigStop, this, std::placeholders::_1));

        for (; i < this->setting_.workProcessNum; ++i){
            pid = Base::system_fork();
            if (pid < 0){
                Log::FLogE("Http Server run on process mode error_ -> system_fork error_ !");
            }else if (pid == 0) {
                // 清空子进程pid
                this->child_pid_.clear();
                // 工作线程
                if (this->setting_.workThreadNum > 0) {
                    this->work_th_.reset(new Core::EventLoopPool(this->setting_.workThreadNum));
                    this->work_th_->start();
                }
                // 开始运行 EventLoop
                this->accept_th_.reset(new Core::EventLoopThread);
                if (this->setting_.enableSSL) {
                    this->initSSL();
                }
                this->accept_th_->start(std::bind(&HttpServer::startListenAccept,
                                                  this, this->accept_th_->loop().get(), true), nullptr);
                return;
            }else {
                // 存储子进程id，以便停止子进程
                this->child_pid_.push_back(pid);
            }
        }
        if (this->setting_.workThreadNum > 0) {
            this->work_th_.reset(new Core::EventLoopPool(this->setting_.workThreadNum));
            this->work_th_->start();
        }
        // 开始运行 EventLoop
        this->accept_th_.reset(new Core::EventLoopThread);
        this->accept_th_->start(std::bind(&HttpServer::startListenAccept,
                                          this, this->accept_th_->loop().get(), true), nullptr);

        Log::FLogI("Http Server run on process mode !");
    }
}

/**
 * 多线程模式
 */
void Http::HttpServer::runThreadFunc()
{
    // 注册 TERM 信号处理函数
    Base::system_signal(SIGTERM, std::bind(&HttpServer::sigStop, this, std::placeholders::_1));
    Base::system_signal(SIGSEGV, std::bind(&HttpServer::sigStop, this, std::placeholders::_1));

    if (this->setting_.workThreadNum > 0){
        this->work_th_.reset(new Core::EventLoopPool(this->setting_.workThreadNum));
        this->work_th_->start();

        this->accept_th_.reset(new Core::EventLoopThread);
        if (this->setting_.enableSSL) {
            this->initSSL();
        }
        this->accept_th_->start(std::bind(&HttpServer::startListenAccept,
                                          this, this->accept_th_->loop().get(), false), nullptr);
        Log::FLogI("Http Server run on thread mode !");
    }
}

/**
 * 子进程退出处理函数
 * @param sig
 */
void Http::HttpServer::processExitCall(Int sig){
    if (sig == SIGCHLD){
        Int status;
        WAIT:
        if (::waitpid(-1, &status, 0) < 0){
            if (errno == EINTR){
                goto WAIT;
            }
        }
    }
}

/**
 * 初始化 SSL
 */
void Http::HttpServer::initSSL()
{
    // 设置 SSL / TSL
    Base::OpenSSL::loadSSL();
    auto loop = this->accept_th_->loop();
    loop->enableSSL();
    if (!this->setting_.certPath.empty()){
        if (!loop->getSSL()->setCertFile(this->setting_.certPath)){
            Log::FLogE("Http Server run on process mode error -> setCertFile error !");
            exit(0);
        }
    }
    if (!this->setting_.keyPath.empty()){
        if (!loop->getSSL()->setKeyFile(this->setting_.keyPath)){
            Log::FLogE("Http Server run on process mode error -> setKeyFile error !");
            exit(0);
        }
    }
    if (!this->setting_.certPath.empty() && !this->setting_.keyPath.empty()){
        if (!loop->getSSL()->check()){
            Log::FLogE("Http Server run on process mode error -> check SSL error !");
            exit(0);
        }
    }
}


