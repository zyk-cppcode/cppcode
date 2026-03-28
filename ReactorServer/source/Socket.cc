#include "Socket.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "../logger.hpp"

Socket::Socket() : _sockfd(-1) {}
Socket::Socket(int sockfd) : _sockfd(sockfd) {}
Socket::~Socket() { Close(); }

bool Socket::Create() {
  _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (_sockfd < 0) {
    LOG(LogLevel::ERROR) << "create socket error";
    return false;
  }
  LOG(LogLevel::INFO) << "create socket success";
  return true;
}

bool Socket::Bind(uint16_t port, const char *ip) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);

  int ret = ::bind(_sockfd, (const struct sockaddr *)&addr, sizeof(addr));
  if (ret == -1) {
    LOG(LogLevel::ERROR) << "bind socket error";
    return false;
  }
  LOG(LogLevel::INFO) << "bind socket success";
  return true;
}

bool Socket::Connect(const char *ip, uint16_t port) {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);
  socklen_t len = sizeof(struct sockaddr_in);
  int ret = connect(_sockfd, (struct sockaddr *)&addr, len);
  if (ret < 0) {
    LOG(LogLevel::ERROR) << "connect socket error";
    return false;
  }
  LOG(LogLevel::INFO) << "connect socket success";
  return true;
}

bool Socket::Listen() {
  int ret = ::listen(_sockfd, 10);
  if (ret == -1) {
    LOG(LogLevel::ERROR) << "listen socket error";
    return false;
  }
  LOG(LogLevel::INFO) << "listen success";
  return true;
}

int Socket::Accept() {
  int newfd = ::accept(_sockfd, NULL, NULL);
  if (newfd >= 0) {
    LOG(LogLevel::INFO) << "新客户端连接，fd=" << newfd;
    return newfd;
  }

  if (errno == EAGAIN || errno == EWOULDBLOCK) {
    return -1;
  }

  LOG(LogLevel::ERROR) << "accept failed errno=" << errno;
  return -1;
}

bool Socket::CreateServer(uint16_t port, const char *ip) {
  if (!Create()) return false;
  if (!setReuseAddr()) return false;
  if (!Bind(port, ip)) return false;
  if (!Listen()) return false;
  LOG(LogLevel::INFO) << "create server success";
  return true;
}

bool Socket::CreateClient(uint16_t port, const char *ip) {
  if (Create() == false) return false;
  if (Connect(ip, port) == false) return false;
  return true;
}

ssize_t Socket::Recv(void *buf, size_t len, int flags) {
  ssize_t ret = recv(_sockfd, buf, len, flags);
  if (ret < 0) return -1;
  return ret;
}

ssize_t Socket::Send(const void *buf, size_t len, int flags) {
  ssize_t ret = send(_sockfd, buf, len, flags);
  if (ret < 0) return -1;
  return ret;
}

void Socket::Close() {
  if (_sockfd != -1) {
    close(_sockfd);
    _sockfd = -1;
  }
}

bool Socket::setReuseAddr() {
  int opt = 1;
  if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
    LOG(LogLevel::ERROR) << "set reuse addr error";
    return false;
  }
  LOG(LogLevel::INFO) << "set reuse addr success";
  return true;
}

void Socket::setNonBlock() {
  int fl = fcntl(_sockfd, F_GETFL);
  if (fl < 0) {
    LOG(LogLevel::ERROR) << "fcntl failed";
    return;
  }
  fcntl(_sockfd, F_SETFL, fl | O_NONBLOCK);
}

int Socket::fd() const {
  return _sockfd;
}