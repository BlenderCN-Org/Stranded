// Enter.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ChineseChess.h"
#include "Enter.h"
#include "afxdialogex.h"
#include "Managers.h"


extern CManagers g_manager;

// CEnter �Ի���

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


// CEnter ��Ϣ�������


void CEnter::OnBnClickedRadio1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	// ����ģʽ
	g_manager.m_bGameType = false;
	m_bControl = true;
}


void CEnter::OnBnClickedRadio2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	// �����ս
	g_manager.m_bGameType = true;
	m_bControl = true;
}


void CEnter::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���
	if (m_bControl == false)
	{
		MessageBox("��ѡ��һ����Ϸģʽ!");
		return;
	}

	CDialog::OnOK();
}


void CEnter::OnCancel()
{
	// TODO: �ڴ����ר�ô����/����û���
	exit(0);

	CDialog::OnCancel();
}
