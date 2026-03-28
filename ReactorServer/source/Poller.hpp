#pragma once

#include <vector>
#include <unordered_map>
#include <sys/epoll.h>

class Channel;

#define MAXEVENTS 1024

class Poller {
public:
  Poller();
  ~Poller() = default;

  void UpdateChannel(Channel *channel);
  void RemoveChannel(Channel *channel);
  void poll(std::vector<Channel*> &active);

private:
  void Update(Channel *channel, int op);
  bool FindChannel(Channel *channel);

private:
  int _epfd;
  std::vector<epoll_event> _evs;
  std::unordered_map<int, Channel *> _channels;
};