/**
* FileName:   Channel.h
* CreateDate: 2022-03-09 20:33:39
* Author:     ticks
* Email:      2938384958@qq.com
* Des:
*/
#ifndef SINBACK_CHANNEL_H
#define SINBACK_CHANNEL_H

#include "core/Event.h"

namespace SinBack {
    namespace Core {

        class Channel : public noncopyable, public std::enable_shared_from_this<Channel>
        {
        public:
            explicit Channel(const std::weak_ptr<Core::IOEvent>& io);
            ~Channel();
        public:
            UInt getId() const{
                return this->io_->id_;
            }
            Base::socket_t getFd() const{
                return this->fd_;
            }
            std::shared_ptr<Core::IOEvent>& getIo(){
                return this->io_;
            }
            Int read();
            Int read(Size_t read_len);
            Int write(const void* buf, Size_t len);
            Int write(const String & buf);
            Int close();

            void setReadCb(const std::function<void(const String&)>& func){
                this->read_cb_ = func;
            }
            void setReadErrCb(const std::function<void(const String&)>& func){
                this->read_err_cb_ = func;
            }
            void setWriteCb(const std::function<void(Size_t)>& func){
                this->write_cb_ = func;
            }
            void setWriteErrCb(const std::function<void(const String&)>& func){
                this->write_err_cb_ = func;
            }
            void setCloseCb(const std::function<void()>& func){
                this->close_cb_ = func;
            }
        private:
            static void onRead(const std::weak_ptr<IOEvent>& ev, const String & buf);
            static void onReadError(const std::weak_ptr<IOEvent>& ev, const String & buf);
            static void onWrite(const std::weak_ptr<IOEvent>& ev, Size_t write_len);
            static void onWriteError(const std::weak_ptr<IOEvent>& ev, const String & buf);
            static void onClose(const std::weak_ptr<IOEvent>& ev);
        private:
            std::shared_ptr<Core::IOEvent> io_;
            Base::socket_t fd_;
            std::function<void(const String&)> read_cb_;
            std::function<void(const String&)> read_err_cb_;
            std::function<void(Size_t)> write_cb_;
            std::function<void(const String&)> write_err_cb_;
            std::function<void()> close_cb_;
        };
    }
}


#endif //SINBACK_CHANNEL_H
