#include <cstdint>
#include <functional>
#include <sys/epoll.h>

using EventCallBack = std::function<void()>;

class channel {
public:
  channel(int fd) : _fd(fd), _events(0), _revents(0) {}
  ~channel();
  int Fd() { return _fd; }
  void Setrevents(uint32_t revents) { _revents = revents; }
  void SetReadCallBack(const EventCallBack &cb) { _read_callback = cb; }
  void SetWriteCallBack(const EventCallBack &cb) { _write_callback = cb; }
  void SetErrorCallBack(const EventCallBack &cb) { _error_callback = cb; }
  void SetCloseCallBack(const EventCallBack &cb) { _close_callback = cb; }
  void SetEventCallBack(const EventCallBack &cb) { _event_callback = cb; }
  // 当前是否可读
  bool ReadAble() { return _events & EPOLLIN; }
  // 当前是否可写
  bool WriteAble() { return _events & EPOLLOUT; }
  // 启动读事件监控
  void StartReadEvent() { _events |= EPOLLIN; }
  // 启动写事件监控
  void StartWriteEvent() { _events |= EPOLLOUT; }
  // 关闭读事件监控
  void CloseReadEvent() { _events &= ~EPOLLIN; }
  // 关闭写事件监控
  void CloseWriteEvent() { _events &= ~EPOLLOUT; }
  // 关闭所有事件监控
  void CloseAllEvent() { _events = 0; }
  // 移除监控
  void RemoveEvent() {}
  // 事件处理，一旦连接触发了事件，就调用这个函数，自己触发了什么事件如何处理自己决定
  void HandleEvent() {
    if (_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
      if (_read_callback)
        _read_callback();
      
    }
    if (_revents & EPOLLOUT) {
      if (_write_callback)
        _write_callback();
      
    }
    if (_revents & (EPOLLERR)) {
      
      if (_error_callback)
        _error_callback();
    }
    if (_revents & EPOLLHUP ) {
      
      if (_close_callback)
        _close_callback();
    }
    if (_event_callback)
        _event_callback();
  }

private:
  int _fd;
  uint32_t _events;
  uint32_t _revents;

  EventCallBack _read_callback;
  EventCallBack _write_callback;
  EventCallBack _error_callback;
  EventCallBack _close_callback;
  EventCallBack _event_callback;
};

