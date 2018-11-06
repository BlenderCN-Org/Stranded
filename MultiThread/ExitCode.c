/*
 * Start two threads and try to exit when the user presses a key.
 */

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>

DWORD WINAPI ThreadFunc(LPVOID);

int main(int argc, char* argv[])
{
	HANDLE hThrd1;
	HANDLE hThrd2;
	DWORD exitCode1 = 0;
	DWORD exitCode2 = 0;
	DWORD threadId;

	hThrd1 = CreateThread(NULL,
		0,
		ThreadFunc,
		(LPVOID)1,
		0,
		&threadId);
	if (hThrd1)
	{
		printf("Thread 1 launched. [0x%x]\n", threadId);
	}

	hThrd2 = CreateThread(NULL,
		0,
		ThreadFunc,
		(LPVOID)2,
		0,
		&threadId);
	if (hThrd2)
	{
		printf("Thread 2 launched. [0x%x]\n", threadId);
	}

	// Keep waiting until both calls to GetExitCodeThread succeed AND neither of them returns STILL_ACTIVE. This method is not optimal.
	for (;;)
	{
		printf("Press any key to exit..\n");
		getch();

		/*
		 * BOOL GetExitCodeThread(HANDLE hThread, LPDWORD lpExitCode);
		 *
		 * hThread - 由 CreateThread() 传回的线程 handle
		 * lpExitCode - 指向一个 DWORD, 用以接受结束代码 (exit code)
		 *
		 * 返回值:
			如果成功，传回 TRUE, 否则传回 FALSE。如果失败，可调用 GetLastError() 找出原因。如果线程
			已结束，那么线程的结束代码会被放在 lpExitCode 参数带回来。如果线程尚未结束， lpExitCode 带回
			来的值是 STILL_ACTIVE.
		 *
		 * 当线程还在进行时，尚未有所谓结束代码时，会传回 TRUE 表示成功。此时，lpExitCode 指向的内存区域应该
		 * 存放的是 STILL_ACTIVE. (如果线程实际就返回了 STILL_ACTIVE, 这里就无法区分线程是否结束)
		 */
		GetExitCodeThread(hThrd1, &exitCode1);
		GetExitCodeThread(hThrd2, &exitCode2);

		if (exitCode1 == STILL_ACTIVE)
			puts("Thread 1 is still running!");
		if (exitCode2 == STILL_ACTIVE)
			puts("Thread 2 is still running!");

		if(exitCode1 != STILL_ACTIVE && exitCode2 != STILL_ACTIVE)
			break;
	}

	CloseHandle(hThrd1);
	CloseHandle(hThrd2);

	printf("Thread 1 returned. [%d]\n", exitCode1);
	printf("Thread 2 returned. [%d]\n", exitCode2);

	return EXIT_SUCCESS;
}

/*
 * Take the startup value, do some simple math on it, and return the calculated value.
 */
DWORD WINAPI ThreadFunc(LPVOID n)
{
	Sleep((DWORD)n * 1000 * 2);
	return (DWORD)n * 10;
}