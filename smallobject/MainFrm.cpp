// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "YKScheduler.h"

#include "MainFrm.h"
#include "ComFunc.h"
#include "YKSchedulerDoc.h"
#include "YKSchedulerView.h"
#include "BCGPToolbarComboBoxButton.h"

#include "Biz_API.h"
#include "GanttApi.h"
#include "NewFilter.h"
#include "LanguagePack.h"
#include "FindDlg.h"
#include "EnumDef.h"
#include "WorkSequence.h"
#include <windows.h>
#include <process.h>    /* _beginthread, _endthread */
#include <stddef.h>
#include <stdlib.h>
#include <conio.h>
#include "PopMenuMgr.h"
#include "SortDlg.h"
#include "CustomizeButton.h"
#include "NewRuleDlg.h"
#include "APSMethodAddDlg.h"
#include "RgbDef.h"
#include "WindowsStateOper.h"
#include "ThreadMsgDef.h"
#include "SplashScreenEx.h"
#include "ProcessBarThread.h"

#define  INTZERO(X) ((X)<0?0:(X))

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define  MSG_BLANK   L"    "
#define  MSG_RUN_SCHULDER    WM_USER + 20001
int g_nProcess(0);
CSchProcessDlg* g_pProDlg;

#ifdef _CLOCK_DEBUG
#include "clock.h"

#define  CLOCK_PASE(tm)  tm.Pause();
#else
#define  CLOCK_PASE(tm)  
#endif
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CBCGPMDIFrameWnd)

const int  iMaxUserToolbars		= 10;
const UINT uiFirstUserToolBarId	= AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId	= uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CBCGPMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_WINDOW_MANAGER, OnWindowManager)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, OnViewCustomize)
	ON_REGISTERED_MESSAGE(BCGM_RESETTOOLBAR, OnToolbarReset)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_2000, ID_VIEW_APPLOOK_VS2008, OnAppLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_2000, ID_VIEW_APPLOOK_VS2008, OnUpdateAppLook)
	ON_COMMAND(ID_MDI_MOVE_TO_NEXT_GROUP, OnMdiMoveToNextGroup)
	ON_COMMAND(ID_MDI_MOVE_TO_PREV_GROUP, OnMdiMoveToPrevGroup)
	ON_COMMAND(ID_MDI_NEW_HORZ_TAB_GROUP, OnMdiNewHorzTabGroup)
	ON_COMMAND(ID_MDI_NEW_VERT_GROUP, OnMdiNewVertGroup)
	ON_COMMAND(ID_MDI_CANCEL, OnMdiCancel)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CMainFrame::OnUpdateFileOpen)
	ON_REGISTERED_MESSAGE(BCGM_ON_GET_TAB_TOOLTIP, OnGetTabToolTip)
	
	ON_COMMAND(ID_FILE_CLOSE, &CMainFrame::OnFileClose)
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_YK_ORDER_CAL, &CMainFrame::OnYkOrderCal)
	ON_COMMAND(ID_YK_SCHEDULER, &CMainFrame::OnYkScheduler)
	ON_COMMAND(ID_YK_DELCOM, &CMainFrame::OnYKDelCom)   // ADD
	ON_COMMAND(ID_YK_RES_GANTT, &CMainFrame::OnYkResGantt)
	ON_COMMAND(ID_YK_ORDER_GANTT, &CMainFrame::OnYkOrderGantt)
	ON_COMMAND(ID_YK_RES_RATE, &CMainFrame::OnYkResRate)
	ON_COMMAND(ID_YK_STAG_RATE, &CMainFrame::OnYkStagRate)

	ON_COMMAND(IDM_ITEM_GATHER_GL, &CMainFrame::OnYkItemGather_GL)
	ON_COMMAND(IDM_ITEM_GATHER_GRID, &CMainFrame::OnYkItemGather_Grid)
	ON_COMMAND(IDM_DAYSHIFT_GD, &CMainFrame::OnYkDayShift_Grid)

	ON_COMMAND(ID_QUERYS, &CMainFrame::OnQuerys)
	ON_UPDATE_COMMAND_UI(ID_YK_OBJ_WIN, &CMainFrame::OnUpdateYkObjWin)
	ON_UPDATE_COMMAND_UI(ID_YK_PRO_WIN, &CMainFrame::OnUpdateYkProWin)
	ON_UPDATE_COMMAND_UI(ID_YK_INFO_WIN, &CMainFrame::OnUpdateYkInfoWin)
	ON_COMMAND(ID_YK_OBJ_WIN, &CMainFrame::OnYkObjWin)
	ON_COMMAND(ID_YK_PRO_WIN, &CMainFrame::OnYkProWin)
	ON_COMMAND(ID_YK_INFO_WIN, &CMainFrame::OnYkInfoWin)
	ON_COMMAND(ID_TOOL_FILTER, &CMainFrame::OnYKFilter)
	ON_COMMAND(ID_TOOL_UNFILTER, &CMainFrame::OnYKUnFilter)
	
	ON_UPDATE_COMMAND_UI(ID_Menu, &CMainFrame::OnUpdateMenu)
	ON_UPDATE_COMMAND_UI(ID_YK_SCHPARM_SET, &CMainFrame::OnUpdateYkSchparmSet)
	ON_COMMAND(ID_Menu, &CMainFrame::OnMenu)
	ON_COMMAND(ID_YK_SCHPARM_SET, &CMainFrame::OnYkSchparmSet)
	ON_UPDATE_COMMAND_UI(ID_YK_ORDER_CAL, &CMainFrame::OnUpdateYkOrderCal)

	ON_COMMAND(ID_INIT_RESOURCE,&CMainFrame::OnInitResource)

	ON_COMMAND(ID_SLECT_APS_COMBO, &CMainFrame::OnSelectApsComboBox)
	ON_UPDATE_COMMAND_UI(ID_SLECT_APS_COMBO, &CMainFrame::OnUpdateSelectApsComboBox)
	ON_CBN_SELENDOK(ID_SLECT_APS_COMBO, &CMainFrame::OnSelectApsComboBox)
	ON_CBN_SETFOCUS(ID_SLECT_APS_COMBO, &CMainFrame::OnReplaceApsCombo)
	ON_COMMAND(ID_AUTO_ENSURE_WORK, &CMainFrame::OnAutoEnsureWork)
	ON_UPDATE_COMMAND_UI(ID_YK_DELCOM, &CMainFrame::OnUpdateDelCom)

	ON_COMMAND(ID_BUTTON_TOGETHOR_RUN, &CMainFrame::OnTogethorRun)

	ON_COMMAND(ID_SCHE_MULTI, &CMainFrame::OnMultiSch)

	ON_COMMAND(ID_STOP_MULTISCHE, &CMainFrame::OnStopMultiSch)
	ON_COMMAND(ID_EDIT_CUT,&CMainFrame::OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT,&CMainFrame::OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_COPY,&CMainFrame::OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY,&CMainFrame::OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE,&CMainFrame::OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE,&CMainFrame::OnUpdateEditPaste)
	ON_COMMAND(ID_YK_CHINESE, &CMainFrame::OnYkChinese)
	ON_COMMAND(ID_YK_ENGLISH, &CMainFrame::OnYkEnglish)
	ON_UPDATE_COMMAND_UI(ID_YK_CHINESE, &CMainFrame::OnUpdateYkChinese)
	ON_UPDATE_COMMAND_UI(ID_YK_ENGLISH, &CMainFrame::OnUpdateYkEnglish)
	ON_COMMAND(ID_CUSTOMIZEFILTER, &CMainFrame::OnCustomizeFilter)
	ON_UPDATE_COMMAND_UI(ID_CUSTOMIZEFILTER, &CMainFrame::OnUpdateCustomizefilter)
	ON_COMMAND(ID_CUSTOMIZESORT, &CMainFrame::OnCustomizesort)
	ON_UPDATE_COMMAND_UI(ID_CUSTOMIZESORT, &CMainFrame::OnUpdateCustomizesort)
	ON_COMMAND(ID_Palette_SetUp, &CMainFrame::OnPaletteSetup)
	ON_UPDATE_COMMAND_UI(ID_Palette_SetUp, &CMainFrame::OnUpdatePaletteSetup)
	ON_COMMAND(ID_ConflictSolve, &CMainFrame::OnConflictsolve)
	ON_UPDATE_COMMAND_UI(ID_ConflictSolve, &CMainFrame::OnUpdateConflictsolve)
	ON_MESSAGE(UM_FLEXGRIDUPDATE, &CMainFrame::OnFlexGridUpdate)
	ON_MESSAGE(UM_FLEXGRIDTHREAD, &CMainFrame::OnFlexGridThread)
	ON_MESSAGE(UM_SBPROGRESSRANGE, &CMainFrame::OnSBProgressRange)
	ON_MESSAGE(UM_SBPROGRESSPOS, &CMainFrame::OnSBProgressPos)
	ON_MESSAGE(UM_SBPROGRESSCANCEL, &CMainFrame::OnSBProgressCancel)

	ON_COMMAND(ID_SHOUCE, &CMainFrame::OnShouce)
	ON_COMMAND(ID_VCRPDF, &CMainFrame::OnVcrpdf)

	ON_UPDATE_COMMAND_UI(ID_VCRPDF, &CMainFrame::OnUpdateVcrpdf)
	ON_UPDATE_COMMAND_UI(ID_SHOUCE, &CMainFrame::OnUpdateShouce)
	ON_WM_SIZE()
ON_WM_PAINT()
//ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_PROGRESS,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

CMainFrame::CMainFrame()
{
#ifdef _OCXPACK
	m_nAppLook = ID_VIEW_APPLOOK_2003;
#else
	m_nAppLook = theApp.GetInt (_T("ApplicationLook"), ID_VIEW_APPLOOK_2003);
#endif
	m_flashDataFlag=TRUE;
	m_imageId	= 0;
	m_bFGThread = FALSE;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBCGPMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	OnAppLook (m_nAppLook);

	if (CBCGPToolBar::GetUserImages () == NULL)
	{
		// Load toolbar user images:
		if (!m_UserImages.Load (_T(".\\UserImages.bmp")))
		{
			TRACE(_T("Failed to load user images\n"));
		}
		else
		{
			CBCGPToolBar::SetUserImages (&m_UserImages);
		}
	}
	

	CBCGPToolBar::EnableQuickCustomization ();


	// TODO: Define your own basic commands. Be sure, that each pulldown 
	// menu have at least one basic command.

	CList<UINT, UINT>	lstBasicCommands;

	lstBasicCommands.AddTail (ID_VIEW_TOOLBARS);
	lstBasicCommands.AddTail (ID_FILE_NEW);
	lstBasicCommands.AddTail (ID_FILE_OPEN);
	lstBasicCommands.AddTail (ID_FILE_SAVE);
	lstBasicCommands.AddTail (ID_FILE_PRINT);
	lstBasicCommands.AddTail (ID_APP_EXIT);
	lstBasicCommands.AddTail (ID_EDIT_CUT);
	lstBasicCommands.AddTail (ID_EDIT_PASTE);
	lstBasicCommands.AddTail (ID_EDIT_UNDO);
	lstBasicCommands.AddTail (ID_RECORD_NEXT);
	lstBasicCommands.AddTail (ID_RECORD_LAST);
	lstBasicCommands.AddTail (ID_APP_ABOUT);
	lstBasicCommands.AddTail (ID_VIEW_TOOLBAR);
	lstBasicCommands.AddTail (ID_VIEW_CUSTOMIZE);
	lstBasicCommands.AddTail (ID_WINDOW_TILE_HORZ);
	lstBasicCommands.AddTail (ID_VIEW_APPLOOK_2000);
	lstBasicCommands.AddTail (ID_VIEW_APPLOOK_XP);
	lstBasicCommands.AddTail (ID_VIEW_APPLOOK_2003);
	lstBasicCommands.AddTail (ID_VIEW_APPLOOK_2007);
	lstBasicCommands.AddTail (ID_VIEW_APPLOOK_VS2005);
	lstBasicCommands.AddTail (ID_VIEW_APPLOOK_WIN_XP);
	lstBasicCommands.AddTail (ID_VIEW_APPLOOK_2007_1);
	lstBasicCommands.AddTail (ID_VIEW_APPLOOK_2007_2);
    lstBasicCommands.AddTail (ID_VIEW_APPLOOK_2007_3);
	lstBasicCommands.AddTail (ID_VIEW_APPLOOK_VS2008);

	CBCGPToolBar::SetBasicCommands (lstBasicCommands);

	if (!m_wndMenuBar.Create (this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}
	//LPLOGFONT lf;
	//lf->
	LOGFONT		logFont;
	memset(&logFont, 0, sizeof(LOGFONT));
	YK_WSTRING strfontName = _T("Times New Roman");
	_tcscpy_s(logFont.lfFaceName,strfontName.size()+1,strfontName.c_str());

	// Add Focus altered values if required
	// Init LOGFONT
	logFont.lfWeight=FW_NORMAL;
	logFont.lfItalic=FALSE;
	logFont.lfUnderline=FALSE;
	logFont.lfStrikeOut=FALSE;
	logFont.lfHeight= 14;
	//-----------------------------------
	// Create font 
	//-----------------------------------
	logFont.lfCharSet=DEFAULT_CHARSET;
	logFont.lfOutPrecision=OUT_DEFAULT_PRECIS;
	logFont.lfClipPrecision=CLIP_DEFAULT_PRECIS;
	logFont.lfQuality=ANTIALIASED_QUALITY;
	logFont.lfPitchAndFamily=DEFAULT_PITCH;
	CBCGPMenuBar::SetMenuFont(&logFont);

	m_wndMenuBar.SetBarStyle(m_wndMenuBar.GetBarStyle() | CBRS_SIZE_DYNAMIC);
	//return TRUE;
	//SetMenu(&tempMenu);
	// Detect color depth. 256 color toolbars can be used in the
	// high or true color modes only (bits per pixel is > 8):
	CClientDC dc (this);
	BOOL bIsHighColor = dc.GetDeviceCaps (BITSPIXEL) > 8;

	UINT uiToolbarHotID = bIsHighColor ? IDB_TOOLBAR256 : 0;

	DWORD toolCtrlId = AFX_IDW_DIALOGBAR + 50;
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME1, 0, 0, FALSE, IDB_TOOLBAR256_2, 0, uiToolbarHotID))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_calToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,CRect(1,1,1,1),++toolCtrlId) ||
		!m_calToolBar.LoadToolBar(IDR_TOOLBAR_CAL, 0, 0, FALSE, IDB_CAL_TOOLBAR_2, 0, IDB_CAL_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}


	if (!m_schToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,CRect(1,1,1,1),++toolCtrlId) ||
		!m_schToolBar.LoadToolBar(IDR_TOOLBAR_SCH, 0, 0, FALSE, IDB_SCH_TOOLBAR_2, 0, IDB_SCH_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CBCGPToolbarComboBoxButton comboButton (ID_SLECT_APS_COMBO, 
		CImageHash::GetImageOfCommand (ID_SLECT_APS_COMBO, FALSE),
		CBS_DROPDOWNLIST);

	comboButton.AddItem (_T(""));
	comboButton.SelectItem (0);
	m_calToolBar.ReplaceButton (ID_SLECT_APS_COMBO,	comboButton);

	if (!m_filterToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,CRect(1,1,1,1),++toolCtrlId) ||
		!m_filterToolBar.LoadToolBar(IDR_TOOLBAR_filter, 0, 0, FALSE, IDB_BITMAP_FILTER2, 0, IDB_BITMAP_FILTER1))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}


	if (!m_ganttToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,CRect(1,1,1,1),++toolCtrlId) ||
		!m_ganttToolBar.LoadToolBar(IDR_TOOLBAR_GANTT, 0, 0, FALSE, IDB_GANTT_TOOLBAR_2, 0, IDB_GANTT_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}


	if (!m_dispatchToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,CRect(1,1,1,1),++toolCtrlId) ||
		!m_dispatchToolBar.LoadToolBar(IDR_TOOLBAR_DISPATCH, 0, 0, FALSE, IDB_BITMAP_DIAPATCH2, 0, IDB_BITMAP_DIAPATCH))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}


	++toolCtrlId;

