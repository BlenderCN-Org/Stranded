/*
* 单链表
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
	first 指向表头，如果为空指针，则返回后它的值变成指向数据域为50的指针，由于这个指针的内容可能改变，
	必须传这个指针的地址.
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
/// 链式多项式
typedef struct _polyNode {
	float coef;	// 系数
	int expon;	// 指数
	struct _polyNode *link;
}polyNode;

/*
* 在表尾接入结点
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
* 每次生成一个新结点，在 coef、expon 两域赋值之后，都接在 c的最后。避免每次从表头向后遍历全表，增设了一个指向 c 表尾的指针 rear.
* 创建新结点，然后把它接在 c 表尾.
* 开始时为 c 设置一个空结点，最后再把这个结点删除掉，可以节省计算时间
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
/// 循环链表 - 单链表首尾相接 表尾链域值指向表头

/* provide a node for use */
/* avail 指向回收链表的表头 */
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
/// 链表其它操作

typedef struct _listNode
{
	char data;
	struct _listNode *link;
} listNode;

/*
* 单链表反向
* invert the list pointed to by lead
* 把链表 "在位" (in place) 变反
* while 语句对长度 n>=1 的链表共执行n次 O(n)
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
 * 将两个单向链表 ptr1、ptr2 连接起来
 * O(ptr1的长度). 不申请新结点，连接结构就存放在 ptr1 中
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
/// 循环链表 last 指向表尾

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
* 表尾后插入结点
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
		// 可以画一个两个结点的链表示意图来理解
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