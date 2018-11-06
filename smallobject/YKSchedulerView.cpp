/*
* $Id: YKSchedulerView.cpp 2010-1-28 11:21:03Z soarchin $
* 
* Thie file is part of Scheduler for util function operation
* 
* Copyright (C)2006-2010 永凯软件技术(上海)有限公司 
* 
* Author yp
* 
* version 1.0
*/
// YKSchedulerView.cpp : implementation of the CYKSchedulerView class
//

#include "stdafx.h"
#include "YKScheduler.h"

#include "YKSchedulerDoc.h"
#include "YKSchedulerView.h"

#include "DataGridCtrl.h"
#include "DtAccess.h"
#include "ViewManager.h"
#include "GanttApi.h"

#include "LanguagePack.h"
#include "ComFunc.h"
#include "HelpIdDef.h"

#include "WorkSplitDlg.h"
#include "WorkOrderCombDlg.h"

#include "CCreatGridMenu.h"
#include "CalendarMgrDlg.h"

#include "FindDlg.h"
#include "PIShowDlg.h"

#include "MainFrm.h"

#include "GtFindDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_WORK_SELECT					42901
#define WM_RES_SELECT					42903
#define WM_GANTT_FIND					42906
#define WM_GANTT_ACTIVE					42907
#define WM_WORK_SPLIT					42908
#define WM_CALENDAR_MGR					42909
#define WM_ORDER_SPLIT					42910
#define WM_WORK_COMB					42911
#define WM_ORDER_COMB					42912
#define WM_PI_COMMOND					42913
#define WM_MOUSE_DOWNS					42914

#define WM_FIND_WORK					42915
#define WM_FIND_RESOURCE				42916
#define WM_FIND_ORDER					42917

#ifdef _CLOCK_DEBUG
#include "clock.h"

#define  CLOCK_PASE(tm)  tm.Pause();
#else
#define  CLOCK_PASE(tm)  
#endif
// CYKSchedulerView

IMPLEMENT_DYNCREATE(CYKSchedulerView, CView)

BEGIN_MESSAGE_MAP(CYKSchedulerView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CLOSE()


END_MESSAGE_MAP()

// CYKSchedulerView construction/destruction

CYKSchedulerView::CYKSchedulerView()
:m_bIsTalbe(FALSE)
,m_bFindFlg(FALSE)
,m_nFindType(-1)
,m_curScrollPos(0)
//,m_sortDirectFlg(TRUE)
,m_changeTable(false)
,m_viewOpenType(0)
//,m_itemGtherGridCtrl(NULL)
,m_dayShiftGridCtrl(NULL)
{
	// TODO: add construction code here
	m_bIsMouseDown = false;
	m_iNumber = 0;
	m_IsSendWorkOrder = FALSE;
	m_bIsUpTabChangeOrderWork = false;
}

CYKSchedulerView::~CYKSchedulerView()
{
	Close();
#ifndef _REMOVE_ITEMGATHER_
	ItemGatherGridCtrlRelease();
#endif
	DayShiftGridCtrlRelease();

}

BOOL CYKSchedulerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CYKSchedulerView drawing

void CYKSchedulerView::OnDraw(CDC* /*pDC*/)
{
	CYKSchedulerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}


// CYKSchedulerView printing

void CYKSchedulerView::OnFilePrintPreview()
{
	BCGPPrintPreview (this);
}

BOOL CYKSchedulerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CYKSchedulerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CYKSchedulerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CYKSchedulerView diagnostics

#ifdef _DEBUG
void CYKSchedulerView::AssertValid() const
{
	CView::AssertValid();
}

void CYKSchedulerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CYKSchedulerDoc* CYKSchedulerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CYKSchedulerDoc)));
	return (CYKSchedulerDoc*)m_pDocument;
}
#endif //_DEBUG


// CYKSchedulerView message handlers

void CYKSchedulerView::InitKeyList()
{
	if (m_tblType > 0 && m_nFindType == -1)
	{
		//CBizDateManager dm;
		//m_nAmount = dm.Init(m_tblType,m_rowKeyList);
	}
	else if (m_tblType > 0 && m_nFindType != -1)
	{
		::SendMessage(theApp.g_hMainFrm, IDU_TREE_FIND, m_tblType, m_nFindType);
	}
}

int CYKSchedulerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_tblType = theApp.g_tblType;

	// TODO:  Add your specialized creation code here

	SaveViewTip();
	if (theApp.g_tblType == ID_GANTT_RES_GL)
	{
		m_bIsTalbe = FALSE;

		GanttApi::Show(this, this, 0);
		//GanttCommonApi::SetFocus(theApp.g_tblType,NULL);
	}
	else if (theApp.g_tblType == ID_GANTT_ORDER_GL)
	{
		m_bIsTalbe = FALSE;

		OrderGanttApi::Show(this, this, 0);
		//GanttCommonApi::SetFocus(theApp.g_tblType,NULL);
	}
	else if (theApp.g_tblType == ID_RATE_RES_GL)
	{
		m_bIsTalbe = FALSE;

		ResRateGanttApi::Show(this, this, 0);
		//GanttCommonApi::SetFocus(theApp.g_tblType,NULL);
	}
	else if (theApp.g_tblType == ID_RATE_STA_GL)
	{
		m_bIsTalbe = FALSE;

		StockGanttApi::Show(this, this, 0);
		//GanttCommonApi::SetFocus(theApp.g_tblType,NULL);
	}
	else if (theApp.g_tblType == ID_Item_Gather_GL)
	{
		m_bIsTalbe = FALSE;
		ItemGatherApi::Show(this,this,0);
		//GanttCommonApi::SetFocus(theApp.g_tblType,NULL);
	}
	else if(theApp.g_tblType == ID_WORK_RESOURCE_GD)
	{
		m_bIsTalbe = FALSE;

		m_pWorkSe.OnCreate(this);
	}
	else if (theApp.g_tblType == ID_BOM_VIEW_DLG)
	{
		m_bIsTalbe = FALSE;

		m_bomViewDlg.SetItemId(theApp.m_BOMViewItemId);
		m_bomViewDlg.Create(CBOMViewDlg::IDD,this);
		m_bomViewDlg.ShowWindow(SW_SHOW);
	}
	else if (theApp.g_tblType == TblItemGather)
	{
#ifndef _REMOVE_ITEMGATHER_
		m_bIsTalbe = FALSE;
		GetItemGatherGridCtrl()->OnCreate(this);
		GetItemGatherGridCtrl()->ShowWindow(SW_SHOW);
#endif
	}
	else if (theApp.g_tblType == ID_DayShift_GD)
	{
		m_bIsTalbe = FALSE;
		GetDayShiftGridCtrl()->OnCreate(this);
		GetDayShiftGridCtrl()->ShowWindow(SW_SHOW);
	}
	else
	{

		//////////////////////////////////////////////////////////////////////////
		//初始化虚拟表格
		InitKeyList();

		m_bIsTalbe = TRUE;
		CRect rectDummy;
		rectDummy.SetRectEmpty ();

		//// Create tabs window:
		if (!m_wndTabs.Create(CBCGPTabWnd::STYLE_3D_ONENOTE, 
			rectDummy, this, 123))
		{
			TRACE0("Failed to create tab window\n");
			return -1;
		}
		m_wndTabs.SetFlatFrame ();
		m_wndTabs.SetLocation(CBCGPBaseTabWnd::LOCATION_TOP);
		m_wndTabs.SetTabBorderSize (0);
		m_wndTabs.AutoDestroyWindow (false);
		m_wndTabs.EnableActiveTabCloseButton(FALSE);

		LOGFONT		logFont;
		memset(&logFont, 0, sizeof(LOGFONT));
		YK_WSTRING strName = L"宋体";
		_tcscpy_s(logFont.lfFaceName,strName.size()+1,strName.c_str());

		// Add Focus altered values if required
		// Init LOGFONT
		logFont.lfWeight=FW_NORMAL;
		logFont.lfItalic=FALSE;
		logFont.lfUnderline=FALSE;
		logFont.lfStrikeOut=FALSE;
		logFont.lfHeight= 11;
		//-----------------------------------
		// Create font 
		//-----------------------------------
		logFont.lfCharSet=DEFAULT_CHARSET;
		logFont.lfOutPrecision=OUT_DEFAULT_PRECIS;
		logFont.lfClipPrecision=CLIP_DEFAULT_PRECIS;
		logFont.lfQuality=ANTIALIASED_QUALITY;
		logFont.lfPitchAndFamily=DEFAULT_PITCH;

		CFont tempFont;
		tempFont.CreateFontIndirect(&logFont);
		m_wndTabs.SetFont(&tempFont,1);

		if (DATATABLAPI::BeginstTble(m_tblType))
		{
			for(; DATATABLAPI::NotEndstTble(); DATATABLAPI::StepstTble())
			{
				stTable& table = DATATABLAPI::GetstTble();
				if (!table.mode.empty())
				{
					OpenPage(table);
					m_wndTabs.SetTabIndex(m_wndTabs.GetTabsNum()-1,table.GetNameIndex());
				}
			}
		}

		//初始化表格
		if (DATATABLAPI::BeginstTble(m_tblType))
		{
			if (DATATABLAPI::NotEndstTble())
			{
				if (theApp.g_tblType == TblBOM)
				{
					m_BOMInfo.SetStTable(DATATABLAPI::GetstTble());
					m_BOMInfo.OnCreate(this);
					//m_BOMInfo.ShowWindow(SW_SHOW);
					return 0;
				}
				else if (theApp.g_tblType == TblProduceIndication)
				{
					m_entityGrid.SetTblInfo(DATATABLAPI::GetstTble());
					m_entityGrid.OnCreate(this);
					m_entityGrid.ShowWindow(SW_SHOW);
					return 0;
				}
				m_pGdCtrl.EnableToolTips();
				m_pGdCtrl.SetStTable(DATATABLAPI::GetstTble());
				//m_pGdCtrl.SetMenuID(0);
				m_pGdCtrl.OnCreate(this);
			}
		}
	}
	return 0;
}

