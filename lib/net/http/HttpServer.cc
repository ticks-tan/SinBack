/**
* FileName:   HttpServer.cc
* CreateDate: 2022-03-14 17:49:22
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "HttpServer.h"

using namespace SinBack;

Http::HttpServer::HttpServer()
    : setting_()
    , running_(false)
    , listen_fd_(-1)
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

void Http::HttpServer::init()
{
    // 默认设置
    this->setting_.keep_alive = false;
    this->setting_.log_dir = "./";
    // 静态文件默认开启，手动指定静态文件根目录才会开启
    this->setting_.static_file_dir = "";
    this->setting_.max_accept_cnt = 4096;
    this->setting_.work_thread_num = std::thread::hardware_concurrency();
}

bool Http::HttpServer::add_service(const String &name, Http::HttpService *service)
{
    auto it = this->services_.find(name);
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
    // 创建监听套接字并监听
    if (!create_listen_socket_v4(port)){
        goto ERR;
    }
    // 开始运行
    start();
    return true;

    ERR:
    if (err_callback){
        err_callback(strerror(errno));
    }
    return false;
}

void Http::HttpServer::stop()
{
    // 停止运行
    if (this->running_){
        this->running_ = false;
        // 等待 accept loop 退出
        this->accept_th_->stop(true);
        this->work_th_->stop();
    }
}

/**
 * 新客户端连接后会调用该函数
 * @param ev : 新连接的客户端对应 IO 事件句柄
 */
void Http::HttpServer::on_new_client(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io){
        // 判断是否超过最大允许的连接数量
        Size_t cnt = this->get_connect_count();
        if (cnt >= this->setting().max_accept_cnt){
            // 超过最大连接数量
            io->loop_->logger().error("over the max connect count -- {} !", cnt);
            io->loop_->close_io(io->fd_);
            return;
        }
        // 获取一个工作线程来管理该 IO 句柄
        Core::EventLoopPtr new_loop = this->loop();
        // 更改 IO事件 loop
        Core::EventLoop::change_io_loop(io, new_loop);
        // 设置  事件回调
        io->read_cb_ = std::bind(&HttpServer::on_new_message, this, std::placeholders::_1, std::placeholders::_2);
        io->read_err_cb_ = std::bind(&HttpServer::on_message_error, this, std::placeholders::_1, std::placeholders::_2);
        io->write_cb_ = std::bind(&HttpServer::on_send, this, std::placeholders::_1, std::placeholders::_2);
        io->write_err_cb_ = std::bind(&HttpServer::on_send_error, this, std::placeholders::_1, std::placeholders::_2);
        io->close_cb_ = std::bind(&HttpServer::on_disconnect, this, std::placeholders::_1);
        auto* http_context = new Http::HttpContext(this);
        // 将 HttpContext与IO事件关联
        io->context_ = http_context;
        // 当前连接客户端 +1
        this->inc_connect_count();
        // 读取数据 (主要作用是将可读事件注册到 Loop)
        io->read();
    }
}

/**
 * 有新数据时会调用该函数 -- 核心处理函数
 * @param ev : 读取数据的IO事件句柄
 * @param read_msg : 本次读取的数据
 */
void Http::HttpServer::on_new_message(const std::weak_ptr<Core::IOEvent>& ev, const String& read_msg)
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
            Int ret = parser->parse_data(read_msg.c_str(), read_msg.length());
            // 判断是否解析完毕
            if (parser->need_receive()){
                // 需要继续读取数据
                return;
            } else {
                parser->reset_parser();
                // 解析完成, 初始化，初始化失败关闭连接
                if (!http_context->init()){
                    io->close();
                    return;
                }

                Int method = http_context->request().method;
                SinBack::String url = http_context->request().url;
                bool keep_alive = http_context->request().header[SIN_STR("Connection")] == SIN_STR("keep-alive");
                Int call_ret = 0;
                bool is_call = false;

                for (auto &service: this->services()) {
                    auto &routers = service.second->match_service(url, method);
                    // 依次执行 service 回调
                    for (auto& call : routers){
                        // 如果方法匹配，调用回调
                        if (call->method & method){
                            if (call->callback){
                                call_ret = call->callback(*http_context);
                                is_call = true;
                            }
                            // 如果不为 NEXT, 后续回调被忽略
                            if (call_ret != Http::ServiceCallBackRet::NEXT){
                                routers.clear();
                                goto SERVICE_END;
                            }
                        }
                    }
                    routers.clear();
                }
                // 没有执行回调 (请求没有被拦截)
                if (!is_call){
                    // 如果启用了静态文件服务，则返回对应文件
                    if (!this->setting_.static_file_dir.empty()){
                        send_static_file(io, http_context, url);
                        goto END;
                    }
                    // 没有启用，返回一段有趣的话吧~
                    io->close();
                    return;
                }
                // 返回数据
                SERVICE_END:
                HttpServer::send_http_response(io, http_context, call_ret);
                END:
                if (!keep_alive){
                    io->close();
                }
                return;
            }
        }
    }
}

