#ifndef __MULTI_STACK_H__
#define __MULTI_STACK_H__

/*
* 多重栈
*
* 一个数组中存储n重栈. 用i标记n重栈的第i号栈。
* 用boundary[i], 0 <= i < MAX_STACKS 指向第i号栈的栈底，即第i号栈最左边向左一位的存储空间；
* 用top[i], 0 <= i < MAX_STACKS 指向第i号栈的栈顶，即第i号栈最右边的存储空间。
* 第i号栈如果为空栈当且仅当 boundary[i]=top[i].
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
