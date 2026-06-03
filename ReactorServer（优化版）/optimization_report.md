# ReactorServer MemoryPool 优化修改文档

## 概述

将 MemoryPool 项目（基于 TCMalloc 设计的三级缓存内存池）集成到 ReactorServer 项目中，优化热点对象的动态内存分配，同时修复原始代码中存在的内存泄漏问题。

---

## 修改统计

| 类别 | 文件数 |
|------|--------|
| 新增 MemoryPool 文件 | 10 |
| 修改的 ReactorServer 文件 | 14 |
| 新增集成文件（Alloc.hpp inline 修复） | 1 |
| **总计修改** | **15 个源文件** |

---

## 修改详情

### 1. `source/server/memorypool/Alloc.hpp`

**位置：** `OptiizedReactorServer/source/server/memorypool/Alloc.hpp`

**为什么修改：** `ConcurrentAlloc()` 和 `ConcurrentFree()` 函数定义在头文件中但未加 `inline` 关键字。多个 `.cc` 文件通过各 class 的 `operator new/delete` 间接 `#include` 此头文件时，会导致 ODR（One Definition Rule）违规，链接时出现"重复定义"错误。

**改前代码：**
```cpp
void* ConcurrentAlloc(size_t size)
{
    if(size>MAX_BYTES) {
        // ...
    } else {
        return GetThreadCache()->Allocate(size);
    }
}

void ConcurrentFree(void* ptr)
{
    Span* span=PageCache::GetInstance()->AddrToSpan(ptr);
    size_t size=span->_objsize;
    if(size>MAX_BYTES) {
        PageCache::GetInstance()->ReleaseSpanToPageCache(span);
    } else {
        GetThreadCache()->Deallocate(ptr, size);
    }
}
```

**改后代码：**
```cpp
inline void* ConcurrentAlloc(size_t size)
{
    // ... (同上，添加了 inline 关键字)
}

inline void ConcurrentFree(void* ptr)
{
    // ... (同上，添加了 inline 关键字)
}
```

---

### 2. `source/server/Buffer.hpp`

**位置：** `OptiizedReactorServer/source/server/Buffer.hpp`

**为什么修改：** Buffer 是每次 TCP 连接创建 2 个的高频分配对象（输入缓冲区 + 输出缓冲区），在高并发场景下频繁 `new/delete` 会造成内存碎片和系统调用开销。使用 MemoryPool 可大幅降低分配延迟。

**改前代码：**
```cpp
#pragma once
#include <cstdint>
#include <string>
#include <vector>

class Buffer {
public:
    Buffer();
    ~Buffer();
    // ... (无 operator new/delete)
};
```

**改后代码：**
```cpp
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
    // 优化后：使用 MemoryPool 的 ConcurrentAlloc/ConcurrentFree 替代全局默认分配器
    void* operator new(size_t size) { return ConcurrentAlloc(size); }
    void operator delete(void* ptr) { ConcurrentFree(ptr); }
    // ...
};
```

---

### 3. `source/server/Socket.hpp`

**位置：** `OptiizedReactorServer/source/server/Socket.hpp`

**为什么修改：** 同 Buffer，每次连接创建一个 Socket 对象。operator new/delete 重载使分配走 MemoryPool 三级缓存（ThreadCache → CentralCache → PageCache），ThreadCache 命中时为零锁竞争分配。

**改前代码：**
```cpp
#pragma once
#include <cstddef>
#include <cstdint>
#include <sys/socket.h>
#include <sys/types.h>

class Socket {
public:
    Socket();
    Socket(int sockfd);
    ~Socket();
    // ... (无 operator new/delete)
};
```

**改后代码：**
```cpp
#pragma once
#include <cstddef>
#include <cstdint>
#include <sys/socket.h>
#include <sys/types.h>
#include "memorypool/Alloc.hpp"

class Socket {
public:
    Socket();
    Socket(int sockfd);
    ~Socket();
    // 原始代码：未定义 operator new/delete，使用全局默认分配器
    // 优化后：使用 MemoryPool 的 ConcurrentAlloc/ConcurrentFree 替代全局默认分配器
    void* operator new(size_t size) { return ConcurrentAlloc(size); }
    void operator delete(void* ptr) { ConcurrentFree(ptr); }
    // ...
};
```

