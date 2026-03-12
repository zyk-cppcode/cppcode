#include <iostream>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <unistd.h>

using TaskFunc = std::function<void()>;
using ReleaseFunc = std::function<void()>;

class TimeTask
{
public:
    TimeTask(uint64_t id, uint32_t delay, const TaskFunc &cb)
        : _id(id),
          _timeout(delay),
          _cbtask(cb)
    {
    }
    ~TimeTask()
    {
        if (!_canceled)
        {
            _cbtask();
        }
        _release();
    }
    void Cancel() { _canceled = true; }

    void SetRelease(const ReleaseFunc &cb) { _release = cb; }
    uint32_t DelayTime() { return _timeout; }

private:
    uint64_t _id;
    uint32_t _timeout; // 任务超时时间
    bool _canceled;    // 任务是否被取消
    TaskFunc _cbtask;
    ReleaseFunc _release;
};
using Taskptr = std::shared_ptr<TimeTask>;
using WeakTask = std::weak_ptr<TimeTask>;
class TimeWheel
{
public:
    TimeWheel()
        : _tick(0),
          _capacity(60),
          _wheel(_capacity)
    {
    }
    void RemoveTimer(uint64_t id)
    {
        auto it = _timers.find(id);
        if (it != _timers.end())
        {
            _timers.erase(it);
        }
    }
    void TimerAdd(uint64_t id, uint32_t delay, const TaskFunc &cb)
    {
        Taskptr pt(new TimeTask(id, delay, cb));
        pt->SetRelease(std::bind(&TimeWheel::RemoveTimer, this, id));
        int pos = (_tick + delay) % _capacity;

        _wheel[pos].push_back(pt);

        _timers[id] = WeakTask(pt);
    }
    // 刷新/延迟定时任务
    void TimerRefresh(uint64_t id)
    {
        // 通过保存的定时器对象的weak_ptr构造一个shared_ptr出来，添加到轮子中
        auto it = _timers.find(id);
        if (it == _timers.end())
        {
            return; // 没找着定时任务，没法刷新，没法延迟
        }
        Taskptr pt = it->second.lock(); // lock获取weak_ptr管理的对象对应的shared_ptr
        int delay = pt->DelayTime();
        int pos = (_tick + delay) % _capacity;
        _wheel[pos].push_back(pt);
    }
    void TimerCancel(uint64_t id)
    {
        auto it = _timers.find(id);
        if (it == _timers.end())
        {
            return; // 没找着定时任务，没法刷新，没法延迟
        }
        Taskptr pt = it->second.lock();
        if (pt)
            pt->Cancel();
    }
    // 秒针滴答
    void RunTimerTask()
    {
        _tick = (_tick + 1) % _capacity;
        _wheel[_tick].clear(); // 清空指定位置的数组，释放shared_ptr
    }
    ~TimeWheel() {}

private:
    int _tick;     // 秒针，释放位置
    int _capacity; // wheel容量
    std::vector<std::vector<Taskptr>> _wheel;
    std::unordered_map<uint64_t, WeakTask> _timers; // 存储任务
};
void Test()
{

    std::cout << "test" << std::endl;
}
int main()
{
    TimeWheel tw;

    tw.TimerAdd(1, 5, Test);
    tw.TimerAdd(2, 10, Test);
    tw.TimerCancel(1);
    for (int i = 0; i < 10; ++i)
    {
        sleep(1);
        std::cout << "Tick: " << i + 1 << std::endl;
        tw.RunTimerTask();
    }

    return 0;
}
