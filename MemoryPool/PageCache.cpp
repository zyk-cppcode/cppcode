#include "PageCache.hpp"
#include "common.hpp"
#include <cstddef>
#include <mutex>

//PageCache PageCache::_sInst;

// Span* PageCache::GetSpan(size_t num){
//     std::lock_guard<std::mutex> lock(GetMutex());
//     if(num>NPAGES-1)
//     {
//         size_t length = num * PAGE_SIZE;
//         void* ptr = mmap(
//         nullptr,                    // 由内核选择起始地址
//         length,
//         PROT_READ | PROT_WRITE,
//         MAP_PRIVATE | MAP_ANONYMOUS,
//         -1, 0
//     );
//         //Span* span=new Span;
//         Span* span=_spanPool.New();
//         span->_objsize = num*PAGE_SIZE;
//         std::cout<<"objsize= "<<span->_objsize<<std::endl;
//         span->_pageid=(size_t)ptr/PAGE_SIZE;
//         span->_page_num=num;
//         span->_isUsing = true;
//         _idSpanMap[span->_pageid] = span;
//         return span;
//     }
//     assert(num>0 && num<=NPAGES-1);
//     //对应页数的桶非空
//    if(!_pagelists[num].Empty())
//    {
//     //返回第一个 span
//     Span* kspan=_pagelists[num].PopFront();
//     // 建立id和span的映射，方便central cache回收小块内存时，查找对应的span
//         for (size_t j = 0; j < kspan->_page_num; ++j)
//         {
//             _idSpanMap[kspan->_pageid + j] = kspan;
//         }
//     return kspan;
//    }
//    //对应桶为空，一次往下找非空
//    size_t i=0;
//    for(i=num;i<NPAGES;i++)
//    {//找到一个非空桶，分割
//     if(!_pagelists[i].Empty())
//     {
//         //Span* ThisSpan=new Span;
//         Span* ThisSpan=_spanPool.New();
//         //Span* AnotherSpan=new Span;
//         Span* AnotherSpan=_spanPool.New();
//         Span* span=_pagelists[i].PopFront();
//         _idSpanMap.erase(span->_pageid);
//         _idSpanMap.erase(span->_pageid+span->_page_num-1);
//         ThisSpan->_pageid=span->_pageid;
//         ThisSpan->_page_num=num;
//         AnotherSpan->_pageid=span->_pageid+num;
//         AnotherSpan->_page_num=span->_page_num-num;
//         //分割剩下的挂到对应桶里
//         _pagelists[AnotherSpan->_page_num].PushFront(AnotherSpan);
//         // 存储nSpan的首位页号跟nSpan映射，方便page cache回收内存时进行的合并查找
//         _idSpanMap[AnotherSpan->_pageid] = AnotherSpan;
//         _idSpanMap[AnotherSpan->_pageid + AnotherSpan->_page_num - 1] = AnotherSpan;
//         // 建立id和span的映射，方便central cache回收小块内存时，查找对应的span
//         for (size_t j = 0; j < ThisSpan->_page_num; ++j)
//         {
//             _idSpanMap[ThisSpan->_pageid + j] = ThisSpan;
//         }
//         ThisSpan->_isUsing=true;
//         //delete span;
//         _spanPool.Delete(span);
//         return ThisSpan;
//    }
//    }
//    //遍历完没有非空桶，向系统申请内存
//    //Span* bigSpan=new Span;
//    Span* bigSpan=_spanPool.New();
//    void* ptr = mmap(
//     nullptr,            // 让系统自动选地址
//     (NPAGES-1) * PAGE_SIZE,    // 申请大小：128页
//     PROT_READ | PROT_WRITE,  // 可读可写
//     MAP_PRIVATE | MAP_ANONYMOUS, // 匿名映射，无文件
//     -1, 0               // 固定
//     );
//     if (ptr == MAP_FAILED) {
//     //delete bigSpan;       // 避免泄露
//     _spanPool.Delete(bigSpan);
//     return nullptr;       // 或抛异常
//     }
//     bigSpan->_pageid=(size_t)ptr/PAGE_SIZE;
//     bigSpan->_page_num=NPAGES-1;
//     _idSpanMap[bigSpan->_pageid] = bigSpan;
//     _idSpanMap[bigSpan->_pageid + bigSpan->_page_num - 1] = bigSpan;
//    _pagelists[NPAGES-1].PushFront(bigSpan);
//    //递归再次获取
//    return GetSpan(num);
// }

