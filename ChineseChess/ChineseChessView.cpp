
// ChineseChessView.cpp : CChineseChessView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "ChineseChess.h"
#endif

#include "ChineseChessDoc.h"
#include "ChineseChessView.h"
#include "Managers.h"
#include "Enter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChineseChessView

IMPLEMENT_DYNCREATE(CChineseChessView, CView)

BEGIN_MESSAGE_MAP(CChineseChessView, CView)
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(CHESS_ID_OK, OnOK)
	ON_BN_CLICKED(CHESS_ID_LOSE, OnLose)
	ON_BN_CLICKED(CHESS_ID_PEACE, OnPeace)
END_MESSAGE_MAP()

// CChineseChessView 构造/析构

CChineseChessView::CChineseChessView()
{
	// TODO: 在此处添加构造代码
	m_playerName[0].Format("player1");
	m_playerName[1].Format("player2");

	m_pControl = new CNetControl(this);
}

CChineseChessView::~CChineseChessView()
{
	m_buffer.DeleteDC();
	m_memBitmap.DeleteObject();
}

void CChineseChessView::OnPeace()
{
	int k = MessageBox("你真的想和棋吗？", "", MB_OKCANCEL);
	if (k == IDOK)
	{
		if (!g_manager.m_bGameType)
		{
			g_manager.m_gameState = 5;
			return;
		}
		else
		{
			m_pControl->SendAction(0, 0, 3);
		}
	}
}

void CChineseChessView::OnLose()
{
	int k = MessageBox("你真的想认输吗？", "", MB_OKCANCEL);
	if (k == IDOK)
	{
		g_manager.m_gameState = (g_manager.m_gameState + 1) % 2;
		g_manager.m_gameState += 2;

		if (!g_manager.m_bGameType)
			return;

		m_pControl->SendAction(0, 0, 2);
		m_pControl->GetReady()[0] = false;
		m_pControl->GetReady()[1] = false;
	}
}

void CChineseChessView::OnOK()
{
	if(g_manager.m_gameState == 0 || g_manager.m_gameState == 1)
		return;

	CDC *pdc = GetDC();

	if(g_manager.m_bGameType)
	{
		if(m_pControl->GetSocket() == nullptr)
			return;

		m_pControl->GetReady()[0] = true;
		if(m_pControl->GetReady()[1])
		{
			g_manager.GameStart();
			PrintAll();
		}

		m_pControl->SendMsg("2005");
	}
	else if(!g_manager.m_bGameType)
		g_manager.GameStart();

	PrintAll();
}

void CChineseChessView::DrawChoose(int x, int y, CDC *pdc)
{
	m_chessAll.SelectObject(&m_chess);
	pdc->BitBlt(60+x*50, 45+y*50, 39, 39, &m_chessAll, 0, 0, MERGEPAINT);
	pdc->BitBlt(60+x*50, 45+y*50, 39, 39, &m_chessAll, 39, 0, SRCAND);
}

void CChineseChessView::DrawChess(SChess ch, CDC *pdc)
{
	if(!ch.m_color)
	{
		m_chessAll.SelectObject(&m_mask);
		pdc->BitBlt(60+ch.m_x*50, 45+ch.m_y*50, 39, 39, &m_chessAll, (7-ch.m_type)*39, 0, MERGEPAINT);
		m_chessAll.SelectObject(&m_chess);
		pdc->BitBlt(60+ch.m_x*50, 45+ch.m_y*50, 39, 39, &m_chessAll, (7-ch.m_type)*39, 0, SRCAND);
	}
	else
	{
		m_chessAll.SelectObject(&m_mask);
		pdc->BitBlt(60+ch.m_x*50, 45+ch.m_y*50, 39, 39, &m_chessAll, (7-ch.m_type)*39, 39, MERGEPAINT);
		m_chessAll.SelectObject(&m_chess);
		pdc->BitBlt(60+ch.m_x*50, 45+ch.m_y*50, 39, 39, &m_chessAll, (7-ch.m_type)*39, 39, SRCAND);
	}
}

void CChineseChessView::DrawStar(int x, int y, CDC *pdc)
{
	int m, n;
	if(x>0 && x<8)
	{
		for(m=-1; m<2;)
		{
			for(n=-1; n<2;)
			{
				pdc->MoveTo(80+50*x+m*5, 65+50*y+n*5);
				pdc->LineTo(80+50*x+m*15, 65+50*y+n*5);
				pdc->MoveTo(80+50*x+m*5, 65+50*y+n*5);
				pdc->LineTo(80+50*x+m*5, 65+50*y+n*15);
				n += 2;
			}
			m += 2;
		}
	}
	else if(x == 0)
	{
		for(m=-1; m<2;)
		{
			pdc->MoveTo(80+5, 65+50*3+m*5);
			pdc->LineTo(80+5, 65+50*3+m*20);
			pdc->MoveTo(80+5, 65+50*3+m*5);
			pdc->LineTo(80+5+15, 65+50*3+m*5);
			m += 2;
		}
	}
	else if(x == 8)
	{
		for(m=-1; m<2;)
		{
			pdc->MoveTo(80+8*50-5, 65+50*3+m*5);
			pdc->LineTo(80+8*50-5, 65+50*3+m*20);
			pdc->MoveTo(80+8*50-5, 65+50*3+m*5);
			pdc->LineTo(80+8*50-20, 65+50*3+m*5);
			m += 2;
		}
	}
}

