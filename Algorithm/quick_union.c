/*
 * 快速合并的补算法. 基于数据结构 - 通过对象名引用数组元素
 * 在一个没有环的结构中，每个对象指向同一集合中的另一个对象。要确定两个对象是否在同一个集合中，
 * 只需跟随每个对象的指针，直到到达指向自身的另一个对象。当且仅当这个过程使两个对象到达同一个对象时，
 * 这两个对象在同一个集合中。如果两者不在同一个集合中，最终一定到达不同对象(每个对象都指向自身)。
 * 为了构造合并操作，只需将一个对象链接到另一个对象以执行合并操作。
 */

#include <stdio.h>

#define N 10000

int main(int argc, char* argv[])
{
	int i, j, p, q, id[N];

	for (i = 0; i < N; i++)
		id[i] = i;

#pragma warning(push)
#pragma warning(disable:4996)
	while (scanf("%d %d", &p, &q) == 2)
	{
		for (i = p; i != id[i]; i = id[i]);

		for (j = q; j != id[j]; j = id[j]);

		if(i == j)
			continue;

		// 实现合并操作
		id[i] = j;

		printf(" %d %d\n", p, q);
	}
#pragma warning(pop)

	return 0;
}