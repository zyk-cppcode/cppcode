#pragma once
#include <iostream>
#include <vector>
#include <ctime>
template <class T>
class FixedLengthMemoryPool
{
    public:

    T* New()
    {
        T* p = nullptr;
        if (_freelist != nullptr)
        {
            p = (T*)_freelist;
            _freelist = *(void**)_freelist;//更新空闲链表头指针
            return (T*)p;
        }
        else 
        {
            if (_left_size < sizeof(T))
            {
            _left_size=128*1024;//设置内存池大小为128KB
            _pool=(char*)malloc(_left_size);//分配内存池
            if (_pool == nullptr)
				{
					throw std::bad_alloc();
				}
            }
            p = (T*)_pool ;
            size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			_pool += objSize;
			_left_size -= objSize;
        }
        return p;
    }
    void Delete(T* p)
    {
        *(void**)p = _freelist;//将删除的对象加入空闲链表
        _freelist = p;//更新空闲链表头指针
    }
private:
    char* _pool=nullptr;//内存池起始地址
    size_t _left_size=0;//剩余内存大小
    void* _freelist=nullptr;//空闲链表头指针
};

