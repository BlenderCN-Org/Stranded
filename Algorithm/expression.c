#include <stdio.h>
#include <stdlib.h>

#define MAX_STACK_SIZE 100	/* maximum stack size */

//
const char* g_expr = "62/3-42*+";

int g_stack[MAX_STACK_SIZE];
int g_top = -1;

void StackFull()
{
	fprintf(stderr, "Stack is full, cannot add element!\n");
	exit(EXIT_FAILURE);
}

void push(int item)
{
	if (g_top >= MAX_STACK_SIZE - 1)
		StackFull();

	g_stack[++g_top] = item;
}

int pop()
{
	if (g_top == -1)
	{
		fprintf(stderr, "Stack is empty, cannot pop element!\n");
		exit(EXIT_FAILURE);
	}

	return g_stack[g_top--];
}

/* ������ʽ���е� token */
typedef enum {
	lparen,
	rparen,
	plus,
	minus,
	times,
	divide,
	mod,
	eos,		/* end of string */
	operand
} precedence;

/*
* �����봮��ȡ token
*
* get the next token, symbol is the character representation, which is returned,
* the token is represented by its enumerated value, which is returned in the function name
*/
precedence GetToken(char *symbol, int *n)
{
	*symbol = g_expr[(*n)++];
	switch (*symbol)
	{
	case '(':
		return lparen;
	case ')':
		return rparen;
	case '+':
		return plus;
	case '-':
		return minus;
	case '*':
		return times;
	case '/':
		return divide;
	case '%':
		return mod;
	case '\0':
		return eos;
	default:
		return operand;
		/* no error checking, default is operand */
	}
}

/*
* ʵ�ֺ�׺���ʽ��ֵ.
*
* ÿ��������(symbol)���ַ����е��ַ�����ֵǰҪ�ȱ����ֵ���� symbol - '0'��
* ��������symbol �� ASCII �ַ������ȥ '0' ��ASCII ���� 48.
*
* Gettoken �ӱ��ʽ��һ��ȡ��һ�� token���� token �ǲ���������ת����ֵ��ջ������(����Ԫ�ǲ�����)
* ��ջ���β����߲������ĺ���ִ����Ӧ�����㣬Ȼ�󽫽����ջ��ֱ������������󽫽����ջ.
*
* Evaluate a postfix expression, expr, maintained as a global variable.
* '\0' is the end of the expression.
* The stack and top of the stack are global variables.
* getToken is used to return the token type and the character symbol.
* Operands are assumed to be single character digits
*/
int eval()
{
	precedence token;
	char symbol;
	int op1, op2;
	int n = 0; /* counter for the expression string */
	int top = -1;

	token = GetToken(&symbol, &n);

	while (token != eos)
	{
		if (token == operand)
			push(symbol - '0');	/* stack insert */
		else
		{
			/* pop two operands, perform operation, and push result to the stack */
			op2 = pop();	/* stack delete */
			op1 = pop();
			switch (token)
			{
			case plus:
				push(op1 + op2);
				break;
			case minus:
				push(op1 - op2);
				break;
			case times:
				push(op1*op2);
				break;
			case divide:
				push(op1 / op2);
				break;
			case mod:
				push(op1%op2);
			}
		}

		token = GetToken(&symbol, &n);
	}

	return pop(); /* return result */
}

/*
* ��׺���ʽת���ɺ�׺���ʽ
* output the postfix of the expression. The expression string, the stack, and top are global
* �������ű��ʽ��
* a+b*c -> ��׺���ʽ abc*+
* �������ű��ʽ��
* a*(b+c)*d -> abc+*d*
*
* ջ��ŵ��� token �����Ƿ�����ջ������������ precedence. 
* ����������ջ����ջ˳��ȡ�������ȼ�������������Ҫ��־����������ջ����һ�������ȼ��Ĳ�������������
* ջ����һ�������ȼ��Ĳ�������ɨ�������һ��������������������ջ����һֱ����ջ�У�ֱ��ɨ�赽ƥ���
* �����Ųų�ջ��������������������ȼ���һ����ջ�����ȼ� isp (in-stack precedence)����һ����ջ����
* �ȼ� icp (incoming precedence).
* �������ȼ���
	isp and icp arrrays == index is value of precedence 
	lparen, rparen, plus, minus, times, divide, mod, eos
*
* �����ŵ�ջ�����ȼ�����Ϊ0��ջ�����ȼ�Ϊ20���������ŵ����ȼ��ߡ�
* ������ʱӦ��ջ����ջ�е����ݣ����eos�������ȼ�0
* ���ջ�ڲ�������ջ�����ȼ����ڻ���ڵ�ǰ������ջ�����������ôջ�ڲ�������ջ.
*/

int g_isp[] = { 0, 19, 12, 12, 13, 13, 13, 0 };
int g_icp[] = { 20, 19, 12, 12, 13, 13, 13, 0 };

precedence g_preStack[MAX_STACK_SIZE];
int g_preTop = -1;

void PrePush(precedence item)
{
	if (g_preTop >= MAX_STACK_SIZE - 1)
	{
		fprintf(stderr, "Stack is full, cannot add element!\n");
		exit(EXIT_FAILURE);
	}

	g_preStack[++g_preTop] = item;
}

precedence PrePop()
{
	if (g_preTop == -1)
	{
		fprintf(stderr, "Stack is empty, cannot pop element!\n");
		exit(EXIT_FAILURE);
	}

	return g_preStack[g_preTop--];
}

/* ��ö���������ַ���ʽ���*/
void PrintToken(precedence pre)
{
	switch (pre)
	{
	case lparen:
		printf("%c", '(');
		break;
	case rparen:
		printf("%c", ')');
		break;
	case plus:
		printf("%c", '+');
		break;
	case minus:
		printf("%c", '-');
		break;
	case times:
		printf("%c", '*');
		break;
	case divide:
		printf("%c", '/');
		break;
	case mod:
		printf("%c", '%');
		break;
	case eos:
		break;
	}
}

/* n �Ǳ��ʽ�е� token ����. O(n) */
void postfix()
{
	char symbol;
	precedence token;
	int n = 0;
	int top = 0;	/* place eos on stack */
	g_preStack[0] = eos;

	for (token = GetToken(&symbol, &n); token != eos; token = GetToken(&symbol, &n))
	{
		if (token == operand)
			printf("%c", symbol);
		else if (token == rparen)	/* ������ */
		{
			/* unstack tokens until left parenthesis */
			while (g_preStack[g_preTop] != lparen)
				PrintToken(PrePop());
			PrePop();	/* discard the left parenthesis */
		}
		else
		{
			/* remove and print symbols whose isp is greater than or equal to the current token's icp */
			while (g_isp[g_preStack[g_preTop]] >= g_icp[token])
				PrintToken(PrePop());
			PrePush(token);
		}
	}

	while ((token = PrePop()) != eos)
		PrintToken(token);
	printf("\n");
}