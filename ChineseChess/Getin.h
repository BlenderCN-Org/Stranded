#pragma once


// CGetin �Ի���

class CGetin : public CDialog
{
	DECLARE_DYNAMIC(CGetin)

public:
	CGetin(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CGetin();

// �Ի�������
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CString m_ip;
	CString m_port;
	virtual void OnOK();
};
