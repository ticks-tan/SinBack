/**
* FileName:   Channel.h
* CreateDate: 2022-03-09 20:33:39
* Author:     ticks
* Email:      2938384958@qq.com
* Des:
*/
#ifndef SINBACK_CHANNEL_H
#define SINBACK_CHANNEL_H

#include "base/Buffer.h"
#include "core/Event.h"
#include "base/noncopyable.h"

namespace SinBack {
    namespace Core {

        class Channel : noncopyable
        {
        public:
            using StringBuf = std::basic_string<Char>;
            explicit Channel(const std::weak_ptr<Core::IOEvent>& io);
            ~Channel();
        public:
            UInt get_id() const{
                return this->io_->id_;
            }
            Base::socket_t get_fd() const{
                return this->fd_;
            }
            std::shared_ptr<Core::IOEvent>& get_io(){
                return this->io_;
            }
            Int read();
            Int read(Size_t read_len);
            Int write(const void* buf, Size_t len);
            Int write(const StringBuf & buf);
            Int close();

            void set_read_cb(const std::function<void(const StringBuf&)>& func){
                this->read_cb_ = func;
            }
            void set_read_err_cb(const std::function<void(const StringBuf&)>& func){
                this->read_err_cb_ = func;
            }
            void set_write_cb(const std::function<void(Size_t)>& func){
                this->write_cb_ = func;
            }
            void set_write_err_cb(const std::function<void(const StringBuf&)>& func){
                this->write_err_cb_ = func;
            }
            void set_close_cb(const std::function<void()>& func){
                this->close_cb_ = func;
            }
        private:
            static void on_read(const std::weak_ptr<IOEvent>& ev, const StringBuf & buf);
            static void on_read_error(const std::weak_ptr<IOEvent>& ev, const StringBuf & buf);
            static void on_write(const std::weak_ptr<IOEvent>& ev, Size_t write_len);
            static void on_write_error(const std::weak_ptr<IOEvent>& ev, const StringBuf & buf);
            static void on_close(const std::weak_ptr<IOEvent>& ev);
        private:
            std::shared_ptr<Core::IOEvent> io_;
            Base::socket_t fd_;
            std::function<void(const StringBuf&)> read_cb_;
            std::function<void(const StringBuf&)> read_err_cb_;
            std::function<void(Size_t)> write_cb_;
            std::function<void(const StringBuf&)> write_err_cb_;
            std::function<void()> close_cb_;
        };
    }
}


#endif //SINBACK_CHANNEL_H
