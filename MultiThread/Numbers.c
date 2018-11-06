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
				LPSECURITY_ATTRIBUTES lpThreadAttributes,	// ����ʩ������һ���̵߳� security ���ԡ�NULL��ʾʹ��ȱʡֵ���β�����Windows95�б�ʡ�ԡ�
				SIZE_T dwStackSize,							// ���߳�ӵ���Լ��Ķ�ջ��0��ʾʹ��ȱʡ��С: 1MB.
				LPTHREAD_START_ROUTINE lpStartAddress,		// ���߳̽���ʼ����ʼ��ַ������һ������ָ�롣����C�����к������Ƽ�������ָ�룩
				LPVOID lpParameter,							// ��ֵ�������͵�������ָ��֮���̺߳���ȥ����Ϊ����.
				DWORD dwCreationFlags,						// �������һ����ʱ������̡߳�Ĭ������ǡ�������ʼִ�С�.
				LPDWORD lpThreadId							// ���̵߳� ID �ᱻ���ص�����.(Windows NT�ɴ�NULL��Windows 95���븳ֵ)
			)
		 * ����ֵ
			����ɹ�������һ��handle���������̡߳����򴫻�һ��FALSE�����ʧ�ܣ��ɵ��� GetLastError() ��֪ԭ��.
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
				BOOL CloseHandle(HANDLE hObject);	// hObject - ����һ���Ѵ�֮���� handle
			 * ����ֵ:
				�ɹ������� TRUE�����ʧ���򴫻� FALSE���ɵ��� GetLastError() ��֪ʧ��ԭ��.
			 *
			 * �̻߳ᵼ�� ���̺߳��Ķ��� ����(Ҳ�����ü���+1)��Ĭ�ϵ����ü�����2������ֻ��ʹ���ü���-1
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