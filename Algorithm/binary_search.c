/*
 * 二分查找 | 折半查找
 *
 * 假定 n>=1 个有序整数存储在数组 list 之中，list[0]<=list[1]<=...<=list[n-1]。我们想知道整数 searchnum 是否出现在这个表中，
 * 如果是，则返回下标i, searchnum = list[i]; 否则返回-1。由于这个婊有序，如下方法。
 *
 * 令left、right分别表示表中待查范围的左、右端点，初值为：left=0, right=n-1。令 middle=(left+right)/2，是表的中点下标值。searchnum
 * 和 list[middle] 比较的结果，有三种可能:
 * (1) searchnum<list[middle]. 此时，如果 searchnum 在表中，它一定在位置0 与 middle-1 之间，因此，把right 设成 middle-1.
 * (2) searchnum=list[middle]. 此时，返回middle.
 * (3) searchnum>list[middle]. 此时，如果 searchnum 在表中，它一定在位置 middle+1 与 n-1之间，因此把 left 设成 middle+1.
 * 当 searchnum 还没被查到，同时尚有没检查的其他整数，重新计算middle, 重复上述过程.
 */

#include <stdio.h>
#include <stdlib.h>
#include "commonInclude.h"

#define MAX_SIZE 101

int compare(int x, int y)
{
	// compare x and y, return -1 for less than, 0 for equal, 1 for greater
	if (x < y)
		return -1;
	else if (x == y)
		return 0;
	else
		return 1;
}

// search list[0]<=list[1]<=...<=list[n-1] for searchnum. Return its position if found, Otherwise return -1.
int binsearch(int list[], int searchnum, int left, int right)
{
	int middle;
	// 下标交叉，无待查找元素
	while (left <= right)
	{
		middle = (left + right) / 2;
		switch (COMM_COMPARE(searchnum, list[middle]))
		{
		case -1:
			right = middle - 1;
			break;
		case 0:
			return middle;
		default:
			left = middle + 1;
			break;
		}
	}

	return -1;
}

// recursive
int binsearch(int list[], int searchnum, int left, int right)
{
	int middle;
	if (left <= right)
	{
		middle = (left + right) / 2;
		switch (COMM_COMPARE(list[middle], searchnum))
		{
		case -1:
			return binsearch(list, searchnum, middle + 1, right);
		case 0:
			return middle;
		case 1:
			return binsearch(list, searchnum, left, middle - 1);
		}
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////

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

void main()
{
	int i, n;
	int list[MAX_SIZE];

	printf("Enter the number of numbers to generate:\n");
#pragma warning(push)
#pragma warning(disable:4996)
	scanf("%d", &n);
#pragma warning(pop)

	if (n<1 || n > MAX_SIZE)
	{
		fprintf(stderr, "Improper value of n!\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < n; i++)	// randomly generate numbers
	{
		list[i] = rand() % 1000;
		printf("%d ", list[i]);
	}

	sort(list, n);
	printf("\nSorted array:\n");
	for (i = 0; i < n; i++)	// print out sorted numbers
		printf("%d ", list[i]);
	printf("\n");

	// 上面为选择排序
	//////////////////////////////////////////////////////////////////////////

	int searchnum;
	printf("Enter the number to search:\n");
#pragma warning(push)
#pragma warning(disable:4996)
	scanf("%d", &searchnum);
#pragma warning(pop)
	
	int index = binsearch(list, searchnum, 0, n - 1);
	if (index != -1)
		printf("searchnum: [%d] is in index [%d].\n", searchnum, index);
	else
		printf("searchnum is not in list.\n");

	printf("\n");
}