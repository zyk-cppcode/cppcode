#include "ThreadCache.hpp"

inline ThreadCache* GetThreadCache()
{
    static thread_local ThreadCache tc;
    return &tc;
}

void* ConcurrentAlloc(size_t size)
{
    return GetThreadCache()->Allocate(size);
}

void ConcurrentFree(void* ptr, size_t size)
{
    GetThreadCache()->Deallocate(ptr, size);
}