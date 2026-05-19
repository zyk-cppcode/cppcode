#include "PageCache.hpp"
#include "common.hpp"
#include <cstddef>
#include <mutex>

Span* PageCache::GetSpan(size_t num) {
    std::lock_guard<std::mutex> lock(GetMutex());  
    while (true) {
        if (num > NPAGES - 1) {
            // 超大对象直接 mmap
            size_t length = num * PAGE_SIZE;
            void* ptr = mmap(nullptr, length, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            Span* span = _spanPool.New();
            span->_pageid = (size_t)ptr / PAGE_SIZE;
            span->_page_num = num;
            span->_isUsing = true;
            _idSpanMap.Ensure(span->_pageid, 1);  
            _idSpanMap.set(span->_pageid, (void*)span);
            return span;
        }

        // 1. 先尝试从匹配的桶中获取
        if (!_pagelists[num].Empty()) {
            Span* kspan=_pagelists[num].PopFront();
            kspan->_isUsing = true;
        // 建立id和span的映射，方便central cache回收小块内存时，查找对应的span
        _idSpanMap.Ensure(kspan->_pageid, kspan->_page_num);
        for (size_t j = 0; j < kspan->_page_num; ++j)
            _idSpanMap.set(kspan->_pageid + j, (void*)kspan);
        return kspan;
        }

        // 2. 尝试从更大的桶分割
        for (size_t i = num + 1; i < NPAGES; ++i) {
            if (!_pagelists[i].Empty()) {
                Span* ThisSpan = _spanPool.New();
                Span* AnotherSpan = _spanPool.New();
                Span* span = _pagelists[i].PopFront();

                ThisSpan->_pageid = span->_pageid;
                ThisSpan->_page_num = num;
                AnotherSpan->_pageid = span->_pageid + num;
                AnotherSpan->_page_num = span->_page_num - num;

                _pagelists[AnotherSpan->_page_num].PushFront(AnotherSpan);
                _idSpanMap.Ensure(AnotherSpan->_pageid, AnotherSpan->_page_num);
                _idSpanMap.set(AnotherSpan->_pageid, (void*)AnotherSpan);
                _idSpanMap.set(AnotherSpan->_pageid + AnotherSpan->_page_num - 1, (void*)AnotherSpan);
    
                _idSpanMap.Ensure(ThisSpan->_pageid, ThisSpan->_page_num);
                for (size_t j = 0; j < ThisSpan->_page_num; ++j)
                    _idSpanMap.set(ThisSpan->_pageid + j, (void*)ThisSpan);
                ThisSpan->_isUsing = true;
                _spanPool.Delete(span);
                return ThisSpan;
            }
        }

        // 3. 所有桶都空，向系统申请一大块内存（NPAGES-1 页）
        Span* bigSpan = _spanPool.New();
        void* ptr = mmap(nullptr, (NPAGES - 1) * PAGE_SIZE,
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED) {
            _spanPool.Delete(bigSpan);
            return nullptr;
        }
        bigSpan->_pageid = (size_t)ptr / PAGE_SIZE;
        bigSpan->_page_num = NPAGES - 1;
        _idSpanMap.Ensure(bigSpan->_pageid, bigSpan->_page_num);
        _idSpanMap.set(bigSpan->_pageid, bigSpan);
        _idSpanMap.set(bigSpan->_pageid + bigSpan->_page_num - 1, bigSpan);
        _pagelists[NPAGES - 1].PushFront(bigSpan);
    }
}
//根据地址判断是那个 span 的部分
Span* PageCache::AddrToSpan(void* start)
{
    std::lock_guard<std::mutex> lock(GetMutex());
    size_t id = (size_t)start/PAGE_SIZE;
    
    Span* span = (Span*)_idSpanMap.get(id);
    if (span != nullptr) return span;
	else
	{
        std::cout<<id<<" not found"<<std::endl;
		assert(false);
		return nullptr;
	}

}

//回收span
void PageCache::ReleaseSpanToPageCache(Span* span)
{
    std::lock_guard<std::mutex> lock(GetMutex());

    if(span->_page_num>NPAGES-1)
    {
        munmap((void*)(span->_pageid*PAGE_SIZE),span->_page_num*PAGE_SIZE);
        _idSpanMap.set(span->_pageid, nullptr);
        _spanPool.Delete(span);
        return;
    }
    //删除idSpanMap中该span的映射
    for (size_t i = 0; i < span->_page_num; ++i)
        _idSpanMap.set(span->_pageid + i, nullptr);
    //向前合并
    while(1)
    {
        if(span->_pageid==0)
        {
            break;//没有前一个span
        }
        size_t previd=span->_pageid-1;
        //auto ret=_idSpanMap.find(previd);
        Span* ret = (Span*)_idSpanMap.get(previd);
        if(ret==nullptr)
        {
            break;//没有找到一个 span
        }
        if(ret->_isUsing==true)
        {
            break;//span正在使用
        }
        if(ret->_page_num+span->_page_num>NPAGES-1)
        {
            break;//组合起来超过大小
        }
        span->_pageid=ret->_pageid;
        span->_page_num+=ret->_page_num;
        _pagelists[ret->_page_num].Erase(ret);
        for (size_t i = 0; i < ret->_page_num; ++i)
            _idSpanMap.set(ret->_pageid + i, nullptr);
        _spanPool.Delete(ret);
    }
    //向后合并
    while(1)
    {
        size_t nextid=span->_pageid+span->_page_num;
    
        Span* ret = (Span*)_idSpanMap.get(nextid);
        if(ret==nullptr)
        {
            break;//没有找到是一个 span
        }
        if(ret->_isUsing==true)
        {
            break;//span正在使用
        }
        if(ret->_page_num+span->_page_num>NPAGES-1)
        {
            break;//组合起来超过大小
        }
        
        span->_page_num+=ret->_page_num;
        _pagelists[ret->_page_num].Erase(ret);
       
         for (size_t i = 0; i < ret->_page_num; ++i)
            _idSpanMap.set(ret->_pageid + i, nullptr);
        _spanPool.Delete(ret);
    }

    _pagelists[span->_page_num].PushFront(span);
    span->_isUsing = false;
    //添加到idSpanMap
    _idSpanMap.Ensure(span->_pageid, span->_page_num);
    _idSpanMap.set(span->_pageid, (void*)span);
    _idSpanMap.set(span->_pageid + span->_page_num - 1, (void*)span);

}