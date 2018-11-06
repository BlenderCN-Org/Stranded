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

/* 处理表达式串中的 token */
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
* 从输入串中取 token
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
* 实现后缀表达式求值.
*
* 每个操作量(symbol)是字符串中的字符，求值前要先变成数值，即 symbol - '0'，
* 这条语句把symbol 的 ASCII 字符编码减去 '0' 的ASCII 编码 48.
*
* Gettoken 从表达式中一次取出一个 token，若 token 是操作量则将其转成数值入栈，否则(即基元是操作符)
* 出栈两次并更具操作符的含义执行相应的运算，然后将结果入栈，直到串结束，最后将结果入栈.
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
* 中缀表达式转换成后缀表达式
* output the postfix of the expression. The expression string, the stack, and top are global
* （无括号表达式）
* a+b*c -> 后缀表达式 abc*+
* （带括号表达式）
* a*(b+c)*d -> abc+*d*
*
* 栈存放的是 token 的助记符，即栈的数据类型是 precedence. 
* 操作符的入栈、出栈顺序取决于优先级，左括号是重要标志，它出现在栈中是一个低优先级的操作符，而不在
* 栈中是一个高优先级的操作符。扫描过程中一旦遇到左括号则立刻入栈，并一直待在栈中，直到扫描到匹配的
* 右括号才出栈。因此左括号有两个优先级，一个是栈内优先级 isp (in-stack precedence)，另一个是栈外优
* 先级 icp (incoming precedence).
* 两种优先级表：
	isp and icp arrrays == index is value of precedence 
	lparen, rparen, plus, minus, times, divide, mod, eos
*
* 左括号的栈内优先级定义为0，栈外优先级为20，比右括号的优先级高。
* 串结束时应出栈所有栈中的内容，因此eos定义优先级0
* 如果栈内操作符的栈内优先级高于或等于当前遇到的栈外操作符，那么栈内操作符出栈.
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

/* 把枚举类型以字符形式输出*/
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

/* n 是表达式中的 token 个数. O(n) */
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
		else if (token == rparen)	/* 右括号 */
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