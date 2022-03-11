/**
* FileName:   Event.cc
* CreateDate: 2022-03-09 21:38:11
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#include "core/EventLoop.h"

using namespace SinBack;

void handle_event_func(const std::weak_ptr<Core::IOEvent>& ev);
void handle_accept_cb(Core::IOEvent* ev);
void handle_read_cb(Core::IOEvent* ev, void* buf, Size_t len);
void handle_write_cb(Core::IOEvent* ev, const void* buf, Size_t len);

void handle_accept(Core::IOEvent* io);
void handle_read(Core::IOEvent* io);
void handle_write(Core::IOEvent* io);

// 处理事件
void handle_event_func(const std::weak_ptr<Core::IOEvent>& ev)
{
    std::shared_ptr<Core::IOEvent> io = ev.lock();
    if (io){
        if ((io->active_evs_ & Core::IO_READ) && (io->evs_ & Core::IO_READ)){
            if (io->accept_){
                handle_accept(io.get());
            } else{
                handle_read(io.get());
            }
        }
        if ((io->active_evs_ & Core::IO_WRITE) && (io->evs_ & Core::IO_WRITE)){
            handle_write(io.get());
        }
    }
}

// 处理接收连接回调
void handle_accept_cb(Core::IOEvent* ev)
{
    if (ev){
        if (ev->accept_cb_){
            ev->accept_cb_(ev);
        }
    }
}

// 处理读取事件回调
void handle_read_cb(Core::IOEvent* ev, void* buf, Size_t len)
{
    if (ev) {
        if (ev->read_cb_) {
            ev->read_cb_(ev, ev->read_buf_);
        }
    }
}

// 处理写入事件回调
void handle_write_cb(Core::IOEvent* ev, const void* buf, Size_t len)
{
    if (ev) {
        if (ev->write_cb_) {
            std::basic_string<Char> buffer((const Char *) buf, len);
            ev->write_cb_(ev, buffer);
        }
    }
}

// 接受连接事件
void handle_accept(Core::IOEvent* io)
{
    if (io){
        ::sockaddr_in address{};
        ::socklen_t len = sizeof address;
        Base::socket_t client_fd = Base::accept_socket(io->fd_, &address, &len);
        if (client_fd < 0) {
            if (errno == EINTR || errno == EAGAIN){
                return;
            }
            io->loop_->logger().error("accept error, pid = {}, id = {}, errno = {} .",
                                      io->pid_, io->id_, errno);
            goto ACCEPT_ERROR;
        }
        std::shared_ptr<Core::IOEvent> io_ptr = io->loop_->get_io_event(client_fd);
        io_ptr->accept_cb_ = io->accept_cb_;
        handle_accept_cb(io_ptr.get());
        return;
    }
    ACCEPT_ERROR:
    io->close();
}

// 处理读取事件
void handle_read(Core::IOEvent* io)
{
    if (io){
        Int read_len = 0;
        Size_t len = 0;
        if (io->read_buf_.capacity() == 0){
            io->read_buf_.reserve(128);
        }
        READ_START:
        len = io->read_buf_.capacity() - io->read_buf_.length();
        read_len = (Int)Base::recv_socket(io->fd_, (void*)(io->read_buf_.data() + io->read_buf_.length()), len);
        if (read_len < 0){
            if (errno == EINTR || errno == EAGAIN){
                return;
            }else{
                io->loop_->logger().error("socket read error, pid = {}, id = {}, errno = {}",
                                          io->pid_, io->id_, errno);
                goto READ_ERROR;
            }
        }
        if (read_len == 0){
            goto DISCONNECT;
        }
        handle_read_cb(io, (void*)io->read_buf_.data(), read_len);
        return;
        READ_ERROR:
        DISCONNECT:
        io->close();
    }
}

// 处理写入回调
void handle_write(Core::IOEvent* io)
{
    Int write_len = 0;
    std::unique_lock<std::mutex> lock(io->write_mutex_);
    WRITE_START:
    if (io->closed) return;
    if (io->write_queue_.empty()){
        return;
    }
    write_len = 0;
    std::basic_string<Char> buf = io->write_queue_.front();
    write_len = (Int)Base::send_socket(io->fd_, buf.data(), buf.length());
    if (write_len < 0){
        if (errno == EINTR || errno == EAGAIN){
            return;
        } else{
            io->loop_->logger().error("socket write error, pid = {}, id = {}, errno = {}",
                                      io->pid_, io->id_, errno);
            goto WRITE_ERROR;
        }
    }
    if (write_len == 0){
        goto DISCONNECT;
    }
    handle_write_cb(io, buf.data(), write_len);
    if (write_len == buf.length()){
        io->write_queue_.pop_front();
        if (!io->closed){
            goto WRITE_START;
        }
    }else if (write_len < buf.length()){
        buf = io->write_queue_.front().substr(write_len);
        io->write_queue_.pop_front();
        io->write_queue_.push_front(buf);
        if (!io->closed){
            goto WRITE_START;
        }
    }
    return;
    WRITE_ERROR:
    DISCONNECT:
    lock.unlock();
    io->close();
}


Int Core::IOEvent::accept()
{
    this->accept_ = true;
    return Core::EventLoop::add_io_event(shared_from_this(), handle_event_func, Core::IO_READ);
}

Int Core::IOEvent::read()
{
    if (this->closed) return -1;
    return Core::EventLoop::add_io_event(shared_from_this(), handle_event_func, Core::IO_READ);
}

Int Core::IOEvent::write(const void *buf, Size_t len)
{
    if (this->closed) return -1;
    Int write_len = 0;
    std::unique_lock<std::mutex> lock(this->write_mutex_);
    if (this->write_queue_.empty()){
        write_len = (Int)Base::send_socket(this->fd_, buf, len);
        if (write_len < 0){
            if (errno == EINTR || errno == EAGAIN){
                write_len = 0;
                goto QUEUE_WRITE;
            } else{
                this->loop_->logger().error("socket write error, id = {}, pid = {}, errno = {}.",
                                            this->id_, this->pid_, errno);
                goto WRITE_ERROR;
            }
        }
        if (write_len == 0){
            goto DISCONNECT;
        }
        if (write_len == len){
            goto WRITE_END;
        }
        QUEUE_WRITE:
        EventLoop::add_io_event(shared_from_this(), handle_event_func, Core::IO_WRITE);
    }
    if (write_len < len){
        std::basic_string<Char> buffer = ((Char*)buf + write_len);
        this->write_queue_.push_back(buffer);
    }
    WRITE_END:
    lock.unlock();
    if (write_len > 0){
        handle_write_cb(this, buf, write_len);
    }
    return write_len;
    DISCONNECT:
    WRITE_ERROR:
    lock.unlock();
    this->close();
    return write_len < 0 ? write_len : -1;
}

Int Core::IOEvent::close()
{
    std::unique_lock<std::mutex> lock(this->write_mutex_);
    if (this->closed || !this->ready_) return -1;
    this->closed = true;
    this->destroy_ = true;
    Core::EventLoop::loop_event_inactive(shared_from_this());
    Core::EventLoop::delete_io_event(shared_from_this(), Core::IO_RDWR);
    Base::close_socket(this->fd_);
    return 0;
}
