#include "TimeWheel.hpp"
#include "Channel.hpp"
TimeTask::TimeTask(uint64_t id, uint32_t delay, const TaskFunc &cb)
    : _id(id), _timeout(delay), _cbtask(cb) {}
TimeTask::~TimeTask() {
  if (!_canceled) {
    _cbtask();
  }
  _release();
}
void TimeTask::Cancel() { _canceled = true; }

void TimeTask::SetRelease(const ReleaseFunc &cb) { _release = cb; }
uint32_t TimeTask::DelayTime() { return _timeout; }

//////////////////////////////////////////////////////////////////////////////
// TimeWheel实现
static int CreateTimerfd() {
  int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  struct itimerspec howlong;
  bzero(&howlong, sizeof(howlong));
  howlong.it_value.tv_sec = 1;    // 首次1秒后
  howlong.it_interval.tv_sec = 1; // 周期1秒
  timerfd_settime(fd, 0, &howlong, nullptr);
  return fd;
}
// void TimeWheel::RunTimerTask() {}
TimeWheel::TimeWheel(EventLoop *loop)
    : _tick(0), _capacity(60), _loop(loop), _wheel(_capacity),
      _timerfd(CreateTimerfd()), _timerfd_channel(new Channel(_loop, _timerfd)) {
  _timerfd_channel->SetReadCallBack(std::bind(&TimeWheel::OnTime, this));
  _timerfd_channel->EnableRead();
}
TimeWheel::~TimeWheel() {
    // 1. 从epoll中移除定时器fd
    _timerfd_channel->DisableAll();
    // 2. 关闭定时器文件描述符
    close(_timerfd);
}
void TimeWheel::RemoveTimer(uint64_t id) {
  auto it = _timers.find(id);
  if (it != _timers.end()) {
    _timers.erase(it);
  }
}
void TimeWheel::TimerAdd(uint64_t id, uint32_t delay, const TaskFunc &cb) {
  Taskptr pt(new TimeTask(id, delay, cb));
  pt->SetRelease(std::bind(&TimeWheel::RemoveTimer, this, id));
  int pos = (_tick + delay) % _capacity;

  _wheel[pos].push_back(pt);

  _timers[id] = WeakTask(pt);
}
// 刷新/延迟定时任务
void TimeWheel::TimerRefresh(uint64_t id) {
  // 通过保存的定时器对象的weak_ptr构造一个shared_ptr出来，添加到轮子中
  auto it = _timers.find(id);
  if (it == _timers.end()) {
    return; // 没找着定时任务，没法刷新，没法延迟
  }
  Taskptr pt = it->second.lock(); // lock获取weak_ptr管理的对象对应的shared_ptr
  int delay = pt->DelayTime();
  int pos = (_tick + delay) % _capacity;
  _wheel[pos].push_back(pt);
}
void TimeWheel::TimerCancel(uint64_t id) {
  auto it = _timers.find(id);
  if (it == _timers.end()) {
    return; // 没找着定时任务，没法刷新，没法延迟
  }
  Taskptr pt = it->second.lock();
  if (pt)
    pt->Cancel();
}
//将操作压入EventLoop线程中执行，保证线程安全
void TimeWheel::TimerAddInLoop(uint64_t id, uint32_t delay,const TaskFunc &cb) {
  _loop->QueueInLoop(std::bind(&TimeWheel::TimerAdd, this, id, delay, cb));
}
void TimeWheel::TimerRefreshInLoop(uint64_t id) {
  _loop->QueueInLoop(std::bind(&TimeWheel::TimerRefresh, this, id));
}
void TimeWheel::TimerCancelInLoop(uint64_t id) {
  _loop->QueueInLoop(std::bind(&TimeWheel::TimerCancel, this, id));
}
// 秒针滴答
void TimeWheel::RunTimerTask() {
  _tick = (_tick + 1) % _capacity;
  _wheel[_tick].clear(); // 清空指定位置的数组，释放shared_ptr
}

void TimeWheel::OnTime() {
  Readtimerfd();
  RunTimerTask();
}