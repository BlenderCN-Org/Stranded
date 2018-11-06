#include "polynomial.h"
#include "commonInclude.h"
#include <stdio.h>
#include <stdlib.h>

// 全局数组
polynomial g_terms[MAX_TERM];
int g_avail = 0;

// add A(x) and B(x) to obtain D(x)
void padd(int startA, int finishA, int startB, int finishB, int *startD, int *finishD)
{
	float coefficient;
	*startD = g_avail;

	while (startA <= finishA && startB <= finishB)
	{
		switch (COMM_COMPARE(g_terms[startA].expon, g_terms[startB].expon))
		{
		case -1:	// a expon < b expon
			attach(g_terms[startB].coef, g_terms[startB].expon);
			startB++;
			break;
		case 0:	// equal exponents
			coefficient = g_terms[startA].coef + g_terms[startB].coef;
			if (coefficient)
				attach(coefficient, g_terms[startA].expon);
			startA++;
			startB++;
			break;
		case 1:	// a expon > b expon
			attach(g_terms[startA].coef, g_terms[startA].expon);
			startA++;
		}
	}

	// add in remaining g_terms of A(x)
	for (; startA <= finishA; startA++)
		attach(g_terms[startA].coef, g_terms[startA].expon);
	// add in remaining g_terms of B(x)
	for (; startB <= finishB; startB++)
		attach(g_terms[startB].coef, g_terms[startB].expon);

	*finishD = g_avail - 1;
}

// add a new term to the polynomial
void attach(float coefficient, int exponent)
{
	if (g_avail >= MAX_TERM)
	{
		fprintf(stderr, "Too many terms in the polynomial\n");
		exit(EXIT_FAILURE);
	}

	g_terms[g_avail].coef = coefficient;
	g_terms[g_avail++].expon = exponent;
}
