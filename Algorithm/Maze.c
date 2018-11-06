/*
* �Թ�����
*
* �ȱ��浱ǰλ�ã�Ȼ������̽��������Ȼ��˳ʱ��һһ��̽��������
* ���������������˿·���ص���ǰ����������λ�ã�Ȼ������һ����δ�����ķ���.
* Ϊ����ص���ǰ��̽����˼·��������mark�������̽����λ�ã���ʼ��Ϊȫ�㣬������maze[row][col]֮��mark[row][col]��1.
*
* �Թ���ǽλ�ã�������ĿС��8�����㴦���Թ�����һȦ��ʡȥ�߽��顣
* m*p���Թ���Ҫ(m+2)*(p+2)��С�����飬��ڵ�maze[1][1]�����ڵ�maze[m][p].
*
* ���ҵ�����ǰ��������ֵλ��(��������)��Ҫ��ջ��m*p���Թ������ֻ����m*p����Ԫ��ջ����Ҳ��m*p.
* O(mp)
*/

#include <windows.h>
#include <stdio.h>
#include <time.h>

#define MAX_ROW 14
#define MAX_COL 17

#define EXIT_ROW MAX_ROW-2
#define EXIT_COL MAX_COL-2

/*
* �ƶ�����
* ˮƽ���� - ��ֱ����
*/
typedef struct {
	short vert;
	short horiz;
} offsets;

typedef struct {
	short row;
	short col;
	short dir;
} element;

/* array of moves for each direction */
offsets g_move[8];
int g_maze[MAX_ROW][MAX_COL];
int g_mark[MAX_ROW][MAX_COL];
int g_top;
element g_stack[MAX_ROW*MAX_COL];

void StackFull()
{
	fprintf(stderr, "Stack is full, cannot add element!\n");
	exit(EXIT_FAILURE);
}

void push(element item)
{
	if (g_top >= MAX_ROW*MAX_COL - 1)
		StackFull();

	g_stack[++g_top] = item;
}

element pop()
{
	if (g_top == -1)
	{
		fprintf(stderr, "Stack is empty, cannot pop element!\n");
		exit(EXIT_FAILURE);
	}
	
	return g_stack[g_top--];
}

void Init()
{
	for (int i = 0; i < MAX_ROW; i++)
		for (int j = 0; j < MAX_COL; j++)
			g_mark[i][j] = 0;
	//////////////////////////////////////////////////////////////////////////
	// N
	g_move[0].vert = -1;
	g_move[0].horiz = 0;
	// NE
	g_move[1].vert = -1;
	g_move[1].horiz = 1;
	// E
	g_move[2].vert = 0;
	g_move[2].horiz = 1;
	// SE
	g_move[3].vert = 1;
	g_move[3].horiz = 1;
	// S
	g_move[4].vert = 1;
	g_move[4].horiz = 0;
	// SW
	g_move[5].vert = 1;
	g_move[5].horiz = -1;
	// W
	g_move[6].vert = 0;
	g_move[6].horiz = -1;
	// NW
	g_move[7].vert = -1;
	g_move[7].horiz = -1;

	//////////////////////////////////////////////////////////////////////////
	// 1 ������ͨ(ǽ)
	for (int i = 0; i < MAX_ROW; i++)
		for (int j = 0; j < MAX_COL; j++)
		{
			if (i == 0)
				g_maze[i][j] = 1;
			else if (i == MAX_ROW - 1)
				g_maze[i][j] = 1;
			else
			{
				if (j == 0)
					g_maze[i][j] = 1;
				else if (j == MAX_COL - 1)
					g_maze[i][j] = 1;
				else
				{
					if (i == 1 && j == 1)
						g_maze[i][j] = 0;
					else if (i == MAX_ROW - 2 && j == MAX_COL - 2)
						g_maze[i][j] = 0;
					else
					{
						// n=rand()%(Y-X+1)+X; /*nΪX~Y֮��������*/
						// ����int rand( void );���ص���һ������0~32767(0x7FFF)֮���α�����������0��32767��
						g_maze[i][j] = rand() % 2;
					}
				}
			}
		}

	//////////////////////////////////////////////////////////////////////////
	// ��ӡ���Թ�
	for (int i = 0; i < MAX_ROW; i++)
	{
		for (int j = 0; j < MAX_COL; j++)
			printf("%2d", g_maze[i][j]);

		printf("\n");
	}
	printf("\n");
}

/* output a path through the maze if such a path exists */
void Path()
{
	int i, row, col, nextRow, nextCol, dir, found = FALSE;
	element position;

	g_mark[1][1] = 1;
	g_top = 0;
	g_stack[0].row = 1;
	g_stack[0].col = 1;
	g_stack[0].dir = 1;

	while (g_top > -1 && !found)
	{
		position = pop();
		row = position.row;
		col = position.col;
		dir = position.dir;

		while (dir < 8 && !found)
		{
			/* move in direction dir */
			nextRow = row + g_move[dir].vert;
			nextCol = col + g_move[dir].horiz;

			if (nextRow == EXIT_ROW && nextCol == EXIT_COL)
				found = TRUE;
			else if (!g_maze[nextRow][nextCol] && !g_mark[nextRow][nextCol])
			{
				g_mark[nextRow][nextCol] = 1;
				position.row = row;
				position.col = col;
				position.dir = ++dir;
				push(position);

				row = nextRow;
				col = nextCol;
				dir = 0;
			}
			else
				++dir;
		}
	}

	if (found)
	{
		printf("the path is:\n");
		printf("row  col:\n");
		for (i = 0; i <= g_top; i++)
			printf("%2d%5d", g_stack[i].row, g_stack[i].col);
		printf("%2d%5d\n", row, col);
		printf("%2d%5d\n", EXIT_ROW, EXIT_COL);
	}
	else
		printf("The maze does not have a path\n");
}

int main(int argc, char* argv[])
{
	// ��ʼ�����������
	__time32_t tNow;
	_time32(&tNow);
	// void srand( unsigned int seed )
	srand(tNow);

	Init();

	Path();

	return 0;
}