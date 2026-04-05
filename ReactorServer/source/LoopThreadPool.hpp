#include "EventLoop.hpp"
#include "LoopThread.hpp"
class LoopThreadPool{
private:
    int _thread_count;//从属线程数量
    EventLoop *_base_loop;//主线程的EventLoop
    std::vector<LoopThread*> _threads;//从属线程列表
    std::vector<EventLoop*> _loops;//从属线程的EventLoop列表
    int _next;//下一个被分配的线程序号
public:
    LoopThreadPool(EventLoop *base_loop);
    ~LoopThreadPool();
    void SetThreadCount(int thread_count);
    void Creat();
    EventLoop* GetNextLoop();

};