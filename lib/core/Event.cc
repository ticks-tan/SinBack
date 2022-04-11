/**
* FileName:   Event.cc
* CreateDate: 2022-03-09 21:38:11
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        Event function
*/
#include "core/EventLoop.h"
#include "Event.h"
#include "cstring"

using namespace SinBack;

// 处理 SSL新连接
void processNewSSLAccept(const std::weak_ptr<Core::IOEvent>& ev);

// 水平触发 write函数
Int io_write(Core::IOEvent* io, const void *buf, Size_t len);

// 根据活动事件选择 accept、read和write
void handle_event_func(const std::weak_ptr<Core::IOEvent>& ev);
//  accept 回调
void handle_accept_cb(const std::weak_ptr<Core::IOEvent>& ev);
// read 回调
void handle_read_cb(const std::weak_ptr<Core::IOEvent>& ev, void* buf, Size_t len);
// read 错误回调
void handle_read_err_cb(const std::weak_ptr<Core::IOEvent>& ev, const std::basic_string<Char>& err);
// read 回调
void handle_write_cb(const std::weak_ptr<Core::IOEvent>& ev, Size_t write_len);
// read 错误回调
void handle_write_err_cb(const std::weak_ptr<Core::IOEvent>& ev, const std::basic_string<Char>& err);
// close回调
void handle_close_cb(const std::weak_ptr<Core::IOEvent>& ev);

// accept 封装
void handle_accept(const std::weak_ptr<Core::IOEvent>& ev);
// 水平触发 read 封装
void handle_read(const std::weak_ptr<Core::IOEvent>& ev);
// 水平触发 read 封装
void handle_write(const std::weak_ptr<Core::IOEvent>& ev);

void handle_keep_alive(Size_t id, const std::shared_ptr<Core::IOEvent>& io, const std::weak_ptr<Core::TimerEvent>& ev);

void processNewSSLAccept(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io){
        Base::OpenSSL::ErrorCode code = Base::OpenSSL::OK;
        Int ret = Base::sslAccept(io->ssl_, code);
        if (ret == 1){
            Core::EventLoop::removeIoEvent(io, Core::IO_READ);
            handle_accept_cb(io);
        }else if (code == Base::OpenSSL::Need_RDWR){
            if ((io->evs_ & Core::IO_READ) == 0){
                Core::EventLoop::addIoEvent(io, processNewSSLAccept, Core::IO_READ);
            }
        }else {
            Log::FLogE("ssl accept error, pid = %ld, fd = %ld, id = %ld", io->pid_, io->fd_, io->id_);
            io->error_ = errno;
            io->close(false);
        }
    }
}

Int io_write(Core::IOEvent* io, const void *buf, Size_t len)
{
    if (io->closed_ || !io->ready_) return -1;
    if (buf == nullptr || len == 0) return 0;

    Long write_len = 0;
    std::unique_lock<std::mutex> lock(io->write_mutex_);

    if (io->write_queue_.empty()){

        if (io->has_ssl_ && io->ssl_){
            Base::OpenSSL::ErrorCode error = Base::OpenSSL::OK;
            write_len = Base::sslWriteSocket(io->ssl_, buf, len, error);
            io->last_write_time_ = Base::getTimeOfDayUs();

            if (write_len <= 0){
                if (error == Base::OpenSSL::Need_RDWR){
                    write_len = 0;
                    goto QUEUE_WRITE;
                }else if (error == Base::OpenSSL::Need_Close) {
                    lock.unlock();
                    goto DISCONNECT;
                }
                if (error == Base::OpenSSL::Error){
                    io->error_ = errno;
                    Log::FLogE("ssl socket read error, id = %ud, pid = %ud, error = %s.",
                               io->id_, io->pid_, strerror(io->error_));
                    lock.unlock();
                    goto WRITE_ERROR;
                }
            }
        }else {
            // 写入队列为空，尝试写入一次，一次没有写入完再添加到事件循环中等待事件触发
            write_len = Base::writeSocket(io->fd_, buf, len);
            io->last_write_time_ = Base::getTimeOfDayUs();

            if (write_len < 0) {
                if (errno == EINTR || errno == EAGAIN) {
                    write_len = 0;
                    goto QUEUE_WRITE;
                } else {
                    io->error_ = errno;
                    Log::FLogE("socket read error, id = %ld, pid = %ld, error = %s.",
                               io->id_, io->pid_, strerror(io->error_));
                    lock.unlock();
                    goto WRITE_ERROR;
                }
            } else if (write_len == 0) {
                // 对端关闭
                lock.unlock();
                goto DISCONNECT;
            }
        }
        // 一次性写入完成
        if (write_len == len){
            goto WRITE_END;
        }
        // 一次性没有写入完成, 添加到事件循环下次可写事件触发时再写
        QUEUE_WRITE:
        SinBack::Core::EventLoop::addIoEvent(io->shared_from_this(), handle_event_func, Core::IO_WRITE);
    }

    if (write_len < len){
        SinBack::String buffer = (static_cast<const Char*>(buf) + write_len);
        io->write_queue_.push_back(std::move(buffer));
    }
    WRITE_END:
    lock.unlock();
    if (write_len > 0){
        handle_write_cb(io->shared_from_this(), write_len);
    }
    return static_cast<Int>(write_len);
    WRITE_ERROR:
    handle_write_err_cb(io->shared_from_this(), strerror(io->error_));
    DISCONNECT:
    io->close();
    return write_len < 0 ? -1 : static_cast<Int>(write_len);
}

