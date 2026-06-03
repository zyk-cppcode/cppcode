#pragma once

#include <cstdint>
#include <functional>
#include <sys/epoll.h>

class EventLoop;
class Poller;

using EventCallBack = std::function<void()>;

class Channel {
public:
  Channel(EventLoop *evlp, int fd);
  ~Channel();
// 获取文件描述符
  int Fd();
  // 获取当前连接触发的事件
  void Setrevents(uint32_t revents);
  // 获取当前需要监控的事件
  uint32_t GetEvents();
//设置读事件回调
  void SetReadCallBack(const EventCallBack &cb);
  //设置写事件回调
  void SetWriteCallBack(const EventCallBack &cb);
  //设置错误事件回调
  void SetErrorCallBack(const EventCallBack &cb);
  //设置关闭事件回调
  void SetCloseCallBack(const EventCallBack &cb);
  //设置任意事件回调
  void SetEventCallBack(const EventCallBack &cb);
// 判断当前连接是否可读
  bool ReadAble();
// 判断当前连接是否可写
  bool WriteAble();
//设置可读
  void EnableRead();
  //设置可写
  void EnableWrite();
  //关闭可读
  void DisableRead();
  //关闭可写
  void DisableWrite();
  //关闭所有事件
  void DisableAll();
  //移除当前连接
  void Remove();
  //处理事件
  void HandleEvent();
  //更新
  void Update();
  EventLoop* GetLoop(

  ){return _evlp;}
private:
  int _fd;
  uint32_t _events;// 当前需要监控的事件
  uint32_t _revents;// 当前连接触发的事件

  EventLoop* _evlp;

  EventCallBack _read_callback;//可读事件被触发的回调函数
  EventCallBack _write_callback;//可写事件被触发的回调函数
  EventCallBack _error_callback;//错误事件被触发的回调函数
  EventCallBack _close_callback;//关闭事件被触发的回调函数
  EventCallBack _event_callback;//任意事件被触发的回调函数
};