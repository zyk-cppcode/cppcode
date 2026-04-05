#include "LoopThreadPool.hpp"
 LoopThreadPool::LoopThreadPool(EventLoop *base_loop) 
        :_thread_count(0),_base_loop(base_loop),_next(0)  {

 }
    LoopThreadPool::~LoopThreadPool(){}
    void LoopThreadPool::SetThreadCount(int thread_count)
    {_thread_count=thread_count;}

    void LoopThreadPool::Creat(){
        if(_thread_count>0)
        {
            _threads.resize(_thread_count);
            _loops.resize(_thread_count);
            for(int i=0;i<_thread_count;i++)
            {

                _threads[i] = new LoopThread();
                _loops[i] =_threads[i]->GetLoop();
            }
        }
    }
    EventLoop* LoopThreadPool::GetNextLoop(){
        if(_thread_count==0)
        {  return _base_loop;
        }
        else
        {
            _next=(_next+1)%_thread_count;
        return _loops[_next];
        }  
    }
