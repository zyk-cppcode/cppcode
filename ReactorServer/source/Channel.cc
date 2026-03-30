#include "Channel.hpp"
#include "../logger.hpp"
#include "Poller.hpp"

Channel::Channel(Poller *poller, int fd)
    : _fd(fd), _events(0), _revents(0), _poller(poller) {
  LOG(LogLevel::DEBUG) << "epoll create success!";
}

Channel::~Channel() { Remove(); }

int Channel::Fd() { return _fd; }
// 设置当前连接触发事件
void Channel::Setrevents(uint32_t revents) { _revents = revents; }
// 获取当前需要监控的事件
uint32_t Channel::GetEvents() { return _events; }

void Channel::SetReadCallBack(const EventCallBack &cb) { 
  _read_callback = cb; 
}
void Channel::SetWriteCallBack(const EventCallBack &cb) {
  _write_callback = cb;
}
void Channel::SetErrorCallBack(const EventCallBack &cb) {
  _error_callback = cb;
}
void Channel::SetCloseCallBack(const EventCallBack &cb) {
  _close_callback = cb;
}
void Channel::SetEventCallBack(const EventCallBack &cb) {
  _event_callback = cb;
}
// 当前是否监控了可读
bool Channel::ReadAble() { return _events & EPOLLIN; }
// 当前是否监控了可写
bool Channel::WriteAble() { return _events & EPOLLOUT; }
// 启动读事件监控
void Channel::EnableRead() {
  _events |= EPOLLIN;
  Update();
}
// 启动写事件监控
void Channel::EnableWrite() {
  _events |= EPOLLOUT;
  Update();
}
// 关闭读事件监控
void Channel::DisableRead() {
  _events &= ~EPOLLIN;
  Update();
}
// 关闭写事件监控
void Channel::DisableWrite() {
  _events &= ~EPOLLOUT;
  Update();
}
// 关闭所有事件监控
void Channel::DisableAll() {
  _events = 0;
  Update();
}
// 移除当前连接
void Channel::Remove() {
  DisableAll();
  _poller->RemoveChannel(this);
}
// 更新当前连接的事件
void Channel::Update() { _poller->UpdateChannel(this); }
// 处理事件
void Channel::HandleEvent() {
  if (_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    if (_read_callback)
      _read_callback();
  }
  if (_revents & EPOLLOUT) {
    if (_write_callback)
      _write_callback();
  }
  if (_revents & EPOLLERR) {
    if (_error_callback)
      _error_callback();
  }
  if (_revents & EPOLLHUP) {
    if (_close_callback)
      _close_callback();
  }
  if (_event_callback)
    _event_callback();
}