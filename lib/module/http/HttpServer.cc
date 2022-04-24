/**
* FileName:   HttpServer.cc
* CreateDate: 2022-03-14 17:49:22
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "HttpServer.h"

using namespace SinBack;
using namespace SinBack::Module;

Http::HttpServer::HttpServer()
    : setting_()
{
    // 默认设置
    this->setting_.keepAlive = false;
    // 静态文件默认开启，手动指定静态文件根目录才会开启
    this->setting_.staticFileDir = "";
}

Http::HttpServer::~HttpServer()
{
    this->services_.clear();
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

/**
 * 新客户端连接后会调用该函数
 * @param ev : 新连接的客户端对应 IO 事件句柄
 */
bool Http::HttpServer::onNewClient(Core::IOEvent& io)
{
    if (io.context_ == nullptr) {
        auto *http_context = new Http::HttpContext(this);
        // 将 HttpContext与IO事件关联
        io.context_ = http_context;
    }
    io.read();
    return true;
}

/**
 * 有新数据时会调用该函数 -- 核心处理函数
 * @param ev : 读取数据的IO事件句柄
 * @param read_msg : 本次读取的数据
 */
bool Http::HttpServer::onNewMessage(Core::IOEvent& io, const String& read_msg)
{
    // 有新数据到来：解析数据 -> 匹配路由并执行对应回调 -> 解析返回信息 -> 调用write写入数据
    // 取出 HttpContext
    auto http_context = (HttpContext*)(io.context_);

    if (http_context){
        // 取出 Http Parser 解析 Http 数据
        auto* parser = http_context->parser();
        // 解析数据
        bool ret = parser->parseData(read_msg.c_str(), read_msg.size());
        // 解析失败，关闭连接
        if (!ret){
            io.close(false);
            return true;
        }
        // 判断请求是否完成，是否还需要接受数据
        if (parser->needReceive()){
            // 需要继续读取数据
            return true;
        } else {
            // 重置解析器
            parser->resetParser();
            // 解析完成, 初始化，初始化失败关闭连接
            if (!http_context->init()){
                io.close(false);
                return true;
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
                        HttpServer::setIOKeepAlive(io.shared_from_this());
                    }
                    return true;
                }
                // 没有启用，返回 405 请求不支持
                http_context->setStatusCode(405);
                http_context->response().header.setHead("Content-Type",
                                                        "text/plain;charset=UTF-8");
                http_context->response().content.data() = HttpContext::error_str;
                url = http_context->response().toString();
                url += http_context->response().content.data();
                io.write(url.c_str(), url.length());
                goto END;
            }
            SERVICE_END:
            // 回应数据
            HttpServer::sendHttpResponse(io.shared_from_this(), http_context, call_ret);
            // 清空解析器数据，为下一次请求解析准备
            http_context->clear();
            END:
            if (!keep_alive){
                // 不保持连接，数据发送完成关闭
                io.close();
            } else {
                // 保持连接 1 min
                HttpServer::setIOKeepAlive(io.shared_from_this());
            }
            return true;
        }
    }
    return true;
}

bool Http::HttpServer::onSend(Core::IOEvent& ev, Size_t write_len)
{
    return true;
}

bool Http::HttpServer::onMessageError(Core::IOEvent &io, const SinBack::String &err_msg)
{
    Log::FLogE("HttpServer receive message error, loop_id = %ld, io_id = %ld, err_msg = %s .",
                   io.loop_->getId(), io.id_, err_msg.c_str());
    return true;
}

bool Http::HttpServer::onSendError(Core::IOEvent &io, const SinBack::String &err_msg)
{
    Log::FLogE("HttpServer send message error, loop_id = %ld, io_id = %ld, err_msg = %s .",
                   io.loop_->getId(), io.id_, err_msg.c_str());
    return true;
}

bool Http::HttpServer::onDisconnect(Core::IOEvent &io)
{
    auto http_context = (Http::HttpContext*)(io.context_);
    io.context_ = nullptr;
    delete http_context;
    return true;
}


void Http::HttpServer::setIOKeepAlive(const std::weak_ptr<Core::IOEvent> &ev)
{
    auto io = ev.lock();
    if (io){
        if (io->loop_){
            ULong id = io->id_;
            io->loop_->addTimer([id, io](const std::weak_ptr<Core::TimerEvent>& ev){
                if (io && io->id_ == id) {
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
void Http::HttpServer::sendStaticFile(Core::IOEvent &io, Http::HttpContext *context, String& path) const
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
        io.write(path.c_str(), path.length());
        // 发生完成，清空解析数据
        context->clear();
    } else {
        // 文件操作。放到线程池中执行
        auto result = io.loop_->queueFunc([](const std::shared_ptr<Core::IOEvent> &io, const String &path, ULong id) {
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

            buf.append(context->response().content.data());
            io->write(buf.data(), buf.size());
            // 清空数据
            context->clear();
            io->close();
        }, io.shared_from_this(), path, io.id_);
        // 等待线程函数执行完成
        result.get();
    }
}