Span* PageCache::GetSpan(size_t num) {
    std::lock_guard<std::mutex> lock(GetMutex());  // 锁只获取一次
    while (true) {
        if (num > NPAGES - 1) {
            // 超大对象直接 mmap
            size_t length = num * PAGE_SIZE;
            std::cout << "objsize= " << length << std::endl;
            void* ptr = mmap(nullptr, length, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            Span* span = _spanPool.New();
            span->_pageid = (size_t)ptr / PAGE_SIZE;
            span->_page_num = num;
            span->_isUsing = true;
            //_idSpanMap[span->_pageid] = span;
            _idSpanMap.Ensure(span->_pageid, 1);   // 确保单个页的节点存在
            _idSpanMap.set(span->_pageid, (void*)span);
            // 最好也加尾页映射：_idSpanMap[span->_pageid + num - 1] = span;
            return span;
        }

        // 1. 先尝试从精确匹配的桶中获取
        if (!_pagelists[num].Empty()) {
            //return _pagelists[num].PopFront();
            Span* kspan=_pagelists[num].PopFront();
    // 建立id和span的映射，方便central cache回收小块内存时，查找对应的span
        // for (size_t j = 0; j < kspan->_page_num; ++j)
        // {
        //     _idSpanMap[kspan->_pageid + j] = kspan;
        // }
        _idSpanMap.Ensure(kspan->_pageid, kspan->_page_num);
        for (size_t j = 0; j < kspan->_page_num; ++j)
            _idSpanMap.set(kspan->_pageid + j, (void*)kspan);
        return kspan;
        }

        // 2. 尝试从更大的桶分割
        for (size_t i = num + 1; i < NPAGES; ++i) {
            if (!_pagelists[i].Empty()) {
                Span* ThisSpan = _spanPool.New();
                Span* AnotherSpan = _spanPool.New();
                Span* span = _pagelists[i].PopFront();

                // _idSpanMap.erase(span->_pageid);
                // _idSpanMap.erase(span->_pageid + span->_page_num - 1);

                ThisSpan->_pageid = span->_pageid;
                ThisSpan->_page_num = num;
                AnotherSpan->_pageid = span->_pageid + num;
                AnotherSpan->_page_num = span->_page_num - num;

                _pagelists[AnotherSpan->_page_num].PushFront(AnotherSpan);
                // _idSpanMap[AnotherSpan->_pageid] = AnotherSpan;
                // _idSpanMap[AnotherSpan->_pageid + AnotherSpan->_page_num - 1] = AnotherSpan;
                _idSpanMap.Ensure(AnotherSpan->_pageid, AnotherSpan->_page_num);
                _idSpanMap.set(AnotherSpan->_pageid, (void*)AnotherSpan);
                _idSpanMap.set(AnotherSpan->_pageid + AnotherSpan->_page_num - 1, (void*)AnotherSpan);
                // for (size_t j = 0; j < ThisSpan->_page_num; ++j)
                //     _idSpanMap[ThisSpan->_pageid + j] = ThisSpan;
                _idSpanMap.Ensure(ThisSpan->_pageid, ThisSpan->_page_num);
                for (size_t j = 0; j < ThisSpan->_page_num; ++j)
                    _idSpanMap.set(ThisSpan->_pageid + j, (void*)ThisSpan);
                ThisSpan->_isUsing = true;
                _spanPool.Delete(span);
                return ThisSpan;
            }
        }

        // 3. 所有桶都空，向系统申请一大块内存（NPAGES-1 页）
        Span* bigSpan = _spanPool.New();
        void* ptr = mmap(nullptr, (NPAGES - 1) * PAGE_SIZE,
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED) {
            _spanPool.Delete(bigSpan);
            return nullptr;
        }
        bigSpan->_pageid = (size_t)ptr / PAGE_SIZE;
        bigSpan->_page_num = NPAGES - 1;
        _idSpanMap.Ensure(bigSpan->_pageid, bigSpan->_page_num);
        //_idSpanMap[bigSpan->_pageid] = bigSpan;
        _idSpanMap.set(bigSpan->_pageid, bigSpan);
        //_idSpanMap[bigSpan->_pageid + bigSpan->_page_num - 1] = bigSpan;
        _idSpanMap.set(bigSpan->_pageid + bigSpan->_page_num - 1, bigSpan);
        _pagelists[NPAGES - 1].PushFront(bigSpan);

        // 不递归，继续 while 循环，下次必然会分割或直接命中
    }
}
//根据地址判断是那个 span 的部分
Span* PageCache::AddrToSpan(void* start)
{
    std::lock_guard<std::mutex> lock(GetMutex());
    size_t id = (size_t)start/PAGE_SIZE;
    
	// auto ret = _idSpanMap.find(id);
	// if (ret != _idSpanMap.end())
	// {
    //     //std::cout<<"start="<<start<<" id="<<id<<" found"<<std::endl;

	// 	return ret->second;
	// }
    Span* span = (Span*)_idSpanMap.get(id);
    if (span != nullptr) return span;
	else
	{
        std::cout<<id<<" not found"<<std::endl;
		assert(false);
		return nullptr;
	}

}

//回收span
void PageCache::ReleaseSpanToPageCache(Span* span)
{
    std::lock_guard<std::mutex> lock(GetMutex());

    if(span->_page_num>NPAGES-1)
    {
        munmap((void*)(span->_pageid*PAGE_SIZE),span->_page_num*PAGE_SIZE);
        //_idSpanMap.erase(span->_pageid);
        _idSpanMap.set(span->_pageid, nullptr);
        _spanPool.Delete(span);
        return;
    }
    //删除idSpanMap中该span的映射
    // for (size_t i = 0; i < span->_page_num; ++i)
    //         _idSpanMap.erase(span->_pageid + i);
    for (size_t i = 0; i < span->_page_num; ++i)
        _idSpanMap.set(span->_pageid + i, nullptr);
    //向前合并
    while(1)
    {
        if(span->_pageid==0)
        {
            break;//没有前一个span
        }
        size_t previd=span->_pageid-1;
        //auto ret=_idSpanMap.find(previd);
        Span* ret = (Span*)_idSpanMap.get(previd);
        if(ret==nullptr)
        {
            break;//没有找到一个 span
        }
        if(ret->_isUsing==true)
        {
            break;//span正在使用
        }
        if(ret->_page_num+span->_page_num>NPAGES-1)
        {
            break;//组合起来超过大小
        }
        span->_pageid=ret->_pageid;
        span->_page_num+=ret->_page_num;
        _pagelists[ret->_page_num].Erase(ret);
        // for (size_t i = 0; i < ret->second->_page_num; ++i)
        //     _idSpanMap.erase(ret->second->_pageid + i);
        for (size_t i = 0; i < ret->_page_num; ++i)
            _idSpanMap.set(ret->_pageid + i, nullptr);
        //::operator delete(ret->second);
        _spanPool.Delete(ret);
    }
    //向后合并
    while(1)
    {
        size_t nextid=span->_pageid+span->_page_num;
        //auto ret=_idSpanMap.find(nextid);
        Span* ret = (Span*)_idSpanMap.get(nextid);
        if(ret==nullptr)
        {
            break;//没有找到是一个 span
        }
        if(ret->_isUsing==true)
        {
            break;//span正在使用
        }
        if(ret->_page_num+span->_page_num>NPAGES-1)
        {
            break;//组合起来超过大小
        }
        
        span->_page_num+=ret->_page_num;
        _pagelists[ret->_page_num].Erase(ret);
        // for (size_t i = 0; i < ret->second->_page_num; ++i)
        //     _idSpanMap.erase(ret->second->_pageid + i);
         for (size_t i = 0; i < ret->_page_num; ++i)
            _idSpanMap.set(ret->_pageid + i, nullptr);
        //::operator delete(ret->second);
        _spanPool.Delete(ret);
    }
    //std::cout<<span->_page_num<<std::endl;

    _pagelists[span->_page_num].PushFront(span);
    span->_isUsing = false;
    //添加到idSpanMap
    // _idSpanMap[span->_pageid] = span;
	// _idSpanMap[span->_pageid+span->_page_num-1] = span;
    _idSpanMap.Ensure(span->_pageid, span->_page_num);
    _idSpanMap.set(span->_pageid, (void*)span);
    _idSpanMap.set(span->_pageid + span->_page_num - 1, (void*)span);

}