---

### 4. `source/server/Channel.hpp`

**位置：** `OptiizedReactorServer/source/server/Channel.hpp`

**为什么修改：** 每次连接创建一个 Channel 对象用于 epoll 事件管理。Channel 对象包含 5 个 `std::function` 回调（每个至少 32 字节），加上其他成员共约 200 字节，属于高频分配的中等大小对象。ThreadCache 对此大小有专门桶位，命中率极高。

**改前代码：**
```cpp
#pragma once
#include <cstdint>
#include <functional>
#include <sys/epoll.h>
// ... (无 operator new/delete)
```

**改后代码：**
```cpp
#pragma once
#include <cstdint>
#include <functional>
#include <sys/epoll.h>
#include "memorypool/Alloc.hpp"
// ... 
    void* operator new(size_t size) { return ConcurrentAlloc(size); }
    void operator delete(void* ptr) { ConcurrentFree(ptr); }
```

---

### 5. `source/server/Connection.hpp` + `Connection.cc`

**位置：** `OptiizedReactorServer/source/server/Connection.hpp`、`Connection.cc`

**为什么修改：** 
1. **性能优化：** Connection 是服务端最核心的热点对象（每个客户端连接一个），添加 MemoryPool operator new/delete 使其分配走三级缓存。
2. **内存泄漏修复：** 原始代码中 `_socket`、`_channel`、`_in_buffer`、`_out_buffer` 均为裸指针，且 `~Connection()` 析构函数为空，导致每次连接断开时这四个子对象的内存全部泄漏。改用 `std::unique_ptr` 自动管理生命周期。

#### 5a. Connection.hpp - operator new/delete

**改前代码：**
```cpp
public:
  Connection(int conn_id, int sockfd, EventLoop *loop);
  ~Connection();
  void handleEvent();
```

**改后代码：**
```cpp
public:
  Connection(int conn_id, int sockfd, EventLoop *loop);
  ~Connection();
  void* operator new(size_t size) { return ConcurrentAlloc(size); }
  void operator delete(void* ptr) { ConcurrentFree(ptr); }
  void handleEvent();
```

#### 5b. Connection.hpp - 成员变量改为 unique_ptr

**改前代码：**
```cpp
  Socket *_socket;     // 套接字操作管理
  Channel *_channel;    // 链接事件管理
  Buffer *_in_buffer;  // 输入缓冲区
  Buffer *_out_buffer; // 输出缓冲区
```

**改后代码：**
```cpp
  // 原始代码：使用裸指针 Socket*, Channel*, Buffer*，且析构函数未 delete，导致内存泄漏
  // 优化后：使用 std::unique_ptr 替代裸指针，自动管理生命周期，配合 MemoryPool operator delete 高效释放
  std::unique_ptr<Socket> _socket;
  std::unique_ptr<Channel> _channel;
  std::unique_ptr<Buffer> _in_buffer;
  std::unique_ptr<Buffer> _out_buffer;
```

#### 5c. Connection.cc - 构造函数

**改前代码：**
```cpp
Connection::Connection(int conn_id, int sockfd, EventLoop *loop)
    : _conn_id(conn_id), _sockfd(sockfd),_loop(loop),
     _socket(new Socket(sockfd)),_channel(new Channel(loop, sockfd)), 
      _enable_inactive_release(false),_statu(CONNECTING) {
  // ...
  _in_buffer = new Buffer();
  _out_buffer = new Buffer();
}
Connection::~Connection() {
  // 空析构函数 —— 所有裸指针泄漏
}
```

