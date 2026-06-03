#include "CentralCache.hpp"
#include "PageCache.hpp"
#include "ThreadCache.hpp"
#include "common.hpp"
#include <cstddef>

inline ThreadCache* GetThreadCache()
{
    static thread_local ThreadCache tc;
    return &tc;
}

void* ConcurrentAlloc(size_t size)
{
    if(size>MAX_BYTES)
    {
        size_t _size=SizeClass::RoundUp(size);
        size_t npage=_size/PAGE_SIZE+1;
        //PageCache::GetInstance()->GetMutex().lock();
        Span* span=PageCache::GetInstance()->GetSpan(npage);
        //PageCache::GetInstance()->GetMutex().unlock();
        span->_isUsing=true;
        span->_objsize = size;
        return (void*)(span->_pageid*PAGE_SIZE);
    }
    else
    {
        return GetThreadCache()->Allocate(size);
    }
    
}

void ConcurrentFree(void* ptr)
{
    Span* span=PageCache::GetInstance()->AddrToSpan(ptr);
    size_t size=span->_objsize;
    
    if(size>MAX_BYTES)
    {
        //PageCache::GetInstance()->GetMutex().lock();
        //Span* span=PageCache::GetInstance()->AddrToSpan(ptr);
        PageCache::GetInstance()->ReleaseSpanToPageCache(span);
        //PageCache::GetInstance()->GetMutex().unlock();

    }
    else
    {
        GetThreadCache()->Deallocate(ptr, size);
    }
    
}