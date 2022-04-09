/**
* FileName:   Module.h
* CreateDate: 2022-04-09 14:05:29
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        模块基类
*/
#ifndef SINBACK_TEST_MODULE_H
#define SINBACK_TEST_MODULE_H

#include "core/EventLoop.h"

namespace SinBack
{
    namespace Module
    {
        // 模块基类
        class BaseModule : noncopyable
        {
        public:
            // 新客户端连接时调用
            virtual bool onNewClient(Core::IOEvent &io){return true;}
            // 新消息回调
            virtual bool onNewMessage(Core::IOEvent& io, const String& read_msg){return true;}
            // 读取错误
            virtual bool onMessageError(Core::IOEvent& io, const String& err_msg){return true;}
            // 发送数据回调
            virtual bool onSend(Core::IOEvent& ev, Size_t write_len){return true;}
            // 发送数据失败
            virtual bool onSendError(Core::IOEvent& ev, const String& err_msg){return true;}
            // 客户端关闭
            virtual bool onDisconnect(Core::IOEvent& ev){return true;}
        };
    }
}

#endif //SINBACK_TEST_MODULE_H