//设置区域位置
void CYKSchedulerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	
	if (m_pGdCtrl.GetSafeHwnd())
	{
		AdjustGridView();
	}
	if(m_pWorkSe.GetSafeHwnd())
	{
		m_pWorkSe.MoveWindow ( 0, 0, cx, cy);
		if (m_pWorkSe.GetClientRowCount() > 0 &&
			m_pWorkSe.UpdateClientRowCount())
		{
			m_pWorkSe.Flash(FALSE);
		}
	}
	if (m_BOMInfo.GetSafeHwnd())
	{
		AdjustGridView();
	}
#ifndef _REMOVE_ITEMGATHER_
	if (m_itemGtherGridCtrl != NULL && m_itemGtherGridCtrl->GetSafeHwnd())
	{
		m_itemGtherGridCtrl->MoveWindow(0,0,cx,cy);
	}
#endif
	if (m_dayShiftGridCtrl != NULL && m_dayShiftGridCtrl->GetSafeHwnd())
	{
		m_dayShiftGridCtrl->MoveWindow(0,0,cx,cy);
	}
	if (m_entityGrid.GetSafeHwnd())
	{
		if (m_wndTabs.GetSafeHwnd())
			m_wndTabs.MoveWindow(0,0,cx,20);
		m_entityGrid.MoveWindow(0,20,cx,cy - 20);
	}
	if (m_tblType == ID_GANTT_RES_GL)
	{
		GanttApi::OnSize(nType,cx,cy);
	}
	else if (m_tblType == ID_GANTT_ORDER_GL)
	{
		OrderGanttApi::OnSize(nType,cx,cy);
	}
	else if (m_tblType == ID_RATE_RES_GL)
	{
		ResRateGanttApi::OnSize(nType,cx,cy);
	}
	else if (m_tblType == ID_RATE_STA_GL)
	{
		StockGanttApi::OnSize(nType,cx,cy);
	}
	else if (m_tblType == ID_Item_Gather_GL)
	{
		ItemGatherApi::OnSize(nType,cx,cy);
	}
	else if (m_tblType ==  ID_BOM_VIEW_DLG)
	{
		m_bomViewDlg.MoveWindow(0,0,cx,cy);
	}
}

void CYKSchedulerView::OpenPage(stTable& table)
{
	CWnd * pWnd = new CWnd;
	pWnd->Create(NULL,table.GetName().c_str(),WS_CHILD | WS_VISIBLE, 
			CRect (0, 0, 0, 0), &m_wndTabs, 1000);

	m_wndTabs.InsertTab(pWnd,table.GetName().c_str(),-1);
	m_tabWndLst.push_back(pWnd);
}

void CYKSchedulerView::RemovePage()
{
	if (ID_GANTT_RES_GL == m_tblType || ID_GANTT_ORDER_GL == m_tblType ||
		ID_RATE_RES_GL == m_tblType || ID_RATE_STA_GL == m_tblType ||
		ID_Item_Gather_GL == m_tblType)
	{
		m_bIsTalbe = FALSE;
	}
	else
	{
		list<CWnd*>::iterator iter = m_tabWndLst.begin();
		for (; iter != m_tabWndLst.end(); iter++)
		{
			CWnd* pWnd = *iter;
			if (pWnd != NULL)
				delete pWnd;
		}
	}
}

void CYKSchedulerView::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class

	CView::PreSubclassWindow();
}

void CYKSchedulerView::SaveViewTip()
{
	wstring str;
	if (theApp.g_tblType == ID_GANTT_RES_GL)
	{
		str =GetRemarkFiled(ID_UI_TEXT_RESGANT).c_str();     //  L"资源甘特图"
	}
	else if (theApp.g_tblType == ID_GANTT_ORDER_GL)
	{
		str =GetRemarkFiled(ID_UI_TEXT_ORDERGANT).c_str();   //  L"订单甘特图"
	}
	else if (theApp.g_tblType == ID_RATE_RES_GL)
	{
		str =GetRemarkFiled(ID_UI_TEXT_RESONUSA).c_str();    //  L"资源负荷图"
	}
	else if (theApp.g_tblType == ID_RATE_STA_GL)
	{
		str =GetRemarkFiled(ID_UI_TEXT_STOREONUSA).c_str();  //  L"库存负荷图"
	}
	else if (theApp.g_tblType == ID_Item_Gather_GL)
	{
		str =GetRemarkFiled(ID_UI_TEXT_ITEM_GATHER_GL).c_str();  //  L"库存负荷图"
	}
	else
	{	
		LANGUAGEPACKAPI::GetTblNameByType(TblType(theApp.g_tblType),str);
	}

	ViewTip vt;
	vt.id = theApp.g_viewID++;
	m_viewId = vt.id;
	vt.name = str;
	vt.pWnd = this;
	vt.wndType = 1;
	vt.tblType = m_tblType;
	theApp.g_viewTips.Insert(vt);
	Node node;
	if (theApp.g_spNodes.Find(m_tblType,node))
	{
		theApp.g_spNodes.SetNodeView(node.showName,this);
	}
}

