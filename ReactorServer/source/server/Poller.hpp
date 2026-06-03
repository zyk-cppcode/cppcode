#pragma once

#include <sys/epoll.h>
#include <unordered_map>
#include <vector>

class Channel;

#define MAXEVENTS 1024

class Poller {
public:
  Poller();
  ~Poller() = default;
  // 更新Channel的事件
  void UpdateChannel(Channel *channel);
  // 从Poller中移除Channel
  void RemoveChannel(Channel *channel);
  // 等待事件发生并处理
  void Poll(std::vector<Channel *> &active);

private:
  // 更新Channel的事件
  void Update(Channel *channel, int op);
  // 查找Channel是否存在
  bool FindChannel(Channel *channel);

private:
  int _epfd;
  std::vector<epoll_event> _evs;                // 存储活跃的事件
  std::unordered_map<int, Channel *> _channels; // fd到Channel的映射
};