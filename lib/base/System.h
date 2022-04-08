/**
* FileName:   Process.h
* CreateDate: 2022-03-18 15:42:31
* Author:     ticks
* Email:      2938384958@qq.com
* Des:        系统相关函数
*/
#ifndef SINBACK_SYSTEM_H
#define SINBACK_SYSTEM_H

#include <cstdlib>
#include <functional>
#include <sys/resource.h>
#include <csignal>

namespace SinBack
{
    namespace Base {
        // 注册终止处理函数，注册一次就会被执行一次，即使函数一样
        static inline bool register_func_exit(void (*func)()) {
            return (std::atexit(func) == 0);
        }

        // 系统进程资源限制类型
        enum ResourceLimitType {
            Limit_All_Size = RLIMIT_AS,
            Limit_Cpu_Time = RLIMIT_CPU,
            Limit_Data_Size = RLIMIT_DATA,
            Limit_File_Size = RLIMIT_FSIZE,
            Limit_Sig_Queue_Size = RLIMIT_MSGQUEUE,
            Limit_Fd_Size = RLIMIT_NOFILE,
            Limit_Child_Size = RLIMIT_NPROC,
            Limit_RSS_Size = RLIMIT_RSS,
            Limit_Sig_Pending_Size = RLIMIT_SIGPENDING,
            Limit_Stack_Size = RLIMIT_STACK
        };

        // 设置进程资源限制值
        static inline bool set_resource_limit(ResourceLimitType type, Size_t limit_) {
            rlimit limit{};
            limit.rlim_cur = limit_;
            limit.rlim_max = limit_;
            return (::setrlimit(type, &limit) == 0);
        }

        // 设置进程资源限制值
        static inline bool set_resource_limit64(ResourceLimitType type, Size_t limit_) {
            rlimit64 limit{};
            limit.rlim_cur = limit_;
            limit.rlim_max = limit_;
            return (::setrlimit64(type, &limit) == 0);
        }

        // fork 子进程
        static inline Int system_fork() {
            return ::fork();
        }

        // vfork 子进程
        static inline Int system_vfork() {
            return ::vfork();
        }

        // 创建匿名管道
        static inline bool system_pipe(Int fds[2]) {
            return (::pipe(fds) == 0);
        }

        // 信号处理函数
        static bool system_signal(Int sig, const Function<void(Int)>& func, Int flags = 0){
            struct sigaction action{};
            action.sa_handler = (__sighandler_t) &func;
            action.sa_flags = flags;
            return (::sigaction(sig, &action, nullptr) == 0);
        }
        // 忽略指定信号
        static bool system_ignore_sig(Int sig) {
            struct sigaction action{};
            action.sa_handler = nullptr;
            return (::sigaction(sig, &action, nullptr) == 0);
        }

        // 发送进程信号
        static inline bool system_send_sig(pid_t pid, Int sig){
            return (::kill(pid, sig) == 0);
        }

        // 获取当前工作目录
        static inline String system_get_cwd() {
            char buf[1024]{};
            return (::getcwd(buf, sizeof(buf)) != nullptr)
                   ? std::move(String (buf)) : std::move(String());
        }

        // 设置当前工作目录
        static inline bool system_set_cwd(const String& path) {
            return (::chdir(path.data()) == 0);
        }
    }
}

#endif //SINBACK_SYSTEM_H
