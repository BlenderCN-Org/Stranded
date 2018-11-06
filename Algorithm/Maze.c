/*
* 迷宫问题
*
* 先保存当前位置，然后先试探正北方向，然后顺时针一一试探其他方向。
* 如果随后的搜索碰到丝路，回到当前保存起来的位置，然后尝试另一个尚未搜索的方向.
* 为避免回到以前试探过的思路，用数组mark标记已试探过的位置，初始化为全零，当访问maze[row][col]之后，mark[row][col]置1.
*
* 迷宫靠墙位置，领域数目小于8，方便处理，迷宫加上一圈，省去边界检查。
* m*p的迷宫需要(m+2)*(p+2)大小的数组，入口点maze[1][1]，出口点maze[m][p].
*
* 在找到出口前，所有零值位置(除出口外)都要入栈，m*p的迷宫，最多只能有m*p个单元，栈容量也是m*p.
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
* 移动方向
* 水平座标 - 垂直座标
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
	// 1 不可联通(墙)
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
						// n=rand()%(Y-X+1)+X; /*n为X~Y之间的随机数*/
						// 函数int rand( void );返回的是一个界于0~32767(0x7FFF)之间的伪随机数，包括0和32767。
						g_maze[i][j] = rand() % 2;
					}
				}
			}
		}

	//////////////////////////////////////////////////////////////////////////
	// 打印出迷宫
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
	// 初始化随机数种子
	__time32_t tNow;
	_time32(&tNow);
	// void srand( unsigned int seed )
	srand(tNow);

	Init();

	Path();

	return 0;
}