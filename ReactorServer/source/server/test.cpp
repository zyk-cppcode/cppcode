// #include "EventLoop.hpp"
// #include <iostream>
// #include <thread>


// using namespace std;

// int main() {
//     EnableConsoleLogStrategy();
//     cout << "=== EventLoop 测试开始 ===" << endl;

//     // 创建一个 loop
//     EventLoop loop;

//     // 测试：往 loop 扔一个任务
//     loop.RunInLoopThread([](){
//         cout << "【任务执行】：我是异步任务！" << endl;
//     });

//     // 开启事件循环（会一直跑）
//     loop.Loop();

//     return 0;
// }

#include "EventLoop.hpp"
#include <thread>
#include <chrono>
int main() {
    EventLoop loop;
    std::thread t([&](){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        loop.RunInLoopThread([](){
            printf("跨线程任务执行成功！\n");
        });
    });
    loop.Loop();
    t.join();
    return 0;
}