void CYKSchedulerView::ClearViewTip()
{
	theApp.g_viewTips.Delete(m_viewId);
//	Node node;
//	if (theApp.g_spNodes.Find(m_tblType,node))
	//{
		//theApp.g_spNodes.SetNodeView(node.showName,NULL);
		theApp.g_spNodes.ClearNodeWnd(m_tblType);
	//}
}

void CYKSchedulerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// TODO: Add your specialized code here and/or call the base class
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CYKSchedulerView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base 
	if (m_pGdCtrl.GetSafeHwnd())
	{
		m_pGdCtrl.Flash(theApp.m_isNeedInitData);
		m_pGdCtrl.ShowWindow(SW_SHOW);
	}

	if(theApp.g_tblType == ID_WORK_RESOURCE_GD)
	{
		m_pWorkSe.Flash();
		m_pWorkSe.ShowWindow(SW_SHOW);
	}

	if (theApp.g_tblType == TblBOM)
	{
		m_BOMInfo.Flash();
		m_BOMInfo.ShowWindow(SW_SHOW);
	}
}

void  CYKSchedulerView::UpdateActiveGrid(BOOL tblFlash)
{
	if (m_tblType <= 0/* || m_tblType == TblProduceIndication*/)
		return;
	
	// 先调整Grid的尺寸，后面Flash可能需重新加载数据。
	AdjustGridView();

	switch (m_tblType)
	{
	case TblBOM:
		UpdateGrid(1);
		m_BOMInfo.Flash(tblFlash);
		break;
	case TblProduceIndication:
		UpdateGrid(3);
		m_entityGrid.Flash();
		break;
	default:
		UpdateGrid();
		m_pGdCtrl.Flash(tblFlash);
		break;
	}
}

// 查找  查找到得数据ID已经放到 m_findRowKeyList中了
void CYKSchedulerView::Find( UINT findType )
{
	if (m_pGdCtrl.GetSafeHwnd())
	{
		m_pGdCtrl.InitSource(m_findRowKeyList);
		m_pGdCtrl.Flash();
		BIZAPI::SetTableFlashFlg(TRUE,m_tblType);
	}
	if (m_entityGrid.GetSafeHwnd())
	{
		m_entityGrid.Flash();
	}

}
// 设置新的stable到数据源中，页签改变使用
void CYKSchedulerView::UpdateGrid(short GridType)
{
	CString strTemp;
	CWnd* pWnd = m_wndTabs.GetTabWnd(m_wndTabs.GetActiveTab());
	if (!pWnd)
		return;

	pWnd->GetWindowText(strTemp);
	if (DATATABLAPI::BeginstTble(m_tblType))
	{
		for(; DATATABLAPI::NotEndstTble(); DATATABLAPI::StepstTble())
		{
			stTable& table = DATATABLAPI::GetstTble();
			if (StrCmp(table.GetName().c_str() , strTemp.GetString()) == 0)
			{
				switch (GridType)
				{
				case 1:
					m_BOMInfo.SetStTable(table);
					break;
				case 3:
					m_entityGrid.SetTblInfo(table);
					break;
				default:
					m_pGdCtrl.SetStTable(table);
				}
				break;
			}
		}
	}
}

LRESULT CYKSchedulerView::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	if(message == WM_MOUSE_DOWNS)
	{
		static_cast<CMainFrame*>(theApp.GetMainWnd())->MouseDown();
	}
	else if (message == WM_WORK_SELECT)
	{
		::SendMessage(theApp.g_hMainFrm,IDU_UPDATE_MODIFY_VIEW,wParam,lParam);
	}
	else if (message == WM_RES_SELECT)
	{
		::SendMessage(theApp.g_hMainFrm,IDU_UPDATE_MODIFY_VIEW,wParam,lParam);
	}
	else if (message == WM_DOUBLE_SELECT_WORK)
	{
		::SendMessage(theApp.g_hMainFrm,IDU_UPDATE_MODIFY_DLG,wParam,lParam);
		Flash();
	}
	else if (message == IDU_RESET_SCHEDULER_FLG)
	{
		g_schCtr = FALSE;
	}
	else if (message == WM_GANTT_FIND)
	{
		::SendMessage(theApp.g_hMainFrm,IDU_TREE_FIND,wParam,lParam);
	}
	else if (message == WM_WORK_SPLIT)
	{
		WorkSplit(wParam);
		Flash();
	}
	else if(message == WM_ORDER_SPLIT)
	{
		OrderSplit(wParam);
		Flash();
	}
	else if(message == WM_WORK_COMB)
	{
		WorkComb(wParam);
		Flash();
	}
	else if(message == WM_ORDER_COMB)
	{
		OrderComb(wParam);
		Flash();
	}
	else if (WM_GANTT_ACTIVE ==message)
	{
		theApp.OpenTblPage((UINT)wParam);
	}
	else if(_ACTIVITE_VIWE_ == message)
	{
		// 表格的页签变化  不重新读取数据  但有刷新数据(用现有的) 

		BOOL b = (wParam == 96) ? FALSE : TRUE;
		if(!b)		// 页签变化  需要清空错误数据
			ViewKillFocusDo();
		Flash(b);
		
	}
	else if(_CLEARGRID_ == message)
	{
		if(m_pGdCtrl.GetSafeHwnd())
			m_pGdCtrl.ClearTabNameIndex();

		if (m_BOMInfo.GetSafeHwnd())
			m_BOMInfo.ClearTabNameIndex();
		/*if(m_pGdCtrl.GetSafeHwnd())
			m_pGdCtrl.ClearFlexGrid();

		if (m_BOMInfo.GetSafeHwnd())
			m_BOMInfo.ClearFlexGrid();*/
	}
	else if ( IDU_FIND_MSG	== message) //接收对话框确定的消息
	{
		/*if(m_pGdCtrl.GetSafeHwnd())
			m_pGdCtrl.FindWordMsg();

		if (m_BOMInfo.GetSafeHwnd())
		{
			m_BOMInfo.FindWordMsg();
		}*/

		return TRUE;
	}
	else if (IDU_FIND_MSG2 == message) 	//接收对话框退出的消息
	{
		MSG *msg = (MSG*)wParam;	

	/*	if(m_pGdCtrl.GetSafeHwnd())
			m_pGdCtrl.ExitFindWord(msg);

		if (m_BOMInfo.GetSafeHwnd())
		{
			m_BOMInfo.ExitFindWord(msg);
		}*/
		return TRUE;
	}
	else if( IDU_REPLACE_ALL == message )//接收替换全部的消息
	{
		/*if(m_pGdCtrl.GetSafeHwnd())
			m_pGdCtrl.ReplaceWordAll();

		if (m_BOMInfo.GetSafeHwnd())
		{
			m_BOMInfo.ReplaceWordAll();
		}*/
		return TRUE;
	}
	else if ( IDU_REPLACE_NEXT == message )//接收替换一个得消息
	{
		/*if(m_pGdCtrl.GetSafeHwnd())
			m_pGdCtrl.ReplaceWord();

		if (m_BOMInfo.GetSafeHwnd())
		{
			m_BOMInfo.ReplaceWord();
		}
		::SendMessage(this->m_hWnd, IDU_FIND_MSG, -1, -1);*/	
		return TRUE;
	}
	else if (message == WM_CALENDAR_MGR)
	{
		MSG *msg = (MSG*)wParam;
		CCalendarMgrDlg dlg;
		dlg.SetId((YK_ULONG)msg->wParam);
		if (dlg.DoModal()== IDOK)
		{
			msg->message = IDOK;
			BOOL* pChangeFlg = (BOOL*)lParam;
			*pChangeFlg = dlg.GetDsChangeFlg();
			YK_WSTRING* pStr = (YK_WSTRING*)(msg->lParam);
			* pStr = TOOLS::JoinStr(dlg.GetDayShiftStrList());
		}
	}
	else if (message == IDU_COPYCELL)//复制单元格
	{
#ifdef LIMITFUNCTION
		if (BIZAPI::GetIsLimited())
			return FALSE;
#endif
		if (m_pGdCtrl.GetSafeHwnd())
		{
			m_pGdCtrl.CopyCell();
		}
	}
	else if (message == IDU_SORTCELL)//以序列方式填充
	{
#ifdef LIMITFUNCTION
		if (BIZAPI::GetIsLimited())
			return FALSE;
#endif
		if (m_pGdCtrl.GetSafeHwnd())
		{
			m_pGdCtrl.SortCell();
		}
	}
	else if (IDU_WORK_RELATION == message)
	{
		
		MSG *msg = (MSG*)wParam;
		if (msg->message == TblWorkAvbResource)
		{
			theApp.OpenTblPage(TblWorkAvbResource,FALSE);//打开指定表格
			::SendMessage(theApp.g_hMainFrm, IDU_WORK, (UINT)msg->wParam, 0);
		}
		if (msg->message == TblWorkRelation)
		{
			theApp.OpenTblPage(TblWorkRelation,FALSE);//打开指定表格
			::SendMessage(theApp.g_hMainFrm, IDU_WORK, (UINT)msg->wParam, 0);
		}
		
	}
	else if (message == WM_PI_COMMOND)
	{
		CPIShowDlg dlg;
		if (IDOK == dlg.DoModal())
		{
			MSG *msg = (MSG*)wParam;
			msg->time = dlg.GetStartTime();
			msg->lParam = dlg.GetSpanTime();
			msg->wParam = dlg.GetWorkStataFlg();
			msg->message = 1;
		}
	}
	//add 2010-12-8  甘特图查找
	else if (message ==  WM_FIND_WORK)
	{
		CGtFindDlg dlg;
		dlg.SetType(1);
		if (IDOK == dlg.DoModal())
		{
			GtFind( dlg.GetFindId(),1);
		}
	}
	else if (message ==  WM_FIND_RESOURCE)
	{
		CGtFindDlg dlg;
		dlg.SetType(2);
		if (IDOK == dlg.DoModal())
		{
			GtFind( dlg.GetFindId(),2);
		}
	}
	else if (message ==  WM_FIND_ORDER)
	{
		CGtFindDlg dlg;
		dlg.SetType(3);
		if (IDOK == dlg.DoModal())
		{
			GtFind( dlg.GetFindId(),2);
		}
	}

	if(message != WM_CONTEXTMENU)
		return CView::DefWindowProc(message, wParam, lParam);
	else 
		return NULL;
}

