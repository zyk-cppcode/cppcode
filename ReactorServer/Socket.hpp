#pragma once
#include <iostream>

#include <arpa/inet.h>
#include <cstddef>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
//
#include "spdlog/spdlog.h"
#include "logger.hpp"
class Socket {
public:
  Socket() : _sockfd(-1) {}
  Socket(int sockfd) : _sockfd(sockfd) {}
  ~Socket() { Close(); }
  // 创建套接字
  bool Create() {
    // int socket(int domain, int type, int protocol);
    _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_sockfd < 0) {
      spdlog::error("create socket error");
      return false;
    }
    spdlog::info("create socket success");
    return true;
  }
  // 绑定地址信息
  bool Bind(uint16_t port, const char *ip = "0.0.0.0") {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;            // IPv4
    addr.sin_port = htons(port);          // 端口（转字节序）
    addr.sin_addr.s_addr = inet_addr(ip); // IP地址（转字节序）

    int ret = ::bind(_sockfd, (const struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
      spdlog::error("bind socket error");
      return false;
    }
    spdlog::info("bind socket success");
    return true;
  }
  // 向服务器发起连接
  bool Connect(const char *ip, uint16_t port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    socklen_t len = sizeof(struct sockaddr_in);
    // int connect(int sockfd, struct sockaddr*addr, socklen_t len);
    int ret = connect(_sockfd, (struct sockaddr *)&addr, len);
    if (ret < 0) {
      spdlog::error("connect socket error");

      return false;
    }
    spdlog::info("connect socket success");

    return true;
  }
  // 开始监听
  bool Listen() {
    int ret = ::listen(_sockfd, 10);
    if (ret == -1) {
      spdlog::error("listen socket error");
      return false;
    }
    spdlog::info("listen success");
    return true;
  }
  // 获取新连接
  int Accept() {
    int newfd = ::accept(_sockfd, NULL, NULL);
    if(newfd >= 0)
        {
            LOG(LogLevel::INFO) << "新客户端连接，fd=" << newfd;
            return newfd;
        }

        // 这两个不是错误！是非阻塞正常返回！
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // 没有新连接，直接返回继续轮询
            return -1;
        }
        LOG(LogLevel::ERROR) << "accept failed errno=" << errno;
        return -1;
    }
    // if (newfd < 0) {
    //   spdlog::error("accept error");
    //   return -1;
    // }
    // spdlog::info("socket accept success");

    // return newfd;
  
  // 创建服务器套接字
  bool CreateServer(uint16_t port, const char *ip = "0.0.0.0") {
    if (!Create()) { // 1. 创建socket
      return false;
    }
    if (!setReuseAddr()) { // 2. 端口复用
      return false;
    }
    if (!Bind(port, ip)) { // 3. 绑定IP端口
      return false;
    }
    if (!Listen()) { // 4. 开始监听
      return false;
    }
    spdlog::info("create server success ");
    return true;
  }
  // 创建一个客户端连接
  bool CreateClient(uint16_t port, const char *ip) {
    // 1. 创建套接字，2.指向连接服务器
    if (Create() == false)
      return false;
    if (Connect(ip, port) == false)
      return false;
    return true;
  }
  // 接收数据
  ssize_t Recv(void *buf, size_t len, int flags = 0) {
    ssize_t ret = recv(_sockfd, buf, len, flags);
    if (ret < 0) {
      return -1;
    }
    return ret;
  }
  // 发送数据
  ssize_t Send(const void *buf, size_t len, int flags = 0) {
    ssize_t ret = send(_sockfd, buf, len, flags);
    if (ret < 0) {
      return -1;
    }
    return ret;
  }
  // 关闭套接字
  void Close() {
    if (_sockfd != -1) {
      close(_sockfd);
      _sockfd = -1;
    }
  }
  // 设置套接字选项---开启地址端口重用
  bool setReuseAddr() {
    int opt = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)) < 0) {
      spdlog::error("set reuse addr error");
      return false;
    }
    spdlog::info("set reuse addr success");
    return true;
  }
  // 设置套接字阻塞属性--设置为非阻塞
  void setNonBlock() {
    int fl = fcntl(_sockfd, F_GETFL);
    if (fl < 0) {
      perror("fcntl");
      return;
    }
    fcntl(_sockfd, F_SETFL, fl | O_NONBLOCK);
  }
  // 获取 fd
  int fd() const { return _sockfd; }

private:
  int _sockfd;
};