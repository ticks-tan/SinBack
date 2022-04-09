/**
* FileName:   Application.cc
* CreateDate: 2022-04-09 14:25:29
* Author:     ticks
* Email:      2938384958@qq.com
* Des:
*/
#include "Application.h"

using namespace SinBack;

/**
 * 创建 IPV4 套接字
 * @param port
 * @param reuse_port
 * @return
 */
Base::socket_t Main::Application::createListenSocketV4(UInt port, bool reuse_port)
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
    // 设置多个进程可同时监听端口
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

Main::Application::Application()
        : setting_()
        , connect_cnt_(0)
        , running_(false)
        , accept_th_(nullptr)
        , work_th_()
        , tid_(0)
        , module_(nullptr)
{
    // 默认设置
    this->setting_.logPath = "./";
    // 静态文件默认开启，手动指定静态文件根目录才会开启
    this->setting_.maxConnectCnt = 4096;
    // 默认为多线程模式，指定进程数量后采用多进程模式
    this->setting_.workProcessNum = 0;
    // 默认线程数量为当前核心数两倍
    this->setting_.workThreadNum = std::thread::hardware_concurrency();
    // 默认不启用 SSL
    this->setting_.enableSSL = false;
    this->setting_.listenPort = 2022;
}

Main::Application::~Application()
{
    if (this->running_){
        this->stop();
    }
}

/**
 * 开始运行
 * @param err_callback
 * @return
 */
