#pragma once
#include "EventLoop.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>
class LoopThread{
    public:
    LoopThread();
    ~LoopThread();
    EventLoop* GetLoop();
 private:
  void ThreadEntry();
     
 private:
  std::mutex _mutex; //互斥锁
  std::condition_variable _cond; //条件变量
  EventLoop *_loop; //EventLoop实例
  std::thread _thread; //EventLoop 对应的线程
  
};