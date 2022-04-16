/**
* FileName:   OpenSSL.cc
* CreateDate: 2022-04-08 16:52:04
* Author:     ticks
* Email:      2938384958@qq.com
* Des:
*/
#include "OpenSSL.h"
#include "System.h"

using namespace SinBack;

pid_t Base::OpenSSL::s_ssl_pid = 0;

// 一个进程只执行一次
void Base::OpenSSL::loadSSL() {
    if (OpenSSL::s_ssl_pid == 0){
        OpenSSL::s_ssl_pid = ::getpid();
        SSL_library_init();
        return;
    }
    if (::getpid() != OpenSSL::s_ssl_pid){
        SSL_library_init();
    }
}

Base::OpenSSL::OpenSSL()
    : ctx_(nullptr)
    , error_(false)
    , verify_mode_(0)
{
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    this->ctx_ = SSL_CTX_new(SSLv23_server_method());
    if (this->ctx_ == nullptr){
        this->error_ = true;
    }
}

Base::OpenSSL::~OpenSSL()
{
    this->exit();
}

SSL *Base::OpenSSL::newSSL(Int fd, ErrorCode& code)
{
    SSL* ssl = nullptr;
    ssl = SSL_new(this->ctx_);
    if (ssl){
        // 设置允许 write (0, len] 模式写入
        SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
        // 设置SSL读写事件
        Int acp = SSL_set_fd(ssl, fd);
        if (acp <= 0){
            setSSLErrorCode(code, SSL_get_error(ssl, acp));
        }
    }
    return ssl;
}

void Base::OpenSSL::exit()
{
    if (this->ctx_){
        SSL_CTX_free(this->ctx_);
        this->ctx_ = nullptr;
    }
}

bool Base::OpenSSL::setPemCertFile(const Base::OpenSSL::string_type &cert_path)
{
    if (this->ctx_ && !cert_path.empty()){
        this->cert_path_ = cert_path;
        return SSL_CTX_use_certificate_file(this->ctx_, this->cert_path_.c_str(), SSL_FILETYPE_PEM);
    }
    return false;
}

bool Base::OpenSSL::setAsnCertFile(const string_type &cert_path)
{
    if (this->ctx_ && !cert_path.empty()){
        this->cert_path_ = cert_path;
        return SSL_CTX_use_certificate_file(this->ctx_, this->cert_path_.c_str(), SSL_FILETYPE_ASN1);
    }
    return false;
}

bool Base::OpenSSL::setPemKeyFile(const Base::OpenSSL::string_type &key_path)
{
    if (this->ctx_ && !key_path.empty()){
        this->key_path_ = key_path;
        return SSL_CTX_use_PrivateKey_file(this->ctx_, this->key_path_.c_str(), SSL_FILETYPE_PEM);
    }
    return false;
}

bool Base::OpenSSL::setAsnKeyFile(const string_type &key_path)
{
    if (this->ctx_ && !key_path.empty()){
        this->key_path_ = key_path;
        return SSL_CTX_use_PrivateKey_file(this->ctx_, this->key_path_.c_str(), SSL_FILETYPE_ASN1);
    }
    return false;
}

bool Base::OpenSSL::check()
{
    if (this->ctx_ && !this->cert_path_.empty() && !this->key_path_.empty()){
        if (SSL_CTX_check_private_key(this->ctx_) <= 0){
            return false;
        }
        // 设置错误选项
        SSL_CTX_set_options(this->ctx_, SSL_OP_ALL);
        // 设置证书验证模式
        SSL_CTX_set_verify(this->ctx_, this->verify_mode_, nullptr);
        // 设置 CTX 模式
        SSL_CTX_set_mode(this->ctx_, SSL_MODE_AUTO_RETRY);
        return true;
    }
    return false;
}

// 设置证书验证模式
void Base::OpenSSL::setVerifyMode(Base::OpenSSL::VerifyMode mode)
{
    if (mode & VERIFY_PEER){
        this->verify_mode_ = SSL_VERIFY_PEER;
        if (mode & VERIFY_CLIENT_ONCE){
            this->verify_mode_ |= SSL_VERIFY_CLIENT_ONCE;
        }
        if (mode & VERIFY_FAIL_IF_NO_PEER_CERT){
            this->verify_mode_ |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
        }
    }else if (mode & VERIFY_PEER){
        this->verify_mode_ = SSL_VERIFY_NONE;
    }else{
        if (mode & VERIFY_CLIENT_ONCE){
            this->verify_mode_ = SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE;
        }
        if (mode & VERIFY_FAIL_IF_NO_PEER_CERT){
            this->verify_mode_ = SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
        }
    }
}

bool
Base::OpenSSL::setCaDirAndFile(const Base::OpenSSL::string_type &ca_dir,
                               const Base::OpenSSL::string_type &ca_file)
{
    if (this->ctx_) {
        if (!ca_dir.empty()) this->ca_dir_ = ca_dir;
        if (!ca_file.empty()) this->ca_file_ = ca_file;
        if (!ca_file.empty() || !ca_dir.empty()) {
            return (SSL_CTX_load_verify_locations(this->ctx_, ca_file.c_str(), ca_dir.c_str()));
        }
        return SSL_CTX_set_default_verify_paths(this->ctx_);
    }
    return false;
}


Base::SSLSocket::SSLSocket(OpenSSL& openssl, Int sock)
    : ssl_(nullptr)
    , close_(false)
    , code_(OpenSSL::OK)
{
    this->ssl_ = openssl.newSSL(sock, this->code_);
    if (!this->ssl_){
        this->code_ = OpenSSL::Error;
    }
}

Base::SSLSocket::~SSLSocket()
{
    if (!this->close_){
        this->close();
    }
}

Long Base::SSLSocket::read(void *buf, Size_t len)
{
    Size_t tmp = 0;
    Int ret = SSL_read_ex(this->ssl_, buf, len, &tmp);
    if (ret == 1 && tmp > 0){
        return static_cast<Long>(tmp);
    }
    END:
    ret = SSL_get_error(this->ssl_, ret);
    OpenSSL::setSSLErrorCode(this->code_, ret);
    return -1;
}

Long Base::SSLSocket::write(const void *buf, Size_t len)
{
    Size_t tmp = 0;
    Int ret = SSL_write_ex(this->ssl_, buf, len, &tmp);
    if (ret == 1 && tmp > 0){
        this->code_ = OpenSSL::OK;
        return static_cast<Long>(tmp);
    }
    END:
    ret = SSL_get_error(this->ssl_, ret);
    OpenSSL::setSSLErrorCode(this->code_, ret);
    return -1;
}

void Base::SSLSocket::close()
{
    if (this->ssl_) {
        SSL_shutdown(this->ssl_);
        SSL_free(this->ssl_);
        this->ssl_ = nullptr;
    }
    this->close_ = true;
}

bool Base::SSLSocket::canReadOrWrite()
{
    if (!this->ssl_){
        return false;
    }
    return (SSL_pending(this->ssl_) > 0);
}

Int Base::SSLSocket::accept()
{
    Int ret = SSL_accept(this->ssl_);
    if (ret == 1) return 1;
    OpenSSL::setSSLErrorCode(this->code_, SSL_get_error(this->ssl_, ret));
    return ret;
}

Int Base::SSLSocket::handShake()
{
    Int ret = SSL_do_handshake(this->ssl_);
    if (ret == 1) return 1;
    OpenSSL::setSSLErrorCode(this->code_, SSL_get_error(this->ssl_, ret));
    return ret;
}
