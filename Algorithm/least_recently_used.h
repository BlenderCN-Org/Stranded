/*
 * LRU Cache - Cache 置换算法. "最近最少使用"
 *
 * 要求:
 * 1、只提供两个接口，一个获取数据 getValue(key), 一个写入数据 putValue(key, value)
 * 2、无论是获取还是写入数据，当前这个数据要保持在最容易访问的位置
 * 3、缓存数量有限，最长时间没被访问的数据应该置换出缓存
 *
 * 算法：
 * 使用双向链表来实现，每次访问完数据(不管是获取还是写入)，调整双向链表的顺序，把刚刚访问的数据调整到链表的最前方，
 * 以后再访问的时候速度将最快。
 * 为了方便，提供一个头和一个尾节点，不存具体的数，链表的基本形式如下面的这个简述
 * Head <===> Node1 <===> Node2 <===> Node3 <===> Near
 */

#include <iostream>

// 存数据的节点
typedef struct _Node_ {
	int key;				// 键
	int value;				// 数据

	struct _Node_ *next;	// 下一个节点
	struct _Node_ *pre;		// 上一个节点
} CacheNode;

class LRUCache
{
public:
	LRUCache(int cacheSize =10);
	~LRUCache();

	// 获取值
	int GetValue(int key);
	// 写入活更新值
	bool PutValue(int key, int value);
	// 显示所有节点
	void DisplayNodes();

private:
	// 分离节点
	void DetachNode(CacheNode *node);
	// 将节点插入到第一个
	void AddToFront(CacheNode *node);

private:
	// cache 长度
	int m_cacheSize;
	// 目前使用的长度
	int m_cacheRealSize;
	// 头节点指针
	CacheNode *m_pCacheListHead;
	// 尾节点指针
	CacheNode *m_pCacheListNear;
};

//////////////////////////////////////////////////////////////////////////

LRUCache::LRUCache(int cacheSize /*=10*/)
{
	m_cacheSize = cacheSize;
	m_cacheRealSize = 0;

	m_pCacheListHead = new CacheNode();
	m_pCacheListNear = new CacheNode();

	m_pCacheListHead->next = m_pCacheListNear;
	m_pCacheListHead->pre = nullptr;

	m_pCacheListNear->pre = m_pCacheListHead;
	m_pCacheListNear->next = nullptr;
}

LRUCache::~LRUCache()
{
	CacheNode *p;
	p = m_pCacheListHead->next;
	while (p != nullptr)
	{
		delete p->pre;
		p = p->next;
	}

	delete m_pCacheListNear;
}

int LRUCache::GetValue(int key)
{
	CacheNode *p = m_pCacheListHead->next;
	while (p->next != nullptr)
	{
		if (p->key == key)	// catch node
		{
			DetachNode(p);
			AddToFront(p);
			return p->value;
		}

		p = p->next;
	}

	return -1;
}

bool LRUCache::PutValue(int key, int value)
{
	CacheNode *p = m_pCacheListHead->next;
	while (p->next != nullptr)
	{
		if (p->key == key)	// catch node
		{
			p->value = value;
			GetValue(key);
			return true;
		}

		p = p->next;
	}

	if (m_cacheRealSize >= m_cacheSize)
	{
		std::cout << "free" << std::endl;
		p = m_pCacheListNear->pre->pre;
		delete p->next;
		p->next = m_pCacheListNear;
		m_pCacheListNear->pre = p;
	}

	p = new CacheNode();	// (CacheNode*)malloc(sizeof(CacheNode));
	if (p == nullptr)
		return false;

	AddToFront(p);
	p->key = key;
	p->value = value;

	m_cacheRealSize++;

	return true;
}

void LRUCache::DisplayNodes()
{
	CacheNode *p = m_pCacheListHead->next;

	while (p->next != nullptr)
	{
		std::cout << " Key: [" << p->key << "] Value: [" << p->value << "]" << std::endl;
		p = p->next;
	}

	std::cout << std::endl;
}

void LRUCache::DetachNode(CacheNode *node)
{
	node->pre->next = node->next;
	node->next->pre = node->pre;
}

void LRUCache::AddToFront(CacheNode *node)
{
	node->next = m_pCacheListHead->next;
	m_pCacheListHead->next->pre = node;
	m_pCacheListHead->next = node;
	node->pre = m_pCacheListHead;
}
