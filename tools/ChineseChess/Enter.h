#pragma once


// CEnter 对话框

class CEnter : public CDialog
{
	DECLARE_DYNAMIC(CEnter)

public:
	CEnter(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CEnter();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG3 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	virtual void OnOK();
	virtual void OnCancel();

public:
	// 是否已经选择模式
	bool m_bControl;
};
