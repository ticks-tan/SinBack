/**
* FileName:   Args.h
* CreateDate: 2022-03-22 23:08:18
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        命令行参数解析
*/
#ifndef SINBACK_ARGS_H
#define SINBACK_ARGS_H

#include <unordered_map>
#include "base/Base.h"

namespace SinBack
{
    namespace Base
    {
        class Args : noncopyable
        {
        public:
            using ForEachKeyFunc = std::function<void(const String&)>;
            using ForEachFunc = std::function<void(const String &, const String &)>;

            Args(int argc, char* argv[]);
            // 参数个数
            Size_t size() const;
            // 获取参数，没有返回 ""
            String getArg(const String& arg);
            // 判断参数是否存在
            bool hasArg(const String& arg) const;
            // 循环获取参数
            Size_t forEach(const ForEachKeyFunc& func);
            Size_t forEach(const ForEachFunc& func);
        private:
            std::unordered_map<String, String> args_;
        };

        /**
         * 构造函数
         * @param arg 参数
         * @return
         */
        Args::Args(int argc, char **argv) {
            if (argc > 1) {
                String tmp, key;
                Size_t i = 1, pos;
                for (; i < argc; ++i) {
                    tmp = argv[i];
                    if ((pos = tmp.find('=')) != String::npos){
                        // exec --path=./
                        if (!key.empty()){
                            this->args_[key] = "";
                        }
                        this->args_[std::move(tmp.substr(0, pos))] = std::move(tmp.substr(pos + 1));
                    }else{
                        // exec --read
                        if (tmp.size() > 2 && tmp[0] == '-' && tmp[1] == tmp[0]){
                            this->args_[tmp] = "";
                            continue;
                        }
                        if (key.empty() && tmp[0] == '-'){
                            // exec -num 12
                            key = tmp;
                        }else {
                            if (key.empty()){
                                // exec version
                                key = tmp;
                                continue;
                            }
                            this->args_[key] = tmp;
                            key.clear();
                        }
                    }
                }
                if (!key.empty()){
                    this->args_[key] = "";
                }
            }
        }

        /**
         * 参数个数
         */
        Size_t Args::size() const {
            return this->args_.size();
        }

        /**
         * 获取参数，没有返回 ""
         * @param arg
         * @return
         */
        String Args::getArg(const String &arg) {
            auto it = this->args_.find(arg);
            if (it != this->args_.end()){
                return it->second;
            }
            return "";
        }

        bool Args::hasArg(const String &arg) const{
            return (this->args_.find(arg) != this->args_.end());
        }

        /**
         * 遍历循环
         * @param func
         * @return
         */
        Size_t Args::forEach(const Args::ForEachKeyFunc &func) {
            Size_t index = 0;
            auto it = this->args_.begin();
            for (; it != this->args_.end(); ++it){
                func(it->first);
                ++index;
            }
            return index;
        }

        /**
         * 遍历循环
         * @param func
         * @return
         */
        Size_t Args::forEach(const Args::ForEachFunc &func) {
            Size_t index = 0;
            auto it = this->args_.begin();
            for (; it != this->args_.end(); ++it){
                func(it->first, it->second);
                ++index;
            }
            return index;
        }
    }
}

#endif //SINBACK_ARGS_H
