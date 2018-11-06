#include <stdio.h>
#include "commonInclude.h"

/*
 * 置换
 *
 * 给定 n>=1 个元素的集合，打印这个集合所有可能的置换。例如，给定集合{a, b, c}，它的所有置换是
 * {(a, b, c), (a, c, b), (b, a, c), (b, c, a), (c, a, b), (c, b, a)}。给定n个元素，共有n!种置换。
 * 通过观察集合{a, b, c, d}，得到生成所有置换的简单算法：
 * (1) a 跟在(b, c, d)的所有置换之后.
 * (2) b 跟在(a, c, d)的所有置换之后.
 * (3) c 跟在(a, b, d)的所有置换之后.
 * (4) d 跟在(a, b, c)的所有置换之后.
 *
 * 当i=n时，耗时O(n). 当i<n时，进入else语句, 里面的for语句要进入n-1+1次, 每次循环耗时O(n+T(i+1, n)).
 * 所以，当i<n时，T(i, n)=O((n-i+1)(n+T(i+1, n))). 因为当i+1<=n时，T(i+1, n)至少是n，所以对i<n, 有
 * T(i, n) = O((n-i+1)T(i+1, n)). 求解，T(i, n) = O(n*n!).
 */

// 初始　perm(list, 0, n-1)
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
			// 保持 list 不变
			COMM_SWAP(list[i], list[j], temp);
		}
	}
}