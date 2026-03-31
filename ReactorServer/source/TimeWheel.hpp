#include <iostream>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <unistd.h>
#include <sys/timerfd.h>
#include <strings.h>
#include "Channel.hpp"
#include "EventLoop.hpp"
using TaskFunc = std::function<void()>;
using ReleaseFunc = std::function<void()>;
//任务类
class TimeTask
{
public:
TimeTask(uint64_t id, uint32_t delay, const TaskFunc &cb);
 ~TimeTask();
 void Cancel();
 void SetRelease(const ReleaseFunc &cb);
 uint32_t DelayTime();
 private:
    uint64_t _id;
    uint32_t _timeout; // 任务超时时间
    bool _canceled;    // 任务是否被取消
    TaskFunc _cbtask;
    ReleaseFunc _release;
};
//时间轮
using Taskptr = std::shared_ptr<TimeTask>;
using WeakTask = std::weak_ptr<TimeTask>;
class TimeWheel
{
public:
    TimeWheel(EventLoop *loop);
    ~TimeWheel();
    void RemoveTimer(uint64_t id);
    void TimerAdd(uint64_t id, uint32_t delay, const TaskFunc &cb);
    void TimerRefresh(uint64_t id);// 刷新/延迟定时任务
    void TimerCancel(uint64_t id);
    void OnTime();//
    void RunTimerTask();// 秒针滴答
    //将操作压入EventLoop线程中执行，保证线程安全
    void TimerAddInLoop(uint64_t id, uint32_t delay, const TaskFunc &cb);
    void TimerRefreshInLoop(uint64_t id);
    void TimerCancelInLoop(uint64_t id);

    private:
    static void Readtimerfd();
    static int CreateTimerfd();
    private:
    int _tick;     // 秒针，释放位置
    int _capacity; // wheel容量
    EventLoop *_loop;
    int _timerfd;
    std::unique_ptr<Channel> _timerfd_channel;
    std::vector<std::vector<Taskptr>> _wheel;
    std::unordered_map<uint64_t, WeakTask> _timers; // 存储任务
};
