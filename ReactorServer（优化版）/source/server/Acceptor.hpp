#pragma once

#include "EventLoop.hpp"
#include "Socket.hpp"
#include "memorypool/Alloc.hpp"
using AcceptCallBack = std::function<void(int)>;
class Acceptor{
    public:
    Acceptor(EventLoop *loop, uint16_t port);
    ~Acceptor();
    // 原始代码：未定义 operator new/delete，使用全局默认分配器
    // 优化后：使用 MemoryPool 的 ConcurrentAlloc/ConcurrentFree 替代全局默认分配器
    void* operator new(size_t size) { return ConcurrentAlloc(size); }
    void operator delete(void* ptr) { ConcurrentFree(ptr); }
    void SetAcceptCallBack(const AcceptCallBack &cb);
private:
    Socket _socket;
    EventLoop *_loop;
    std::unique_ptr<Channel> _accept_channel;
    AcceptCallBack _accept_callback;
    void HandleRead();

};