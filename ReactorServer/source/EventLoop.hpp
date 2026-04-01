#include "Channel.hpp"
#include "common.cpp"
#include "../logger.hpp"
#include "Poller.hpp"
#include <functional>
#include <memory>
#include <sys/eventfd.h>
#include <mutex>
#include <thread>
#include <vector>
using Functor = std::function<void()>;
class TimeWheel;

class EventLoop {
public:
  EventLoop();
  ~EventLoop();
  void Loop(); // 事件监控->>就绪事件处理->>执行任务
  void Quit();
  void Wakeup();
  void RunAllTask();//执行任务队列里的任务
  void RunInLoopThread(const Functor &cb); // 判断将要执行的任务是否处于当前线程中，如果是则执行，不是则压入队列。
  void QueueInLoop(const Functor &cb); // 将操作压入任务池
  bool IsInLoop(); // 用于判断当前线程是否是EventLoop对应的线程;
  void UpdateEvent(Channel *channel); // 添加/修改描述符的事件监控
  void RemoveEvent(Channel *channel); // 移除描述符的监控
   void TimerAdd(uint64_t id, uint32_t delay, const TaskFunc &cb);
    void TimerRefresh(uint64_t id);
    void TimerCancel(uint64_t id);
    bool HasTimer(uint64_t id);
public:
  int CreateEventFd();//创建 eventfd
  void ReadEventfd();//读 eventfd，唤醒
  private:
  std::thread::id _tid; // 线程 Id，判断是否本线程
  Poller _poller;       // 监控 channel
  int _event_fd;
  std::unique_ptr<Channel> _event_channel;
  std::vector<Functor> _tasks; // 任务队列
  bool _exit;                  // 判断是否退出
  std::mutex _mutex;
  std::unique_ptr<TimeWheel> _time_wheel; // 定时器
};