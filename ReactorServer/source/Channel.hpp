#pragma once

#include <cstdint>
#include <functional>
#include <sys/epoll.h>


class Poller;

using EventCallBack = std::function<void()>;

class Channel {
public:
  Channel(Poller* poller, int fd);
  ~Channel();

  int Fd();
  void Setrevents(uint32_t revents);
  uint32_t GetEvents();

  void SetReadCallBack(const EventCallBack &cb);
  void SetWriteCallBack(const EventCallBack &cb);
  void SetErrorCallBack(const EventCallBack &cb);
  void SetCloseCallBack(const EventCallBack &cb);
  void SetEventCallBack(const EventCallBack &cb);

  bool ReadAble();
  bool WriteAble();

  void StartReadEvent();
  void StartWriteEvent();
  void CloseReadEvent();
  void CloseWriteEvent();
  void CloseAllEvent();
  void RemoveEvent();
  void HandleEvent();
  void Update();
private:
  int _fd;
  uint32_t _events;
  uint32_t _revents;

  Poller* _poller;

  EventCallBack _read_callback;
  EventCallBack _write_callback;
  EventCallBack _error_callback;
  EventCallBack _close_callback;
  EventCallBack _event_callback;
};