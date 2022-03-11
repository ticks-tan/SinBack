/**
* FileName:   Channel.cc
* CreateDate: 2022-03-09 20:33:39
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "Channel.h"

SinBack::Core::Channel::Channel(const std::weak_ptr<Core::IOEvent> &io)
{
    this->io_ = io.lock();
    if (this->io_){
        this->fd_ = this->io_->fd_;
        if (!io_->read_cb_){
            this->io_->read_cb_ = on_read;
        }
        if (!this->io_->write_cb_){
            this->io_->write_cb_ = on_write;
        }
        if (!this->io_->close_cb_){
            this->io_->close_cb_ = on_close;
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
        this->io_->read_buf_.reserve(this->io_->read_buf_.capacity() + read_len);
        return this->io_->read();
    }
    return 0;
}

SinBack::Int SinBack::Core::Channel::write(const StringBuf &buf)
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

void SinBack::Core::Channel::on_read(const std::weak_ptr<IOEvent>& ev, StringBuf& buf)
{
    auto io = ev.lock();
    auto* channel = (Channel*)io->context_;
    if (channel && channel->read_cb_){
        channel->read_cb_(buf);
    }
}

void SinBack::Core::Channel::on_write(const std::weak_ptr<IOEvent>& ev, const StringBuf& buf)
{
    auto io = ev.lock();
    auto* channel = (Channel*)io->context_;
    if (channel && channel->write_cb_){
        channel->write_cb_(buf);
    }
}

void SinBack::Core::Channel::on_close(const std::weak_ptr<IOEvent>& ev)
{
    auto io = ev.lock();
    auto* channel = (Channel*)io->context_;
    if (channel && channel->close_cb_){
        channel->close_cb_();
    }
}
