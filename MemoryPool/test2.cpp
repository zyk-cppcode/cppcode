#include <iostream>
#include <pthread.h>
#include <vector>
#include <ctime>
#include <cassert>
#include "Alloc.hpp"

using namespace std;

// 多线程分配释放函数
void* StressTest(void* arg) {
    int thread_id = *(int*)arg;
    size_t alloc_size = (thread_id % 4 + 1) * 8;  // 不同线程分配不同大小：8, 16, 24, 32 字节
    const size_t rounds = 10;      // 多轮循环，模拟持续的压力
    const size_t per_round = 5; // 每轮分配 5 万次，总量足以打满缓存

    cout << "线程 " << thread_id << " 开始，分配大小 = " << alloc_size << " 字节" << endl;

    for (size_t round = 0; round < rounds; ++round) {
        std::vector<void*> ptrs;
        ptrs.reserve(per_round);

        // 分配
        for (size_t i = 0; i < per_round; ++i) {
            std::cout << "  线程 " << thread_id << " 分配 " << alloc_size << " 字节" << std::endl;
            void* ptr = ConcurrentAlloc(alloc_size);
            assert(ptr != nullptr);
            ptrs.push_back(ptr);
        }

        // 释放
        for (void* p : ptrs) {
            ConcurrentFree(p);
        }
        ptrs.clear();

        // 每 10 轮输出一次进度
        if (round % 10 == 0 && thread_id == 0) {
            cout << "  主线程完成 " << round << " / " << rounds << " 轮" << endl;
        }
    }

    cout << "线程 " << thread_id << " 完成" << endl;
    return nullptr;
}

// 混合大小、分配与释放交替的测试（更贴近真实场景）
void* MixTest(void* arg) {
    const size_t rounds = 50;
    const size_t per_round = 30000;

    cout << "混合线程开始" << endl;

    for (size_t round = 0; round < rounds; ++round) {
        std::vector<void*> pts;
        // 分配不同大小的对象（触发多个桶的 Span 分配）
        for (size_t i = 0; i < per_round; ++i) {
            size_t size = (i % 6 + 1) * 8;  // 8, 16, 24, 32, 40, 48 字节
            pts.push_back(ConcurrentAlloc(size));
        }
        // 随机释放一部分，再分配一部分（模拟真实负载）
        for (size_t i = 0; i < pts.size(); i += 2) {
            ConcurrentFree(pts[i]);
        }
        for (size_t i = 0; i < pts.size() / 2; ++i) {
            pts.push_back(ConcurrentAlloc((i % 6 + 1) * 8));
        }
        // 全部释放
        for (size_t i = 0; i < pts.size(); ++i) {
            size_t size = (i % 6 + 1) * 8;
            ConcurrentFree(pts[i]);
        }
    }

    cout << "混合线程完成" << endl;
    return nullptr;
}

// 主测试入口
void TLSTest_Stress() {
    cout << "======== 开始压力测试 ========" << endl;
    clock_t start = clock();

    const int thread_num = 4;
    pthread_t threads[thread_num];
    int thread_ids[thread_num];

    for (int i = 0; i < thread_num; ++i) {
        thread_ids[i] = i;
        pthread_create(&threads[i], nullptr, StressTest, &thread_ids[i]);
    }

    for (int i = 0; i < thread_num; ++i) {
        pthread_join(threads[i], nullptr);
    }

    clock_t end = clock();
    cout << "======== 压力测试完成，耗时 " << (end - start) / CLOCKS_PER_SEC << " 秒 ========" << endl;

    // 追加一轮混合测试，验证回收合并逻辑
    cout << "======== 开始混合测试 ========" << endl;
    start = clock();

    pthread_t mix_thread;
    pthread_create(&mix_thread, nullptr, MixTest, nullptr);
    pthread_join(mix_thread, nullptr);

    end = clock();
    cout << "======== 混合测试完成，耗时 " << (end - start) / CLOCKS_PER_SEC << " 秒 ========" << endl;
}

int main() {
    TLSTest_Stress();
    return 0;
}