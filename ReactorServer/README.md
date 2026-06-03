# ReactorServer

高性能 C++17 TCP/HTTP 服务器框架，基于 Reactor 模式 + epoll I/O 多路复用，"one-event-loop-per-thread" 多线程架构。

## 架构概览

```
                       ┌─────────────┐
                       │  TcpServer  │  顶层编排
                       └──────┬──────┘
                              │
          ┌───────────────────┼───────────────────┐
          │                   │                   │
    ┌─────▼─────┐     ┌──────▼───────┐     ┌─────▼─────┐
    │ Acceptor  │     │LoopThreadPool│     │Connection │
    │ (监听fd)  │     │ (工作线程池) │     │  管理      │
    └─────┬─────┘     └──────┬───────┘     └─────┬─────┘
          │                  │                   │
          │            ┌─────▼──────┐            │
          │            │ LoopThread │  × N       │
          │            │ ┌────────┐ │            │
          │            │ │EventLoop│ │  ◄────────┘
          │            │ └───┬────┘ │   每个连接绑定
          │            └─────┼──────┘   到一个 EventLoop
          │                  │
    ┌─────▼──────────────────▼──────┐
    │          EventLoop             │
    │  ┌──────┐  ┌──────┐  ┌──────┐ │
    │  │Poller│  │Time  │  │Task  │ │
    │  │(epoll)│  │Wheel │  │Queue │ │
    │  └──┬───┘  └──────┘  └──────┘ │
    │     │                          │
    │  ┌──▼────┐    ┌──────────┐    │
    │  │Channel│───►│  Socket  │    │
    │  │(fd+cb)│    │  (RAII)  │    │
    │  └──┬────┘    └──────────┘    │
    └─────┼─────────────────────────┘
          │
    ┌─────▼──────┐
    │ Connection │
    │ Buffer IN  │  ◄── 动态读写缓冲
    │ Buffer OUT │
    │ Any context│  ◄── 协议上下文(类型擦除)
    │ 状态机     │
    └─────┬──────┘
          │
    ┌─────▼──────┐
    │ HttpServer │  ◄── HTTP 应用层
    │ 路由/静态  │
    │ HttpContext│  ◄── 状态机解析器
    │ HttpUtil   │  ◄── MIME/URL/路径安全
    └────────────┘
```

## 底层 TCP 框架

| 模块 | 说明 |
|------|------|
| `EventLoop` | epoll 事件循环，eventfd 跨线程唤醒，任务队列 |
| `Poller` | epoll 封装，fd→Channel 映射，ADD/MOD/DEL 操作 |
| `Channel` | fd + epoll 事件抽象，读写/错误/关闭回调 |
| `Socket` | RAII 套接字，非阻塞模式，SO_REUSEADDR/SO_REUSEPORT |
| `Buffer` | 动态读写缓冲，自动扩容与数据压缩，支持按行读取 |
| `TimeWheel` | 60 槽秒级定时轮，timerfd 驱动，weak_ptr 安全取消 |
| `Connection` | TCP 连接状态机，优雅关闭，协议升级，非活跃自动释放 |
| `Acceptor` | 监听端口，accept 新连接，回调通知 |
| `LoopThread` | 独立线程运行 EventLoop，安全获取 loop 实例 |
| `LoopThreadPool` | 轮询分发连接到 N 个工作线程 |
| `TcpServer` | 顶层编排，组装 Acceptor + 线程池 + 连接管理 |
| `Mutex/LockGuard` | RAII 互斥锁包装 |
| `Any` | 类型擦除容器，用于连接协议上下文 |
| `Logger` | 多级别日志（DEBUG~FATAL），策略模式（控制台/文件） |

## HTTP 应用层

| 模块 | 说明 |
|------|------|
| `HttpServer` | 方法路由（GET/POST/PUT/DELETE），静态文件服务，keep-alive |
| `HttpContext` | 状态机解析器：请求行→头部→正文，Content-Length 处理 |
| `HttpRequest` | 请求数据：方法/URL/版本/头部/查询参数/正文 |
| `HttpResponse` | 响应构造：状态码/头部/正文，重定向，Connection 控制 |
| `HttpUtil` | URL 编解码、MIME 映射、HTTP 状态码描述、路径规范化与安全校验 |

## 特性

- **one-loop-per-thread**：每个工作线程独享 epoll 实例，零竞争
- **非阻塞 I/O**：所有 socket 设为非阻塞，ET 边缘触发模式
- **优雅关闭**：DISCONNECTING 状态确保发送缓冲排空后再释放连接
- **协议升级**：Connection 通过 Any 上下文支持运行时协议切换（HTTP→WebSocket）
- **路径安全**：normalizePath 解析 `..` / `.`，防止目录穿越攻击
- **定时器机制**：60 槽时间轮 + weak_ptr 防止悬垂指针

## 构建与测试

```bash
cd ReactorServer
mkdir build && cd build
cmake .. && make

# TCP echo server (端口 8888)
./Tcpserver

# HTTP server (端口 8111)
./HttpServer

# Buffer 单元测试
./TestBuffer
```

附带 WebBench 1.5 HTTP 压力测试工具。
