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
		 * hThread - �� CreateThread() ���ص��߳� handle
		 * lpExitCode - ָ��һ�� DWORD, ���Խ��ܽ������� (exit code)
		 *
		 * ����ֵ:
			����ɹ������� TRUE, ���򴫻� FALSE�����ʧ�ܣ��ɵ��� GetLastError() �ҳ�ԭ������߳�
			�ѽ�������ô�̵߳Ľ�������ᱻ���� lpExitCode ����������������߳���δ������ lpExitCode ����
			����ֵ�� STILL_ACTIVE.
		 *
		 * ���̻߳��ڽ���ʱ����δ����ν��������ʱ���ᴫ�� TRUE ��ʾ�ɹ�����ʱ��lpExitCode ָ����ڴ�����Ӧ��
		 * ��ŵ��� STILL_ACTIVE. (����߳�ʵ�ʾͷ����� STILL_ACTIVE, ������޷������߳��Ƿ����)
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