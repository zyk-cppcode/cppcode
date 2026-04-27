#include "PageCache.hpp"
#include "common.hpp"
#include <cstddef>

//PageCache PageCache::_sInst;

Span* PageCache::GetSpan(size_t num){
    assert(num>0 && num<NPAGES-1);
    //对应页数的桶非空
   if(!_pagelists[num].Empty())
   {
    //返回第一个 span
    return _pagelists[num].PopFront();
   }
   //对应桶为空，一次往下找非空
   size_t i=0;
   for(i=num;i<NPAGES;i++)
   {//找到一个非空桶，分割
    if(!_pagelists[i].Empty())
   {
    Span* ThisSpan=new Span;
    Span* AnotherSpan=new Span;
    Span* span=_pagelists[i].PopFront();
    ThisSpan->_pageid=span->_pageid;
    ThisSpan->_page_num=num;
    AnotherSpan->_pageid=span->_pageid+num;
    AnotherSpan->_page_num=span->_page_num-num;
    //分割剩下的挂到对应桶里
    _pagelists[i-num].PushFront(AnotherSpan);
    delete span;
    return ThisSpan;
   }
   }
   //遍历完没有非空桶，向系统申请内存
   Span* bigSpan=new Span;
   void* ptr = mmap(
    nullptr,            // 让系统自动选地址
    128 * PAGE_SIZE,    // 申请大小：128页
    PROT_READ | PROT_WRITE,  // 可读可写
    MAP_PRIVATE | MAP_ANONYMOUS, // 匿名映射，无文件
    -1, 0               // 固定
    );
    if (ptr == MAP_FAILED) {
    delete bigSpan;       // 避免泄露
    return nullptr;       // 或抛异常
    }
    bigSpan->_pageid=(size_t)ptr/PAGE_SIZE;
    bigSpan->_page_num=128;
   _pagelists[128].PushFront(bigSpan);
   //递归再次获取
   return GetSpan(num);
}
