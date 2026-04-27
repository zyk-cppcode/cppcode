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
    private:
	SpanList _spanLists[NFREELIST];

private:
	CentralCache(){ std::cout << "CentralCache 构造完成" << std::endl;}

	CentralCache(const CentralCache&) = delete;

	static CentralCache _sInst;
};