#ifndef __SPARSE_MATRIX_H__
#define __SPARSE_MATRIX_H__

// 稀疏矩阵

#define MAX_MATRIX_TERMS	101

typedef struct _MatrixTerm {
	int col;
	int row;
	int value;
} MatrixTerm;

// g_matrixTerm[0].row - 矩阵行数
// g_matrixTerm[0].col - 矩阵列数
// g_matrixTerm[0].value - 矩阵中非零元个数
//
// 其余矩阵非零元三元组，行下标row, 列下标col，元素值下标value。三元组按行序升序，同行元素按列序升序排列.

extern MatrixTerm g_matrixTerm[MAX_MATRIX_TERMS];

// 矩阵转置
// 行、列互换. 原矩阵中的 a[i][j] 转置后在结果矩阵中位于 b[j][i].
// 避免直接转置插入：
// 先把第0列的所有三元组顺序存放到结果矩阵的第0行，再把第1列的所有三元组顺序存放到结果矩阵的第1行，等等。
// 结果保持同样排列顺序.
void transpose(MatrixTerm a[], MatrixTerm b[]);

/*
 * @brief:
 * 先确定原矩阵中每列非零元素的个数，它正是转置矩阵中每行非零元素的个数.
 * 如此，转置矩阵中每行的起始位置就确定下来，因而，原矩阵中的每项数据可以对应到转置矩阵的已知位置.
 * 约定：原矩阵的最大列数不超过 MAX_COl.
 */
void fastTranspose(MatrixTerm a[], MatrixTerm b[]);

void Mmult(MatrixTerm a[], MatrixTerm b[], MatrixTerm d[]);
void StoreSum(MatrixTerm d[], int *totalD, int row, int col, int *sum);

#endif
