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
#include "Base.h"

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

            enum VerifyMode{
                VERIFY_NONE = 0x00,
                VERIFY_PEER = 0x01,
                VERIFY_FAIL_IF_NO_PEER_CERT = 0x02,
                VERIFY_CLIENT_ONCE = 0x04
            };

            static void setSSLErrorCode(OpenSSL::ErrorCode& code, Int error){
                switch (error) {
                    case SSL_ERROR_NONE:
                        code = OpenSSL::OK; break;
                    case SSL_ERROR_WANT_CONNECT:
                        code = OpenSSL::Need_Connect; break;
                    case SSL_ERROR_WANT_ACCEPT:
                        code = OpenSSL::Need_Accept;
                    case SSL_ERROR_WANT_READ:
                        code = OpenSSL::Need_RDWR; break;
                    case SSL_ERROR_WANT_WRITE:
                        code = OpenSSL::Need_RDWR; break;
                    case SSL_ERROR_SYSCALL:
                        code = OpenSSL::Error; break;
                    case SSL_ERROR_SSL:
                        code = OpenSSL::Error; break;
                    case SSL_ERROR_WANT_X509_LOOKUP:
                        code = OpenSSL::Wait;
                    case SSL_ERROR_ZERO_RETURN:
                        code = OpenSSL::Need_Close;
                    default:
                        code = OpenSSL::Error; break;
                }
            }

            using string_type = SinBack::String;

            static pid_t s_ssl_pid;
            static void loadSSL();

            explicit OpenSSL();
            ~OpenSSL();
            SSL *newSSL(Int fd, ErrorCode& code);
            // 设置证书验证模式
            void setVerifyMode(VerifyMode mode);
            // 设置证书
            bool setPemCertFile(const string_type& cert_path);
            bool setAsnCertFile(const string_type& cert_path);
            // 设置私钥
            bool setPemKeyFile(const string_type& key_path);
            bool setAsnKeyFile(const string_type& key_path);
            // 设置CA证书目录和文件
            bool setCaDirAndFile(const string_type& ca_dir, const string_type& ca_file);
            // 验证证书
            bool check();
        private:
            void exit();
        private:
            // SSl 上下文
            SSL_CTX *ctx_;
            // CERT 证书文件路径
            string_type cert_path_;
            // KEY 证书文件路径
            string_type key_path_;
            // CA证书文件
            string_type ca_file_;
            // CA证书目录
            string_type ca_dir_;
            // 错误
            bool error_;
            // 证书验证模式
            Int verify_mode_;
        };

        class SSLSocket final : noncopyable
        {
        public:
            explicit SSLSocket(OpenSSL& openssl, Int sock);
            ~SSLSocket();

            bool canReadOrWrite();
            Int accept();
            Int handShake();
            Long read(void* buf, Size_t len);
            Long write(const void* buf, Size_t len);
            void close();
            OpenSSL::ErrorCode& code(){
                return this->code_;
            }
        private:
            SSL* ssl_;
            bool close_;
            OpenSSL::ErrorCode code_;
        };
    }
}

#endif //SINBACK_TEST_OPENSSL_H
