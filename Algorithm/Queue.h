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
	// �����(��β)
	int m_rear;
	// ɾ����(��ͷ)
	int m_front;

	QueueElement m_queue[MAX_QUEUE_SIZE];
};

//////////////////////////////////////////////////////////////////////////

// ѭ����̬����
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
	// �����(��β)
	int m_rear;
	// ɾ����(��ͷ)
	int m_front;

	QueueElement m_queue[MAX_QUEUE_SIZE];
};

//////////////////////////////////////////////////////////////////////////
/*
* ��̬ѭ������
*
* ���������ӱ�:
	(1) ���������� newQueue�������� capacity ������.
	(2) ��ԭ�����еڶ�������(�ӡ�queue[front+1] ����queue[capacity-1])�����Ƶ���newQueue ����ʼλ��(0)֮��.
	(3) ��ԭ�����еĵ�һ������(�� queue[0] ����queue[rear]) ���Ƶ� newQueue ��λ��֮��(��capacity-front-1)֮��.
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
	// �����(��β)
	int m_rear;
	// ɾ����(��ͷ)
	int m_front;

	// ���е�����
	int m_capacity;

	QueueElement* m_pQueue;
};

#endif
