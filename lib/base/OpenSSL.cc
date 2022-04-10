/**
* FileName:   OpenSSL.cc
* CreateDate: 2022-04-08 16:52:04
* Author:     ticks
* Email:      2938384958@qq.com
* Des:
*/
#include "OpenSSL.h"
#include "base/System.h"

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
    : method_(nullptr)
    , ctx_(nullptr)
    , error_(false)
{
    this->init();
}

Base::OpenSSL::~OpenSSL()
{
    this->exit();
}

SSL *Base::OpenSSL::newSSL()
{
    SSL* ssl = nullptr;
    ssl = SSL_new(this->ctx_);
    if (ssl == nullptr){
        // error_ handle
    }
    return ssl;
}

void Base::OpenSSL::init()
{
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    this->method_ = const_cast<SSL_METHOD*>(SSLv23_server_method());
    this->ctx_ = SSL_CTX_new(this->method_);
    if (this->ctx_ == nullptr){
        this->error_ = true;
    }
}

void Base::OpenSSL::exit()
{
    if (this->ctx_){
        SSL_CTX_free(this->ctx_);
        this->ctx_ = nullptr;
    }
}


bool Base::OpenSSL::setCertFile(const Base::OpenSSL::string_type &cert_path)
{
    if (this->ctx_ && !cert_path.empty()){
        this->cert_path_ = cert_path;
        return SSL_CTX_use_certificate_file(this->ctx_, this->cert_path_.c_str(), SSL_FILETYPE_PEM);
    }
    return false;
}

bool Base::OpenSSL::setKeyFile(const Base::OpenSSL::string_type &key_path)
{
    if (!this->ctx_ && !key_path.empty()){
        this->key_path_ = key_path;
        return SSL_CTX_use_PrivateKey_file(this->ctx_, this->key_path_.c_str(), SSL_FILETYPE_PEM);
    }
    return false;
}

bool Base::OpenSSL::setCertAndKeyFile(const string_type& cert_path, const string_type& key_path)
{
    return (setCertFile(std::forward<const string_type&>(cert_path))
            && setKeyFile(std::forward<const string_type&>(key_path)));
}

bool Base::OpenSSL::check()
{
    if (!this->ctx_ && !this->cert_path_.empty() && !this->key_path_.empty()){
        if (SSL_CTX_check_private_key(this->ctx_) <= 0){
            return false;
        }
        SSL_CTX_set_verify(this->ctx_, SSL_VERIFY_NONE, nullptr);
        SSL_CTX_set_options(this->ctx_, SSL_OP_ALL);
        SSL_CTX_set_mode(this->ctx_, SSL_MODE_AUTO_RETRY);
        return true;
    }
    return false;
}