void CChineseChessView::DrawChessbord(CDC *pdc)
{
	CPen chessbord1, chessbord2;
	chessbord1.CreatePen(0, 1, RGB(0, 0, 0));
	chessbord2.CreatePen(0, 3, RGB(0, 0, 0));
	
	int i, k;
	
	for(i=0; i<2; i++)
	{
		pdc->SelectObject(&chessbord1);

		// 画横线
		for(k=1; k<=4; k++)
		{
			pdc->MoveTo(80, 65+k*50+i*4*50);
			pdc->LineTo(80+50*8, 65+k*50+i*4*50);
		}

		// 画竖线
		for (k = 1; k <= 7; k++)
		{
			pdc->MoveTo(80 + 50 * k, 65 + i * 5 * 50);
			pdc->LineTo(80 + 50 * k, 65 + 4 * 50 + i * 5 * 50);
		}

		// 画宫
		pdc->MoveTo(80 + 50 * 3, 65 + i * 7 * 50);
		pdc->LineTo(80 + 50 * 5, 65 + 2 * 50 + i * 7 * 50);
		pdc->MoveTo(80 + 50 * 3, 65 + 2 * 50 + i * 7 * 50);
		pdc->LineTo(80 + 5 * 50, 65 + i * 7 * 50);

		// 画边框
		if (i == 1)
			pdc->SelectObject(&chessbord2);
		pdc->MoveTo(80 - i * 10, 65 - i * 10);
		pdc->LineTo(80 - i * 10, 65 + 9 * 50 + i * 10);
		pdc->MoveTo(80 - i * 10, 65 - i * 10);
		pdc->LineTo(80 + 8 * 50 + i * 10, 65 - i * 10);
		pdc->MoveTo(80 + 8 * 50 + i * 10, 65 - i * 10);
		pdc->LineTo(80 + 8 * 50 + i * 10, 65 + 9 * 50 + i * 10);
		pdc->MoveTo(80 + 8 * 50 + i * 10, 65 + 9 * 50 + i * 10);
		pdc->LineTo(80 - i * 10, 65 + 9 * 50 + i * 10);

		// 画星星
		pdc->SelectObject(&chessbord2);
		DrawStar(1, 2 + i * 5, pdc);
		DrawStar(7, 2 + i * 5, pdc);
		DrawStar(2, 3 + i * 3, pdc);
		DrawStar(4, 3 + i * 3, pdc);
		DrawStar(6, 3 + i * 3, pdc);
		DrawStar(0, 3 + i * 3, pdc);
		DrawStar(8, 3 + i * 3, pdc);
	}

	ReleaseDC(pdc);
	chessbord1.DeleteObject();
	chessbord2.DeleteObject();
}

void CChineseChessView::PrintState(CDC *padc)
{
	padc->SetBkMode(TRANSPARENT);
	padc->SetTextColor(RGB(255, 0, 0));
	padc->TextOut(260, 15, m_playerName[0]);
	padc->TextOut(260, 545, m_playerName[1]);
	padc->SetTextColor(RGB(0, 0, 0));

	CString str;
	if(g_manager.m_gameState == -1)
		str.Format("游戏还没开始  ");
	else if(g_manager.m_gameState == 0)
		str.Format("%s走棋  ", m_playerName[0]);
	else if(g_manager.m_gameState == 1)
		str.Format("%s走棋  ", m_playerName[1]);
	else if(g_manager.m_gameState == 2)
		str.Format("%s胜利  ", m_playerName[0]);
	else if(g_manager.m_gameState == 3)
		str.Format("%s胜利  ", m_playerName[1]);

	padc->TextOut(610, 60, str);
	str.ReleaseBuffer();
}

