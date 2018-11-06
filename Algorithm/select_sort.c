#include <stdio.h>
#include <stdlib.h>
#include "SelectSort.h"


// ½»»»Á½¸öint
void swap(int *x, int *y)
{
	// both parameters are pointers to ints
	int temp = *x;	// declare temp as an int and assignes to it the contents of what x points to
	*x = *y;		// stores what y points to into the location where x points
	*y = temp;		// places the contents of temp in location pointed to by y
}

void main()
{
	int i, n, *list;

	printf("Enter the number of numbers to generate:\n");
#pragma warning(push)
#pragma warning(disable:4996)
	scanf("%d", &n);
#pragma warning(pop)
	
	if (n<1)
	{
		fprintf(stderr, "Improper value of n!\n");
		exit(EXIT_FAILURE);
	}

	list = (int*)malloc(n * sizeof(int));

	for (i = 0; i < n; i++)	// randomly generate numbers
	{
		list[i] = rand() % n;
		printf("%d ", list[i]);
	}

	sort(list, n);
	printf("\nSorted array:\n");
	for (i = 0; i < n; i++)	// print out sorted numbers
		printf("%d ", list[i]);
	printf("\n");

	free((void*)list);
}