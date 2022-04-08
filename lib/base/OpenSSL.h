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
            using string_type = SinBack::String;

            static pid_t s_ssl_pid;
            static void loadSSL();

            explicit OpenSSL();
            ~OpenSSL();
            SSL *newSSL();
            // 设置证书
            bool setCertFile(const string_type& cert_path);
            bool setKeyFile(const string_type& key_path);
            bool setCertAndKeyFile(const string_type& cert_path, const string_type& key_path);
            // 验证证书
            bool check();
        private:
            void init();
            void exit();
        private:
            SSL_METHOD *method_;
            SSL_CTX *ctx_;
            string_type cert_path_;
            string_type key_path_;
            bool error_;
        };
    }
}

#endif //SINBACK_TEST_OPENSSL_H