void CChineseChessView::PrintAll()
{
	CDC *pdc = GetDC();
	m_chessAll.SelectObject(&m_chessbord);
	m_buffer.BitBlt(0, 0, 800, 600, &m_chessAll, 0, 0, SRCCOPY);

	DrawChessbord(&m_buffer);

	int k = 0;
	for(; k <= 31; k++)
	{
		if(g_manager.m_nChessInfo[k].m_type <= CHESS_KING)
			DrawChess(g_manager.m_nChessInfo[k], &m_buffer);
	}

	if(g_manager.m_gameState != 0 && g_manager.m_gameState != 1)
	{
		m_num1 = 100;
		m_num1 = 100;
	}

	if(m_num1 < 100)
		DrawChoose(m_num1%10, m_num1/10, &m_buffer);

	if(m_num2 < 100)
		DrawChoose(m_num2%10, m_num2/10, &m_buffer);

	PrintState(&m_buffer);

	pdc->BitBlt(0, 0, 800, 600, &m_buffer, 0, 0, SRCCOPY);
}

int CChineseChessView::SelectNum(int x, int y)
{
	x = (x - 60) / 50;
	y = (y - 45) / 50;

	return y * 10 + x;
}

BOOL CChineseChessView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式
	m_chessbord.LoadBitmap(IDB_CHESSBORD);
	m_chess.LoadBitmap(IDB_CHESS);
	m_mask.LoadBitmap(IDB_MASK);
	m_choose.LoadBitmap(IDB_CHOOSE);

	return CView::PreCreateWindow(cs);
}

// CChineseChessView 绘制

void CChineseChessView::OnDraw(CDC* pDC)
{
	CChineseChessDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
	static bool st = true;
	if (st)
	{
		m_chessAll.CreateCompatibleDC(pDC);
		m_buffer.CreateCompatibleDC(NULL);

		m_memBitmap.CreateCompatibleBitmap(pDC, 800, 600);
		m_buffer.SelectObject(&m_memBitmap);
		m_buffer.FillSolidRect(0, 0, 800, 600, RGB(255, 255, 255));

		st = false;
	}

	PrintAll();
}


// CChineseChessView 诊断

#ifdef _DEBUG
void CChineseChessView::AssertValid() const
{
	CView::AssertValid();
}

void CChineseChessView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CChineseChessDoc* CChineseChessView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CChineseChessDoc)));
	return (CChineseChessDoc*)m_pDocument;
}
#endif //_DEBUG


// CChineseChessView 消息处理程序


void CChineseChessView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDC *pdc = GetDC();

	int mx = point.x;
	int my = point.y;

	CString st;

	if (mx >= 65 && my >= 50 && mx <= 490 && my <= 525)
	{
		int k;
		
		pdc->TextOut(600, 400, st);
		k = SelectNum(mx, my);

		if (g_manager.m_gameState == 0 || g_manager.m_gameState == 1)
		{
			if (g_manager.DoMsg(k % 10, k / 10, 0))
			{
				m_num2 = k;
				PrintAll();
			}
			else
			{
				m_num2 = 100;
				PrintAll();
			}
		}
	}

	ReleaseDC(pdc);

	CView::OnMouseMove(nFlags, point);
}


int CChineseChessView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	CEnter enDlg;
	enDlg.DoModal();

	int k;
	if (g_manager.m_bGameType)
	{
		m_pControl->OnSorc();
		if (m_pControl->IsServer())
		{
			k = MessageBox("是否选择先手(Y/N)", "象棋", MB_OKCANCEL);
			if (k == IDOK)
				g_manager.m_playerAc = 0;
			else if (k == IDCANCEL)
				g_manager.m_playerAc = 1;
		}
	}

	m_ok.Create(_T("开始"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(615, 355, 715, 395), this, CHESS_ID_OK);
	m_lose.Create(_T("认输"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(615, 305, 715, 345), this, CHESS_ID_LOSE);
	m_peace.Create(_T("和棋"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(615, 255, 715, 295), this, CHESS_ID_PEACE);

	m_ok.ShowWindow(SW_SHOW);
	m_lose.ShowWindow(SW_SHOW);
	m_peace.ShowWindow(SW_SHOW);

	return 0;
}


void CChineseChessView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int mx = point.x;
	int my = point.y;

	CDC *pdc = GetDC();
	CString st;
	int k;

	if (mx >= 65 && my >= 50 && mx <= 490 && my <= 525)
	{
		k = SelectNum(mx, my);
		if (g_manager.DoMsg(k % 10, k / 10, 1))
		{
			m_num1 = k;
			PrintAll();

			if (g_manager.m_bGameType)
				m_pControl->SendAction(k % 10, k / 10);

			if (g_manager.m_gameState > 1)
			{
				m_pControl->GetReady()[0] = false;
				m_pControl->GetReady()[1] = false;
			}
		}
		else
		{
			m_num1 = 100;
			PrintAll();
		}

		st.Format("%d, %d    ", m_num1 % 10, m_num1 / 10);
		pdc->TextOut(600, 5, st);
	}

	ReleaseDC(pdc);

	CView::OnLButtonDown(nFlags, point);
}
