#include "ThreadCache.hpp"
#include "CentralCache.hpp"
#include <cassert>

static int cishu=0;
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

	// 当链表长度大于一次批量申请的内存时就开始还一段list给central cache
	if (_freeLists[index].Size() >= _freeLists[index].MaxSize())
	{
		ListTooLong(_freeLists[index], size);
	}
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
		_freeLists[index].PushRange(NextObj(start), end, num - 1);
		return start;
	}
	else
	{
		return nullptr;
	}

}

// 释放对象时，链表过长时，回收内存回到中心缓存
void ThreadCache::ListTooLong(FreeList& list, size_t size)
{
	void* start = nullptr;
	void* end = nullptr;
	list.PopRange(start, end, list.MaxSize());
	CentralCache::GetInstance()->ReleaseListToCentralCache(start, size);
}