void CYKSchedulerView::OnSetFocus(CWnd* pOldWnd)
{
	CView::OnSetFocus(pOldWnd);
	//GanttCommonApi::SetFocus(m_tblType,pOldWnd);
}
void CYKSchedulerView::ViewKillFocusDo()
{
	m_pGdCtrl.ViewKillFocusDo();
	//m_BOMInfo.SetUnError();

}
void CYKSchedulerView::OnKillFocus(CWnd* pNewWnd)
{
	CView::OnKillFocus(pNewWnd);
	// TODO: Add your message handler code here
	//GanttCommonApi::KillFocus(m_tblType,pNewWnd);
	// 失去焦点的时候清空一下错误数据
	ViewKillFocusDo();
}

// 刷新视图  tblFlash=TRUE：不是页签变更刷新(需要重新取数据 并且刷新数据) 
// 否则：不重新读取数据  更改数据的stble就好了  add 2010-5-17 by ll
// tblFlash:是否全部数据刷新标志
// 默认 tblFlash=TRUE
void CYKSchedulerView::Flash(BOOL tblFlash)
{
	switch (m_tblType)
	{
	case ID_GANTT_RES_GL:
		GanttApi::FlashGantt();
		break;
	case ID_GANTT_ORDER_GL:
		OrderGanttApi::FlashGantt();
		break;
	case ID_RATE_RES_GL:
		ResRateGanttApi::FlashGantt();
		break;
	case ID_RATE_STA_GL:
		StockGanttApi::FlashGantt();
		break;
	case ID_Item_Gather_GL:
		ItemGatherApi::FlashGantt();
		break;
	case ID_WORK_RESOURCE_GD:
		m_pWorkSe.Flash();
		break;
#ifndef _REMOVE_ITEMGATHER_
	case TblItemGather:
		GetItemGatherGridCtrl()->Flash();
		break;
#endif
	case ID_DayShift_GD:
		GetDayShiftGridCtrl()->Flash();
		break;
	case ID_BOM_VIEW_DLG:
		m_bomViewDlg.Flash();
		break;
	default:
		UpdateActiveGrid(tblFlash);
	}

	GanttApi::HideMsgDlg();
}


void CYKSchedulerView::Close()
{
//	CWindowsStateOper::Delete( GetTblType() );

	RemovePage();
	ClearViewTip();
	if (m_tblType == ID_GANTT_RES_GL)
	{
		GanttApi::Clear();
	}
	else if (m_tblType == ID_GANTT_ORDER_GL)
	{
		OrderGanttApi::Clear();
	}
	else if (m_tblType == ID_RATE_RES_GL)
	{
		ResRateGanttApi::Clear();
	}
	//else if (m_tblType == TblWork)
	//{
	//	theApp.g_mapWork = m_TreeTbl.m_mapRow;
	//}
	//else if (m_tblType == TblOrder)
	//{
	//	theApp.g_mapOrder = m_TreeTbl.m_mapRow;
	//}
	else if (m_tblType == ID_Item_Gather_GL)
	{
		ItemGatherApi::Clear();
	}
	else if (m_tblType == ID_RATE_STA_GL)
	{
		StockGanttApi::Clear();
	}
	if (theApp.m_bOpen)
		::SendMessage(theApp.g_hMainFrm, _ACTIVITE_VIWE_,123,(long)GetParentFrame());

	SortConditionManager* sorMager=GetSortConditionManager();
	if (NULL != sorMager)
	{
		sorMager->DeleteConByIndex(m_tblType);
	}
}

