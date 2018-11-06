#include "SparseMatrix.h"
#include <stdio.h>
#include <stdlib.h>
#include "commonInclude.h"

MatrixTerm g_matrixTerm[MAX_MATRIX_TERMS];

/*
 * @brief: 时间复杂度 O(cols*element)
 * element = rows*cols
 * O(cols(2) * rows);
 *
 * 如果 rows * cols 矩阵用二维数组表示，转置算法
	for(j=0; j<columns; j++)
		for(i=0; i<rows; i++)
			b[j][i] = a[i][j];
 * 时间复杂度 O(rows*cols)
 *
 *
 * @param: b is set of the transpose of a
 */
void transpose(MatrixTerm a[], MatrixTerm b[])
{
	int n, i, j, curentb;

	// total number of elements
	n = a[0].col;

	// rows in b = cols in a
	b[0].row = a[0].col;
	// cols in b = rows in a
	b[0].col = a[0].row;
	b[0].value = n;

	// non zero matrix
	if (n > 0)
	{
		curentb = 1;
		// transpose by the cols in a
		for(i=0; i<a[0].col; i++)
			for(j=1; j<=n; j++)
				if (a[j].col == i)
				{
					// element is in current col, add it to b
					b[curentb].row = a[j].col;
					b[curentb].col = a[j].row;
					b[curentb].value = a[j].value;
					curentb++;
				}
	}
}

/*
* @brief:
* 转置矩阵第i行的起始位置是rowTerms[i-1]+startingPos[i-1],
* rowTerms[i-1] 是第i-1行的非零元素个数，startingPos[i-1]是第i-1行的起始位置.
* 前两条for语句计算rowTerms的值，第三条for语句计算startingPos的值，最后一条for语句把
* 三元组填入转置矩阵.
* 执行次数 numCols、numTerms、numCols-1、numTerms
* 算法计算时间 O(cols+element)
*/
void fastTranspose(MatrixTerm a[], MatrixTerm b[])
{
	static const int MAX_COL = 100;
	// the transpose of a is placed in b
	int rowTerms[MAX_COL], startingPos[MAX_COL];

	int i, j, numCols = a[0].col, numTerms = a[0].value;
	b[0].row = numCols;
	b[0].col = a[0].row;
	b[0].value = numTerms;

	// nonzero matrix
	if (numTerms > 0)
	{
		for (i = 0; i < numCols; i++)
			rowTerms[i] = 0;

		for (i = 1; i <= numTerms; i++)
			rowTerms[a[i].col]++;

		startingPos[0] = 1;
		for (i = 1; i <= numCols; i++)
			startingPos[i] = startingPos[i - 1] + rowTerms[i - 1];

		for (i = 1; i < numTerms; i++)
		{
			j = startingPos[a[i].col]++;
			b[j].row = a[i].col;
			b[j].col = a[i].row;
			b[j].value = a[i].value;
		}
	}
}

/*
 * 两个三元组表示的稀疏矩阵相乘，结果存储在D中，非零项按行序升序排列，新项直接放在合适的位置，不移动早先算出的结果.
 * 先选A中的第i行，然后扫描整个B，找出它的所有j=0,1,...,colsB-1列. 先转置B，事先把所有列按序存储起来. 处理第i行与第
 * j列的算术运算，类似多项式加法的归并.
 * 
 *
 * 标准用量为数组表示的矩阵相乘算法
 *
	for(i=0; i<rowsA; i++)
		for(j=0; j<colsB; j++)
		{
			sum = 0;
			for(k=0; k<colsA; k++)
				sum += a[i][k] * b[k][j];
			d[i][j] = sum;
		}
 * 时间复杂度 O(rowsA*colsA*colsB)
 */
void Mmult(MatrixTerm a[], MatrixTerm b[], MatrixTerm d[])
{
	// multiply two sparse matrices

	// col 是B中正与A的一行相乘的列号
	// totalD是当前结果矩阵D中的元素个数
	// i,j 是A、B的行、列变量
	int i, j, col, totalB = b[0].value, totalD = 0;
	int rowsA = a[0].row, colsA = a[0].col;
	int colsB = b[0].col, totalA = a[0].value;
	// row是A的当前行，与B的所有列相乘
	// rowBegin是当前行的第一个元素在a中的位置
	int rowBegin = 1, row = a[1].row, sum = 0;
	// b的转置
	MatrixTerm newB[MAX_MATRIX_TERMS];

	if (colsA != b[0].row)
	{
		fprintf(stderr, "Incompatible matrices\n");
		exit(EXIT_FAILURE);
	}

	fastTranspose(b, newB);

	// set boundary condition
	// A多预留了一个单元 - 哨兵
	a[totalA + 1].row = rowsA;
	// B多预留了一个单元 - 哨兵
	newB[totalB + 1].row = colsB;
	newB[totalB + 1].col = 0;

	for (i = 1; i <= totalA;)
	{
		col = newB[1].row;
		for (j = 1; j <= totalB;)
		{
			// multiply row of a by col of b
			if (a[i].row != row)
			{
				StoreSum(d, &totalD, row, col, &sum);
				i = rowBegin;

				for (; newB[j].row == col; j++)
				{
					;
				}

				col = newB[j].row;
			}
			else if (newB[j].row != col)
			{
				StoreSum(d, &totalD, row, col, &sum);
			}
			else
				switch (COMM_COMPARE(a[i].col, newB[j].col))
				{
				case -1:	// go to next term in a
					i++;
					break;
				case 0:	// add terms, go to next term in a and b
					sum += (a[i++].value * newB[j++].value);
					break;
				case 1:	// advance to next term in b
					j++;
				}	// end of for j<=totalB+1

			for (; a[i].row == row; i++)
			{
				;
			}

			rowBegin = i;
			row = a[i].row;
		}	// end of for i<=totalA

		d[0].row = rowsA;
		d[0].col = colsB;
		d[0].value = totalD;
	}
}

/*
 * 把三元组存储在数组d中。
 * 返回前负责把sum清零.
 */
void StoreSum(MatrixTerm d[], int *totalD, int row, int col, int *sum)
{
	// if *sum!=0, then it along with its row and col.position is stored as the *totalD+1 entry in d
	if(*sum)
		if (*totalD < MAX_MATRIX_TERMS)
		{
			d[++*totalD].row = row;
			d[*totalD].col = col;
			d[*totalD].value = *sum;
			*sum = 0;
		}
		else
		{
			fprintf(stderr, "Numbers of terms in productexceeds [%d]\n", MAX_MATRIX_TERMS);
			exit(EXIT_FAILURE);
		}
}