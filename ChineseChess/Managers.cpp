#include "stdafx.h"
#include "Managers.h"


CManagers::CManagers()
{
	int i, j;
	for (i = 0; i < 32; ++i)
		m_nChessInfo[i].m_type = CHESS_INVALID;

	for (i = 0; i < 9; ++i)
		for (j = 0; j < 10; ++j)
			m_map[i][j] = 0;

	m_gameState = -1;
	m_pPointCh = nullptr;
	m_pLossCh = nullptr;
	m_bPause = true;
	m_playerAc = -1;
	m_bGameType = false;
}


CManagers::~CManagers()
{
}

void CManagers::GameStart()
{
	int i, j;
	for (i = 0; i < 9; ++i)
		for (j = 0; j < 10; ++j)
			m_map[i][j] = 0;

	for (i = 0; i < 16; ++i)
		m_nChessInfo[i].m_color = true;

	for (i = 16; i < 32; ++i)
		m_nChessInfo[i].m_color = false;

	for (i = 0; i < 5; ++i)
	{
		m_map[i * 2][6] = 2;
		m_nChessInfo[i].m_type = CHESS_PAWN;
		m_nChessInfo[i].m_x = i * 2;
		m_nChessInfo[i].m_y = 6;
	}

	for (i = 16; i < 21; ++i)
	{
		m_map[(i - 16) * 2][3] = 1;
		m_nChessInfo[i].m_type = CHESS_PAWN;
		m_nChessInfo[i].m_x = (i - 16) * 2;
		m_nChessInfo[i].m_y = 3;
	}

	m_map[1][7] = 2;
	m_map[7][7] = 2;

	m_nChessInfo[5].m_type = CHESS_CANNON;					// 炮1
	m_nChessInfo[5].m_x = 1;
	m_nChessInfo[5].m_y = 7;

	m_nChessInfo[6].m_type = CHESS_CANNON;					// 炮2
	m_nChessInfo[6].m_x = 7;
	m_nChessInfo[6].m_y = 7;

	for (i = 7; i < 16; ++i)
		m_nChessInfo[i].m_y = 9;

	m_nChessInfo[7].m_type = CHESS_CHARIOT;					// 车1
	m_nChessInfo[7].m_x = 0;

	m_nChessInfo[8].m_type = CHESS_CHARIOT;					// 车2
	m_nChessInfo[8].m_x = 8;

	m_nChessInfo[9].m_type = CHESS_HORSE;					// 马1
	m_nChessInfo[9].m_x = 1;

	m_nChessInfo[10].m_type = CHESS_HORSE;					// 马2
	m_nChessInfo[10].m_x = 7;

	m_nChessInfo[11].m_type = CHESS_ELEPHANT;				// 相1
	m_nChessInfo[11].m_x = 2;

	m_nChessInfo[12].m_type = CHESS_ELEPHANT;				// 相2
	m_nChessInfo[12].m_x = 6;

	m_nChessInfo[13].m_type = CHESS_MILITARY_COUNSELOR;		// 士1
	m_nChessInfo[13].m_x = 3;

	m_nChessInfo[14].m_type = CHESS_MILITARY_COUNSELOR;		// 士2
	m_nChessInfo[14].m_x = 5;

	m_nChessInfo[15].m_type = CHESS_KING;					// 帅
	m_nChessInfo[15].m_x = 4;

	for (i = 0; i < 9; ++i)
	{
		m_map[i][0] = 1;
		m_map[i][9] = 2;
	}

	m_map[1][2] = 1;
	m_map[7][2] = 1;

	m_nChessInfo[21].m_type = CHESS_CANNON;					// 炮3
	m_nChessInfo[21].m_x = 1;
	m_nChessInfo[21].m_y = 2;

	m_nChessInfo[22].m_type = 2;							// 炮4
	m_nChessInfo[22].m_x = 7;
	m_nChessInfo[22].m_y = 2;

	for (i = 23; i < 32; ++i)
		m_nChessInfo[i].m_y = 0;

	m_nChessInfo[23].m_type = CHESS_CHARIOT;				// 车3
	m_nChessInfo[23].m_x = 0;

	m_nChessInfo[24].m_type = CHESS_CHARIOT;				// 车4
	m_nChessInfo[24].m_x = 8;

	m_nChessInfo[25].m_type = CHESS_HORSE;					// 马3
	m_nChessInfo[25].m_x = 1;

	m_nChessInfo[26].m_type = CHESS_HORSE;					// 马4
	m_nChessInfo[26].m_x = 7;

	m_nChessInfo[27].m_type = CHESS_ELEPHANT;				// 相3
	m_nChessInfo[27].m_x = 2;

	m_nChessInfo[28].m_type = CHESS_ELEPHANT;				// 相4
	m_nChessInfo[28].m_x = 6;

	m_nChessInfo[29].m_type = CHESS_MILITARY_COUNSELOR;		// 士3
	m_nChessInfo[29].m_x = 3;

	m_nChessInfo[30].m_type = CHESS_MILITARY_COUNSELOR;		// 士4
	m_nChessInfo[30].m_x = 5;

	m_nChessInfo[31].m_type = CHESS_KING;					// 将2
	m_nChessInfo[31].m_x = 4;

	m_pPointCh = nullptr;
	m_bPause = false;
	m_gameState = 0;
	m_pLossCh = nullptr;
}