void CYKSchedulerView::EnsureWorkInEnsureArea()
{
	if (m_tblType == ID_GANTT_RES_GL)
	{
		GanttApi::EnsureWorkInEnsureArea();
	}
	else
	{
		BIZAPI::DoWatchBoard();
		Flash();
	}
}
//是否可以复制
BOOL CYKSchedulerView::IsCanCopy()
{
	BOOL bOK = FALSE;

	if (m_pGdCtrl.GetSafeHwnd())
	{
		bOK = m_pGdCtrl.IsCanCopy();
	}
	/*if (m_tblType == ID_GANTT_RES_GL||m_tblType == ID_GANTT_ORDER_GL
		||m_tblType == ID_RATE_RES_GL||m_tblType == ID_RATE_STA_GL
		||m_tblType == ID_WORK_RESOURCE_GD||m_tblType == TblBOM
		||m_tblType == TblLoadFileInfo
		||m_tblType == TblWorkInputItem||m_tblType == TblWorkOutputItem
		||m_tblType == TblWorkAvbResource||m_tblType == TblRestrictInfo
		||m_tblType ==TblUseResource||m_tblType ==TblCategorySequence
		||m_tblType ==TblResourceSequence
		||m_tblType == ID_Item_Gather_GL)
	{
		bOK = FALSE;
	}
	else
	{
		if (m_pGdCtrl.GetSafeHwnd())
		{
			bOK = m_pGdCtrl.IsCanCopy();
		}
	}*/

	return bOK;
}

BOOL CYKSchedulerView::IsCanCut()
{
	BOOL bOK = FALSE;

	if (m_pGdCtrl.GetSafeHwnd())
	{
		bOK = m_pGdCtrl.IsCanCut();
	}
	/*if (m_tblType == ID_GANTT_RES_GL||m_tblType == ID_GANTT_ORDER_GL
		||m_tblType == ID_RATE_RES_GL||m_tblType == ID_RATE_STA_GL
		||m_tblType == ID_WORK_RESOURCE_GD||m_tblType == TblBOM
		||m_tblType == TblLoadFileInfo
		||m_tblType == TblWorkInputItem||m_tblType == TblWorkOutputItem
		||m_tblType == TblWorkAvbResource||m_tblType == TblRestrictInfo
		||m_tblType ==TblUseResource||m_tblType ==TblCategorySequence
		||m_tblType ==TblResourceSequence
		||m_tblType == ID_Item_Gather_GL)
	{
		bOK = FALSE;
	}
	else
	{
		if (m_pGdCtrl.GetSafeHwnd())
		{
			bOK = m_pGdCtrl.IsCanCut();
		}
	}*/

	return bOK;
}

//是否可以粘贴
BOOL CYKSchedulerView::IsCanPaste()
{
	BOOL bOK = FALSE;

	if (m_pGdCtrl.GetSafeHwnd())
		bOK = m_pGdCtrl.IsCanPaste();
	/*if (m_tblType == ID_GANTT_RES_GL||m_tblType == ID_GANTT_ORDER_GL
		||m_tblType == ID_RATE_RES_GL||m_tblType == ID_RATE_STA_GL
		||m_tblType == ID_WORK_RESOURCE_GD||m_tblType == TblBOM
		||m_tblType == TblLoadFileInfo
		||m_tblType == TblWorkInputItem||m_tblType == TblWorkOutputItem
		||m_tblType == TblWorkAvbResource||m_tblType == TblRestrictInfo
		||m_tblType ==TblUseResource||m_tblType ==TblCategorySequence
		||m_tblType ==TblResourceSequence||m_tblType == TblSchSysParam
		||m_tblType == ID_Item_Gather_GL)
	{
		bOK = FALSE;
	}
	else
	{
		if (m_pGdCtrl.GetSafeHwnd())
			bOK = m_pGdCtrl.IsCanPaste();
	}*/

	return bOK;
}

CDataGridCtrl* CYKSchedulerView::GetActiveGridCtrl()
{
	
	if (m_pGdCtrl.GetSafeHwnd())
	{
		return &m_pGdCtrl;
	}

	return NULL;
}

CBOMInfo* CYKSchedulerView::GetBOMGridCtrl()
{
	return &m_BOMInfo;
}

void CYKSchedulerView::SureWillOrder()
{
	int actTab = m_wndTabs.GetActiveTab();
	if (actTab >= 0)
	{
		CWnd* pWnd = m_wndTabs.GetTabWnd(actTab);
		if ( !pWnd ) return ;
		CDataGridCtrl* pGrid = DYNAMIC_DOWNCAST(CDataGridCtrl,pWnd);
		if (pGrid != NULL)
		{
			//pGrid->SureWillOrder();
		}
	}
}

void CYKSchedulerView::CancelWillOrder()
{
	int actTab = m_wndTabs.GetActiveTab();
	if (actTab >= 0)
	{
		CWnd* pWnd = m_wndTabs.GetTabWnd(actTab);
		if ( !pWnd ) return ;
		CDataGridCtrl* pGrid = DYNAMIC_DOWNCAST(CDataGridCtrl,pWnd);
		if (pGrid != NULL)
		{
		//	pGrid->CancelWillOrder();
		}
	}
}

void CYKSchedulerView::SetFindType( UINT nType )
{
	m_nFindType = nType;

	/*if (m_pGdCtrl.GetSafeHwnd())
		m_pGdCtrl.SetFindType(m_nFindType);*/
}

void CYKSchedulerView::GLFind( vector<UINT>& fList/*int kind,vector<UINT>& findList*/ )
{

	if (m_tblType==ID_GANTT_RES_GL)
	{
		GanttApi::GanttFilter(fList);
	}
	else if (m_tblType==ID_RATE_RES_GL)
	{
		ResRateGanttApi::GanttFilter(fList);
	}
	else if (m_tblType==ID_WORK_RESOURCE_GD)
	{
		m_pWorkSe.GanttFilter();
	}
#ifndef _REMOVE_ITEMGATHER_
	else if (m_tblType == TblItemGather)
	{
		GetItemGatherGridCtrl()->FilterRes();
	}
#endif
	else if (m_tblType==ID_GANTT_ORDER_GL)
	{
		OrderGanttApi::GanttFilter(fList);
	}
	else if (m_tblType==ID_RATE_STA_GL)
	{
		StockGanttApi::GanttFilter(fList);
	}
	else if (m_tblType == ID_Item_Gather_GL)
	{
		ItemGatherApi::GanttFilter(fList);
	}
}

int CYKSchedulerView::GetSelectFiled()
{
	int result = HI_HID_Default;
	if (m_tblType == ID_GANTT_RES_GL)
	{
		 return DoTypeHelpId(m_tblType);
	}
	else if (m_tblType == ID_GANTT_ORDER_GL)
	{
		 return DoTypeHelpId(m_tblType);
	}
	else if (m_tblType == ID_RATE_RES_GL)
	{
		 return DoTypeHelpId(m_tblType);;
	}
	else if (m_tblType == ID_RATE_STA_GL)
	{
		 return DoTypeHelpId(m_tblType);
	}
	else if (m_tblType == ID_WORK_RESOURCE_GD)
	{
		return DoTypeHelpId(m_tblType);
	}
	else if (m_tblType == ID_Item_Gather_GL)
	{
		return DoTypeHelpId(m_tblType);
	}
	else
	{
		CDataGridCtrl* pDgCtrl = GetActiveGridCtrl();
		if (pDgCtrl  != NULL)
		{
			int index = pDgCtrl->GetSelectFiled();
			if (index != -1)
			{
				YKFieldPtr fp = BIZAPI::GetBizMap()->GetFieldPara(pDgCtrl->GetTblType(),index);
				if (fp->m_helpId != -1)
				{
					return fp->m_helpId;
				}
				else
					return DoTypeHelpId(pDgCtrl->GetTblType());
			}
			else
				result = DoTypeHelpId(pDgCtrl->GetTblType());

		}
		else
			result = DoTypeHelpId(m_tblType);
	}
	return result;
}

