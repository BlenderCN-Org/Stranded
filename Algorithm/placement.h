#include <stdio.h>
#include "commonInclude.h"

/*
 * �û�
 *
 * ���� n>=1 ��Ԫ�صļ��ϣ���ӡ����������п��ܵ��û������磬��������{a, b, c}�����������û���
 * {(a, b, c), (a, c, b), (b, a, c), (b, c, a), (c, a, b), (c, b, a)}������n��Ԫ�أ�����n!���û���
 * ͨ���۲켯��{a, b, c, d}���õ����������û��ļ��㷨��
 * (1) a ����(b, c, d)�������û�֮��.
 * (2) b ����(a, c, d)�������û�֮��.
 * (3) c ����(a, b, d)�������û�֮��.
 * (4) d ����(a, b, c)�������û�֮��.
 *
 * ��i=nʱ����ʱO(n). ��i<nʱ������else���, �����for���Ҫ����n-1+1��, ÿ��ѭ����ʱO(n+T(i+1, n)).
 * ���ԣ���i<nʱ��T(i, n)=O((n-i+1)(n+T(i+1, n))). ��Ϊ��i+1<=nʱ��T(i+1, n)������n�����Զ�i<n, ��
 * T(i, n) = O((n-i+1)T(i+1, n)). ��⣬T(i, n) = O(n*n!).
 */

// ��ʼ��perm(list, 0, n-1)
void perm(char *list, int i, int n)
{
	// generate all the permutations of list[i] to list[n].
	int j, temp;
	if (i == n)
	{
		for (j = 0; j <= n; j++)
			printf("%c", list[j]);
		printf("    ");
	}
	else
	{
		// list[i] to list[n] has more than one permutation, generate these recursively
		for (j = i; j <= n; j++)
		{
			COMM_SWAP(list[i], list[j], temp);
			perm(list, i + 1, n);
			// ���� list ����
			COMM_SWAP(list[i], list[j], temp);
		}
	}
}