**改后代码：**
```cpp
Connection::Connection(int conn_id, int sockfd, EventLoop *loop)
    : _conn_id(conn_id), _sockfd(sockfd),_loop(loop),
     // 原始代码：使用裸指针 new，且析构函数未 delete，内存泄漏
     // 优化后：使用 std::make_unique + MemoryPool，自动管理生命周期，零泄漏
     _socket(std::make_unique<Socket>(sockfd)),
     _channel(std::make_unique<Channel>(loop, sockfd)), 
      _enable_inactive_release(false),_statu(CONNECTING) {
  // ...
  // 原始代码：使用裸指针 new，且析构函数未 delete，内存泄漏
  // 优化后：使用 std::make_unique + MemoryPool，自动管理生命周期
  _in_buffer = std::make_unique<Buffer>();
  _out_buffer = std::make_unique<Buffer>();
}
Connection::~Connection() {
  // 原始代码：析构函数为空，_socket/_channel/_in_buffer/_out_buffer 裸指针全部泄漏
  // 优化后：std::unique_ptr 自动调用 delete，配合 MemoryPool operator delete 高效释放
}
```

---

### 6. `source/server/Acceptor.hpp`

**位置：** `OptiizedReactorServer/source/server/Acceptor.hpp`

**为什么修改：** Acceptor 负责监听端口和接受连接，使用 MemoryPool 优化其分配。

**改前/改后：** 模式同上，添加 `operator new/delete` 使用 `ConcurrentAlloc/ConcurrentFree`。

---

### 7. `source/server/LoopThread.hpp`

**位置：** `OptiizedReactorServer/source/server/LoopThread.hpp`

**为什么修改：** 每个工作线程分配一个 LoopThread 对象，使用 MemoryPool 优化其分配。

**改前/改后：** 模式同上，添加 `operator new/delete` 使用 `ConcurrentAlloc/ConcurrentFree`。

---

### 8. `source/server/TimeWheel.hpp`（TimeTask 类）

**位置：** `OptiizedReactorServer/source/server/TimeWheel.hpp`

**为什么修改：** TimeTask 是定时器任务对象，每次设置定时任务（连接超时检测、定时回调等）都会通过 `new TimeTask()` 创建，之后由 `shared_ptr` 管理。频率中等但长期运行累积量可观。

**改前代码：**
```cpp
class TimeTask
{
public:
TimeTask(uint64_t id, uint32_t delay, const TaskFunc &cb);
 ~TimeTask();
  void Run();
  // ... (无 operator new/delete)
```

**改后代码：**
```cpp
#include "memorypool/Alloc.hpp"

class TimeTask
{
public:
TimeTask(uint64_t id, uint32_t delay, const TaskFunc &cb);
 ~TimeTask();
  void Run();
  // 原始代码：未定义 operator new/delete
  // 优化后：使用 MemoryPool 的 ConcurrentAlloc/ConcurrentFree
  void* operator new(size_t size) { return ConcurrentAlloc(size); }
  void operator delete(void* ptr) { ConcurrentFree(ptr); }
```

---

### 9. `source/server/TcpServer.hpp` + `TcpServer.cc`

**位置：** `OptiizedReactorServer/source/server/TcpServer.hpp`、`TcpServer.cc`

**为什么修改：** 
1. **内存泄漏修复：** `_acceptor` 为裸指针 `Acceptor*` 且 `~TcpServer()` 为空 → 改为 `std::unique_ptr<Acceptor>`。
2. **Connection 分配优化：** `std::make_shared<Connection>()` 使用全局 `operator new`，不触发类级别的 MemoryPool 分配器。改为 `std::shared_ptr<Connection>(new Connection(...))` 使 Connection 对象通过 MemoryPool 分配。

#### 9a. TcpServer.hpp - Acceptor 指针

**改前代码：**
```cpp
        Acceptor *_acceptor;// 监控
```

**改后代码：**
```cpp
        // 原始代码：裸指针 Acceptor*，~TcpServer() 为空导致内存泄漏
        // 优化后：使用 std::unique_ptr 自动管理生命周期
        std::unique_ptr<Acceptor> _acceptor;// 监控
```

#### 9b. TcpServer.cc - Acceptor 创建

**改前代码：**
```cpp
    _acceptor = new Acceptor(&_base_loop, _port);
```

**改后代码：**
```cpp
    // 原始代码：使用裸指针 new，~TcpServer() 为空导致内存泄漏
    // 优化后：使用 std::make_unique + MemoryPool，自动管理生命周期
    _acceptor = std::make_unique<Acceptor>(&_base_loop, _port);
```

