/*
 * 加权快速合并算法
 *
 * 用数组sz记录每个id[i] == i 的对象所在树中的节点树，使得合并操作能够把较小的树连接到较大的树上，防止树中长路径的增长.
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
		for (i = p; i != id[i]; i = id[i]);
		for (j = q; j != id[j]; j = id[j]);

		if(i==j)
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