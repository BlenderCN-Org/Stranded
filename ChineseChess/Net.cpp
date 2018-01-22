#include "stdafx.h"
#include "Net.h"
#include "NetControl.h"
#include "ChineseChessView.h"


#pragma warning(push)
#pragma warning(disable:4996)

CNet::CNet()
{
}


CNet::CNet(CNetControl *pNcc)
{
	m_pNc = pNcc;
}

CNet::~CNet()
{
}

void CNet::OnReceive(int nErrorCode)
{
	m_pNc->FetchMsg(this);

	CSocket::OnReceive(nErrorCode);
}

void CNet::OnAccept(int nErrorCode)
{
	CNet *pSocket = new CNet(m_pNc);
	if (m_pNc->GetListen()->Accept(*pSocket) && m_pNc->GetSocket() == nullptr)
	{
		m_pNc->SetSocket(pSocket);

		char sz[100] = { 0 };
		sprintf(sz, "2007%c%s", g_manager.m_playerAc + 100, (LPCTSTR)m_pNc->GetChineseChessView()->m_playerName[g_manager.m_playerAc]);
		m_pNc->SendMsg(sz);
	}
	else
		delete pSocket;

	CSocket::OnAccept(nErrorCode);
}

#pragma warning(pop)