bool Main::Application::run(const std::function<void(const String &)> &err_callback)
{
    if (this->running_){
        if (err_callback){
            err_callback("Application is running !");
        }
        return false;
    }
    if (!this->module_){
        if (err_callback){
            err_callback("Application module is null !");
        }
        return false;
    }
    this->tid_ = gettid();
    this->running_ = true;

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

/**
 * 停止
 */
void Main::Application::stop()
{
    // 停止运行
    if (this->running_){
        this->running_ = false;

        this->accept_th_->stop(true);
        this->work_th_->stop(true);
        if (this->setting_.workProcessNum > 0){
            // 多进程模式
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
 * 初始化 SSL
 * @return
 */
bool Main::Application::initSSL()
{
    // 设置 SSL / TSL
    Base::OpenSSL::loadSSL();
    auto loop = this->accept_th_->loop();
    loop->enableSSL();
    if (!this->setting_.certPath.empty()){
        if (!loop->getSSL()->setCertFile(this->setting_.certPath)){
            Log::FLogE("Http Server run on process mode error -> setCertFile error !");
            return false;
        }
    }
    if (!this->setting_.keyPath.empty()){
        if (!loop->getSSL()->setKeyFile(this->setting_.keyPath)){
            Log::FLogE("Http Server run on process mode error -> setKeyFile error !");
            return false;
        }
    }
    if (!this->setting_.certPath.empty() && !this->setting_.keyPath.empty()){
        if (!loop->getSSL()->check()){
            Log::FLogE("Http Server run on process mode error -> check SSL error !");
            return false;
        }
    }
    return true;
}

/**
 * 内部开始函数
 */
void Main::Application::start()
{
    // 注册日志记录器
    Log::Logger::registerLogger( "SinBackDefault",this->setting_.logPath,
                                 Log::LoggerType::Rolling, Log::Info, 2);
    // 优先选择多进程
    if (this->setting_.workProcessNum > 0){
        // 多进程运行模式
        this->runProcessMode();
        return;
    }
    if (this->setting_.workThreadNum > 0){
        // 多线程运行模式
        this->runThreadMode();
    }
}

/**
 * 开始监听并 accept
 * @param loop
 * @param reuse_port
 */
void Main::Application::startListenAccept(Core::EventLoopPtr loop, bool reuse_port)
{
    if (loop) {
        Base::socket_t listen_fd = Application::createListenSocketV4(this->setting_.listenPort, reuse_port);
        // 监听
        if (listen_fd < 0){
            Log::FLogE("Http Server startListenAccept error -> create listen socket error !");
            loop->stop();
        }
        // 注册 accept 事件并设置回调
        loop->acceptIo(listen_fd, std::bind(&Application::onNewClient, this, std::placeholders::_1));
    }
}

/**
 * 有新客户端连接时会调用该函数
 * @param ev
 */
void Main::Application::onNewClient(const std::weak_ptr<Core::IOEvent> &ev)
{
    auto io = ev.lock();
    if (io){
        // 判断是否超过最大允许的连接数量
        Size_t cnt = this->getConnectCount();
        if (cnt > this->setting().maxConnectCnt){
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
        io->read_cb_ = std::bind(&Application::onNewMessage, this, std::placeholders::_1, std::placeholders::_2);
        io->read_err_cb_ = std::bind(&Application::onMessageError, this, std::placeholders::_1, std::placeholders::_2);
        io->write_cb_ = std::bind(&Application::onSend, this, std::placeholders::_1, std::placeholders::_2);
        io->write_err_cb_ = std::bind(&Application::onSendError, this, std::placeholders::_1, std::placeholders::_2);
        io->close_cb_ = std::bind(&Application::onDisconnect, this, std::placeholders::_1);
        // 当前连接客户端 +1
        this->incConnectCount();
        this->module_->onNewClient(*io);
    }
}

/**
 * 套接字读取了新消息会调用该函数
 * @param ev
 * @param read_msg
 */
void Main::Application::onNewMessage(const std::weak_ptr<Core::IOEvent> &ev,
                                     const Main::Application::string_type &read_msg)
{
    auto io = ev.lock();
    if (io){
        // 调用模块对应函数
        this->module_->onNewMessage(*io,std::forward<const string_type&>(read_msg));
    }
}

/**
 * 套接字读取消息失败会调用该函数
 * @param ev
 * @param err_msg
 */
void Main::Application::onMessageError(const std::weak_ptr<Core::IOEvent> &ev,
                                       const Main::Application::string_type &err_msg)
{
    auto io = ev.lock();
    if (io){
        // 调用模块对应函数
        this->module_->onMessageError(*io,std::forward<const string_type&>(err_msg));
    }
}

/**
 * 套接字发送消息会调用该函数
 * @param ev
 * @param write_len
 */
void Main::Application::onSend(const std::weak_ptr<Core::IOEvent> &ev, Size_t write_len)
{
    auto io = ev.lock();
    if (io){
        // 调用模块对应函数
        this->module_->onSend(*io, std::forward<Size_t>(write_len));
    }
}

/**
 * 套接字发生消息失败会调用该函数
 * @param ev
 * @param err_msg
 */
void
Main::Application::onSendError(const std::weak_ptr<Core::IOEvent> &ev,
                               const Main::Application::string_type &err_msg)
{
    auto io = ev.lock();
    if (io){
        // 调用模块对应函数
        this->module_->onSendError(*io, std::forward<const string_type&>(err_msg));
    }
}

/**
 * 套接字关闭会调用该函数
 * @param ev
 */
void Main::Application::onDisconnect(const std::weak_ptr<Core::IOEvent> &ev)
{
    auto io = ev.lock();
    if (io){
        this->decConnectCount();
        // 调用模块对应函数
        this->module_->onDisconnect(*io);
    }
}

/**
 * 单进程多线程模式
 */
void Main::Application::runThreadMode()
{
    // 注册 TERM 信号处理函数
    Base::system_signal(SIGTERM, std::bind(&Application::sigHandleExit, this, std::placeholders::_1));

    if (this->setting_.workThreadNum > 0){
        this->work_th_.reset(new Core::EventLoopPool(this->setting_.workThreadNum));
        this->work_th_->start();

        this->accept_th_.reset(new Core::EventLoopThread);
        if (this->setting_.enableSSL) {
            this->initSSL();
        }
        this->accept_th_->start(std::bind(&Application::startListenAccept,
                                          this, this->accept_th_->loop().get(), false), nullptr);
        Log::FLogI("Http Server run on thread mode !");
    }
}

/**
 * 多进程模式
 */
void Main::Application::runProcessMode()
{
    if (this->setting_.workProcessNum > 0){
        Size_t i = 0;
        pid_t pid;

        // 注册子进程退出信号
        Base::system_signal(SIGCHLD, std::bind(&Application::sigWaitChild, this, std::placeholders::_1));
        // 注册 TERM 信号处理函数
        Base::system_signal(SIGTERM, std::bind(&Application::sigHandleExit, this, std::placeholders::_1));

        for (; i < this->setting_.workProcessNum; ++i){
            pid = Base::system_fork();
            if (pid < 0){
                Log::FLogE("Http Server run on process mode error -> system_fork error !");
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
                this->accept_th_->start(std::bind(&Application::startListenAccept,
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
        this->accept_th_->start(std::bind(&Application::startListenAccept,
                                          this, this->accept_th_->loop().get(), true), nullptr);
        Log::FLogI("Http Server run on process mode !");
    }
}

/**
 * 捕获子进程退出信号
 * @param sig
 */
void Main::Application::sigWaitChild(int sig)
{
    if (sig == SIGCHLD) {
        Int status = 0, ret = 0;
        ret = ::wait(&status);
    }
}

/**
 * 捕获SIGTERM信号
 * @param sig
 */
void Main::Application::sigHandleExit(int sig)
{

    Log::Logger::unregisterAllLogger();
    this->stop();
    exit(0);
}