#ifndef PMCMODE
	if (!m_netToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,CRect(1,1,1,1),++toolCtrlId) ||
		!m_netToolBar.LoadToolBar(IDR_TOOLBAR_NET, 0, 0, FALSE, IDB_NET_TOOLBAR_2, 0, IDB_NET_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
#endif

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	CString strStatusBar;
	strStatusBar.LoadString(AFX_IDS_IDLEMESSAGE);
	theApp.SetStatusBarText(strStatusBar);
	// Load control bar icons:
	CBCGPToolBarImages imagesWorkspace;
	imagesWorkspace.SetImageSize (CSize (16, 16));
	imagesWorkspace.SetTransparentColor (COLOR_BCGTBar_Image/*RGB (255, 0, 255)*/);
	imagesWorkspace.Load (IDB_WORKSPACE);
	if (GetSafeHwnd())
		m_wndWorkSpace.SetMainFrm(GetSafeHwnd());
	if (!m_wndWorkSpace.Create (GetRemarkFiled(ID_UI_TEXT_FUNCTIONVIEW).c_str(), this, CRect (0, 0,200, 200),
		TRUE, ID_VIEW_WORKSPACE,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI)) // L"功能菜单"
	{
		TRACE0("Failed to create Workspace bar\n");
		return -1;      // fail to create
	}

	m_wndWorkSpace.SetIcon (imagesWorkspace.ExtractIcon (0), FALSE);

	//读入备用Icon资源
	m_spareIcon.LoadToolBar(IDR_TOOLBAR_SPARE, 0, 0, FALSE, IDB_SPARE_NUMBER, 0, IDB_SPARE_NUMBER);


    // L"属性窗口"
	if (!m_wndWatchBar.Create (GetRemarkFiled(ID_UI_TEXT_PROPERTYWND).c_str(), this, CRect (0, 0, 500, 150),
		TRUE, 
		ID_VIEW_WATCH,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create watch bar\n");
		return FALSE;      // fail to create
	}
    // L"消息窗口"
	if (!m_wndOutput.Create (GetRemarkFiled(ID_UI_TEXT_MESSAGEWND).c_str(), this, CRect (0, 0, 200, 150),
		TRUE, 
		ID_VIEW_OUTPUT,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create output bar\n");
		return FALSE;      // fail to create
	}
	
	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndWorkSpace.EnableDocking(CBRS_ALIGN_ANY);

	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	m_wndWatchBar.EnableDocking(CBRS_ALIGN_ANY);

	m_calToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_schToolBar.EnableDocking(CBRS_ALIGN_ANY);	
	m_ganttToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_filterToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_dispatchToolBar.EnableDocking(CBRS_ALIGN_ANY);
	//临时删除一起生产控件
#ifndef PMCMODE
	m_netToolBar.EnableDocking(CBRS_ALIGN_ANY);
#endif
	EnableDocking(CBRS_ALIGN_ANY);
	EnableAutoHideBars(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndMenuBar);
	DockControlBar(&m_wndToolBar);
	DockControlBar(&m_calToolBar);	
	DockControlBar(&m_ganttToolBar);
	DockControlBar(&m_schToolBar);
	DockControlBar(&m_filterToolBar);
	DockControlBar(&m_dispatchToolBar);

#ifndef PMCMODE
	//临时删除一起生产控件
	DockControlBar(&m_netToolBar);
#endif
	DockControlBar(&m_wndWorkSpace);
	DockControlBar(&m_wndOutput);
	DockControlBar(&m_wndWatchBar);

	////临时删除一起生产控件
#ifndef PMCMODE
	DockControlBarLeftOf(&m_schToolBar,&m_netToolBar);
#endif
	DockControlBarLeftOf(&m_dispatchToolBar,&m_schToolBar);
	DockControlBarLeftOf(&m_ganttToolBar,&m_dispatchToolBar);
	DockControlBarLeftOf(&m_calToolBar,&m_ganttToolBar);
	DockControlBarLeftOf(&m_filterToolBar,&m_calToolBar);
	DockControlBarLeftOf(&m_wndToolBar,&m_filterToolBar);

#ifndef PMCMODE
	m_dispatchToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, GetRemarkFiled(ID_UI_TEXT_SELFDEFINE).c_str());		// _T("自定义...")
	m_wndToolBar.EnableCustomizeButton (TRUE, ID_VIEW_CUSTOMIZE, GetRemarkFiled(ID_UI_TEXT_SELFDEFINE).c_str());		// _T("自定义...")
	m_calToolBar.EnableCustomizeButton (TRUE, ID_VIEW_CUSTOMIZE, GetRemarkFiled(ID_UI_TEXT_SELFDEFINE).c_str());		// _T("自定义...")
	m_ganttToolBar.EnableCustomizeButton (TRUE, ID_VIEW_CUSTOMIZE, GetRemarkFiled(ID_UI_TEXT_SELFDEFINE).c_str());		// _T("自定义...")
	m_schToolBar.EnableCustomizeButton (TRUE, ID_VIEW_CUSTOMIZE, GetRemarkFiled(ID_UI_TEXT_SELFDEFINE).c_str());		// _T("自定义...")
	m_filterToolBar.EnableCustomizeButton (TRUE, ID_VIEW_CUSTOMIZE,GetRemarkFiled(ID_UI_TEXT_SELFDEFINE).c_str());		// _T("自定义...")

	m_netToolBar.EnableCustomizeButton (TRUE, ID_VIEW_CUSTOMIZE,GetRemarkFiled(ID_UI_TEXT_SELFDEFINE).c_str());		
#endif

	//重置工具名字
	RestToolsText();

	// Allow user-defined toolbars operations:
	InitUserToobars (NULL,
					uiFirstUserToolBarId,
					uiLastUserToolBarId);

	// Enable windows manager:
	EnableWindowsDialog (ID_WINDOW_MANAGER, IDS_WINDOWS_MANAGER, TRUE);

	// Enable control bar context menu (list of bars + customize command):
	EnableControlBarMenu (	
		TRUE,				// Enable
		ID_VIEW_CUSTOMIZE, 	// Customize command ID
		GetRemarkFiled(ID_UI_TEXT_SELFDEFINE).c_str(),	// Customize command text // _T("自定义...")
		ID_VIEW_TOOLBARS);	// Menu items with this ID will be replaced by
							// toolbars menu

	if (GetSafeHwnd())
	 theApp.g_hMainFrm = GetSafeHwnd();

	CreateSBProgress();
	InitApsComboBox();

	m_wndOutput.ShowWindow(FALSE);
	m_wndWorkSpace.ShowWindow(FALSE);
	m_wndWatchBar.ShowWindow(FALSE);

#ifdef PMCMODE
	ChangeMenuType(IDR_MAINFRAME_TEST);
	SetWindowText(GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str());

	//APS教学版，去掉启动图片
//	CSplashScreenEx::Show(this, IDB_BITMAP_YKSPLASH, 3000);
#else
	ChangeMenuType(IDR_MAINFRAME);
	SetWindowText(GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str());
#endif

	return 0;
}



//BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
//{
//	if( !CBCGPMDIFrameWnd::PreCreateWindow(cs) )
//		return FALSE;
//
//	return TRUE;
//}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CBCGPMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CBCGPMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers




void CMainFrame::OnViewCustomize()
{
	//------------------------------------
	// Create a customize toolbars dialog:
	//------------------------------------
	CBCGPToolbarCustomize* pDlgCust = new CBCGPToolbarCustomize (this,
		TRUE /* Automatic menus scaning */);

	pDlgCust->EnableUserDefinedToolbars ();
	pDlgCust->Create ();
}

afx_msg LRESULT CMainFrame::OnToolbarReset(WPARAM wp,LPARAM)
{
	// TODO: reset toolbar with id = (UINT) wp to its initial state:
	//
	 UINT uiToolBarId = (UINT) wp;
	 if (uiToolBarId == IDR_TOOLBAR_CAL)
	 {
		 CBCGPToolbarComboBoxButton comboButton (ID_SLECT_APS_COMBO, 
			 CImageHash::GetImageOfCommand (ID_SLECT_APS_COMBO, FALSE),
			 CBS_DROPDOWNLIST);
		 //list<CString> codeList;
		 comboButton.AddItem (_T(""));
		 //comboButton.AddItem (_T("Win32 Release"));
		 comboButton.SelectItem (0);

		 m_calToolBar.ReplaceButton (ID_SLECT_APS_COMBO,
			 comboButton);
		 InitApsComboBox();
	 }

	return 0;
}

void CMainFrame::OnWindowManager() 
{
	ShowWindowsDialog ();
}

void CMainFrame::OnAppLook(UINT id)
{
	CBCGPDockManager::SetDockMode (BCGP_DT_SMART);

	m_nAppLook = id;

	CBCGPTabbedControlBar::m_StyleTabWnd = CBCGPTabWnd::STYLE_3D;

	CBCGPMDITabParams mdiTabParams;
	mdiTabParams.m_bTabIcons = TRUE;
	mdiTabParams.m_bAutoColor = FALSE;
	mdiTabParams.m_style = CBCGPTabWnd::STYLE_3D_SCROLLED;

	switch (m_nAppLook)
	{
	case ID_VIEW_APPLOOK_2000:
		// enable Office 2000 look:
		CBCGPVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGPVisualManager));
		break;

	case ID_VIEW_APPLOOK_XP:
		// enable Office XP look:
		CBCGPVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGPVisualManagerXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		// enable Windows XP look (in other OS Office XP look will be used):
		CBCGPWinXPVisualManager::m_b3DTabsXPTheme = TRUE;
		CBCGPVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGPWinXPVisualManager));
		break;

	case ID_VIEW_APPLOOK_2003:
		// enable Office 2003 look:
		CBCGPVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGPVisualManager2003));

		mdiTabParams.m_style = CBCGPTabWnd::STYLE_3D_VS2005;
		mdiTabParams.m_bAutoColor = TRUE;
		CBCGPDockManager::SetDockMode (BCGP_DT_SMART);

		g_schCtr = false;
		break;

	case ID_VIEW_APPLOOK_2007:
	case ID_VIEW_APPLOOK_2007_1:
	case ID_VIEW_APPLOOK_2007_2:
	case ID_VIEW_APPLOOK_2007_3:
		// enable Office 2007 look:
		switch (m_nAppLook)
		{
		case ID_VIEW_APPLOOK_2007:
			CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_2007_1:
			CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_2007_2:
			CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_Silver);
			break;

		case ID_VIEW_APPLOOK_2007_3:
			CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_Aqua);
			break;
		}

		CBCGPVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGPVisualManager2007));

		mdiTabParams.m_style = CBCGPTabWnd::STYLE_3D_VS2005;
		mdiTabParams.m_bAutoColor = TRUE;
		CBCGPDockManager::SetDockMode (BCGP_DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS2005:
		// enable VS 2005 look:
		CBCGPVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGPVisualManagerVS2005));

		mdiTabParams.m_style = CBCGPTabWnd::STYLE_3D_VS2005;
		mdiTabParams.m_bDocumentMenu = TRUE;
		CBCGPDockManager::SetDockMode (BCGP_DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS2008:
		// enable VS 2008 look:
		CBCGPVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGPVisualManagerVS2008));

		mdiTabParams.m_style = CBCGPTabWnd::STYLE_3D_VS2005;
		mdiTabParams.m_bDocumentMenu = TRUE;
		CBCGPDockManager::SetDockMode (BCGP_DT_SMART);
		g_schCtr = false;
		break;
	}

	EnableMDITabbedGroups (TRUE, mdiTabParams);

	CBCGPDockManager* pDockManager = GetDockManager ();
	if (pDockManager != NULL)
	{
		ASSERT_VALID (pDockManager);
		pDockManager->AdjustBarFrames ();
	}

	CBCGPTabbedControlBar::ResetTabs ();

	RecalcLayout ();
	RedrawWindow (NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

	theApp.WriteInt (_T("ApplicationLook"), m_nAppLook);
}

void CMainFrame::OnUpdateAppLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio (m_nAppLook == pCmdUI->m_nID);
}

BOOL CMainFrame::OnShowMDITabContextMenu (CPoint point, DWORD dwAllowedItems, BOOL bDrop)
{
	//CMenu menu;
	//VERIFY(menu.LoadMenu (bDrop ? IDR_POPUP_DROP_MDITABS : IDR_POPUP_MDITABS));
	CPopMenuMgr popMgr;
	popMgr.SetMemId(bDrop ? IDR_POPUP_DROP_MDITABS : IDR_POPUP_MDITABS);
	CMenu* pPopup = popMgr.GetMenu().GetSubMenu(0);
	ASSERT(pPopup != NULL);

	pPopup->DeleteMenu (ID_FILE_SAVE, MF_BYCOMMAND);
	if ((dwAllowedItems & BCGP_MDI_CREATE_HORZ_GROUP) == 0)
	{
		pPopup->DeleteMenu (ID_MDI_NEW_HORZ_TAB_GROUP, MF_BYCOMMAND);
	}

	if ((dwAllowedItems & BCGP_MDI_CREATE_VERT_GROUP) == 0)
	{
		pPopup->DeleteMenu (ID_MDI_NEW_VERT_GROUP, MF_BYCOMMAND);
	}

	if ((dwAllowedItems & BCGP_MDI_CAN_MOVE_NEXT) == 0)
	{
		pPopup->DeleteMenu (ID_MDI_MOVE_TO_NEXT_GROUP, MF_BYCOMMAND);
	}

	if ((dwAllowedItems & BCGP_MDI_CAN_MOVE_PREV) == 0)
	{
		pPopup->DeleteMenu (ID_MDI_MOVE_TO_PREV_GROUP, MF_BYCOMMAND);
	}

	CBCGPPopupMenu* pPopupMenu = new CBCGPPopupMenu;
	pPopupMenu->SetAutoDestroy (FALSE);
	pPopupMenu->Create (this, point.x, point.y, pPopup->GetSafeHmenu ());

	return TRUE;
}

void CMainFrame::OnMdiMoveToNextGroup() 
{
	MDITabMoveToNextGroup ();
}

void CMainFrame::OnMdiMoveToPrevGroup() 
{
	MDITabMoveToNextGroup (FALSE);
}

void CMainFrame::OnMdiNewHorzTabGroup() 
{
	MDITabNewGroup (FALSE);
}

void CMainFrame::OnMdiNewVertGroup() 
{
	MDITabNewGroup ();
}

void CMainFrame::OnMdiCancel() 
{
	// TODO: Add your command handler code here
	
}

CBCGPMDIChildWnd* CMainFrame::CreateDocumentWindow (LPCTSTR lpcszDocName, CObject* /*pObj*/)
{
	if (lpcszDocName != NULL && lpcszDocName [0] != '\0')
	{
		CDocument* pDoc = AfxGetApp()->OpenDocumentFile (lpcszDocName);

		if (pDoc != NULL)
		{
			POSITION pos = pDoc->GetFirstViewPosition();

			if (pos != NULL)
			{
				CView* pView = pDoc->GetNextView (pos);
				if (pView == NULL)
				{
					return NULL;
				}

				return DYNAMIC_DOWNCAST (CBCGPMDIChildWnd, pView->GetParent ());
			}   
		}
	}

	return NULL;
}

#include "WindowsStateOper.h"
void CMainFrame::OnClose() 
{

	int result = theApp.FinalClose();
	if ( result== IDYES)
	{
		theApp.SaveSystemSet();	// 写注册表  保存默认打开
		if (false == theApp.SaveOrSaveAs())
			return;
		SaveMDIState (theApp.GetRegSectionPath ());
		CBCGPMDIFrameWnd::OnClose();
		theApp.Close();
	}
	else if (result == IDNO)
	{
		theApp.SaveSystemSet();	// 写注册表  保存默认打开
		SaveMDIState (theApp.GetRegSectionPath ());
		CBCGPMDIFrameWnd::OnClose();
		theApp.Close();
	}
	else
	{
		CBCGPToolbarButton* button = m_calToolBar.GetButton(4);		// 展开按钮不要了 
		if(NULL != button&&button->GetImage() != m_imageId)
			button->SetImage(m_imageId);

		return;
	}
	
}
void CMainFrame::OnLimitClose()
{
	int result = AfxMessageBox(GetRemarkFiled(ID_UI_OUT_MESSAGE_BOX_STRING).c_str(),MB_YESNO);
	if ( result== IDYES)
	{
		theApp.SaveOrSaveAs();
		theApp.Close();
		SaveMDIState (theApp.GetRegSectionPath ());
		CBCGPMDIFrameWnd::OnClose();
	}
	else
	{
		theApp.Close();
		SaveMDIState (theApp.GetRegSectionPath ());
		CBCGPMDIFrameWnd::OnClose();
	}
}

 // RIBBON_APP

