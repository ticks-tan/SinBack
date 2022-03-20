/**
* FileName:   Event.cc
* CreateDate: 2022-03-09 21:38:11
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        Event function
*/
#include "core/EventLoop.h"
#include "Event.h"


using namespace SinBack;

Int io_read_et(Core::IOEvent* io);

// 水平触发 write函数
Int io_write(Core::IOEvent* io, const void *buf, Size_t len);
// 边缘触发 write 函数
Int io_write_et(Core::IOEvent* io, const void *buf, Size_t len);

// 根据活动事件选择 accept、read和write
void handle_event_func(const std::weak_ptr<Core::IOEvent>& ev);
//  accept 回调
void handle_accept_cb(const std::weak_ptr<Core::IOEvent>& ev);
// read 回调
void handle_read_cb(const std::weak_ptr<Core::IOEvent>& ev, void* buf, Size_t len);
// read 错误回调
void handle_read_err_cb(const std::weak_ptr<Core::IOEvent>& ev, const std::basic_string<Char>& err);
// write 回调
void handle_write_cb(const std::weak_ptr<Core::IOEvent>& ev, Size_t write_len);
// write 错误回调
void handle_write_err_cb(const std::weak_ptr<Core::IOEvent>& ev, const std::basic_string<Char>& err);
// close回调
void handle_close_cb(const std::weak_ptr<Core::IOEvent>& ev);

// accept 封装
void handle_accept(const std::weak_ptr<Core::IOEvent>& ev);
// 边缘触发 read 封装
void handle_read_et(const std::weak_ptr<Core::IOEvent>& ev);
// 水平触发 read 封装
void handle_read(const std::weak_ptr<Core::IOEvent>& ev);
// 水平触发 write 封装
void handle_write(const std::weak_ptr<Core::IOEvent>& ev);
// 边缘触发 write 封装
void handle_write_et(const std::weak_ptr<Core::IOEvent>& ev);

void handle_keep_alive(const std::shared_ptr<Core::IOEvent>& io, const std::weak_ptr<Core::TimerEvent>& ev);


Int io_read_et(Core::IOEvent* io)
{
    if (io){
        if (io->closed || io->destroy_) return -1;

        std::vector<Char> buf;
        buf.reserve(256);
        // 每次需要读取长度
        Size_t need_read_len = 0, handle_len;
        // 实际读取长度
        Long read_len = 0, len;

        while (true){
            // 获取buffer剩余长度
            need_read_len = buf.capacity() - buf.size();
            len = Base::readSocket(io->fd_, buf.data(), need_read_len);
            io->last_read_time_ = Base::getTimeOfDayUs();

            if (len < 0){
                io->error = errno;
                if (errno == EAGAIN || errno == EWOULDBLOCK){
                    // 没有数据可读，返回
                    goto READ_END;
                } else{
                    // 读取错误，处理错误
                    Log::loge("socket read error, pid = {}, id = {}, error -> {} .",
                              io->pid_, io->id_, strerror(io->error));
                    goto READ_ERR;
                }
            }else if (len == 0){
                // 客户端断开连接
                goto DISCONNECT;
            }else{
                // 成功读取数据
                read_len += len;
                io->read_buf_.append(buf.data(), len);
                buf.clear();
            }
        }
        READ_END:
        // 读取结束，调用回调函数
        handle_len = (io->read_len_ > 0 && io->read_len_ <= io->read_buf_.length()) ? io->read_len_ : io->read_buf_.length();
        if (handle_len > 0) {
            handle_read_cb(io->shared_from_this(), (void *) (io->read_buf_.data()), handle_len);
            // 删除前面数据，防止缓冲区无限膨胀
            io->read_buf_.erase(0, handle_len);
        }
        SinBack::Core::EventLoop::addIoEvent(io->shared_from_this(), handle_event_func,
                                             Core::IO_READ | Core::IO_TYPE_ET);
        return (Int)handle_len;
        READ_ERR:
        // 错误回调
        handle_read_err_cb(io->shared_from_this(), strerror(io->error));
        DISCONNECT:
        // 关闭连接
        io->close();
        return 0;
    }
    return -1;
}

