
// ChineseChessView.h : CChineseChessView ��Ľӿ�
//

#pragma once

#include "NetControl.h"
#include "ChineseChessDoc.h"

#define CHESS_ID_OK			100
#define CHESS_ID_LOSE		101
#define CHESS_ID_PEACE		102

class CChineseChessView : public CView
{
protected: // �������л�����
	CChineseChessView();
	DECLARE_DYNCREATE(CChineseChessView)

// ����
public:
	CChineseChessDoc* GetDocument() const;

// ����
public:
	int m_num1;
	int m_num2;

	CDC m_chessAll;
	CDC m_buffer;

	CBitmap m_chessbord;
	CBitmap m_chess;
	CBitmap m_mask;
	CBitmap m_choose;

	CBitmap m_memBitmap;

	CButton m_ok;
	CButton m_lose;
	CButton m_peace;

	CNetControl *m_pControl;

	CString m_playerName[2];

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// ʵ��
public:
	virtual ~CChineseChessView();
	
	void OnPeace();
	void OnLose();
	void OnOK();

	void DrawChoose(int x, int y, CDC *pdc);
	void DrawChess(SChess ch, CDC *pdc);
	void DrawStar(int x, int y, CDC *pdc);
	void DrawChessbord(CDC *pdc);

	void PrintState(CDC *padc);
	void PrintAll();
	int SelectNum(int x, int y);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // ChineseChessView.cpp �еĵ��԰汾
inline CChineseChessDoc* CChineseChessView::GetDocument() const
   { return reinterpret_cast<CChineseChessDoc*>(m_pDocument); }
#endif