void CMainFrame::OnUpdateFileOpen(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_
	
	pCmdUI->Enable(TRUE);

	//if(BIZAPI::GetBizMap()->GetApsParam()->GetStopPatchCondition() != 2 )
	//{
	//	CBCGPToolbarButton* button = m_calToolBar.GetButton(5);
	//	if(NULL != button&&button->GetImage() != m_imageId)
	//		m_calToolBar.SetButtonInfo(5,button->m_nID,button->m_nStyle,m_imageId);
	//}
}

LRESULT CMainFrame::OnGetTabToolTip(WPARAM /*wp*/, LPARAM lp)
{
	return 0;
}

void CMainFrame::OnFileClose()
{
	// TODO: Add your command handler code here
	//UINT id = GetMDITabs().GetActiveTab();
	//theApp.g_viewTips.Delete(id);
	GetActiveDocument()->OnCloseDocument();
}

void  CMainFrame::OnSetActivitView(CWnd* pWnd)
{
	ASSERT(pWnd->IsKindOf(RUNTIME_CLASS(CYKSchedulerView)));

	CYKSchedulerView* pToView = (CYKSchedulerView*)pWnd;
	CMDIChildWnd* child = MDIGetActive();
	do
	{
		if (child->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
		{
			CYKSchedulerView* view = (CYKSchedulerView*)child->GetActiveView();
			if (view->GetTblType() == pToView->GetTblType())
			{
				if ( view->GetTblType() == TblOrder)
				{
					//GetFilterConditionManager(2)->DeleteConByIndex(TblOrder);
					BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
					InsertFilter(TblOrder,L"");
				}
				else if (view->GetTblType() == TblResource)
				{
					//GetFilterConditionManager(2)->DeleteConByIndex(TblResource);
					BIZAPI::SetTableFlashFlg(TRUE,TblResource);
					theApp.m_isNeedInitData = 1 ;
					InsertFilter(TblResource,L"");
				}
				
				//view->SetFindType(-1);
				// 需要初始化数据
				view->Flash(theApp.m_isNeedInitData);
				
				child->MDIActivate();        // or MDIActivate(child);
				break;
			}
		}		
		child = (CMDIChildWnd*) child->GetWindow(GW_HWNDNEXT);
	}
	while (child);
}
 // 刷新当前视图  修改属性窗口调用函数   如果属性窗口与视图窗口不是一个表  则不更新 
void CMainFrame::UpdateActiveView(UINT tblType)
{
	if (tblType > 0)
	{
		CYKSchedulerView* pCurView = GetActiveView();
		if (NULL != pCurView)
		{
			if (pCurView->GetTblType() == tblType ||
				TblSchSysParam == tblType ||
				(ID_BOM_VIEW_DLG == pCurView->GetTblType() && TblBOM == tblType))
			{
				pCurView->Flash();
			}
		}
	}
}

void CMainFrame::FlashGantt()
{
	CYKSchedulerView* pCurView = GetActiveView();
	if (NULL != pCurView)
		pCurView->Flash();
}
 // 刷新itemid 的BOM编辑
void CMainFrame::FlashBomForGridEdited(YK_ULONG itemid)
{
	CMDIChildWnd* child = MDIGetActive();
	if (child != NULL)
	{
		do
		{
			if (child->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
			{
				CYKSchedulerView* view = (CYKSchedulerView*)child->GetActiveView();
				if (view != NULL&& view->GetTblType() == ID_BOM_VIEW_DLG)
				{
					if (view->m_bomViewDlg.GetItemId() == itemid)
						view->m_bomViewDlg.Flash();
					break;
				}
			}
			child = (CMDIChildWnd*) child->GetWindow(GW_HWNDNEXT);
		}
		while (child);
	}
}
LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	if (message == IDU_ACTIVIT_VIEW&& lParam > 0)
	{
		CWnd* pWnd = (CWnd*)lParam;
		OnSetActivitView(pWnd);
		return TRUE;
	}
	else if (message == IDU_UPDATE_MODIFY_VIEW)
	{
		m_wndWatchBar.Update((UINT)wParam,(UINT) lParam);
		return TRUE;
	}
	else if (message == IDU_UPDATE_MODIFY_VIEW+1)
	{
		m_wndWatchBar.Update_IitemChange((UINT)wParam,(UINT) lParam);
		return TRUE;
	}
	else if (message == IDU_UPDATE_WACTH_BAR)
	{
		m_wndWatchBar.Update();
		return TRUE;
	}
	else if (message == IDU_UPDATE_MODIFY_DLG)
	{
		//TimeLog::WriteLog("0,GetMassage \n");
		PopModifyDlg(wParam,lParam);
		return FALSE;
	}
	else if (message == IDU_UPDATE_MODIFY_WACTH)
	{
		// 新的数据已经保存到bizmodel  需要重新取数据 （不取ID列表）
		UpdateActiveView((UINT)wParam);
		if((int)wParam == TblSchSysParam)
		{
			m_wndOutput.Flash();
			m_wndWatchBar.Update();
		}
		return TRUE;
	}
	else if (message == IDU_MAINFRM_NEWITEM)
	{
		CYKSchedulerView* pCurView = GetActiveView();
		if (NULL != pCurView)
		{
			const UINT uTblType = pCurView->GetTblType();
			switch (uTblType)
			{
			case TblBOM:
				{
					CBOMInfo* cBOMInfo = pCurView->GetBOMGridCtrl();
					if (NULL != cBOMInfo && 1== lParam)
						cBOMInfo->NewBOM((YK_ULONG)wParam,(int)lParam);
				}
				break;
			case ID_BOM_VIEW_DLG:
				{
					CBOMInfo& bonInfo = pCurView->m_bomViewDlg.GetBOMGrid();
					if(bonInfo.GetSafeHwnd() && 1== lParam)
						bonInfo.NewBOM((YK_ULONG)wParam,(int)lParam);
				}
				break;
			default:
				{
					CDataGridCtrl* pCurrentGrid = pCurView->GetActiveGridCtrl();
					if(NULL != pCurrentGrid)
						pCurrentGrid->RealNew((YK_ULONG)wParam, (int)lParam);
				}
				break;
			}
		}
		return TRUE;
	}
	else if (message == _ACTIVITE_VIWE_ && wParam == 123)
	{
		if(m_flashDataFlag)  //在新建的时候不能更新
		{
			FlashGantt();
			return TRUE;
		}
		return TRUE;
	}
	else if (message == _CLOSE_ALL_VIEW && wParam == 10)
	{
		CloseAllView();
		return TRUE;
	}
	else if (message == ID_MESSAGE_INITTREE)
	{
		OnInitTree();
		return TRUE;
	}
	else if (message == ID_INIT_RESOURCE )
	{
		OnInitResource();
		return TRUE;
	}
	else if (message == IDU_TREE_FIND)
	{
		UINT tblType = (UINT)wParam;
		if (tblType == TblResource)
		{
			FindYKResource((UINT)lParam);
		}
		else if (tblType == TblOrder)
		{
			FindYKOrder((UINT)lParam);
		}
		else if (tblType == TblProduceIndication)
		{
			FindYKProduceIndication((UINT)lParam);
		}
		else if (tblType == TblResourceSequence)
		{
			FindYKResourceSequence((UINT)lParam);
		}
		else if (tblType  == TblCalendar)
		{
			FindYKCalendar((UINT)lParam);
		}
		return TRUE;
	}
	//初始化APS下拉框
	else if (message == IDU_INIT_APS_COMBOBOX)
	{
		InitApsComboBox();
		return TRUE;
	}
	else if (message == ID_PMCTOOLBAR_CHANGE)
	{
		PMCToolBar();
		return TRUE;
	}
	else if ( message == IDU_FIND_CODE_MFG)
	{
		FindCode();
		return TRUE;
	}
	else if( message == IDU_FIND_CODE_REPLACE)
	{
		ReplaceCode();
		return TRUE;
	}
	else if (IDU_WORK == message)
	{
		CYKSchedulerView* pCurView = GetActiveView();
		if (NULL != pCurView)
		{
			const UINT uTblType = pCurView->GetTblType();
			vector<GridRowInfo> rowList;
			switch (uTblType)
			{
			case TblWorkAvbResource:
				pCurView->GetWorkToUseResource(rowList, (UINT)wParam);
				break;
			case TblWorkRelation:
				pCurView->GetWorkToWorkRelation(rowList, (UINT)wParam);
				break;
			}

			if (pCurView->m_pGdCtrl.GetSafeHwnd())
			{
				pCurView->m_pGdCtrl.InitSource(rowList);
				pCurView->m_pGdCtrl.ResetSource();
				pCurView->m_pGdCtrl.Flash();
				//pCurView->m_pGdCtrl.InitFlexGrid();
			}
			BIZAPI::SetTableFlashFlg(TRUE, uTblType);
		}
		return TRUE;
	}
	else if (message == ID_MESSAGE_FLASH_OUTPUTWND)
	{
		BIZAPI::SetTableFlashFlg(TRUE,TblMessageData);
		m_wndOutput.UpdateMsgForShale();
		//m_wndOutput.Flash();
		if(lParam == 7)		// new/open
			m_wndOutput.AdjustWith();
	}
	else if (message == ID_MESSAGE_MSGCONDLG)
	{
		YK_ULONG msgId=(YK_ULONG)wParam;
		if (!m_msgConDlg.GetSafeHwnd())
		{
			m_msgConDlg.Create(CMessageContentDlg::IDD,this);
			CRect cmainCrect;
			CRect cdlgCrect;
			GetClientRect(&cmainCrect);
			m_msgConDlg.GetWindowRect(&cdlgCrect);
			CPoint pp(cmainCrect.left+cmainCrect.Width()/2-cdlgCrect.Width()/2,cmainCrect.top+cmainCrect.Height()/2-cdlgCrect.Height()/2);
			ClientToScreen(&pp);
			CRect needcrect;
			needcrect.left=pp.x;
			needcrect.top=pp.y;
			needcrect.right=needcrect.left+cdlgCrect.Width();
			needcrect.bottom=needcrect.top+cdlgCrect.Height();
			m_msgConDlg.MoveWindow(needcrect);
		}

		m_msgConDlg.SetMsgId(msgId);
		m_msgConDlg.ShowWindow(SW_SHOW);
	}
	else if (message == ID_MESSAGE_MSGJUMP)
	{
		CYKSchedulerView* pCurView = GetActiveView();
		if (NULL != pCurView)
		{
			vector<UINT> vecID = m_msgConDlg.GetObjIdList1();
			vector<GridRowInfo> rowInfoList;
			const int nVecLen = vecID.size();
			for (int i= 0; i < nVecLen; i++)
			{
				GridRowInfo rowInfo;
				rowInfo.id = vecID[i];
				rowInfo.tblType = pCurView->GetTblType();
				rowInfoList.push_back(rowInfo);
			}
			pCurView->SetFindRowKeyList(rowInfoList);
			pCurView->Find(0);
		}
	}
	else if (message == WM_KEYDOWN)
	{
		switch(wParam)
		{
		case VK_F3:
			OnMenu();
			break;
		case VK_F4:
			OnYkSchparmSet();
			break;
		case VK_F5:
			OnYkScheduler();
			break;
		}
	}
	else if (MSG_RUN_SCHULDER == message)
	{
		SchudlerThread(lParam);
	}
	//加密狗连接变化
	else if (message == IDM_LICENSEOBSERVER)
	{
#ifdef _APPLYPROTECTION
		bool newLience = wParam == 1;
		bool oleLience = lParam == 1;
		if (!newLience)
		{
			if(!DATATABLAPI::IsOnLine())
			{
				if (theApp.IsProctDogOverdue())
				{
					if (IDYES == theApp.ProctDogMsg(1))
					{
						BIZAPI::SetLimitEdition(true);
					}
					//else
					//{
					//	OnLimitClose();
					//	return false;
					//}
				}
				else
				{
					if (IDYES == theApp.ProctDogMsg(0))
					{
						BIZAPI::SetLimitEdition(true);
					}
					//else
					//{
					//	OnLimitClose();
					//	return false;
					//}
				}
				BIZAPI::SetLimitEdition(true);
			}

		}
		else
		{
			BIZAPI::SetLimitEdition(false);
		}
		theApp.SetMainFrmText();
#endif
		return true;
	}
	else if (message == UM_SCHUPDATECURVIEW)
	{
		CString str;
		int process = min(BIZAPI::GetProcess(),100);
		str.Format(_T("%d"),process);
		str += _T("%");
		SetStatusBarText(str);

		BIZAPI::GetBizMap()->RedrawAllGantt();
		BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
		BIZAPI::SetTableFlashFlg(TRUE,TblWork);
		FlashGantt();
	}

	return CBCGPMDIFrameWnd::WindowProc(message, wParam, lParam);
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// TODO: Add your specialized code here and/or call the base class

	return CBCGPMDIFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

HBRUSH CMainFrame::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CBCGPMDIFrameWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
//	SetBkColor(pDC->GetSafeHdc(),COLOR_Main_Ctl_Bkg/*RGB(120,131,163)*/);

	return hbr;
}

void CMainFrame::OnYkOrderCal()
{
	// TODO: Add your command handler code here

	////限制版
	//BIZAPI::LimitCheck();
	//if (BIZAPI::GetIsLimited())
	//{
	//	m_wndOutput.Flash();
	//	return;
	//}
	//BIZAPI::ClearUndoRedo();		// 清空撤销栈

	//g_schCtr = FALSE;
	//theApp.m_couldCreatWorkChange=FALSE;   // 工作变更不能生成
	////m_wndOutput.ShowOrderExpendStartInfo();
	//CString cstrCode=L"";
	//Timer  mtime;

	//SetMsgInfo(ID_UI_BAR_ORDEREXPANDSTART,cstrCode,false,MSG_TYPE_APS);		//   订单展开开始
	//mtime.start();
	//BIZAPI::ApsCalculate();
	//double time = mtime.stop();
	//cstrCode.Format(L"( %0.2lf %s)",time,GetRemarkFiled(ID_UI_BAR_SECOND).c_str());  // L"订单展开结束"
	//SetMsgInfo(ID_UI_BAR_ORDEREXPANDEND,cstrCode,true,MSG_TYPE_APS);

	//BIZAPI::LimitCheck();
	//m_wndOutput.Flash();

	//BIZAPI::GetBizMap()->RedrawAllGantt();
	///*BIZAPI::ClearUndoRedo();*/
	//GanttCommonApi::Reset();

	//BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
	//BIZAPI::SetTableFlashFlg(TRUE,TblWork);
	//FlashGantt();
}

static  bool bSchudlerExit = false;
unsigned int WINAPI Run_Arith(void* pParam)
{
// 	if (BIZAPI::EnterDataBaseCriSel())
// 	{
// 		try
// 		{
// 			int schLevel = (int) pParam;
// 			BIZAPI::Scheduler(schLevel);
// 			g_nProcess = -1;
// 		}
// 		catch(...)
// 		{
// 		}
// 		BIZAPI::LeaveDataBaseCriSel();
// 	}
	return 0;
}
unsigned int WINAPI Run_GetKey(void* pParam)
{
	CMainFrame* pMainFrame = (CMainFrame*)pParam;
	if (pMainFrame != NULL)
	{
		while(true)
		{
			if (g_nProcess < 0)
			{
				return 0;
			}

			if (BIZAPI::GetViewFlashFlg())
			{
				BIZAPI::SetViewFlashFlg(false);
				BIZAPI::GetBizMap()->RedrawAllGantt();
				BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
				BIZAPI::SetTableFlashFlg(TRUE,TblWork);

				CString str;
				int process = min(BIZAPI::GetProcess(),100);
				str.Format(_T("%d"),process);
				str += _T("%");
				pMainFrame->SetStatusBarText(str);
				::SendMessage(pMainFrame->GetSafeHwnd(), _ACTIVITE_VIWE_, 123, 0);

				/*POSITION docTempPos = theApp.GetFirstDocTemplatePosition();
				CDocTemplate* pDocTemp = theApp.GetNextDocTemplate(docTempPos);
				while (pDocTemp != NULL)
				{
					POSITION docPos = pDocTemp->GetFirstDocPosition();
					CDocument* pDoc = pDocTemp->GetNextDoc(docPos);
					while (pDoc != NULL)
					{
						POSITION pos = pDoc->GetFirstViewPosition();
						if (pos != NULL)
						{
							CView* pView = pDoc->GetNextView (pos);
							while (pView != NULL)
							{
								CYKSchedulerView* pYKView = (CYKSchedulerView*)pView;
								if (pYKView->GetTblType() == ID_GANTT_RES_GL)
									pYKView->Flash();
								if (pos != NULL)
									pView = pDoc->GetNextView (pos);
								else
									pView = NULL;
							}

						}   
						if (docPos != NULL)
							pDoc = pDocTemp->GetNextDoc(docPos);
						else
							pDoc = NULL;
					}
					if (docTempPos != NULL)
						pDocTemp = theApp.GetNextDocTemplate(docTempPos);
					else
						pDocTemp = NULL;
				}*/
				
			}
		}
	}
	g_nProcess = -1;

	return 0;
}
void CMainFrame::SetStatusBarText( CString& strText )
{
	m_wndStatusBar.SetWindowText(strText);
}

unsigned int WINAPI Run_SchudlerThread(void* pParam)
{
	clock_t t0 = clock();
	CMainFrame* pMainFrame = (CMainFrame*)pParam;
	YKBizMap* bMap=BIZAPI::GetBizMap();
	if (pMainFrame != NULL &&NULL != bMap)
	{
		g_nProcess = 0;
		if (BIZAPI::EnterDataBaseCriSel())
		{
			try
			{
				int schLevel = (int) pParam;
				BIZAPI::Scheduler(schLevel, pMainFrame->GetSafeHwnd());
				g_nProcess = -1;
			}
			catch(...)
			{
			}
			BIZAPI::LeaveDataBaseCriSel();
		}
//  		HANDLE   hThread[2];
//  		hThread[0] = (HANDLE)_beginthreadex(NULL,0,Run_Arith,(void*)g_schLevel,0,NULL);
// 		hThread[1] = (HANDLE)_beginthreadex(NULL,0,Run_GetKey,(void*)pParam,0,NULL);
//  		WaitForMultipleObjects(2,hThread,TRUE,INFINITE);
	}
	clock_t endTm = (clock()-t0) ;

	::PostMessage(pMainFrame->GetSafeHwnd(),MSG_RUN_SCHULDER,0,(LPARAM)endTm);

	return 1;
}

void CMainFrame::SchudlerThread(LPARAM endTm)
{
	YKBizMap* bMap=BIZAPI::GetBizMap();
	if(NULL == bMap) return;

	
	if(BIZAPI::GetBizMap()->GetApsParam()->GetStopPatchCondition() == 1
		|| g_schLevel >= 4)
		g_schLevel = 0;
	g_schCtr = TRUE;
	GanttApi::SetReSchFlg(false);

	g_nProcess = -1;

	BIZAPI::ClearUndoRedo();
	GanttCommonApi::Reset();

	bMap->FlashSonWorksEmptyCode(); //刷新动态拆分子工作代码 add 2011-1-31

	BIZAPI::CreateUseResource();    // always do
	theApp.m_couldCreatWorkChange=TRUE; // 排完程后可以计算工作变更

	unsigned long id_=bMap->GetEstimateApsId();
	bool b_=false;
	if(id_>0)
	{
		b_=true;
		bMap->SetApsParanID(id_);
	}
	ShowSchedulerEndInfo(b_,double(endTm)/CLOCKS_PER_SEC);
#ifdef LIMITFUNCTION
	BIZAPI::LimitCheck();
#endif
	bMap->RedrawAllGantt();
	BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
	BIZAPI::SetTableFlashFlg(TRUE,TblWork);
	//Add 2012-01-31
	BIZAPI::SetTableFlashFlg(TRUE,TblWorkChange);
	FlashGantt();
	EndWaitCursor();
	m_wndOutput.Flash();

	theApp.SetSchudlingFlg(false);
}

void CMainFrame::OnYkScheduler()
{
	if (theApp.GetSchudlingFlg())
		return;
	//old
#ifdef _USEMULTITHREADINGSCH
#ifdef LIMITFUNCTION
	BIZAPI::LimitCheck();

	if (BIZAPI::GetIsLimited())
	{
		m_wndOutput.Flash();
		return;
	}
#endif

	if (  g_shiftStata != 1 )		//shift 键控制进度条
	{
		if ( AfxMessageBox( GetRemarkFiled(ID_PROMT_YKSCHEDULER).c_str() ,MB_YESNO|MB_ICONQUESTION) == IDNO )
			return;
	}

	YKBizMap* bMap=BIZAPI::GetBizMap();
	if(NULL == bMap) return;

	if(bMap->GetSchSysParam()->GetAreaSchPara() == By_Long_Sch)
	{
		BIZAPI::LongScheduler();
		BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
		FlashGantt();
		return;
	}

	CString cstrCode=L"";	
	SetMsgInfo(ID_UI_BAR_SCHEDULSTART,cstrCode,false,MSG_TYPE_SCHEDUL);		//  排程开始
	BeginWaitCursor();

	// 清空工作使用指令  物品指令 资源指令
	BIZAPI::ClearUseResCateSequenceResSequenceMap();	
	bMap->SetIsCreatedItemSequence(FALSE);
	bMap->SetIsCreatedResSequence(FALSE);
	bMap->SetIsCreatedUseSequence(FALSE);
	bMap->SetIsCreatedProIndia(FALSE);
	GanttApi::OnClearAllWorkSelect();

	theApp.SetSchudlingFlg(true);
	theApp.SetCanSchedStep(false);
	CYKSchedulerView* pCurView = GetActiveView();
	if (NULL != pCurView)
	{
		const UINT uTblType = pCurView->GetTblType();
		if (uTblType == ID_GANTT_RES_GL || uTblType == ID_GANTT_ORDER_GL ||
			uTblType == ID_RATE_RES_GL || uTblType == ID_RATE_STA_GL ||
			uTblType == ID_Item_Gather_GL)
		{
			theApp.SetCanSchedStep(true);
		}
	}

	HANDLE   hThread;
	hThread = (HANDLE)_beginthreadex(NULL,0,Run_SchudlerThread,(void*)this,0,NULL);
	SetThreadPriority(hThread,THREAD_PRIORITY_ABOVE_NORMAL);

	if (  g_shiftStata != 1 )		//shift 键控制进度条
	{
		CProcessBarThread* pPrBarThead = (CProcessBarThread*)AfxBeginThread(RUNTIME_CLASS(CProcessBarThread),THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
		pPrBarThead->SetHeadTitle(3);
		pPrBarThead->DisableCycleShow();
		pPrBarThead->EnableDestroyByOut();
		pPrBarThead->ResumeThread();
	}

#else
#ifdef LIMITFUNCTION
	//限制版
	BIZAPI::LimitCheck();
	if (BIZAPI::GetIsLimited())
	{
		m_wndOutput.Flash();
		return;
	}
#endif

	YKBizMap* bMap=BIZAPI::GetBizMap();
	if(NULL == bMap) return;

	if(bMap->GetSchSysParam()->GetAreaSchPara() == By_Long_Sch)
	{
		BIZAPI::LongScheduler();
		BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
		FlashGantt();
		return;
	}

	//m_wndOutput.ShowSchedulerStartInfo();
	CString cstrCode=L"";	
	Timer  mtime;
	SetMsgInfo(ID_UI_BAR_SCHEDULSTART,cstrCode,false,MSG_TYPE_SCHEDUL);		//  排程开始
	mtime.start();
	BeginWaitCursor();
	// niu
	//CFlexDataSource::WaitForThreadQuit();

	// 清空工作使用指令  物品指令 资源指令
	BIZAPI::ClearUseResCateSequenceResSequenceMap();	
	bMap->SetIsCreatedItemSequence(FALSE);
	bMap->SetIsCreatedResSequence(FALSE);
	bMap->SetIsCreatedUseSequence(FALSE);
	bMap->SetIsCreatedProIndia(FALSE);

	bool splitFlg = GanttApi::GetReSchFlg();
#ifndef _OCXPACK
	///进度条
	if (g_pProDlg != NULL)
	{
		if (g_pProDlg->GetSafeHwnd())
		{
			g_pProDlg->DestroyWindow();
			delete g_pProDlg;
			g_pProDlg = NULL;
		}
	}
	if (g_pProDlg == NULL)
	{
		g_pProDlg = new CSchProcessDlg;
		g_pProDlg->Create(CSchProcessDlg::IDD,this);
		CRect rcClient;
		GetClientRect(rcClient);
		CRect rcProcess;
		g_pProDlg->GetClientRect(rcProcess);
		rcClient.left = rcClient.left + rcClient.Width()/2;
		rcClient.top = rcClient.top + rcClient.Height()/2;
		rcClient.left -= rcProcess.Width()/2;
		rcClient.top  -= rcProcess.Height()/2;
		rcClient.right = rcClient.left + rcProcess.Width();
		rcClient.bottom = rcClient.top + rcProcess.Height();
		g_pProDlg->MoveWindow(rcClient,false);
		g_pProDlg->ShowWindow(SW_SHOW);
		g_pProDlg->SetMinMax(100);
		g_pProDlg->SetCurPos(0);
		g_pProDlg->InitPro();
	}
#endif
	++g_schLevel;
	g_nProcess = 0;
	BIZAPI::IniProcess();
	uintptr_t nTread = _beginthreadex(NULL,0,Run_SchudlerThread,(void*)this,0,NULL);
#ifndef _OCXPACK
	while(g_nProcess != -1)
	{
		if (g_pProDlg != NULL&&g_pProDlg->GetSafeHwnd())
		{
			g_nProcess = BIZAPI::GetProcess();
			CString str;
			str.Format(_T("%d\r\n"),g_nProcess);
			TRACE(str);
			g_pProDlg->SetCurPos(g_nProcess);
			g_pProDlg->RedrawWindow();
			Sleep(250);
		}
	}
#endif
	{

		if(BIZAPI::GetBizMap()->GetApsParam()->GetStopPatchCondition() == 1
			|| g_schLevel >= 4)
			g_schLevel = 0;
		g_schCtr = TRUE;
		GanttApi::SetReSchFlg(false);
	}
	g_nProcess = -1;
#ifndef _OCXPACK
	if (g_pProDlg != NULL)
	{
		if (g_pProDlg->GetSafeHwnd())
		{
			g_pProDlg->DestroyWindow();
			delete g_pProDlg;
			g_pProDlg = NULL;
		}
	}
#endif
	BIZAPI::ClearUndoRedo();
	bMap->FlashSonWorksEmptyCode(); //刷新动态拆分子工作代码 add 2011-1-31
	double endTm=mtime.stop();
#ifdef LIMITFUNCTION
	BIZAPI::LimitCheck();
#endif
	EndWaitCursor();

#ifndef _OCXPACK
	GanttCommonApi::Reset();

	BIZAPI::CreateUseResource();    // always do
	theApp.m_couldCreatWorkChange=TRUE; // 排完程后可以计算工作变更

	unsigned long id_=bMap->GetEstimateApsId();
	bool b_=false;
	if(id_>0)
	{
		b_=true;
		bMap->SetApsParanID(id_);
	}
	ShowSchedulerEndInfo(b_,endTm);
//	bMap->SetApsParanID(0);

		bMap->RedrawAllGantt();
	BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
	BIZAPI::SetTableFlashFlg(TRUE,TblWork);
	BIZAPI::SetTableFlashFlg(TRUE, TblWorkChange);
	FlashGantt();
	m_wndOutput.Flash();
#endif	///_OCXPACK

#endif
}

void CMainFrame::OnYKDelCom()
{
	CYKSchedulerView* pCurView = GetActiveView();
	if (NULL != pCurView)
	{
		const UINT uTblType = pCurView->GetTblType();

		switch (uTblType)
		{
		case ID_GANTT_RES_GL:
			GanttApi::DelCom();
			break;
		case ID_GANTT_ORDER_GL:
			OrderGanttApi::DelCom();
			break;
		case ID_RATE_RES_GL:
			ResRateGanttApi::DelCom();
			break;
		case ID_RATE_STA_GL:
			StockGanttApi::DelCom();
			break;
		case ID_Item_Gather_GL:
			ItemGatherApi::DelCom();
			break;
		default:
			break;
		}
	}
}

void CMainFrame::OnYkResGantt()
{
	theApp.OpenTblPage(ID_GANTT_RES_GL);
}

void CMainFrame::OnYkOrderGantt()
{
	theApp.OpenTblPage(ID_GANTT_ORDER_GL);
}

void CMainFrame::OnYkResRate()
{
	theApp.OpenTblPage(ID_RATE_RES_GL);
}

void CMainFrame::OnYkStagRate()
{
	theApp.OpenTblPage(ID_RATE_STA_GL);
}


void CMainFrame::OnQuerys()
{
	if (NULL != GetActiveView())
		FindCode();
}

void CMainFrame::OnUpdateYkObjWin(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(m_wndWorkSpace.IsWindowVisible ());
}

void CMainFrame::OnUpdateYkProWin(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(m_wndWatchBar.IsWindowVisible ());
}

void CMainFrame::OnUpdateYkInfoWin(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(m_wndOutput.IsWindowVisible ());
}

void CMainFrame::OnYkObjWin()
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_wndWorkSpace);
}

