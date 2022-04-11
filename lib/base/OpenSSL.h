/**
* FileName:   OpenSSL.h
* CreateDate: 2022-04-08 16:22:31
* Author:     ticks
* Email:      2938384958@qq.com
* Des:
*/
#ifndef SINBACK_TEST_OPENSSL_H
#define SINBACK_TEST_OPENSSL_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include "base/Base.h"

namespace SinBack
{
    namespace Base {
        class OpenSSL final : noncopyable {
        public:

            enum ErrorCode
            {
                OK = 0x01,
                Need_Close = 0x02,
                Need_RDWR = 0x03,
                Need_Accept = 0x04,
                Need_Connect = 0x05,
                Wait = 0x06,
                Error = 0x07
            };

            using string_type = SinBack::String;

            static pid_t s_ssl_pid;
            static void loadSSL();

            explicit OpenSSL();
            ~OpenSSL();
            SSL *newSSL();
            // 设置证书
            bool setPemCertFile(const string_type& cert_path);
            bool setAsnCertFile(const string_type& cert_path);
            bool setPemKeyFile(const string_type& key_path);
            bool setAsnKeyFile(const string_type& key_path);
            // 验证证书
            bool check();
        private:
            void exit();
        private:
            SSL_CTX *ctx_;
            string_type cert_path_;
            string_type key_path_;
            bool error_;
        };
    }
}

#endif //SINBACK_TEST_OPENSSL_H
