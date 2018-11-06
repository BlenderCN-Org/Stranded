#ifndef __MEMORY_POOL_H__
#define __MEMORY_POOL_H__

#include "TimeCheck.h"

/*
 * 固定大小缓冲池
 * 实现：每次从List的头上取内存，如果取不到则重新分配一定数量；
 * 用完后把内存放回List头部.
 */
template <typename T> class CMemoryPool
{
public:
	enum { EXPANSION_SIZE = 32 };

	CMemoryPool(unsigned int uiItemCount = EXPANSION_SIZE)
	{
		ExpandFreeList(uiItemCount);
	}

	~CMemoryPool()
	{
		// free all memory in the List
		CMemoryPool<T>* pNext = NULL;
		for (pNext = m_pFreeList; pNext != NULL; pNext = m_pFreeList)
		{
			m_pFreeList = m_pFreeList->m_pFreeList;
			delete[] (char*)pNext;
		}
	}

	void* Alloc(unsigned int /*size*/)
	{
		if (m_pFreeList == NULL)
		{
			ExpandFreeList();
		}

		// get free memory from head
		CMemoryPool<T>* pHead = m_pFreeList;
		m_pFreeList = m_pFreeList->m_pFreeList;
		return pHead;
	}

	void Free(void* p)
	{
		// push the free memory back to List
		CMemoryPool<T>* pHead = static_cast<CMemoryPool<T>*>(p);
		pHead->m_pFreeList = m_pFreeList;
		m_pFreeList = pHead;
	}

protected:
	// allocate memory and push to the list
	void ExpandFreeList(unsigned uiItemCount = EXPANSION_SIZE)
	{
		unsigned int uiSize = sizeof(T) > sizeof(CMemoryPool<T>*) ? sizeof(T) : sizeof(CMemoryPool<T>*);
		CMemoryPool<T>* pLastItem = static_cast<CMemoryPool<T>*>(static_cast<void*>(new char[uiSize]));
		m_pFreeList = pLastItem;

		for (int i = 0; i < uiItemCount - 1; ++i)
		{
			pLastItem->m_pFreeList = static_cast<CMemoryPool<T>*>(static_cast<void*>(new char[uiSize]));
			pLastItem = pLastItem->m_pFreeList;
		}

		pLastItem->m_pFreeList = NULL;
	}
private:
	CMemoryPool<T>* m_pFreeList;
};

// 多线程临界资源类
class CCtriticalSection
{
public:
	CCtriticalSection()
	{
		::InitializeCriticalSection(&m_cs);
	}

	~CCtriticalSection()
	{
		::DeleteCriticalSection(&m_cs);
	}

	void Lock()
	{
		::EnterCriticalSection(&m_cs);
	}

	void Unlock()
	{
		::LeaveCriticalSection(&m_cs);
	}

protected:
	CRITICAL_SECTION m_cs;
};

// 多线程版内存池
template <typename POOLTYPE, typename LOCKTYPE> class CMTMemoryPool
{
public:
	void* MTAlloc(unsigned int uiSize)
	{
		void* p = NULL;

		m_lock.Lock();
		p = m_pool.Alloc(uiSize);
		m_lock.Unlock();

		return p;
	}

	void MTFree(void* p)
	{
		m_lock.Lock();
		m_pool.Free(p);
		m_lock.Unlock();
	}

private:
	POOLTYPE m_pool;
	LOCKTYPE m_lock;
};

#endif

/*
// 用法
class CTest
{
public:
	void* operator new(size_t size)
	{
		void* p = sm_pool->MTAlloc(size);
		return p;
	}

	void operator delete(void* p, size_t size)
	{
		sm_pool->MTFree(p);
	}

	static void NewPool()
	{
		sm_pool = new CMTMemoryPool < CMemoryPool<CTest>, CCtriticalSection > ;
	}

	static void DeletePool()
	{
		if (sm_pool)
		{
			delete sm_pool;
			sm_pool = NULL;
		}
	}
public:
	static CMTMemoryPool<CMemoryPool<CTest>, CCtriticalSection>* sm_pool;

	int m_iN;
	int m_iN1;
};

CMTMemoryPool<CMemoryPool<CTest>, CCtriticalSection>* CTest::sm_pool = NULL;

void TestFun()
{
	int i = 0;
	const int iLoop = 10;
	const int iCount = 10000;

	for (int j = 0; j < iLoop; ++j)
	{
		typedef CTest* LPTest;
		LPTest arData[iCount];
		for (i = 0; i < iCount; ++i)
		{
			arData[i] = new CTest;
		}

		for (i = 0; i < iCount; ++i)
		{
			delete arData[i];
		}
	}
}

int main(int argc, char* argv[])
{
	CTimeCheck chk;

	chk.QueryPerformanceStart();
	chk.TimeStart();
	chk.RDTSCStart();

	CTest::NewPool();

	TestFun();

	CTest::DeletePool();

	std::cout << "Total cost: " << chk.QueryPerformanceEnd() << std::endl;
	std::cout << "Total cost: " << chk.TimeEnd() << std::endl;
	printf("Use time: %lld", chk.RDTSCStop());

	system("pause");

	return 0;
}
*/