void CMainFrame::OnYkProWin()
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_wndWatchBar);
}

void CMainFrame::OnYkInfoWin()
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_wndOutput);
}

void CMainFrame::ShowControlBar(CBCGPControlBar* pBar)
{
	if (pBar != NULL)
	{
		pBar->ShowControlBar (!pBar->IsVisible (), FALSE, TRUE);

		CFrameWnd* pFrame = BCGCBProGetTopLevelFrame (pBar);
		if (pFrame == NULL)
		{
			RecalcLayout ();
		}
		else
		{
			pFrame->RecalcLayout ();
		}
	}
}
void CMainFrame::CreateSBProgress()
{
	m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_PROGRESS, SBPS_DISABLED, 250);
	CRect rcProgress, rcLabel;
	m_wndStatusBar.GetItemRect(1, &rcProgress);
	rcLabel = rcProgress;
	rcLabel.right -= 100;
	m_labelSBProgress.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_RIGHT, rcLabel, &m_wndStatusBar);
	m_labelSBProgress.SetFont(m_wndStatusBar.GetFont());

	rcProgress.left += 150;
	m_progressCtl.Create(WS_VISIBLE | WS_CHILD | PBS_SMOOTH, rcProgress, &m_wndStatusBar, 10000);
	m_progressCtl.ShowWindow(SW_HIDE);
}
//系统参数
void CMainFrame::OnUpdateMenu(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
		pCmdUI->Enable(theApp.m_bOpen);
}
//排程参数
void CMainFrame::OnUpdateYkSchparmSet(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_;
	if (theApp.HideTable(TblApsParam))
		return pCmdUI->Enable(FALSE);
	pCmdUI->Enable(theApp.m_bOpen);
}

//系统参数
void CMainFrame::OnMenu()
{
	// TODO: Add your command handler code here
	CModifyDlg*  pdlgModify = new CModifyDlg;
	if(!pdlgModify->GetOpenFlg())
	{
		CString str;
		str = GetRemarkFiled(UINT(TblSchSysParam)).c_str();
		pdlgModify->SetText(str,L"Go",L"Out");
		pdlgModify->SetTbl((UINT)TblSchSysParam,(UINT)BIZAPI::GetBizMap()->GetSchSysParam()->GetId());
		pdlgModify->DoModal();
	}
	delete pdlgModify;
	
}
//排程参数
void CMainFrame::OnYkSchparmSet()
{
#ifndef PMCMODE
	// change 2010-11-5 by ll
	//增加对话框单令标志
	if (theApp.HideTable(TblApsParam))
		return ;
	CAPSMethodAddDlg  dlg;
	if(!dlg.GetOpenFlg())
	{
		//BIZAPI::BeginRebackOper(OPERTYPEMODIFY);
		wstring wstr = BIZAPI::GetCurrentRunMethodRule();
		if (dlg.DoModal() == IDOK)
		{
			BIZAPI::BeginUndoRedo();
			wstring getCode = dlg.GetCode();
			if (getCode != wstr)
			{
				// 加入撤销信息
				RecordInfo reinfo;
				reinfo.id = 1;
				reinfo.operType = OPERTYPEMODIFY;
				BIZAPI::InsertRelDataStruct(TblSchSysParam,reinfo);

				BIZAPI::SetRunMethodRule(getCode);
				InitApsComboBox();
			}
			//BIZAPI::EndRebackOper();
			BIZAPI::EndUndoRedo();
		}
// 		else 
// 			BIZAPI::DelRebackOper();
	}
#endif
}

