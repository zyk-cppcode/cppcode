#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "memorypool/Alloc.hpp"

class Buffer {
public:
    Buffer();
    ~Buffer();
    // 原始代码：未定义 operator new/delete，使用全局默认分配器，每次 new Buffer 调用 malloc
    // void* operator new(size_t size);  // 原始代码中不存在
    // void operator delete(void* ptr);  // 原始代码中不存在
    // 优化后：使用 MemoryPool 的 ConcurrentAlloc/ConcurrentFree 替代全局默认分配器
    void* operator new(size_t size) { return ConcurrentAlloc(size); }
    void operator delete(void* ptr) { ConcurrentFree(ptr); }
// 获取写入数据的起始地址
    char* getWriteOffset();
// 获取读取数据的起始地址
    char* getReadOffset();
    // 获取前面空闲的大小
    uint64_t getFrontFreeSize();
    // 获取后面空闲的大小
    uint64_t getRearFreeSize();
    // 获取可读数据的大小
    uint64_t getReadableSize();

    char& operator[](int index);
    // 移动读偏移
    void moveReadOffset(uint64_t size);
    // 移动写偏移
    void moveWriteOffset(uint64_t size);
    // 获取总大小
    uint64_t getTotalSize();
    // 获取可写数据的大小
    uint64_t getWritableSize();
// 确保有足够的空间写入数据
    void ensureWriteSize(uint64_t size);
    // 写入数据
    void write(const char* data, uint64_t len);
    // 写入字符串
    void write(std::string str);
    // 读取数据
    void read(char* buf, uint64_t len);
    void read(std::string& buf, uint64_t len);
    std::string GetLine();
    void clear();

private:
    std::vector<char> _buffer;// 数据缓冲区
    uint64_t _writeOffset;// 写偏移
    uint64_t _readOffset;// 读偏移
};