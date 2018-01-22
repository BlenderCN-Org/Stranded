#pragma once

#include "NetControl.h"

class CManagers
{
public:
	CManagers();
	virtual ~CManagers();
	friend class CNetControl;

public:
	void GameStart();
	bool DoMsg(int x, int y, int action);

protected:
	void Update();
	bool MoveTo(int x, int y, bool isgo);
	SChess* Search(int x, int y);

public:
	int			m_gameState;
	bool		m_bPause;
	bool		m_bGameType;		// 单机模式 false, 网络对战 true
	int			m_playerAc;			// 先手 0, 后手 1
	int			m_map[9][10];		// 记录棋盘上的信息

	SChess		m_LastCh;
	SChess		*m_pLossCh;
	SChess		*m_pPointCh;
	SChess		m_nChessInfo[32];	// 记录32个棋子的信息
};