int CYKSchedulerView::DoTypeHelpId( int tblType )
{
	YK_ULONG  TblEnmuId=HI_HID_Default;

	switch(m_tblType)
	{
	case TblQualityLevel:
		TblEnmuId = HI_HID_TblQulity;
		break;
	case  TblOrder:
		TblEnmuId = HI_HID_TblOrder;
		break;
	case  TblItem:
		TblEnmuId = HI_HID_TblItem;
		break;
	/*case  TblProductRouting  : 
		TblEnmuId = 	HI_HID_TblProductRouting;   */  
		break;
	case  TblOperation  :    
		TblEnmuId = 	HI_HID_TblOperation;     
		break;   
	/*case  TblOperInputItem  : 
		TblEnmuId = 	HI_HID_TblOperInputItem;     
		break;
	case  TblOperOutputItem  :  
		TblEnmuId = 	HI_HID_TblOperOutputItem;     
		break;
	case  TblOperAvbRes  :  
		TblEnmuId = 	HI_HID_TblOperAvbRes;    */ 
		break; 
	case  TblResource  : 
		TblEnmuId = 	HI_HID_TblResource;     
		break;
	case  TblResourceRestrict  : 
		TblEnmuId = 	HI_HID_TblResourceRestrict;     
		break;  
	case  TblProcess  :  
		TblEnmuId = 	HI_HID_TblProcess;     
		break;
	case  TblRouting  :   
		TblEnmuId = 	HI_HID_TblRouting;     
		break;
	case  TblClient  :  
		TblEnmuId = 	HI_HID_TblClient;     
		break;
	case  TblTransportTm  :  
		TblEnmuId = 	HI_HID_TblTransportTm;     
		break;
	case  TblSettingTm  : 
		TblEnmuId = 	HI_HID_TblSettingTm;  
		break;
	case  TblCalendar  :  
		TblEnmuId = 	HI_HID_TblCalendar;   
		break;
	case  TblDayShift  :  
		TblEnmuId = 	HI_HID_TblDayShift; 
		break;
	case  TblSkill  : 
		TblEnmuId = 	HI_HID_Default; 
		break;
	case  TblSpecItem  :   
		TblEnmuId = 	HI_HID_TblSpecItem;  
		break;
	case  TblWatchBoard  :
		TblEnmuId = 	HI_HID_TblWatchBoard;  
		break;
	case TblUseResource	:
		TblEnmuId = HI_HID_TblUseResource;
		break;
	case TblProduceIndication:
		TblEnmuId = HI_HID_TblProduceIndication;
		break;
	case TblCategorySequence:
		TblEnmuId = HI_HID_TblCategorySequence;
		break;
	case  TblResourceSequence  :   
		TblEnmuId = 	HI_HID_TblResourceSequence;  
		break;
	case  TblApsParam  :   
		TblEnmuId = 	HI_HID_TblApsParam;  
		break;
	case  TblDisobeyRestrict  :   
		TblEnmuId = 	HI_HID_Default;  
		break;
	case TblWork:
		TblEnmuId = 	HI_HID_TblWork;  
		break;
	case TblWorkInputItem:
		TblEnmuId = 	HI_HID_TblWorkInputItem;  
		break;
	case TblWorkOutputItem:
		TblEnmuId = 	HI_HID_TblWorkOutputItem;  
		break;
	case TblWorkAvbResource:
		TblEnmuId = 	HI_HID_TblWorkAvbResource;  
		break;
	case TblWorkRelation:
		TblEnmuId = 	HI_HID_TblWorkRelation;
		break;
	case  TblRestrictInfo  :		
		TblEnmuId = 	HI_HID_TblRestrictInfo;  
		break;
	case  TblGanttPara  :		
		TblEnmuId = 	HI_HID_Gantt_Param;  
		break;
	/*case   TblProcessLink  :
		TblEnmuId = 	HI_HID_TblProcessLink;*/
		break;
	case TblSchSysParam:
		TblEnmuId = HI_HID_TblSchSysParam;
		break;
	case TblBOM:
		TblEnmuId = HI_HID_TblBom;
		break;
	case ID_WORK_RESOURCE_GD:
		TblEnmuId = HI_HID_TblWorkSeq;
		break;
	case ID_GANTT_ORDER_GL:
		TblEnmuId = 	HI_HID_Order_Gantt;
		break;
	case ID_GANTT_RES_GL:
		TblEnmuId = 	HI_HID_Res_Gantt;
		break;
	case ID_RATE_RES_GL:
		TblEnmuId = 	HI_HID_Res_Rate;
		break;
	case ID_RATE_STA_GL:
		TblEnmuId = 	HI_HID_Sta_Rate;
		break;
	case ID_Item_Gather_GL:	
		TblEnmuId = HI_HID_Item_Gather_GL;
	case TblItemGather:
		TblEnmuId = HI_HID_Item_Gather_Grid;
		break;
	case TblCreateMTOrderParam:
		TblEnmuId = HI_HID_TblCMTOrderParam;
		break;
	case TblWorkChange:
		TblEnmuId = HI_HID_TblWorkChange;
		break;
	case TblLoadFileInfo:
		TblEnmuId = HI_HID_TblLoadFileInfo;
		break;
	default:
		return HI_HID_Default;
	}
	return TblEnmuId;
}





//工作拆分
void CYKSchedulerView::WorkSplit( ULONG workId )
{
#ifndef PMCMODE
	YKBizMap* bizMap =	BIZAPI::GetBizMap();
	if(bizMap == NULL)
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKSPLITFAIL).c_str());
		return;
	}

	YKWorkPtrMap * workMap = bizMap->GetYKWorkPtrMap();
	if(workMap == NULL )
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKSPLITFAIL).c_str());
		return;
	}

	YKWorkPtr workPtr = workMap->Get(workId);

	CWorkSplitDlg  dlgWorkSplit;


	dlgWorkSplit.SetWorkCode(workPtr->GetCode());
	dlgWorkSplit.SetTotalCount(workPtr->GetPlantAmount());
	dlgWorkSplit.SetTitleType(1);

	if(!workPtr->CanBeSplit())
	{
		if(workPtr->GetState()!= WorkState_StartPd)
		{
			AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKSPLITFAIL).c_str());
			return;
		}
	}

	if(dlgWorkSplit.DoModal() == IDOK)
	{
		BIZAPI::GetBizMap()->GetSchSysParam()->SetCurApsParam();

		if(dlgWorkSplit.m_firstCount > 0.00001
			&& dlgWorkSplit.m_firstCount < workPtr->GetPlantAmount())
		{
			BIZAPI::ManualWorkSplit(workId,dlgWorkSplit.m_firstCount);

			if(bizMap != NULL)
			{
				bizMap->RedrawAllGantt();
				BIZAPI::SetTableFlashFlg(TRUE,TblOrder);  
				BIZAPI::SetTableFlashFlg(TRUE,TblWork);   
				BIZAPI::SetTableFlashFlg(TRUE,TblWorkRelation);   
				BIZAPI::SetTableFlashFlg(TRUE,TblWorkAvbResource);  
				Flash();
			}
		}

	}
#endif
}

