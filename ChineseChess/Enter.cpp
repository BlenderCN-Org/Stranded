// Enter.cpp : 实现文件
//

#include "stdafx.h"
#include "ChineseChess.h"
#include "Enter.h"
#include "afxdialogex.h"
#include "Managers.h"


extern CManagers g_manager;

// CEnter 对话框

IMPLEMENT_DYNAMIC(CEnter, CDialog)

CEnter::CEnter(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG3, pParent)
{
	m_bControl = false;
}

CEnter::~CEnter()
{
}

void CEnter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEnter, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, &CEnter::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CEnter::OnBnClickedRadio2)
END_MESSAGE_MAP()


// CEnter 消息处理程序


void CEnter::OnBnClickedRadio1()
{
	// TODO: 在此添加控件通知处理程序代码
	// 单机模式
	g_manager.m_bGameType = false;
	m_bControl = true;
}


void CEnter::OnBnClickedRadio2()
{
	// TODO: 在此添加控件通知处理程序代码
	// 网络对战
	g_manager.m_bGameType = true;
	m_bControl = true;
}


void CEnter::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	if (m_bControl == false)
	{
		MessageBox("请选择一种游戏模式!");
		return;
	}

	CDialog::OnOK();
}


void CEnter::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	exit(0);

	CDialog::OnCancel();
}
