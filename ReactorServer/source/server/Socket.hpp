#pragma once

#include <cstddef>
#include <cstdint>
#include <sys/socket.h>
#include <sys/types.h>

class Socket {
public:
  Socket();

  Socket(int sockfd);
  ~Socket();
  // 创建套接字
  bool Create();
  // 绑定地址和端口
  bool Bind(uint16_t port, const char *ip = "0.0.0.0");
  // 连接服务器
  bool Connect(const char *ip, uint16_t port);
  // 监听连接
  bool Listen();
  // 获取新链接
  int Accept();
  // 创建一个服务器套接字（测试）
  bool CreateServer(uint16_t port, const char *ip = "0.0.0.0");
  // 创建一个客户端套接字（测试）
  bool CreateClient(uint16_t port, const char *ip);
  // 发送数据
  ssize_t Recv(void *buf, size_t len, int flags = 0);
  // 接收数据
  ssize_t Send(const void *buf, size_t len, int flags = 0);
  // 关闭套接字
  void Close();
  // 设置套接字选项
  bool setReuseAddr();
  // 设置非阻塞
  void setNonBlock();
  // 获取套接字文件描述符
  int Fd() const;

private:
  int _sockfd;
};