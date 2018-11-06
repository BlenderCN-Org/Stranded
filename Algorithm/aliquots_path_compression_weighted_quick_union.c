/*
* ���еȷ�·��ѹ����Ȩ���ٺϲ��㷨
*
* �ںϲ����������У���Ӿ���ÿ��·������һ��ָ�룬ʹ��·������ÿ�������Ӧ��idԪ��ָ�����ĸ��ڵ�
* �ȷ�·��ѹ��: ͨ��ʹÿ��������Ծ���������ϵ�·������һ���ڵ�ʵ��ѹ��
* ����������еĺϲ��������������Ǳ�ƽ�ġ�
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