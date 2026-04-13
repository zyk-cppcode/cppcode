#pragma once

#include "EventLoop.hpp"
#include "Socket.hpp"
using AcceptCallBack = std::function<void(int)>;
class Acceptor{
    public:
    Acceptor(EventLoop *loop, uint16_t port);
    ~Acceptor();
    void SetAcceptCallBack(const AcceptCallBack &cb);
private:
    Socket _socket;
    EventLoop *_loop;
    std::unique_ptr<Channel> _accept_channel;
    AcceptCallBack _accept_callback;
    void HandleRead();

};