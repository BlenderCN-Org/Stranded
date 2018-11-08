#include "stdafx.h"
#include "NetControl.h"
#include "ChineseChessView.h"
#include "Server.h"
#include "Getin.h"

#pragma warning(push)
#pragma warning(disable:4996)

CNetControl::CNetControl()
{
	m_ready[0] = false;
	m_ready[1] = false;
	m_pSocket = nullptr;
	m_port = 100;
	m_pListening = nullptr;
}

CNetControl::CNetControl(CChineseChessView *p)
{
	m_ready[0] = false;
	m_ready[1] = false;
	m_pView = p;
	m_pSocket = nullptr;
	m_server = "";
	m_port = 100;
	m_pListening = nullptr;
}

CNetControl::~CNetControl()
{
	if(m_pSocket)
	{
		delete m_pSocket;
		m_pSocket = nullptr;
	}

	if(m_pListening)
	{
		delete m_pListening;
		m_pListening = nullptr;
	}
}

void CNetControl::SendAction(int x, int y, int action/*=1*/)
{
	char st[10] = "2008";
	st[4] = x+100;
	st[5] = y+100;
	st[6] = action + 100;
	st[7] = '\0';
	SendMsg(st);
}

void CNetControl::SendMsg(char const *pMsg)
{
	m_pSocket->Send(pMsg, strlen(pMsg));
}

void CNetControl::FetchMsg(CNet *pRequest)
{
	char msg[10000], tmpMsg[1000];
	int byteCount;
	int end = 0;

	CStringArray *tmp = new CStringArray;
	strcpy(msg, "");
	do 
	{
		strcpy(tmpMsg, "");
		byteCount = pRequest->Receive(tmpMsg, 1000);
		if(byteCount > 1000 || byteCount <= 0)
		{
			MessageBox(GetActiveWindow(), "����������Ϣ��������", "������Ϣ", MB_OK);
			return;
		}
		else if(byteCount < 1000 && byteCount > 0)
			end = 1;

		tmpMsg[byteCount] = 0;
		strcat(msg, tmpMsg);
	} while (end = 0);

	CString oldMsg = msg;

	if(!strncmp(msg, "2007", 4))
	{
		int k = msg[4] - 100;
		g_manager.m_playerAc = (k+1)%2;
		m_pView->m_playerName[k].Format("%s", oldMsg.Right(oldMsg.GetLength()-5));
		
		char st[100] = { 0 };
		sprintf(st, "2006%c%s", g_manager.m_playerAc+100, (LPCTSTR)m_pView->m_playerName[g_manager.m_playerAc]);
		SendMsg(st);
	}
	else if(!strncmp(msg, "2006", 4))
		m_pView->m_playerName[msg[4]-100].Format("%s", oldMsg.Right(oldMsg.GetLength()-5));
	else if(!strcmp(msg, "2005"))
	{
		m_ready[1] = true;
		if(m_ready[0])
		{
			g_manager.GameStart();
			m_pView->PrintAll();
		}
	}
	else if(!strncmp(msg, "2008", 4))
	{
		int x, y;
		x = msg[4]-100;
		y = msg[5]-100;
		if(msg[6]-100 == 1)
		{
			if(g_manager.m_map[x][y] == g_manager.m_gameState+1)
			{
				g_manager.m_pPointCh = g_manager.Search(x, y);
				return;
			}
			else
			{
				g_manager.MoveTo(x, y, true);
				g_manager.Update();
				m_pView->PrintAll();
				return;
			}
		}
		else if(msg[6]-100 == 2)
		{
			g_manager.m_gameState = (g_manager.m_gameState+1)%2;
			g_manager.m_gameState += 2;
			MessageBox(GetActiveWindow(), "��Ķ���������", "��ϲ", MB_OK);
			m_ready[0] = false;
			m_ready[1] = false;
		}
		else if(msg[6]-100 == 3)
		{
			int k;
			k = MessageBox(GetActiveWindow(), "�Է�Ҫ����壬���Ƿ����?", "����", MB_OKCANCEL);
			if(k == IDOK)
			{
				SendAction(0, 0, 4);
				m_ready[0] = false;
				m_ready[1] = false;
				g_manager.m_gameState = 5;
			}
			else
				SendAction(0, 0, 5);
		}
		else if(msg[6]-100 == 4)
		{
			MessageBox(GetActiveWindow(), "�Է������˺���", "����", MB_OK);
			g_manager.m_gameState = 5;
			m_ready[0] = false;
			m_ready[1] = false;
		}
		else if(msg[6]-100 == 5)
			MessageBox(GetActiveWindow(), "�Է���Ը�����", "����", MB_OK);
	}
}

void CNetControl::OnSorc()
{
	int k;
	k = MessageBox(GetActiveWindow(), "���Ƿ�������? (����/�ͻ��� Y/N)", "��������", MB_OKCANCEL);
	if(k == IDOK)
	{
		CServer serverDlg;
		if(serverDlg.DoModal() == IDOK)
		{
			m_port = atoi(serverDlg.m_port);

			OnSet();

			m_isServer = true;
		}
		
		g_manager.m_playerAc = 0;
	}
	else
	{
		CGetin clientDlg;
		if(clientDlg.DoModal() == IDOK)
		{
			m_server = clientDlg.m_ip;
			m_port = atoi(clientDlg.m_port);

			OnOpen();

			m_isServer = false;
		}

		g_manager.m_playerAc = 1;
	}
}

void CNetControl::OnSet()
{
	m_pListening = new CNet(this);
	if(m_pListening->Create(m_port))
	{
		if(!m_pListening->Listen())
			MessageBox(GetActiveWindow(), "�˿����ô���!", "�������", MB_OK);
	}
}

void CNetControl::OnOpen()
{
	if(m_pSocket)
	{
		MessageBox(GetActiveWindow(), "�Ѿ����ӵ�����������ر���������һ��!", "������Ϣ", MB_OK);
		return;
	}

	if(!(m_pSocket = ConnectServer()))
	{
		m_pSocket = nullptr;
		return;
	}
}

CNet* CNetControl::ConnectServer()
{
	CNet *pSocket = new CNet(this);
	if(!(pSocket->Create()))
	{
		delete pSocket;
		MessageBox(GetActiveWindow(), "����Socketʧ��", "������Ϣ", MB_OK);
		return nullptr;
	}

	// ���ӵ�������
	if(!(pSocket->Connect((LPCTSTR)m_server, m_port)))
	{
		delete pSocket;
		MessageBox(GetActiveWindow(), "�������ӵ�������ʧ��", "������Ϣ", MB_OK);
		return nullptr;
	}

	return pSocket;
}

#pragma warning(pop)
