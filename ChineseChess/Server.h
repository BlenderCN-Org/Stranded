#pragma once

#include "Resource.h"

// CServer �Ի���

class CServer : public CDialog
{
	DECLARE_DYNAMIC(CServer)

public:
	CServer(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CServer();

// �Ի�������
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CString m_port;
};
