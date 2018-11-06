/*
 * ħ��
 *
 * ħ����nxn�ľ���ÿ����Ԫȡ����ֵ����Χ��1��nƽ��, Ҫ��ÿ�С�ÿ�У��Լ��������Խ��ߵĺͶ���ȡ�
 * ���磬n=5��ħ������ȵĺ�����65.
 *
 15	8	1	24	17
 16	14	7	5	23
 22	20	13	6	4
 3	21	19	12	10
 9	2	25	18	11
 *
 * ����������(n������)ħ���ķ�����
 * ��ʼʱ����ħ����һ�е��м�һ���1.Ȼ���ظ�һ�²��裺
 * �ƶ�������һ�񣬰ѵ�ǰ������1�������λ�á�����ƶ�ʱ����ħ����Χ���������뵱ǰ״̬��ȫ��ͬ����һ��ħ����
 * �����룩�����ڳ����������߽����ϣ�������Լ���������ƶ����ĸ����Ѿ����ù����֣����������ӵ�λ�������·�
 * �ƶ�һ��ֱ�������и��Ӷ���������Ϊֹ��
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