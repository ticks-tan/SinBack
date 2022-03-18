/**
* FileName:   HttpService.cc
* CreateDate: 2022-03-14 16:22:58
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/

#include <regex>
#include "HttpMessage.h"
#include "HttpService.h"

using namespace SinBack;

void Http::HttpService::addService(const String &path, Int method, const Http::HttpServiceCall::Call &call)
{
    auto it = this->services_.find(path);
    if (it == this->services_.end()){
        auto service = std::make_shared<HttpServiceCall>(method, call);
        this->services_[path] =  service;
        return;
    }
    it->second->method = method;
    it->second->callback = call;
}

void Http::HttpService::removeService(const String &path, Int method)
{
    auto it = this->services_.find(path);
    if (it == this->services_.end()){
        return ;
    }
    if (it->second->method & method){
        it->second->method &= ~method;
    }
    if (it->second->method == 0){
        this->services_.erase(it);
    }
}

std::vector<std::shared_ptr<Http::HttpServiceCall>>
Http::HttpService::matchService(const String &path, Int method)
{
    std::vector<std::shared_ptr<Http::HttpServiceCall>> calls;
    for (auto& item : this->services_){
        if (std::regex_match(path, std::basic_regex<Char>(item.first))){
            calls.push_back(item.second);
        }
    }
    return calls;
}

void Http::HttpService::clearService()
{
    this->services_.clear();
}

void Http::HttpService::GET(const String &path, const Http::HttpServiceCall::Call &call)
{
    this->addService(path, Http::HTTP_GET, call);
}

void Http::HttpService::POST(const String &path, const Http::HttpServiceCall::Call &call)
{
    this->addService(path, Http::HTTP_POST, call);
}

void Http::HttpService::DELETE(const String &path, const Http::HttpServiceCall::Call &call)
{
    this->addService(path, Http::HTTP_DELETE, call);
}

void Http::HttpService::HEAD(const String &path, const Http::HttpServiceCall::Call &call)
{
    this->addService(path, Http::HTTP_HEAD, call);
}

void Http::HttpService::PUT(const String &path, const Http::HttpServiceCall::Call &call)
{
    this->addService(path, Http::HTTP_PUT, call);
}

void Http::HttpService::On(Int method, const String &path, const Http::HttpServiceCall::Call &call)
{
    this->addService(path, method, call);
}
