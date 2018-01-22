#pragma once

#include "Net.h"

class CChineseChessView;

class CNetControl
{
public:
	CNetControl();
	CNetControl(CChineseChessView *p);
	virtual ~CNetControl();

	void SendAction(int x, int y, int action=1);
	void SendMsg(char const *pMsg);
	void FetchMsg(CNet *pRequest);
	void OnSorc();
	void OnSet();
	void OnOpen();
	CNet* ConnectServer();

public:
	UINT GetPort() { return m_port; }
	CNet* GetSocket() { return m_pSocket; }
	void SetSocket(CNet* p) { m_pSocket = p; }
	CNet* GetListen() { return m_pListening; }
	bool* GetReady() { return m_ready; }
	bool IsServer() { return m_isServer; }
	CChineseChessView* GetChineseChessView() { return m_pView; }

private:
	UINT			m_port;				// �˿�
	CString			m_password;
	CString			m_server;			// ����������
	CNet			*m_pSocket;			// ��������Socket
	CNet			*m_pListening;		// ����Socket
	CChineseChessView	*m_pView;
	bool			m_ready[2];
	bool			m_isServer;
};