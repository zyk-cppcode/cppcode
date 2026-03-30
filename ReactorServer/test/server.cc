// #include "../source/Socket.hpp"
#include <iostream>

#include "../source/Channel.hpp"
#include "../source/Poller.hpp"
#include "../source/Socket.hpp"
#include "../logger.hpp"

#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <functional>


void HandleClose(Channel *channel) {
  std::cout << "close fd: " << channel->Fd() << std::endl;
  channel->Remove();   // 先从epoll移除
  close(channel->Fd());     // 关闭fd
  delete channel;           // 释放channel
}

void HandleRead(Channel *channel) {
  int fd = channel->Fd();
  char buf[1024] = {0};
  ssize_t ret = recv(fd, buf, 1023, 0);

  if (ret <= 0) {
    HandleClose(channel);
    return;
  }

  std::cout << "recv from " << fd << " : " << buf << std::endl;
  // 收到数据后触发发送
  channel->EnableWrite();
}

void HandleWrite(Channel *channel) {
  int fd = channel->Fd();
  const char *data = "你好吗";
  ssize_t ret = send(fd, data, strlen(data), MSG_NOSIGNAL);

  if (ret < 0) {
    HandleClose(channel);
    return;
  }

  // 发送完关闭写事件，避免一直触发
  channel->DisableWrite();
}

void HandleError(Channel *channel) {
  HandleClose(channel);
}

void HandleEvent(Channel *channel) {
  int fd = channel->Fd();
  std::cout << "fd=" << fd << " 事件触发" << std::endl;
}

// ==============================
// 新连接回调
// ==============================
void Acceptor(Poller *poller, Channel *lst_channel) {
  int listen_fd = lst_channel->Fd();
  int newfd = accept(listen_fd, nullptr, nullptr);
  if (newfd < 0) return;

  std::cout << "新连接：" << newfd << std::endl;

  Channel *channel = new Channel(poller, newfd);
  channel->SetReadCallBack(std::bind(HandleRead, channel));
  channel->SetWriteCallBack(std::bind(HandleWrite, channel));
  channel->SetCloseCallBack(std::bind(HandleClose, channel));
  channel->SetErrorCallBack(std::bind(HandleError, channel));
  channel->SetEventCallBack(std::bind(HandleEvent, channel));
  channel->EnableRead();
}

int main() {
  EnableConsoleLogStrategy();
  std::cout << "========= 服务器启动 ==========" << std::endl;
  Poller poller;
  Socket lst_sock;
  lst_sock.CreateServer(8888);
  std::cout << "===================" << std::endl;

  Channel channel(&poller, lst_sock.Fd());
  channel.SetReadCallBack(std::bind(Acceptor, &poller, &channel));
  channel.EnableRead();
  

  while (1) {
    std::vector<Channel *> actives;
    poller.Poll(actives);

    for (auto &ch : actives) {
      ch->HandleEvent();   // 正确处理事件
    }
  }

  return 0;
}
