// Scheduler.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Scheduler.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "SchedulerDoc.h"
#include "SchedulerView.h"
#include "ComFunc.h"

#include "FileFilter.h"

#include "YukonBase.h"
#include "VCProtectionCenter.h"
#include "Exceptions.h"

using namespace Yukon;

#include "CCreatGridMenu.h"

#include "LanguagePack.h"
#include "CSelectTableDlg.h"
#include "Biz_API.h"
#include "NewRule.h"
#include "GanttApi.h"
#include "BCGPRegistry.h"
#include "HelpIdDef.h"
#include "DataTabl_API.h"
#include "YKBOM.h"
#include "OutputSetDlg.h"
#include "PISettingDlg.h"
#include "GlassStatic.h"
#include "FxButton.h"
#include "YKLicenseObserver.h"
#include "LimitOutOfTimeDlg.h"
#include "NewModelDlg.h"
#include "ApsLoggin.h"
#include "DlgFind.h"
#include "YKGridFindAndReplace.h"
#include "LoadModeDlg.h"
#include "DlgRegister.h"

#include "DialogExpoMessageBox.h"
#include "SplashScreenEx.h"
#include "ProcessBarThread.h"

#ifdef PMCMODE
#include "YKUserInfor.h"
#endif

#ifdef _OCXPACK
#include "GTRulePool.h"
#include "GTResumetReCall.h"
#include "XMLTool.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define  MSG_BLANK   L"    "

//#ifndef _APPLYPROTECTION
#include <sys/stat.h>
//#endif

#include "clock.h"
//加密狗所使用的宏
//_APPLYPROTECTION
 
#include "afxwin.h"
HHOOK	hHOOK;
HHOOK	hHOOKKEYBOARD;
CCriticalSection g_criKeyDown;
int     g_shiftStata = 0; 

BOOL  g_bloadFileEndFlg = FALSE;
BOOL  g_bLoadWorkFlg = FALSE;

#ifdef _APPLYPROTECTION
Yukon::YKLicenseObserver* g_pYkLicenseObserver;
#endif

//#include "ThreadMsgDef.h"
//
//typedef struct tagThreadParamer
//{
//	wstring m_str;
//	int m_n;
//	bool m_flg;
//} ThreadParamer;

LRESULT CALLBACK MouseProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	if (theApp.GetSchudlingFlg())
		return TRUE;
	//if(wParam == WM_MOUSEWHEEL/*WM_MOUSEHWHEEL*/)
	//{
	//	PMOUSEHOOKSTRUCT p = PMOUSEHOOKSTRUCT(lParam);
	//	if(theApp.IsOutlineGrid())
	//	{
	//		return TRUE;
	//	}
	//}
	return CallNextHookEx(hHOOK,nCode,wParam,lParam);
}

LRESULT CALLBACK KeyboardProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	if (wParam == VK_SHIFT)
	{
		g_criKeyDown.Lock();
		if (!(0x80000000 & lParam))
			g_shiftStata = 1;
		else
			g_shiftStata = 0;
		g_criKeyDown.Unlock();
	}
	if (theApp.GetSchudlingFlg() && theApp.GetCanSchedStep())
	{
		g_criKeyDown.Lock();
		switch (wParam)
		{
		case VK_SPACE:
			{
				if ( !(0x80000000 & lParam))
					BIZAPI::SetSBHkeyState(g_shiftStata,VK_SPACE);
			}
			break;
		case VK_CONTROL:
			{
				if (!(0x80000000 & lParam))
					BIZAPI::SetSBHkeyState(g_shiftStata,VK_CONTROL);
				else
					BIZAPI::SetSBHkeyState(g_shiftStata,0);
			}
			break;
		default:
			BIZAPI::SetSBHkeyState(g_shiftStata);
			break;
		}
		g_criKeyDown.Unlock();
		return TRUE;
// 		if (wParam == VK_SPACE)
// 		{
// 			if ( !(0x80000000 & lParam))
// 				BIZAPI::SetSBHkeyState(g_shiftStata,VK_SPACE);
// 			return TRUE;
// 		}
// 		if (wParam == VK_CONTROL)
// 		{
// 			if (!(0x80000000 & lParam))
// 				BIZAPI::SetSBHkeyState(g_shiftStata,VK_CONTROL);
// 			else
// 				BIZAPI::SetSBHkeyState(g_shiftStata,0);	
// 			return TRUE;
// 		}
// 		BIZAPI::SetSBHkeyState(g_shiftStata);
// 		return TRUE;
	} 
	else
	{
		if(wParam == VK_F1 && (0x80000000 & lParam)
			&& ((0x7FFF & lParam) == 1))
		{
			theApp.OnYkhelp();
			return TRUE;
		}
		else if (wParam >= VK_F2 && wParam <= VK_F12
			&&(0x80000000 & lParam)
			&& ((0x7FFF & lParam) == 1)
			&& theApp.m_bOpen)
		{
			//TimeLog::WriteLog(wParam);
			//TimeLog::WriteLog(lParam);
			::SendMessage(theApp.g_hMainFrm,WM_KEYDOWN,wParam,0);

			return TRUE;
		}
	}
	
	return CallNextHookEx(hHOOKKEYBOARD,nCode,wParam,lParam);
}

#ifdef PMCMODE
unsigned int WINAPI RunSendCInfo(void * pPara)
{
	//Add 2012-02
	//YKUserInfor info;
	//YK_WSTRING url = L"http://222.66.0.14:83/Register.aspx?";	//内外URL
	////YK_WSTRING url = L"http://192.168.0.78/PMCServer/Register.aspx?";	//内外URL

	//YK_BOOL bIsConnectNet  = info.IsConnectionNet( url );
	//if ( bIsConnectNet )
	//{
	//	info.OpenURL( url );
	//}
//////////////////////////////////////////////////////////////////////////
	//方法2，开进程
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	si.cb = sizeof(STARTUPINFO);
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.cbReserved2 = NULL;
	si.lpReserved2 = NULL;

	WCHAR wPath[256];
	GetModuleFileName(NULL,wPath,256);
	CString curProPath = theApp.GetFilePath(CString(wPath));
	curProPath += L"\\SData.dll";

	CreateProcess(curProPath.GetString(),
		NULL,
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi );           // Pointer to PROCESS_INFORMATION structure

	return 0;
}
#endif
// CSchedulerApp

BEGIN_MESSAGE_MAP(CSchedulerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_FILE_OPEN, &CSchedulerApp::OnFileOpen)
	//ON_COMMAND(ID_FILE_OPEN, COutputBar::ShowImportInfo) // add
	ON_UPDATE_COMMAND_UI(ID_YK_CLOSE_FILE, &CSchedulerApp::OnUpdateYkCloseFile)
	ON_COMMAND(ID_YK_CLOSE_FILE, &CSchedulerApp::OnYkCloseFile)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CSchedulerApp::OnUpdateFileSave)
	ON_COMMAND(ID_FILE_SAVE, &CSchedulerApp::OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_YK_LOAD_SCH, &CSchedulerApp::OnUpdateYkLoadSch)
	ON_COMMAND(ID_YK_LOAD_SCH, &CSchedulerApp::OnYkLoadSch)
	ON_UPDATE_COMMAND_UI(ID_YK_SAVE_AS, &CSchedulerApp::OnUpdateYkSaveAs)
	ON_COMMAND(ID_YK_SAVE_AS, &CSchedulerApp::OnYkSaveAs)
	ON_COMMAND(ID_ExportCsv, &CSchedulerApp::OnExportcsv)
	ON_UPDATE_COMMAND_UI(ID_YK_SCHEDULER, &CSchedulerApp::OnUpdateYkScheduler)
	
	ON_UPDATE_COMMAND_UI(ID_YK_RES_GANTT, &CSchedulerApp::OnUpdateYkResGantt)
	ON_UPDATE_COMMAND_UI(ID_YK_ORDER_GANTT, &CSchedulerApp::OnUpdateYkOrderGantt)
	ON_UPDATE_COMMAND_UI(ID_YK_RES_RATE, &CSchedulerApp::OnUpdateYkResRate)
	ON_UPDATE_COMMAND_UI(ID_YK_STAG_RATE, &CSchedulerApp::OnUpdateYkStagRate)
	ON_UPDATE_COMMAND_UI(ID_QUERYS, &CSchedulerApp::OnUpdateQuerys)
	ON_UPDATE_COMMAND_UI(IDM_ITEM_GATHER_GL, &CSchedulerApp::OnUpdateYkItemGather_GL)
	ON_UPDATE_COMMAND_UI(IDM_ITEM_GATHER_GRID, &CSchedulerApp::OnUpdateYkItemGather_Grid)
	ON_UPDATE_COMMAND_UI(IDM_DAYSHIFT_GD, &CSchedulerApp::OnUpdateYkDayShift_Grid)

	ON_COMMAND(ID_ALL_FIX, &CSchedulerApp::OnAllFix)
	ON_COMMAND(ID_TIMEFIX, &CSchedulerApp::OnTimeFix)
	ON_COMMAND(ID_ALONE_SURE, &CSchedulerApp::OnAloneSure)
	ON_COMMAND(ID_COMPLETE, &CSchedulerApp::OnCompleteOder)
	ON_COMMAND(ID_PLANOVER, &CSchedulerApp::OnPlanOver)
	ON_COMMAND(ID_ALL_WORK, &CSchedulerApp::OnAllWork)
	ON_COMMAND(ID_RES_FIX, &CSchedulerApp::OnResFix)
	ON_COMMAND(ID_REFIX, &CSchedulerApp::OnReFix)
	ON_UPDATE_COMMAND_UI(ID_ALL_FIX, &CSchedulerApp::OnAllFix)
	ON_UPDATE_COMMAND_UI(ID_TIMEFIX, &CSchedulerApp::OnTimeFix)
	ON_UPDATE_COMMAND_UI(ID_ALONE_SURE, &CSchedulerApp::OnAloneSure)
	ON_UPDATE_COMMAND_UI(ID_COMPLETE, &CSchedulerApp::OnCompleteOder)
	ON_UPDATE_COMMAND_UI(ID_PLANOVER, &CSchedulerApp::OnPlanOver)
	ON_UPDATE_COMMAND_UI(ID_ALL_WORK, &CSchedulerApp::OnAllWork)
	ON_UPDATE_COMMAND_UI(ID_RES_FIX, &CSchedulerApp::OnResFix)
	ON_UPDATE_COMMAND_UI(ID_REFIX, &CSchedulerApp::OnReFix)
	ON_UPDATE_COMMAND_UI(ID_TOOL_FILTER, &CSchedulerApp::OnUpdateYKFilter)
	ON_UPDATE_COMMAND_UI(ID_TOOL_UNFILTER, &CSchedulerApp::OnUpdateUnYKFilter)

	ON_UPDATE_COMMAND_UI(ID_STOP_MULTISCHE, &CSchedulerApp::OnUpdateStopMultiSch)
	ON_UPDATE_COMMAND_UI(ID_SCHE_MULTI, &CSchedulerApp::OnUpdateMultiSch)

	ON_UPDATE_COMMAND_UI(ID_UNDO, &CSchedulerApp::OnUpdateUndo)
	ON_UPDATE_COMMAND_UI(ID_REDO, &CSchedulerApp::OnUpdateRedo)
	ON_COMMAND(ID_UNDO, &CSchedulerApp::OnUndo)
	ON_COMMAND(ID_REDO, &CSchedulerApp::OnRedo)
	ON_COMMAND(ID_MY_SAVE, &CSchedulerApp::OnMySave)

	ON_COMMAND(ID_YKHELP, &CSchedulerApp::OnYkhelp)
	ON_COMMAND(ID_DELETF12,&CSchedulerApp::OnDeleteF12)
	ON_UPDATE_COMMAND_UI(ID_ExportCsv, &CSchedulerApp::OnUpdateExportcsv)
	ON_UPDATE_COMMAND_UI(ID_MY_SAVE, &CSchedulerApp::OnUpdateMySave)
	ON_COMMAND(ID_YK_PRINT, &CSchedulerApp::OnYkPrint)
	ON_UPDATE_COMMAND_UI(ID_YK_PRINT, &CSchedulerApp::OnUpdateYkPrint)
	ON_COMMAND(ID_Menu_NEW, &CSchedulerApp::OnMenuNew)
	ON_UPDATE_COMMAND_UI(ID_Menu_NEW, &CSchedulerApp::OnUpdateMenuNew)

	ON_COMMAND(ID_LOAD_FILE_SETTING,&CSchedulerApp::OnLoadFileSetting)
	ON_UPDATE_COMMAND_UI(ID_LOAD_FILE_SETTING,&CSchedulerApp::OnUpdateLoadFileSetting)
	ON_UPDATE_COMMAND_UI(ID_AUTO_ENSURE_WORK, &CSchedulerApp::OnUpdateAutoEnsureWork)
	ON_COMMAND(ID_CREATRESSEQUENCE, &CSchedulerApp::OnCreatressequence)
	ON_COMMAND(ID_CREATITEMSEQUENCE, &CSchedulerApp::OnCreatitemsequence)
	ON_COMMAND(ID_RESINFO, &CSchedulerApp::OnResinfo)
	ON_UPDATE_COMMAND_UI(ID_CREATRESSEQUENCE, &CSchedulerApp::OnUpdateCreatressequence)
	ON_UPDATE_COMMAND_UI(ID_CREATITEMSEQUENCE, &CSchedulerApp::OnUpdateCreatitemsequence)
	ON_UPDATE_COMMAND_UI(ID_RESINFO, &CSchedulerApp::OnUpdateResinfo)
	ON_COMMAND(ID_CREATEUSERES, &CSchedulerApp::OnCreatuseres)
	ON_COMMAND(ID_CREATEPROINDIA, &CSchedulerApp::OnCreatepro)
	ON_UPDATE_COMMAND_UI(ID_CREATEUSERES, &CSchedulerApp::OnUpdateCreatuseres)
	ON_UPDATE_COMMAND_UI(ID_CREATEPROINDIA, &CSchedulerApp::OnUpdateCreatepro)
	ON_COMMAND(ID_CREATWORKCHANGE, &CSchedulerApp::OnCreatworkchange)
	ON_UPDATE_COMMAND_UI(ID_CREATWORKCHANGE, &CSchedulerApp::OnUpdateCreatworkchange)

	ON_UPDATE_COMMAND_UI(ID_BUTTON_TOGETHOR_RUN, &CSchedulerApp::OnUpdateTogethorRun)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TOGETHOR_SELECT, &CSchedulerApp::OnUpdateTogethorSelect)
	
	ON_COMMAND(ID_BUTTON_TOGETHOR_SELECT, &CSchedulerApp::OnTogethorSelect)
	ON_COMMAND(ID_WORKSEQUENCE, &CSchedulerApp::OnWorksequence)
	ON_COMMAND(ID_OUTPUT_RES_RATE, &CSchedulerApp::OnOutputResRate)
	ON_UPDATE_COMMAND_UI(ID_WORKSEQUENCE, &CSchedulerApp::OnUpdateWorksequence)
	ON_UPDATE_COMMAND_UI(ID_OUTPUT_RES_RATE,&CSchedulerApp::OnUpdateOutputResRate)

	ON_UPDATE_COMMAND_UI(ID_QUICK_BUILD_MODE,&CSchedulerApp::OnUpdateQuickBuildMode)
	ON_COMMAND(ID_QUICK_BUILD_MODE, &CSchedulerApp::OnQuickBuildMode)
	
	ON_COMMAND(ID_MBUTTON, &CSchedulerApp::OnYkhelp)
	ON_COMMAND(ID_FRAME_MENU_GRIDSHOWTIP, &CSchedulerApp::OnFrameMenuGridshowtip)
	ON_UPDATE_COMMAND_UI(ID_FRAME_MENU_GRIDSHOWTIP, &CSchedulerApp::OnUpdateFrameMenuGridshowtip)
	ON_COMMAND(ID_INPUTSYS, &CSchedulerApp::OnInputsys)
	ON_UPDATE_COMMAND_UI(ID_INPUTSYS, &CSchedulerApp::OnUpdateInputsys)
	ON_COMMAND(ID_OUTPUTSYS, &CSchedulerApp::OnOutputsys)
	ON_UPDATE_COMMAND_UI(ID_OUTPUTSYS, &CSchedulerApp::OnUpdateOutputsys)
	ON_COMMAND(ID_DOWNLOAD, &CSchedulerApp::OnDownload)
	ON_UPDATE_COMMAND_UI(ID_DOWNLOAD, &CSchedulerApp::OnUpdateDownload)
	ON_COMMAND(ID_SUBMIT, &CSchedulerApp::OnSubmit)
	ON_UPDATE_COMMAND_UI(ID_SUBMIT, &CSchedulerApp::OnUpdateSubmit)
	ON_COMMAND(ID_LOADING, &CSchedulerApp::OnLoading)
	ON_UPDATE_COMMAND_UI(ID_ConflictSolve, &CSchedulerApp::OnUpdateConflictSolve)
	ON_UPDATE_COMMAND_UI(ID_LOADING, &CSchedulerApp::OnUpdateLOADING)
	ON_COMMAND(ID_SendDemand, &CSchedulerApp::OnSenddemand)
	ON_UPDATE_COMMAND_UI(ID_SendDemand, &CSchedulerApp::OnUpdateSenddemand)
	ON_COMMAND(ID_GetDemand, &CSchedulerApp::OnGetdemand)
	ON_UPDATE_COMMAND_UI(ID_GetDemand, &CSchedulerApp::OnUpdateGetdemand)
	ON_COMMAND(ID_FEEDBACK, &CSchedulerApp::OnFeedback)
	ON_UPDATE_COMMAND_UI(ID_FEEDBACK, &CSchedulerApp::OnUpdateFeedback)
	ON_COMMAND(ID_ACCETPFEEDBACK, &CSchedulerApp::OnAccetpFeedback)
	ON_UPDATE_COMMAND_UI(ID_ACCETPFEEDBACK, &CSchedulerApp::OnUpdateAccetpFeedback)
	ON_COMMAND(IDM_HELP_REGISTER, &CSchedulerApp::OnHelpRegister)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_SETUP, &CSchedulerApp::OnUpdateFilePrintSetup)
	END_MESSAGE_MAP()