//订单拆分
void CYKSchedulerView::OrderSplit( ULONG workId )
{
#ifndef PMCMODE
	YKBizMap* bizMap =	BIZAPI::GetBizMap();
	if(bizMap == NULL)
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKSPLITFAIL).c_str());
		return;
	}

	YKWorkPtr& workPtr = bizMap->GetYKWorkPtrMap()->Get(workId);

	if(workPtr == NULL)
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKSPLITFAIL).c_str());
		return;
	}

	YKOrderPtr orderPtr = workPtr->GetOrder();
	if(orderPtr == NULL)	
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKSPLITFAIL).c_str());
		return;
	}

	if(!orderPtr->CanBeSplit())
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKSPLITFAIL).c_str());
		return;
	}

	CWorkSplitDlg  dlgWorkSplit;

	dlgWorkSplit.SetWorkCode(orderPtr->GetCode());
	dlgWorkSplit.SetTotalCount(orderPtr->GetAmount());
	dlgWorkSplit.SetTitleType(2);

	if(dlgWorkSplit.DoModal() == IDOK)
	{
		BIZAPI::GetBizMap()->GetSchSysParam()->SetCurApsParam();

		if(dlgWorkSplit.m_firstCount > 0.00001
			&& dlgWorkSplit.m_firstCount < orderPtr->GetAmount())
		{
			BIZAPI::ManualOrderSplit(orderPtr->GetId(),dlgWorkSplit.m_firstCount);

			bizMap->RedrawAllGantt();

			BIZAPI::SetTableFlashFlg(TRUE,TblWork);
			BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
			BIZAPI::SetTableFlashFlg(TRUE,TblWorkRelation);   
			BIZAPI::SetTableFlashFlg(TRUE,TblWorkAvbResource);  
            Flash();	
		}

	}
#endif
}

//工作合并
void CYKSchedulerView::WorkComb( ULONG workId )
{
#ifndef PMCMODE
	YKBizMap* bizMap =	BIZAPI::GetBizMap();
	if(bizMap == NULL)
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKCOMBFAIL).c_str());
		return;
	}

	YKWorkPtr& workPtr = bizMap->GetYKWorkPtrMap()->Get(workId);
	if(workPtr == NULL)
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKCOMBFAIL).c_str());
		return;
	}

	if(!workPtr->CanBeSplit())
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKCOMBFAIL).c_str());
		return;
	}

	WorkOrderCombDlg  dlgWorkSplit;
	
	dlgWorkSplit.SetType(1);
	dlgWorkSplit.SetSrcId(workId);

	if(dlgWorkSplit.DoModal() == IDOK)
	{
		BIZAPI::GetBizMap()->GetSchSysParam()->SetCurApsParam();

		list<YK_ULONG> workList = dlgWorkSplit.GetSelectIds();
		BIZAPI::ManualWorkComb(workId,workList);

		if(bizMap != NULL)
			bizMap->RedrawAllGantt();

		BIZAPI::SetTableFlashFlg(TRUE,TblOrder);  
		BIZAPI::SetTableFlashFlg(TRUE,TblWork);   
		BIZAPI::SetTableFlashFlg(TRUE,TblWorkRelation);   
		BIZAPI::SetTableFlashFlg(TRUE,TblWorkAvbResource);  
        Flash();	
	}
#endif
}

//订单合并
void CYKSchedulerView::OrderComb( ULONG workId )
{
#ifndef PMCMODE

	YKBizMap* bizMap =	BIZAPI::GetBizMap();
	if(bizMap == NULL)
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKCOMBFAIL).c_str());
		return;
	}

	YKWorkPtr& workPtr = bizMap->GetYKWorkPtrMap()->Get(workId);
	if(workPtr == NULL)
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKCOMBFAIL).c_str());
		return;
	}

	YKOrderPtr orderPtr = workPtr->GetOrder();
	if(orderPtr == NULL)	
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKCOMBFAIL).c_str());
		return;
	}

	if(!orderPtr->CanBeSplit())
	{
		AfxMessageBox(GetRemarkFiled(ID_UI_BOX_WORKCOMBFAIL).c_str());
		return;
	}

	WorkOrderCombDlg  dlgWorkSplit;
	dlgWorkSplit.SetType(2);
	dlgWorkSplit.SetSrcId(workId);

	if(dlgWorkSplit.DoModal() == IDOK)
	{
		BIZAPI::GetBizMap()->GetSchSysParam()->SetCurApsParam();

		list<YK_ULONG> orderList = dlgWorkSplit.GetSelectIds();

		BIZAPI::ManualOrderComb(dlgWorkSplit.GetSrcId(),orderList);

		if(bizMap != NULL)
			bizMap->RedrawAllGantt();


		BIZAPI::SetTableFlashFlg(TRUE,TblWork);
		BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
		BIZAPI::SetTableFlashFlg(TRUE,TblWorkRelation);   
		BIZAPI::SetTableFlashFlg(TRUE,TblWorkAvbResource);  
		Flash();	
	}
#endif
}

void CYKSchedulerView::UpdateEntiyGrid()
{
	UpdateGrid(3);
	//m_entityGrid.Flash();
}


void CYKSchedulerView::ResetView()
{
	ViewTip vt;
	if (theApp.g_viewTips.Find(m_tblType,vt))
		GetDocument()->SetTitle(GetRemarkFiled(m_tblType).c_str());
	else
		GetDocument()->SetTitle(L"");

	if (m_wndTabs.GetSafeHwnd())
	{
		int pageCount = m_wndTabs.GetTabsNum();

		for( int i = 0; i < pageCount; i++)
		{
			CString str = GetRemarkFiled(m_wndTabs.GetTabIndex(i)).c_str();
			m_wndTabs.SetTabLabel(i,str);
		}
	}
	Flash();
}

void CYKSchedulerView::GetWorkToUseResource( vector<GridRowInfo>& idList ,UINT workId)
{
	idList.clear();

	YKWorkPtrMap* workMap = BIZAPI::GetBizMap()->GetYKWorkPtrMap();
	if(workMap != NULL)
	{
		YKWorkPtr& workPtr = workMap->Get(workId);

		list<YKAvbResTempPtr>  workAvbResource = workPtr->GetYKAvbResTempPtr();
		for (list<YKAvbResTempPtr>::iterator iter= workAvbResource.begin();
			iter!= workAvbResource.end();iter++)
		{
			YKAvbResTempPtr&workAvbResourcePtr=*iter;
			if(workAvbResourcePtr != NULL)
			{
				GridRowInfo rowInfo;
				rowInfo.id=workAvbResourcePtr->GetId();
				rowInfo.tblType = TblWorkAvbResource;
				idList.push_back(rowInfo);
			}
		}

		list<YKAvbResTempPtr> worksecond = workPtr->GetYKSecondaryTempPtr();
		for (list<YKAvbResTempPtr>::iterator iter= worksecond.begin();
			iter!= worksecond.end();iter++)
		{
			YKAvbResTempPtr&worksecondPtr=*iter;
			if(worksecondPtr != NULL)
			{
				GridRowInfo rowInfo;
				rowInfo.id=worksecondPtr->GetId();
				rowInfo.tblType = TblWorkAvbResource;
				idList.push_back(rowInfo);
			}
		}


	}
}

