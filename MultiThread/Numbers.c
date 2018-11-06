/*
 * Starts five threads and gives visible feedback of these threads running by printing a number passed in from the primary thread.
 */

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

DWORD WINAPI ThreadFunc(LPVOID);

int main(int argc, char* argv[])
{
	HANDLE hThrd;
	DWORD threadId;
	int i;

	for (i = 0; i < 5; i++)
	{
		/*
		 * 
			HANDLE CreateThread(
				LPSECURITY_ATTRIBUTES lpThreadAttributes,	// 描述施行于这一新线程的 security 属性。NULL表示使用缺省值。次参数在Windows95中被省略。
				SIZE_T dwStackSize,							// 新线程拥有自己的堆栈。0表示使用缺省大小: 1MB.
				LPTHREAD_START_ROUTINE lpStartAddress,		// 新线程将开始的起始地址。这是一个函数指针。（在C语言中函数名称即代表函数指针）
				LPVOID lpParameter,							// 此值将被传送到上述所指定之新线程函数去，作为参数.
				DWORD dwCreationFlags,						// 允许产生一个暂时挂起的线程。默认情况是“立即开始执行”.
				LPDWORD lpThreadId							// 新线程的 ID 会被传回到这里.(Windows NT可传NULL，Windows 95必须赋值)
			)
		 * 返回值
			如果成功，传回一个handle，代表新线程。否则传回一个FALSE。如果失败，可调用 GetLastError() 获知原因.
		 */
		hThrd = CreateThread(NULL,
			0,
			ThreadFunc,
			(LPVOID)i,
			0,
			&threadId);
		if (hThrd)
		{
			printf("Thread launched. [%d] [Ox%x]\n", i, threadId);

			/*
			 *
				BOOL CloseHandle(HANDLE hObject);	// hObject - 代表一个已打开之对象 handle
			 * 返回值:
				成功，传回 TRUE。如果失败则传回 FALSE，可调用 GetLastError() 获知失败原因.
			 *
			 * 线程会导致 “线程核心对象” 开启(也即引用计数+1)，默认的引用计数是2，这里只是使引用计数-1
			 */
			CloseHandle(hThrd);
		}
	}

	// Wait for the threads to complete. We'll see a better way of doing this later.
	Sleep(2000);

	return EXIT_SUCCESS;
}

DWORD WINAPI ThreadFunc(LPVOID n)
{
	int i;
	for (i = 0; i < 10; i++)
		printf("%d%d%d%d%d%d%d%d\n", n, n, n, n, n, n, n, n);

	return 0;
}