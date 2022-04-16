/**
* FileName:   HttpContext.cc
* CreateDate: 2022-03-14 16:19:25
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include <regex>
#include "HttpContext.h"
#include "base/Url.h"

using namespace SinBack;
using namespace SinBack::Module;

const String Http::HttpContext::error_str = " .----------------.  .----------------.  .----------------.  .----------------.  .----------------. \n"
                                            "| .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |\n"
                                            "| |  _________   | || |  _______     | || |  _______     | || |     ____     | || |  _______     | |\n"
                                            "| | |_   ___  |  | || | |_   __ \\    | || | |_   __ \\    | || |   .'    `.   | || | |_   __ \\    | |\n"
                                            "| |   | |_  \\_|  | || |   | |__) |   | || |   | |__) |   | || |  /  .--.  \\  | || |   | |__) |   | |\n"
                                            "| |   |  _|  _   | || |   |  __ /    | || |   |  __ /    | || |  | |    | |  | || |   |  __ /    | |\n"
                                            "| |  _| |___/ |  | || |  _| |  \\ \\_  | || |  _| |  \\ \\_  | || |  \\  `--'  /  | || |  _| |  \\ \\_  | |\n"
                                            "| | |_________|  | || | |____| |___| | || | |____| |___| | || |   `.____.'   | || | |____| |___| | |\n"
                                            "| |              | || |              | || |              | || |              | || |              | |\n"
                                            "| '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |\n"
                                            " '----------------'  '----------------'  '----------------'  '----------------'  '----------------' ";

// const SinString Http::HttpContext::notfound_str = SIN_STR("-_- -_- -_- -_- 404 Not Found -_- -_- -_- -_-\n");
const String Http::HttpContext::notfound_str = "   _____  _______      _____    _______          __    ___________                      .___\n"
                   "  /  |  | \\   _  \\    /  |  |   \\      \\   _____/  |_  \\_   _____/___  __ __  ____    __| _/\n"
                   " /   |  |_/  /_\\  \\  /   |  |_  /   |   \\ /  _ \\   __\\  |    __)/  _ \\|  |  \\/    \\  / __ | \n"
                   "/    ^   /\\  \\_/   \\/    ^   / /    |    (  <_> )  |    |     \\(  <_> )  |  /   |  \\/ /_/ | \n"
                   "\\____   |  \\_____  /\\____   |  \\____|__  /\\____/|__|    \\___  / \\____/|____/|___|  /\\____ | \n"
                   "     |__|        \\/      |__|          \\/                   \\/                   \\/      \\/ ";

Http::HttpContext::HttpContext(Http::HttpServer* server)
    : parser_()
    , server_(server)
    , url_params_(nullptr)
    , init_(false)
{
    this->parser_.reset(new Http::Http1Parse(&request_, &response_));
    this->cache_file_ = std::make_shared<Base::File>();
}

Http::HttpContext::~HttpContext()
{
    this->request_.clear();
    this->response_.clear();
    if (this->url_params_ != nullptr){
        delete url_params_;
    }
    this->url_params_ = nullptr;
}

Int Http::HttpContext::sendText(const string_type &text)
{
    this->response_.status_code = 200;
    this->response_.header.setHead("Content-Type", "text/plain;charset=UTF-8");
    this->response_.content.data().append(text);
    return 1;
}

Int Http::HttpContext::notFound()
{
    this->response_.status_code = 404;
    this->response_.header.setHead("Content-Type", "text/plain;charset=UTF-8");
    this->response_.content.data() += notfound_str;
    return 1;
}

Int Http::HttpContext::error() {
    this->response_.status_code = 405;
    this->response_.header.setHead("Content-Type", "text/plain;charset=UTF-8");
    this->response_.content.data() += error_str;
    return 1;
}

Int Http::HttpContext::sendFile(const string_type &file_name)
{
    if (!this->cache_file_->reOpen(file_name, Base::ReadOnly)){
        goto NotFound;
    }
    if (this->cache_file_->exist()) {
        this->response_.status_code = 200;
        this->response_.header.setHead("Content-Type",
                                       Http::get_http_content_type(this->cache_file_->suffix()));
        this->response_.content.data() = std::move(this->cache_file_->readAll());
    } else{
        NotFound:
        return this->notFound();
    }
    return 1;
}

bool Http::HttpContext::parseUrl()
{
    string_type& url = this->request_.url;
    std::basic_regex<Char> reg("/([0-9a-zA-Z-_]{1,}[/]?)*\\?([^?=&.]{1,}=[^?=&.]{1,}[&]?)*");
    if (std::regex_match(url, reg)){
        // 请求格式正确格式
        Size_t pos, start, end;
        string_type key, value;
        url = Tools::url_decode(url);
        pos = url.find_first_of('?');
        if (pos != string_type::npos){
            start = pos + 1;
            // 循环查找
            while ((pos = url.find_first_of('&', pos + 1)) != string_type::npos){
                end = url.find_first_of('=', start);
                if (end != string_type::npos && end < pos){
                    // 添加到 url_params
                    key = url.substr(start, end - start);
                    start = end + 1;
                    end = pos;
                    value = url.substr(start, end - start);
                    this->urlParams()[key] = value;
                    start = end + 1;
                } else{
                    this->urlParams().clear();
                    return false;
                }
            }
            if (start < url.length()){
                end = url.length();
                pos = url.find_first_of('=', start);
                if (pos != string_type::npos && pos < end){
                    key = url.substr(start, pos - start);
                    start = pos + 1;
                    value = url.substr(start, end - start);
                    this->urlParams()[key] = value;
                } else{
                    this->urlParams().clear();
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

bool Http::HttpContext::parseBody()
{
    return false;
}

bool Http::HttpContext::init()
{
    this->init_ = true;
    this->response_.http_version = this->request_.http_version;
    this->response_.header.setHead("Server", "SinBack");
    if (!this->request_.header.getHead("Connection").empty()){
        this->response_.header["Connection"] = this->request_.header["Connection"];
    } else{
        this->response_.header["Connection"] = "close";
    }
    return true;
}