// 处理事件
void handle_event_func(const std::weak_ptr<Core::IOEvent>& ev)
{
    std::shared_ptr<Core::IOEvent> io = ev.lock();
    if (io){
        if ((io->active_evs_ & Core::IO_READ) && (io->evs_ & Core::IO_READ)){
            if (io->accept_){
                handle_accept(io);
            } else{
                io->last_read_time_ = Base::getTimeOfDayUs();
                handle_read(io);
            }
        }
        if ((io->active_evs_ & Core::IO_WRITE) && (io->evs_ & Core::IO_WRITE)){
            io->last_write_time_ = Base::getTimeOfDayUs();
            handle_write(io);
        }
    }
}

// keepalive回调 ?
void handle_keep_alive(Size_t id, const std::shared_ptr<Core::IOEvent>& io, const std::weak_ptr<Core::TimerEvent>& ev)
{
    auto timer = ev.lock();
    if (timer){
        if (io){
            // 不是同一个IO或IO已经关闭
            if (io->closed_ || !io->ready_ || io->id_ != id) return;
            auto loop = (Core::EventLoopPtr)(io->loop_);
            if (loop) {
                ULong last_rw_time = std::max(io->last_read_time_, io->last_write_time_);
                ULong time_ms = (loop->getCurrentTime() - last_rw_time) / 1000;
                if (time_ms + 100 < io->keep_alive_ms_) {
                    loop->addTimer(std::bind(&handle_keep_alive, io->id_, io, std::placeholders::_1),
                                   io->keep_alive_ms_, 1);
                } else {
                    io->close();
                }
            }
        }
    }
}

// 处理接收连接回调
void handle_accept_cb(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io){
        if (io->accept_cb_){
            io->accept_cb_(io);
        }
    }
}

// 处理读取事件回调
void handle_read_cb(const std::weak_ptr<Core::IOEvent>& ev, void* buf, Size_t len)
{
    auto io = ev.lock();
    if (io) {
        if (io->read_cb_) {
            String buffer((Char*)buf, len);
            io->read_cb_(io, buffer);
        }
    }
}

void handle_read_err_cb(const std::weak_ptr<Core::IOEvent>& ev, const std::basic_string<Char>& err)
{
    auto io = ev.lock();
    if (io){
        if (io->read_err_cb_){
            io->read_err_cb_(io, err);
        }
    }
}

// 处理写入事件回调
void handle_write_cb(const std::weak_ptr<Core::IOEvent>& ev, Size_t write_len)
{
    auto io = ev.lock();
    if (io) {
        if (io->write_cb_) {
            io->write_cb_(io, write_len);
        }
    }
}

void handle_write_err_cb(const std::weak_ptr<Core::IOEvent>& ev, const std::basic_string<Char>& err)
{
    auto io = ev.lock();
    if (io){
        if (io->write_err_cb_){
            io->write_err_cb_(io, err);
        }
    }
}

void handle_close_cb(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io){
        if (io->close_cb_){
            io->close_cb_(io);
        }
    }
}

// 接受连接事件
void handle_accept(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io){
        if (io->closed_ || !io->ready_) return;
        //  新客户端地址信息
        ::sockaddr_in address{};
        // 地址信息长度（）字节
        ::socklen_t len = sizeof address;
        Base::socket_t client_fd = Base::acceptSocket(io->fd_, &address, &len);
        if (client_fd < 0) {
            io->error_ = errno;
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK){
                return;
            }
            Log::FLogE("accept error, pid = %ld, id = %ld, error = %s .",
                       io->pid_, io->id_, strerror(io->error_));
            goto ACCEPT_ERROR;
        }
        std::shared_ptr<Core::IOEvent> io_ptr = io->loop_->getIoEvent(client_fd);
        io_ptr->accept_cb_ = io->accept_cb_;
        io_ptr->context_ = io->context_;
        // OpenSSL
        if (io_ptr->has_ssl_ && !io_ptr->ssl_){
            Base::OpenSSL::ErrorCode error = Base::OpenSSL::OK;
            io_ptr->ssl_ = Base::sslCreate(*io_ptr->loop_->getSSL(), io_ptr->fd_, error);
            if (io_ptr->ssl_ == nullptr || error != Base::OpenSSL::OK){
                Log::FLogE("create SSL socket error! -- %ld", io_ptr->fd_);
                io_ptr->close(false);
                return;
            }
            processNewSSLAccept(io_ptr);
            return;
        }
        handle_accept_cb(io_ptr);
        return;
    }
    ACCEPT_ERROR:
    return;
}


