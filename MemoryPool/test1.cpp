#include <iostream>
#include <pthread.h>
#include <vector>
#include <ctime>
#include "Alloc.hpp"
#include "FixedLengthMemoryPool.hpp"
using namespace std;

struct TreeNode
{
	int _val;
	TreeNode* _left;
	TreeNode* _right;

	TreeNode()
		:_val(0)
		, _left(nullptr)
		, _right(nullptr)
	{}
};

void TestObjectPool()
{
	// 申请释放的轮次
	const size_t Rounds = 10;

	// 每轮申请释放多少次
	const size_t N = 100000;

	std::vector<TreeNode*> v1;
	v1.reserve(N);

	size_t begin1 = clock();
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v1.push_back(new TreeNode);
		}
		for (int i = 0; i < N; ++i)
		{
			delete v1[i];
		}
		v1.clear();
	}

	size_t end1 = clock();

	std::vector<TreeNode*> v2;
	v2.reserve(N);

	FixedLengthMemoryPool<TreeNode> TNPool;
	size_t begin2 = clock();
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v2.push_back(TNPool.New());
		}
		for (int i = 0; i < N; ++i)
		{
			TNPool.Delete(v2[i]);
		}
		v2.clear();
	}
	size_t end2 = clock();

	std::cout << "new cost time:" << end1 - begin1 << std::endl;
	std::cout << "object pool cost time:" << end2 - begin2 << std::endl;
}

void* Alloc1(void* arg)
{
    cout << "线程 1 开始运行" << endl;
    for (size_t i = 0; i < 5; ++i)
    {
        void* ptr = ConcurrentAlloc(6);
        cout << "线程1 申请 6字节：ptr = " << ptr << endl;
    }
    return nullptr;
}

// 线程2函数：申请 5 次 7 字节
void* Alloc2(void* arg)
{
    cout << "线程 2 开始运行" << endl;
    for (size_t i = 0; i < 5; ++i)
    {
        void* ptr = ConcurrentAlloc(7);
        cout << "线程2 申请 7字节：ptr = " << ptr << endl;
    }
    return nullptr;
}

// TLS 测试（Linux 原生 pthread）
void TLSTest()
{
    pthread_t t1, t2;
	
    // 创建线程
    pthread_create(&t1, nullptr, Alloc1, nullptr);
    pthread_create(&t2, nullptr, Alloc2, nullptr);
	

    // 等待线程结束
    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);
}

int main()
{
    
    TLSTest();
    
    return 0;
}