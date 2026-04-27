#include "CentralCache.hpp"
#include "PageCache.hpp"
#include "common.hpp"

// CentralCache CentralCache::_sInst;
// 获取一个非空span
Span *CentralCache::FetchOneSpan(SpanList &list, size_t size) {
  Span *pos = list.Begin();
  // 遍历 spanlist，找到一个非空桶
  for (; pos != list.End(); pos = pos->_next) {
    if (pos->_freeList != nullptr) {
      return pos;
    }
    // 是否需要释放空桶？
    //_spanLists->Erase(pos);
  }
  // 释放桶锁
  list._mtx.unlock();
  // 没有非空，从 pagecache 获取span
  // PageCache::GetInstance()->_pageMtx.lock();
  PageCache::GetInstance()->GetMutex().lock();
  Span *span = PageCache::GetInstance()->GetSpan(SizeClass::NumMovePage(size));
  // PageCache::GetInstance()->_pageMtx.unlock();
  PageCache::GetInstance()->GetMutex().unlock();

  assert(span != nullptr);
  // 分割
  // 获取 span 起始地址
  char *start = (char *)(span->_pageid * PAGE_SIZE);
  // 从 pagecache 获取的内存大小
  size_t bytes = span->_page_num * PAGE_SIZE;
  char *end = start + bytes;

  span->_freeList = start;
  start += size;
  void *tail = span->_freeList;
  while (start < end) {
    NextObj(tail) = start;
    tail = start;
    start += size;
  }
  NextObj(tail) = nullptr;
  list._mtx.lock();
  list.PushFront(span);

  return span;
}
// 获取一批freelist
size_t CentralCache::FetchRangeObj(void *&start, void *&end, size_t batchNum,
                                   size_t size) {
  size_t index = SizeClass::Index(size);
  //    std::cout << "size = " << size << ", index = " << index << std::endl;
  //    std::cout << "index = " << index
  //           << ", array size = " << sizeof(_spanLists)/sizeof(_spanLists[0])
  //           << std::endl;
  assert(index < sizeof(_spanLists) / sizeof(_spanLists[0]));
  _spanLists[index]._mtx.lock();
  Span *span = FetchOneSpan(_spanLists[index], size);
  assert(span != nullptr);
  start = span->_freeList;
  end = start;
  int i = 0;
  int getNum = 1;
  while (i < batchNum - 1 && NextObj(end)) {
    end = NextObj(end);
    i++;
    getNum++;
  }
  span->_freeList = NextObj(end);
  span->_use_num += getNum;
  NextObj(end) = nullptr;
  _spanLists[index]._mtx.unlock();
  return getNum;
}