/*
 * @brief: 用户消息响应函数
 * @param: (x,y) 棋盘位置; action - 动作 0 鼠标经过　1 鼠标左键
 */
bool CManagers::DoMsg(int x, int y, int action)
{
	if (m_bPause)
		return false;

	if (m_gameState < 0 || m_gameState > 1)
		return false;

	if (action == 1 && (m_gameState == m_playerAc || !m_bGameType))
	{
		if (m_pPointCh == nullptr)
		{
			if (m_map[x][y] - 1 == m_gameState)
			{
				m_pPointCh = Search(x, y);
				return true;
			}
			else
				return false;
		}
		else if ((m_pPointCh->m_color && m_gameState == 1) || (!m_pPointCh->m_color && m_gameState == 0))
		{
			if (m_pPointCh->m_x == x && m_pPointCh->m_y == y)
			{
				m_pPointCh = nullptr;
				return true;
			}
			else if (MoveTo(x, y, true))
			{
				Update();
				return true;
			}
			else if (m_map[x][y] == m_gameState + 1)
			{
				m_pPointCh = Search(x, y);
				return true;
			}
			else
			{
				m_pPointCh = nullptr;
				return false;
			}
		}
		else
		{
			m_pPointCh = nullptr;
			return false;
		}
	}
	else if (action == 0 && (m_gameState == m_playerAc || !m_bGameType))
	{
		if (m_pPointCh != nullptr)
		{
			if (MoveTo(x, y, false))
			{
				// 在(x,y)处高亮显示
				return true;
			}
		}
	}

	return false;
}

void CManagers::Update()
{
	if (m_pLossCh != nullptr)
	{
		if (m_pLossCh->m_type == CHESS_KING)
		{
			m_gameState += 2;
			m_pLossCh->m_type = CHESS_INVALID;
			return;
		}
		else
			m_pLossCh->m_type = CHESS_INVALID;
	}

	if (m_nChessInfo[15].m_x == m_nChessInfo[31].m_x)
	{
		int i;
		for (i = m_nChessInfo[31].m_y + 1; i < m_nChessInfo[15].m_y; ++i)
			if (m_map[m_nChessInfo[31].m_x][i] != 0 && m_map[m_nChessInfo[31].m_x][i] != 100)
				break;

		if (i == m_nChessInfo[15].m_y - m_nChessInfo[31].m_y)
		{
			m_gameState = (m_gameState + 1) % 2;
			m_gameState += 2;
			return;
		}
	}

	m_gameState = (m_gameState + 1) % 2;
	m_pPointCh = nullptr;
}

