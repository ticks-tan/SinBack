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
    this->setting_ .keep_alive = false;
    this->setting_.log_dir = "./";
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
    if (!create_listen_socket_v4(port)){
        goto ERR;
    }

    start();
    return true;

    ERR:
    if (err_callback){
        err_callback(SIN_STR("listen error !"));
    }
    return false;
}

void Http::HttpServer::stop()
{
    if (this->running_){
        this->running_ = false;
        this->accept_th_->stop(true);
        this->work_th_->stop();
    }
}

void Http::HttpServer::on_new_client(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io){
        Size_t cnt = this->get_connect_count();
        if (cnt >= this->setting().max_accept_cnt){
            // 超过最大连接数量
            io->loop_->logger().error("over the max connect count -- {} !", cnt);
            io->loop_->close_io(io->fd_);
            return;
        }
        Core::EventLoopPtr new_loop = this->loop();
        // 更改 io loop
        Core::EventLoop::change_io_loop(io, new_loop);
        // 设置回调
        io->read_cb_ = std::bind(&HttpServer::on_new_message, this, std::placeholders::_1, std::placeholders::_2);
        io->read_err_cb_ = std::bind(&HttpServer::on_message_error, this, std::placeholders::_1, std::placeholders::_2);
        io->write_cb_ = std::bind(&HttpServer::on_send, this, std::placeholders::_1, std::placeholders::_2);
        io->write_err_cb_ = std::bind(&HttpServer::on_send_error, this, std::placeholders::_1, std::placeholders::_2);
        io->close_cb_ = std::bind(&HttpServer::on_disconnect, this, std::placeholders::_1);
        auto* http_context = new Http::HttpContext();
        io->context_ = http_context;
        // 当前连接客户端+1
        this->inc_connect_count();
        // 读取io
        io->read();
    }
}

void Http::HttpServer::on_new_message(const std::weak_ptr<Core::IOEvent>& ev, const String& read_msg)
{
    // 有新数据到来：解析数据 -> 匹配路由并执行对应回调 -> 解析返回信息 -> 调用write写入数据
    auto io = ev.lock();
    if (io){
        auto http_context = (HttpContext*)(io->context_);
        if (http_context){

            auto* parser = http_context->parser();
            parser->parse_data(read_msg.c_str(), read_msg.length());
            if (parser->need_receive()){
                return;
            } else {
                Int method = http_context->request().method;
                const SinBack::String url = http_context->request().url;
                Int call_ret = 0;

                for (auto &service: http_context->context()->services()) {
                    auto &routers = service.second->match_service(url, method);
                    // 依次执行 service 回调
                    for (auto& call : routers){
                        if (call->method & method){
                            if (call->callback){
                                call_ret = call->callback(*http_context);
                            }
                            if (call_ret == Http::ServiceCallBackRet::END){
                                goto SERVICE_END;
                            }
                        }
                    }
                }
                SERVICE_END:
                HttpServer::send_http_response(http_context);
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
    if (!Base::bind_socket(this->listen_fd_, Base::IP_4, nullptr, port)){
        goto ERR;
    }

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
    this->accept_th_.reset(new Core::EventLoopThread);
    this->work_th_.reset(new Core::EventLoopPool(setting_.work_thread_num));
    // 设置日志文件路径
    if (this->setting_.work_thread_num > 0){
        this->work_th_->start();
    }
    this->accept_th_->start(std::bind(&HttpServer::start_accept, this), nullptr);

}

void Http::HttpServer::start_accept()
{
    auto acceptor = this->accept_th_->loop()->accept_io(this->listen_fd_,
                                                        std::bind(&HttpServer::on_new_client, this, std::placeholders::_1));
}

void Http::HttpServer::send_http_response(Http::HttpContext *context) {

}




