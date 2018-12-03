/*
* ������
*/

#include "commonInclude.h"

typedef struct _SListNode {
	int data;
	struct _SListNode *link;
} SListNode;

/* create a linked list with two nodes */
SListNode* create(int val)
{
	SListNode* first;
	COMM_MALLOC(first, sizeof(SListNode));

	first->data = val;
	first->link = NULL;

	return first;
}


/*
* @brief:
	insert a new node with data = 50 into the chain first after node x
	first ָ���ͷ�����Ϊ��ָ�룬�򷵻غ�����ֵ���ָ��������Ϊ50��ָ�룬�������ָ������ݿ��ܸı䣬
	���봫���ָ��ĵ�ַ.
*
* @sample: insert(&first, x);
*/
void insertList(SListNode ** first, SListNode* x, int val)
{
	SListNode *temp;
	COMM_MALLOC(temp, sizeof(SListNode));
	temp->data = val;

	if (*first)
	{
		temp->link = x->link;
		x->link = temp;
	}
	else
	{
		temp->link = NULL;
		*first = temp;
	}
}

/*
* delete x from the list, trail is the preceding node and *first is the front of the list
* @sample: delete(&first, y, y->link);
		delete(&first, NULL, first);
*/
void deleteList(SListNode ** first, SListNode* trail, SListNode* x)
{
	if (trail)
		trail->link = x->link;
	else
		*first = (*first)->link;

	free(x);
}

void printList(SListNode* first)
{
	printf("The list contains: \n");

	for (; first; first = first->link)
		printf("%4d", first->data);

	printf("\n");
}

//////////////////////////////////////////////////////////////////////////
/// ��ʽ����ʽ
typedef struct _polyNode {
	float coef;	// ϵ��
	int expon;	// ָ��
	struct _polyNode *link;
}polyNode;

/*
* �ڱ�β������
* create a new node with coef = coefficient and expon = exponent,
* attach it to the node pointed to by ptr, ptr is updated to point ot this new node
*/
void AttachPolynomials(float eoefficient, int exponent, polyNode** ptr)
{
	polyNode* temp;
	COMM_MALLOC(temp, sizeof(polyNode));
	temp->coef = eoefficient;
	temp->expon = exponent;
	(*ptr)->link = temp;
	*ptr = temp;
}

/*
* return a polynomial which is the sum of a and b
* ÿ������һ���½�㣬�� coef��expon ����ֵ֮�󣬶����� c����󡣱���ÿ�δӱ�ͷ������ȫ��������һ��ָ�� c ��β��ָ�� rear.
* �����½�㣬Ȼ��������� c ��β.
* ��ʼʱΪ c ����һ���ս�㣬����ٰ�������ɾ���������Խ�ʡ����ʱ��
* O(m+n)
*/
polyNode* PaddPolynomials(polyNode *a, polyNode *b)
{
	polyNode *c, *rear, *temp;
	int sum;

	COMM_MALLOC(rear, sizeof(polyNode));

	c = rear;

	while(a && b)
		switch (COMM_COMPARE(a->expon, b->expon))
		{
		case -1:	/* a->expon < b->expon */
			AttachPolynomials(b->coef, b->expon, &rear);
			b = b->link;
			break;
		case 0:		/* a->expon == b->expon */
			sum = a->coef + b->coef;
			if (sum)
				AttachPolynomials(sum, a->expon, &rear);
			a = a->link;
			b = b->link;
			break;
		case 1:		/* a->expon > b->expon */
			AttachPolynomials(a->coef, a->expon, &rear);
			a = a->link;
		}

	/*  copy rest of list a and then list b */
	for (; a; a = a->link)
		AttachPolynomials(a->coef, a->expon, &rear);
	for (; b; b = b->link)
		AttachPolynomials(b->coef, b->expon, &rear);

	rear->link = NULL;

	/* delete extra initial node */
	temp = c;
	c = c->link;
	free(temp);
	return c;
}

/* erase the polynomial pointed to by ptr */
void ErasePolynomials(polyNode **ptr)
{
	polyNode *temp;
	while (*ptr)
	{
		temp = *ptr;
		*ptr = (*ptr)->link;
		free(temp);
	}
}

//////////////////////////////////////////////////////////////////////////
/// ѭ������ - ��������β��� ��β����ֵָ���ͷ

/* provide a node for use */
/* avail ָ���������ı�ͷ */
polyNode* GetNode(polyNode* avail)
{
	polyNode *node;
	if (avail)
	{
		node = avail;
		avail = avail->link;
	}
	else
		COMM_MALLOC(node, sizeof(polyNode));

	return node;
}

/* return a node to the available list */
void RetNode(polyNode* node, polyNode* avail)
{
	node->link = avail;
	avail = node;
}

/* erase the circular list pointed to by ptr */
void NewErasePoly(polyNode **ptr, polyNode* avail)
{
	polyNode *temp;
	if (*ptr)
	{
		temp = (*ptr)->link;
		(*ptr)->link = avail;
		avail = temp;
		*ptr = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
/// ������������

typedef struct _listNode
{
	char data;
	struct _listNode *link;
} listNode;

/*
* ��������
* invert the list pointed to by lead
* ������ "��λ" (in place) �䷴
* while ���Գ��� n>=1 ������ִ��n�� O(n)
*/
listNode* InvertList(listNode *lead)
{
	listNode *middle, *trail;
	middle = NULL;
	while (lead)
	{
		trail = middle;
		middle = lead;
		lead = lead->link;
		middle->link = trail;
	}

	return middle;
}

/*
 * �������������� ptr1��ptr2 ��������
 * O(ptr1�ĳ���). �������½�㣬���ӽṹ�ʹ���� ptr1 ��
 * produce a new list that contains the list ptr1 followed by the list ptr2.
 * The list pointed to by ptr1 is changed permanently
 */
listNode* ConcatenateList(listNode* ptr1, listNode* ptr2)
{
	listNode* temp;
	/* check for empty lists */
	if (!ptr1) return ptr2;
	if (!ptr2) return ptr1;

	/* neither list is empty, find end of first list */
	for (temp = ptr1; temp->link; temp = temp->link);

	/* link end of first to start of second */
	temp->link = ptr2;
}

//////////////////////////////////////////////////////////////////////////
/// ѭ������ last ָ���β

/*
* insert node at the front of the circular list whose last node is last
*/
void CircularLinkedListInsertFront(listNode **last, listNode *node)
{
	if (!(*last))
	{
		/* list is empty, change last to point to new entry */
		*last = node;
		node->link = node;
	}
	{
		/* list is not empty, add new entry at front */
		node->link = (*last)->link;
		(*last)->link = node;
	}
}

/*
* ��β�������
*/
void CircularLinkedListInsertBack(listNode **last, listNode *node)
{
	if (!(*last))
	{
		/* list is empty, change last to point to new entry */
		*last = node;
		node->link = node;
	}
	{
		/* list is not empty, add new entry at front */
		node->link = (*last)->link;
		(*last)->link = node;
		// ���Ի�һ��������������ʾ��ͼ�����
		*last = node;
	}
}

/* find the length of the circular list last */
int CircularLinkedListLength(listNode *last)
{
	listNode *temp;
	int count = 0;
	if (last)
	{
		temp = last;
		do 
		{
			count++;
			temp = temp->link;
		} while (temp != last);
	}

	return count;
}