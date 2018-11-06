#include "SCrashReport.h"
#include <DbgHelp.h>
#include <TlHelp32.h>
#include <time.h>
#include <Shlwapi.h>
#include "Global.h"

#ifdef _MSC_VER
// PathAppend
#pragma comment(lib, "Shlwapi.lib")
#endif

// 如果系统头文件dbghelp.h不存在，则使用crash.h
#ifndef _DBGHELP_

typedef struct _MINIDUMP_EXCEPTION_INFORMATION {
	DWORD					ThreadId;
	PEXCEPTION_POINTERS		ExceptionPointers;
	BOOL					ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION, *PMINIDUMP_EXCEPTION_INFORMATION;

typedef enum _MINIDUMP_TYPE {
	MiniDumpNormal = 0x00000000,
	MiniDumpWithDataSegs = 0x00000001,
} MINIDUMP_TYPE;

typedef BOOL(WINAPI *MINIDUMP_WRITE_DUMP)(
	IN HANDLE									hProcess,
	IN DWORD									ProcessId,
	IN HANDLE									hFile,
	IN MINIDUMP_TYPE							DumpType,
	IN CONST PMINIDUMP_EXCEPTION_INFORMATION	ExceptionParam OPTIONAL,
	IN PVOID									UserStreamParam OPTIONAL,
	IN PVOID									CallbackParam OPTIONAL
	);

#else

typedef BOOL(WINAPI *MINIDUMP_WRITE_DUMP)(
	IN HANDLE									hProcess,
	IN DWORD									ProcessId,
	IN HANDLE									hFile,
	IN MINIDUMP_TYPE							DumpType,
	IN CONST PMINIDUMP_EXCEPTION_INFORMATION	ExceptionParam OPTIONAL,
	IN PMINIDUMP_USER_STREAM_INFORMATION		UserStreamParam OPTIONAL,
	IN PMINIDUMP_CALLBACK_INFORMATION			CallbackParam OPTIONAL
	);

#endif


#define DUMP_SIZE_MAX		8000		// max size of our dump
#define CALL_TRACE_MAX		((DUMP_SIZE_MAX - 2000) / (MAX_PATH + 40))	// max number of traced calls
#define NL					_T("\n")	// new line

// 异常调试自动加载
struct DBG_HELP_AUTOLOAD
{
	DBG_HELP_AUTOLOAD();
	~DBG_HELP_AUTOLOAD();

	// for dbghelp.dll
	HMODULE					hDbgHelp;
	MINIDUMP_WRITE_DUMP		pfnMiniDumpWriteDump;
} theDbgHelper;

DBG_HELP_AUTOLOAD::DBG_HELP_AUTOLOAD()
{
	// 加载微软调试模块 dbghelp.dll
	this->hDbgHelp = ::LoadLibrary(_T("dbghelp.dll"));
	if (NULL == theDbgHelper.hDbgHelp)
		return;

	pfnMiniDumpWriteDump = (MINIDUMP_WRITE_DUMP)::GetProcAddress(this->hDbgHelp, (LPCSTR)("MiniDumpWriteDump"));
	if (NULL == pfnMiniDumpWriteDump)
	{
		::FreeLibrary(this->hDbgHelp);
		this->hDbgHelp = NULL;
		return;
	}
}

DBG_HELP_AUTOLOAD::~DBG_HELP_AUTOLOAD()
{
	if (this->hDbgHelp)
	{
		::FreeLibrary(this->hDbgHelp);
		this->hDbgHelp = NULL;
	}
}

/*
* Find module by retAddr (address in the module).
* Return moduleName (full path) and moduleAddr (start address).
* Return TRUE if found.
*/
BOOL WINAPI GetModuleByRetAddr(PBYTE retAddr, PTCHAR moduleName, PBYTE& moduleAddr)
{
	MODULEENTRY32 M = { sizeof(M) };
	HANDLE hSnapshot;

	moduleName[0] = 0;

	hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
	if ((hSnapshot != INVALID_HANDLE_VALUE) && ::Module32First(hSnapshot, &M))
	{
		do
		{
			if (DWORD(retAddr - M.modBaseAddr) < M.modBaseSize)
			{
				lstrcpyn(moduleName, M.szExePath, MAX_PATH);
				moduleAddr = M.modBaseAddr;
				break;
			}
		} while (::Module32Next(hSnapshot, &M));
	}

	::CloseHandle(hSnapshot);

	return !!moduleName[0];
}

/*
* Fill Str with call stack info.
* pException can be either GetExceptionInformation() or NULL.
* If pException = NULL - get current call stack.
*/
void GetCallStack(PEXCEPTION_POINTERS pException, FILE* fp)
{
	TCHAR moduleName[MAX_PATH];
	PBYTE moduleAddr = 0;
	PBYTE moduleAddr1;

	// nonstandard extension used: zero-size array in struct/union
#pragma warning(disable: 4200)
	typedef struct STACK
	{
		STACK*		Ebp;
		PBYTE		RetAddr;
		DWORD		Param[0];
	} STACK, *PSTACK;
#pragma warning(default: 4200)

	STACK Stack = { 0, 0 };
	PSTACK Ebp;

	// fake frame for exception address
	if (pException)
	{
		Stack.Ebp = (PSTACK)(DWORD_PTR)pException->ContextRecord->Ebp;
		Stack.RetAddr = (PBYTE)pException->ExceptionRecord->ExceptionAddress;
		Ebp = &Stack;
	}
	else
	{
		Ebp = (PSTACK)&pException - 1;	// frame addr of GetCallStack()

		// Skip frame of GetCallStack().
		if (!::IsBadReadPtr(Ebp, sizeof(PSTACK)))
			Ebp = Ebp->Ebp;				// caller ebp
	}

	// Trace CALL_TRACE_MAX calls maximum - not to exceed DUMP_SIZE_MAX.
	// Break trace on wrong stack frame.
	for (int retAddrI = 0;
		(retAddrI < CALL_TRACE_MAX) && !::IsBadReadPtr(Ebp, sizeof(PSTACK)) && !::IsBadCodePtr(FARPROC(Ebp->RetAddr));
		retAddrI++, Ebp = Ebp->Ebp)
	{
		// If module with Ebp->RetAddr found.
		if (GetModuleByRetAddr(Ebp->RetAddr, moduleName, moduleAddr1))
		{
			if (moduleAddr1 != moduleAddr)	// new module
			{
				// Save module's address and full path
				moduleAddr = moduleAddr1;
				_ftprintf(fp, _T("%08X  %s")NL, (LONG_PTR)moduleAddr, moduleName);
			}

			// Save call offset.
			_ftprintf(fp, _T("  _%08X"), Ebp->RetAddr - moduleAddr);

			// Save 5 params of the call. We don't know the real number of params.
			if (pException && !retAddrI)	// fake frame for exception address
				_ftprintf(fp, _T("  Exception Offset")NL);
			else if (!::IsBadReadPtr(Ebp, sizeof(PSTACK) + 5 * sizeof(DWORD)))
			{
				_ftprintf(fp, _T("  (%X, %X, %X, %X, %X)")NL,
					Ebp->Param[0], Ebp->Param[1], Ebp->Param[2], Ebp->Param[3], Ebp->Param[4]);
			}
		}
		else
			_ftprintf(fp, _T("%08X")NL, (LONG_PTR)(Ebp->RetAddr));
	}
}

// Fill Str with Windows version.
void WINAPI GetVersionStr(FILE* fp)
{
	OSVERSIONINFOEX V = { sizeof(OSVERSIONINFOEX) };	// EX for NT 5.0 and later

	if (!::GetVersionEx((POSVERSIONINFO)&V))
	{
		::ZeroMemory(&V, sizeof(V));
		V.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		::GetVersionEx((POSVERSIONINFO)&V);
	}

	if (V.dwPlatformId != VER_PLATFORM_WIN32_NT)
		V.dwBuildNumber = LOWORD(V.dwBuildNumber);	// for 9x HIWORD(dwBuildNumber) = 0x04xx

	TCHAR dateBuf[32];
	_tstrdate(dateBuf);
	TCHAR timeBuf[32];
	_tstrtime(timeBuf);

	_ftprintf(fp,
		_T("==============================================================================")NL
		_T("Windows:  %d.%d.%d, SP %d.%d, Product Type %d")NL	// SP - service pack, Product Type - VER_NT_WORKSTATION,...
		NL
		_T("time:     %s %s")NL,
		V.dwMajorVersion, V.dwMinorVersion, V.dwBuildNumber, V.wServicePackMajor, V.wServicePackMinor, V.wProductType,
		dateBuf, timeBuf);
}

void CreateExceptionDesc(PEXCEPTION_POINTERS pException, FILE* fp, DWORD dwLastError)
{
	if (!pException || !fp) return;

	EXCEPTION_RECORD&	E = *pException->ExceptionRecord;
	CONTEXT&			C = *pException->ContextRecord;

	// 取得异常发生地
	TCHAR		szModuleInfo[MAX_PATH];
	TCHAR		szModuleName[MAX_PATH];
	PBYTE		pModuleAddr;
	if (GetModuleByRetAddr((PBYTE)E.ExceptionAddress, szModuleName, pModuleAddr))
	{
		_sntprintf(szModuleInfo, MAX_PATH, _T("%s"), szModuleName);
	}
	else
	{
		_sntprintf(szModuleInfo, MAX_PATH, _T("%08X"), (DWORD_PTR)(E.ExceptionAddress));
	}

	switch (E.ExceptionCode)
	{
		// 核心软异常
	case 0xE000C0DE:
	{
		const std::string* pCPPException = (const std::string*)E.ExceptionInformation[0];
		_ftprintf(fp,
			_T("C++ Exception\n")
			_T("\n")
			_T("Expr:      %s\n"),
			pCPPException->c_str()
			);
	}
	break;
	// 试图对一个虚地址进行读写
	case EXCEPTION_ACCESS_VIOLATION:
	{
		// Access violation type - Write/Read.
		_ftprintf(fp,
			_T("\t\tAccess violation\n")
			_T("\n")
			_T("@:         %s\n")
			_T("Operate:   %s\n")
			_T("Address:   0x%08X\n")
			_T("LastError: 0x%08X\n"),
			szModuleInfo,
			(E.ExceptionInformation[0]) ? _T("Write") : _T("Read"),
			E.ExceptionInformation[1],
			dwLastError
			);
	}
	break;
	default:
	{
		_ftprintf(fp,
			_T("\t\tOTHER\n")
			_T("\n")
			_T("@:         %s\n")
			_T("Code:      0x%08X\n")
			_T("LastError: 0x%08X\n"),
			szModuleInfo,
			E.ExceptionCode,
			dwLastError
			);
	}
	break;
	}
}

// Allocate Str[DUMP_SIZE_MAX] and return Str with dump, if !pException - just return call stack in Str.
VOID GetExceptionInfo(PEXCEPTION_POINTERS pException, FILE* fp, UINT dwLastError)
{
	TCHAR		szModuleName[MAX_PATH];
	PBYTE		pModuleAddr;
	HANDLE		hFile;
	FILETIME	lastWriteTime;
	FILETIME	localFileTime;
	SYSTEMTIME	t;

	GetVersionStr(fp);

	_ftprintf(fp, _T("------------------------------------------------------------------------------")NL);
	_ftprintf(fp, _T("Process:  "));

	::GetModuleFileName(NULL, szModuleName, MAX_PATH);
	_ftprintf(fp, _T("%s")NL, szModuleName);

	// If exception occurred.
	if (pException)
	{
		EXCEPTION_RECORD&	E = *pException->ExceptionRecord;
		CONTEXT&			C = *pException->ContextRecord;

		// If module with E.ExceptionAddress found - save its path and date.
		if (GetModuleByRetAddr((PBYTE)E.ExceptionAddress, szModuleName, pModuleAddr))
		{
			_ftprintf(fp, _T("Module:   %s")NL, szModuleName);

			if ((hFile = ::CreateFile(szModuleName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
			{
				if (::GetFileTime(hFile, NULL, NULL, &lastWriteTime))
				{
					::FileTimeToLocalFileTime(&lastWriteTime, &localFileTime);
					::FileTimeToSystemTime(&localFileTime, &t);

					_ftprintf(fp, _T("Date Modified:  %02d/%02d/%d")NL,
						t.wMonth, t.wDay, t.wYear);
				}
				::CloseHandle(hFile);
			}
		}
		else
		{
			_ftprintf(fp, _T("Exception Addr:  %08X")NL, (LONG_PTR)(E.ExceptionAddress));
		}

		_ftprintf(fp, _T("------------------------------------------------------------------------------")NL);

		// 加入具体异常解释信息
		CreateExceptionDesc(pException, fp, dwLastError);

		_ftprintf(fp, _T("------------------------------------------------------------------------------")NL);

		// Save instruction that caused exception.
		if (E.ExceptionAddress)
		{
			_ftprintf(fp, _T("Instruction: ")NL);
			for (int i = 0; i < 16; i++)
				_ftprintf(fp, _T(" %02X"), PBYTE(E.ExceptionAddress)[i]);
		}
		// Save registers at exception.
		_ftprintf(fp, NL _T("Registers: ") NL);
		_ftprintf(fp, _T("EAX: %08X  EBX: %08X  ECX: %08X  EDX: %08X") NL, C.Eax, C.Ebx, C.Ecx, C.Edx);
		_ftprintf(fp, _T("ESI: %08X  EDI: %08X  ESP: %08X  EBP: %08X") NL, C.Esi, C.Edi, C.Esp, C.Ebp);
		_ftprintf(fp, _T("EIP: %08X EFlags: %08X") NL, C.Eip, C.EFlags);
	}

	_ftprintf(fp, _T("------------------------------------------------------------------------------")NL);

	// Save call stack info.
	_ftprintf(fp, _T("Call Stack: ")NL);
	GetCallStack(pException, fp);
}

BOOL SaveCurrentlyLog(VOID* theLog, size_t logSize)
{
	TCHAR szTempFile[MAX_PATH] = { 0 };
	extern HINSTANCE g_hInstance;
	if (NULL == ::GetModuleFileName(g_hInstance, szTempFile, MAX_PATH))
	{
		memset(szTempFile, 0, MAX_PATH);
		memcpy(szTempFile, "C:\\Stranded_Sml.txt", strlen("C:\\Stranded_Sml.txt"));
	}
	else
	{
		if (TRUE == ::PathRemoveFileSpec(szTempFile))
		{
			strcat(szTempFile, "\\Stranded_Sml.txt");
		}
		else
		{
			memset(szTempFile, 0, MAX_PATH);
			memcpy(szTempFile, "C:\\Stranded_Sml.txt", strlen("C:\\Stranded_Sml.txt"));
		}
	}

	FILE* fp = _tfopen(szTempFile, _T("at"));
	if (!fp) return FALSE;

	fwrite("\n操作描述为: ", strlen("\n操作描述为: "), 1, fp);
	fwrite(theLog, logSize, 1, fp);

	SYSTEMTIME tNow;
	memset(&tNow, 0, sizeof(tNow));
	::GetLocalTime(&tNow);
	CHAR sTime[100] = { 0 };
	memset(sTime, 0, 100);
	sprintf(sTime, "\nTime: %d年%d月%d日%d时%d分%d秒\n", tNow.wYear, tNow.wMonth, tNow.wDay, tNow.wHour, tNow.wMinute, tNow.wSecond);
	fwrite(sTime, strlen(sTime), 1, fp);
	fwrite("\n\n\n------------------------------------------------------\n", strlen("\n\n\n------------------------------------------------------\n"), 1, fp);

	fclose(fp);
	fp = NULL;
	return TRUE;
}

BOOL CreateSmallDumpInfo(PEXCEPTION_POINTERS pException, PTCHAR szSmallFile, UINT dwLastError)
{
	if (!pException) return FALSE;

	// 创建基本描述文件
	TCHAR szTempDir[MAX_PATH] = { 0 };
	::GetTempPath(MAX_PATH, szTempDir);

	TCHAR szTempFile[MAX_PATH] = { 0 };
	::GetTempFileName(szTempDir, _T("sml"), MAX_PATH, szTempFile);

	FILE* fp = _tfopen(szTempFile, _T("w"));
	if (!fp) return FALSE;

	CreateExceptionDesc(pException, fp, dwLastError);

	fclose(fp);
	fp = NULL;

	::GetShortPathName(szTempFile, szSmallFile, MAX_PATH);
	if (szSmallFile[0] == 0) return FALSE;

	return TRUE;
}

BOOL CreateBigInfoFile(PEXCEPTION_POINTERS pException, PTCHAR szBigFile, UINT dwLastError)
{
	if (!pException) return FALSE;

	// 创建基本描述文件
	TCHAR szTempDir[MAX_PATH] = { 0 };
	::GetTempPath(MAX_PATH, szTempDir);

	TCHAR szTempFile[MAX_PATH] = { 0 };
	::GetTempFileName(szTempDir, _T("dtl"), MAX_PATH, szTempFile);

	FILE* fp = _tfopen(szTempFile, _T("w"));
	if (!fp) return FALSE;

	GetExceptionInfo(pException, fp, dwLastError);

	::GetShortPathName(szTempFile, szBigFile, MAX_PATH);
	if (szBigFile[0] == 0) return FALSE;

	return TRUE;
}

BOOL CreateDumpHelpFile(PEXCEPTION_POINTERS pException, PTCHAR szDumpFile)
{
	if (!pException) return FALSE;

	szDumpFile[0] = ' ';
	szDumpFile[1] = 0;

	// If MiniDumpWriteDump() of DbgHelp.dll available.
	if (!theDbgHelper.pfnMiniDumpWriteDump) return FALSE;

	// 创建基本描述文件
	TCHAR szTempDir[MAX_PATH] = { 0 };
	::GetTempPath(MAX_PATH, szTempDir);

	TCHAR szTempFile[MAX_PATH] = { 0 };
	::GetTempFileName(szTempDir, _T("dmp"), MAX_PATH, szTempFile);

	MINIDUMP_EXCEPTION_INFORMATION M;

	M.ThreadId = ::GetCurrentThreadId();
	M.ExceptionPointers = pException;
	M.ClientPointers = 0;

	HANDLE hDumpFile = ::CreateFile(szTempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	theDbgHelper.pfnMiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hDumpFile,
		MiniDumpNormal, (pException) ? &M : NULL, NULL, NULL);

	::CloseHandle(hDumpFile);

	::GetShortPathName(szTempFile, szDumpFile, MAX_PATH);
	if (szDumpFile[0] == 0) return FALSE;

	return TRUE;
}

BOOL CatchSEHException(PEXCEPTION_POINTERS pException, bool bRunCrashReport)
{
	// 保存最后的错误代码
	DWORD dwLastError = ::GetLastError();

	if (!pException) return FALSE;

	// 生成基本描述文件
	TCHAR szSmallInfoFile[MAX_PATH] = { 0 };
	if (!CreateSmallDumpInfo(pException, szSmallInfoFile, dwLastError))
		return FALSE;

	// 生成完整表述文件
	TCHAR szBigInfoFile[MAX_PATH] = { 0 };
	if (!CreateBigInfoFile(pException, szBigInfoFile, dwLastError))
		return FALSE;

	TCHAR szDumpFile[MAX_PATH] = { 0 };
	if (!CreateDumpHelpFile(pException, szDumpFile))
		return FALSE;

	if (bRunCrashReport)
	{
		// 寻找CrashReport
		TCHAR szCrashReportFile[MAX_PATH] = { 0 };
		::GetModuleFileName(NULL, szCrashReportFile, MAX_PATH);
		::PathRemoveFileSpec(szCrashReportFile);
		::PathAppend(szCrashReportFile, _T("CrashReport.exe"));

		TCHAR szCmdLine[1024] = { 0 };
		_sntprintf(szCmdLine, 1024, _T("%s \"%s\" \"%s\" \"%s\""), szCrashReportFile, szSmallInfoFile, szBigInfoFile, szDumpFile);

		// 启动CrashReport
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOWNORMAL;

		if (::CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi) == FALSE)
			return FALSE;
	}

	return TRUE;
}