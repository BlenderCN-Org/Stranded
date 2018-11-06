#include "commonInclude.h"

// selection sort
/*
* 当最外层 for 循环结束第 i=q 次循环时，我们有 list[q] <= list[r], q<r<n. 接着，执行后续循环到 i>q，此时从
* list[0]到list[q]的内容不变。因此，当最外层for执行到最后一个循环时(即i=n-2之后)，有list[0]<=list[1]<=...<=list[n-1].
*/

/*
* 假定当前最小值在list[i]中，把它和list[i+1], list[i+2],...,list[n-1]比较，只要找到更小值，就用它作最小值.
* 这样到list[n-1]，就找出最小整数.
*/
void sort(int list[], int n)
{
	int i, j, min, temp;
	for (i = 0; i < n - 1; i++)
	{
		min = i;
		for (j = i + 1; j < n; j++)
			if (list[j] < list[min])
				min = j;

		COMM_SWAP(list[i], list[min], temp);
	}
}