/*
 * ���ֲ��� | �۰����
 *
 * �ٶ� n>=1 �����������洢������ list ֮�У�list[0]<=list[1]<=...<=list[n-1]��������֪������ searchnum �Ƿ������������У�
 * ����ǣ��򷵻��±�i, searchnum = list[i]; ���򷵻�-1�����������������·�����
 *
 * ��left��right�ֱ��ʾ���д��鷶Χ�����Ҷ˵㣬��ֵΪ��left=0, right=n-1���� middle=(left+right)/2���Ǳ���е��±�ֵ��searchnum
 * �� list[middle] �ȽϵĽ���������ֿ���:
 * (1) searchnum<list[middle]. ��ʱ����� searchnum �ڱ��У���һ����λ��0 �� middle-1 ֮�䣬��ˣ���right ��� middle-1.
 * (2) searchnum=list[middle]. ��ʱ������middle.
 * (3) searchnum>list[middle]. ��ʱ����� searchnum �ڱ��У���һ����λ�� middle+1 �� n-1֮�䣬��˰� left ��� middle+1.
 * �� searchnum ��û���鵽��ͬʱ����û�����������������¼���middle, �ظ���������.
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
	// �±꽻�棬�޴�����Ԫ��
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

	// ����Ϊѡ������
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