// Getin.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ChineseChess.h"
#include "Getin.h"
#include "afxdialogex.h"


// CGetin �Ի���

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


// CGetin ��Ϣ�������


void CGetin::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���
	UINT k = atoi(m_port);
	if(k < 1 || k > 60000)
	{
		MessageBox("����˿���Ч������Ĭ�϶˿�", "����");
		m_port = "100";
	}

	CDialog::OnOK();
}
