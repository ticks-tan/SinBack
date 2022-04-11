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

SSL *Base::OpenSSL::newSSL()
{
    SSL* ssl = nullptr;
    ssl = SSL_new(this->ctx_);
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




