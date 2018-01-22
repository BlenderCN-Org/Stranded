// Server.cpp : 实现文件
//

#include "stdafx.h"
#include "ChineseChess.h"
#include "Server.h"
#include "afxdialogex.h"


// CServer 对话框

IMPLEMENT_DYNAMIC(CServer, CDialog)

CServer::CServer(CWnd* pParent /*=NULL*/)
	: CDialog(CServer::IDD, pParent)
	, m_port(_T("100"))
{

}

CServer::~CServer()
{
}

void CServer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_port);
}


BEGIN_MESSAGE_MAP(CServer, CDialog)
END_MESSAGE_MAP()


// CServer 消息处理程序