// CSchedulerApp construction

CSchedulerApp::CSchedulerApp() :
	CBCGPWorkspace (TRUE /* m_bResourceSmartUpdate */)
		,m_bOpen(FALSE)
		,m_bIsLoad(FALSE)
		,m_couldCreatWorkChange(FALSE)
#ifdef PMCMODE
		,m_strVersion(_T("V5.0.2"))
#else
		,m_strVersion(_T("V6.0.6"))
#endif

		,m_BOMViewItemId(0)
		,m_isNeedInitData(TRUE)
		,m_bSchduling(false)
		,m_bCanSchedStep(false)
//		,m_needMouseDown(FALSE)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_strConfigur = L"";
	m_langueType = LANGUAGE_CHS;
	m_hinstBCGCBRes = NULL;
	m_gridShowLabel = false;
	m_bNetLoaded = false;
	m_bAutoOpen = FALSE;

}


// The one and only CSchedulerApp object

CSchedulerApp theApp;


BOOL CSchedulerApp::ProctDog()
{
	try
	{
		// Ensure the software active and not expired
#ifdef _USEMACRO
		//ENSURE_CURRENT_SOFTWARE_ACTIVE
#else
		g_protector.EnsureSoftwareActive();
#endif

		//验证客户名
		//CString registeredName;
		//if (!ReadRegisteredCustomerName(registeredName))
		//{
		//	CCustomerRegister ccr;

		//	if (IDOK == ccr.DoModal())
		//	{
		//		registeredName = ccr.GetCustomerName();
		//		if (!SaveRegisteredCustomerName(registeredName))
		//		{
		//			return FALSE;	
		//		}
		//	}
		//	else
		//	{
		//		return FALSE;
		//	}
		//}

		// Ensure the customer name valid
//#ifdef _USEMACRO
//		ENSURE_LICENSE_VALID_4_CUSTOMER(registeredName.GetBuffer())
//#else
//		g_protector.EnsureLicenseValid4Customer(registeredName.GetBuffer());
//#endif

		// Ensure current key is just for current machine, must after customer validation,
		// otherwise this key can not use any more
#ifdef _USEMACRO
		//ENSURE_LICENSE_KEY_NOT_EMBEZZELED
#else
		g_protector.EnsureLicenseKeyNotEmbezzeled();


		//g_protector.GetLeaseInfo().IsLease;
		// If current license is lease or demo, show dialog
		LEASEINFO& lease = g_protector.GetLeaseInfo();
		if (lease.IsLease)
		{
			//if (lease.Feature.bEnableDurationTime || lease.Feature.bEnableStopTime)
			{
				CString msg = _T("This license is only for demo.");//, will expired at UTC
				//msg.AppendFormat(_T("%d-%02d-%02d %02d-%02d-%02d")
				//	, lease.Feature.timeControl.stopTime.year
				//	, lease.Feature.timeControl.stopTime.month
				//	, lease.Feature.timeControl.stopTime.dayOfMonth
				//	, lease.Feature.timeControl.stopTime.hour
				//	, lease.Feature.timeControl.stopTime.minute
				//	, lease.Feature.timeControl.stopTime.second);
				//MessageBox(NULL, msg, _T("Demo License"), MB_OK);
			}
		}

#endif

#ifdef _USEMACRO
		//VALIDATE_LICENSE()
#else
		g_protector.ValidateLicense();
#endif

	}

	catch (CLicenseKeyNotExistException lie)
	{
		return FALSE;
	}
	//catch (CSoftwareInactiveException sie)
	//{
	//	MessageBox(NULL, L"Software inactive", L"Error", MB_OK);
	//	return FALSE;
	//}
	catch (CLicenseExpiredException lee)
	{
//		MessageBox(NULL, L"License expired", L"Error", MB_OK);
		return FALSE;
	}
	catch (CLicenseInvalidException lie1)
	{
//		MessageBox(NULL, L"License invalid", L"Error", MB_OK);
		return FALSE;
	}
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return TRUE;
}

BOOL CSchedulerApp::IsProctDogOverdue()
{
#ifndef _USEMACRO
	Yukon::LEASEINFO& lf = g_protector.GetLeaseInfo();
	if (lf.IsExpired)
		return TRUE;
	else
#endif
		return FALSE;
}

// CSchedulerApp initialization

