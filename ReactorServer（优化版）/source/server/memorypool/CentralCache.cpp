#include "CentralCache.hpp"
#include "PageCache.hpp"
#include "common.hpp"

// 获取一个非空span
Span *CentralCache::FetchOneSpan(SpanList &list, size_t size) {
  Span *pos = list.Begin();
  // 遍历 spanlist，找到一个非空桶
  for (; pos != list.End(); pos = pos->_next) {
    if (pos->_freeList != nullptr) {
      return pos;
    }
  }
  // 释放桶锁
  list._mtx.unlock();
  Span *span = PageCache::GetInstance()->GetSpan(SizeClass::NumMovePage(size));
   span->_isUsing=true;
   span->_objsize=size;
 
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
size_t CentralCache::FetchRangeObj(void *&start, void *&end, size_t batchNum,size_t size) {
  size_t index = SizeClass::Index(size);
  
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

//释放的内存放入 centralcache
void CentralCache::ReleaseListToCentralCache(void *start,size_t size)
{

  size_t index = SizeClass::Index(size);
  _spanLists[index]._mtx.lock();
  
  while(start)
  {
    void* next = NextObj(start);

    Span* span=PageCache::GetInstance()->AddrToSpan(start);
    NextObj(start) = span->_freeList;
		span->_freeList = start;
		span->_use_num--;

    if(span->_use_num == 0)
    {
      // 回收span
      _spanLists[index].Erase(span);
			span->_freeList = nullptr;
			span->_next = nullptr;
			span->_prev = nullptr;
      
      PageCache::GetInstance()->ReleaseSpanToPageCache(span);
     
      break;
    }
    start = next;

  }
  _spanLists[index]._mtx.unlock();
  
}