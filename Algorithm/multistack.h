#ifndef __MULTI_STACK_H__
#define __MULTI_STACK_H__

/*
* ����ջ
*
* һ�������д洢n��ջ. ��i���n��ջ�ĵ�i��ջ��
* ��boundary[i], 0 <= i < MAX_STACKS ָ���i��ջ��ջ�ף�����i��ջ���������һλ�Ĵ洢�ռ䣻
* ��top[i], 0 <= i < MAX_STACKS ָ���i��ջ��ջ��������i��ջ���ұߵĴ洢�ռ䡣
* ��i��ջ���Ϊ��ջ���ҽ��� boundary[i]=top[i].
*/

#define MEMORY_SIZE 100	/* size of memory */
#define MAX_STACKS 10	/* max number of stacks plus 1 */

typedef struct{
	int val;
} element;

extern element g_memory[MEMORY_SIZE];
extern int g_top[MAX_STACKS];
extern int g_boundary[MAX_STACKS];
extern int g_n;	/* number of stacks entered by the user */

extern void Init();
extern void push(int i, element item);
extern element pop(int i);
extern void stackFull(int i);
extern element stackEmpty(int i);

#endif
