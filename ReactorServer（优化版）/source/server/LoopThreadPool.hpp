#pragma once
#include "EventLoop.hpp"
#include "LoopThread.hpp"
#include <memory>
#include <vector>
class LoopThreadPool{
private:
    int _thread_count;//从属线程数量
    EventLoop *_base_loop;//主线程的EventLoop
    // 原始代码：使用裸指针 std::vector<LoopThread*>，~LoopThreadPool() 为空导致内存泄漏
    // std::vector<LoopThread*> _threads;// 原始代码
    // 优化后：使用 std::vector<std::unique_ptr<LoopThread>>，自动管理生命周期
    std::vector<std::unique_ptr<LoopThread>> _threads;//从属线程列表
    std::vector<EventLoop*> _loops;//从属线程的EventLoop列表
    int _next;//下一个被分配的线程序号
public:
    LoopThreadPool(EventLoop *base_loop);
    ~LoopThreadPool();
    void SetThreadCount(int thread_count);
    void Creat();
    EventLoop* GetNextLoop();

};