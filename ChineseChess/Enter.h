#pragma once


// CEnter �Ի���

class CEnter : public CDialog
{
	DECLARE_DYNAMIC(CEnter)

public:
	CEnter(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CEnter();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG3 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	virtual void OnOK();
	virtual void OnCancel();

public:
	// �Ƿ��Ѿ�ѡ��ģʽ
	bool m_bControl;
};
