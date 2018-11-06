#ifndef __STACK_H__
#define __STACK_H__

#define MAX_STACK_SIZE 100	/* maximum stack size */

typedef struct {
	int key;
	/* other fields */
} StackElement;

class CStaticStack
{

public:
	CStaticStack();
	virtual ~CStaticStack();

public:
	bool IsEmpty();
	bool IsFull();

	// ��ջ
	void push(StackElement item);
	// ��ջ
	StackElement* pop();

private:
	void StackFull();
	StackElement* StackEmpty();

private:
	// m_stack[0] ��ջ��
	StackElement m_stack[MAX_STACK_SIZE];
	// ָ��ջ��Ԫ��
	int m_top;
};

//////////////////////////////////////////////////////////////////////////

class CDynamicStack
{
public:
	CDynamicStack();
	virtual ~CDynamicStack();

public:
	bool IsEmpty();
	bool IsFull();

	// ��ջ
	void push(StackElement item);
	// ��ջ
	StackElement* pop();

private:
	void StackFull();
	StackElement* StackEmpty();

private:
	StackElement *m_pStack;
	int m_capacity;
	int m_top;
};

#endif