#### 9c. TcpServer.cc - Connection 创建

**改前代码：**
```cpp
    auto conn = std::make_shared<Connection>(_conn_id, fd, _pool.GetNextLoop());
```

**改后代码：**
```cpp
    // 原始代码：std::make_shared 使用全局 operator new，不触发 Connection 的类级 MemoryPool operator new
    // 优化后：使用 new Connection 触发类级 MemoryPool operator new，然后包装为 shared_ptr
    auto conn = std::shared_ptr<Connection>(new Connection(_conn_id, fd, _pool.GetNextLoop()));
```

> **说明：** `std::make_shared` 在内部分配对象 + 控制块共用的一块内存，调用的是全局 `::operator new`，不会使用类重载的 `operator new`。改用 `shared_ptr<T>(new T(...))` 后，`new T` 会调用类级别重载的 `operator new`，从而走 MemoryPool 路径。代价是 shared_ptr 控制块额外使用一次全局分配（约 32 字节），但 Connection 对象本身（约 400 字节）走 MemoryPool，整体收益仍然显著。

---

### 10. `source/server/LoopThreadPool.hpp` + `LoopThreadPool.cc`

**位置：** `OptiizedReactorServer/source/server/LoopThreadPool.hpp`、`LoopThreadPool.cc`

**为什么修改：** 原始代码 `std::vector<LoopThread*> _threads` 存储裸指针，`~LoopThreadPool()` 为空导致所有 LoopThread 对象内存泄漏。改用 `std::vector<std::unique_ptr<LoopThread>>` 自动管理生命周期。

#### 10a. LoopThreadPool.hpp

**改前代码：**
```cpp
    std::vector<LoopThread*> _threads;//从属线程列表
```

**改后代码：**
```cpp
    // 原始代码：使用裸指针 std::vector<LoopThread*>，~LoopThreadPool() 为空导致内存泄漏
    // 优化后：使用 std::vector<std::unique_ptr<LoopThread>>，自动管理生命周期
    std::vector<std::unique_ptr<LoopThread>> _threads;//从属线程列表
```

#### 10b. LoopThreadPool.cc

**改前代码：**
```cpp
                _threads[i] = new LoopThread();
```

**改后代码：**
```cpp
                // 原始代码：使用裸指针 new，~LoopThreadPool() 为空导致内存泄漏
                // 优化后：使用 std::make_unique + MemoryPool，自动管理生命周期
                _threads[i] = std::make_unique<LoopThread>();
```

---

### 11. `source/http/makefile`

**位置：** `OptiizedReactorServer/source/http/makefile`

**为什么修改：** 
1. 增加 `-I../server/memorypool` 编译选项以包含 MemoryPool 头文件。
2. 增加 `../server/memorypool/ThreadCache.cpp`、`CentralCache.cpp`、`PageCache.cpp` 到编译输入，编译 MemoryPool 的核心实现。

**改前代码：**
```makefile
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -I../server -I./

SRCS = \
    ../server/*.cc \
    *.cc \
    test.cpp
```

**改后代码：**
```makefile
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -I../server -I../server/memorypool -I./

SRCS = \
    ../server/*.cc \
    ../server/memorypool/ThreadCache.cpp \
    ../server/memorypool/CentralCache.cpp \
    ../server/memorypool/PageCache.cpp \
    *.cc \
    test.cpp
```

---

## 内存泄漏修复汇总

| 原始代码 (泄漏点) | 文件 | 修复方式 |
|---|---|---|
| `_socket` (Socket*) | Connection.hpp/cc | `std::unique_ptr<Socket>` |
| `_channel` (Channel*) | Connection.hpp/cc | `std::unique_ptr<Channel>` |
| `_in_buffer` (Buffer*) | Connection.hpp/cc | `std::unique_ptr<Buffer>` |
| `_out_buffer` (Buffer*) | Connection.hpp/cc | `std::unique_ptr<Buffer>` |
| `_acceptor` (Acceptor*) | TcpServer.hpp/cc | `std::unique_ptr<Acceptor>` |
| `_threads[i]` (LoopThread*) | LoopThreadPool.hpp/cc | `std::unique_ptr<LoopThread>` |

