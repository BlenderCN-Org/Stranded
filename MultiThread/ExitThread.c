/*
 * Demonstrate ExitThread
 */

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "MtVerify.h"

DWORD WINAPI ThreadFunc(LPVOID);
void AnotherFunc(void);

int main(int argc, char* argv[])
{
	HANDLE hThrd;
	DWORD exitCode = 0;
	DWORD threadId;

	hThrd = CreateThread(NULL,
		0,
		ThreadFunc,
		(LPVOID)1,
		0,
		&threadId);
	if (hThrd)
		printf("Thread launched. [0x%x]\n", threadId);

	for (;;)
	{
		BOOL rc;
		MTVERIFY( rc = GetExitCodeThread(hThrd, &exitCode) );
		if(rc && exitCode != STILL_ACTIVE)
			break;
	}

	MTVERIFY(CloseHandle(hThrd));

	printf("Thread returned. [%d]\n", exitCode);

	return EXIT_SUCCESS;
}

/*
 * Call a function to do something that terminates the thread with ExitThread instead of retuning.
 */
DWORD WINAPI ThreadFunc(LPVOID n)
{
	printf("Thread running.\n");
	AnotherFunc();
	return 0;
}

void AnotherFunc()
{
	printf("About to exit thread.\n");

	/*
	 * 强制结束一个线程
	 * VOID ExitThread(DWORD dwExitCode);
	 * dwExitCode - 指定此线程之结束代码
	 */
	ExitThread(4);

	// It is impossible to get here, this line will never be printed.
	printf("This will never print.\n");
}