Int io_write(Core::IOEvent* io, const void *buf, Size_t len)
{
    if (io->closed || io->destroy_) return -1;
    if (buf == nullptr || len == 0) return 0;

    Long write_len = 0;

    std::unique_lock<std::mutex> lock(io->write_mutex_);

    if (io->write_queue_.empty()){
        // 写入队列为空，尝试写入一次，一次没有写入完再添加到事件循环中等待事件触发
        write_len = Base::writeSocket(io->fd_, buf, len);
        io->last_write_time_ = Base::getTimeOfDayUs();

        if (write_len < 0){
            io->error = errno;
            if (errno == EINTR || errno == EAGAIN){
                write_len = 0;
                goto QUEUE_WRITE;
            } else{
                Log::loge("socket write error, id = {}, pid = {}, error = {}.",
                                            io->id_, io->pid_, strerror(io->error));
                lock.unlock();
                goto WRITE_ERROR;
            }
        } else if (write_len == 0){
            // 对端关闭
            lock.unlock();
            goto DISCONNECT;
        }
        // 一次性写入完成
        if (write_len == len){
            goto WRITE_END;
        }
        // 一次性没有写入完成, 添加到事件循环下次可写事件触发时再写
        QUEUE_WRITE:
        // SinBack::Core::EventLoop::addIoEvent(io->shared_from_this(), handle_event_func, Core::IO_WRITE | Core::IO_TYPE_ET);
        SinBack::Core::EventLoop::addIoEvent(io->shared_from_this(), handle_event_func, Core::IO_WRITE);
    }

    if (write_len < len){
        std::basic_string<Char> buffer = ((Char*)buf + write_len);
        io->write_queue_.push_back(buffer);
    }
    io->last_write_time_ = Base::getTimeOfDayUs();
    WRITE_END:
    lock.unlock();
    if (write_len > 0){
        handle_write_cb(io->shared_from_this(), write_len);
    }
    return (Int)write_len;
    WRITE_ERROR:
    handle_write_err_cb(io->shared_from_this(), strerror(io->error));
    DISCONNECT:
    io->close();
    return write_len < 0 ? -1 : (Int)write_len;
}

Int io_write_et(Core::IOEvent* io, const void *buf, Size_t len)
{
    if (io->closed) return -1;
    if (buf == nullptr || len == 0) return 0;

    Long write_len = 0, w_len;

    std::unique_lock<std::mutex> lock(io->write_mutex_);
    if (io->write_queue_.empty()){

        while (true) {
            w_len = Base::writeSocket(io->fd_, (Char *) buf + write_len, len - write_len);
            io->last_write_time_ = Base::getTimeOfDayUs();

            if (w_len < 0) {
                io->error = errno;
                // 写入缓冲区满
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    goto WRITE_QUEUE;
                } else {
                    Log::loge("socket write error, pid = {}, id = {}, error = {} .",
                                              io->pid_, io->id_, strerror(io->error));
                    lock.unlock();
                    // 写入出错
                    goto WRITE_ERR;
                }
            } else if (w_len == 0) {
                lock.unlock();
                // 对端关闭
                goto DISCONNECT;
            }
            write_len += w_len;
            // 一次性写完
            if (write_len == len) {
                goto WRITE_END;
            }
        }
        WRITE_QUEUE:
        SinBack::Core::EventLoop::addIoEvent(io->shared_from_this(), handle_event_func,
                                             Core::IO_WRITE | Core::IO_TYPE_ET);
    }
    if (write_len < len){
        std::basic_string<Char> buffer = ((Char*)buf + write_len);
        io->write_queue_.push_back(buffer);
    }
    WRITE_END:
    lock.unlock();
    if (write_len > 0){
        handle_write_cb(io->shared_from_this(), write_len);
    }
    return (Int)write_len;
    WRITE_ERR:
    // 错误回调
    handle_write_err_cb(io->shared_from_this(), strerror(io->error));
    DISCONNECT:
    io->close();
    return (write_len < 0) ? -1 : (Int)write_len;
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
                if (io->evs_ & Core::IO_TYPE_ET){
                    handle_read_et(io);
                }else {
                    handle_read(io);
                }
            }
        }
        if ((io->active_evs_ & Core::IO_WRITE) && (io->evs_ & Core::IO_WRITE)){
            io->last_write_time_ = Base::getTimeOfDayUs();
            if (io->evs_ & Core::IO_TYPE_ET){
                handle_write_et(io);
            } else {
                handle_write(io);
            }
        }
    }
}