**原始泄漏总量（按 1000 并发连接 + 4 工作线程估算）：**
- Connection × 1000: (Socket + Channel + Buffer×2) 每次 4 个泄漏 = 4000 对象
- Acceptor: 1 个
- LoopThread: 4 个
- **合计约 4005 个对象泄漏**

---

## MemoryPool 三级缓存架构

```
operator new(size)
  → ConcurrentAlloc(size)
    ├─ ≤256KB → GetThreadCache()->Allocate(size)     【零锁竞争】
    │            ├─ 命中 FreeList → Pop() 直接返回
    │            └─ 未命中 → FetchFromCentralCache()
    │                         └─ CentralCache::FetchRangeObj()  【桶级互斥锁】
    │                              └─ FetchOneSpan() → PageCache::GetSpan() 【全局互斥锁】
    │
    └─ >256KB → PageCache::GetSpan() → mmap()
    
operator delete(ptr)
  → ConcurrentFree(ptr)
    └─ GetThreadCache()->Deallocate(ptr, size)
         ├─ Push 到线程本地 FreeList
         └─ 超出 MaxSize → ListTooLong → 归还 CentralCache
```

---

## 性能优势分析

| 优化项 | 原始方案 | 优化后方案 | 优势 |
|--------|----------|------------|------|
| Buffer 分配 | `malloc` 系统调用 | ThreadCache 命中（零锁） | 避免系统调用 |
| Socket 分配 | `malloc` 系统调用 | ThreadCache 命中（零锁） | 避免系统调用 |
| Channel 分配 | `malloc` 系统调用 | ThreadCache 命中（零锁） | 避免系统调用 |
| Connection 分配 | 全局 `::operator new` | ThreadCache 命中（零锁） | 避免系统调用 |
| TimeTask 分配 | `malloc` 系统调用 | ThreadCache 命中（零锁） | 避免系统调用 |
| 内存碎片 | 由 malloc 管理 | 固定大小桶 + Span 管理 | 减少碎片 |
| 多线程竞争 | 全局 malloc 锁 | 每线程独立 ThreadCache | 消除锁竞争 |

---

## 文件目录结构

```
OptiizedReactorServer/
├── source/
│   ├── server/
│   │   ├── memorypool/           ← 新增 MemoryPool 模块
│   │   │   ├── Alloc.hpp         (已修改: 添加 inline)
│   │   │   ├── CentralCache.cpp
│   │   │   ├── CentralCache.hpp
│   │   │   ├── common.hpp
│   │   │   ├── FixedLengthMemoryPool.hpp
│   │   │   ├── PageCache.cpp
│   │   │   ├── PageCache.hpp
│   │   │   ├── PageMap.hpp
│   │   │   ├── ThreadCache.cpp
│   │   │   └── ThreadCache.hpp
│   │   ├── Buffer.hpp            (已修改: operator new/delete)
│   │   ├── Socket.hpp            (已修改: operator new/delete)
│   │   ├── Channel.hpp           (已修改: operator new/delete)
│   │   ├── Connection.hpp        (已修改: operator new/delete + unique_ptr)
│   │   ├── Connection.cc         (已修改: make_unique + 注释)
│   │   ├── Acceptor.hpp          (已修改: operator new/delete)
│   │   ├── LoopThread.hpp        (已修改: operator new/delete)
│   │   ├── TimeWheel.hpp         (已修改: TimeTask operator new/delete)
│   │   ├── TcpServer.hpp         (已修改: unique_ptr<Acceptor>)
│   │   ├── TcpServer.cc          (已修改: make_unique, shared_ptr(new Connection))
│   │   ├── LoopThreadPool.hpp    (已修改: unique_ptr<LoopThread>)
│   │   ├── LoopThreadPool.cc     (已修改: make_unique<LoopThread>)
│   │   └── ... (其他未修改文件)
│   └── http/
│       ├── makefile              (已修改: 增加 memorypool 源文件)
│       └── ... (其他未修改文件)
├── test/
├── WebBench-master/
└── optimization_report.md        ← 本文档
```
