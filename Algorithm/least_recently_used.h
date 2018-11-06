/*
 * LRU Cache - Cache �û��㷨. "�������ʹ��"
 *
 * Ҫ��:
 * 1��ֻ�ṩ�����ӿڣ�һ����ȡ���� getValue(key), һ��д������ putValue(key, value)
 * 2�������ǻ�ȡ����д�����ݣ���ǰ�������Ҫ�����������׷��ʵ�λ��
 * 3�������������ޣ��ʱ��û�����ʵ�����Ӧ���û�������
 *
 * �㷨��
 * ʹ��˫��������ʵ�֣�ÿ�η���������(�����ǻ�ȡ����д��)������˫�������˳�򣬰Ѹոշ��ʵ����ݵ������������ǰ����
 * �Ժ��ٷ��ʵ�ʱ���ٶȽ���졣
 * Ϊ�˷��㣬�ṩһ��ͷ��һ��β�ڵ㣬����������������Ļ�����ʽ��������������
 * Head <===> Node1 <===> Node2 <===> Node3 <===> Near
 */

#include <iostream>

// �����ݵĽڵ�
typedef struct _Node_ {
	int key;				// ��
	int value;				// ����

	struct _Node_ *next;	// ��һ���ڵ�
	struct _Node_ *pre;		// ��һ���ڵ�
} CacheNode;

class LRUCache
{
public:
	LRUCache(int cacheSize =10);
	~LRUCache();

	// ��ȡֵ
	int GetValue(int key);
	// д������ֵ
	bool PutValue(int key, int value);
	// ��ʾ���нڵ�
	void DisplayNodes();

private:
	// ����ڵ�
	void DetachNode(CacheNode *node);
	// ���ڵ���뵽��һ��
	void AddToFront(CacheNode *node);

private:
	// cache ����
	int m_cacheSize;
	// Ŀǰʹ�õĳ���
	int m_cacheRealSize;
	// ͷ�ڵ�ָ��
	CacheNode *m_pCacheListHead;
	// β�ڵ�ָ��
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
