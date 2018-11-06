/*
 * 魔方
 *
 * 魔方是nxn的矩阵，每个单元取整数值，范围从1到n平方, 要求每行、每列，以及两条主对角线的和都相等。
 * 例如，n=5的魔方，相等的和数是65.
 *
 15	8	1	24	17
 16	14	7	5	23
 22	20	13	6	4
 3	21	19	12	10
 9	2	25	18	11
 *
 * 生成奇数阶(n是奇数)魔方的方法：
 * 开始时，在魔方第一行的中间一格放1.然后重复一下步骤：
 * 移动到左上一格，把当前的数加1放在这个位置。如果移动时超出魔方范围，则想象与当前状态完全相同的另一个魔方，
 * （对齐）紧靠在超出的那条边界线上，因而可以继续。如果移动到的格子已经放置过数字，则从这个格子的位置向正下方
 * 移动一格。直到把所有格子都放满数字为止。
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE	15		// maximum size of square

// construct a magic square, iteratively
void main()
{
	int square[MAX_SIZE][MAX_SIZE];
	int i, j, row, col;		// indexes
	int count;				// counter
	int size;				// square size;

	printf("Enter the size of the square:\n");
#pragma warning(push)
#pragma warning(disable:4996)
	scanf("%d", &size);
#pragma warning(pop)

	// check for input errors
	if (size<1 || size>MAX_SIZE)
	{
		fprintf(stderr, "Error! Size is out of range.\n");
		exit(EXIT_FAILURE);
	}

	if (!(size % 2))
	{
		fprintf(stderr, "Error! Size is even.\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < size; i++)
		for (j = 0; j < size; j++)
			square[i][j] = 0;

	// middle of first row
	square[0][(size - 1) / 2] = 1;
	// i and j are current position
	i = 0;
	j = (size - 1) / 2;
	for (count = 2; count <= size*size; count++)
	{
		// up
		row = (i - 1 < 0) ? (size - 1) : (i - 1);
		// left
		col = (j - 1 < 0) ? (size - 1) : (j - 1);
		// down
		if (square[row][col])
			i = (++i) % size;
		else
		{
			// square is unoccupied
			i = row;
			j = (j - 1 < 0) ? (size - 1) : --j;
		}

		square[i][j] = count;
	}

	// output the magic square
	printf("Magic Square of size [%d]: \n\n", size);
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
			printf("%5d", square[i][j]);
		printf("\n");
	}

	printf("\n\n");
}