// 处理读取事件
void handle_read(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io){
        if (io->closed_ || !io->ready_) return;

        Base::Buffer buf;
        buf.reserve(256);

        // 需要读取的长度
        Size_t need_read_len = 0;
        // 读取长度
        Long read_len = 0;

        need_read_len = buf.capacity();
        // 避免读取长度为0,返回0导致错误关闭套接字
        if (need_read_len == 0) return;

        if (io->has_ssl_ && io->ssl_){
            Base::OpenSSL::ErrorCode error = Base::OpenSSL::OK;
            Long len = 0;
            do {
                error = Base::OpenSSL::OK;
                len = Base::sslReadSocket(io->ssl_, buf.data(), need_read_len, error);
                io->last_read_time_ = Base::getTimeOfDayUs();

                if (len <= 0) {
                    if (error == Base::OpenSSL::Need_RDWR) {
                        return;
                    } else if (error == Base::OpenSSL::Need_Close) {
                        goto DISCONNECT;
                    }
                    if (error == Base::OpenSSL::Error) {
                        io->error_ = errno;
                        // 读取错误
                        Log::FLogE("ssl socket read error, pid = %ld, id = %ld, error = %s",
                                   io->pid_, io->id_, strerror(io->error_));
                        goto READ_ERROR;
                    }
                    len = 0;
                }
                io->read_buf_.write(buf.data(), len);
                buf.clear();
                read_len += len;
            }while(Base::sslCanReadOrWrite(io->ssl_));

            if (read_len > 0) {
                handle_read_cb(io, (void *) (io->read_buf_.data()), read_len);
                io->read_buf_.removeFront(read_len);
            }
            return;
        }else {
            // 开始读取
            read_len = Base::readSocket(io->fd_, buf.data(), need_read_len);
            io->last_read_time_ = Base::getTimeOfDayUs();

            if (read_len < 0) {
                // 信号中断，下次再读取
                if (errno == EINTR || errno == EAGAIN) {
                    return;
                } else {
                    io->error_ = errno;
                    // 读取错误
                    Log::FLogE("socket read error, pid = %ld, id = %ld, error = %s",
                               io->pid_, io->id_, strerror(io->error_));
                    goto READ_ERROR;
                }
            }
            // 对端关闭
            if (read_len == 0) {
                goto DISCONNECT;
            }
        }
        io->read_buf_.write(buf.data(), read_len);
        buf.clear();
        // 读取回调
        handle_read_cb(io, (void*)(io->read_buf_.data()), read_len);
        io->read_buf_.removeFront(read_len);
        return;
        READ_ERROR:
        // 读取错误回调
        handle_read_err_cb(io, strerror(io->error_));
        DISCONNECT:
        // 关闭套接字
        io->close();
        return;
    }
}


// 处理写入回调
void handle_write(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io) {
        Long write_len = 0;
        SinBack::String buf;

        std::unique_lock<std::mutex> lock(io->write_mutex_);
        WRITE_START:
        if (io->closed_ || !io->ready_) return;
        if (io->write_queue_.empty()) {
            return;
        }

        write_len = 0;
        // 获取写入队列头部数据
        buf = io->write_queue_.front();
        io->write_queue_.pop_front();

        if (io->has_ssl_ && io->ssl_){
            Base::OpenSSL::ErrorCode error;
            write_len = Base::sslWriteSocket(io->ssl_, buf.data(), buf.size(), error);
            io->last_write_time_ = Base::getTimeOfDayUs();

            if (write_len <= 0){
                if (error == Base::OpenSSL::Need_RDWR){
                    write_len = 0;
                    goto WRITE_END;
                } else if (error == Base::OpenSSL::Need_Close){
                    lock.unlock();
                    goto DISCONNECT;
                }
                if (error == Base::OpenSSL::Error){
                    io->error_ = error;
                    // 写入错误
                    Log::FLogE("ssl socket read error, pid = %uld, id = %uld, error = %s",
                               io->pid_, io->id_, strerror(io->error_));
                    lock.unlock();
                    goto WRITE_ERROR;
                }
            }
        }else {
            write_len = Base::writeSocket(io->fd_, buf.data(), buf.size());
            io->last_write_time_ = Base::getTimeOfDayUs();

            if (write_len < 0) {
                // 写入失败，下次再写
                if (errno == EINTR || errno == EAGAIN) {
                    write_len = 0;
                    goto WRITE_END;
                } else {
                    io->error_ = errno;
                    // 写入错误
                    Log::FLogE("socket read error_, pid = %uld, id = %uld, error_ = %s",
                               io->pid_, io->id_, strerror(io->error_));
                    lock.unlock();
                    goto WRITE_ERROR;
                }
            } else if (write_len == 0) {
                lock.unlock();
                goto DISCONNECT;
            }
        }
        lock.unlock();
        // 写入回调
        handle_write_cb(io, write_len);
        lock.lock();
        WRITE_END:
        if (write_len < buf.length()) {
            io->write_queue_.push_front(std::move(buf.substr(write_len)));
        }
        if (!io->closed_) {
            goto WRITE_START;
        }
        return;
        WRITE_ERROR:
        handle_write_err_cb(io, strerror(io->error_));
        DISCONNECT:
        io->close();
    }
}


