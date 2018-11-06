/*
 * 求解连通问题的 快速-查找算法(quick-find algorithm)
 *
 * 算法的基础是一个整形数组，当且仅当第p个元素和第q个元素相等时，p和q是连通的。初始时，数组中的第i个元素的值为i，0≤i≤N。
 * 为实现p与q的合并操作，我们遍历数组，把所有名为p的元素值改为q。我们也可以选择另一种方式，把所有名为q的元素改为p。
 *
 * 程序从标准输入读取小于N的非负整数对序列(对p-q表示“把对象p连接到q”)，并且输出还未连通的输入对。
 * 程序中使用数组id，每个元素表示一个对象，且具有一下性质，而且仅当p和q是连通的，id[p]和id[q]相等。
 * 为简化起见，定义N为编译时的常数。另一方面，也可以从输入得到它，并动态地为它分配id数组。
 *
 * 求解N个对象的连通性问题，如果执行M次合并操作，快速查找算法至少执行MN条指令。对于每个合并操作，for循环迭代N次。每次迭代
 * 至少需要执行一次指令(如果只检测循环是否结束).
 */

#include <stdio.h>

#define N 10000

int main(int argc, char* argv[])
{
	int i, p, q, t, id[N];

	for (i = 0; i < N; i++) id[i] = i;

#pragma warning(push)
#pragma warning(disable:4996)
	while (scanf("%d %d", &p, &q) == 2)
	{
		// 实现查找，只需测试指示数组中的元素是否相等(快速查找)
		if(id[p] == id[q]) continue;

		// 合并操作对于每对输入需要扫描整个数组
		for (t = id[p], i = 0; i < N; i++)
			if (id[i] == t)
				id[i] = id[q];

		printf(" %d %d\n", p, q);
	}
#pragma warning(pop)

	return 0;
}