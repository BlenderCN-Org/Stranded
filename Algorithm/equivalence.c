/*
* �ȼ���
*/

#include "commonInclude.h"

#define MAX_SIZE 24
#define FALSE 0
#define TRUE 1

typedef struct _node {
	int data;
	struct _node* link;
} node;

#pragma warning(push)
#pragma warning(disable:4996)
int main(int argc, char* argv[])
{
	// ָ���Ƿ��Ѿ���ӡ�˳�Աi
	short out[MAX_SIZE];
	// ������ʵ�i�У����ÿ����Ԫ��ͷ���
	node* seq[MAX_SIZE];
	node *x, *y, *top;
	int i, j, n;

	printf("Enter the size (<= %d)\n", MAX_SIZE);
	scanf("%d", &n);

	for (i = 0; i < n; i++)
	{
		/* initialize seq and out */
		out[i] = TRUE;
		seq[i] = NULL;
	}

	/* Phase 1: Input the equivalence pairs: */
	printf("Enter a pair of numbers (-1 -1 to quit): \n");
	scanf("%d%d", &i, &j);
	while (i >= 0)
	{
		COMM_MALLOC(x, sizeof(node));
		x->data = j;
		x->link = seq[i];
		seq[i] = x;

		COMM_MALLOC(x, sizeof(node));
		x->data = i;
		x->link = seq[j];
		seq[j] = x;

		printf("Enter a pair of numbers (-1 -1 to quit): \n");
		scanf("%d%d", &i, &j);
	}

	return EXIT_SUCCESS;
}
#pragma warning(pop)