Int Core::IOEvent::accept()
{
    this->accept_ = true;
    return Core::EventLoop::addIoEvent(shared_from_this(), handle_event_func, Core::IO_READ);
}

Int Core::IOEvent::read()
{
    return Core::EventLoop::addIoEvent(shared_from_this(), handle_event_func, Core::IO_READ);
}

Int Core::IOEvent::read(Size_t read_len)
{
    if (read_len > 0){
        if (!this->read_buf_.empty() && this->read_buf_.size() >= read_len){
            // 有数据
            handle_read_cb(this->shared_from_this(), (void*)this->read_buf_.data(), read_len);
            if (this->read_buf_.size() == read_len){
                this->read_buf_.clear();
            } else {
                this->read_buf_.removeFront(read_len);
            }
        } else {
            return this->read();
        }
    }
    return static_cast<Int>(read_len);
}

Int Core::IOEvent::write(const void *buf, Size_t len) {
    return io_write(this, buf, len);
}

Int Core::IOEvent::close(bool timer)
{
    {
        std::unique_lock<std::mutex> lock(this->write_mutex_);
        if (this->closed_ || !this->ready_) return -1;

        // 还有数据没完成，加入定时器
        ULong id = this->id_;
        if (!this->write_queue_.empty() && this->error_ == 0 && timer){
            this->loop_->addTimer([id, this](const std::weak_ptr<Core::TimerEvent> &ev) {
                if (this->id_ == id) {
                    this->close();
                }
            }, 100, 1);
            return 1;
        }
        this->closed_ = true;
        // 清理IO事件
        this->clean();
        Core::EventLoop::loop_event_inactive(shared_from_this());
        // 关闭SSL套接字
        if (this->has_ssl_){
            Base::sslCloseSocket(this->ssl_);
            if (this->ssl_){
                SSL_free(this->ssl_);
            }
            this->ssl_ = nullptr;
        }
        Base::closeSocket(this->fd_);
    }
    // 关闭回调
    handle_close_cb(shared_from_this());
    return 0;
}

bool Core::IOEvent::setKeepAlive(Size_t timeout_ms)
{
    if (this->closed_ || !this->active_) return false;
    if (this->loop_) {
        this->last_read_time_ = this->last_write_time_ = this->loop_->getCurrentTime();
        this->keep_alive_ms_ = timeout_ms;
        this->loop_->addTimer(std::bind(&handle_keep_alive, this->id_, shared_from_this(),
                                        std::placeholders::_1),timeout_ms, 1);
        return true;
    }
    return false;
}

/**
 * 准备IO事件
 */
void Core::IOEvent::ready()
{
    this->ready_ = true;
    this->closed_ = false;
    this->last_read_time_ = this->last_write_time_ = Base::getTimeOfDayUs();
    this->read_buf_.clear();
    this->write_queue_.clear();
    this->pending_ = false;
    this->error_ = 0;
    this->evs_ = this->active_evs_ = 0;
    this->accept_ = false;
    this->read_len_ = 0;
    this->read_cb_ = this->read_err_cb_ = nullptr;
    this->write_cb_ = nullptr;
    this->write_err_cb_ = nullptr;
    this->accept_cb_ = this->close_cb_ = nullptr;
    this->id_ = Core::event_next_id();
    this->pid_ = ::getpid();
}

/**
 * 清理IO事件
 */
void Core::IOEvent::clean()
{
    if (!this->ready_) return;
    this->ready_ = false;
    Core::EventLoop::removeIoEvent(shared_from_this(), Core::IO_RDWR);
    this->read_buf_.clear();
    this->read_len_ = 0;
    this->write_queue_.clear();
    this->ssl_ = nullptr;
    this->has_ssl_ = false;
}
