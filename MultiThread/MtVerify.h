/*
 * @sample:
	MTVERIFY( CloseHandle(hThread) );
 *
 * Error handling for applications.
 * For simplicity, this code includes the complete function PrintError as a static function.
 * To use the PrintError() in an application, it should be taken out, placed in its own source file,
 * and the "static" declaration removed so the function will be globally available.
 */

// 内嵌一个命令道链接器，使它链接 USER32 函数库——甚至即使这个函数库没有出现在链接器参数中
#pragma comment(lib, "USER32")

// 定义了一组由 Visual C++ 4.0 引进的调试函数. 提供"切入调试器并放置 Abort，Retry，Ignore 对话框"功能，为了_ASSERTE() 宏
#include <crtdbg.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

// MTASSERT() 不调用 GetLastError()
// MTVERIFY() 用来检验函数是否成功，并且在函数失败时打印出一段根据 GetLastError() 而得的错误信息。
// MTVERIFY() 使用预处理符号 __FILE__ 和 __LINE__ 提供错误描述时所需的详细信息。
// 也使用 "stringizing" 运算符(能够把宏参数视为一个引号字符串)，也就是宏中的 #a.

#define MTASSERT(a) _ASSERTE(a)

#define MTVERIFY(a) if(!(a)) \
	PrintError(#a, __FILE__, __LINE__, GetLastError())

/*
 * 获得经由预处理器所收集的所有关于MTVERIFY() 宏的信息。如果程序执行与一个窗口中，函数会放出一个消息框，类似MFC 用于asserts.
 * 如果程序是 console 程序，PrintError 会把错误信息送到 stderr 去.
 */

//static void PrintError(LPSTR linedesc, LPSTR filename, int lineno, DWORD errnum)
__inline void PrintError(LPSTR linedesc, LPSTR filename, int lineno, DWORD errnum)
{
	LPSTR lpBuffer;
	char errbuf[256];
#ifdef _WINDOWS
	char modulename[MAX_PATH];
#else // _WINDOWS
	DWORD numread;
#endif // _WINDOWS

	/*
	 * __inline DWORD FormatMessage(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPTSTR lpBuffer, DWORD nSize, va_list *Arguments);
	 * 自动配置所需内存，无需担心缓冲区饱和情况.
	 * 根据 GetLastError() 的结果产生出具有阅读价值的错误信息，这些错误信息是本土文字。
	 */
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errnum,
		LANG_NEUTRAL,
		(LPTSTR)&lpBuffer,
		0,
		NULL);

	wsprintf(errbuf, "\nThe following call failed at line [%d] in [%s]:\n\n"
		"    %s\n\nReason:[%s]\n", lineno, filename, linedesc, lpBuffer);

	// console 程序，取得 stderr handle, 并使用 WriteFile() 显示错误信息. 等候3s确定使用这看到了它。
#ifndef _WINDOWS
	/*
	 * BOOL WriteFile(
		HANDLE hFile,
		LPCVOID lpBuffer,
		DWORD nNumberOfBytesToWrite,
		LPDWORD lpNumberOfBytesWritten,
		LPOVERLAPPED lpOverlapped);
	 */
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), errbuf, strlen(errbuf), &numread, FALSE);
	Sleep(5000);
#else
	// GUI 程序，以消息框显示错误信息，并强迫窗口放在最上层以免被忽略.
	GetModuleFileName(NULL, modulename, MAX_PATH);
	MessageBox(NULL, errbuf, modulename, MB_ICONWARNING | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
#endif

	exit(EXIT_FAILURE);
}