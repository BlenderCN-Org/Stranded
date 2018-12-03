#include "multistack.h"

/* global memory declaration */
element g_memory[MEMORY_SIZE];
int g_top[MAX_STACKS];
int g_boundary[MAX_STACKS];
int g_n = 0;	/* number of stacks entered by the user */

static void Init()
{
	g_top[0] = g_boundary[0] = -1;
	/* 把数组分成长度基本相等的存储区 */
	/* 第i号栈的存储区域是boundary[i]+1到boundary[i+1]一段 */
	/* 最后一个栈的栈底设在boundayr[n]=MEMORY_SIZE-1 */
	for (int j = 1; j < g_n; j++)
		g_top[j] = g_boundary[j] = (MEMORY_SIZE / g_n)*j;
	g_boundary[g_n] = MEMORY_SIZE - 1;
}

/* add an item to te ith stack */
static void push(int i, element item)
{
	if (g_top[i] == g_boundary[i + 1])
		stackFull(i);

	g_memory[++g_top[i]] = item;
}

static void stackFull(int i)
{
}

/* remove top element from the ith stack */
static element pop(int i)
{
	if (g_top[i] == g_boundary[i])
		return stackEmpty(i);

	return g_memory[g_top[i]--];
}

static element stackEmpty(int i)
{
	element emp = { 0 };
	return emp;
}