BOOL CSchedulerApp::InitInstance()
{

#ifdef _DEBUG
//	_CrtSetBreakAlloc(85186);
#endif

#ifdef _OCXPACK
	RegVsFlexGrid();
	AfxEnableControlContainer();
	GTRulePool::RulePoolInitializtion();
	GTResumetReCall::Initialize();
	return COleControlModule::InitInstance();
#else

#ifdef DEF_LANGUAGE_SWITCH
	int nAuto(0);
	CString strPath;
	OpenSystemSet(nAuto,strPath);
	{
		m_hinstResChs = AfxGetResourceHandle();
		if (m_langueType == LANGUAGE_CHS)
		{
			m_hinstBCGCBRes = LoadLibrary (_T("YKResCHS.dll"));    // *** - language
			BCGCBProSetResourceHandle (m_hinstBCGCBRes);
			const YK_ULONG lt = LANGUAGE_CHS;
			SetLanguageType(lt);
			//DATATABLAPI::Clear();
			//DATATABLAPI::InitTableInfo();
			//g_tblManager.Init();
			//g_filedParaMap.Init();
		}
		else
		{
			m_hinstRes = NULL;
			m_hinstRes = LoadLibrary (_T("YKResENG.dll"));    // *** - language
			if (m_hinstRes)
				AfxSetResourceHandle(m_hinstRes);
			const YK_ULONG lt = LANGUAGE_US;
			SetLanguageType(lt);
			//DATATABLAPI::Clear();
			//DATATABLAPI::InitTableInfo();
			//g_tblManager.Init();
			//g_filedParaMap.Init();
		}
	}


#else 
#ifdef DEF_LANGUAGE_ENG
	m_hinstRes = LoadLibrary (_T("YKResENG.dll"));    // *** - language
	if (m_hinstRes)
		AfxSetResourceHandle(m_hinstRes);
		//BCGCBProSetResourceHandle(m_hinstRes);
	m_langueType = LANGUAGE_US;
	const YK_ULONG lt = LANGUAGE_US;
	SetLanguageType(lt);
	//DATATABLAPI::Clear();
	//DATATABLAPI::InitTableInfo();
	//DATATABLAPI::InitTableModeManager(theApp.m_curProPath.GetString());
	//DATATABLAPI::InitFiledMap();
#else //ifdef DEF_LANGUAGE_CHS

	m_hinstBCGCBRes = LoadLibrary (_T("YKResCHS.dll"));    // *** - language
	BCGCBProSetResourceHandle (m_hinstBCGCBRes);
	m_langueType = LANGUAGE_CHS;
	const YK_ULONG lt = LANGUAGE_CHS;
	SetLanguageType(lt);
	//DATATABLAPI::Clear();
	//DATATABLAPI::InitTableInfo();
	//DATATABLAPI::InitTableModeManager(theApp.m_curProPath.GetString());
	//BIZAPI::InitFiledMap();
#endif

#endif

	InitRuleMap();
#ifdef LIMITFUNCTION
	BIZAPI::SetLimitEdition(false);
#endif
//加密版
#ifdef _APPLYPROTECTION
	if(!ProctDog())
	{
#ifndef OLDNETCONNET
		BIZAPI::SetLimitEdition(true);
		BIZAPI::SetIsLimited(true);
#else
		if (theApp.IsProctDogOverdue())
		{
			if (IDYES == ProctDogMsg(1))
			{
				BIZAPI::SetLimitEdition(true);
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			if (IDYES == ProctDogMsg(0))
			{
				BIZAPI::SetLimitEdition(true);
			}
			else
			{
				return FALSE;
			}
		}
#endif



		//return FALSE;

		//time_t	installTime =	GetInstalTime();

		//CDealTime curTime = CDealTime::GetLocalTime();

		//if(installTime <= 0 || curTime.GetTime() < installTime)
		//{		
		//	int iresult; 
		//	struct _stat buf;
		//	WCHAR wPath[256];
		//	GetCurrentDirectory(256,wPath);
		//	wcscat_s(wPath , L"\\YKBizModel.dll");

		//	iresult = _wstat(wPath,&buf); 
		//	if(iresult != -1)
		//	{
		//		CDealTime DeadTime(buf.st_mtime + 30 * 86400);

		//		if(curTime > DeadTime)
		//		{
		//			if (IDYES == ProctDogMsg(2))
		//			{
		//				BIZAPI::SetLimitEdition(true);
		//			}
		//		}
		//	}
		//}
		//else
		//{
		//	CDealTime DeadTime(installTime + 30 * 86400);
		//	if(curTime > DeadTime)
		//	{
		//		if (IDYES == ProctDogMsg(2))
		//		{
		//			BIZAPI::SetLimitEdition(true);
		//		}
		//	}
		//}

	}
#else
	//del by yp 2011.05.18
	//使用加壳软件控制可用时间
	//{
	//	CDealTime curTime = CDealTime::GetLocalTime();

	//	int iresult; 
	//	struct _stat buf;
	//	WCHAR /*wPath[256],*/tempPath[256];
	//	//GetCurrentDirectory(256,wPath);
	//	GetModuleFileName(NULL,tempPath,256);
	//	CString strPath = GetFilePath(CString(tempPath));
	//	//memcpy(wPath,strPath.GetString(),256);
	//	strPath +=  L"\\YKBizModel.dll";
	//	//wcscat_s(wPath , L"\\YKBizModel.dll");
	//	iresult = _wstat(strPath.GetString(),&buf); 
	//	if(iresult != -1)
	//	{
	//		CDealTime DeadTime(buf.st_mtime + 15 * 86400);

	//		if(curTime > DeadTime)
	//		{
	//			if (IDYES == ProctDogMsg(2))
	//			{
	//				BIZAPI::SetLimitEdition(true);
	//			}
	//		}
	//	}
	//}
#endif

#ifdef PMCMODE
	//CDlgRegister dlgReg;
	//if (false == dlgReg.CheckRegisterInfo())	// 校验注册信息失败，要求注册
	//{
	//	if (IDCANCEL == dlgReg.DoModal())
	//		return -1;
	//}
#endif


	hHOOK = SetWindowsHookEx(WH_MOUSE,MouseProc,0,GetCurrentThreadId());
	hHOOKKEYBOARD = SetWindowsHookEx(WH_KEYBOARD,KeyboardProc,0,GetCurrentThreadId());


//	CoInitialize(NULL);
	ChangeTitle();
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	
	//保存程序路径
	WCHAR wPath[256];
	//GetCurrentDirectory(256,wPath);
	GetModuleFileName(NULL,wPath,256);
	m_curProPath = GetFilePath(CString(wPath));



	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	
	//InitTableInfo();  del 2009/11/22
	//m_tblInfoList=DATATABLAPI::GetTblInfoList();
	theApp.g_viewID = 0;
	theApp.g_tblType = TblOrder;
	//DATATABLAPI::InitTableModeManager(theApp.m_curProPath.GetString());
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization

	RegVsFlexGrid();

	SetRegistryKey(_T("YKAPS Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	SetRegistryBase (_T("Settings"));

	// Initialize all Managers for usage. They are automatically constructed
	// if not yet present
	InitContextMenuManager();
	InitKeyboardManager();

	// TODO: Remove this if you don't want extended tooltips:
	InitTooltipManager();

	CBCGPToolTipParams params;
	params.m_bVislManagerTheme = TRUE;

	theApp.GetTooltipManager ()->SetTooltipParams (
		BCGP_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS (CBCGPToolTipCtrl),
		&params);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDI_ICON2,
		RUNTIME_CLASS(CYKSchedulerDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CYKSchedulerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;

	m_pMainWnd = pMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;

#ifdef _APPLYPROTECTION
	// Register active observer and start to observe
	g_pYkLicenseObserver = new Yukon::YKLicenseObserver();
	if (g_pYkLicenseObserver != NULL)
		g_protector.RegisterObserver(g_pYkLicenseObserver);
	g_protector.StartObserveLicense();
	g_protector.StartObserveLicenseEmbezzelment();
#endif
	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	//ProcessShellCommand(cmdInfo);

	if (!pMainFrame->LoadMDIState (GetRegSectionPath ()))
	{
		m_nCmdShow = SW_SHOWMAXIMIZED;
	}

	//TimeLog::WriteLog(TOOLS::W2A_S(cmdInfo.m_strFileName.GetString()).c_str());
	//TimeLog::WriteLog(TOOLS::W2A_S(cmdInfo.m_strDriverName.GetString()).c_str());
	//TimeLog::WriteLog(TOOLS::W2A_S(cmdInfo.m_strPortName.GetString()).c_str());
	//TimeLog::WriteLog(TOOLS::W2A_S(cmdInfo.m_strPrinterName.GetString()).c_str());
	//TimeLog::WriteLog("4");
	// The main window has been initialized, so show and update it
	SetMainFrmText();
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

#ifdef PMCMODE
	//Add 2012-03-14 判断是否显示提示信息	
	YK_SHORT nIsShow = IsShowTopInfor();
	if ( nIsShow == 1 )
	{
		Sleep(5000);
		PMCTopInforDlg pmcDlg;
		pmcDlg.DoModal();
	}
#endif
	

	int nAuto(0);
	CString operFileName = cmdInfo.m_strFileName;
	if(operFileName.IsEmpty())				// 不是命令打开
	{
		OpenSystemSet(nAuto,operFileName);	// 取注册表中打开项
	}
	else nAuto = 1;
	// 打开对应路径文件 nAuto = 1 打开对应 返回是否正确打开(如果正确打开 提示打开信息)
	m_bOpen = Open(operFileName,nAuto);		//统一都调用打开函数  
	if (!m_bOpen)							// 没有打开 提示APS提示
	{
		CString cstrCode=L"";
		SetMsgInfo(ID_UI_BAR_WELUSEYKSCHEDULER,cstrCode,false,MSG_TYPE_SYS);		// 欢迎使用APS
		SetMsgInfo(ID_UI_MSG_TYPE_BLANK,cstrCode,false,MSG_TYPE_SYS);			// 增加1行空白行
	}

	EnableHtmlHelp();

	// 初始化复合编辑框  必须调用(否则不能使用)
	::AfxInitRichEdit();
	
	InitIconForGrid();

#ifdef PMCMODE

	uintptr_t nTread = _beginthreadex(NULL,0,RunSendCInfo,NULL,0,NULL);
#endif

	return TRUE;
#endif
}

// CSchedulerApp message handlers

int CSchedulerApp::ExitInstance() 
{
#ifdef _OCXPACK

	GTRulePool::RulePooRelese();
	GTResumetReCall::Release();
	CXMLColMgr::Release();
	return COleControlModule::ExitInstance();
#else
	SaveSystemSet();
	BCGCBProCleanUp();
	YKGridFindAndReplaceMgr::Realse();
	YKFindAndReplaceDlgMgr::Clear();

//	if(IsOnConnectNet())
	{
		OffConnectedNet();
		DATATABLAPI::Logout();
	}

	if(hHOOK)
		UnhookWindowsHookEx(hHOOK);

	if(hHOOKKEYBOARD)
		UnhookWindowsHookEx(hHOOKKEYBOARD);

	CGridMenu::GridMenuRelese();
	FreeLibrary (m_hinstBCGCBRes);
	FreeLibrary(m_hinstRes);

	CSplashScreenEx::Free();

	UnInitLanguagePack();
#ifdef _APPLYPROTECTION
	if (g_pYkLicenseObserver != NULL)
	{
		g_protector.UnregisterObserver(g_pYkLicenseObserver);
		delete g_pYkLicenseObserver;
	}
#endif
	return CWinApp::ExitInstance();
#endif
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };
	CBCGPURLLinkButton m_btnURL;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
	
protected:
	CString m_strVersion;
	DECLARE_MESSAGE_MAP()
public:
	void SetVerison(CString& str){m_strVersion = str;};
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	GlassStatic m_stcVersion;
	GlassStatic m_stcCopyRight;
	GlassStatic m_stcAllRight;
	GlassStatic m_stcCompeny;
	CFxButton m_btnOk;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMPANY_URL, m_btnURL);
	DDX_Control(pDX, IDC_STC_VERSION, m_stcVersion);
	DDX_Control(pDX, IDC_ABOUT_COPYRIGHT, m_stcCopyRight);
	DDX_Control(pDX, IDC_ABOUT_ALLRIGHT, m_stcAllRight);
	DDX_Control(pDX, IDC_ABOUT_COMPENY, m_stcCompeny);
	DDX_Control(pDX, IDOK, m_btnOk);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// App command to run the dialog
void CSchedulerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.SetVerison(m_strVersion);
	aboutDlg.DoModal();
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	 m_stcVersion.SetGlass(TRUE);
	 m_stcVersion.SetGroub(FALSE);
	 m_stcVersion.SetBottonLine(FALSE);
	 m_stcVersion.SetNoBack(TRUE);

	 m_stcCopyRight.SetGlass(TRUE);
	 m_stcCopyRight.SetBottonLine(FALSE);
	 m_stcCopyRight.SetNoBack(TRUE);

	 m_stcAllRight.SetGlass(TRUE);
	 m_stcAllRight.SetBottonLine(FALSE);
	 m_stcAllRight.SetNoBack(TRUE);

	 m_stcCompeny.SetGlass(TRUE);
	 m_stcCompeny.SetBottonLine(FALSE);
	 m_stcCompeny.SetNoBack(TRUE);
	 m_stcCompeny.SetWindowText(GetRemarkFiled(ID_UI_APP_CODE2).c_str());

	 SetWindowText(GetRemarkFiled(ID_UI_APP_CODE1).c_str());

	 int res = IDB_BTN_NOMAL;
	 if (m_btnOk.GetSafeHwnd())
	 {
		 CString strText = GetRemarkFiled(ID_UI_BOMRESAVBDLG_OK).c_str();
		 m_btnOk.SetFxBitmap(res);
		 m_btnOk.SetFxBitmapRect(m_btnOk.m_Button.rectangle);
		 m_btnOk.SetFxBorderSize(0);
		 m_btnOk.SetFxText(strText,false,strText,strText,strText);
		 m_btnOk.FxCreateButton(NULL,IDOK,DISABLE_FOCUSRECT,m_btnOk.m_Control.isChecked);
	 }
	SetDlgItemText(IDC_STC_VERSION,m_strVersion);

//#ifdef PMCMODE
//	GetDlgItem(IDC_STATIC)->ShowWindow(FALSE);
//#else
//	GetDlgItem(IDC_STATIC7)->ShowWindow(FALSE);
//#endif


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


// CSchedulerApp message handlers


void CSchedulerApp::PreLoadState ()
{

	//GetContextMenuManager()->AddMenu (_T("My menu"), IDR_CONTEXT_MENU);
	//GetContextMenuManager()->AddMenu (_T("SpaceTree menu"), IDR_MENU_SPTREE);
	// TODO: add another context menus here
}
// isNeedInitData 是否需要加载数据  跳转的时候不需要加载数据 
// 数据ID列表放在VIEW中的FindList中  add 2010-5-17 by ll
bool CSchedulerApp::OpenTblPage(UINT tblType,BOOL isNeedInitData)
{
	if (HideTable(tblType))
		return true;
	m_isNeedInitData = isNeedInitData;
	
	g_tblType = tblType;

	ViewTip vt;
	if (g_viewTips.Find(tblType,vt))//视图已经打开
	{
		if (vt.pWnd != NULL)
		{
			if (tblType == ID_BOM_VIEW_DLG)
			{
				// close bom view  and new bom view
				if(theApp.m_BOMViewItemId>0)
					((CYKSchedulerView*)vt.pWnd)->m_bomViewDlg.TranToPage(theApp.m_BOMViewItemId);
			}
			if (vt.pWnd->IsKindOf(RUNTIME_CLASS(CYKSchedulerView)))
			{
				((CYKSchedulerView*)vt.pWnd)->SetFindType(-1);
			}
			
			::SendMessage(g_hMainFrm, IDU_ACTIVIT_VIEW, 0, long(vt.pWnd));
		}	
	}
	else
	{
		if(g_tblType > 0)
		{
			//add by yp 订单
			if ( g_tblType == TblOrder)
			{
				InsertFilter(TblOrder,L"");
			}
			else if (g_tblType == TblResource)
			{
				InsertFilter(TblResource,L"");
			}

			theApp.OnFileNew();
		}

	}

	return true;
}

bool CSchedulerApp::Open(CString strPath, int nAuto)
{
	BIZAPI::EnterDataBaseCriSel();

	YKBizMap* pBizMap = BIZAPI::GetBizMap();
	if(NULL == pBizMap || strPath.IsEmpty() || 0 == nAuto)
	{
		BIZAPI::InitInsidePropertyManager();			// 打开失败 使用内部定义初始化
		DATATABLAPI::InitTableModeManager(L"");			// 打开失败 使用内部定义初始化数据
		BIZAPI::LeaveDataBaseCriSel();
		return false;
	}

	// 关闭数据 包括界面标示  BIZModel数据
   	Close();

	int bAuto(0);
	CString temp;
	OpenSystemSet(bAuto, temp);
	YK_SHORT nAutoApsSelect = pBizMap->GetSchSysParam()->GetAutoApsSelect();

	BeginWaitCursor();
	m_strFile = strPath;
	YK_WSTRING strFile = strPath.GetString();
	m_curPath = GetFilePath(strPath);

	DATATABLAPI::SetTimeUnit(pBizMap->GetSchSysParam()->GetSystemTimeUnit());
	//初始化导入表  add by lq 2009-11-23

	YK_WSTRING strVerison;
	m_bOpen = DATATABLAPI::ReadYks(strFile, strVerison);

	if (pBizMap->GetYKLimitOfAuthorPtrMap()->Empty())
		pBizMap->GetYKLimitOfAuthorPtrMap()->InitDefault();

	if(!m_bOpen)
	{
		Close();
		EndWaitCursor();
		BIZAPI::LeaveDataBaseCriSel();
		return false;
	}

	YKExtraValue::SetTimeUnit(pBizMap->GetSchSysParam()->GetSystemTimeUnit());  //设置界面的时间类型

	pBizMap->GetSchSysParam()->SetAutoApsSelect(nAutoApsSelect);

#ifdef  PMCMODE
	pBizMap->GetSchSysParam()->SetAreaSchPara(1);
#else
#ifndef _OCXPACK
	//模型选择 
	if (nAutoApsSelect <= 1)
	{
		CLoadModeDlg dlg;
		if (dlg.DoModal() != IDOK)
		{
			m_bOpen = FALSE;
			BIZAPI::LeaveDataBaseCriSel();
			return false;
		}
		pBizMap->GetSchSysParam()->SetAreaSchPara(dlg.GetSelectMode());
	}
	else
	{
		pBizMap->GetSchSysParam()->SetAreaSchPara(nAutoApsSelect - 1);
	}
#else
	pBizMap->GetSchSysParam()->SetAreaSchPara(1);
#endif

#endif

	SetSchudlingFlg(true);

	//重置界面
#ifndef _OCXPACK
	((CMainFrame*)m_pMainWnd)->LoadWindowsState();
#endif

	DATATABLAPI::InitLoadFileInfo();	// 导入导出数据信息
	ChangeShowPrecision();				// 界面显示数据

	//Add 2012-01-06	//导入时检验日历和班次是否为空，若为空设置指定的值
#ifdef PMCMODE
	BIZAPI::GetBizMap()->CheckCalendarAndDayOfShift();
#endif

	BIZAPI::IniProcess();

	DATATABLAPI::ReadSys(strFile, strVerison);

	BIZAPI::TransFormAlg(false);

	// 初始化BOM	add 2010-3-10 by ll
	BIZAPI::GetBizMap()->InitForBOM();


	// 初始化BOM对应的输入输出类型
	BIZAPI::GetBizMap()->InitInCmdCodeMap();
	BIZAPI::GetBizMap()->InitOutCmdCodeMap();

	BIZAPI::GetBizMap()->SetStrFilePath(strPath.GetString());
	m_bAutoOpen = FALSE;
	// 打开YKS消息显示
#ifndef _OCXPACK
	CString cstrCode=L"";
	SetMsgInfo(ID_UI_BAR_WELUSEYKSCHEDULER,cstrCode,false,MSG_TYPE_SYS);		// 欢迎使用APS
	SetMsgInfo(ID_UI_MSG_TYPE_BLANK,cstrCode,false,MSG_TYPE_SYS);		// 增加1行空白行
	unsigned long mesId = SetMsgInfo(ID_UI_BAR_FILEOPEN,strPath,true,MSG_TYPE_SYS);
	cstrCode.Format(L"%d",pBizMap->GetObjCount());
//	unsigned long mesId = SetMsgInfo(ID_UI_BAR_OBJCOUNT,cstrCode,TRUE,MSG_TYPE_SYS);			// 总对象数	
#endif

#ifdef LIMITFUNCTION
	BIZAPI::LimitCheck();
#endif
	theApp.m_couldCreatWorkChange=FALSE;   // 工作变更不能生成


#ifndef _OCXPACK
	::SendMessage(theApp.g_hMainFrm, ID_MESSAGE_INITTREE,0,0);
	::SendMessage(theApp.g_hMainFrm, IDU_INIT_APS_COMBOBOX,0,0);
	::SendMessage(theApp.g_hMainFrm, ID_MESSAGE_FLASH_OUTPUTWND,0,7);
	::SendMessage(theApp.g_hMainFrm,IDU_UPDATE_MODIFY_VIEW,53,mesId);	
	//::SendMessage(theApp.g_hMainFrm,ID_PMCTOOLBAR_CHANGE,0,0);	

	OpenLastTable();
#endif
	m_bOpen = TRUE;	

	SetSchudlingFlg(false);
	EndWaitCursor();
	BIZAPI::LeaveDataBaseCriSel();
	return true;
}

bool CSchedulerApp::OpenCaseDialog()
{
	//Add 2012-01-13 添加打开YKS 系统提示是否要保存信息
	YK_UINT nResult = FinalClose();

	if ( IDCANCEL == nResult )
	{
		return false;
	}
	else
	{		
		if ( IDYES == nResult )		//执行保存操作
		{
			OnMySave();

			TCHAR szFilters[]= _T("YukonSys data (*.yks)|*.yks|All Files (*.*)|*.*||");

			CFileDialog  openFile(TRUE,_T("yks"),_T("Data.yks"),OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters);
			openFile.GetOFN().lpstrInitialDir = m_curPath;

			if ( IDOK == openFile.DoModal() )
			{
				return Open(openFile.GetPathName());
			}
		}
		else
		{
			TCHAR szFilters[]= _T("YukonSys data (*.yks)|*.yks|All Files (*.*)|*.*||");

			CFileDialog  openFile(TRUE,_T("yks"),_T("Data.yks"),OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters);
			openFile.GetOFN().lpstrInitialDir = m_curPath;
		
			if ( IDOK == openFile.DoModal() )
			{
				return Open(openFile.GetPathName());
			}
		}
	}


	return true;
}

bool CSchedulerApp::SaveAs()
{
	TCHAR szFilters[]= _T("YukonSys data (*.yks)|*.yks|All Files (*.*)|*.*||");

	CFileDialog  openFile(FALSE,_T("yks"),_T("Data.yks"),OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters);

	if ( IDOK == openFile.DoModal() )
	{
		((CMainFrame*)m_pMainWnd)->SaveWindowsState();

		CString strFile = openFile.GetPathName();
		
		m_strFile = strFile;

		BeginWaitCursor();
		YK_WSTRING strVerison = m_strVersion.GetString();
		YK_WSTRING strPath = strFile.GetString();
		time_t  startTm=clock();
		BIZAPI::GetBizMap()->SetCurrentUseType(2);
		DATATABLAPI::SaveYks(strPath,strVerison);
		BIZAPI::GetBizMap()->SetCurrentUseType(3);
		double  endTm=(clock()-startTm)/1000.0;
		//WCHAR wPath[256];
		//GetCurrentDirectory(256,wPath);
		m_curPath = GetFilePath(strFile);
		//TimeLog::WriteLog("SAVEAS");
		//TimeLog::WriteLog(TOOLS::W2A_S(m_curPath.GetString()).c_str());

		CString strCode;
		strCode.Format(L"( %f ",endTm);
		strCode+=GetRemarkFiled(ID_UI_BAR_SECOND).c_str();
		strCode+=L")";
		strCode+=strFile;
		SetMsgInfo(ID_UI_BAR_FILEOSAVE,strCode,TRUE,MSG_TYPE_SYS);		// 保存文件

		// 更新消息窗口
		::SendMessage(m_pMainWnd->GetSafeHwnd(),ID_MESSAGE_FLASH_OUTPUTWND,0,0);
		return true;
	}

	return false;
}



bool CSchedulerApp::Close()
{
	//if(IsOnConnectNet())
	//{
	//	OffConnectedNet();
	//	DATATABLAPI::Logout();
	//}

	m_strFile.Empty();
	m_bOpen = FALSE;
	m_bIsLoad = FALSE;

	if (g_hMainFrm != NULL)
		::SendMessage(g_hMainFrm,ID_MESSAGE_INITTREE,0,0);

	return ClearAllData();

}
//  关闭撤销
bool CSchedulerApp::CloseForLoadSch()
{
	m_strFile.Empty();
	m_bOpen = FALSE;
	m_bIsLoad = FALSE;

	BIZAPI::ClearReback();

	if (g_hMainFrm != NULL)
		::SendMessage(g_hMainFrm,_CLOSE_ALL_VIEW,10,11);
	if (g_hMainFrm != NULL)
		::SendMessage(g_hMainFrm,ID_MESSAGE_INITTREE,0,0);
	m_outputResRatePath.Empty();
	m_outputResRateFileFormat = OutputFileFormat_Coma;
	return true;
}


void CSchedulerApp::OnFileOpen()
{	
	OpenCaseDialog();
}

void CSchedulerApp::OnUpdateYkCloseFile(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnYkCloseFile()
{
	// TODO: Add your command handler code here
	//static_cast<CMainFrame*>(m_pMainWnd)->sShowCloseFile(m_strFile);
	SetMsgInfo(ID_UI_BAR_CLOSESCHEDULER,m_strFile,TRUE,MSG_TYPE_SYS);
	Close();
	m_bOpen = FALSE;
}

void CSchedulerApp::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnFileSave()
{
	// TODO: Add your command handler code here
	//BeginWaitCursor();
	//g_dtUtil->Clear();
	//g_dtUtil->SaveMapsToDt();	
	//SUCCEEDED(g_dtUtil->CommitTrans());
	//EndWaitCursor();

	if ( m_bOpen )
	{	
		BeginWaitCursor();
		YK_WSTRING strVerison = m_strVersion.GetString();
		YK_WSTRING strPath = m_strFile.GetString();
		BIZAPI::GetBizMap()->SetCurrentUseType(2);
		DATATABLAPI::SaveYks(strPath,strVerison);
		BIZAPI::GetBizMap()->SetCurrentUseType(3);
		EndWaitCursor();
	}
	else
	{
		SaveAs();
	}
}

void CSchedulerApp::OnUpdateYkLoadSch(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	//pCmdUI->Enable(TRUE);
	_LIMIT_CHECK_
	pCmdUI->Enable(m_bOpen);
}


//unsigned int WINAPI Run_LoadFile(void* pParam)
//{
//	ThreadParamer* tp = (ThreadParamer*)pParam;
//	if (tp != NULL)
//	{
//		if (tp->m_flg)
//		{
//			DATATABLAPI::ReadAll(tp->m_str);
//			//g_bLoadWorkFlg = rf.GetWorkInputFlg();
//			
//		}
//		else
//		{
//			DATATABLAPI::WriteFile(tp->m_str);
//		}
//	}
//	g_bloadFileEndFlg = TRUE;
//	return 0;
//}

void CSchedulerApp::OnYkLoadSch()
{
	// TODO: Add your command handler code here
	YK_SHORT nOpType = AfxMessageBox( GetRemarkFiled(ID_PROMT_IMPORT).c_str() ,MB_YESNO|MB_ICONQUESTION);
	if ( nOpType == IDNO )
		return;

	CString strPath = m_curPath; //OpenDir();
	{	
		theApp.SetSchudlingFlg(true);

		theApp.m_couldCreatWorkChange=FALSE;   // 工作变更不能生成
		//static_cast<CMainFrame*>(m_pMainWnd)->sShowImportInfo(strPath);  // add 
		list<UINT> selectedResult;
		list<UINT>::iterator idIter;
		BSTR path;
		time_t startTm=0;
		CString cstrCode=L"";
		g_schCtr = FALSE;
		BeginWaitCursor();
		path = strPath.AllocSysString();
		CString info_path=m_strFile;
		CloseForLoadSch();
		m_strFile=info_path;   // 还原路径

		cstrCode=L"";

		BIZAPI::BizClearEraseData();

		SetMsgInfo(ID_UI_BAR_BEGINLOADFILE,cstrCode,false,MSG_TYPE_IO);		// 开始导入
		startTm=clock();
		BIZAPI::GetBizMap()->ReSetObjCount();

		DATATABLAPI::SetTimeUnit(BIZAPI::GetBizMap()->GetSchSysParam()->GetSystemTimeUnit());  //设置DATATABLAPI的时间类型

		//开线程导入数据
		/*g_bloadFileEndFlg = FALSE;
		g_bLoadWorkFlg = FALSE;
		ThreadParamer tp;
		tp.m_flg = true;
		tp.m_str = m_strFile;
		tp.m_n = 0;
		uintptr_t nTread = _beginthreadex(NULL,0,Run_LoadFile,(void*)&tp,0,NULL);
		while(!g_bloadFileEndFlg)
		{
			Sleep(250);
		}*/
		//CReadFiles rf;
		g_bLoadWorkFlg = DATATABLAPI::ReadAll(path);
		BIZAPI::GetBizMap()->SetRecentlyEvenTm(_time64(NULL));
		double endTm=(clock()-startTm)/1000.0f;

		YKBizMap* pBizMap = BIZAPI::GetBizMap();
		if ( pBizMap != NULL )
			YKExtraValue::SetTimeUnit(pBizMap->GetSchSysParam()->GetSystemTimeUnit());  //设置界面的时间类型

		//g_bLoadWorkFlg = rf.GetWorkInputFlg();_____________________
		// 工作导入标志
		if(g_bLoadWorkFlg)
			BIZAPI::DoSchedWork();
		m_bIsLoad = TRUE;
		m_bOpen = TRUE;

		BIZAPI::GetBizMap()->InitRelation();

		CheckInputData();

		//Add 2012-01-06	//导入时检验日历和班次是否为空，若为空设置指定的值
#ifdef PMCMODE
		BIZAPI::GetBizMap()->CheckCalendarAndDayOfShift();
#endif

		// 初始化BOM	add 2010-3-10 by ll
		//BIZAPI::GetBizMap()->InitForBOM();

		::SendMessage(theApp.g_hMainFrm, ID_MESSAGE_INITTREE,0,0);
		::SendMessage(g_hMainFrm,_ACTIVITE_VIWE_,123,0);
		ChangeShowPrecision();

		BIZAPI::TransFormAlg(true,true);
		BIZAPI::CopyPlanToSRes();
		BIZAPI::GetBizMap()->AdjustData();

		BIZAPI::GetBizMap()->InitInCmdCodeMap();
		BIZAPI::GetBizMap()->InitOutCmdCodeMap();
		BIZAPI::GetBizMap()->InitItemBomIdList();

		EndWaitCursor();
		/*double endTm=(clock()-startTm)/1000.0f;*/
		cstrCode.Format(L"( %f",endTm);
		cstrCode+=GetRemarkFiled(ID_UI_BAR_SECOND).c_str();
		cstrCode+=L")";
		cstrCode+=strPath;
		SetMsgInfo(ID_UI_BAR_INPORTDATA,cstrCode,TRUE,MSG_TYPE_IO);			// 导入数据
		cstrCode.Format(L"%d",BIZAPI::GetBizMap()->GetObjCount());
//		SetMsgInfo(ID_UI_BAR_OBJCOUNT,cstrCode,TRUE,MSG_TYPE_IO);			// 总对象数	
#ifdef LIMITFUNCTION
		BIZAPI::LimitCheck();
#endif
		::SendMessage(theApp.g_hMainFrm, ID_MESSAGE_FLASH_OUTPUTWND,0,0);	
		g_schLevel = 0;
		::SysFreeString(path);

		theApp.SetSchudlingFlg(false);
	}
}

void CSchedulerApp::OnUpdateYkSaveAs(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnYkSaveAs()
{
	// TODO: Add your command handler code here
	SaveAs();
}


CString CSchedulerApp::OpenDir()
{
	CString strDir;

	CWnd * pWnd = theApp.GetMainWnd();

	if ( !pWnd ) return strDir;

	BROWSEINFO bi;
	ITEMIDLIST* pidl;
	/*ITEMIDLIST* root = new ITEMIDLIST();
	WCHAR chroot[20] = L"D:\\";
	root->mkid.cb = 20;
	root->mkid.abID[0] = BYTE(DWORD(&chroot)<<8);
	root->mkid.abID[1] = BYTE(DWORD(&chroot)>>8);*/
	//RegLoadKey()
	TCHAR buf[MAX_PATH];
	memset(buf,0,MAX_PATH);

	bi.hwndOwner		= pWnd->GetSafeHwnd();
	bi.pidlRoot			= NULL;
	bi.pszDisplayName	= buf;
	bi.lpszTitle		= GetRemarkFiled(ID_UI_TEXT_CHOOSEOPENFILE).c_str();   // _T("请选择打开的文件夹")
	bi.ulFlags			= BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN ;
	bi.lpfn				= NULL;
	bi.lParam			= 0;
	bi.iImage			= IDR_MAINFRAME;

	pidl = SHBrowseForFolder(&bi);
	SHGetPathFromIDList(pidl,buf);

	strDir = buf;
	LPMALLOC lpMalloc;
	if( FAILED(SHGetMalloc(&lpMalloc)) ) 
		return strDir;

	lpMalloc->Free(pidl);
	lpMalloc->Release();
	//delete root;
	return strDir;
}



void CSchedulerApp::OnExportcsv()
{
	YK_SHORT nOpType = AfxMessageBox( GetRemarkFiled(ID_PROMT_EXPORT).c_str() ,MB_YESNO|MB_ICONQUESTION);
	if ( nOpType == IDNO )
		return;

	bool bBackup = false;
	CDialogExpoMessageBox dlg;
	dlg.DoModal() ;	//OK覆盖
	bBackup = dlg.GetBackupFlag();

	if(m_curPath.IsEmpty())
	{
		WCHAR wPath[256];
		GetModuleFileName(NULL,wPath,256);
		CString curProPath = GetFilePath(CString(wPath));
	}

	CString strPath = m_curPath;//OpenDir();

//	if ( !strPath.IsEmpty() )
	{	
		theApp.SetSchudlingFlg(true);
		BSTR path = strPath.AllocSysString();

		CString cstrCode;
		time_t startTm=clock();
		cstrCode=L"";		
		SetMsgInfo(ID_UI_BAR_BEGINEXPROCSV,strPath,false,MSG_TYPE_IO);			// 开始导出数据
		m_bIsLoad = FALSE;

		DATATABLAPI::WriteFile(path,bBackup);
		double endTm=(clock()-startTm)/1000.0f;

		cstrCode.Format(L"( %f ",endTm);
		cstrCode+=GetRemarkFiled(ID_UI_BAR_SECOND).c_str();
		cstrCode+=L")";
		cstrCode+=strPath;
		SetMsgInfo(ID_UI_BAR_EXPORTDATA,cstrCode,TRUE,MSG_TYPE_IO);		// 导出数据
		// 更新消息窗口
		::SendMessage(m_pMainWnd->GetSafeHwnd(),ID_MESSAGE_FLASH_OUTPUTWND,0,0);
		::SendMessage(g_hMainFrm,_ACTIVITE_VIWE_,123,0);     
		::SysFreeString(path);
		SaveSystemSet();
		theApp.SetSchudlingFlg(false);
	}
}

void CSchedulerApp::OnUpdateYkScheduler(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_
	if(m_bOpen && (BIZAPI::GetBizMap()->GetApsParam() != NULL && BIZAPI::GetBizMap()->GetApsParam()->GetStopPatchCondition() != 2))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void CSchedulerApp::OnUpdateYkResGantt(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_;
	if (HideTable(ID_GANTT_RES_GL))
		return pCmdUI->Enable(FALSE);
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnUpdateYkOrderGantt(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_;
	if (HideTable(ID_GANTT_ORDER_GL))
		return pCmdUI->Enable(FALSE);
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnUpdateYkResRate(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_;
	if (HideTable(ID_RATE_RES_GL))
		return pCmdUI->Enable(FALSE);
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnUpdateYkStagRate(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_;
	if (HideTable(ID_RATE_STA_GL))
		return pCmdUI->Enable(FALSE);
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnUpdateQuerys(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_
	
	BOOL bEnable = TRUE;
	if (g_tblType <= 0 || !m_bOpen) 
	{
		bEnable = FALSE;
	}
	pCmdUI->Enable(bEnable);
}

 void CSchedulerApp::OnUpdateYKFilter(CCmdUI *pCmdUI)
 {
	 _LIMIT_CHECK_

	 BOOL bEnable = FALSE;
	 if (g_tblType > 0 || m_bOpen)
	 {
		 CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
		 if (NULL != pCurView)
		 {
			 const UINT uTblType = pCurView->GetTblType();
			 if (uTblType == ID_GANTT_RES_GL || uTblType == ID_GANTT_ORDER_GL ||
				 uTblType == ID_RATE_RES_GL || uTblType == ID_RATE_STA_GL ||
				 uTblType == ID_Item_Gather_GL || uTblType == ID_WORK_RESOURCE_GD ||
				 uTblType == TblItemGather ||
				 (uTblType && !(uTblType == TblSpec || uTblType == TblSchSysParam || 
				                uTblType == TblGanttPara || uTblType == TblReducePeriod)
				 ))
			 {
				 bEnable = TRUE;
			 }
		 }
	 }

	 pCmdUI->Enable(bEnable);
 }
 void CSchedulerApp::OnUpdateUnYKFilter(CCmdUI *pCmdUI)
 {
	 _LIMIT_CHECK_

	 BOOL bEnable = FALSE;
	 if (g_tblType > 0 || m_bOpen)
	 {
		 CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
		 if (NULL != pCurView)
		 {
			 const UINT uTblType = pCurView->GetTblType();
			 if (pCurView->m_bIsTalbe && 
				 !(uTblType == TblSpec || uTblType == TblSchSysParam ||
				   uTblType == TblApsParam || uTblType == TblGanttPara ||
				   uTblType == TblReducePeriod || uTblType == TblSpecItem || 
				   uTblType == TblQualityLevel)
				 )
			 {
				bEnable = TRUE;
			 }
		 }
	 }

	 pCmdUI->Enable(bEnable);
 }


void CSchedulerApp::OnAllFix()
{
	void* workSq = GetWorkSequence();
	if( workSq != NULL)
	{
		((CWorkSequence*)workSq)->SetAlllock();
	}
	else
	{
		CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
		if (NULL != pCurView)
			GanttCommonApi::OnAllFix(pCurView->GetTblType());
	}
}

void CSchedulerApp::OnTimeFix()
{
	void* workSq = GetWorkSequence();
	if( workSq != NULL)
	{
		((CWorkSequence*)workSq)->SetTmlock();
	}
	else
	{
		CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
		if (NULL != pCurView)
			GanttCommonApi::OnTimeFix(pCurView->GetTblType());
	}
}

void CSchedulerApp::OnAloneSure()
{
	void* workSq = GetWorkSequence();
	if( workSq != NULL)
	{
		((CWorkSequence*)workSq)->SetAloneSure();
	}
	else
	{
		CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
		if (NULL != pCurView)
			GanttCommonApi::OnAloneSure(pCurView->GetTblType());
	}
}

void CSchedulerApp::OnCompleteOder()
{
	void* workSq = GetWorkSequence();
	if( workSq != NULL)
	{
		((CWorkSequence*)workSq)->SetCompleteOder();
	}
	else
	{
		CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
		if (NULL != pCurView)
			GanttCommonApi::OnCompleteOder(pCurView->GetTblType());
	}
}

void CSchedulerApp::OnPlanOver()
{
	void* workSq = GetWorkSequence();
	if( workSq != NULL)
	{
		((CWorkSequence*)workSq)->SetPlanOver();
	}
	else
	{
		CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
		if (NULL != pCurView)
			GanttCommonApi::OnPlanOver(pCurView->GetTblType());
	}
}

void CSchedulerApp::OnAllWork()
{
	CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	if (NULL != pCurView)
		GanttCommonApi::OnAllSelect(pCurView->GetTblType());
}

void CSchedulerApp::OnResFix()
{
	void* workSq = GetWorkSequence();
	if( workSq != NULL)
	{
		((CWorkSequence*)workSq)->SetReslock();
	}
	else
	{
		CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
		if (NULL != pCurView)
			GanttCommonApi::OnResFix(pCurView->GetTblType());
	}
}
void CSchedulerApp::OnReFix()
{
	void* workSq = GetWorkSequence();
	if( workSq != NULL)
	{
		((CWorkSequence*)workSq)->SetUnlock();
	}
	else
	{
		CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
		if (NULL != pCurView)
			GanttCommonApi::OnReFix(pCurView->GetTblType());
	}
}

void CSchedulerApp::OnAllFix(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	if(IsGanttOpen()&& GanttCommonApi::HadSelectWork())
		pCmdUI->Enable(TRUE);
	else if(GetWorkSequence() != NULL)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
	if( !BIZAPI::GetCommLimit(  TblWork )
		&&!BIZAPI::GetCommLimit(  TblOrder ))
		pCmdUI->Enable(FALSE);
}
void CSchedulerApp::OnTimeFix(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	if(IsGanttOpen()&& GanttCommonApi::HadSelectWork())
		pCmdUI->Enable(TRUE);
	else if(GetWorkSequence() != NULL)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
	if( !BIZAPI::GetCommLimit(  TblWork )
		&&!BIZAPI::GetCommLimit(  TblOrder ))
		pCmdUI->Enable(FALSE);
}
void CSchedulerApp::OnAloneSure(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	if(IsGanttOpen()&& GanttCommonApi::HadSelectWork())
		pCmdUI->Enable(TRUE);
	else if(GetWorkSequence() != NULL)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
	if( !BIZAPI::GetCommLimit(  TblWork )
		&&!BIZAPI::GetCommLimit(  TblOrder ))
		pCmdUI->Enable(FALSE);
}
void CSchedulerApp::OnCompleteOder(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	if(IsGanttOpen()&& GanttCommonApi::HadSelectWork())
		pCmdUI->Enable(TRUE);
	else if(GetWorkSequence() != NULL)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
	if( !BIZAPI::GetCommLimit(  TblWork )
		&&!BIZAPI::GetCommLimit(  TblOrder ))
		pCmdUI->Enable(FALSE);
}
void CSchedulerApp::OnPlanOver(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	if(IsGanttOpen()&& GanttCommonApi::HadSelectWork())
		pCmdUI->Enable(TRUE);
	else if(GetWorkSequence() != NULL)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
	if( !BIZAPI::GetCommLimit(  TblWork )
		&&!BIZAPI::GetCommLimit(  TblOrder ))
		pCmdUI->Enable(FALSE);
}
void CSchedulerApp::OnAllWork(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	if(IsGanttOpen())
		pCmdUI->Enable(TRUE);
	//if(GetWorkSequence() != NULL)
	//	pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
	if( !BIZAPI::GetCommLimit(  TblWork )
		&&!BIZAPI::GetCommLimit(  TblOrder ))
		pCmdUI->Enable(FALSE);
}
void CSchedulerApp::OnResFix(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	if(IsGanttOpen()&& GanttCommonApi::HadSelectWork())
		pCmdUI->Enable(TRUE);
	else if(GetWorkSequence() != NULL)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
	if( !BIZAPI::GetCommLimit(  TblWork )
		&&!BIZAPI::GetCommLimit(  TblOrder ))
		pCmdUI->Enable(FALSE);
}

void CSchedulerApp::OnReFix(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	if(IsGanttOpen()&& GanttCommonApi::HadSelectWork())
		pCmdUI->Enable(TRUE);
	else if(GetWorkSequence() != NULL)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
	if( !BIZAPI::GetCommLimit(  TblWork )
		&&!BIZAPI::GetCommLimit(  TblOrder ))
		pCmdUI->Enable(FALSE);
}

bool CSchedulerApp::IsGanttOpen( UINT type )
{
	if (g_tblType <= 0) 
	{		
		return false;
	}
	if (type > 0)
	{
		CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
		if (NULL != pCurView && type == pCurView->GetTblType())
			return true;
	}
	else
	{
		CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
		if (NULL != pCurView)
		{
			const UINT uTblType = pCurView->GetTblType();
			if (uTblType == ID_GANTT_RES_GL || uTblType == ID_GANTT_ORDER_GL ||
				uTblType == ID_RATE_RES_GL || uTblType == ID_RATE_STA_GL ||
				uTblType == ID_WORK_RESOURCE_GD || uTblType == ID_Item_Gather_GL)
			{
				return true;
			}
		}
	}
	return false;
}

void CSchedulerApp::OnUndo()
{
	BIZAPI::Undo();
	CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	if (NULL != pCurView)
	{
		pCurView->Flash();
	}
	/*CUndoRedoManager* pURMgr = BIZAPI::GetURMgr();
	YKBizMap* pMap = BIZAPI::GetBizMap();
	if (pURMgr != NULL)
	{
		if (pURMgr->Undo())
		{
			int type = -1;
			CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
			if (NULL != pCurView)
			{
				CURCell& urCell = pURMgr->GetCurSavePoint();
				if (urCell.m_type == Cell_GD)
				{
					BIZAPI::Undo();
				}
				else
				{

					if (urCell.m_event == 1000)
					{	
						BIZAPI::Undo();
						YK_SHORT moveType = pMap->GetDragManner();
						pMap->SetDragManner(Grag_No_Limited);
						GanttCommonApi::Undo(pCurView->GetTblType(), false);
						pMap->SetDragManner(moveType);

					}
					else if (urCell.m_event == 1001)
					{
						BIZAPI::Undo();
						BIZAPI::Undo();
						BIZAPI::Undo();
						pMap->RedrawAllGantt();
					}
					else
					{
						GanttCommonApi::Undo(pCurView->GetTblType());
					}

				}
				pCurView->Flash();
				::SendMessage(theApp.g_hMainFrm,ID_MESSAGE_FLASH_OUTPUTWND,0,0);
				::SendMessage(theApp.g_hMainFrm,IDU_UPDATE_WACTH_BAR,0,0);
				::SendMessage(theApp.g_hMainFrm,IDU_INIT_APS_COMBOBOX,0,0);
				::SendMessage(theApp.g_hMainFrm,ID_INIT_RESOURCE,0,0);
			}
		}
	}*/
}

void CSchedulerApp::OnRedo()
{
	BIZAPI::Redo();
	CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	if (NULL != pCurView)
	{
		pCurView->Flash();
	}
// 	CUndoRedoManager* pURMgr = BIZAPI::GetURMgr();
// 	YKBizMap* pMap = BIZAPI::GetBizMap();
// 	if (pURMgr != NULL)
// 	{
// 		if (pURMgr->Redo())
// 		{
// 			int type = -1;
// 			CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
// 			if (NULL != pCurView)
// 			{
// 				CURCell& urCell = pURMgr->GetCurSavePoint();
// 				if (urCell.m_type == Cell_GD)
// 				{
// 					BIZAPI::Redo();
// 				}
// 				else
// 				{
// 					if (urCell.m_event == 1000)
// 					{	
// 						YK_SHORT moveType = pMap->GetDragManner();
// 						pMap->SetDragManner(Grag_No_Limited);
// 						GanttCommonApi::Redo(pCurView->GetTblType(), false);
// 						pMap->SetDragManner(moveType);
// 						BIZAPI::Redo();
// 
// 
// 					}
// 					else if (urCell.m_event == 1001)
// 					{
// 						BIZAPI::Redo();
// 						BIZAPI::Redo();
// 						BIZAPI::Redo();
// 						pMap->RedrawAllGantt();
// 					}
// 					else
// 					{
// 						GanttCommonApi::Redo(pCurView->GetTblType());
// 					}
// 				}
// 				pCurView->Flash();
// 				::SendMessage(theApp.g_hMainFrm,ID_MESSAGE_FLASH_OUTPUTWND,0,0);
// 				::SendMessage(theApp.g_hMainFrm,IDU_UPDATE_WACTH_BAR,0,0);
// 				::SendMessage(theApp.g_hMainFrm,IDU_INIT_APS_COMBOBOX,0,0);
// 				::SendMessage(theApp.g_hMainFrm,ID_INIT_RESOURCE,0,0);
// 			}
// 		}
// 	}
}

void CSchedulerApp::OnUpdateUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(BIZAPI::CanUndo() ? TRUE : FALSE);
// 	CUndoRedoManager* pURMgr = BIZAPI::GetURMgr();
// 	if (pURMgr != NULL)
// 	{
// 		if (pURMgr->IsCanUndo())
// 			pCmdUI->Enable(TRUE);
// 		else
// 			pCmdUI->Enable(FALSE);
// 
// 	}
// 	else
// 		pCmdUI->Enable(FALSE);
}

void CSchedulerApp::OnUpdateRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(BIZAPI::CanRedo() ? TRUE : FALSE);
	/*CUndoRedoManager* pURMgr = BIZAPI::GetURMgr();
	if (pURMgr != NULL)
	{
		if (pURMgr->IsCanRedo())
			pCmdUI->Enable(TRUE);
		else
			pCmdUI->Enable(FALSE);

	}
	else
		pCmdUI->Enable(FALSE);*/
}

CString CSchedulerApp::GetTempFileName()
{
	CString strPath;

	CTime tm = CTime::GetCurrentTime();

	CString tmpPath;
	::GetTempPath(MAX_PATH,tmpPath.GetBuffer(MAX_PATH));
	tmpPath.ReleaseBuffer();

	strPath = tmpPath;
	strPath += tm.Format(_T("%Y%m%d%H%M%S.tmp"));

	return strPath;
}

void CSchedulerApp::OnMySave()
{
	// TODO: Add your command handler code here
	//SaveAs();
	//static_cast<CMainFrame*>(m_pMainWnd)->sShowSavefile(m_strFile);
	CString cstrCode;
	cstrCode=L"";

	//SetMsgInfo(ID_UI_BAR_FILESAVE,m_strFile,TRUE,MSG_TYPE_SYS);		// 保存数据
	YK_WSTRING wstr1, wstr2;
	DATATABLAPI::SaveYks(wstr1, wstr2);
	return;
	SaveOrSaveAs();
}

bool CSchedulerApp::SaveOrSaveAs()
{
	//保存界面
	if ( m_bOpen && !m_strFile.IsEmpty())
	{	
		BeginWaitCursor();

		((CMainFrame*)m_pMainWnd)->SaveWindowsState();

		CString strFileName = m_strFile ;

		WCHAR strFind(L'.');
		int index = strFileName.ReverseFind(strFind);
		CString str =  strFileName.Left(index);


		__int64 tme = _time64(NULL);

		const INT maxTimeBufferSize = 128;

		static LPCTSTR pFormat = L"%m%d-%H%M%S";

		WCHAR szBuffer[maxTimeBufferSize];

		struct tm ptmTemp;
		errno_t err = _localtime64_s(&ptmTemp, &tme);
		if ( err != 0 ||!_tcsftime(szBuffer, maxTimeBufferSize, pFormat, &ptmTemp))
		{
			szBuffer[0] = '\0';
		}

		str += L"-";

		str += szBuffer;

		str += L".yks";

		CopyFile(m_strFile,str,true);

		YK_WSTRING strPath = m_strFile.GetString();
		YK_WSTRING strVersion = m_strVersion.GetString();
		time_t  startTm=clock();
		DATATABLAPI::SaveYks(strPath,strVersion);
		double  endTm=(clock()-startTm)/1000.0;

		CString strCode;
		strCode.Format(L"( %f ",endTm);
		strCode+=GetRemarkFiled(ID_UI_BAR_SECOND).c_str();
		strCode+=L")";
		strCode+=strPath.c_str();
		SetMsgInfo(ID_UI_BAR_FILESAVE,strCode,TRUE,MSG_TYPE_SYS);		// 保存数据
		//SetMsgInfo(ID_UI_BAR_FILEOSAVE,strCode,TRUE,MSG_TYPE_SYS);		// 保存文件

		EndWaitCursor();

		// 更新消息窗口
		::SendMessage(m_pMainWnd->GetSafeHwnd(),ID_MESSAGE_FLASH_OUTPUTWND,0,0);
		return true;
	}
	else
	{
		return SaveAs();
	}
}

#include "DialogPSI.h"
void CSchedulerApp::OnYkhelp()
{

	if(m_pszHelpFilePath != NULL)
	{
		//delete m_pszHelpFilePath;
		free((void*)m_pszHelpFilePath);

		m_helpPath = m_curProPath;
		m_helpPath += L"\\";
		m_helpPath += GetRemarkFiled(ID_UI_YKSCHEDULER_HELP).c_str();

		int size = (int)(m_helpPath.GetLength()+1)* sizeof(TCHAR);
		TCHAR* path = new TCHAR[size];

		memcpy(path,m_helpPath.GetString(),size);
		m_pszHelpFilePath = path;
	}

	CString str;
	int helpId = HI_HID_Default;

#ifdef PMCMODE
	HtmlHelp(0, HH_DISPLAY_TOPIC);
#else
	CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	if (NULL != pCurView)
	{
		helpId = pCurView->GetSelectFiled();
		if (helpId != -1)
			HtmlHelp(helpId, HH_HELP_CONTEXT);
	}
	else
	{
		HtmlHelp(helpId, HH_HELP_CONTEXT);
	}
#endif
}

void CSchedulerApp::OnUpdateExportcsv(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnUpdateMySave(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bOpen);
	// TODO: Add your command update UI handler code here
}


void CSchedulerApp::OnYkPrint()
{
	if(IsGanttOpen(ID_GANTT_RES_GL)) 
	{
		GanttApi::PrintGantt( );
		return;
	}
	if(IsGanttOpen(ID_GANTT_ORDER_GL)) 
	{
		OrderGanttApi::PrintGantt();
		return;
	}
	if(IsGanttOpen(ID_RATE_RES_GL)) 
	{
		ResRateGanttApi::PrintGantt();
		return;
	}
	if(IsGanttOpen(ID_RATE_STA_GL)) 
	{
		StockGanttApi::PrintGantt();
		return;
	}
	if (IsGanttOpen(ID_Item_Gather_GL))
	{
		ItemGatherApi::PrintGantt();
		return;
	}
	//if (IsGanttOpen(ID_WORK_RESOURCE_GD))
	//{
	//	ResRateGanttApi::PrintGantt( );
	//	return;
	//}

	//表格打印质量不好，暂时去掉
	CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	if (NULL != pCurView)
	{
		if(pCurView->m_pGdCtrl.GetSafeHwnd())
			pCurView->m_pGdCtrl.Print();

		if (pCurView->m_pWorkSe.GetSafeHwnd())
		{
			pCurView->m_pWorkSe.Print();
		}
#ifndef _REMOVE_ITEMGATHER_
		else if (pCurView->GetTblType() == TblItemGather)
		{
			pCurView->GetItemGatherGridCtrl()->Print();
		}
#endif
	}
}

void CSchedulerApp::OnUpdateYkPrint(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_

	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnMenuNew()
{

	TCHAR szFilters[]= _T("YukonSys data (*.yks)|*.yks|All Files (*.*)|*.*||");

	CFileDialog  openFile(FALSE,_T("yks"),_T("Data.yks"),OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters);

//	openFile.SetWindowText(L"请选择路径:");
//	openFile.GetFileTitle()

	if ( IDOK == openFile.DoModal() )
	{

		Close();
#ifndef  PMCMODE
		//模型选择 
		CLoadModeDlg dlg;
		if (dlg.DoModal() != IDOK)
			return ;

		BIZAPI::GetBizMap()->SetAreaSchPara(dlg.GetSelectMode());
#else
		BIZAPI::GetBizMap()->SetAreaSchPara(1);
#endif
		BIZAPI::GetBizMap()->GetYKLimitOfAuthorPtrMap()->InitDefault();

		// 初始化字段信息 建立规则字段索引
		BIZAPI::InitInsidePropertyManager();
		DATATABLAPI::InitTableModeManager(L"");		// 使用内部定义初始化数据
		DATATABLAPI::InitLoadFileInfo();

//		BIZAPI::GetBizMap()->InitApsParam();

		// 新建项目时根据语言设置时间显示格式  2010-7-19
		if(theApp.m_langueType == LANGUAGE_CHS)
			BIZAPI::GetBizMap()->SetTimeShowType(10);
		else if(theApp.m_langueType == LANGUAGE_US)
			BIZAPI::GetBizMap()->SetTimeShowType(20);

		CString strFile = openFile.GetPathName();
		m_curPath = GetFilePath(strFile);

		m_strFile = strFile;

		YK_WSTRING strPath = strFile.GetString();
		YK_WSTRING strVerison =m_strVersion.GetString();

		//Add 2012-01-05 添加PMC 系统默认排程参数
#ifdef PMCMODE
		BIZAPI::GetBizMap()->CreatePMCApsRunMethodRule();
#else
		BIZAPI::GetBizMap()->CreatApsRunMethodRule();
#endif
		BIZAPI::GetBizMap()->GetSchSysParam()->SetStrFilePath(strPath);
		DATATABLAPI::SaveYks(strPath,strVerison);
		BIZAPI::GetBizMap()->SetSchTmAndShowTm();
		//WCHAR wPath[256];
		//GetCurrentDirectory(256,wPath);

		BIZAPI::GetBizMap()->InitInCmdCodeMap();
		BIZAPI::GetBizMap()->InitOutCmdCodeMap();

		
		m_bOpen = TRUE;	
		::SendMessage(theApp.g_hMainFrm, ID_MESSAGE_INITTREE,0,0);
		//static_cast<CMainFrame*>(m_pMainWnd)->sShowNewSchedul(strFile);
		CString cstrCode=L"";
		SetMsgInfo(ID_UI_BAR_WELUSEYKSCHEDULER,cstrCode,false,MSG_TYPE_SYS);		// 欢迎使用APS
		SetMsgInfo(ID_UI_MSG_TYPE_BLANK,cstrCode,false,MSG_TYPE_SYS);		// 增加1行空白行	
		SetMsgInfo(ID_UI_BAR_NEWSCHEDULER,strFile,true,MSG_TYPE_SYS);		// 新建数据
		// 设置加载时间
		CDealTime cloadtime=CDealTime::GetLocalTime();
		BIZAPI::GetBizMap()->SetLoadTm(cloadtime);
		BIZAPI::GetBizMap()->ResetProjectCode();
		theApp.m_couldCreatWorkChange=FALSE;   // 工作变更不能生成

		::SendMessage(theApp.g_hMainFrm, IDU_INIT_APS_COMBOBOX,0,0);		// 初始化APS 排程参数选择框  add 2010-8-31 
		::SendMessage(theApp.g_hMainFrm, ID_MESSAGE_FLASH_OUTPUTWND,0,7);
	}

}

int CSchedulerApp::FinalClose()
{
	if (m_bOpen)
	{
		return MessageBox(m_pMainWnd->GetSafeHwnd(),GetRemarkFiled(ID_UI_OUT_MESSAGE_BOX_STRING).c_str(),GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str(),MB_YESNOCANCEL);
	}
	else
		return IDNO;
}


BOOL CSchedulerApp::OpenSystemSet(int& bAuto, CString& strPath)
{
	BOOL bOK = FALSE;
	YKBizMap* pMap = BIZAPI::GetBizMap();
	
	CBCGPRegistry reg(FALSE, FALSE);
	CString strKey(L"YKSystemSet");
	CString strValue1(L"bAuto");
	CString strValue2(L"YKSPath");
	CString strValue3(_T("Language"));
	CString strValue4(_T("SelectMode"));
	CString strValue5(_T("SystemDayTime"));
	CString strValue6(_T("SystemSerial"));
	if (reg.Open(strKey))
	{
		if (reg.Read(strValue1, bAuto))
		{
			pMap->SetAntoOpenYKS(1 == bAuto);
			m_bAutoOpen = bAuto;
		}
		if (reg.Read(strValue2, strPath))
		{
			pMap->SetStrFilePath(strPath.GetString());
		}
		CString lan;
		if (reg.Read(strValue3, lan))
		{
			m_langueType = _ttoi(lan.GetString());
		}
		CString selectMode;
		if(reg.Read(strValue4, selectMode))
		{
			pMap->GetSchSysParam()->SetAutoApsSelect(_ttoi(selectMode.GetString())) ;
		}
		CString  strDayTime;
		__int64  dayTime = CDealTime::GetLocalTime().GetDate().GetTime();
		if (reg.Read(strValue5, strDayTime))
		{
			dayTime = _ttoi64(strDayTime.GetString());
		}
		int iSeral(0);
		reg.Read(strValue6,iSeral);
		InitSysSerialNumber(dayTime, iSeral);
		bOK = TRUE;
	}
	return bOK;
}

BOOL CSchedulerApp::SaveSystemSet()
{
	if(!m_bOpen) return FALSE;
	YKBizMap* pMap = BIZAPI::GetBizMap();
	if (pMap != NULL)
	{
		CBCGPRegistry reg(FALSE,FALSE);
		CString strKey(L"YKSystemSet");
		CString strValue1(L"bAuto");
		CString strValue2(L"YKSPath");
		CString strValue3(_T("Language"));
		CString strValue4(_T("SelectMode"));
		CString strValue5(_T("SystemDayTime"));
		CString strValue6(_T("SystemSerial"));
		if (reg.CreateKey(strKey))
		{
			if (pMap->GetAntoOpenYKS())
				reg.Write(strValue1,1);
			else
				reg.Write(strValue1,0);
			reg.Write(strValue2,pMap->GetStrFilePath().c_str());
			CString strLan;
			strLan.Format(_T("%d"),m_langueType);
			reg.Write(strValue3,strLan);

			CString selectMode;
			selectMode.Format(_T("%d"),pMap->GetSchSysParam()->GetAutoApsSelect());
			reg.Write(strValue4,selectMode);
		
			__int64  dayTime(0);
			long   iSeral(0);
			GetSysSerialNumber(dayTime,iSeral);
			if (dayTime < 0)
			{
				dayTime = CDealTime::GetLocalTime().GetDate().GetTime();
			}
			CString strDayTime;
			strDayTime.Format(_T("%ld"),dayTime);
			reg.Write(strValue5,strDayTime);
			reg.Write(strValue6,iSeral);
			return TRUE;
		}
		
	}
	return FALSE;
}

//去掉最后一个'\\'后的字符串
CString CSchedulerApp::GetFilePath( CString& strFileName )
{
	WCHAR strFind(L'\\');
	int index = strFileName.ReverseFind(strFind);
	CString str =  strFileName.Left(index);
	return str;
}

void CSchedulerApp::OnUpdateMenuNew(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	pCmdUI->Enable(!m_bOpen);
}


void CSchedulerApp::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
{
	// TODO: Add your specialized code here and/or call the base class

	__super::HtmlHelp(dwData, nCmd);
}

CDocument* CSchedulerApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	// TODO: Add your specialized code here and/or call the base class

	return __super::OpenDocumentFile(lpszFileName);
}

void CSchedulerApp::OnDeleteF12()
{

}

void CSchedulerApp::OnLoadFileSetting()
{
	//g_tblType=TblLoadFileInfo;
	//CSchedulerApp::OnFileNew();
	OpenTblPage(TblLoadFileInfo);
	
}
void CSchedulerApp::OnUpdateLoadFileSetting(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_
	pCmdUI->Enable(m_bOpen);
}
void CSchedulerApp::ShowDataErrorView(UINT tblType,short errType,int row,int cor,YK_WSTRING strItem,YK_ULONG itemid)
{
	return ;		// 不提示
	switch(errType)
	{
	case EOR_TYPE_NONE:// 正确
	case EOR_TYPE_NONE_REFLSH:
		return;
		break;
	}
	// new error
	CMessageData msgData; msgData.New();
	CError errorInfo;
	errorInfo.clear();
	errorInfo.m_errorType=(EOR_TYPE)errType;						// 消息类型
	errorInfo.m_Tbl=tblType;							// 消息发生所在表格
	errorInfo.m_TblCode=GetRemarkFiled(tblType);
	if (errType != EOR_TYPE_DELETE)
	{
		errorInfo.m_Field=cor;
		errorInfo.m_FieldCode=BIZAPI::GetFieldCode(tblType,cor);
	}
	switch(errType)
	{
	case EOR_TYPE_REPEAT://代码重复
		{
			if (m_errorVar.vt == VT_BSTR)
			{
				CString errorCode = m_errorVar.bstrVal;
				errorInfo.m_errorFieldCode = errorCode.GetString();
			}
			msgData.SetMessageData(MSG_LEVEL_WARNING,MSG_TYPE_VIEWGRID,TOOLS::FormatTime(CDealTime::GetLocalTime(),true),errorInfo);
			break;
		}
	case EOR_TYPE_NULL://必填项为空
		{
			msgData.SetMessageData(MSG_LEVEL_WARNING,MSG_TYPE_VIEWGRID,TOOLS::FormatTime(CDealTime::GetLocalTime(),true),errorInfo);
			break;
		}
	case EOR_TYPE_FORMAT://格式错误
		{
			if (m_errorVar.vt == VT_BSTR)
			{
				CString errorCode = m_errorVar.bstrVal;
				errorInfo.m_errorFieldCode = errorCode.GetString();
			}
			msgData.SetMessageData(MSG_LEVEL_WARNING,MSG_TYPE_VIEWGRID,TOOLS::FormatTime(CDealTime::GetLocalTime(),true),errorInfo);
			break;
		}
	case EOR_TYPE_LOGIC://逻辑错误
		{
			msgData.SetMessageData(MSG_LEVEL_WARNING,MSG_TYPE_VIEWGRID,TOOLS::FormatTime(CDealTime::GetLocalTime(),true),errorInfo);
			break;
		}
	case EOR_TYPE_CYCLE://循环错误
		{
			msgData.SetMessageData(MSG_LEVEL_ERROR,MSG_TYPE_VIEWGRID,TOOLS::FormatTime(CDealTime::GetLocalTime(),true),errorInfo);
			break;
		}
	case EOR_TYPE_DELETE://删除
		{
			errorInfo.m_count=cor;
			msgData.SetMessageData(MSG_LEVEL_ERROR,MSG_TYPE_VIEWGRID,TOOLS::FormatTime(CDealTime::GetLocalTime(),true),errorInfo);
			break;
		}
	case EOR_TYPE_LIMIT:
		{
			if (m_errorVar.vt == VT_BSTR)
			{
				CString errorCode = m_errorVar.bstrVal;
				errorInfo.m_errorFieldCode = errorCode.GetString();
			}
			msgData.SetMessageData(MSG_LEVEL_WARNING,MSG_TYPE_VIEWGRID,TOOLS::FormatTime(CDealTime::GetLocalTime(),true),errorInfo);
			break;
		}
	default:
		{
			msgData.Free();
			return;
			break;
		}
	}
	if(errType!= EOR_TYPE_DELETE)
		msgData.AddRelId(itemid);
	//MessageBox(NULL,errd.GetErrorMsg().c_str(), L"提示", MB_OK);
	//static_cast<CMainFrame*>(m_pMainWnd)->sShowDataErrorView(msgData->GetMessageCode().c_str());
	//BIZAPI::GetBizMap()->InsertBiz(msgData);
	::SendMessage(m_pMainWnd->GetSafeHwnd(),ID_MESSAGE_FLASH_OUTPUTWND,0,0);
}

void CSchedulerApp::CheckInputData()
{
	/*int index=0;
	for (BIZAPI::GetBizMap()->GetYKOperAvbResPtrMap()->Begin();BIZAPI::GetBizMap()->GetYKOperAvbResPtrMap()->NotEnd()
		;BIZAPI::GetBizMap()->GetYKOperAvbResPtrMap()->Step())
	{
		index++;
		if(BIZAPI::GetBizMap()->GetYKOperAvbResPtrMap()->Get()!=NULL)
			if(BIZAPI::GetBizMap()->GetYKOperAvbResPtrMap()->Get()->GetResourcePtrList().empty())
			{
				CDataError errd=CDataError(CDataError::READ_FILES,CDataError::ERROR_DATA_LOGIC,TblOperAvbRes,index,L"");
				g_deManager.Insert(errd);
			}
	}
	index=0;
	for (BIZAPI::GetBizMap()->GetYKOperationPtrMap()->Begin();BIZAPI::GetBizMap()->GetYKOperationPtrMap()->NotEnd()
		;BIZAPI::GetBizMap()->GetYKOperationPtrMap()->Step())
	{
		index++;
		if(BIZAPI::GetBizMap()->GetYKOperationPtrMap()->Get()!=NULL)
			if(BIZAPI::GetBizMap()->GetYKOperationPtrMap()->Get()->GetOperAvbPtrList().empty())
			{
				CDataError errd=CDataError(CDataError::READ_FILES,CDataError::ERROR_DATA_LOGIC,TblOperation,index,L"");
				g_deManager.Insert(errd);
			}
	}*/
}
void CSchedulerApp::OnUpdateAutoEnsureWork(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_

	if (HideTable(TblApsParam))
		return pCmdUI->Enable(FALSE);
	pCmdUI->Enable(m_bOpen);
}
void CSchedulerApp::OnCreatressequence()
{
	// TODO: Add your command handler code here
	YKBizMap* bMap=BIZAPI::GetBizMap();
	if(NULL == bMap) return;
	if(!bMap->GetIsCreatedResSequence())
	{
		BIZAPI::CreatResourceSequence();
		bMap->SetIsCreatedResSequence(TRUE);
	}
	//if (g_tblType==TblResourceSequence)
	//{
		//SendMessage(m_pMainWnd->GetSafeHwnd(),_ACTIVITE_VIWE_,123,0);
	//}
	BIZAPI::SetTableFlashFlg(TRUE,TblResourceSequence);
	OpenTblPage(TblResourceSequence);
}

void CSchedulerApp::OnCreatitemsequence()
{
	// TODO: Add your command handler code here
	YKBizMap* bMap=BIZAPI::GetBizMap();
	if(NULL == bMap) return;
	if(!bMap->GetIsCreatedItemSequence())
	{
		BIZAPI::CreatNewCategorySequence();
		bMap->SetIsCreatedItemSequence(TRUE);
	}
	//if (g_tblType==TblCategorySequence)
	//{
		//SendMessage(m_pMainWnd->GetSafeHwnd(),_ACTIVITE_VIWE_,123,0);
	//}
	BIZAPI::SetTableFlashFlg(TRUE,TblCategorySequence);
	OpenTblPage(TblCategorySequence);
}

void CSchedulerApp::OnResinfo()
{
	// TODO: Add your command handler code here
	BIZAPI::Estimate();
	//if (g_tblType==TblRestrictInfo)
	//{
		//SendMessage(m_pMainWnd->GetSafeHwnd(),_ACTIVITE_VIWE_,123,0);
	//}
	BIZAPI::SetTableFlashFlg(TRUE,TblRestrictInfo);
	OpenTblPage(TblRestrictInfo);
}

void CSchedulerApp::OnUpdateCreatressequence(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnUpdateCreatitemsequence(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnUpdateResinfo(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnCreatuseres()
{
	// TODO: Add your command handler code here
	YKBizMap* bMap=BIZAPI::GetBizMap();
	if(NULL == bMap) return;
	if(!bMap->GetIsCreatedUseSequence())
	{
		BIZAPI::CreateUseResource();
		bMap->SetIsCreatedUseSequence(TRUE);
	}
	SendMessage(m_pMainWnd->GetSafeHwnd(),_ACTIVITE_VIWE_,123,0);
}

void CSchedulerApp::OnCreatepro()
{
	// TODO: Add your command handler code here
	YKBizMap* bMap=BIZAPI::GetBizMap();
	if(NULL == bMap) return;
	if(!bMap->GetIsCreatedProIndia())
	{
		CPISettingDlg dlg;
		dlg.DoModal();

	}

}

void CSchedulerApp::OnUpdateCreatuseres(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnUpdateCreatepro(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_;
	if (HideTable(TblProduceIndication))
		return pCmdUI->Enable(FALSE);
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnCreatworkchange()
{
	// TODO: Add your command handler code here
	// 工作计划变更
	// 保存工作信息 改变的
	if(NULL == BIZAPI::GetBizMap()) return;
	time_t t=clock();
	BIZAPI::GetBizMap()->CalWorkChange();
	time_t tt=clock()-t;

	theApp.OpenTblPage(TblWorkChange);
}

void CSchedulerApp::OnUpdateCreatworkchange(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_;
	if (HideTable(TblWorkChange))
		return pCmdUI->Enable(FALSE);
	pCmdUI->Enable(m_bOpen&&m_couldCreatWorkChange);
}

void CSchedulerApp::OnUpdateTogethorRun( CCmdUI *pCmdUI )
{
	_LIMIT_CHECK_
	if(BIZAPI::EnterDataBaseCriSel())
	{
		if(IsGanttOpen()&& GanttCommonApi::HadSelectWork())
			pCmdUI->Enable(TRUE);
		else
			pCmdUI->Enable(FALSE);
		if( !BIZAPI::GetCommLimit(  TblWork )
			&&!BIZAPI::GetCommLimit(  TblOrder ))
			pCmdUI->Enable(FALSE);
		BIZAPI::LeaveDataBaseCriSel();
	}

	pCmdUI->Enable(FALSE);
}

void CSchedulerApp::OnUpdateTogethorSelect( CCmdUI *pCmdUI )
{
	_LIMIT_CHECK_
	if(IsGanttOpen())
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
	if( !BIZAPI::GetCommLimit(  TblResource )  )
		pCmdUI->Enable(FALSE);
}



void CSchedulerApp::OnTogethorSelect()
{
	CNewModelDlg  dlg;
	//dlg.SetShowRect(showRect);
	dlg.SetDivisionStr(L"/");
	dlg.SetMainDivisionStr(L",");
	wstring strCStr2(BIZAPI::GetBizMap()->GetTogethorCd());
	if (!strCStr2.empty())
	{
		vector<wstring> codeList;
		vector<wstring>::iterator iCode ;
		TOOLS::ParseStr(strCStr2,codeList,L",");
		strCStr2.clear();
		for(iCode = codeList.begin();iCode != codeList.end();iCode++)
		{
			dlg.CheckContiveFlg(*iCode);
		}
		strCStr2 = TOOLS::JoinStr(codeList,L",");

	}
	dlg.Constive();
	dlg.SetCodeUseType(TRUE,strCStr2);
	dlg.SetStoveResAvbExpType(3);
	dlg.TransInfo(TblResource,YKResource::Resource_StoveResAvbExp,0);
	if (dlg.DoModal() == IDOK)
	{
		wstring str = dlg.GetStoveResAvbExpStr();
		//BIZAPI::BeginRebackOper();
		BIZAPI::BeginUndoRedo();
		// 加入撤销信息
		RecordInfo reinfo;
		reinfo.id = 1;
		reinfo.operType = OPERTYPEMODIFY;
		BIZAPI::InsertRelDataStruct(TblSchSysParam,reinfo);

		BIZAPI::GetBizMap()->SetTogethorCd(str);

		//BIZAPI::EndRebackOper();
		BIZAPI::EndUndoRedo();
	}

}

void CSchedulerApp::RegVsFlexGrid()
{
	{
		HKEY	hKey = HKEY_CLASSES_ROOT;
		LPTSTR flexKey = L"Licenses\\A2C44195-8B4E-4f5d-80AF-875A8F9BE183";

		DWORD dw;

		char temp[] = {0x05,0x01,0xd6,0x00,0xd6,0x00,0x4c,0x01,0x85,0x00,0xa5,0x01,0xa2,0x00,0x16,0x01,0xe8,0x00,0x87,0x00,0xb8,0x00,0xc9,
			0x00,0x87,0x00,0x1a,0x01,0xcc,0x00,0x38,0x01,0x6f,0x00,0x51,0x01,0xed,0x00,0xbe,0x00,0x8c,0x00,0x8d,0x00,0xf4,0x00,0x45,0x00,
			0x6c,0x00,0x0a,0x01,0xd9,0x00,0x1a,0x01,0x66,0x00,0x45,0x01,0xfe,0x00,0x37,0x00,0x77,0x00,0x6a,0x01,0xd4,0x00,0xf1,0x00,0x5e,
			0x01,0x82,0x00,0x0e,0x01,0x7d,0x00,0x8e,0x00,0x83,0x00,0xa7,0x00,0xff,0x00,0x03,0x01,0x59,0x01,0x89,0x00,0x77,0x00,0xe4,0x00,
			0x7d,0x00,0xd2,0x00,0x6f,0x00,0xe2,0x00,0x41,0x01,0x9a,0x00,0x94,0x00,0x74,0x00,0x36,0x01,0x66,0x01,0x68,0x00,0x58,0x00};

		//构造
		LONG lRet = RegCreateKeyEx (HKEY_CLASSES_ROOT, flexKey, 0L, NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dw);

		if(lRet == ERROR_SUCCESS)
		{
			RegSetValueEx (hKey, _T("1"), 0L, REG_BINARY,
				(CONST BYTE*) &temp, sizeof(temp));
		}

		RegCloseKey (hKey);
	}

	{
		HKEY	hKey = HKEY_CLASSES_ROOT;
		//LPTSTR flexKey = L"Licenses\\a2c44196-8b4e-4f5d-80af-875a8f9be183";
		LPTSTR flexKey = L"Licenses\\BEA2ED97-562A-4510-974C-C4F20AE429AD";

		DWORD dw;

		//char temp[] = {0x9f,0x00,0x8a,0x01,0xc0,0x00,0xfb,0x00,0x7d,0x00,0xc1,0x00,0xd9,0x00,0x0c,0x00,0x6e,0x00,0x60,0x00,0x17,0x01,0x1b,
		//	0x01,0x71,0x00,0x41,0x00,0xdc,0x00,0xf2,0x00,0xfe,0x00,0x51,0x01,0x96,0x00,0xcb,0x00,0xd3,0x00,0x18,0x6d,0xc2,0x52,0x9c,0x8f,
		//	0x3b,0x4f,0x79,0x00,0x89,0x4e,0x5f,0x6e,0xbb,0x00,0x94,0x67,0xa7,0x96,0xff,0x51,0x1b,0x54,0xcd,0x00,0xb5,0x00,0xda,0x00,0x5a,
		//	0x00,0x8f,0x00,0xff,0x00,0xd2,0x00,0xf6,0x00,0x07,0x01,0x83,0x00,0x54,0x00,0xe5,0x00,0xe6,0x00,0xce,0x00,0xf0,0x00};
		char temp[] = {0xf2,0x00,0x9e,0x01,0x1c,0x01,0xa7,0x00,0x71,0x00,0x43,0x01,0x6e,0x00,0xc6,0x00,0x3b,0x00,0xad,0x00,0xe8,0x00,0x1e,
			0x00,0x78,0x00,0x08,0x01,0x2f,0x01,0x22,0x01,0xd9,0x00,0xa7,0x00,0x62,0x01,0xcd,0x00,0x88,0x00,0x7b,0x00,0x16,0x01,0x73,0x00,
			0x80,0x00,0x92,0x00,0x48,0x01,0x70,0x00,0x2d,0x01};

		//构造
		LONG lRet = RegCreateKeyEx (HKEY_CLASSES_ROOT, flexKey, 0L, NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dw);

		if(lRet == ERROR_SUCCESS)
		{
			RegSetValueEx (hKey, _T("1"), 0L, REG_BINARY,
				(CONST BYTE*) &temp, sizeof(temp));
		}

		RegCloseKey (hKey);
	}

}

void CSchedulerApp::OnWorksequence()
{
	// TODO: Add your command handler code here
	OpenTblPage(ID_WORK_RESOURCE_GD);
}

void CSchedulerApp::OnUpdateWorksequence(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_;
	if (HideTable(ID_WORK_RESOURCE_GD))
		return pCmdUI->Enable(FALSE);
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnUpdateOutputResRate(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	pCmdUI->Enable(m_bOpen&&IsGanttOpen(ID_RATE_RES_GL));
}

void CSchedulerApp::OnOutputResRate()
{
	// TODO: Add your command handler code here
	//OpenTblPage(ID_WORK_RESOURCE_GD);
	COutputSetDlg dlg;
	dlg.SetFormat(m_outputResRateFileFormat);
	dlg.SetPath(m_outputResRatePath);
	if (dlg.DoModal() == IDOK)
	{
		m_outputResRatePath = dlg.GetPath();
		const YK_WSTRING strPath = m_outputResRatePath.GetString();
		m_outputResRateFileFormat = dlg.GetFormat();

		wostringstream strContent;
		wchar_t* pSep = _T("\t");	if(m_outputResRateFileFormat != OutputFileFormat_Tab)	pSep = _T(",");

		strContent<<GetRemarkFiled(ID_UI_TEXT_RESOURCE).c_str()<<pSep
			<<GetRemarkFiled(ID_UI_TEXT_DATE).c_str()<<pSep
			<<GetRemarkFiled(ID_UI_ResRate_LoadUint).c_str()<<_T("\r\n");
		ResRateGanttApi::GetCurResRateInfo(strContent,m_outputResRateFileFormat);

		DATATABLAPI::WriteResRate(strPath,strContent,m_outputResRateFileFormat);

	}
}

//判断是否工作顺序表打开
void* CSchedulerApp::GetWorkSequence()
{
	if (g_tblType <= 0) 
	{		
		return NULL;
	}

	CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	if (NULL != pCurView && pCurView->m_pWorkSe.GetSafeHwnd())
	{
		return &(pCurView->m_pWorkSe);
	}
	return NULL;
}

void CSchedulerApp::OnUpdateMultiSch( CCmdUI *pCmdUI )
{
	_LIMIT_CHECK_
	if(m_bOpen && (BIZAPI::GetBizMap()->GetApsParam()->GetStopPatchCondition() == 2))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void CSchedulerApp::OnUpdateStopMultiSch( CCmdUI *pCmdUI )
{
	_LIMIT_CHECK_
	if(m_bOpen && (BIZAPI::GetBizMap()->GetApsParam()->GetStopPatchCondition() == 2))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void CSchedulerApp::OnUpdateQuickBuildMode(CCmdUI* pCmdUI)
{
	_LIMIT_CHECK_
	pCmdUI->Enable(!m_bOpen);
}
void CSchedulerApp::OnQuickBuildMode()
{
	//if (!m_bOpen)
	//{
	//	PROAPI_1::Show(m_pMainWnd);
	//	YK_WSTRING strFilePath = PROAPI_1::GetFilePath();
	//	if (!strFilePath.empty())
	//	{
	//		CString strPath(strFilePath.c_str());
	//		Open(strPath);
	//	}
	//}
}


BOOL CSchedulerApp::IsOutlineGrid()
{
	BOOL bOK = FALSE;
	CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	if (NULL != pCurView)
	{
		const UINT uTblType = pCurView->GetTblType();
		if (uTblType == TblOrder || uTblType == TblWork)
			bOK = TRUE;
	}

	return bOK;
}

// 导入导出  新建 打开  信息提示  add 2010-4-15
unsigned long CSchedulerApp::SetMsgInfo( YK_ULONG infoId ,CString& code,BOOL b,long msgType)
{
	//CMessageData* mesData=CMessageData::NewMsgData();
	CMessageData mesData; mesData.New();
	mesData.SetType((MSG_TYPE)msgType);
	if(ID_UI_MSG_TYPE_BLANK == infoId)
		mesData.SetLevel(MSG_LEVEL_BLANK);
	YK_WSTRING wstr=L":";
	if(!b) wstr=L"";
	wstring startCode;
	if(infoId == ID_UI_BAR_OBJCOUNT)
		startCode = MSG_BLANK;
	mesData.SetMessageCode(startCode+GetRemarkFiled(infoId)+wstr+code.GetString());
	// 时间
	mesData.SetTime(TOOLS::FormatTime(CDealTime::GetLocalTime(),true));
	return mesData.GetId();
}

BOOL CAboutDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	if (pDC->GetSafeHdc()&&GetSafeHwnd())
	{

		CRect rcClient;
		GetClientRect(rcClient);

		CDC mDC;
		mDC.CreateCompatibleDC(pDC);
		int bmpId = IDB_BITMAP_FIND2;
		CBitmap bkBmp;
		if ( !bkBmp.LoadBitmap(bmpId) )
			return CDialog::OnEraseBkgnd(pDC);
		mDC.SelectObject(&bkBmp);

		pDC->BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &mDC, 0, 0, SRCCOPY);
	}
	return TRUE;
}

int CSchedulerApp::ProctDogMsg( int type )
{
#ifdef NOTTRYOUT
	//add by yp 2011.06.10
	//去掉试用版功能
	//MessageBox(m_pMainWnd->GetSafeHwnd(),_T("永凯的加密狗已经过期或没有连接！"),GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str(),MB_OK);
	//return 2;
#endif

	if (type == 0)
		return MessageBox(m_pMainWnd->GetSafeHwnd(),GetRemarkFiled(ID_UI_MSG_LIMIT_OVERDUR).c_str(),GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str()/*_T("永凯的加密狗已经过期，是否继续运行体验版？\n体验版有数据量的限制，仅供体验、评估使用。")*/
		,MB_ICONQUESTION | MB_OK);
	else 
		return  MessageBox(m_pMainWnd->GetSafeHwnd(),GetRemarkFiled(ID_UI_MSG_LIMIT_UNLINK).c_str(),GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str()/*_T("永凯的加密狗没有连接，是否继续运行体验版？\n体验版有数据量的限制，仅供体验、评估使用。")*/
		,MB_ICONQUESTION | MB_OK);


	////////////////////////////////////////////////////////////////////////////
	//if (type == 1)
	//	return MessageBox(m_pMainWnd->GetSafeHwnd(),GetRemarkFiled(ID_UI_MSG_LIMIT_OVERDUR).c_str(),GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str()/*_T("永凯的加密狗已经过期，是否继续运行体验版？\n体验版有数据量的限制，仅供体验、评估使用。")*/
	//	,MB_ICONQUESTION | MB_YESNO);
	//else if (type == 0)
	//	return  MessageBox(m_pMainWnd->GetSafeHwnd(),GetRemarkFiled(ID_UI_MSG_LIMIT_UNLINK).c_str(),GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str()/*_T("永凯的加密狗没有连接，是否继续运行体验版？\n体验版有数据量的限制，仅供体验、评估使用。")*/
	//	,MB_ICONQUESTION | MB_YESNO);
	//else if (type == 2)
	//{
	//	CLimitOutOfTimeDlg dlg;
	//	dlg.SetLanguageType(theApp.m_langueType);
	//	dlg.DoModal();
	//	return IDYES;
	//}

	return 2;
}


void CSchedulerApp::SetMainFrmText()
{
	CString str;
	if (m_pMainWnd != NULL)
	{
		CMainFrame* pMainFrm = (CMainFrame*)m_pMainWnd;
//#ifdef LIMITFUNCTION
//		if (BIZAPI::GetBizMap()->IsLimitEdition())
//			str = GetRemarkFiled(ID_UI_MAINFRM_TITLE_LIMIT).c_str();
//		else
//#endif
			str = GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str();
		pMainFrm->SetTitle(str);
	}
	
}

time_t CSchedulerApp::GetInstalTime()
{
	time_t insTm(0);

	HKEY	hKey = HKEY_LOCAL_MACHINE;
	LPTSTR insTmKey = L"SOFTWARE\\YUKON\\APS";

	RegOpenKeyEx (hKey, insTmKey, 0L, KEY_ALL_ACCESS, &hKey);

	LPTSTR insTmvalues = L"StartDate";

	UINT  nBytes = 0; 
	DWORD dwType = 0; 
	DWORD dwCount = 0;

	LONG lResult = ::RegQueryValueEx (hKey, insTmvalues, NULL, &dwType, 
		NULL, &dwCount); 

	if (lResult == ERROR_SUCCESS && dwCount > 0)
	{ 
		nBytes = dwCount; 
		ASSERT (dwType == REG_SZ || dwType == REG_EXPAND_SZ);

		BYTE* pData = new BYTE [nBytes + 1];

		lResult = ::RegQueryValueEx (hKey, insTmvalues, NULL, &dwType, 
			pData, &dwCount); 

		if (lResult == ERROR_SUCCESS &&  dwCount > 0) 
		{ 
			ASSERT (dwType == REG_SZ || dwType == REG_EXPAND_SZ);
			insTm = TOOLS::ParseTime((TCHAR*)pData).GetTime();
		} 

		delete [] pData;
		pData = NULL; 
	} 

	RegCloseKey (hKey);

	return insTm;
}

LRESULT CSchedulerApp::ChangeTitle()
{
	CString title = GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str();;
//	title.LoadString(AFX_IDS_APP_TITLE);
	delete (TCHAR *)m_pszAppName;
	m_pszAppName=new TCHAR[title.GetLength()+1];
	memcpy((TCHAR *)m_pszAppName, title.GetBuffer(0), sizeof(TCHAR)*(title.GetLength()+1));
	return 0;

}

void CSchedulerApp::OpenLastTable()
{

	UINT curOpenTbl = 0;
	for(UINT i = ID_GANTT_RES_GL;i<=ID_BOM_VIEW_DLG;i++)
	{
		if(CWindowsState::IsOpen(i))
		{
			if(CWindowsState::IsCurOpen(i))
			{
				curOpenTbl = i;
			}
			else
			{
				OpenTblPage(i);      //打开表格
			}
		}
	}
      
	for(UINT i = TblClient;i<=TblManufactureCapitalInfor;i++)
	{
		if(CWindowsState::IsOpen(i))
		{
			if(CWindowsState::IsCurOpen(i))
			{
				curOpenTbl = i;
			}
			else
			{
				OpenTblPage(i);      //打开表格
			}
		}
	}

	if(curOpenTbl != 0)
		OpenTblPage(curOpenTbl);  
}

//void CSchedulerApp::ShowGridLabelState()
//{
//	CMDIChildWnd* child = static_cast<CMainFrame*>(m_pMainWnd)->MDIGetActive();
//	if (child != NULL)
//	{
//		CYKSchedulerView* view = (CYKSchedulerView*)child->GetActiveView();
//		if (view != NULL)
//		{
//			if (view->m_bIsTalbe)
//			{
//				view->AdjustGridView();
//			}
//		}
//	}
//}

void CSchedulerApp::OnFrameMenuGridshowtip()
{
	// TODO: Add your command handler code here
	// 表格视图
	m_gridShowLabel = !m_gridShowLabel;

	CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	if (NULL != pCurView)
		pCurView->AdjustGridView();
}

void CSchedulerApp::OnUpdateFrameMenuGridshowtip(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	if (m_gridShowLabel)
	{
		pCmdUI->SetCheck(1);
	}
	else pCmdUI->SetCheck(0);
	
	if (g_tblType <= 0 && !m_bOpen) 
	{
		pCmdUI->Enable(FALSE);
		return ;
	}
	// 表格视图
	CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	if (NULL != pCurView && pCurView->m_bIsTalbe)
	{
		pCmdUI->Enable(TRUE);
		return;
	}

	pCmdUI->Enable(FALSE);
}

bool CSchedulerApp::GanttMourseWheel( int nCode,WPARAM wParam,LPARAM lParam )
{
	//CMDIChildWnd* child = static_cast<CMainFrame*>(m_pMainWnd)->MDIGetActive();
	//if (child != NULL)
	//{
	//	CYKSchedulerView* view = (CYKSchedulerView*)child->GetActiveView();
	//	if (view != NULL)
	//	{
	//		if (view->m_tblType == ID_GANTT_RES_GL
	//			|| view->m_tblType == ID_RATE_RES_GL
	//			||view->m_tblType == ID_RATE_STA_GL
	//			||view->m_tblType == ID_GANTT_ORDER_GL
	//			||view->m_tblType == ID_Item_Gather_GL)
	//		{
	//			//GanttCommonApi::OnMouseWheel(view->m_tblType,)
	//		}
	//	}
	//}
	return true;
}


void CSchedulerApp::OnInputsys()
{
	// TODO: Add your command handler code here
	TCHAR szFilters[]= _T("YukonSys data (*.yks)|*.yks|All Files (*.*)|*.*||");
	CFileDialog  openFile(TRUE,_T("yks"),_T("Data.yks"),OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters);
	if ( IDOK == openFile.DoModal() )
	{
		BIZAPI::ClearReback();

		CString strFile = openFile.GetPathName();
		wstring path = strFile.GetString();
		wstring version;
		DATATABLAPI::ReadSys(path,version);
		YKBizMap* pBizMap = BIZAPI::GetBizMap();
		if ( pBizMap != NULL )
			YKExtraValue::SetTimeUnit(pBizMap->GetSchSysParam()->GetSystemTimeUnit());  //设置界面的时间类型

		::SendMessage(theApp.g_hMainFrm, IDU_INIT_APS_COMBOBOX,0,0);
		::SendMessage(theApp.g_hMainFrm, ID_MESSAGE_FLASH_OUTPUTWND,0,0);

		CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
		if (NULL != pCurView)
		{
			pCurView->Flash();
		}
	}
}

void CSchedulerApp::OnUpdateInputsys(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	pCmdUI->Enable(theApp.m_bOpen);
}

void CSchedulerApp::OnOutputsys()
{
	// TODO: Add your command handler code here
	TCHAR szFilters[]= _T("YukonSys data (*.yks)|*.yks|All Files (*.*)|*.*||");
	CFileDialog  openFile(FALSE,_T("yks"),_T("Data.yks"),OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters);
	if ( IDOK == openFile.DoModal() )
	{
		CString strFile = openFile.GetPathName();
		wstring path = strFile.GetString();
		wstring version = m_strVersion.GetString();
		DATATABLAPI::WriteSys(path,version);
	}
}

void CSchedulerApp::OnUpdateOutputsys(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	_LIMIT_CHECK_
	pCmdUI->Enable(theApp.m_bOpen);
}

void CSchedulerApp::SetSchudlingFlg( bool flg )
{
	m_bSchduling = flg;
}

bool CSchedulerApp::GetSchudlingFlg()
{
	return m_bSchduling;
}

void CSchedulerApp::SetCanSchedStep( bool flg )
{
	m_bCanSchedStep = flg;
}

bool CSchedulerApp::GetCanSchedStep()
{
	return m_bCanSchedStep;
}

void CSchedulerApp::OnUpdateYkItemGather_GL( CCmdUI *pCmdUI )
{
	_LIMIT_CHECK_;
	if (HideTable(ID_Item_Gather_GL))
		return pCmdUI->Enable(FALSE);
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnUpdateYkItemGather_Grid( CCmdUI *pCmdUI )
{
	_LIMIT_CHECK_;
	if (HideTable(ID_Item_Gather_GL))
		return pCmdUI->Enable(FALSE);
	pCmdUI->Enable(m_bOpen);
}

//unsigned int WINAPI Run_DataDownloadProBar( void* lParam)
//{
//	CProcessBarThread* pPrBarThead = (CProcessBarThread*)lParam;
//	g_nProValue = 0;
//	pPrBarThead->SetHeadTitle(1);
//	pPrBarThead->InitProcessBar();
//	pPrBarThead->ShowProcessBarInfor(g_nProValue);
//
//	return 1;
//}

//unsigned int WINAPI Run_SubmitProBar( void* lParam)
//{
//	CProcessBarThread* pPrBarThead = (CProcessBarThread*)lParam;
//	g_nProValue = 0;
//	pPrBarThead->SetHeadTitle(2);
//	pPrBarThead->InitProcessBar();
//	pPrBarThead->ShowProcessBarInfor(g_nProValue);
//
//	return 1;
//}

void CSchedulerApp::OnDownload()
{
#ifdef OLDNETCONNET
	map<unsigned long,bool> renewTable;
	GetSelectInfo(renewTable);

	CSelectTableDlg dlg;
	dlg.SetRenewTable(renewTable);
	dlg.SetText(GetRemarkFiled(ID_UI_DOWNLOAD_SELECTTAB).c_str());
	if(dlg.DoModal() == IDOK)
	{
		renewTable = dlg.GetRenewTable();
		DATATABLAPI::SetSelectTable(renewTable);
		SeveSelectInfo(renewTable);
	}
	else
		return;
#endif
	
	if ( AfxMessageBox( GetRemarkFiled(ID_PROMT_UPLOAD).c_str() ,MB_YESNO|MB_ICONQUESTION) == IDNO )
		return;
	
	CProcessBarThread* pPrBarThead = (CProcessBarThread*)AfxBeginThread(RUNTIME_CLASS(CProcessBarThread),THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
	pPrBarThead->SetHeadTitle(1);
	pPrBarThead->InitProcessBar(0, 54);
	pPrBarThead->DisableCycleShow();
	pPrBarThead->ResumeThread();

	BIZAPI::InitFiledMap();
	GanttApi::OnClearAllWorkSelect();
	bool bRt = DATATABLAPI::DownLoad();

	pPrBarThead->InitProcessBar(-1, 31);
	if (bRt)
	{
		//目前暂时不用初始化算法 del by yp 2012.06.15
		//BIZAPI::TransFormAlg(0);			//转化数学模型
		MessageBox(m_pMainWnd->GetSafeHwnd(),GetRemarkFiled(DownLoad_Succeed).c_str(),GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str(),MB_OK);

		BIZAPI::SetTableFlashByType(TblFlashType_FlashAll);
		BIZAPI::GetBizMap()->RedrawAllGantt();
		FlashActiveView();
		::PostMessage(g_hMainFrm,IDU_INIT_APS_COMBOBOX,0,0);
	}
	else
	{
		CString strMessage(GetRemarkFiled(DownLoad_Fail).c_str());
		strMessage += _T("\n");
		YK_WSTRING strLastError = DATATABLAPI::GetNetWorkLastError();
		strMessage += strLastError.c_str();
		AfxMessageBox(strMessage);
	}
}

void CSchedulerApp::OnUpdateDownload(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_;
	pCmdUI->Enable(IsOnConnectNet());
}

void CSchedulerApp::OnSubmit()
{
#ifdef OLDNETCONNET
	map<unsigned long,bool> renewTable;
	GetSelectInfo(renewTable);
	CSelectTableDlg dlg;
	dlg.SetRenewTable(renewTable);
	dlg.SetText(GetRemarkFiled(ID_UI_UPLOAD_SELECTTAB).c_str());
	if(dlg.DoModal() == IDOK)
	{
		renewTable = dlg.GetRenewTable();
		DATATABLAPI::SetSelectTable(renewTable);
		SeveSelectInfo(renewTable);
	}
	else
		return;
#endif

	if ( AfxMessageBox( GetRemarkFiled(ID_PROMT_YKSUBMIT).c_str(),MB_YESNO|MB_ICONQUESTION) == IDNO )
		return;

	CProcessBarThread* pPrBarThead = (CProcessBarThread*)AfxBeginThread(RUNTIME_CLASS(CProcessBarThread),THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
	pPrBarThead->SetHeadTitle(2);
	pPrBarThead->InitProcessBar(0, 31);
	pPrBarThead->DisableCycleShow();
	pPrBarThead->ResumeThread();

	GanttApi::OnClearAllWorkSelect();
	BIZAPI::ClearCustomTblMode();			//清除 TblCustorRule 
	BIZAPI::ClearTblMode();					//清除 TblTableState 
	bool bRt = DATATABLAPI::Submit();
	pPrBarThead->InitProcessBar(-1, 31);
	if (bRt)
	{
		SaveSystemSet();
		BIZAPI::SetTableFlashByType(TblFlashType_FlashAll);
		BIZAPI::GetBizMap()->RedrawAllGantt();
		FlashActiveView();
		MessageBox(m_pMainWnd->GetSafeHwnd(),GetRemarkFiled(UpData_Succeed).c_str(),GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str(),MB_OK);
	}
	else
	{
		CString strMessage(GetRemarkFiled(UpData_Fail).c_str());
		strMessage += _T("\n");
		YK_WSTRING strLastError = DATATABLAPI::GetNetWorkLastError();
		strMessage += strLastError.c_str();
		AfxMessageBox(strMessage);
	}

}

void CSchedulerApp::OnUpdateSubmit(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_;
	pCmdUI->Enable(IsOnConnectNet());
}

void CSchedulerApp::OnLoading()
{
	CApsLoggin loggin;
	if( loggin.DoModal() == IDOK )
	{
		ConnectedNet();
		m_bOpen = TRUE;
		//登录功能不下载数据
//		OnDownload();
		::PostMessage(g_hMainFrm, ID_MESSAGE_INITTREE,0,0);
	}
}

bool CSchedulerApp::IsOnConnectNet()
{
	return theApp.m_bOpen &&m_bNetLoaded ;
}

void CSchedulerApp::ConnectedNet()
{
	m_bNetLoaded = true;
}

void CSchedulerApp::OffConnectedNet()
{
	m_bNetLoaded = false;
//	::SendMessage(theApp.g_hMainFrm,ID_STATUSBAR_TEXT,0,0);

}

void CSchedulerApp::OnUpdateLOADING( CCmdUI *pCmdUI )
{
	pCmdUI->Enable(TRUE);
	//pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::OnUpdateConflictSolve( CCmdUI *pCmdUI )
{
	pCmdUI->Enable(m_bOpen);
}

int CSchedulerApp::GetCurViewType()
{
	int nTblType = 0;
	CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	if (NULL != pCurView)
		nTblType = pCurView->GetTblType();
	
	return nTblType;
}

CVsflexgridn1* CSchedulerApp::GetCurDataGridCtrl()
{
	CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	if (NULL != pCurView && pCurView->m_bIsTalbe)
	{
		if (pCurView->m_pGdCtrl.GetSafeHwnd())
			return &pCurView->m_pGdCtrl;
		else if (pCurView->m_BOMInfo.GetSafeHwnd())
			return &pCurView->m_BOMInfo;
	}
	return NULL;
}

void CSchedulerApp::FlashActiveView()
{
	CYKSchedulerView* pCurView = (DYNAMIC_DOWNCAST(CMainFrame, m_pMainWnd))->GetActiveView();
	if (NULL != pCurView)
	{
		pCurView->Flash();
		::SendMessage(theApp.g_hMainFrm, ID_MESSAGE_FLASH_OUTPUTWND, 0, 0);
		::SendMessage(theApp.g_hMainFrm, IDU_UPDATE_WACTH_BAR, 0, 0);
		::SendMessage(theApp.g_hMainFrm, IDU_INIT_APS_COMBOBOX, 0, 0);
		::SendMessage(theApp.g_hMainFrm, ID_INIT_RESOURCE, 0, 0);
	}
}

bool CSchedulerApp::ClearAllData()
{
	CWindowsState::Clear();
	GanttCommonApi::Reset();
	BIZAPI::BizReset();

	m_outputResRatePath.Empty();
	m_outputResRateFileFormat = OutputFileFormat_Coma;

	YKBizMap* pBizMap = BIZAPI::GetBizMap();
	assert(pBizMap != NULL);
	pBizMap->ReSetObjCount();		// 重置对象数据

	DelSortConditionManager();

	if (g_hMainFrm != NULL)
		::SendMessage(g_hMainFrm,_CLOSE_ALL_VIEW,10,11);

	return true;
}

bool CSchedulerApp::HideTable( UINT tblType )
{
	switch (tblType)
	{
	case TblSettingTm:
	case TblTransportTm://{return Refined_Sch != BIZAPI::GetBizMap()->GetSchSysParam()->GetAreaSchPara();}
		break;
	case TblQualityLevel:
	case TblWork:
	case TblWorkChange:
	case TblWorkInputItem:
	case TblWorkOutputItem:
	case TblWorkAvbResource:
	case TblWorkRelation:
	case TblUseResource:
	case TblResourceSequence:
	case TblApsParam:
	case TblWatchBoard:
//	case TblCreateMTOrderParam:
	case TblCategorySequence:
	case  ID_GANTT_RES_GL      :
	case  ID_GANTT_ORDER_GL    :
	case  ID_RATE_RES_GL       :
	case  ID_RATE_STA_GL       :
	case  ID_WORK_RESOURCE_GD  :
	case  ID_Item_Gather_GL  : 
	case ID_DayShift_GD:
	case TblProduceIndication:{return By_Long_Sch == BIZAPI::GetBizMap()->GetSchSysParam()->GetAreaSchPara();}
		break;
	case TblManufactureCapitalInfor:{return By_Long_Sch != BIZAPI::GetBizMap()->GetSchSysParam()->GetAreaSchPara();}
		break;
	}
	return false;
}

void CSchedulerApp::OnUpdateYkDayShift_Grid( CCmdUI *pCmdUI )
{
	_LIMIT_CHECK_;
	if (HideTable(ID_DayShift_GD))
		return pCmdUI->Enable(FALSE);
	pCmdUI->Enable(m_bOpen);
}

void CSchedulerApp::InitIconForGrid()
{
	if (m_iList.Create(IDB_BITMAP_GREEN, 16, 0, COLOR_Image1/*RGB (192,192, 192)*/))
	{
		CPictureHolder picCell;
		if (m_iList.GetSafeHandle())
			picCell.CreateFromIcon(m_iList.ExtractIcon(0));

		V_VT(&vCellPic) = VT_DISPATCH;
		IPictureDisp * pDisp = picCell.GetPictureDispatch();
		V_DISPATCH(&vCellPic) = pDisp;
		//pDisp->Release();

		CPictureHolder picCell2;
		if (m_iList.GetSafeHandle())
			picCell2.CreateFromIcon(m_iList.ExtractIcon(1));

		V_VT(&vCellPic2) = VT_DISPATCH;
		V_DISPATCH(&vCellPic2) = picCell2.GetPictureDispatch();
	}
}

CSchedulerApp::~CSchedulerApp()
{

}

void CSchedulerApp::OnSenddemand()
{
	BIZAPI::Work2Demand();
	BIZAPI::SetTableFlashFlg(TRUE,TblManufactureDemand);
	FlashActiveView();
	//if (DATATABLAPI::SendDemand())
	//{
	//	SaveSystemSet();
	//	//AfxMessageBox(GetRemarkFiled(SendManuDemand_Succeed).c_str());
	//	MessageBox(m_pMainWnd->GetSafeHwnd(),GetRemarkFiled(SendManuDemand_Succeed).c_str(),GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str(),MB_OK);
	//	BIZAPI::SetTableFlashFlg(TRUE,TblManufactureDemand);
	//	FlashActiveView();
	//}
	//else
	//{
	//	AfxMessageBox(GetRemarkFiled(SendManuDemand_Fail).c_str());
	//}
}

void CSchedulerApp::OnUpdateSenddemand(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsOnConnectNet());
}

void CSchedulerApp::OnGetdemand()
{
	BIZAPI::Demand2Order();
	BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
	BIZAPI::SetTableFlashFlg(TRUE,TblManufactureDemand);
	FlashActiveView();

	//if (DATATABLAPI::GetDemand())
	//{		
	//	//AfxMessageBox(GetRemarkFiled(GetManuDemand_Succeed).c_str());
	//	MessageBox(m_pMainWnd->GetSafeHwnd(),GetRemarkFiled(GetManuDemand_Succeed).c_str(),GetRemarkFiled(ID_UI_MAINFRM_TITLE).c_str(),MB_OK);
	//	BIZAPI::Demand2Order();
	//	BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
	//	BIZAPI::SetTableFlashFlg(TRUE,TblManufactureDemand);
	//	FlashActiveView();
	//}
	//else
	//{
	//	AfxMessageBox(GetRemarkFiled(GetManuDemand_Fail).c_str());

	//}
}

void CSchedulerApp::OnUpdateGetdemand(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsOnConnectNet());
}


CString CSchedulerApp::GetStatusBarText(LPCTSTR lpszNewText)
{
	if(lpszNewText != NULL)
	{
		CString strStatusBar;
		strStatusBar.LoadString(AFX_IDS_IDLEMESSAGE);
		if(strStatusBar.Compare(lpszNewText) == 0)	//相同
		{
			if(IsOnConnectNet())
			{
				CString cstrt = GetRemarkFiled(ID_UI_Login_User).c_str(); 
				CString depart = BIZAPI::GetCurDepart().c_str();
				if(depart.IsEmpty())
					m_statusBarText = (cstrt+_T("(")+m_loginName+_T(")")); 
				else
					m_statusBarText = (cstrt+_T("(")+depart+_T(":")+m_loginName+_T(")")); 

				return m_statusBarText;
			}
		}
	}
	return lpszNewText;
}
void CSchedulerApp::OnFeedback()
{

	//YK_WSTRING excess;                            //用于接收上次设定的时间值
 //   GetTimeExcessStr(excess);                     //调用函数接收上次设定的时间值
	//CTimeExcessDlg dlg;
	//dlg.SetTimeExcess(excess);

	//if(dlg.DoModal() == IDOK)
	//{
	//	excess = dlg.GetTimeExcess();

		BIZAPI::FeedBackDemand(0);     //调用函数反馈计划

//		BIZAPI::FeedBackDemand(excessTm);          //调用函数反馈计划

		BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
		BIZAPI::SetTableFlashFlg(TRUE,TblManufactureDemand);
		FlashActiveView();  

		//SaveTimeExcessStr(excess);                //调用函数保存设定的时间值
}

void CSchedulerApp::OnUpdateFeedback(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsOnConnectNet());
}

void CSchedulerApp::OnAccetpFeedback()
{
	BIZAPI::AcceptFeedBackDemand();
	BIZAPI::SetTableFlashFlg(TRUE,TblOrder);
	BIZAPI::SetTableFlashFlg(TRUE,TblManufactureDemand);
	FlashActiveView();

}

void CSchedulerApp::OnUpdateAccetpFeedback(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsOnConnectNet());
}

BOOL CSchedulerApp::SeveSelectInfo(map<unsigned long,bool>& renewTable)
{

	CBCGPRegistry reg(FALSE,FALSE);
	CString strKey(_T("YKSystemSet\\SelectTable"));

	if (reg.CreateKey(strKey))
	{
		for(map<unsigned long,bool>::iterator iter = renewTable.begin();
			iter != renewTable.end();iter++)
		{
			CString tblKey,tblValue;
			tblKey.Format(_T("%d"),iter->first);
			tblValue.Format(_T("%d"),iter->second);
			reg.Write(tblKey,tblValue);
		}
		return TRUE;
	}


	return TRUE;
}

BOOL CSchedulerApp::GetSelectInfo( map<unsigned long,bool>& renewTable )
{
	CBCGPRegistry reg(FALSE,FALSE);
	CString strKey(_T("YKSystemSet\\SelectTable"));
	if (reg.CreateKey(strKey))
	{
		CString tblKey,tblValue;

		for(unsigned long tbl = 1;tbl<101;tbl++)
		{
			tblKey.Format(_T("%d"),tbl);
			if (reg.Read(tblKey,tblValue))
			{
				//renewTable[tbl] = _wtoi(tblValue.AllocSysString());
				//Add 2012-04-08
				renewTable[tbl] = _wtoi(tblValue.GetString());
			}
			else
			{
				renewTable[tbl] = true;
			}
		}
	}

	return TRUE;
}

BOOL CSchedulerApp::SaveTimeExcessStr(YK_WSTRING & val)
{

	CBCGPRegistry reg(FALSE,FALSE);
	CString strKey(_T("YKSystemSet\\TimeExcessTable"));

	if (reg.CreateKey(strKey))
	{
		
		CString tblKey(_T("excess")),tblValue;
		//YK_WSTRING vari = "excess";
		tblValue = val.c_str();
		//tblKey.Format(_T("%d"),vari);
		//tblValue.Format(_T("%d"),val);
		reg.Write(tblKey,tblValue);
		
		return TRUE;
	}

	return TRUE;
}

BOOL CSchedulerApp::GetTimeExcessStr( YK_WSTRING & val )
{
	CBCGPRegistry reg(FALSE,FALSE);
	CString strKey(_T("YKSystemSet\\TimeExcessTable"));
	if (reg.CreateKey(strKey))
	{
		CString tblKey(_T("excess")),tblValue;
	
		if (reg.Read(tblKey,tblValue))
		{
			val = tblValue.GetString();
		}
			
	}

	return TRUE;
}


void CSchedulerApp::OnHelpRegister()
{
	// TODO: Add your command handler code here
	CDlgRegister dlgReg;
	dlgReg.CheckRegisterInfo();
	dlgReg.DoModal();
}

BOOL CSchedulerApp::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (CSplashScreenEx::PreTranslateAppMessage(pMsg))
		return TRUE;

	return __super::PreTranslateMessage(pMsg);
}

void CSchedulerApp::OnUpdateFilePrintSetup(CCmdUI *pCmdUI)
{
	_LIMIT_CHECK_

	pCmdUI->Enable(TRUE);
}

//Add 2012-03-14 判断是否显示提示信息
YK_SHORT CSchedulerApp::IsShowTopInfor()
{
	HKEY hKey;
	LONG lRet;
	YK_SHORT nRen;

	lRet = ::RegOpenKeyEx(HKEY_CURRENT_USER, topInforStruct.strPath, 0, KEY_ALL_ACCESS, &hKey);
	if (ERROR_SUCCESS == lRet)
	{
		DWORD dwType = 0;
		DWORD dwCount = 0;

		lRet = ::RegQueryValueEx(hKey, topInforStruct.strTopInfor, NULL, &dwType, NULL, &dwCount);
		if (ERROR_SUCCESS == lRet)	// 已创建注册信息
		{
			::RegQueryValueEx(hKey, topInforStruct.strTopInfor, NULL, &dwType, (BYTE*)&nRen, &dwCount);
			
		}else
		{
			return 1;
		}

		::RegCloseKey(hKey);
	}

	return  nRen;
}