/**
* FileName:   SocketUtil.h
* CreateDate: 2022-03-08 21:34:53
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_SOCKETUTIL_H
#define SINBACK_SOCKETUTIL_H

#ifdef OS_WINDOWS
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <functional>
#include "base/Base.h"
#endif

namespace SinBack
{
    namespace Base {

#ifdef OS_WINDOWS
        typedef HANDLE socket_t;
#else
#define IPV4_ADDRESS_LEN INET_ADDRSTRLEN
#define IPV6_ADDRESS_LEN INET6_ADDRSTRLEN
typedef Int socket_t;

#endif
        // 定义Socket类型和协议族类型
        enum Socket_Type {
            S_TCP = 0,
            S_UDP = 1,
        };
        enum Ip_Type {
            IP_4 = 0,
            IP_6 = 1
        };

        constexpr bool checkIpv4(Ip_Type type) {
            return (type == IP_4);
        }

        // 字节序转换
        // long 类型本地字节序转网络字节序
        static ULong htonl(ULong host_long) {
            return ::htonl((ULong) host_long);
        }

        // long 类型网络字节序转本地字节序
        static ULong ntohl(ULong net_long) {
            return ::ntohl((ULong) net_long);
        }

        // short 类型本地字节序转网络字节序
        static unsigned short htons(unsigned short host_short) {
            return ::htons((unsigned short) host_short);
        }

        // short 类型网络字节序转本地字节序
        static unsigned short ntohs(unsigned short net_short) {
            return ::ntohs((unsigned short) net_short);
        }

        // IP 地址转换
        // IP字符串转换为网络IP结构体
        static in_addr_t str_ipaddr(const Char* ip_str) {
            return inet_addr(ip_str);
        }

        static bool str_ipaddr(const Char *ip_str, in_addr *ip_addr) {
            return (inet_aton(ip_str, ip_addr) == 1);
        }

        // 网络IP地址转字符串格式
        static Char *ipaddr_str(in_addr ip_addr) {
            return inet_ntoa(ip_addr);
        }

        // 复用端口
        static void socketReuseAddress(socket_t sock){
            Int opt = 1;
            ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
        }

        // 设置套接字非阻塞
        static void setSocketNonblock(socket_t sock){
            Int flag = ::fcntl(sock, F_GETFL);
            ::fcntl(sock, F_SETFL, flag | O_NONBLOCK);
        }

        // 更新版本，支持 IPV4 与 IPV6
        static Int str_ipaddr(Ip_Type type, const Char *ip_str, void *ip_buf) {
            switch (type) {
                case IP_4:
                    return (inet_pton(PF_INET, ip_str, ip_buf) == 1);
                    break;
                case IP_6:
                    return (inet_pton(PF_INET6, ip_str, ip_buf) == 1);
                    break;
                default:
                    break;
            }
            return false;
        }

        static const Char *ipaddr_str(Ip_Type type, const void *ip_addr, Char *ip_buf, socklen_t buf_len) {
            switch (type) {
                case IP_4:
                    return inet_ntop(type, ip_addr, ip_buf, buf_len);
                    break;
                case IP_6:
                    return inet_ntop(type, ip_addr, ip_buf, buf_len);
                    break;
                default:
                    return nullptr;
                    break;
            }
            return nullptr;
        }

        /**
         * 创建 socket
         * @param ip_type : IP类型 --> IP_4 / IP_6
         * @param sock_type : SinBack 类型 --> S_TCP / S_UDP
         * @param no_block : 是否设置非阻塞
         * @param close_fork : 用 fork 打开子进程后在子进程关闭
         * @return : 成功返回套接字，失败返回 -1
         */
        static socket_t
        creatSocket(Ip_Type ip_type, Socket_Type sock_type, bool no_block = true, bool close_fork = false) {
            int domain = (ip_type == IP_4) ? PF_INET : PF_INET6;
            int type = (sock_type == S_TCP) ? SOCK_STREAM : SOCK_DGRAM;
            if (no_block) {
                type |= SOCK_NONBLOCK;
            }
            if (close_fork) {
                type |= SOCK_CLOEXEC;
            }
            return ::socket(domain, type, 0);
        }

        /**
         * 绑定socket
         * @param sock_fd : 要绑定的socket
         * @param my_addr : 要绑定的地址信息
         * @param addr_len : 存储绑定信息地址大小
         * @return : 成功返回 0 ， 失败返回 -1
         */
        static bool
        bindSocket(socket_t sock_fd, const sockaddr_in *my_addr, socklen_t addr_len) {
            return (::bind(sock_fd, (sockaddr *) (my_addr), addr_len) == 0);
        }

        /**
         * 绑定 socket
         * @param sock_fd : 要绑定的 socket
         * @param ip_type : 要绑定的 ip 类型
         * @param ip_addr : 要绑定的 ip 地址
         * @param port : 要绑定的端口
         * @return : 是否成功
         */
        static bool
        bindSocket(socket_t sock_fd, Ip_Type ip_type, const Char *ip_addr, Int port) {
            if (ip_type == IP_4) {
                sockaddr_in addr{};
                addr.sin_family = AF_INET;
                if (ip_addr == nullptr) {
                    addr.sin_addr.s_addr = htonl(INADDR_ANY);
                } else {
                    str_ipaddr(ip_type, ip_addr, &addr.sin_addr);
                }
                addr.sin_port = htons(port);
                return (::bind(sock_fd, (sockaddr *) (&addr), sizeof(addr)) == 0);
            } else if (ip_type == IP_6) {
                sockaddr_in6 addr{};
                addr.sin6_family = AF_INET6;
                if (ip_addr == nullptr) {
                    str_ipaddr(ip_type, "", &addr.sin6_addr);
                } else {
                    str_ipaddr(ip_type, ip_addr, &addr.sin6_addr);
                }
                addr.sin6_port = htons(port);
                return (::bind(sock_fd, (sockaddr *) (&addr), sizeof(addr)) == 0);
            }
            return false;
        }

        /**
         * 为 socket 创建监听队列
         * @param sock_fd : 要监听的 socket
         * @param max_listen_cnt : 最大连接队列长度
         * @return
         */
        static bool
        listenSocket(socket_t sock_fd, Int max_listen_cnt = 5) {
            return (::listen(sock_fd, max_listen_cnt) == 0);
        }

        /**
         * 接受连接
         * @param listen_fd : 监听的 socket
         * @param addr : 地址信息指针
         * @param adddr_len : 存储地址信息大小的地址
         * @return
         */
        static Int
        acceptSocket(socket_t listen_fd, sockaddr_in *addr, socklen_t *adddr_len) {
            return ::accept(listen_fd, (sockaddr *) addr, adddr_len);
        }

        static Int
        acceptSocket(socket_t listen_fd, sockaddr_in6 *addr, socklen_t *adddr_len) {
            return ::accept(listen_fd, (sockaddr *) addr, adddr_len);
        }

        /**
         * 读取socket内容
         * @param sock_fd
         * @param buf
         * @param len
         * @param flag
         * @return
         */
        static Long
        readSocket(socket_t sock_fd, void *buf, Size_t len, Int flag = 0) {
            return ::recv(sock_fd, buf, len, flag);
        }

        // 写入socket
        /**
         *
         * @param sock_fd
         * @param buf
         * @param len
         * @param flag
         * @return
         */
        static Long
        writeSocket(socket_t sock_fd, const void *buf, Size_t len, Int flag = 0) {
            return ::send(sock_fd, buf, len, flag);
        }

        static void
        closeSocket(socket_t sock_fd, Int opt = 0) {
            if (opt & SHUT_RD || opt & SHUT_WR || opt & SHUT_RDWR) {
                ::shutdown(sock_fd, opt);
                return;
            }
            ::close(sock_fd);
        }
    }
}

#endif //SINBACK_SOCKETUTIL_H
