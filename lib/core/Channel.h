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
            explicit Channel(const std::weak_ptr<Core::IOEvent>& io);
            ~Channel();
        public:
            Int read();
            Int read(Size_t read_len);
            Int write(const void* buf, Size_t len);
            Int write(const std::basic_string<Char>& buf);
            Int close();

            void set_read_cb(const std::function<void(Base::Buffer*)>& func){
                this->read_cb_ = func;
            }
            void set_write_cb(const std::function<void(const Base::Buffer*)>& func){
                this->write_cb_ = func;
            }
            void set_close_cb(const std::function<void()>& func){
                this->close_cb_ = func;
            }
        private:
            static void on_read(IOEvent* io, std::basic_string<Char>& buf);
            static void on_write(IOEvent* io, const std::basic_string<Char>& buf);
            static void on_close(IOEvent* io);
        private:
            std::shared_ptr<Core::IOEvent> io_;
            Base::socket_t fd_;
            std::function<void(Base::Buffer* buf)> read_cb_;
            std::function<void(const Base::Buffer* buf)> write_cb_;
            std::function<void()> close_cb_;
        };
    }
}


#endif //SINBACK_CHANNEL_H