void Http::HttpServer::on_send(const std::weak_ptr<Core::IOEvent>& ev, Size_t write_len)
{
}

void Http::HttpServer::on_message_error(const std::weak_ptr<Core::IOEvent> &ev, const String &err_msg)
{
    auto io = ev.lock();
    if (io){
        io->loop_->logger().error("HttpServer receive message error, loop_id = {}, id = {}, err_msg = {} .",
                                  err_msg.c_str(), 0, io->id_);
    }
}

void Http::HttpServer::on_send_error(const std::weak_ptr<Core::IOEvent> &ev, const String &err_msg)
{
    auto io = ev.lock();
    if (io){
        io->loop_->logger().error("HttpServer send message error, loop_id = {}, id = {}, err_msg = {} .",
                                  err_msg.c_str(), 0, io->id_);
    }
}

void Http::HttpServer::on_disconnect(const std::weak_ptr<Core::IOEvent> &ev)
{
    this->dec_connect_count();
    auto io = ev.lock();
    if (io){
        auto http_context = (Http::HttpContext*)(io->context_);
        delete http_context;
    }
}

bool Http::HttpServer::create_listen_socket_v4(UInt port)
{
    this->listen_fd_ = Base::creat_socket(Base::IP_4, Base::S_TCP, true);
    if (this->listen_fd_ < 0){
        return false;
    }
    // 绑定
    if (!Base::bind_socket(this->listen_fd_, Base::IP_4, nullptr, port)){
        goto ERR;
    }
    // 设置非阻塞
    Base::set_socket_nonblock(this->listen_fd_);
    // 设置端口复用
    Base::socket_reuse_address(this->listen_fd_);

    // 监听
    if (!Base::listen_socket(this->listen_fd_)){
        goto ERR;
    }
    return true;
    ERR:
    Base::close_socket(this->listen_fd_);
    this->listen_fd_ = -1;
    return false;
}

void Http::HttpServer::start()
{
    // 开始事件循环
    this->accept_th_.reset(new Core::EventLoopThread);
    this->work_th_.reset(new Core::EventLoopPool(setting_.work_thread_num));
    // 设置日志文件路径
    this->accept_th_->set_logger_dir(setting_.log_dir);
    this->work_th_->set_logger_dir(setting_.log_dir);

    if (this->setting_.work_thread_num > 0){
        this->work_th_->start();
    }
    // 开始监听
    this->accept_th_->start(std::bind(&HttpServer::start_accept, this), nullptr);

}

void Http::HttpServer::start_accept()
{
    // 注册 accept 事件并设置回调
    auto acceptor = this->accept_th_->loop()->accept_io(this->listen_fd_,
                                                        std::bind(&HttpServer::on_new_client, this, std::placeholders::_1));
}

/**
 * 发送 Http 响应
 * @param io
 * @param context
 * @param call_ret
 */
void Http::HttpServer::send_http_response(const std::shared_ptr<Core::IOEvent>& io, Http::HttpContext *context, Int call_ret)
{
    if (context->response().status_code == 0){
        context->response().status_code = 200;
    }
    if (call_ret > 1){
        context->response().status_code = call_ret;
    }
    // 是否保持连接
    if (!this->setting_.keep_alive) {
        context->response().header.set_head(SIN_STR("Connection"), SIN_STR("close"));
    }

    SinBack::String buf = context->response().to_string();
    buf += context->response().content.data();
    io->write(buf.c_str(), buf.length());
}

void Http::HttpServer::send_static_file(const std::shared_ptr<Core::IOEvent> &io, Http::HttpContext *context, String& path)
{
    if (!this->setting_.keep_alive){
        context->response().header.set_head(SIN_STR("Connection"), SIN_STR("close"));
    }
    path.insert(0, this->setting_.static_file_dir);
    Base::File file(path, Base::ReadOnly);
    if (!file.exist()){
        // 文件不存在
        context->response().status_code = 404;
        context->response().header.set_head(SIN_STR("Content-Type"), SIN_STR("text/plain;charset=UTF-8"));
        context->response().content.data() += SIN_STR("-_- -_- -_- 找不到您要访问的页面呢! -_- -_- -_- ");
        path = context->response().to_string();
        path += context->response().content.data();
        io->write(path.c_str(), path.length());
    } else{

        // 文件操作。放到线程池中执行
        io->loop_->queue_func([](const std::shared_ptr<Core::IOEvent>& io, HttpContext* context, const String& path){
            Base::File file(path, Base::ReadOnly);
            context->response().status_code = 200;
            context->response().header.set_head(SIN_STR("Content-Type"), Http::get_http_content_type(file.suffix()));
            file >> context->response().content.data();
            String buf = context->response().to_string();
            buf += context->response().content.data();
            io->write(buf.c_str(), buf.length());
        }, io, context, path);
    }
}

