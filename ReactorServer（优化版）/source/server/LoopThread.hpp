#pragma once
#include "EventLoop.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>
#include "memorypool/Alloc.hpp"
class LoopThread{
    public:
    LoopThread();
    ~LoopThread();
    // 原始代码：未定义 operator new/delete，使用全局默认分配器
    // 优化后：使用 MemoryPool 的 ConcurrentAlloc/ConcurrentFree 替代全局默认分配器
    void* operator new(size_t size) { return ConcurrentAlloc(size); }
    void operator delete(void* ptr) { ConcurrentFree(ptr); }
    EventLoop* GetLoop();
 private:
  void ThreadEntry();
     
 private:
  std::mutex _mutex; //互斥锁
  std::condition_variable _cond; //条件变量
  EventLoop *_loop; //EventLoop实例
  std::thread _thread; //EventLoop 对应的线程
  
};