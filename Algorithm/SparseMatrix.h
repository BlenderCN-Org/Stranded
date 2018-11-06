#ifndef __SPARSE_MATRIX_H__
#define __SPARSE_MATRIX_H__

// ϡ�����

#define MAX_MATRIX_TERMS	101

typedef struct _MatrixTerm {
	int col;
	int row;
	int value;
} MatrixTerm;

// g_matrixTerm[0].row - ��������
// g_matrixTerm[0].col - ��������
// g_matrixTerm[0].value - �����з���Ԫ����
//
// ����������Ԫ��Ԫ�飬���±�row, ���±�col��Ԫ��ֵ�±�value����Ԫ�鰴��������ͬ��Ԫ�ذ�������������.

extern MatrixTerm g_matrixTerm[MAX_MATRIX_TERMS];

// ����ת��
// �С��л���. ԭ�����е� a[i][j] ת�ú��ڽ��������λ�� b[j][i].
// ����ֱ��ת�ò��룺
// �Ȱѵ�0�е�������Ԫ��˳���ŵ��������ĵ�0�У��ٰѵ�1�е�������Ԫ��˳���ŵ��������ĵ�1�У��ȵȡ�
// �������ͬ������˳��.
void transpose(MatrixTerm a[], MatrixTerm b[]);

/*
 * @brief:
 * ��ȷ��ԭ������ÿ�з���Ԫ�صĸ�����������ת�þ�����ÿ�з���Ԫ�صĸ���.
 * ��ˣ�ת�þ�����ÿ�е���ʼλ�þ�ȷ�������������ԭ�����е�ÿ�����ݿ��Զ�Ӧ��ת�þ������֪λ��.
 * Լ����ԭ������������������ MAX_COl.
 */
void fastTranspose(MatrixTerm a[], MatrixTerm b[]);

void Mmult(MatrixTerm a[], MatrixTerm b[], MatrixTerm d[]);
void StoreSum(MatrixTerm d[], int *totalD, int row, int col, int *sum);

#endif
