#pragma once

class CNetControl;

class CNet : public CSocket
{
public:
	CNet();
	CNet(CNetControl *pNcc);
	virtual ~CNet();

public:
	virtual void OnReceive(int nErrorCode);
	virtual void OnAccept(int nErrorCode);

public:
	CNetControl *m_pNc;
};

