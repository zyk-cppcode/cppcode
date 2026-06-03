// 原始代码：缺少 #pragma once 头文件保护，多路径包含会导致重定义错误
// 优化后：添加 #pragma once 防止同一翻译单元内重复包含
#pragma once
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

// 原始代码中 ConcurrentAlloc 未加 inline，多文件包含会导致重定义错误
// 优化后：添加 inline 关键字，支持多翻译单元包含
// inline void* ConcurrentAlloc(size_t size)
inline void* ConcurrentAlloc(size_t size)
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

// 原始代码中 ConcurrentFree 未加 inline，多文件包含会导致重定义错误
// 优化后：添加 inline 关键字，支持多翻译单元包含
// inline void ConcurrentFree(void* ptr)
inline void ConcurrentFree(void* ptr)
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