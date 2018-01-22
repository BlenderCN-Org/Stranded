#pragma once


// CGetin 对话框

class CGetin : public CDialog
{
	DECLARE_DYNAMIC(CGetin)

public:
	CGetin(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CGetin();

// 对话框数据
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_ip;
	CString m_port;
	virtual void OnOK();
};
