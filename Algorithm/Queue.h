#ifndef __STATIC_QUEUE_H__
#define __STATIC_QUEUE_H__

/* maximum queue size */
#define MAX_QUEUE_SIZE 100

typedef struct {
	int key;
	/* other fields */
} QueueElement;

class CStaticQueue
{
public:
	CStaticQueue();
	virtual ~CStaticQueue();

	bool IsEmpty();
	bool IsFullQ();
	void AddQ(QueueElement item);
	QueueElement* DeleteQ();

private:
	void QueueFull();
	QueueElement* QueueEmpty();

private:
	// 插入端(队尾)
	int m_rear;
	// 删除端(队头)
	int m_front;

	QueueElement m_queue[MAX_QUEUE_SIZE];
};

//////////////////////////////////////////////////////////////////////////

// 循环静态队列
class CCyclicQueue
{
public:
	CCyclicQueue();
	virtual ~CCyclicQueue();

	bool IsEmpty();
	bool IsFullQ();
	void AddQ(QueueElement item);
	QueueElement* DeleteQ();

private:
	void QueueFull();
	QueueElement* QueueEmpty();

private:
	// 插入端(队尾)
	int m_rear;
	// 删除端(队头)
	int m_front;

	QueueElement m_queue[MAX_QUEUE_SIZE];
};

//////////////////////////////////////////////////////////////////////////
/*
* 动态循环队列
*
* 数组容量加倍:
	(1) 申请新数组 newQueue，容量是 capacity 的两倍.
	(2) 把原数组中第二段数据(从　queue[front+1] 到　queue[capacity-1])　复制到　newQueue 的起始位置(0)之后.
	(3) 把原数组中的第一段数据(从 queue[0] 到　queue[rear]) 复制到 newQueue 的位置之后(即capacity-front-1)之后.
*/

class CDynamicCyclicQueue
{
public:
	CDynamicCyclicQueue();
	virtual ~CDynamicCyclicQueue();

	bool IsEmpty();
	bool IsFullQ();
	void AddQ(QueueElement item);
	QueueElement* DeleteQ();

private:
	void QueueFull();
	QueueElement* QueueEmpty();
	void Copy(QueueElement* a, QueueElement* b, QueueElement* c);

private:
	// 插入端(队尾)
	int m_rear;
	// 删除端(队头)
	int m_front;

	// 队列的容量
	int m_capacity;

	QueueElement* m_pQueue;
};

#endif
