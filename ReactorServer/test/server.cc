// #include "../source/Socket.hpp"
#include <iostream>

#include "../source/Channel.hpp"
#include "../source/Poller.hpp"
#include "../source/Socket.hpp"
#include "../logger.hpp"
#include "../source/EventLoop.hpp"
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <functional>


void HandleClose(Channel *channel) {
  std::cout << "close fd: " << channel->Fd() << std::endl;
  channel->Remove();   // 先从epoll移除
  close(channel->Fd());     // 关闭fd
  //delete channel;           // 释放channel
  channel->GetLoop()->QueueInLoop([channel]() {
        delete channel;
    });
}

void HandleRead(Channel *channel) {

  int fd = channel->Fd();
  if(fd<0)
  return;
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

void HandleEvent(EventLoop*loop,Channel *channel,int id) {
  loop->TimerRefresh(id);
  
}

// ==============================
// 新连接回调
// ==============================
void Acceptor(EventLoop*loop, Channel *lst_channel) {
  int listen_fd = lst_channel->Fd();
  int newfd = accept(listen_fd, nullptr, nullptr);
  if (newfd < 0) return;
  int id=rand();
  std::cout << "新连接：" << newfd << std::endl;

  Channel *channel = new Channel(loop, newfd);
  channel->SetReadCallBack(std::bind(HandleRead, channel));
  channel->SetWriteCallBack(std::bind(HandleWrite, channel));
  channel->SetCloseCallBack(std::bind(HandleClose, channel));
  channel->SetErrorCallBack(std::bind(HandleError, channel));
  channel->SetEventCallBack(std::bind(HandleEvent, loop,channel,id));
  channel->EnableRead();
  loop->TimerAdd(id, 10, std::bind(HandleClose, channel));
}

int main() {
  EnableConsoleLogStrategy();
  std::cout << "========= 服务器启动 ==========" << std::endl;
  EventLoop loop;
  Socket lst_sock;
  lst_sock.CreateServer(8888);
  std::cout << "===================" << std::endl;

  Channel channel(&loop, lst_sock.Fd());
  channel.SetReadCallBack(std::bind(Acceptor, &loop, &channel));
  channel.EnableRead();
  
  while (1) {
  std::cout << "=========inloop==========" << std::endl;

    loop.Loop();
  }

  return 0;
}
