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

	// 入栈
	void push(StackElement item);
	// 出栈
	StackElement* pop();

private:
	void StackFull();
	StackElement* StackEmpty();

private:
	// m_stack[0] 是栈底
	StackElement m_stack[MAX_STACK_SIZE];
	// 指向栈顶元素
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

	// 入栈
	void push(StackElement item);
	// 出栈
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
