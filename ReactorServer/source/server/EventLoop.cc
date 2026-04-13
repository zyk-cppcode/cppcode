#include "EventLoop.hpp"
#include "logger.hpp"
#include "Channel.hpp"
#include "TimeWheel.hpp"

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <thread>
#include <unistd.h>

void EventLoop::ReadEventfd() {
  uint64_t val;
  int res = read(_event_fd, &val, sizeof(val));
  if (res < 0) {
    if (errno == EINTR) {
      return;
    }
    // LOG(LogLevel::ERROR)<<"ReadEventfd failed!";
  }
}
EventLoop::EventLoop()
    : _tid(std::this_thread::get_id()), _event_fd(CreateEventFd()),
      _event_channel(new Channel(this, _event_fd)), _exit(false),
      _time_wheel(std::make_unique<TimeWheel>(this)) {
  _event_channel->SetReadCallBack(std::bind(&EventLoop::ReadEventfd, this));
  _event_channel->EnableRead();
}
EventLoop::~EventLoop() {}
// 创建 eventfd
int EventLoop::CreateEventFd() {
  int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (fd < 0) {
    LOG(LogLevel::ERROR) << "eventfd create failed";
    exit(1);
  }
  return fd;
}
// 事件监控->>就绪事件处理->>执行任务
void EventLoop::Loop() {
  while (!_exit) {
    std::vector<Channel *> actives;
    _poller.Poll(actives);
    for (auto a : actives) {
      a->HandleEvent();
    }
    RunAllTask();
  }
}

void EventLoop::Quit() { _exit = true; }
// 唤醒 eventloop
void EventLoop::Wakeup() {
  uint64_t val = 1;
  int res = write(_event_fd, &val, sizeof(val));
  if (res < 0) {
    if (errno == EINTR) {
      return;
    }
    LOG(LogLevel::ERROR) << "Wakeup eventfd failed";
    abort();
  }
}

// 执行任务队列里的任务
void EventLoop::RunAllTask() {
  std::vector<Functor> tasks;
  {
    std::unique_lock<std::mutex> _lock(_mutex);
    _tasks.swap(tasks);
  }
  for (auto &t : tasks) {
    t();
  }
}
// 判断将要执行的任务是否处于当前线程中，如果是则执行，不是则压入队列。
void EventLoop::RunInLoopThread(const Functor &cb) {
  if (IsInLoop()) {
    return cb();
  }
  QueueInLoop(cb);
}
// 将操作压入任务池
void EventLoop::QueueInLoop(const Functor &cb) {
  {
    std::unique_lock<std::mutex> _lock(_mutex);
    _tasks.push_back(cb);
  }
  // 唤醒任务队列
  Wakeup();
}
// 用于判断当前线程是否是EventLoop对应的线程;
bool EventLoop::IsInLoop() { return _tid == std::this_thread::get_id(); }
// 断言当前线程是EventLoop对应的线程
void EventLoop::AssertInLoopThread(){
  assert(_tid == std::this_thread::get_id() );
 }
  
// 添加/修改描述符的事件监控
void EventLoop::UpdateEvent(Channel *channel) {
  _poller.UpdateChannel(channel);
}
// 移除描述符的监控
void EventLoop::RemoveEvent(Channel *channel) {
  _poller.RemoveChannel(channel);
}
void EventLoop::TimerAdd(uint64_t id, uint32_t delay, const TaskFunc &cb) {
  _time_wheel->TimerAddInLoop(id, delay, cb);
}
void EventLoop::TimerRefresh(uint64_t id) {
  _time_wheel->TimerRefreshInLoop(id);
}
void EventLoop::TimerCancel(uint64_t id) { _time_wheel->TimerCancelInLoop(id); }
bool EventLoop::HasTimer(uint64_t id){
  return  _time_wheel->HasTimer(id);
}