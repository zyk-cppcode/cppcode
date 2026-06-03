# MemoryPool

高性能多线程内存池分配器，借鉴 Google TCMalloc 设计，用现代 C++ 实现。

## 架构概览

```
ConcurrentAlloc(size)                ConcurrentFree(ptr)
      │                                     │
      ▼                                     ▼
┌─────────────┐                       ┌─────────────┐
│ ThreadCache  │  ◄─── per-thread ──►  │ ThreadCache  │
│ (208 桶)     │    零锁竞争           │ (208 桶)     │
└──────┬──────┘                       └──────┬──────┘
       │ 慢启动批量获取                       │ 链表过长时回收
       ▼                                      ▼
┌─────────────┐                       ┌─────────────┐
│ CentralCache │ ←── 全局单例 ──────► │ CentralCache │
│ (208 SpanList)│    细粒度桶锁         │ (208 SpanList)│
└──────┬──────┘                       └──────┬──────┘
       │ 请求新 Span                         │ 释放空 Span
       ▼                                      ▼
┌─────────────┐                       ┌─────────────┐
│  PageCache   │ ←── 全局单例 ──────► │  PageCache   │
│ (129 页桶)   │    Span 拆分/合并     │ (129 页桶)   │
│ 3层基数树    │    相邻 Span 合并      │ 3层基数树    │
└──────┬──────┘                       └──────┬──────┘
       │                                      │
       ▼                                      ▼
    mmap()                               munmap()
```

## 核心模块

| 模块 | 说明 |
|------|------|
| `ThreadCache` | 每线程独立空闲链表，≤256KB 分配零锁竞争，慢启动批量获取 |
| `CentralCache` | 全局中央缓存，按桶细粒度加锁，批量供给/回收对象 |
| `PageCache` | 页级分配器，通过 mmap 申请内存，支持 Span 拆分与相邻合并 |
| `SizeClass` | 大小类计算：对齐、分桶索引、批量计算、页数计算 |
| `FreeList` | 侵入式单链表，空闲指针存储在已释放的内存块内部 |
| `Span` / `SpanList` | 连续页范围描述，双向循环链表组织 |
| `PageMap` | 三级基数树，实现 O(1) 地址→Span 查找 |
| `FixedLengthMemoryPool` | 定长内存池，供 PageCache 内部分配 Span 对象使用 |

## 特性

- **三级缓存**：ThreadCache → CentralCache → PageCache，逐级兜底
- **低锁竞争**：thread_local + 每桶 mutex，最大化并发
- **慢启动**：线程缓存初始批量=1，逐步递增至上限，避免线程囤积
- **大对象旁路**：>256KB 分配直接走 PageCache + mmap
- **碎片控制**：大小类对齐 + 空闲 Span 相邻合并
- **O(1) 地址反查**：三级基数树快速定位指针所属 Span
