#include "Channel.hpp"
#include "Poller.hpp"

Channel::Channel(Poller* poller, int fd) 
  : _fd(fd),
    _events(0), 
    _revents(0),
    _poller(poller)
{}

Channel::~Channel() {RemoveEvent();}

int Channel::Fd() { 
  return _fd; 
}

void Channel::Setrevents(uint32_t revents) { 
  _revents = revents; 
}

uint32_t Channel::GetEvents() {
  return _events;
}

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

bool Channel::ReadAble() { 
  return _events & EPOLLIN; 
}
bool Channel::WriteAble() { 
  return _events & EPOLLOUT; 
}

void Channel::StartReadEvent() { 
  _events |= EPOLLIN; 
  Update();
}
void Channel::StartWriteEvent() { 
  _events |= EPOLLOUT; 
  Update();
}
void Channel::CloseReadEvent() { 
  _events &= ~EPOLLIN; 
  Update();
}
void Channel::CloseWriteEvent() { 
  _events &= ~EPOLLOUT; 
  Update();
}
void Channel::CloseAllEvent() { 
  _events = 0; 
  Update();
}
void Channel::RemoveEvent() {
_poller->RemoveChannel(this);
}
void Channel::Update(){
 _poller->UpdateChannel(this);
}
void Channel::HandleEvent() {
  if (_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    if (_read_callback) _read_callback();
  }
  if (_revents & EPOLLOUT) {
    if (_write_callback) _write_callback();
  }
  if (_revents & EPOLLERR) {
    if (_error_callback) _error_callback();
  }
  if (_revents & EPOLLHUP) {
    if (_close_callback) _close_callback();
  }
  if (_event_callback) _event_callback();
}