#pragma once
#include "common.hpp"

class CentralCache{
    public:
    // static CentralCache* GetInstance()
	// {
	// 	return &_sInst;
	// }
	static CentralCache* GetInstance()
    {
        static CentralCache instance;   // 线程安全的延迟初始化
        return &instance;
    }

    Span *FetchOneSpan(SpanList& list, size_t size);//获取一个非空span
    size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);//获取一批span
    //释放的内存放入 centralcache
    void ReleaseListToCentralCache(void *start,size_t size);
    private:
	SpanList _spanLists[NFREELIST];

private:
	CentralCache(){ }

	CentralCache(const CentralCache&) = delete;

	static CentralCache _sInst;
};