bool CManagers::MoveTo(int x, int y, bool isgo)
{
	if(m_pPointCh == nullptr)
		return false;

	SChess &ch = *m_pPointCh;
	if ((m_map[x][y] == 1 && !ch.m_color) || (m_map[x][y] == 2 && ch.m_color))
		return false;

	if (ch.m_type == CHESS_PAWN)	// 兵
	{
		if ((abs(ch.m_x - x) == 1 && ch.m_y == y) || (abs(ch.m_y - y) == 1 && ch.m_x == x))
		{
			if (!ch.m_color && ch.m_y < 5 && ch.m_x != x)
				return false;

			if (ch.m_color && ch.m_y > 4 && ch.m_x != x)
				return false;

			if (!ch.m_color && y < ch.m_y)
				return false;

			if (ch.m_color && y > ch.m_y)
				return false;

			if (isgo)
			{
				m_LastCh = ch;
				m_map[ch.m_x][ch.m_y] = 0;
				if (m_map[x][y] > 0)
				{
					m_pLossCh = Search(x, y);
				}

				if (ch.m_color)
					m_map[x][y] = 2;
				else
					m_map[x][y] = 1;

				ch.m_x = x;
				ch.m_y = y;

				return true;
			}
			else
				return true;
		}
		else
			return false;
	}
	else if (ch.m_type == CHESS_CANNON)	// 炮
	{
		if ((ch.m_x != x && ch.m_y != y) || (ch.m_x == x && ch.m_y == y))
			return false;

		int i = ch.m_x - x, j = ch.m_y - y;
		if (i > 0)
			i--;
		else if (i < 0)
			i++;

		if (j > 0)
			j--;
		else if (j < 0)
			j++;

		int num = 0;
		while (i != 0 || j != 0)
		{
			if (m_map[ch.m_x - i][ch.m_y - j] != 0)
				num++;

			if (i > 0)
				i--;
			else if (i < 0)
				i++;

			if (j > 0)
				j--;
			else if (j < 0)
				j++;
		}

		if (m_map[x][y] != 0 && num != 1)
			return false;

		if (m_map[x][y] == 0 && num != 0)
			return false;

		if (isgo)
		{
			m_LastCh = ch;
			m_map[ch.m_x][ch.m_y] = 0;
			if (m_map[x][y] > 0)
			{
				m_pLossCh = Search(x, y);
			}

			if (ch.m_color)
				m_map[x][y] = 2;
			else
				m_map[x][y] = 1;

			ch.m_x = x;
			ch.m_y = y;

			return true;
		}
		else
			return true;
	}
	else if (ch.m_type == CHESS_CHARIOT)	// 车
	{
		if ((ch.m_x != x && ch.m_y != y) || (ch.m_x == x && ch.m_y == y))
			return false;

		int i = ch.m_x - x, j = ch.m_y - y;
		if (i > 0)
			i--;
		else if (i < 0)
			i++;

		if (j > 0)
			j--;
		else if (j < 0)
			j++;

		while(i != 0 || j != 0)
		{
			if(m_map[ch.m_x-i][ch.m_y-j] != 0)
				return false;

			if(i > 0)
				i--;
			else if (i < 0)
				i++;

			if(j > 0)
				j--;
			else if (j < 0)
				j++;
		}

		if(isgo)
		{
			m_LastCh = ch;
			m_map[ch.m_x][ch.m_y] = 0;
			if(m_map[x][y] > 0)
				m_pLossCh = Search(x, y);

			if(ch.m_color)
				m_map[x][y] = 2;
			else
				m_map[x][y] = 1;

			ch.m_x = x;
			ch.m_y = y;

			return true;
		}
		else
			return true;
	}
	else if (ch.m_type == CHESS_HORSE)	// 马
	{
		if((abs(ch.m_x-x)==1 && abs(ch.m_y-y)==2) || (abs(ch.m_x-x)==2 && abs(ch.m_y-y)==1))
		{
			// 蹩脚马
			int xi, yi;
			if(abs(ch.m_x-x) == 1)
			{
				xi = ch.m_x;
				yi = (ch.m_y+y)/2;
			}
			else
			{
				yi = ch.m_y;
				xi = (ch.m_x+x)/2;
			}

			// 有棋子
			if(m_map[xi][yi] != 0)
				return false;

			if(isgo)
			{
				m_LastCh = ch;
				m_map[ch.m_x][ch.m_y] = 0;
				if(m_map[x][y] > 0)
					m_pLossCh = Search(x, y);

				if(ch.m_color)
					m_map[x][y] = 2;
				else
					m_map[x][y] = 1;

				ch.m_x = x;
				ch.m_y = y;

				return true;
			}
			else
				return true;
		}
		else
			return false;
	}
	else if(ch.m_type == CHESS_ELEPHANT)	// 象
	{
		// 上界 0、1、2、3、4
		if(!ch.m_color && y > 4)
			return false;

		// 下界 5、6、7、8、9
		if(ch.m_color && y < 5)
			return false;

		if(abs(ch.m_x - x == 2) && abs(ch.m_y - y) == 2 && m_map[(ch.m_x+x)/2][(ch.m_y+y)/2] == 0)
		{
			if(isgo)
			{
				m_LastCh = ch;
				// 走过去
				m_map[ch.m_x][ch.m_y] = 0;
				if(m_map[x][y] > 0)
					m_pLossCh = Search(x, y);

				if(ch.m_color)
					m_map[x][y] = 2;
				else
					m_map[x][y] = 1;

				ch.m_x = x;
				ch.m_y = y;

				return true;
			}
			else
				return true;
		}
		else
			return false;
	}
	else if(ch.m_type == CHESS_MILITARY_COUNSELOR)	// 士
	{
		// x 范围: 0~8
		if (x < 3 || x > 5)
			return false;

		if(y>2 && y<7)
			return false;

		if(abs(ch.m_x-x)==1 && abs(ch.m_y-y) == 1)
		{
			if(isgo)
			{
				m_LastCh = ch;
				m_map[ch.m_x][ch.m_y] = 0;
				if(m_map[x][y] > 0)
					m_pLossCh = Search(x, y);

				if(ch.m_color)
					m_map[x][y] = 2;
				else
					m_map[x][y] = 1;

				ch.m_x = x;
				ch.m_y = y;

				return true;
			}
			else
				return true;
		}
		else
			return false;
	}
	else if(ch.m_type == CHESS_KING)	// 帅
	{
		if(x < 3 || x > 5)
			return false;

		if(y > 2 && y < 7)
			return false;

		if((abs(ch.m_x-x)==1 && ch.m_y==y) || (ch.m_x==x && abs(ch.m_y-y)==1))
		{
			if(isgo)
			{
				m_LastCh = ch;
				m_map[ch.m_x][ch.m_y] = 0;
				if(m_map[x][y] > 0)
					m_pLossCh = Search(x, y);

				if(ch.m_color)
					m_map[x][y] = 2;
				else
					m_map[x][y] = 1;

				ch.m_x = x;
				ch.m_y = y;
				
				return true;
			}
			else
				return true;
		}
		else
			return false;
	}

	return false;
}

SChess* CManagers::Search(int x, int y)
{
	int i;
	for (i = 0; i < 32; ++i)
		if (m_nChessInfo[i].m_x == x && m_nChessInfo[i].m_y == y && m_nChessInfo[i].m_type <= CHESS_KING)
			return m_nChessInfo + i;

	return nullptr;
}
