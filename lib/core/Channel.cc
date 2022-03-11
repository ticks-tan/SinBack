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

SinBack::Int SinBack::Core::Channel::write(const std::basic_string<Char> &buf)
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

void SinBack::Core::Channel::on_read(SinBack::Core::IOEvent *io, std::basic_string<Char>& buf)
{
    auto* channel = (Channel*)io->context_;
    if (channel && channel->read_cb_){
        Base::Buffer buffer(buf);
        channel->read_cb_(&buffer);
    }
}

void SinBack::Core::Channel::on_write(SinBack::Core::IOEvent *io, const std::basic_string<Char>& buf)
{
    auto* channel = (Channel*)io->context_;
    if (channel && channel->write_cb_){
        Base::Buffer buffer(buf);
        channel->write_cb_(&buffer);
    }
}

void SinBack::Core::Channel::on_close(SinBack::Core::IOEvent *io)
{
    auto* channel = (Channel*)io->context_;
    if (channel && channel->close_cb_){
        channel->close_cb_();
    }
}
