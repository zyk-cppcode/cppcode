#pragma once
#include <algorithm>
#include <unordered_map>
#include <map>
#include <cstring>  
#include <assert.h>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <sys/mman.h>
#include "FixedLengthMemoryPool.hpp"


static const size_t MAX_BYTES = 256 * 1024; // 256KB
static const size_t NFREELIST = 208;      // threadCache 及centralCache 桶数
static const size_t NPAGES = 129;         // PageCache 桶数
static const size_t PAGE_SIZE = 4 * 1024; // linux下一页的大小
static const size_t PAGE_SHIFT = 12;
static const int PAGE_ID_BITS = 48 - PAGE_SHIFT;

static void *&NextObj(void *obj) { return *(void **)obj; }
class FreeList {
public:
  FreeList() : _freeList(nullptr) {}
  void Push(void *obj) {
    NextObj(obj) = _freeList;
    _freeList = obj;
    ++_size;
  }
  void PushRange(void *start, void *end, size_t num) {
    NextObj(end) = _freeList;
    _freeList = start;
    _size += num;
  }

  void *Pop() {
    if (_freeList == nullptr) {
      return nullptr;
    }
    void *ret = _freeList;
    _freeList = NextObj(_freeList);
    --_size;
    return ret;
  }
  void PopRange(void *&start, void *&end, size_t n) {
    assert(n <= _size);
    start = _freeList;
    end = start;

    for (size_t i = 0; i < n - 1; ++i) {
      end = NextObj(end);
    }

    _freeList = NextObj(end);
    NextObj(end) = nullptr;
    _size -= n;
  }
  bool Empty() const { return _freeList == nullptr; }
  size_t &MaxSize() { return _maxSize; }
  size_t Size() { return _size; }	
private:
  void *_freeList;
  size_t _maxSize = 1;
  size_t _size = 0;
};

class SizeClass {
public:
   static inline size_t _RoundUp(size_t bytes, size_t alignNum) {
    return ((bytes + alignNum - 1) & ~(alignNum - 1));
  }

  static inline size_t RoundUp(size_t size) {
    if (size <= 128) {
      return _RoundUp(size, 8);
    } else if (size <= 1024) {
      return _RoundUp(size, 16);
    } else if (size <= 8 * 1024) {
      return _RoundUp(size, 128);
    } else if (size <= 64 * 1024) {
      return _RoundUp(size, 1024);
    } else if (size <= 256 * 1024) {
      return _RoundUp(size, 8 * 1024);
    } else {
      
      return _RoundUp(size, PAGE_SIZE);
    }
  }

  
  static inline size_t _Index(size_t bytes, size_t align_shift) {
    return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
  }

  // 计算映射的哪一个自由链表桶
  static inline size_t Index(size_t bytes) {
    assert(bytes <= MAX_BYTES);

    // 每个区间有多少个链
    static int group_array[4] = {16, 56, 56, 56};
    if (bytes <= 128) {
      return _Index(bytes, 3);
    } else if (bytes <= 1024) {
      return _Index(bytes - 128, 4) + group_array[0];
    } else if (bytes <= 8 * 1024) {
      return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
    } else if (bytes <= 64 * 1024) {
      return _Index(bytes - 8 * 1024, 10) + group_array[2] + group_array[1] +
             group_array[0];
    } else if (bytes <= 256 * 1024) {
      return _Index(bytes - 64 * 1024, 13) + group_array[3] + group_array[2] +
             group_array[1] + group_array[0];
    } else {
      assert(false);
    }

    return -1;
  }

  // 一次thread cache从中心缓存获取多少个
  static size_t NumMoveSize(size_t size) {
    assert(size > 0);

    int num = MAX_BYTES / size;
    if (num < 2)
      num = 2;

    if (num > 256)
      num = 256;

    return num;
  }
  static size_t NumMovePage(size_t size) {
    size_t num = NumMoveSize(size);
    size_t npage = num * size;

    npage /= PAGE_SIZE;
    if (npage == 0)
      npage = 1;

    return npage;
  }
};

class Span {
public:
  size_t _pageid = 0;
  size_t _page_num = 0;
  Span *_next = nullptr;
  Span *_prev = nullptr;
  size_t _objsize = 0;
  size_t _use_num = 0;
  void *_freeList = nullptr;
  bool _isUsing=false;
};
class SpanList {
public:
  SpanList() {
    _head = static_cast<Span*>(::operator new(sizeof(Span)));
    _head->_pageid = 0;
    _head->_page_num = 0;
    _head->_freeList = nullptr;
    _head->_use_num = 0;
    _head->_isUsing = false;
    _head->_next = _head;
    _head->_prev = _head;
  }
  Span *Begin() { return _head->_next; }
  Span *End() { return _head; }
  bool Empty() { return _head->_next == _head; }
  void PushFront(Span *newSpan) {
    assert(newSpan);
    Insert(Begin(), newSpan);
  }
  Span *PopFront() {
    Span *span = _head->_next;
    Erase(span);
    return span;
  }
  void Insert(Span *pos, Span *newSpan) {
    assert(pos);
    assert(newSpan);

    Span *prev = pos->_prev;
    prev->_next = newSpan;
    newSpan->_prev = prev;
    newSpan->_next = pos;
    pos->_prev = newSpan;
  }
  void Erase(Span *pos) {
    assert(pos);
    assert(pos != _head);
    pos->_prev->_next = pos->_next;
    pos->_next->_prev = pos->_prev;
    pos->_next = nullptr;
    pos->_prev = nullptr;
  }

private:
  Span *_head;

public:
  std::mutex _mtx;
};