
// ChineseChessView.h : CChineseChessView 类的接口
//

#pragma once

#include "NetControl.h"
#include "ChineseChessDoc.h"

#define CHESS_ID_OK			100
#define CHESS_ID_LOSE		101
#define CHESS_ID_PEACE		102

class CChineseChessView : public CView
{
protected: // 仅从序列化创建
	CChineseChessView();
	DECLARE_DYNCREATE(CChineseChessView)

// 特性
public:
	CChineseChessDoc* GetDocument() const;

// 操作
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

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// 实现
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

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // ChineseChessView.cpp 中的调试版本
inline CChineseChessDoc* CChineseChessView::GetDocument() const
   { return reinterpret_cast<CChineseChessDoc*>(m_pDocument); }
#endif