//工作对应工作关联关系
void CYKSchedulerView::GetWorkToWorkRelation( vector<GridRowInfo>& idList ,UINT workId )
{
	idList.clear();

	//取工作表
	YKWorkPtrMap* workMap = BIZAPI::GetBizMap()->GetYKWorkPtrMap();
	if(workMap != NULL)
	{
		YKWorkPtr& workPtr = workMap->Get(workId);

		list<YKWorkRelationPtr> listWorkRelation;
		list<YKWorkRelationPtr> listPreRelation;

		workPtr->GetNextRelationPtrList(listWorkRelation);
		workPtr->GetPreRelationPtrList(listPreRelation);

		//后工作
		for (list<YKWorkRelationPtr>::iterator itr = listWorkRelation.begin(); itr != listWorkRelation.end(); itr++)
		{
			GridRowInfo rowInfo;
			rowInfo.id=(*itr)->GetId();
			rowInfo.tblType = TblWorkRelation;
			idList.push_back(rowInfo);
		}
		//前工作
		for (list<YKWorkRelationPtr>::iterator itr = listPreRelation.begin(); itr != listPreRelation.end(); itr++)
		{
			GridRowInfo rowInfo;
			rowInfo.id=(*itr)->GetId();
			rowInfo.tblType = TblWorkRelation;
			idList.push_back(rowInfo);
		}
	}
}

void CYKSchedulerView::AdjustGridView()
{
	m_showGridLable = theApp.m_gridShowLabel;
	int needH = 20;
	if(!m_showGridLable)
	{
		needH = 0;
	}
	CRect rect;
	GetClientRect(&rect);
	
	if (m_pGdCtrl.GetSafeHwnd())
	{
		if( m_wndTabs.GetSafeHwnd() )
			m_wndTabs.MoveWindow( 0, 0, rect.Width(), needH );
		m_pGdCtrl.MoveWindow( 0, needH, rect.Width(), rect.Height()-needH );
		if (m_pGdCtrl.GetClientRowCount() > 0 &&
			m_pGdCtrl.UpdateClientRowCount())
		{
			m_pGdCtrl.Flash();
		}
	}
	else if (m_BOMInfo.GetSafeHwnd())
	{
		if( m_wndTabs.GetSafeHwnd() )
			m_wndTabs.MoveWindow( 0, 0, rect.Width(), needH );
		m_BOMInfo.MoveWindow(0, needH, rect.Width(), rect.Height()-needH );
		if (m_BOMInfo.GetClientRowCount() > 0 &&
			m_BOMInfo.UpdateClientRowCount())
		{
			m_BOMInfo.Flash(FALSE);
		}
	}
}



void CYKSchedulerView::GtFind(unsigned long id,int type)
{
     if (m_tblType==ID_GANTT_RES_GL)
     {
		 vector<UINT> findList;
		 findList.push_back(id);
		 if (type==1)   //查找工作
		 {
			 GanttApi::Query(type,findList);
		 }
		 else    //查找资源
		 {
			 GanttApi::Query(type,findList);
		 }
     }
	 else if (m_tblType==ID_GANTT_ORDER_GL)
	 {
		 vector<UINT> findList;
		 findList.push_back(id);
		 //if (type==1) //查找工作
		 //{
			// YKBizMap*bizMap=BIZAPI::GetBizMap();
			// if (NULL!=bizMap)
			// {
			//	 YKWorkPtrMap*ptrMap=bizMap->GetYKWorkPtrMap();
			//	 if (NULL!=ptrMap)
			//	 {
			//		 YKWorkPtr ptr=ptrMap->Get(id);
			//		 if (NULL!=ptr)
			//		 {
			//			 findList.push_back(ptr->GetOrderId());
			//		 }
			//	 }
			// }
		 //}
		 //else
			// findList.push_back(id);
		 //GLFind( findList);
		 if (type==1) //查找工作
		 {
			 OrderGanttApi::Query(type,findList);
		 }
		 else   //查找订单
		 {
			 OrderGanttApi::Query(type,findList);
		 }
	 }
	 else if (m_tblType == ID_Item_Gather_GL)
	 {
		 vector<UINT> findList;
		 findList.push_back(id);
		 if (type==1)   //查找工作
		 {
			 ItemGatherApi::Query(type,findList);
		 }
		 else    //查找资源
		 {
			 ItemGatherApi::Query(type,findList);
		 }
	 }
}

#ifndef _REMOVE_ITEMGATHER_
void CYKSchedulerView::ItemGatherGridCtrlRelease()
{
	if (m_itemGtherGridCtrl != NULL)
	{
		delete m_itemGtherGridCtrl;
		m_itemGtherGridCtrl = NULL;
	}

}

CItemGatherGridCtrl* CYKSchedulerView::GetItemGatherGridCtrl()
{
	if (m_itemGtherGridCtrl == NULL)
		m_itemGtherGridCtrl = new CItemGatherGridCtrl();
	return m_itemGtherGridCtrl;
}
#endif
void CYKSchedulerView::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	Close();
	CView::OnClose();
}

CDayShiftGridCtrl* CYKSchedulerView::GetDayShiftGridCtrl()
{
	if (m_dayShiftGridCtrl == NULL)
		m_dayShiftGridCtrl = new CDayShiftGridCtrl();
	return m_dayShiftGridCtrl;
}

void CYKSchedulerView::DayShiftGridCtrlRelease()
{
	
	if (m_dayShiftGridCtrl != NULL)
	{
		delete m_dayShiftGridCtrl;
		m_dayShiftGridCtrl = NULL;
	}
}

void CYKSchedulerView::FlexGridUpdate(bool bFlashed)
{
	if (m_pGdCtrl.GetSafeHwnd())
		m_pGdCtrl.UpdateFlexGrid(bFlashed);
}
void CYKSchedulerView::FlexGridSort()
{
	if (m_pGdCtrl.GetSafeHwnd())
		m_pGdCtrl.SortTable();
}
void CYKSchedulerView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: Add your specialized code here and/or call the base class
	if(ID_GANTT_RES_GL == GetTblType() ) 
	{
		GanttApi::PrintGantt(pDC,pInfo,50, 50, 
			pInfo->m_rectDraw.Width() - 50, 
			pInfo->m_rectDraw.Height() - 50 );
	}
	//if(IsGanttOpen(ID_GANTT_ORDER_GL)) 
	//{
	//	OrderGanttApi::PrintGantt();
	//	return;
	//}
	//if(IsGanttOpen(ID_RATE_RES_GL)) 
	//{
	//	ResRateGanttApi::PrintGantt();
	//	return;
	//}
	//if(IsGanttOpen(ID_RATE_STA_GL)) 
	//{
	//	StockGanttApi::PrintGantt();
	//	return;
	//}
	//if (IsGanttOpen(ID_Item_Gather_GL))
	//{
	//	ItemGatherApi::PrintGantt();
	//	return;
	//}


	//表格打印质量不好，暂时去掉
	//CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	//if (NULL != pCurView)
	//{
	//	if(pCurView->m_pGdCtrl.GetSafeHwnd())
	//		pCurView->m_pGdCtrl.Print();

	//	if (pCurView->m_pWorkSe.GetSafeHwnd())
	//	{
	//		pCurView->m_pWorkSe.Print();
	//	}
	//	else if (pCurView->GetTblType() == TblItemGather)
	//	{
	//		pCurView->GetItemGatherGridCtrl()->Print();
	//	}
	//}

	CView::OnPrint(pDC, pInfo);
}
