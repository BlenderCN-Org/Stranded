/*
 * @sample:
	MTVERIFY( CloseHandle(hThread) );
 *
 * Error handling for applications.
 * For simplicity, this code includes the complete function PrintError as a static function.
 * To use the PrintError() in an application, it should be taken out, placed in its own source file,
 * and the "static" declaration removed so the function will be globally available.
 */

// ��Ƕһ���������������ʹ������ USER32 �����⡪��������ʹ���������û�г�����������������
#pragma comment(lib, "USER32")

// ������һ���� Visual C++ 4.0 �����ĵ��Ժ���. �ṩ"��������������� Abort��Retry��Ignore �Ի���"���ܣ�Ϊ��_ASSERTE() ��
#include <crtdbg.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

// MTASSERT() ������ GetLastError()
// MTVERIFY() �������麯���Ƿ�ɹ��������ں���ʧ��ʱ��ӡ��һ�θ��� GetLastError() ���õĴ�����Ϣ��
// MTVERIFY() ʹ��Ԥ������� __FILE__ �� __LINE__ �ṩ��������ʱ�������ϸ��Ϣ��
// Ҳʹ�� "stringizing" �����(�ܹ��Ѻ������Ϊһ�������ַ���)��Ҳ���Ǻ��е� #a.

#define MTASSERT(a) _ASSERTE(a)

#define MTVERIFY(a) if(!(a)) \
	PrintError(#a, __FILE__, __LINE__, GetLastError())

/*
 * ��þ���Ԥ���������ռ������й���MTVERIFY() �����Ϣ���������ִ����һ�������У�������ų�һ����Ϣ������MFC ����asserts.
 * ��������� console ����PrintError ��Ѵ�����Ϣ�͵� stderr ȥ.
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
	 * �Զ����������ڴ棬���赣�Ļ������������.
	 * ���� GetLastError() �Ľ�������������Ķ���ֵ�Ĵ�����Ϣ����Щ������Ϣ�Ǳ������֡�
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

	// console ����ȡ�� stderr handle, ��ʹ�� WriteFile() ��ʾ������Ϣ. �Ⱥ�3sȷ��ʹ���⿴��������
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
	// GUI ��������Ϣ����ʾ������Ϣ����ǿ�ȴ��ڷ������ϲ����ⱻ����.
	GetModuleFileName(NULL, modulename, MAX_PATH);
	MessageBox(NULL, errbuf, modulename, MB_ICONWARNING | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
#endif

	exit(EXIT_FAILURE);
}