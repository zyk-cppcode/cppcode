#include "LoopThread.hpp"
#include "EventLoop.hpp"

//创建线程传入线程函数
LoopThread::LoopThread():_loop(NULL),
                         _thread(std::thread(&LoopThread::ThreadEntry,this)){}
LoopThread::~LoopThread() {
    if (_loop != nullptr) {
        _loop->Quit();
    }
    if (_thread.joinable()) {
        _thread.join();
    }
}

//获取 eventloop
    EventLoop* LoopThread::GetLoop(){
        EventLoop *loop;
        {std::unique_lock<std::mutex> lock(_mutex);
    while(_loop == nullptr){
        _cond.wait(lock);
    }
        loop = _loop;
    }
    return loop;
    }
 //创建 loop，执行  EventLoop模块功能
  void LoopThread::ThreadEntry(){

        EventLoop loop;
        {std::unique_lock<std::mutex> lock(_mutex);
            _loop =& loop;
            _cond.notify_one();
        }
        loop.Loop();
 
  }