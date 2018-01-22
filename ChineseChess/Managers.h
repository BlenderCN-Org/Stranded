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
	bool		m_bGameType;		// ����ģʽ false, �����ս true
	int			m_playerAc;			// ���� 0, ���� 1
	int			m_map[9][10];		// ��¼�����ϵ���Ϣ

	SChess		m_LastCh;
	SChess		*m_pLossCh;
	SChess		*m_pPointCh;
	SChess		m_nChessInfo[32];	// ��¼32�����ӵ���Ϣ
};

