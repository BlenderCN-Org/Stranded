/*
* 带有等分路径压缩加权快速合并算法
*
* 在合并操作过程中，添加经过每条路径的另一个指针，使沿路遇见的每个顶点对应的id元素指向树的根节点
* 等分路径压缩: 通过使每条链接跳跃到树中向上的路径的下一个节点实现压缩
* 结果：长序列的合并操作后，树几乎是扁平的。
*/

#include <stdio.h>

#define N 1000

int main(int argc, char* argv[])
{
	int i, j, p, q, id[N], sz[N];

	for (i = 0; i < N; i++)
	{
		id[i] = i;
		sz[i] = 1;
	}

#pragma warning(push)
#pragma warning(disable: 4996)
	while (scanf("%d %d", &p, &q) == 2)
	{
		for (i = p; i != id[i]; i = id[i])
			id[i] = id[id[i]];

		for (j = q; j != id[j]; j = id[j])
			id[j] = id[id[j]];

		if (i == j)
			continue;

		if (sz[i] < sz[j])
		{
			id[i] = j;
			sz[j] += sz[i];
		}
		else
		{
			id[j] = i;
			sz[i] += sz[j];
		}

		printf("[%d %d]\n", p, q);
	}
#pragma warning(pop)

	return 0;
}