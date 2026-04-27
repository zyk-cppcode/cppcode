#include "ThreadCache.hpp"
#include "CentralCache.hpp"
#include <cassert>


thread_local FreeList ThreadCache::_freeLists[NFREELIST];
//申请内存
void* ThreadCache::Allocate(size_t size) {
	

    assert(size <= MAX_BYTES);
	size_t alignSize = SizeClass::RoundUp(size);
	size_t index = SizeClass::Index(size);

	if (!_freeLists[index].Empty())
	{
		
		return _freeLists[index].Pop();
	}
	else
	{
		return FetchFromCentralCache(index, alignSize);
	}
}
//释放内存
void ThreadCache::Deallocate(void* ptr, size_t size) {
    assert(ptr);
	assert(size <= MAX_BYTES);

	// 找对映射的自由链表桶，插入对象
	size_t index = SizeClass::Index(size);
	_freeLists[index].Push(ptr);
}
//从中心缓存申请
void* ThreadCache::FetchFromCentralCache(size_t index, size_t size) {
	void* start = nullptr;
	void* end = nullptr;
	//慢启动
	int batchNum =std::min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));
	if(batchNum==_freeLists[index].MaxSize())
	{
		_freeLists[index].MaxSize() += 5;
	}
	size_t num = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	assert(num > 0 && num <= batchNum);
	if (num == 1)
	{
		return start;
	}
	else if (num > 1)
	{
		_freeLists[index].PushRange(start, end, num - 1);
		return start;
	}
	else
	{
		return nullptr;
	}

}