// keepalive回调
void handle_keep_alive(const std::shared_ptr<Core::IOEvent>& io, const std::weak_ptr<Core::TimerEvent>& ev)
{
    auto timer = ev.lock();
    if (timer){
        if (io){
            if (io->closed || io->destroy_) return;
            auto loop = (Core::EventLoopPtr)(io->loop_);
            if (loop) {
                ULong last_rw_time = std::max(io->last_read_time_, io->last_write_time_);
                ULong time_ms = (loop->getCurrentTime() - last_rw_time) / 1000;
                if (time_ms + 100 < io->keep_alive_ms_) {
                    loop->addTimer(std::bind(&handle_keep_alive, io, std::placeholders::_1), io->keep_alive_ms_, 1);
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
            std::basic_string<Char> buffer((Char*)buf, len);
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
        ::sockaddr_in address{};
        ::socklen_t len = sizeof address;
        Base::socket_t client_fd = Base::acceptSocket(io->fd_, &address, &len);
        if (client_fd < 0) {
            io->error = errno;
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK){
                return;
            }
            Log::loge("accept error, pid = {}, id = {}, error = {} .",
                                      io->pid_, io->id_, strerror(io->error));
            goto ACCEPT_ERROR;
        }
        std::shared_ptr<Core::IOEvent> io_ptr = io->loop_->getIoEvent(client_fd);
        io_ptr->accept_cb_ = io->accept_cb_;
        io_ptr->context_ = io->context_;
        handle_accept_cb(io_ptr);
        return;
    }
    ACCEPT_ERROR:
    // io->close();
    return;
}

// 处理ET模式下读取事件
void handle_read_et(const std::weak_ptr<Core::IOEvent>& ev)
{
    // ET模式下必须一次性将数据读取完，不然后续再触发事件就难了
    auto io = ev.lock();
    if (io){
        if (io->closed || io->destroy_) return;

        std::vector<Char> buf;
        buf.reserve(256);
        // 每次需要读取长度
        Size_t need_read_len = 0, handle_len;
        // 实际读取长度
        Long read_len = 0, len;

        while (true){
            // 获取buffer剩余长度
            need_read_len = buf.capacity() - buf.size();
            len = Base::readSocket(io->fd_, buf.data(), need_read_len);
            io->last_read_time_ = Base::getTimeOfDayUs();

            if (len < 0){
                io->error = errno;
                if (errno == EAGAIN || errno == EWOULDBLOCK){
                    // 没有数据可读，返回
                    goto READ_END;
                } else{
                    // 读取错误，处理错误
                    Log::loge("socket read error, pid = {}, id = {}, error -> {} .",
                                              io->pid_, io->id_, strerror(io->error));
                    goto READ_ERR;
                }
            }else if (len == 0){
                // 客户端断开连接
                goto DISCONNECT;
            }else{
                // 成功读取数据
                read_len += len;
                io->read_buf_.append(buf.data(), len);
                buf.clear();
            }
        }
        READ_END:
        // 读取结束，调用回调函数
        handle_len = (io->read_len_ > 0 && io->read_len_ <= io->read_buf_.length()) ? io->read_len_ : io->read_buf_.length();
        handle_read_cb(io, (void*)(io->read_buf_.data()), handle_len);
        // 删除前面数据，防止缓冲区无限膨胀
        io->read_buf_.erase(0, handle_len);
        return;
        READ_ERR:
        // 错误回调
        handle_read_err_cb(io, strerror(io->error));
        DISCONNECT:
        // 关闭连接
        io->close();
        return;
    }
}

// 处理读取事件
void handle_read(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io){
        if (io->closed || io->destroy_) return;

        std::vector<Char> buf;
        buf.reserve(256);
        // 需要读取的长度
        Size_t need_read_len = 0;
        // 读取长度
        Long read_len = 0;

        need_read_len = buf.capacity();
        // 避免读取长度为0,返回0导致错误关闭套接字
        if (need_read_len == 0) return;

        // 开始读取
        read_len = Base::readSocket(io->fd_, (void *) (buf.data()), need_read_len);
        io->last_read_time_ = Base::getTimeOfDayUs();

        if (read_len < 0){
            io->error = errno;
            // 信号中断，下次再读取
            if (errno == EINTR || errno == EAGAIN){
                return;
            }else{
                // 读取错误
                Log::loge("socket read error, pid = {}, id = {}, error = {}",
                                          io->pid_, io->id_, strerror(io->error));
                goto READ_ERROR;
            }
        }
        // 对端关闭
        if (read_len == 0){
            goto DISCONNECT;
        }
        io->read_buf_.append(buf.data(), read_len);
        buf.clear();
        // 读取回调
        handle_read_cb(io, (void*)(io->read_buf_.data()), read_len);
        io->read_buf_.erase(0, read_len);
        return;
        READ_ERROR:
        // 读取错误回调
        handle_read_err_cb(io, strerror(io->error));
        DISCONNECT:
        // 关闭套接字
        io->close();
        return;
    }
}

// 处理ET模式下写入事件
void handle_write_et(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io){
        Long write_len = 0, len;
        std::basic_string<Char> buf;

        std::unique_lock<std::mutex> lock(io->write_mutex_);

        if (io->closed) return;
        if (io->write_queue_.empty()) return;

        while (!io->write_queue_.empty()){
            if (io->closed) return;

            // 取出队列头部数据
            buf = io->write_queue_.front();
            io->write_queue_.pop_front();

            len = Base::writeSocket(io->fd_, buf.data(), buf.length());
            io->last_write_time_ = Base::getTimeOfDayUs();

            if (len < 0){
                io->error = errno;
                if (errno == EAGAIN || errno == EWOULDBLOCK){
                    lock.unlock();
                    goto WRITE_END;
                } else {
                    Log::loge("socket write error, pid = {}, id = {}, error = {} .",
                                              io->pid_, io->id_, strerror(io->error));
                    lock.unlock();
                    goto WRITE_ERR;
                }
            }else if (len == 0){
                // 对端关闭
                lock.unlock();
                goto DISCONNECT;
            }else{
                write_len += len;
                // 写入成功
                if (len < buf.length()){
                    // 没写完整，将剩余部分添加到队列头部
                    io->write_queue_.push_front(buf.substr(len));
                }
            }
        }
        WRITE_END:
        handle_write_cb(io, write_len);
        return;
        WRITE_ERR:
        // 错误回调
        handle_write_err_cb(io, strerror(io->error));
        DISCONNECT:
        io->close();
        return;
    }
}

