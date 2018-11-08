// Getin.cpp : 实现文件
//

#include "stdafx.h"
#include "ChineseChess.h"
#include "Getin.h"
#include "afxdialogex.h"


// CGetin 对话框

IMPLEMENT_DYNAMIC(CGetin, CDialog)

CGetin::CGetin(CWnd* pParent /*=NULL*/)
	: CDialog(CGetin::IDD, pParent)
	, m_ip(_T(""))
	, m_port(_T("100"))
{

}

CGetin::~CGetin()
{
}

void CGetin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_ip);
	DDX_Text(pDX, IDC_EDIT2, m_port);
}


BEGIN_MESSAGE_MAP(CGetin, CDialog)
END_MESSAGE_MAP()


// CGetin 消息处理程序


void CGetin::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	UINT k = atoi(m_port);
	if(k < 1 || k > 60000)
	{
		MessageBox("输入端口无效，采用默认端口", "错误");
		m_port = "100";
	}

	CDialog::OnOK();
}
