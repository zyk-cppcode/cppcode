#include "Poller.hpp"
#include "logger.hpp"
#include "Channel.hpp"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

Poller::Poller() {
  _epfd = epoll_create(1);
  _evs.resize(256);
  if (_epfd < 0) {
    LOG(LogLevel::ERROR) << "epoll create error";
    abort();
  }
  //LOG(LogLevel::DEBUG) << "epoll create success!";
}
// 更新Channel的事件
void Poller::UpdateChannel(Channel *channel) {
  bool ret = FindChannel(channel);
  if (ret == false) {
    Update(channel, EPOLL_CTL_ADD);
    _channels[channel->Fd()] = channel;
  } else {
    Update(channel, EPOLL_CTL_MOD);
  }
}
// 从Poller中移除Channel
void Poller::RemoveChannel(Channel *channel) {
  bool ret = FindChannel(channel);
  if (ret == true) {
    Update(channel, EPOLL_CTL_DEL);
    _channels.erase(channel->Fd());
  }
  else
  {
    LOG(LogLevel::WARNING) << "RemoveChannel: Channel not found in _channels";
  }
}
// 等待事件发生并处理
void Poller::Poll(std::vector<Channel *> &active) {
  int ret = epoll_wait(_epfd, _evs.data(), _evs.size(), -1);
  if (ret < 0) {
    if (errno == EINTR)
      return;
    LOG(LogLevel::ERROR) << "epoll wait error:" << strerror(errno);
    abort();
  }

  for (int i = 0; i < ret; i++) {
    auto it = _channels.find(_evs[i].data.fd);
    assert(it != _channels.end());
    it->second->Setrevents(_evs[i].events);
    active.push_back(it->second);
  }
}
// 更新Channel的事件
void Poller::Update(Channel *channel, int op) {
  if (!channel || channel->Fd() < 0) {
    return;
  }
  struct epoll_event event;
  int fd = channel->Fd();
  event.data.fd = fd;
  event.events = channel->GetEvents();

  int ret = epoll_ctl(_epfd, op, fd, &event);
  if (ret < 0) {
    
    LOG(LogLevel::ERROR) << "epoll_ctl failed, op=" << op << ", fd=" << fd
                         << ", errno=" << errno << " (" << strerror(errno)
                         << ")";
    return;
  }
}
// 查找Channel是否存在
bool Poller::FindChannel(Channel *channel) {
  int fd = channel->Fd();
  auto pos = _channels.find(fd);
  return pos != _channels.end();
}