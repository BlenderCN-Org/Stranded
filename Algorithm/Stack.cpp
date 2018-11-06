#include "Stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -1 ±Ì æø’’ª
CStaticStack::CStaticStack() : m_top(-1)
{

}

CStaticStack::~CStaticStack()
{

}

bool CStaticStack::IsEmpty()
{
	return(m_top < 0);
}

bool CStaticStack::IsFull()
{
	return(m_top >= MAX_STACK_SIZE - 1);
}

/* add an item to the stack */
void CStaticStack::push(StackElement item)
{
	if (m_top >= MAX_STACK_SIZE - 1)
		StackFull();

	m_stack[++m_top] = item;
}

/* delete and return the top element from the stack */
StackElement* CStaticStack::pop()
{
	if (m_top == -1)
		return StackEmpty();

	return &(m_stack[m_top--]);
}

void CStaticStack::StackFull()
{
	fprintf(stderr, "Stack is full, cannot add element!\n");
	exit(EXIT_FAILURE);
}

StackElement* CStaticStack::StackEmpty()
{
	fprintf(stderr, "Stack is empty, cannot pop element!\n");

	return nullptr;
}


//////////////////////////////////////////////////////////////////////////

CDynamicStack::CDynamicStack() :
	m_capacity(1),
	m_top(-1)
{
	m_pStack = new StackElement[1];
}

CDynamicStack::~CDynamicStack()
{
	if (m_pStack)
	{
		delete[] m_pStack;
		m_pStack = nullptr;
	}
}

bool CDynamicStack::IsEmpty()
{
	return (m_top < 0);
}

bool CDynamicStack::IsFull()
{
	return (m_top >= m_capacity - 1);
}

void CDynamicStack::push(StackElement item)
{
	if (m_top >= m_capacity - 1)
		StackFull();

	m_pStack[++m_top] = item;
}

StackElement* CDynamicStack::pop()
{
	if (m_top == -1)
		return StackEmpty();

	return &(m_pStack[m_top--]);
}

void CDynamicStack::StackFull()
{
	StackElement* pTmp = new StackElement[2 * m_capacity];
	memcpy(pTmp, m_pStack, sizeof(StackElement)*m_capacity);
	delete[] m_pStack;
	m_pStack = pTmp;

	m_capacity *= 2;
}

StackElement* CDynamicStack::StackEmpty()
{
	fprintf(stderr, "Stack is empty, cannot pop element!\n");

	return nullptr;
}