void CMainFrame::CloseAllView()
{
	theApp.m_bOpen = FALSE;
//	CMDIChildWnd* oldChild = MDIGetActive();//关闭view
	// 关闭out bar
	m_wndWatchBar.Close();

	m_wndOutput.Close();

	theApp.CloseAllDocuments(TRUE);
}

void CMainFrame::OnUpdateYkOrderCal(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	pCmdUI->Enable(theApp.m_bOpen);
	//pCmdUI->Enable(FALSE);
}

BOOL CMainFrame::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CBCGPMDIFrameWnd::DestroyWindow();
}


void  CMainFrame::OnYKFilter()
{
	CYKSchedulerView* pCurView = GetActiveView();
	if (NULL != pCurView)
	{
		const UINT uTblType = pCurView->GetTblType();
		if (uTblType == ID_GANTT_RES_GL || uTblType == ID_GANTT_ORDER_GL ||
			uTblType == ID_RATE_RES_GL|| uTblType == ID_RATE_STA_GL ||
			uTblType == ID_Item_Gather_GL ||uTblType == ID_WORK_RESOURCE_GD ||
			uTblType == TblItemGather)
		{
			if(uTblType == ID_GANTT_ORDER_GL)
			{
				CTime t = CTime::GetCurrentTime(); 
				wstring str;
				{
					CDealTime fTm(t.GetYear(), t.GetMonth(), 1);
					int m = t.GetMonth()+1;
					int y = t.GetYear();
					if (m>12)
					{
						y+=1;
						m=1;
					}
					CDealTime sTm(y, m, 1);
					CString curTmStr;
					str = L"ME.LastCompleteTime >= #";
					str += TOOLS::FormatTime(fTm.GetTime());
					str += L"#&&ME.LastCompleteTime < #";
					str += TOOLS::FormatTime(sTm.GetTime());
					str += L"#";
				}

				InsertFilter(uTblType, str);
			}

			CNewRuleDlg dlg;
			dlg.InitRule(RULE_DEF_FILTER, GetFilterCode(uTblType), uTblType);
			if(dlg.DoModal() == IDOK)
			{
				InsertFilter(uTblType, dlg.GetCode());
				vector<UINT>fList;
				GLFilter(uTblType, fList);
				pCurView->GLFind(fList);
			}
		}
		else
		{
			CNewRuleDlg dlg;
			dlg.InitRule(RULE_DEF_FILTER,GetFilterCode(uTblType), uTblType);
			if(dlg.DoModal() == IDOK)
			{
				InsertFilter(uTblType, dlg.GetCode());
				BIZAPI::SetTableFlashFlg(TRUE, uTblType);
				FlashGantt();
			}
		}
	}
}

 void  CMainFrame::OnYKUnFilter()
 {
	 CYKSchedulerView* pCurView = GetActiveView();
	 if (NULL != pCurView)
		 pCurView->UpdateActiveGrid();
 }

 void CMainFrame::FindActivitView(CWnd* pWnd)
 {
	 ASSERT(pWnd->IsKindOf(RUNTIME_CLASS(CYKSchedulerView)));

	 CYKSchedulerView* pToView = (CYKSchedulerView*)pWnd;
	 CMDIChildWnd* child = MDIGetActive();
	 do
	 {
		 if (child->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
		 {
			 CYKSchedulerView* view = (CYKSchedulerView*)child->GetActiveView();
			 if (view->GetTblType() == pToView->GetTblType())
			 {
				 child->MDIActivate();        // or MDIActivate(child);
				 break;
			 }
		 }		
		 child = (CMDIChildWnd*) child->GetWindow(GW_HWNDNEXT);
	 }
	 while (child);
 }
 void CMainFrame::FindYKResource(UINT resId)
 {
	 CYKSchedulerView* pCurView = GetActiveView();
	 if (NULL != pCurView)
	 {
		 const UINT uTblType = pCurView->GetTblType();
		 if (TblResource == uTblType)
		 {
			 YKBizMap* pMap = BIZAPI::GetBizMap();
			 if (pMap != NULL)
			 {
				 YKResourcePtrMap* pResMap = pMap->GetYKResourcePtrMap();
				 if (pResMap != NULL)
				 {
					 YKResourcePtr& ptr = pResMap->Get(resId);
					 if ( ptr != NULL)
					 {
						 wstring filterCode;
						 filterCode = L"ME.Code == '";
						 filterCode += ptr->GetCode();
						 filterCode += L"'||ME.BelongRes == '";
						 filterCode += ptr->GetCode();
						 filterCode += L"'";
						 InsertFilter(TblResource, filterCode);
						 BIZAPI::SetTableFlashFlg(TRUE,TblResource);
						 FlashGantt();
						 BIZAPI::SetTableFlashFlg(TRUE, uTblType);
					 }
				 }
			 }
		 }
		 else
		 {
			 Node node;
			 theApp.g_spNodes.Find(TblResource, node);
			 if (node.pWnd != NULL)
			 {
				 FindActivitView(node.pWnd);
			 }
			 else
			 {
				 theApp.OpenTblPage(TblResource);
				 theApp.g_spNodes.Find(TblResource, node);
			 }

			 if (node.pWnd->IsKindOf(RUNTIME_CLASS(CYKSchedulerView)))
				 pCurView = (CYKSchedulerView*)node.pWnd;
		 }
	 }
 }

 void CMainFrame::FindYKOrder(UINT orderFindType)
 {
	 //Add 2012-03-19
	 YK_UINT tblType;

	 CYKSchedulerView* pCurView = GetActiveView();
	 if (NULL != pCurView)
	 {
		 //Add 2012-03-19 
		 tblType = ResetDataGridTypTble(orderFindType);
		 pCurView->SetTblTypeByParm(tblType);

		 const UINT uTblType = pCurView->GetTblType();
		 if (TblOrder == uTblType)
		 {
			

			 wstring filterCode;			 
			 switch(orderFindType)
			 {
			 case CSpaceTreeCTrl::Sell1_Order:
				 {			 
					 filterCode = L"ME.Type == '" + GetRemarkFiled(Order_Sell) + L"'";
					 break;
				 }
			 case CSpaceTreeCTrl::Manufacture_Order:
				 {
					 filterCode = L"ME.Type == '" + GetRemarkFiled(Order_Produce) + L"'";
					 break;
				 }
			 case CSpaceTreeCTrl::Buy_Order:
				 {
					 filterCode = L"ME.Type == '" + GetRemarkFiled(Order_Buy) + L"'";
					 break;
				 }
			 case CSpaceTreeCTrl::Out_Time_Order:
				 {
					 filterCode = L"ME.LastCompleteTime > ME.PlanEndTime ";
					 break;
				 }
			 case CSpaceTreeCTrl::Out_Time_Sell_Order:
				 {
					 break;
				 }
			 case CSpaceTreeCTrl:: Out_Time_Manu_Order:
				 {
					 break;
				 }
			 case CSpaceTreeCTrl:: Out_Time_Buy_Order:
				 {
					 break;
				 }
			 case CSpaceTreeCTrl::UnSch_Order:
				 {
					 filterCode = L"ME.SchState == '" + GetRemarkFiled(Order_Not_Plan) + L"'";
							 break;
				 }
			 case CSpaceTreeCTrl::UnSch_Sell_Order:
				 {
					 filterCode = L"ME.Type == '" + GetRemarkFiled(Order_Sell)
						+ L"&&ME.SchState == '"
						+ GetRemarkFiled(Order_Not_Plan) + L"'";
					 break;
				 }
			 case CSpaceTreeCTrl::UnSch_Manu_Order:
				 {
					 filterCode = L"ME.Type == '" + GetRemarkFiled(Order_Produce)
						+ L"'&&ME.SchState == '"
						+ GetRemarkFiled(Order_Not_Plan) + L"'";
					 break;
				 }
			 case CSpaceTreeCTrl::UnSch_Buy_Order:
				 {
					 filterCode = L"ME.Type == '" + GetRemarkFiled(Order_Buy)
						+ L"'&&ME.SchState == '"
						+ GetRemarkFiled(Order_Not_Plan) + L"'";
					 break;
				 }
			 case CSpaceTreeCTrl::CurMonth_Order:
				 {
					 CDealTime delTime(CDealTime::GetLocalTime());
					 if (delTime.GetTime() > 0)
					 {
						 int endMonth = delTime.GetMonth();
						 int endYear = delTime.GetYear();
						 CDealTime monthBegin(endYear,endMonth,1,0,0,0);
						 endMonth++;
						 if (endMonth == 13)
						 {
							 endMonth = 1;
							 endYear++;
						 }
						 CDealTime monthEnd(endYear,endMonth,1,0,0,0);
						 wstring begTm = TOOLS::FormatTime(monthBegin);
						 wstring endTm = TOOLS::FormatTime(monthEnd);


						 filterCode = L"ME.LastCompleteTime >= #";
						 filterCode += begTm;
						 filterCode += L"#&&ME.LastCompleteTime <= #";
						 filterCode += endTm;
						 filterCode += L"#";
					 }
					 break;
				 }
			 case CSpaceTreeCTrl::Will_Order:
				 {
					 filterCode = L"ME.Type == '" + GetRemarkFiled(Order_Mind) + L"'";
				 }break;
			 case CSpaceTreeCTrl::UnSch_Will_Order:
				 {
					 filterCode = L"ME.Type == '" + GetRemarkFiled(Order_Mind) 
						+ L"'&&ME.SchState == '"
						+ GetRemarkFiled(Order_Not_Plan) + L"'";
				 }break;
			 case CSpaceTreeCTrl::Storage_Order:
				 {
					 filterCode = L"ME.Type == '" + GetRemarkFiled(Order_Save_comp) 
					 + L"'||ME.Type == '"
					 + GetRemarkFiled(Order_Save_Abs) + L"'";
					 break;
				 }
			 case CSpaceTreeCTrl::Mend_Order:
				 {
					 filterCode = L"ME.Type == '" + GetRemarkFiled(Order_Mainten) + L"'";
					 break;
				 }
			 default:
				 {
					 return;
					 break;
				 }
			 }

			 ////Add 2012-03-19
			 //InsertFilter(tblType, filterCode);
			 //// 设置订单表格需要刷新
			 //BIZAPI::SetTableFlashFlg(TRUE, tblType);
			 //FlashGantt();	
			 //BIZAPI::SetTableFlashFlg(TRUE, uTblType);
			 //BIZAPI::SetTableFlashFlg(FALSE, tblType);

			 InsertFilter(TblOrder, filterCode);
			 // 设置订单表格需要刷新
			 BIZAPI::SetTableFlashFlg(TRUE, TblOrder);
			 FlashGantt();	
			 BIZAPI::SetTableFlashFlg(TRUE, uTblType);
			 BIZAPI::SetTableFlashFlg(FALSE, TblOrder);
		 }
		 else
		 {
			 Node node;
			 theApp.g_spNodes.Find(TblOrder, node);
			 if (node.pWnd != NULL)
			 {
				 FindActivitView(node.pWnd);
			 }
			 else
			 {
				 theApp.OpenTblPage(TblOrder);
				 theApp.g_spNodes.Find(TblOrder,node);
			 }

			 if (node.pWnd->IsKindOf(RUNTIME_CLASS(CYKSchedulerView)))
				 pCurView = (CYKSchedulerView*)node.pWnd;
		 }
	 }
 }

 void CMainFrame::FindYKProduceIndication( UINT resId )
 {
	 CYKSchedulerView* pCurView = GetActiveView();
	 if (NULL != pCurView)
	 {
		 const UINT uTblType = pCurView->GetTblType();
		 if (TblProduceIndication == uTblType)
		 {
			 pCurView->Find(resId);
		 }
		 else
		 {
			 Node node;
			 theApp.g_spNodes.Find(TblProduceIndication,node);
			 if (node.pWnd != NULL)
				 FindActivitView(node.pWnd);
			 else
				 theApp.OpenTblPage(TblProduceIndication);

			 ViewTip vp;
			 if (theApp.g_viewTips.Find(TblProduceIndication, vp))
				 pCurView = (CYKSchedulerView*)vp.pWnd;
		 }
	 }
 }

 void CMainFrame::FindYKResourceSequence( UINT resId )
 {
	 CYKSchedulerView* pCurView = GetActiveView();
	 if (NULL != pCurView)
	 {
		 const UINT uTblType = pCurView->GetTblType();
		 if (TblResourceSequence == uTblType)
		 {
			 YKBizMap* pMap = BIZAPI::GetBizMap();
			 if (pMap != NULL)
			 {
				 vector<GridRowInfo>* pList = pCurView->GetFindRowKeyList();
				 pList->clear();
				 YKResourceSequencePtrMap* pProMap = pMap->GetYKResourceSequencePtrMap();
				 for (pProMap->Begin(); pProMap->NotEnd(); pProMap->Step())
				 {
					 YKResourceSequencePtr& ptr = pProMap->Get();
					 if (ptr != NULL && ptr->GetResPtr() != NULL && ptr->GetResPtr()->GetId() == resId)
					 {
						 GridRowInfo rowInfo;
						 rowInfo.id = ptr->GetId();
						 rowInfo.tblType = TblResourceSequence;
						 pList->push_back(rowInfo);
					 }
				 }
				 pCurView->Find(resId);
			 }
		 }
		 else
		 {
			 Node node;
			 theApp.g_spNodes.Find(TblResourceSequence, node);
			 if (node.pWnd != NULL)
			 {
				 FindActivitView(node.pWnd);
			 }
			 else
			 {
				 theApp.OpenTblPage(TblResourceSequence);
				 theApp.g_spNodes.Find(TblResourceSequence, node);
			 }

			 if (node.pWnd->IsKindOf(RUNTIME_CLASS(CYKSchedulerView)))
				 pCurView = (CYKSchedulerView*)node.pWnd;
		 }
	 }
 }
 void CMainFrame::FindYKCalendar( UINT calendarId )
 {
	 CYKSchedulerView* pCurView = GetActiveView();
	 if (NULL != pCurView)
	 {
		 const UINT uTblType = pCurView->GetTblType();
		 if (TblCalendar == uTblType)
		 {
			 YKBizMap* pMap = BIZAPI::GetBizMap();
			 if (pMap != NULL)
			 {
				 vector<GridRowInfo>* pList = pCurView->GetFindRowKeyList();
				 pList->clear();
				 YKCalendarPtrMap* pProMap = pMap->GetYKCalendarPtrMap();
				 for (pProMap->Begin(); pProMap->NotEnd(); pProMap->Step())
				 {
					 YKCalendarPtr& ptr = pProMap->Get();
					 if (ptr != NULL &&  ptr->GetId() == calendarId)
					 {
						 GridRowInfo rowInfo;
						 rowInfo.id = ptr->GetId();
						 rowInfo.tblType = TblCalendar;
						 pList->push_back(rowInfo);
						 break;
					 }
				 }
				 pCurView->Find(calendarId);
			 }
		 }
		 else
		 {
			 Node node;
			 theApp.g_spNodes.Find(TblCalendar, node);
			 if (node.pWnd != NULL)
			 {
				 FindActivitView(node.pWnd);
			 }
			 else
			 {
				 theApp.OpenTblPage(TblCalendar);
				 theApp.g_spNodes.Find(TblCalendar, node);
			 }

			 if (node.pWnd->IsKindOf(RUNTIME_CLASS(CYKSchedulerView)))
				 pCurView = (CYKSchedulerView*)node.pWnd;
		 }
	 }
 }

 //////////////////////////////////////////////////////////////////////////
 //下拉排程参数
 void CMainFrame::OnUpdateSelectApsComboBox(CCmdUI *pCmdUI)
 {
	 _LIMIT_CHECK_;
	 if (theApp.HideTable(TblApsParam))
		 return pCmdUI->Enable(FALSE);
	pCmdUI->Enable(theApp.m_bOpen);	 
 }
 void CMainFrame::InitApsComboBox()
 {
	 CBCGPToolbarComboBoxButton* pSrcCombo = NULL;

	 CObList listButtons;
	 if (CBCGPToolBar::GetCommandButtons (ID_SLECT_APS_COMBO, listButtons) > 0)
	 {
		 for (POSITION posCombo = listButtons.GetHeadPosition (); 
			 pSrcCombo == NULL && posCombo != NULL;)
		 {
			 CBCGPToolbarComboBoxButton* pCombo = 
				 DYNAMIC_DOWNCAST (CBCGPToolbarComboBoxButton, listButtons.GetNext (posCombo));

			 if (pCombo != NULL)
			 {
				 pSrcCombo = pCombo;
			 }
		 }
	 }

	 if (pSrcCombo != NULL)
	 {
		 if(!theApp.m_bOpen)
		 {
			 pSrcCombo->RemoveAllItems();
			return;
		 }
		 int index = pSrcCombo->GetCount();
		 for (int i = 0; i < index; i++)
			 pSrcCombo->DeleteItem(0);
		 wstring currentUseAps = BIZAPI::GetBizMap()->GetCurrentApsCode();
		 map<wstring,RunCell> cmdMap = BIZAPI::GetRunMethodRule2();
		 int nSel(-1),i = 0;
		 for ( map<wstring,RunCell>::iterator itor = cmdMap.begin();
			 itor!=cmdMap.end();itor++,i++)
		 {
			 pSrcCombo->AddItem(itor->first.c_str());	
			 if (currentUseAps == itor->first)
				 nSel = i;
		 }
		 if (nSel != -1)
			 pSrcCombo->SelectItem(nSel);
		 else pSrcCombo->SelectItem(-1);
		/* list<CString> codeList;
		 GetApsCodeList(codeList);
		 YKBizMap* pMap = BIZAPI::GetBizMap();
		 if (pMap == NULL)return;
		 YKApsParamPtrMap* pAps = pMap->GetYKApsParamPtrMap();
		 if (pAps->Empty()) return;
		 YKApsParamPtr curAps = pMap->GetApsParam();
		 if (curAps == NULL) return;
		 CString str(curAps->GetCode().c_str());
		 int nSel(-1),i = 0;
		 for (list<CString>::iterator i_code = codeList.begin();
			 i_code != codeList.end();i_code++,i++)
		 {
			 pSrcCombo->AddItem(*i_code);	
			 if (*i_code == str)
				 nSel = i;
		 }
		 if (nSel != -1)
		     pSrcCombo->SelectItem(nSel);*/
	 }
 }
 void CMainFrame::GetApsCodeList(list<CString>& lst)
 {
	 YKBizMap* pMap = BIZAPI::GetBizMap();
	 if (pMap != NULL)
	 {
		 YKApsParamPtrMap* pApsMap = pMap->GetYKApsParamPtrMap();
		 if (pApsMap != NULL)
		 for (pApsMap->Begin(); pApsMap->NotEnd(); pApsMap->Step())
		 {
			 YKApsParamPtr& ptr = pApsMap->Get();
			 if (ptr != NULL)
			 {
				 CString str(ptr->GetCode().c_str());
				 lst.push_back(str);
			 }
		 }
	 }
 }
 void CMainFrame::OnReplaceApsCombo()
 {
	 /*CBCGPToolbarComboBoxButton* pSrcCombo = NULL;

	 CObList listButtons;
	 if (CBCGPToolBar::GetCommandButtons (ID_SLECT_APS_COMBO, listButtons) > 0)
	 {
		 for (POSITION posCombo = listButtons.GetHeadPosition (); 
			 pSrcCombo == NULL && posCombo != NULL;)
		 {
			 CBCGPToolbarComboBoxButton* pCombo = 
				 DYNAMIC_DOWNCAST (CBCGPToolbarComboBoxButton, listButtons.GetNext (posCombo));

			 if (pCombo != NULL && 
				 CBCGPToolBar::IsLastCommandFromButton (pCombo))
			 {
				 pSrcCombo = pCombo;
			 }
		 }
	 }

	 if (pSrcCombo != NULL)
	 {
		 int index = pSrcCombo->GetCount();
		 for (int i = 0; i < index; i++)
			 pSrcCombo->DeleteItem(0);
		 list<CString> codeList;
		 GetApsCodeList(codeList);
		 for (list<CString>::iterator i_code = codeList.begin();
			 i_code != codeList.end();i_code++)
		 {
			 pSrcCombo->AddItem(*i_code);
		 }
	 }*/
 }
 void CMainFrame::OnSelectApsComboBox()
 {
	 CBCGPToolbarComboBoxButton* pSrcCombo = NULL;

	 CObList listButtons;
	 if (CBCGPToolBar::GetCommandButtons (ID_SLECT_APS_COMBO, listButtons) > 0)
	 {
		 for (POSITION posCombo = listButtons.GetHeadPosition (); 
			 pSrcCombo == NULL && posCombo != NULL;)
		 {
			 CBCGPToolbarComboBoxButton* pCombo = 
				 DYNAMIC_DOWNCAST (CBCGPToolbarComboBoxButton, listButtons.GetNext (posCombo));

			 if (pCombo != NULL && 
				 CBCGPToolBar::IsLastCommandFromButton (pCombo))
			 {
				 pSrcCombo = pCombo;
			 }
		 }
	 }

	 if (pSrcCombo != NULL)
	 {
		 ASSERT_VALID (pSrcCombo);

		 LPCTSTR lpszSelItem = pSrcCombo->GetItem ();
		 CString strSelItem = (lpszSelItem == NULL) ? _T("") : lpszSelItem;
		 YKBizMap* pMap = BIZAPI::GetBizMap();
		 if (pMap != NULL)
		 {
			 pMap->SetCurrentApsCode(strSelItem.GetString());
			 /*YKApsParamPtrMap* pApsMap = pMap->GetYKApsParamPtrMap();
			 for (pApsMap->Begin(); pApsMap->NotEnd(); pApsMap->Step())
			 {
				 YKApsParamPtr& ptr = pApsMap->Get();
				 if (ptr != NULL)
				 {
					 CString str(ptr->GetCode().c_str());
					 if (str == strSelItem)
					 {
						  pMap->SetApsParanID(ptr->GetId());
						  g_schCtr = FALSE;
						  break;
					 }
				 }
			 }*/
		 }
			

		 //AfxMessageBox (strSelItem);
	 }
	 //else
	 //{
		// AfxMessageBox (_T("error!"));
	 //}
 }

 void CMainFrame::OnAutoEnsureWork()
 {
	 // TODO: Add your command handler code here
	 CYKSchedulerView* pCurView = GetActiveView();
	 if (NULL != pCurView)
	 {
		 if (pCurView->GetTblType() > 0)
		 {
			 BIZAPI::GetBizMap()->GetSchSysParam()->SetCurApsParam();
			 pCurView->EnsureWorkInEnsureArea();
		 }
	 }
 }

 void CMainFrame::FindCode()
 {
	 YKFindAndReplaceDlgMgr::Get()->ShowFind(this);
 }

 void CMainFrame::ReplaceCode()
 {
	 YKFindAndReplaceDlgMgr::Get()->ShowReplace(this);
 }

 void CMainFrame::OnUpdateDelCom( CCmdUI *pCmdUI )
 {
	 _LIMIT_CHECK_
	 // add code
	 if(BIZAPI::EnterDataBaseCriSel())
	 {
		 if(theApp.IsGanttOpen()&& GanttCommonApi::HadSelectWork())
			 pCmdUI->Enable(TRUE);
		 else
			 pCmdUI->Enable(FALSE);
		 if( !BIZAPI::GetCommLimit(  TblOrder ) &&  
			 !BIZAPI::GetCommLimit(  TblWork ) )
			 pCmdUI->Enable(FALSE);
		 BIZAPI::LeaveDataBaseCriSel();
	 }
	pCmdUI->Enable(FALSE);
 }

 CYKSchedulerView* CMainFrame::GetActiveView()
 {
	 CYKSchedulerView* pView = NULL;

	 CMDIChildWnd* pChildWnd = MDIGetActive();
	 if (pChildWnd && pChildWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
		 pView = DYNAMIC_DOWNCAST(CYKSchedulerView, pChildWnd->GetActiveView());

	 return pView;
 }

 void CMainFrame::OnTogethorRun()
 {
	 BIZAPI::CombineWork();	
	 BIZAPI::GetBizMap()->RedrawAllGantt();
	 FlashGantt();
 }

 void CMainFrame::OnMultiSch()
 {
	OnYkScheduler();

	 CBCGPToolbarButton* button = m_calToolBar.GetButton(5);
	 if(g_schLevel == 0)
		 m_calToolBar.SetButtonInfo(5,button->m_nID,button->m_nStyle,m_imageId);
	 else if(g_schLevel>0 && g_schLevel< 5)
		m_calToolBar.SetButtonInfo(5,button->m_nID,button->m_nStyle,m_spareIcon.GetButton(g_schLevel-1)->GetImage());
 }

 void CMainFrame::OnStopMultiSch()
 {
	 g_schLevel = 0;
	 CBCGPToolbarButton* button = m_calToolBar.GetButton(5);
	 m_calToolBar.SetButtonInfo(5,button->m_nID,button->m_nStyle,m_imageId);
 }

void CMainFrame::OnEditCut()
{
	CYKSchedulerView* pCurView = GetActiveView();
	if (NULL != pCurView)
		pCurView->m_pGdCtrl.CTRL_X();
}
void CMainFrame::OnEditCopy()
{
	CYKSchedulerView* pCurView = GetActiveView();
	if (NULL != pCurView)
		pCurView->m_pGdCtrl.CTRL_C();
}
void CMainFrame::OnEditPaste()
{
	CYKSchedulerView* pCurView = GetActiveView();
	if (NULL != pCurView)
		pCurView->m_pGdCtrl.CTRL_V();
}
void CMainFrame::OnUpdateEditCut( CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_
	BOOL bEnable = FALSE;
	if(BIZAPI::EnterDataBaseCriSel())
	{
		CYKSchedulerView* pCurView = GetActiveView();
		if (NULL != pCurView)
			bEnable = pCurView->IsCanCut();

		BIZAPI::LeaveDataBaseCriSel();
	}

	pCmdUI->Enable(bEnable);
}
void CMainFrame::OnUpdateEditCopy( CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_
	BOOL bEnable = FALSE;

	if(BIZAPI::EnterDataBaseCriSel())
	{
		CYKSchedulerView* pCurView = GetActiveView();
		if (NULL != pCurView)
			bEnable = pCurView->IsCanCopy();
		BIZAPI::LeaveDataBaseCriSel();
	}

	pCmdUI->Enable(bEnable);
}
void CMainFrame::OnUpdateEditPaste( CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_

	BOOL bEnable = FALSE;
	if(BIZAPI::EnterDataBaseCriSel())
	{
		CYKSchedulerView* pCurView = GetActiveView();
		if (NULL != pCurView)
			bEnable = pCurView->IsCanPaste();
		BIZAPI::LeaveDataBaseCriSel();
	}
	
	pCmdUI->Enable(bEnable);
}

void CMainFrame::PopModifyDlg(WPARAM wParam, LPARAM lParam)
{
	CModifyDlg  m_dlgModify;
	CString str;
	MSG* ms = (MSG*)wParam;
	str = GetRemarkFiled(UINT(ms->message)).c_str();
#ifdef PMCMODE
	//Add 2012-01-18 控制BOM视图中工序弹出对话框
	if ( ms->message == TblOperation )
	{
		return;
	}
#endif
	m_dlgModify.SetText(str,L"Go",L"Out");
	m_dlgModify.SetTbl((UINT)ms->message,(UINT) ms->wParam);
	m_dlgModify.SetModefyType((short)lParam);
	if (ms->message==TblBOM)
	{
		YKBOMMap* bomMap=BIZAPI::GetBizMap();
		if (NULL != bomMap)
		{
			YKBOMPtr& bomPtr=bomMap->Get((YK_ULONG)ms->wParam);
			if (NULL != bomPtr)
				m_dlgModify.SetBOMCmdType(bomPtr->GetCmdType());
		}
	}
	if (ms->lParam > 0)
		m_dlgModify.SetActTab((int)ms->lParam);
	m_pDlgList.push_back(&m_dlgModify);
	if (m_pDlgList.size() == 2)
	{
		m_pDlgList.front()->GetWindowRect(&m_lastDlgRect);
		//m_pDlgList.front()->ClientToScreen(&m_lastDlgRect);
		//CRect rcClient;
		//GetClientRect(&rcClient);
		//ClientToScreen(&rcClient);
		//long width = m_lastDlgRect.Width();
		//long heigth = m_lastDlgRect.Height();
		//m_lastDlgRect.left = m_lastDlgRect.left - rcClient.left;
		//m_lastDlgRect.top = m_lastDlgRect.top - rcClient.top;
		//m_lastDlgRect.bottom = m_lastDlgRect.top + heigth;
		//m_lastDlgRect.right = m_lastDlgRect.left + width;
	}
	if (m_pDlgList.size() > 1)
	{
		CRect rcClient;
		rcClient.left = m_lastDlgRect.left + 20;
		rcClient.top = m_lastDlgRect.top + 20;
		rcClient.bottom = rcClient.top + m_lastDlgRect.Height();
		rcClient.right = rcClient.left + m_lastDlgRect.Width();
		m_lastDlgRect = rcClient;
		m_dlgModify.SetClientRect(rcClient);
	}
	//TimeLog::WriteLog("0,beforeDoModal \n");
	m_flashDataFlag=FALSE;
	//TimeLog::WriteLog("1,beforeDoModal \n");
	if (m_dlgModify.DoModal()==IDOK)
	{
		m_flashDataFlag=TRUE;
		//TimeLog::WriteLog("0,beforeErase \n");
		m_pDlgList.erase(--m_pDlgList.end());
		//TimeLog::WriteLog("0,AfterErase \n");
		if (!m_pDlgList.empty())
		{
			list<CModifyDlg*>::reverse_iterator i_pDlg = m_pDlgList.rbegin();
			//i_pDlg++;
			//TimeLog::WriteLog("0,BeforeModifyDlg \n");
			CModifyDlg* pWnd = *i_pDlg;
			if (pWnd->GetSafeHwnd())
			{
				//TimeLog::WriteLog("0,BeforeUpdate \n");
				pWnd->Update(TRUE);
			}
		}
	}
	else
	{
		//TimeLog::WriteLog("DoModal-NOTOK \n");
		m_pDlgList.erase(--m_pDlgList.end());
		return;
	}
	m_flashDataFlag=TRUE;

	if (ms->message==TblSchSysParam
		||ms->message == TblFieldItem)
	{
		m_wndOutput.Flash();
		m_wndWatchBar.Update();
	}
	if (ms->message == TblFieldItem)
	{
		CMDIChildWnd* child = MDIGetActive();
		if (child != NULL)
		{
			CYKSchedulerView* view = (CYKSchedulerView*)child->GetActiveView();
			if (view != NULL)
				view->Flash();
		}
	}
}

// 导入导出  新建 打开 展开  排程  信息提示  add 2010-4-16
void CMainFrame::SetMsgInfo( YK_ULONG infoId,CString& code ,BOOL b,long msgType,bool flashMessageGrid)
{
	CMessageData mesData; mesData->New();
	if(ID_UI_MSG_TYPE_BLANK == infoId)
		mesData.SetLevel(MSG_LEVEL_BLANK);
	mesData.SetType((MSG_TYPE)msgType);
	YK_WSTRING wstr=L":";
	if(!b) wstr=L"";
	mesData.SetMessageCode(GetRemarkFiled(infoId)+wstr+code.GetString());
	// 时间
	mesData.SetTime(TOOLS::FormatTime(CDealTime::GetLocalTime(),true));
	/*if(flashMessageGrid)
		::SendMessage(GetSafeHwnd(),ID_MESSAGE_FLASH_OUTPUTWND,0,0);*/
}
// 评价信息  参数：级别  消息内容  对象ID	
void CMainFrame::SetMsgInfo(MSG_LEVEL level,CString& msgCode,YK_ULONG Objid,long msgType,bool flashMessageGrid)
{
	CMessageData mesData; mesData->New();
	// 消息级别
	mesData.SetLevel(level);
	// 显示信息（消息内容）
	mesData.SetMessageCode(msgCode.GetString());
	// 类别/分类（消息发生的位置）
	mesData.SetType((MSG_TYPE)msgType);		// 逻辑
	// 时间
	mesData.SetTime(TOOLS::FormatTime(CDealTime::GetLocalTime(),true));
	// 对象
	mesData.SetTbl(TblRestrictInfo);
	if(Objid>0)
		mesData.AddRelId(Objid);
	if(flashMessageGrid)
		::SendMessage(GetSafeHwnd(),ID_MESSAGE_FLASH_OUTPUTWND,0,0);
};
// 评价信息  参数：级别  消息内容  对象ID	
void CMainFrame::SetMsgInfo(MSG_LEVEL level,CString& msgCode,YK_ULONG ObjTbl,list<YK_ULONG>& objIdlist,long msgType,bool flashMessageGrid)
{
	CMessageData mesData; mesData.New();
	// 消息级别
	mesData.SetLevel(level);
	// 显示信息（消息内容）
	mesData.SetMessageCode(msgCode.GetString());
	// 类别/分类（消息发生的位置）
	mesData.SetType((MSG_TYPE)msgType);		// 逻辑
	// 时间
	mesData.SetTime(TOOLS::FormatTime(CDealTime::GetLocalTime(),true));
	// 对象
	mesData.SetTbl(TblRestrictInfo);
	// 新的对象iD列表
	mesData.SetObjTbl(ObjTbl);
	
	mesData.SetIdlist(objIdlist);
};
 // 显示排程后消息   isShow：是否显示评价信息  add 2010-4-16
void CMainFrame::ShowSchedulerEndInfo( bool isShow ,double useTm)
{
	// 显示评价信息
	YKBizMap*pMap = BIZAPI::GetBizMap();
	if (pMap == NULL) return;
	if(isShow)
	{
		map<UINT,BOOL> restrictFlgList;
		BIZAPI::SetTableType(TblApsParam);
		BIZAPI::GetRestrictShowFlgList(restrictFlgList);
		YKRestrictInfoPtr& temp = pMap->GetLastOneRestrictInfo();
		if(temp != NULL)
		{
			BIZAPI::SetTableType(TblRestrictInfo);
			int index = YKRestrictInfoStruct::j_SellOrderAmount;
			// 个数   提示信息  需要设置新的对象
			for(index =  YKRestrictInfoStruct::j_SellOrderAmount;
				index <=  YKRestrictInfoStruct::j_WorkAmount; index++)
			{
				if (!restrictFlgList[index]) continue;
				CString strInfo;
				UINT type = 0;
				YK_ULONG objTbl=TblOrder;
				if (index==YKRestrictInfoStruct::j_WorkAmount)         // 输入品目不足工作数	
				{
					objTbl=TblWork;
				}
				
				//Add 2011-10-24
				wstring wstr =  BIZAPI::GetValue(TblRestrictInfo,temp->GetId(),index);
				UINT number = _wtoi((wchar_t*)wstr.c_str());
				strInfo.Format(L"%s:%d",BIZAPI::GetFieldCode(TblRestrictInfo,index).c_str(),number);
				strInfo = MSG_BLANK+strInfo;

				SetMsgInfo(MSG_LEVEL_MESSAGE,strInfo,objTbl,temp->GetInfo().GetObjIdList(index),MSG_TYPE_SCHEDUL,false);
			}
			// 个数   提示/警告信息  需要设置新的对象
			for(index =  YKRestrictInfoStruct::j_OverdueSellOrderAmount;
				index <=  YKRestrictInfoStruct::j_InputItemWorkAmount; index++)
			{
				if (!restrictFlgList[index]) continue;
				CString strInfo;
				UINT type = 0;
				YK_ULONG objTbl=TblOrder;
				if (index==YKRestrictInfoStruct::j_NotAllotWorkData||            // 未分派工作数据	
					index==YKRestrictInfoStruct::j_VirResAllotWorkAmount||       // 虚拟资源分派工作数	
					index==YKRestrictInfoStruct::j_ForceAllotWorkAmount||        // 强制分派工作数	)
					index==YKRestrictInfoStruct::j_ResWorkAmount||               // 违反资源量制约工作数	
					index==YKRestrictInfoStruct::j_TimeWorkAmount||              // 违反时间制约工作数	
					index==YKRestrictInfoStruct::j_InputItemWorkAmount)         // 输入品目不足工作数	
				{
					objTbl=TblWork;
				}
				
				/*
				_variant_t var;
				BIZAPI::GetValue(TblRestrictInfo,temp->GetId(),index,var,type);
				strInfo.Format(L"%s: %d",BIZAPI::GetFieldCode(TblRestrictInfo,index).c_str(),var.intVal);
				strInfo=MSG_BLANK+strInfo;				

				MSG_LEVEL msgLevel=MSG_LEVEL_MESSAGE;
				if (var.intVal>0)
					msgLevel=MSG_LEVEL_WARNING;
				SetMsgInfo(msgLevel,strInfo,objTbl,temp->GetObjIdList(index),MSG_TYPE_SCHEDUL,false);
				var.Detach();
				*/

				//Add 2011-10-24
				wstring wstr = BIZAPI::GetValue(TblRestrictInfo,temp->GetId(),index);
				UINT number = _wtoi((wchar_t*)wstr.c_str());
				strInfo.Format(L"%s: %d",BIZAPI::GetFieldCode(TblRestrictInfo,index).c_str(),number);
				strInfo=MSG_BLANK+strInfo;
				MSG_LEVEL msgLevel=MSG_LEVEL_MESSAGE;
				if (number > 0)
				{
					msgLevel = MSG_LEVEL_WARNING;
				}
				SetMsgInfo(msgLevel,strInfo,objTbl,temp->GetInfo().GetObjIdList(index),MSG_TYPE_SCHEDUL,false);
			}
			// 时间/浮点数   提示信息
			for (index= YKRestrictInfoStruct::j_OrderProAverage;index <=  YKRestrictInfoStruct::j_OrderOverdueMax;index++)
			{
				if (!restrictFlgList[index]) continue;
				CString strInfo;

				//UINT type = 0;
				//_variant_t var;
				//BIZAPI::GetValue(TblRestrictInfo,temp->GetId(),index,var,type);
				//// VT_I4  
				//if(index == YKRestrictInfoStruct::j_ItemChangeAmount)
				//	strInfo.Format(L"%s: %d",BIZAPI::GetFieldCode(TblRestrictInfo,index).c_str(),var.lVal);
				//else 
				//	strInfo.Format(L"%s: %0.2f",BIZAPI::GetFieldCode(TblRestrictInfo,index).c_str(),var.dblVal);
				//strInfo=MSG_BLANK+strInfo;

				//Add 2011-10-24
				wstring wstr = BIZAPI::GetValue(TblRestrictInfo,temp->GetId(),index);
				YK_ULONG nLong = _wtol((wchar_t*)wstr.c_str());
				YK_FLOAT nFloat = _wtof((wchar_t*)wstr.c_str());
				if (YKRestrictInfoStruct::j_ItemChangeAmount == index )
				{
					strInfo.Format(L"%s: %d",BIZAPI::GetFieldCode(TblRestrictInfo,index).c_str(),nLong);
				}
				else
				{
					strInfo.Format(L"%s: %0.2f",BIZAPI::GetFieldCode(TblRestrictInfo,index).c_str(),nFloat);
				}
				strInfo = MSG_BLANK+strInfo;
				
				// 负荷率添加详细信息  test用
				if (index == YKRestrictInfoStruct::j_OnusaAverage&&FALSE)
				{
					CString cstr_=L"\r\n";
					cstr_+=temp->GetInfo().GetResRateInfo().c_str();
					CString useTmCode;
					useTmCode.Format(L"\r\nUseTm:%f",temp->GetInfo().GetUseTm());
					if(cstr_.GetAllocLength()<800)
						strInfo+=cstr_;
					strInfo+=useTmCode;
				}
				SetMsgInfo(MSG_LEVEL_MESSAGE,strInfo,temp->GetId(),MSG_TYPE_SCHEDUL,false);
				//var.Detach();
			}
		}
	}
	else		// 计算一些简单的排程信息
	{
		YKRestrictInfo  restrictInfo; restrictInfo.New();
		/* 
		// 逾期销售订单数  逾期制造订单数   逾期采购订单数   
		// 违反最早开工制造订单数   违反最早完工制造订单数  违反最晚开工制造订单数  
		// 制造订单提前期平均
		// 交货期余裕时间合计值 
		// 订单逾期时间合计值
		*/
		restrictInfo.CalComData();
		YK_ULONG showList[]={
			YKRestrictInfoStruct::j_SellOrderAmount,             // 销售定单数 +
			YKRestrictInfoStruct::j_ManuOrderAmount,             // 制造订单数	+
			YKRestrictInfoStruct::j_BuyOrderAmount,              // 采购订单数	+
			YKRestrictInfoStruct::j_StoreAbsOrderAmount,         // 库存定单（绝对量）数 +
			YKRestrictInfoStruct::j_StoreRelOrderAmount,         // 库存定单（相对量）数 +
			YKRestrictInfoStruct::j_ProOrderAmount,              // 维护定单数 +
			YKRestrictInfoStruct::j_OverdueSellOrderAmount,      // 逾期销售订单数	
			YKRestrictInfoStruct::j_OverdueManuOrderAmount,      // 逾期制造订单数	
			YKRestrictInfoStruct::j_OverdueBuyOrderAmount,       // 逾期订单数	
			YKRestrictInfoStruct::j_EarlyStartOrderAmount,       // 违反最早开工制造订单数	
			YKRestrictInfoStruct::j_EarlyCompleteOrderAmount,    // 违反最早完工制造订单数	
			YKRestrictInfoStruct::j_LastStartOrderAmount,        // 违反最晚开工制造订单数	

			YKRestrictInfoStruct::j_WorkAmount,                  // 工作数 +
			YKRestrictInfoStruct::j_NotAllotWorkData,            // 未分派工作数据	
			YKRestrictInfoStruct::j_VirResAllotWorkAmount,       // 虚拟资源分派工作数	
			YKRestrictInfoStruct::j_ForceAllotWorkAmount,        // 强制分派工作数	
			YKRestrictInfoStruct::j_InputItemWorkAmount,		// 输入品目不足数量
		};

		// 个数提示信息
		//CBizDateManager dtaManager;
		for(int i = 0; i < sizeof(showList)/sizeof(YK_ULONG); ++i)
		{
			YK_ULONG objTbl = TblOrder;
			if (showList[i] == YKRestrictInfoStruct::j_WorkAmount||                  // 工作数 +
				showList[i] == YKRestrictInfoStruct::j_NotAllotWorkData||            // 未分派工作数据	
				showList[i] == YKRestrictInfoStruct::j_VirResAllotWorkAmount||       // 虚拟资源分派工作数	
				showList[i] == YKRestrictInfoStruct::j_ForceAllotWorkAmount||
				showList[i] == YKRestrictInfoStruct::j_InputItemWorkAmount)		// 输入品目不足数量)        // 强制分派工作数	)
			{
				objTbl = TblWork;
			}

			//Add 2011-10-27 
			_variant_t var;
			CString  strInfo;
			wstring wstr;
			//unsigned long id = restricPtr->GetId();
			unsigned long  id = restrictInfo.GetId();

			wstr = BIZAPI::GetValue(TblRestrictInfo, id, showList[i]);
			UINT inputInt = _wtoi((wchar_t*)wstr.c_str());

			strInfo.Format(L"%s: %d",BIZAPI::GetFieldCode(TblRestrictInfo,showList[i]).c_str(),inputInt);
			strInfo = MSG_BLANK + strInfo;

			MSG_LEVEL msgLevel=MSG_LEVEL_MESSAGE;
			if (inputInt > 0)
			{
				msgLevel = restrictInfo->GetInfo().GetLevel(showList[i])>0?MSG_LEVEL_WARNING:MSG_LEVEL_MESSAGE; 
			}
			SetMsgInfo(msgLevel,strInfo,objTbl,restrictInfo->GetInfo().GetObjIdList(showList[i]),MSG_TYPE_SCHEDUL,false);
		}
		restrictInfo.Free();
	}
	
	// 排程结束
	CString cstrCode=L"";
	cstrCode.Format(L"( %0.2lf %s)",useTm,GetRemarkFiled(ID_UI_BAR_SECOND).c_str());  // L"排程结束"
	SetMsgInfo(ID_UI_BAR_SCHEDULEND,cstrCode,true,MSG_TYPE_SCHEDUL,false);
	::SendMessage(GetSafeHwnd(),ID_MESSAGE_FLASH_OUTPUTWND,0,0);

}
void CMainFrame::OnYkChinese()
{
	// TODO: Add your command handler code here
	BIZAPI::GetBizMap()->SetTimeShowType(10);
	theApp.m_hinstBCGCBRes = LoadLibrary (_T("YKResCHS.dll"));    // *** - language
	BCGCBProSetResourceHandle (theApp.m_hinstBCGCBRes);
	if (theApp.m_hinstResChs)
	AfxSetResourceHandle(theApp.m_hinstResChs);
	theApp.m_langueType = LANGUAGE_CHS;
	RestLanguage();
	theApp.ChangeTitle();
}

void CMainFrame::OnYkEnglish()
{
	// TODO: Add your command handler code here
	BIZAPI::GetBizMap()->SetTimeShowType(20);
	theApp.m_hinstBCGCBRes = LoadLibrary (_T("55252.dll"));    // *** - language
	BCGCBProSetResourceHandle (theApp.m_hinstBCGCBRes);
	theApp.m_hinstRes = LoadLibrary (_T("YKResENG.dll"));    // *** - language
	if (theApp.m_hinstRes)
	AfxSetResourceHandle(theApp.m_hinstRes);
	theApp.m_langueType = LANGUAGE_US;
	RestLanguage();
	theApp.ChangeTitle();
}

void CMainFrame::OnUpdateYkChinese(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(theApp.m_langueType != LANGUAGE_CHS);
	//pCmdUI->Enable(false);
	pCmdUI->SetCheck(theApp.m_langueType == LANGUAGE_CHS);
}

void CMainFrame::OnUpdateYkEnglish(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(theApp.m_langueType != LANGUAGE_US);
	//pCmdUI->Enable(false);
	pCmdUI->SetCheck(theApp.m_langueType == LANGUAGE_US);
}

void CMainFrame::RestLanguage()
{
	const YK_ULONG lt = theApp.m_langueType;
	SetLanguageType(lt);
	//DATATABLAPI::Clear();
	//LANGUAGEPACKAPI::InitTableInfo();
	//DATATABLAPI::InitTableModeManager(theApp.m_curProPath.GetString());
	m_wndMenuBar.ResetAll();
	BIZAPI::InitFiledMap();
	RestToolsText();
	RecalcLayout ();
	RestWnd();
	// 重置语言转换
	BIZAPI::GetBizMap()->ResetConditionCodeTrans();
	BIZAPI::GetBizMap()->ResetNewRuleLanguage();
	if (m_msgConDlg.GetSafeHwnd())
	{
		m_msgConDlg.InitControlView();
	}
	RedrawWindow (NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	CYKSchedulerView* pCurView = GetActiveView();
	if (NULL != pCurView)
		pCurView ->Flash();
}

void CMainFrame::RestToolsText()
{
	theApp.SetMainFrmText();
	CString strMainToolbarTitle;
	CString strGantt,strSch,strCal,StrSatus,strFilter,strDispatch,strWillOrder,strTogethor;

	strMainToolbarTitle.LoadString (IDS_MAIN_TOOLBAR);
	strGantt.LoadString (IDS_TOOL_BAR_GANTT);
	strSch.LoadString (IDS_TOOL_BAR_SCH);
	strCal.LoadString (IDS_TOOL_BAR_CAL);
	StrSatus.LoadString (IDS_STATUS_STRING);
	strFilter.LoadString(IDS_TOOL_BAR_FILTER);
	strDispatch.LoadString(IDS_TOOL_BAR_DISPATCH);
	strWillOrder.LoadString(IDS_TOOL_BAR_WILL_ORDER);
	strTogethor.LoadString(IDS_TOOL_TOGETHOR);
	m_wndToolBar.SetWindowText (strMainToolbarTitle);
	m_calToolBar.SetWindowText (strCal);
	m_schToolBar.SetWindowText (strSch);
	m_ganttToolBar.SetWindowText (strGantt);
	m_filterToolBar.SetWindowText(strFilter);
	m_dispatchToolBar.SetWindowText(strDispatch);
	m_wndStatusBar.SetWindowText (StrSatus);

#ifndef PMCMODE
	CString netStr(GetRemarkFiled(ID_UI_NET_TOOLBAR).c_str());
	m_netToolBar.SetWindowText (netStr);
#endif
	//临时删除一起生产控件
	//m_togethorToolBar.SetWindowText(strTogethor);


	CString strSelDifine = GetRemarkFiled(ID_UI_TEXT_SELFDEFINE).c_str();
	if (m_dispatchToolBar.GetCustomizeButton() != NULL)
		m_dispatchToolBar.GetCustomizeButton()->m_strText = strSelDifine;
	if (m_wndToolBar.GetCustomizeButton() != NULL)
		m_wndToolBar.GetCustomizeButton()->m_strText = strSelDifine;
	if (m_calToolBar.GetCustomizeButton() != NULL)
		m_calToolBar.GetCustomizeButton()->m_strText = strSelDifine;
	if (m_ganttToolBar.GetCustomizeButton() != NULL)
		m_ganttToolBar.GetCustomizeButton()->m_strText = strSelDifine;
	if (m_schToolBar.GetCustomizeButton() != NULL)
		m_schToolBar.GetCustomizeButton()->m_strText = strSelDifine;
	if (m_filterToolBar.GetCustomizeButton() != NULL)
		m_filterToolBar.GetCustomizeButton()->m_strText = strSelDifine;
	//临时删除一起生产控件
	//if (m_togethorToolBar.GetCustomizeButton() != NULL)
	//	m_togethorToolBar.GetCustomizeButton()->m_strText = strSelDifine;
#ifndef PMCMODE
	if (m_netToolBar.GetCustomizeButton() != NULL)
		m_netToolBar.GetCustomizeButton()->m_strText = strSelDifine;
#endif
}

void CMainFrame::RestWnd()
{
	//树
	m_wndWorkSpace.ResetWnd();
	//属性
	m_wndWatchBar.ResetWnd();
	//输出
	m_wndOutput.ResetWnd();
	//view
	CMDIChildWnd* child = MDIGetActive();
	while (child)
	{

		if (child->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
		{
			CYKSchedulerView* view = (CYKSchedulerView*)child->GetActiveView();
			if(view != NULL)
			{
				view->ResetView();
			}
			//child->MDIActivate();        // or MDIActivate(child);
		}	
		child = (CMDIChildWnd*) child->GetWindow(GW_HWNDNEXT);
	}
}

void CMainFrame::OnCustomizeFilter()
{
	CYKSchedulerView* pCurView = GetActiveView();
	if (NULL != pCurView)
	{
		YK_ULONG uTblType = pCurView->GetTblType();
		CNewRuleDlg dlg;
		dlg.InitRule(RULE_DEF_FILTER, GetFilterCode(uTblType), uTblType);
		if(dlg.DoModal() == IDOK)
		{
			InsertFilter(pCurView->GetTblType(), dlg.GetCode());
			BIZAPI::SetTableFlashFlg(TRUE, pCurView->GetTblType());
			if(uTblType == ID_GANTT_RES_GL && BIZAPI::GetBizMap()->GetGanttPara()->GetFilterType() == 1)
			{
				BIZAPI::GetBizMap()->GetGanttPara()->SetFilterWork(true);
				BIZAPI::GetBizMap()->GetGanttPara()->SetFilterWorkCustomRule(true);
			}
			FlashGantt();
		}
	}

}

void CMainFrame::OnUpdateCustomizefilter(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_
	// TODO: Add your command update UI handler code here
	BOOL bEnable = FALSE;
	CYKSchedulerView* pCurView = GetActiveView();

	if (NULL != pCurView)
	{
		const UINT uTblType = pCurView->GetTblType();
		//甘特图
		if(ID_GANTT_RES_GL == uTblType || ID_GANTT_ORDER_GL == uTblType
			|| ID_RATE_RES_GL == uTblType
			|| ID_RATE_STA_GL == uTblType
			|| ID_WORK_RESOURCE_GD == uTblType)
		{
			bEnable = TRUE;
		}
		else
		{
			BOOL bOK = TRUE;
			if (uTblType == TblLoadFileInfo || 
				uTblType == TblSchSysParam || 
				uTblType == TblGanttPara)
			{
				bOK = FALSE;
			}

			bEnable = pCurView->m_bIsTalbe && bOK;
		}
	}

	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnCustomizesort()
{
	// TODO: Add your command handler code here

	CYKSchedulerView* pCurView = GetActiveView();
	if (pCurView)
	{
		CSortDlg dlgSort;
		dlgSort.SetTable(pCurView->GetTblType());
		if(dlgSort.DoModal() == IDOK)
		{
			pCurView->Flash(FALSE);
			pCurView->FlexGridSort();
		}
	}
}

void CMainFrame::OnUpdateCustomizesort(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_
	// TODO: Add your command update UI handler code here
	BOOL bEnable = FALSE;

	CYKSchedulerView* pCurView = GetActiveView();
	if (NULL != pCurView)
	{
		BOOL bOK = TRUE;
		const UINT uTblType = pCurView->GetTblType();
		if (uTblType == TblLoadFileInfo || uTblType == TblSchSysParam || 
			uTblType == TblGanttPara || uTblType == TblBOM)
		{
			bOK = FALSE;
		}

		bEnable = pCurView->m_bIsTalbe && bOK && (!m_bFGThread);
	}

	pCmdUI->Enable(bEnable);
}

void CMainFrame::MouseDown()
{
	m_wndMenuBar.ClosePopdownMenu();
}

void CMainFrame::GLFilter(UINT tblType,vector<UINT>& pList)
{
	vector<unsigned int>  idList;
	UINT rTblType=0;
	if (tblType ==  ID_GANTT_RES_GL
		||tblType ==  ID_RATE_RES_GL
		||tblType ==  ID_WORK_RESOURCE_GD
		|| tblType == ID_Item_Gather_GL)
	{
		BIZAPI::Init(TblResource,idList);
		rTblType=TblResource;
	}
	else if(tblType == ID_GANTT_ORDER_GL)
	{
		BIZAPI::Init(TblOrder,idList);
		rTblType=TblOrder;
	}
	else if(tblType == ID_RATE_STA_GL)
	{
		BIZAPI::Init(TblItem,idList);
		rTblType=TblItem;
	}
	else 
		return;
	CNewRuleManager rulManager(RULE_DEF_FILTER,GetFilterCode(tblType),rTblType);
	for (vector<UINT>::iterator itor=idList.begin();itor!=idList.end();itor++)
	{
		if (rulManager.GetBoolValue((YK_ULONG)(*itor)))
		{
			pList.push_back(*itor);
		}
	}
}
void CMainFrame::OnPaletteSetup()
{
	// TODO: Add your command handler code here
	CModifyDlg  m_dlgModify;
	CString str;
	str = GetRemarkFiled(UINT(TblPallette)).c_str();
	m_dlgModify.SetText(str,L"Go",L"Out");
	m_dlgModify.SetTbl((UINT)TblPallette,(UINT) BIZAPI::GetBizMap()->Getid());
	m_dlgModify.DoModal();

	CYKSchedulerView* pCurView = GetActiveView();
	if (NULL != pCurView)
	{
		const UINT uTblType = pCurView->GetTblType();
		BIZAPI::SetTableFlashFlg(TRUE, uTblType);
		FlashGantt();
	}
}

void CMainFrame::OnUpdatePaletteSetup(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
		pCmdUI->Enable(theApp.m_bOpen);
}

void CMainFrame::LoadWindowsState()
{
	CWindowsStateOper::RestoreReg();
	ReLoadFrame();
}

void CMainFrame::SaveWindowsState()
{	
	//清理上次保存参数
	CWindowsState::Clear();
	ReCloseFrame();

	CWindowsStateOper::SaveReg();


	//打开的窗口的列表
	for(theApp.g_viewTips.Begin();theApp.g_viewTips.NotEnd();theApp.g_viewTips.Step())
	{
		ViewTip& viewTip = theApp.g_viewTips.Get();

		CWindowsState::SetWindowsState( viewTip.tblType,TRUE,FALSE );

	}

	//当前窗口
	CYKSchedulerView* pCurView = GetActiveView();
	if (NULL != pCurView)
	{
		//记录关闭窗口的状态
		CWindowsState::SetWindowsState(pCurView->GetTblType(), TRUE, TRUE);
	}
}

void CMainFrame::OnYkItemGather_GL()
{
#ifndef _REMOVE_ITEMGATHER_
	theApp.OpenTblPage(ID_Item_Gather_GL);
#endif
}

void CMainFrame::OnYkItemGather_Grid()
{
#ifndef _REMOVE_ITEMGATHER_
	theApp.OpenTblPage(TblItemGather);
#endif
}
void CMainFrame::OnConflictsolve()
{
	// TODO: Add your command handler code here
	//YKBizMap*ptrMap=BIZAPI::GetBizMap();

	//if (!DATATABLAPI::GetNetWorkConflictSolve())
	//{
	//	CString strMessage(_T("获取冲突信息失败!\n"));
	//	YK_WSTRING strLastError = DATATABLAPI::GetNetWorkLastError();
	//	strMessage += strLastError.c_str();
	//}
	theApp.OpenTblPage(TblManufactureDemand);
	//theApp.OpenTblPage(TblOrder);
}

void CMainFrame::OnUpdateConflictsolve(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
		pCmdUI->Enable(theApp.m_bOpen);
}

void CMainFrame::OnYkDayShift_Grid()
{
	theApp.OpenTblPage(ID_DayShift_GD);
}

LRESULT CMainFrame::OnFlexGridUpdate(WPARAM wParam, LPARAM lParam)
{
	CDataGridCtrl* pDataGrid = (CDataGridCtrl*)wParam;
	const BOOL bFlashed = (BOOL)lParam;

	if (pDataGrid->GetSafeHwnd())
	{
		pDataGrid->UpdateFlexGrid(bFlashed);
	}

	return 0;
}
LRESULT CMainFrame::OnFlexGridThread(WPARAM wParam, LPARAM lParam)
{
	m_bFGThread = (BOOL)lParam;
	if (FALSE == m_bFGThread)
		Sleep(100);

	return 0;
}
LRESULT CMainFrame::OnSBProgressRange(WPARAM wParam, LPARAM lParam)
{
	if (!m_progressCtl.IsWindowVisible())
		m_progressCtl.ShowWindow(SW_SHOW);

	int nLower = 0, nUpper = 0;
	nLower = (int)wParam;
	nUpper = (int)lParam;
	m_progressCtl.SetRange32(nLower, nUpper);
	m_progressCtl.SetPos(nLower);
	return 0;
}
LRESULT CMainFrame::OnSBProgressPos(WPARAM wParam, LPARAM lParam)
{
	int nLower = 0, nUpper = 0;
	m_progressCtl.GetRange(nLower, nUpper);

	const int nPos = (int)lParam;
	if (nPos >= nLower && nPos <= nUpper && m_progressCtl.IsWindowVisible())
	{
		if (m_progressCtl.GetPos() == nLower)
			m_labelSBProgress.SetWindowText(m_strProgressLabel);

		m_progressCtl.SetPos(nPos);

		if (nUpper == nPos)
		{
			m_progressCtl.ShowWindow(SW_HIDE);
			SetSBProgressLabel(_T(""));
			m_labelSBProgress.SetWindowText(m_strProgressLabel);
		}
	}
	return 0;
}

LRESULT CMainFrame::OnSBProgressCancel(WPARAM wParam, LPARAM lParam)
{
	if (m_progressCtl.IsWindowVisible())
	{
		m_progressCtl.ShowWindow(SW_HIDE);
		SetSBProgressLabel(_T(""));
	}

	return 0;
}

void CMainFrame::SetSBProgressLabel(const CString& strLabel)
{
	m_strProgressLabel = strLabel;

	if (!m_strProgressLabel.IsEmpty())
		m_strProgressLabel += _T("    ");
}
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CMyMDIFrameWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	if (m_wndStatusBar.GetSafeHwnd())
	{
		CRect rcProgress, rcLabel;
		m_wndStatusBar.GetItemRect(1, &rcProgress);
		rcLabel = rcProgress;
		rcLabel.right -= 100;
		m_labelSBProgress.MoveWindow(&rcLabel);

		rcProgress.left += 150;
		m_progressCtl.MoveWindow(&rcProgress);
	}
}


void CMainFrame::RecalcLayout(BOOL bNotify)
{
	// TODO: Add your specialized code here and/or call the base class
	CMyMDIFrameWnd::RecalcLayout(bNotify);
}

void CMainFrame::PMCToolBar()
{
#ifdef PMCMODE
	if(m_calToolBar.GetCount() == 4)
	{
		m_calToolBar.RemoveButton(1);
		m_calToolBar.AdjustLayout();
		m_calToolBar.AdjustSizeImmediate();
	}
	if(m_schToolBar.GetCount() == 10)
	{
		m_schToolBar.RemoveButton(5);
		m_schToolBar.RemoveButton(5);
		m_schToolBar.RemoveButton(5);
		m_schToolBar.AdjustLayout();
		m_schToolBar.AdjustSizeImmediate();
	}

	if(m_ganttToolBar.GetCount() == 6)
	{
		m_ganttToolBar.RemoveButton(3);
		m_ganttToolBar.AdjustLayout();
		m_ganttToolBar.AdjustSizeImmediate();
	}

#endif
}

void CMainFrame::OnPaint()
{
	PMCToolBar();
	CPaintDC dc(this); // device context for painting
}

void CMainFrame::ChangeMenuType(UINT MenuID)
{
	m_wndMenuBar.SetDefaultMenuResId(MenuID);
	m_wndMenuBar.ResetAll();
}


void CMainFrame::OnShouce()
{
	WCHAR wPath[256];
	GetModuleFileName(NULL,wPath,256);
	CString curProPath = theApp.GetFilePath(CString(wPath));
	curProPath += L"\\永凯APS教学版快速入门.pdf";
	::ShellExecute(NULL,NULL,curProPath,NULL,NULL,SW_SHOW);
}

void CMainFrame::OnVcrpdf()
{
	//WCHAR wPath[256];
	//GetModuleFileName(NULL,wPath,256);
	//CString curProPath = theApp.GetFilePath(CString(wPath));
	//curProPath += L"\\永凯PMC操作演示.wmv";
	//::ShellExecute(NULL,NULL,curProPath,NULL,NULL,SW_SHOW);

	::ShellExecute(NULL, L"Open", L"http://v.youku.com/v_show/id_XMzc0MzMxMTY4.html", NULL, NULL, SW_SHOWNORMAL);
}

void CMainFrame::OnUpdateVcrpdf(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateShouce(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

//Add 2012-03-19 重现设置表格类型
YK_LONG CMainFrame::ResetDataGridTypTble( UINT orderFindType )
{
	UINT ttblType;
	if ( orderFindType == CSpaceTreeCTrl::Sell1_Order )
	{
		ttblType = TblSellOrder;
	}else if ( orderFindType == CSpaceTreeCTrl::Manufacture_Order )
	{
		ttblType = TblPudOrder;
		
	}else if ( orderFindType == CSpaceTreeCTrl::Buy_Order )
	{
		ttblType = TblBuyOrder;
	}else if ( orderFindType== CSpaceTreeCTrl::Storage_Order )
	{
		ttblType = TblStockOrder;
	}else if ( orderFindType== CSpaceTreeCTrl::Mend_Order )
	{
		ttblType = TblWillOrder;
	}

	return ttblType;
}