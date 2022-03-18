/**
* FileName:   Channel.cc
* CreateDate: 2022-03-09 20:33:39
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "Channel.h"
#include "core/EventLoop.h"

SinBack::Core::Channel::Channel(const std::weak_ptr<Core::IOEvent> &io)
{
    this->io_ = io.lock();
    if (this->io_){
        this->fd_ = this->io_->fd_;
        if (!io_->read_cb_){
            this->io_->read_cb_ = onRead;
        }
        if (!this->io_->write_cb_){
            this->io_->write_cb_ = onWrite;
        }
        if (!this->io_->close_cb_){
            this->io_->close_cb_ = onClose;
        }
        if (!this->io_->read_err_cb_){
            this->io_->read_err_cb_ = onReadError;
        }
        if (!this->io_->write_err_cb_){
            this->io_->write_err_cb_ = onWriteError;
        }
        this->io_->context_ = this;
    }
}

SinBack::Core::Channel::~Channel()
{
    if (this->io_){
        this->close();
    }
}

SinBack::Int SinBack::Core::Channel::read()
{
    if (this->io_){
        return this->io_->read();
    }
    return 0;
}

SinBack::Int SinBack::Core::Channel::write(const void* buf, Size_t len)
{
    if (this->io_){
        return this->io_->write(buf, len);
    }
    return 0;
}

SinBack::Int SinBack::Core::Channel::read(SinBack::Size_t read_len)
{
    if (this->io_){
        if (read_len > 0){
            this->io_->read_len_ = read_len;
            this->io_->read_buf_.reserve(this->io_->read_buf_.capacity() + read_len);
            if (read_len <= this->io_->read_buf_.length()){
                // 需要读取的数据已经在缓冲区中，直接调用回调
                if (this->io_->read_cb_){
                    std::basic_string<Char> buffer(this->io_->read_buf_, read_len);
                    this->io_->read_cb_(this->io_, buffer);
                    this->io_->read_buf_.erase(0, read_len);
                }
                return 0;
            }
            return this->io_->read();
        }
    }
    return 0;
}

SinBack::Int SinBack::Core::Channel::write(const String &buf)
{
    if (this->io_){
        return this->io_->write(buf.data(), buf.length());
    }
    return 0;
}

SinBack::Int SinBack::Core::Channel::close()
{
    if (this->io_){
        return this->io_->close();
    }
    return 0;
}

void SinBack::Core::Channel::onRead(const std::weak_ptr<IOEvent>& ev, const String& buf)
{
    auto io = ev.lock();
    if (io) {
        auto channel = (Channel*)(io->context_);
        if (channel && channel->read_cb_) {
            channel->read_cb_(buf);
        }
    }
}

void SinBack::Core::Channel::onWrite(const std::weak_ptr<IOEvent>& ev, Size_t write_len)
{
    auto io = ev.lock();
    if (io) {
        auto channel = (Channel*)(io->context_);
        if (channel && channel->write_cb_) {
            channel->write_cb_(write_len);
        }
    }
}

void SinBack::Core::Channel::onClose(const std::weak_ptr<IOEvent>& ev)
{
    auto io = ev.lock();
    if (io) {
        auto channel = (Channel*)(io->context_);
        if (channel && channel->close_cb_) {
            channel->close_cb_();
        }
    }
}

void
SinBack::Core::Channel::onReadError(const std::weak_ptr<IOEvent> &ev, const SinBack::String &buf)
{
    auto io = ev.lock();
    if (io){
        auto channel = (Channel*)(io->context_);
        if (channel && channel->read_err_cb_){
            channel->read_err_cb_(buf);
        }
    }
}

void
SinBack::Core::Channel::onWriteError(const std::weak_ptr<IOEvent> &ev, const SinBack::String &buf)
{
    auto io = ev.lock();
    if (io){
        auto channel = (Channel*)(io->context_);
        if (channel && channel->write_err_cb_){
            channel->write_err_cb_(buf);
        }
    }
}