// 处理写入回调
void handle_write(const std::weak_ptr<Core::IOEvent>& ev)
{
    auto io = ev.lock();
    if (io) {
        Long write_len = 0, len;
        std::basic_string<Char> buf;

        std::unique_lock<std::mutex> lock(io->write_mutex_);
        WRITE_START:
        if (io->closed) return;
        if (io->write_queue_.empty()) {
            return;
        }

        // 获取写入队列头部数据
        buf = io->write_queue_.front();
        io->write_queue_.pop_front();

        len = Base::writeSocket(io->fd_, buf.data(), buf.length());
        io->last_write_time_ = Base::getTimeOfDayUs();

        if (write_len < 0) {
            io->error = errno;
            // 写入失败，下次再写
            if (errno == EINTR || errno == EAGAIN) {
                return;
            } else {
                // 写入错误
                Log::loge("socket write error, pid = {}, id = {}, error = {}",
                                          io->pid_, io->id_, strerror(io->error));
                lock.unlock();
                goto WRITE_ERROR;
            }
        } else if (len == 0) {
            lock.unlock();
            goto DISCONNECT;
        }
        // 写入回调
        handle_write_cb(io, len);

        write_len += len;
        if (write_len < buf.length()) {
            io->write_queue_.push_front(buf.substr(write_len));
        }
        if (!io->closed) {
            goto WRITE_START;
        }
        return;
        WRITE_ERROR:
        handle_write_err_cb(io, strerror(io->error));
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
    if (this->closed) return -1;
    if (this->evs_ & Core::IO_TYPE_ET){
        return io_read_et(this);
    }
    return Core::EventLoop::addIoEvent(shared_from_this(), handle_event_func, Core::IO_READ);
}

Int Core::IOEvent::write(const void *buf, Size_t len)
{
    if (this->evs_ & Core::IO_TYPE_ET){
        return io_write_et(this, buf, len);
    }
    return io_write(this, buf, len);
}

Int Core::IOEvent::close(bool timer)
{
    {
        std::unique_lock<std::mutex> lock(this->write_mutex_);
        if (this->closed || !this->ready_) return -1;

        // 还有数据没完成，加入定时器
        if (!this->write_queue_.empty() && this->error == 0 && timer){
            this->loop_->addTimer([this](const std::weak_ptr<Core::TimerEvent> &ev) {
                this->close();
            }, 100, 1);
            return 1;
        }
        this->closed = true;
        this->destroy_ = true;
        Core::EventLoop::removeIoEvent(shared_from_this(), Core::IO_RDWR | Core::IO_TYPE_ET);
        Core::EventLoop::loop_event_inactive(shared_from_this());
        Base::closeSocket(this->fd_);
    }
    // 关闭回调
    handle_close_cb(shared_from_this());
    return 0;
}

bool Core::IOEvent::setKeepalive(Size_t timeout_ms)
{
    if (this->closed || !this->active_ || !this->loop_) return false;
    if (this->loop_) {
        this->last_read_time_ = this->last_write_time_ = this->loop_->getCurrentTime();
        this->keep_alive_ms_ = timeout_ms;
        this->loop_->addTimer(std::bind(&handle_keep_alive, shared_from_this(), std::placeholders::_1), timeout_ms, 1);
    }
    return true;
}
