/**
* FileName:   HttpContext.cc
* CreateDate: 2022-03-14 16:19:25
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include "HttpContext.h"

using namespace SinBack;

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

// const String Http::HttpContext::notfound_str = SIN_STR("-_- -_- -_- -_- 404 Not Found -_- -_- -_- -_-\n");
const String Http::HttpContext::notfound_str = "   _____  _______      _____    _______          __    ___________                      .___\n"
                   "  /  |  | \\   _  \\    /  |  |   \\      \\   _____/  |_  \\_   _____/___  __ __  ____    __| _/\n"
                   " /   |  |_/  /_\\  \\  /   |  |_  /   |   \\ /  _ \\   __\\  |    __)/  _ \\|  |  \\/    \\  / __ | \n"
                   "/    ^   /\\  \\_/   \\/    ^   / /    |    (  <_> )  |    |     \\(  <_> )  |  /   |  \\/ /_/ | \n"
                   "\\____   |  \\_____  /\\____   |  \\____|__  /\\____/|__|    \\___  / \\____/|____/|___|  /\\____ | \n"
                   "     |__|        \\/      |__|          \\/                   \\/                   \\/      \\/ ";

Http::HttpContext::HttpContext(Http::HttpServer* server)
    : parser_()
    , server_(server)
{
    this->parser_.reset(new Http::Http1Parse(&request_, &response_));
    this->cache_file_ = std::make_shared<Base::File>();
}

Http::HttpContext::~HttpContext()
{
    this->request_.clear();
    this->response_.clear();
}

Int Http::HttpContext::senText(const String &text)
{
    this->response_.status_code = 200;
    this->response_.header.setHead(SIN_STR("Content-Type"), SIN_STR("text/plain;charset=UTF-8"));
    this->response_.content.data().append(text);
    return 1;
}

Int Http::HttpContext::notFound()
{
    this->response_.status_code = 404;
    this->response_.header.setHead(SIN_STR("Content-Type"), SIN_STR("text/plain;charset=UTF-8"));
    this->response_.content.data() += notfound_str;
    return 1;
}

Int Http::HttpContext::senFile(const String &file_name)
{
    if (!this->cache_file_->reOpen(file_name, Base::ReadOnly)){
        goto NotFound;
    }
    if (this->cache_file_->exist()) {
        this->response_.status_code = 200;
        this->response_.header.setHead(SIN_STR("Content-Type"),
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
    return false;
}

bool Http::HttpContext::parseBody()
{
    return false;
}

bool Http::HttpContext::init()
{
    this->response_.http_version = this->request_.http_version;
    this->response_.header.setHead(SIN_STR("Server"), SIN_STR("SinBack"));
    if (!this->request_.header.getHead(SIN_STR("Connection")).empty()){
        this->response_.header[SIN_STR("Connection")] = this->request_.header[SIN_STR("Connection")];
    }
    return true;
}

