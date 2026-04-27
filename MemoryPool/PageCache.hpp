#pragma once
#include "common.hpp"
#include <cstddef>

class PageCache
{
    public:
        // static PageCache* GetInstance()
        // {
        //     return &_sInst;
        // }
        static PageCache* GetInstance()
    {
        static PageCache instance;   // 线程安全的延迟初始化
        return &instance;
    }
        Span* GetSpan(size_t num);
    private:
        SpanList _pagelists[NPAGES];
    public:
       std::mutex& GetMutex()
{
    static std::mutex mtx;
    return mtx;
}
    private:
	PageCache(){}

	PageCache(const PageCache&) = delete;

	static PageCache _sInst;
};