#include "SparseMatrix.h"
#include <stdio.h>
#include <stdlib.h>
#include "commonInclude.h"

MatrixTerm g_matrixTerm[MAX_MATRIX_TERMS];

/*
 * @brief: ʱ�临�Ӷ� O(cols*element)
 * element = rows*cols
 * O(cols(2) * rows);
 *
 * ��� rows * cols �����ö�ά�����ʾ��ת���㷨
	for(j=0; j<columns; j++)
		for(i=0; i<rows; i++)
			b[j][i] = a[i][j];
 * ʱ�临�Ӷ� O(rows*cols)
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
* ת�þ����i�е���ʼλ����rowTerms[i-1]+startingPos[i-1],
* rowTerms[i-1] �ǵ�i-1�еķ���Ԫ�ظ�����startingPos[i-1]�ǵ�i-1�е���ʼλ��.
* ǰ����for������rowTerms��ֵ��������for������startingPos��ֵ�����һ��for����
* ��Ԫ������ת�þ���.
* ִ�д��� numCols��numTerms��numCols-1��numTerms
* �㷨����ʱ�� O(cols+element)
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
 * ������Ԫ���ʾ��ϡ�������ˣ�����洢��D�У�����������������У�����ֱ�ӷ��ں��ʵ�λ�ã����ƶ���������Ľ��.
 * ��ѡA�еĵ�i�У�Ȼ��ɨ������B���ҳ���������j=0,1,...,colsB-1��. ��ת��B�����Ȱ������а���洢����. �����i�����
 * j�е��������㣬���ƶ���ʽ�ӷ��Ĺ鲢.
 * 
 *
 * ��׼����Ϊ�����ʾ�ľ�������㷨
 *
	for(i=0; i<rowsA; i++)
		for(j=0; j<colsB; j++)
		{
			sum = 0;
			for(k=0; k<colsA; k++)
				sum += a[i][k] * b[k][j];
			d[i][j] = sum;
		}
 * ʱ�临�Ӷ� O(rowsA*colsA*colsB)
 */
void Mmult(MatrixTerm a[], MatrixTerm b[], MatrixTerm d[])
{
	// multiply two sparse matrices

	// col ��B������A��һ����˵��к�
	// totalD�ǵ�ǰ�������D�е�Ԫ�ظ���
	// i,j ��A��B���С��б���
	int i, j, col, totalB = b[0].value, totalD = 0;
	int rowsA = a[0].row, colsA = a[0].col;
	int colsB = b[0].col, totalA = a[0].value;
	// row��A�ĵ�ǰ�У���B�����������
	// rowBegin�ǵ�ǰ�еĵ�һ��Ԫ����a�е�λ��
	int rowBegin = 1, row = a[1].row, sum = 0;
	// b��ת��
	MatrixTerm newB[MAX_MATRIX_TERMS];

	if (colsA != b[0].row)
	{
		fprintf(stderr, "Incompatible matrices\n");
		exit(EXIT_FAILURE);
	}

	fastTranspose(b, newB);

	// set boundary condition
	// A��Ԥ����һ����Ԫ - �ڱ�
	a[totalA + 1].row = rowsA;
	// B��Ԥ����һ����Ԫ - �ڱ�
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
 * ����Ԫ��洢������d�С�
 * ����ǰ�����sum����.
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