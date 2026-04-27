#pragma once
#include "common.hpp"

class ThreadCache {
    public:
    //申请内存
    void* Allocate(size_t size);
    //释放内存
	void Deallocate(void* ptr, size_t size);
	// 从中心cache获取
	void* FetchFromCentralCache(size_t index, size_t size);
private:
	static thread_local FreeList _freeLists[NFREELIST];

};
