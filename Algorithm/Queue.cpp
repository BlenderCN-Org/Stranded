#include "Queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
//#include <iterator>

CStaticQueue::CStaticQueue() :
	m_rear(-1),
	m_front(-1)
{

}

CStaticQueue::~CStaticQueue()
{

}

bool CStaticQueue::IsEmpty()
{
	return (m_front == m_rear);
}

bool CStaticQueue::IsFullQ()
{
	return (m_rear == MAX_QUEUE_SIZE - 1);
}

/* add an item to the queue */
void CStaticQueue::AddQ(QueueElement item)
{
	if (m_rear == MAX_QUEUE_SIZE - 1)
		QueueFull();

	m_queue[++m_rear] = item;
}

/* remove element at the front of the queue */
QueueElement* CStaticQueue::DeleteQ()
{
	if (m_front == m_rear)
		return QueueEmpty();

	return &(m_queue[++m_front]);
}

void CStaticQueue::QueueFull()
{
	fprintf(stderr, "Queue is full, cannot put element!\n");
	exit(EXIT_FAILURE);
}

QueueElement* CStaticQueue::QueueEmpty()
{
	fprintf(stderr, "Queue is empty, cannot pop element!\n");

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////

CCyclicQueue::CCyclicQueue() :
	m_rear(-1),
	m_front(-1)
{

}

CCyclicQueue::~CCyclicQueue()
{

}

bool CCyclicQueue::IsEmpty()
{
	return (m_front == m_rear);
}

bool CCyclicQueue::IsFullQ()
{
	return (m_front == (m_rear + 1) % MAX_QUEUE_SIZE);
}

/* add an item to the queue */
void CCyclicQueue::AddQ(QueueElement item)
{
	m_rear = (m_rear + 1) % MAX_QUEUE_SIZE;

	/* ��ͷλ�� m_queue[m_front]ǰһ��λ��,������ʵ�ʻ�ʣһ����λ�ã������ܲ���Ԫ�أ��������m_front=m_rear�����жϿյ������ͻ */
	if (m_front == m_rear)
		QueueFull();

	m_queue[m_rear] = item;
}

/* remove element at the front of the queue */
QueueElement* CCyclicQueue::DeleteQ()
{
	if (m_front == m_rear)
		return QueueEmpty();

	m_front = (m_front + 1) % MAX_QUEUE_SIZE;
	return &(m_queue[m_front]);
}

/* print error and exit */
void CCyclicQueue::QueueFull()
{
	fprintf(stderr, "Queue is full, cannot put element!\n");
	exit(EXIT_FAILURE);
}

/* return an error key */
QueueElement* CCyclicQueue::QueueEmpty()
{
	fprintf(stderr, "Queue is empty, cannot pop element!\n");

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////

CDynamicCyclicQueue::CDynamicCyclicQueue() :
	m_rear(-1),
	m_front(-1),
	m_capacity(1)
{
	m_pQueue = new QueueElement[1];
}

CDynamicCyclicQueue::~CDynamicCyclicQueue()
{
	if (m_pQueue)
	{
		delete[] m_pQueue;
		m_pQueue = nullptr;
	}
}

bool CDynamicCyclicQueue::IsEmpty()
{
	return (m_front == m_rear);
}

bool CDynamicCyclicQueue::IsFullQ()
{
	return (m_front == ((m_rear + 1) % m_capacity));
}

/* add an item to the queue */
void CDynamicCyclicQueue::AddQ(QueueElement item)
{
	m_rear = (m_rear + 1) % m_capacity;

	if (m_front == m_rear)
		QueueFull();

	m_pQueue[m_rear] = item;
}

QueueElement* CDynamicCyclicQueue::DeleteQ()
{
	if (m_front == m_rear)
		QueueEmpty();

	m_front = (m_front + 1) % m_capacity;
	return &(m_pQueue[m_front]);
}

/* double capacity */
/* allocate an array with twice the capacity */
void CDynamicCyclicQueue::QueueFull()
{
	QueueElement* newQueue;

	newQueue = new QueueElement[2 * m_capacity];

	/* copy from queue to newQueue */
	int start = (m_front + 1) % m_capacity;
	if (start < 2)
		/* no wrap around */
		// Visual C++ throws a C4996 error if std::copy is called with array parameters (instead of the conventional iterator parameters)
		// Visual C++ uses checked iterators by default everywhere for security reasons
		// This error is a manifestation of that design decision
		// There are a few ways to deal with it: checked_array_iterator, unchecked_copy, std::vector, pragma warning, _SCL_SECURE_NO_WARNINGS
		//std::copy(m_pQueue + start, m_pQueue + start + m_capacity - 1, stdext::checked_array_iterator<QueueElement*>(newQueue, m_capacity - 1));
		Copy(m_pQueue + start, m_pQueue + start + m_capacity - 1, newQueue);
	else
	{
		/* queue wraps around */
		//std::copy(m_pQueue + start, m_pQueue + m_capacity, stdext::checked_array_iterator<QueueElement*>(newQueue, m_capacity));
		Copy(m_pQueue + start, m_pQueue + start + m_capacity, newQueue);
		//std::copy(m_pQueue, m_pQueue + m_rear + 1, stdext::checked_array_iterator<QueueElement*>(newQueue + m_capacity - start, m_rear + 1));
		Copy(m_pQueue, m_pQueue + m_rear + 1, newQueue + m_capacity - start);
	}

	/* switch to newQueue */
	m_front = 2 * m_capacity - 1;
	m_rear = m_capacity - 2;
	m_capacity *= 2;
	delete[] m_pQueue;
	m_pQueue = newQueue;
}

QueueElement* CDynamicCyclicQueue::QueueEmpty()
{
	fprintf(stderr, "Queue is empty, cannot pop element!\n");

	return nullptr;
}

/* �Ѵ�λ��a��b-1�����ݸ��Ƶ�λ��c֮�� */
void CDynamicCyclicQueue::Copy(QueueElement* a, QueueElement* b, QueueElement* c)
{
	assert(c != NULL);
	assert(a != b);

	QueueElement* pA = a;	//���Ա���src�����ֵ���䣬��������������ı������ŵ�ֵ
	QueueElement* pB = b;

	int count = b - a;

	//����Ƿ����ص�����
	if (a < b)
	{
		// ���򿽱�
		while (count--)
			*c++ = *a++;
	}
	else if (b < a)
	{
		// ���򿽱�
		pA = pA + count - 1;
		pB = pB + count - 1;
		while (count--)
			*pB-- = *pA--;
	}
}
