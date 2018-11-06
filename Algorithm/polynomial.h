#ifndef __ALGORITHM_POLYNOMIAL_H__
#define __ALGORITHM_POLYNOMIAL_H__

// 稀疏多项式: 很多项系数0
/*
 * A(x) = 2X(1000) + 1; B(x) = X(4) + 10X(3) + 3X(2) + 1
 *
 */

// size of terms array
#define MAX_TERM 100

typedef struct _polynomial {
	float coef;		// 系数
	int expon;		// 指数
} polynomial;

extern polynomial g_terms[MAX_TERM];
extern int g_avail;

// D = A + B
void padd(int startA, int finishA, int startB, int finishB, int *startD, int *finishD);
void attach(float coefficient, int exponent);

#endif
