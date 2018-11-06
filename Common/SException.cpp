#include "SException.h"
#include <Shlwapi.h>
#include "XZip.h"
#include <ShlObj.h>

#ifdef _MSC_VER
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Shell32.lib")
#endif


VOID ThrowStringException(LPCTSTR szFmt, ...)
{
	va_list argList;
	va_start(argList, szFmt);

	UINT nSize = _vsctprintf(szFmt, argList);

	std::string strMessage;
	strMessage.assign(nSize + 1, _T(' '));
	_vstprintf((LPTSTR)&(strMessage[0]), szFmt, argList);
	va_end(argList);

	throw std::exception(strMessage.c_str());
}

VOID ThrowAssertException(LPCTSTR szFileName, INT nFileLine, LPCTSTR pExpr)
{
	const char *szAssertFmt =
		_T("Assert	\r\n")
		_T("\r\n")
		_T("File:			%s\r\n")
		_T("Line:			%d\r\n")
		_T("Expr:			%s\r\n");

	UINT nSize = _sctprintf(szAssertFmt, szFileName, nFileLine, pExpr);

	std::string strMessage;
	strMessage.assign(nSize + 1, _T(' '));
	_stprintf((LPTSTR)&(strMessage[0]), szAssertFmt, szFileName, nFileLine, pExpr);

	throw std::exception(strMessage.c_str());
}

static bool IsWinDbgEnable(char* sWinDbg)
{
	// 寻找 WinDbg
	DWORD dwType = 0, dwSize = MAX_PATH;
	if (ERROR_SUCCESS != ::SHGetValue(HKEY_CURRENT_USER, "Software\\Microsoft\\DebuggingTools", "WinDbg", &dwType, sWinDbg, &dwSize))
		return false;

	return true;
}

// PEXCEPTION_POINTERS pException = ::GetExceptionInformation()
void SendInnerCrashReport(PEXCEPTION_POINTERS pException, HWND hInnerWnd)
{
	if (!pException)
		return;

	////////////////////////////////////////////
	CHAR szCurrentDir[MAX_PATH] = { 0 };
	::GetCurrentDirectory(MAX_PATH, szCurrentDir);

	CHAR szCrashFile[MAX_PATH] = { 0 };
	SYSTEMTIME timeNow;
	::GetLocalTime(&timeNow);
	_snprintf(szCrashFile, MAX_PATH, "crash%02d_%02d_%02d_%02d_%02d.zip",
		timeNow.wMonth, timeNow.wDay, timeNow.wHour, timeNow.wMinute, timeNow.wSecond);

	//生成dump报告
	TCHAR szDumpFile[MAX_PATH] = { 0 };
	if (!CreateDumpHelpFile(pException, szDumpFile))
		return;

	//////////////////////////////////////////////
	CHAR szLocalFile[MAX_PATH] = { 0 };
	strncpy(szLocalFile, szCurrentDir, MAX_PATH);
	::PathAppend(szLocalFile, szCrashFile);
	HZIP hz = CreateZip((void*)szLocalFile, 0, ZIP_FILENAME);
	if (!hz)
	{
		::MessageBox(hInnerWnd, "Create Crash.zip error!", "Crash Report", MB_OK | MB_ICONSTOP);
		return;
	}

	ZipAdd(hz, "dump.dump", (void*)szDumpFile, 0, ZIP_FILENAME);

	CHAR szLogFile[MAX_PATH] = { 0 };

	strncpy(szLogFile, szCurrentDir, MAX_PATH);
	PathAppend(szLogFile, "Stranded.log");
	ZipAdd(hz, "StrandedRender.log", (void*)szLogFile, 0, ZIP_FILENAME);

	CloseZip(hz);

	//////////////////////////////////////////////////////////////////////////
	BROWSEINFO binfo;
	::ZeroMemory(&binfo, sizeof(BROWSEINFO));
	binfo.hwndOwner = hInnerWnd;
	binfo.pidlRoot = NULL;
	binfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;
	binfo.pszDisplayName = NULL;
	binfo.lpszTitle = "选择一个目录用以保存数据文件";

	LPITEMIDLIST pRetFold = ::SHBrowseForFolder(&binfo);
	if (!pRetFold)
		return;

	CHAR szReturn[MAX_PATH] = { 0 };
	::SHGetPathFromIDList(pRetFold, szReturn);

	//////////////////////////////////////////////////////////////////////////
	CHAR szSaveAs[MAX_PATH] = { 0 };
	strncpy(szSaveAs, szReturn, MAX_PATH);
	::PathAppend(szSaveAs, szCrashFile);
	::CopyFile(szLocalFile, szSaveAs, FALSE);
}
