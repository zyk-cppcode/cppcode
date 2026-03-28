#pragma once

#include <cstdint>
#include <cstddef>
#include <sys/types.h>
#include <sys/socket.h>

class Socket {
public:
  Socket();
  Socket(int sockfd);
  ~Socket();

  bool Create();
  bool Bind(uint16_t port, const char *ip = "0.0.0.0");
  bool Connect(const char *ip, uint16_t port);
  bool Listen();
  int Accept();
  bool CreateServer(uint16_t port, const char *ip = "0.0.0.0");
  bool CreateClient(uint16_t port, const char *ip);

  ssize_t Recv(void *buf, size_t len, int flags = 0);
  ssize_t Send(const void *buf, size_t len, int flags = 0);
  void Close();

  bool setReuseAddr();
  void setNonBlock();
  int fd() const;

private:
  int _sockfd;
};