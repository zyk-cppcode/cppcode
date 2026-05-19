#pragma once
#include "common.hpp"
#include "PageMap.hpp"
#include <cstddef>

class PageCache
{
    public:
        static PageCache* GetInstance()
    {
        static PageCache instance;   // 线程安全的延迟初始化
        return &instance;
    }
        Span* GetSpan(size_t num);
        //根据地址判断是那个 span 的部分
        Span* AddrToSpan(void* start);
        //回收span
        void ReleaseSpanToPageCache(Span* span);
    private:
        SpanList _pagelists[NPAGES];
        //std::unordered_map<size_t,Span*> _idSpanMap;
        //std::map<size_t,Span*> _idSpanMap;
        TCMalloc_PageMap3<PAGE_ID_BITS> _idSpanMap;

        bool _isUsing;
    public:
       std::mutex& GetMutex()
    {
        static std::mutex mtx;
        return mtx;
    }
    private:
	PageCache(){}
    FixedLengthMemoryPool<Span> _spanPool;
	PageCache(const PageCache&) = delete;

	static PageCache _sInst;
};