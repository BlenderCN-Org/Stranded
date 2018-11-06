#include <stdio.h>
#include <time.h>
#include "SelectSort.h"

// 选择排序最差情况时间统计

#define MAX_SIZE 1001

void main()
{
	int i, n, step = 10;
	int a[MAX_SIZE];
	double duration;
	clock_t start;
	long repetitions;

	// time for n=0, 10, ..., 100, 200, ..., 1000
	printf("    n   time\n");
	for (n = 0; n <= 1000; n += step)
	{// get time for size n
		repetitions = 0;
		
		// 返回CPU内部绝对时钟周期
		start = clock();

		do
		{
			repetitions++;

			// initialize with worst-case data
			for (i = 0; i < n; i++)
				a[i] = n - i;

			sort(a, n);
		} while (clock() - start < CLOCKS_PER_SEC);	// repeat until enough time has elapsed

		// 每秒时钟周期数
		duration = ((double)(clock() - start)) / CLOCKS_PER_SEC;
		duration /= repetitions;

		printf("%6d   %9d   %f\n", n, repetitions, duration);

		if (n == 100)
			step = 100;
	}
}