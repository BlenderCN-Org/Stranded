#include "CompletionRoutineServer.h"
#include <process.h>
#include <tchar.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

// ���ջ�����
#define COMPLETE_ROUTINE_DATA_BUFSIZE 4096

#define NULL_SOCKET 1


HANDLE CCompletionRoutineServer::s_serverListenThread = nullptr;
HANDLE CCompletionRoutineServer::s_waitForCompletionThread = nullptr;

SOCKET CCompletionRoutineServer::s_sockListen = INVALID_SOCKET;
SOCKET CCompletionRoutineServer::s_sockArray[MAX_SOCKET];
WSAOVERLAPPED CCompletionRoutineServer::s_acceptOverlapped[MAX_SOCKET];
WSABUF CCompletionRoutineServer::s_dataBuf[MAX_SOCKET];
WSAEVENT CCompletionRoutineServer::s_eventArray[1];

BOOL CCompletionRoutineServer::s_bNewSocket = FALSE;
int CCompletionRoutineServer::s_nCurSockIndex = 0;
int CCompletionRoutineServer::s_nSockIndex = 0;
int CCompletionRoutineServer::s_nSockTotal = 0;


CCompletionRoutineServer::CCompletionRoutineServer()
{
	for (int i = 0; i < MAX_SOCKET; ++i)
	{
		s_sockArray[i] = INVALID_SOCKET;
	}

	s_eventArray[0] = INVALID_HANDLE_VALUE;
}

CCompletionRoutineServer::~CCompletionRoutineServer()
{
	for (int i = 0; i < MAX_SOCKET; ++i)
	{
		if (s_dataBuf[i].buf)
		{
			delete[] s_dataBuf[i].buf;
			s_dataBuf[i].buf = nullptr;
			s_dataBuf[i].len = 0;
		}
	}

	StopListening();

	::WSACleanup();
}

int CCompletionRoutineServer::StartListening(const UINT port)
{
	m_nPort = port;

	WSADATA wsaData;

	int nRet;
	// ����winsock.dll
	nRet = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nRet != 0)
	{
		OutputDebugString("Load winsock2 failed!");
		::WSACleanup();
		return -1;
	}

	// ��������Socket(��ʽ)
	s_sockListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// ����˿ڼ�Э���岢��
	SOCKADDR_IN serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(port);

	// ���׽���
	nRet = ::bind(s_sockListen, (LPSOCKADDR)&serverAddr, sizeof(serverAddr));
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("Bind Socket Fail!");
		::closesocket(s_sockListen);
		return -1;
	}

	// ��ʼ�����������ü����ͻ�������
	nRet = ::listen(s_sockListen, 1);
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("Listening fail!");
		return -1;
	}

	// ��ʼ�����߳�
	UINT nThreadID;
	s_serverListenThread = (HANDLE)::_beginthreadex(nullptr, 0, _ServerListenThread, this, CREATE_SUSPENDED | THREAD_QUERY_INFORMATION, &nThreadID);
	if (s_serverListenThread == NULL)
	{
		OutputDebugString(_T("Can't create connect thread!"));
		::closesocket(s_sockListen);
		return -1;
	}

	// ��ʼ���м����߳�
	::ResumeThread(s_serverListenThread);

	// ��ʼ OVERLAPPED I/O�߳�, �����ߣ���Ϊû����Ҫ�ȴ����¼�
	s_waitForCompletionThread = (HANDLE)::_beginthreadex(nullptr, 0, _WaitForCompletionThread, this, CREATE_SUSPENDED | THREAD_QUERY_INFORMATION, &nThreadID);
	if (s_waitForCompletionThread == NULL)
	{
		OutputDebugString(_T("Can't create connect thread!"));
		::closesocket(s_sockListen);
		return -1;
	}

	return 0;
}

int CCompletionRoutineServer::StopListening()
{
	if (s_serverListenThread != NULL)
	{
		DWORD dwStatus;
		::GetExitCodeThread(s_serverListenThread, &dwStatus);

		// �����߳������У�ǿ����ֹ
		if (dwStatus == STILL_ACTIVE)
		{
			::TerminateThread(s_serverListenThread, 0);
			::CloseHandle(s_serverListenThread);
		}

		s_serverListenThread = NULL;
	}

	if (s_waitForCompletionThread != NULL)
	{
		DWORD dwStatus;
		::GetExitCodeThread(s_waitForCompletionThread, &dwStatus);

		// �����߳������У�ǿ����ֹ
		if (dwStatus == STILL_ACTIVE)
		{
			::TerminateThread(s_waitForCompletionThread, 0);
			::CloseHandle(s_waitForCompletionThread);
		}

		s_waitForCompletionThread = NULL;
	}

	s_nSockTotal = 0;
	s_bNewSocket = FALSE;

	return 0;
}

/*
 * DWORD dwError, // ��־Ͷ�ݵ�OVERLAPPED����������WSARecv����ɵ�״̬��ʲô 
 * DWORD cbTransferred, // ָ������OVERLAPPED�����ڼ䣬ʵ�ʴ�����ֽ����Ƕ��
 * LPWSAOVERLAPPED lpOverlapped, // ����ָ�����ݵ������IO�����ڵ�һ��OVERLAPPED �ṹ
 * DWORD dwFlags  // ���ز�������ʱ�����õı�־(һ��û��)); 
 */
VOID CALLBACK CCompletionRoutineServer::CompletionRoutine(DWORD error, DWORD bytesTransfered, LPWSAOVERLAPPED overlapped, DWORD inFlags)
{
	// ���ݴ�����ص��ṹ�� ��Ѱ�Ҿ������ĸ�SOCKET�ϴ������¼�
	s_nCurSockIndex = GetCurrentSocketIndex(overlapped);

	// �����������ǶԷ��ر��׽��֣����߷���һ�����ش���
	if (error != 0 || bytesTransfered == 0)
	{
		ReleaseSocket(s_nCurSockIndex);
		return;
	}

	// ����
	OutputDebugString(s_dataBuf[s_nCurSockIndex].buf);
}

// ���ݻص������е�Overlapped�������ж����ĸ�SOCKET�Ϸ������¼�
int CCompletionRoutineServer::GetCurrentSocketIndex(LPWSAOVERLAPPED overlapped)
{
	for (int i = 0; i < s_nSockTotal; ++i)
	{
		if (&s_acceptOverlapped[i] == overlapped)
			return i;
	}

	return -1;
}

void CCompletionRoutineServer::ReleaseSocket(const int index)
{
	// �ͻ��˶Ͽ�����, �ر�SOCKET
	::closesocket(s_sockArray[index]);

	s_nSockTotal--;

	s_nCurSockIndex = NULL_SOCKET;

	// ���SOCKET����Ϊ0�������߳�����
	if (s_nCurSockIndex <= 0)
		::SuspendThread(s_waitForCompletionThread);
}

UINT CALLBACK CCompletionRoutineServer::_ServerListenThread(LPVOID lParam)
{
	CCompletionRoutineServer* pServer = (CCompletionRoutineServer*)lParam;

	SOCKADDR_IN clientAddr;		// �ͻ��˵�ַ�ṹ
	int addrLen = sizeof(clientAddr);

	while (TRUE)
	{
		// ��������Ŀͻ���
		SOCKET sockTmp = ::accept(s_sockListen, (SOCKADDR*)&clientAddr, &addrLen);
		if (sockTmp == INVALID_SOCKET)
		{
			OutputDebugString("Accept Conection failed!");
			continue;
		}

		// ���һ�����е�SOCKET������
		s_nSockIndex = pServer->GetEmptySocket();

		s_sockArray[s_nSockIndex] = sockTmp;

		// �������ȡ�ÿͻ��˵�IP�Ͷ˿ڣ���������ֻȡ���е�SOCKET���
		//LPCTSTR lpIP =  inet_ntoa(clientAddr.sin_addr);     // IP
		//UINT nPort = ntohs(clientAddr.sin_port);            // PORT

		s_nSockTotal++;

		// ��־Ͷ��һ���µ�WSARecv����
		s_bNewSocket = TRUE;

		// SOCKET������Ϊ��ʱ������߳�
		if (s_nSockTotal == 1)
			::ResumeThread(s_waitForCompletionThread);
	}

	return 0;
}

UINT CALLBACK CCompletionRoutineServer::_WaitForCompletionThread(LPVOID lParam)
{
	s_eventArray[0] = ::WSACreateEvent();

	DWORD dwRecvBytes = 0, flags = 0;

	while (TRUE)
	{
		// �����־ΪTrue�����ʾͶ�ݵ�һ��WSARecv����
		if (s_bNewSocket)
		{
			s_bNewSocket = FALSE;

			////////////////////////////���ڿ�ʼͶ�ݵ�һ��WSARecv���� /////////////////////////////////
			flags = 0;
			ZeroMemory(&s_acceptOverlapped[s_nSockIndex], sizeof(WSAOVERLAPPED));

			char* buffer = new char[COMPLETE_ROUTINE_DATA_BUFSIZE];
			ZeroMemory(buffer, COMPLETE_ROUTINE_DATA_BUFSIZE);

			s_dataBuf[s_nSockIndex].len = COMPLETE_ROUTINE_DATA_BUFSIZE;
			s_dataBuf[s_nSockIndex].buf = buffer;

			// ��WSAOVERLAPPED�ṹָ��Ϊһ������,���׽�����Ͷ��һ���첽WSARecv()����
			// ���ṩ�������Ϊ������̵�CompletionRoutine�ص�����
			if (::WSARecv(s_sockArray[s_nSockIndex], &s_dataBuf[s_nSockIndex], 1, &dwRecvBytes, &flags, &s_acceptOverlapped[s_nSockIndex], CompletionRoutine) == SOCKET_ERROR)
			{
				if (::WSAGetLastError() != WSA_IO_PENDING)
				{
					ReleaseSocket(s_nSockIndex);
					continue;
				}
			}
		}

		// �ȴ��ص�������ɣ��Զ��ص�������̺���
		/*
		 * DWORD SleepEx(
		 *	DWORD dwMilliseconds,  // �ȴ��ĳ�ʱʱ�䣬�������ΪINFINITE�ͻ�һֱ�ȴ���ȥ
		 *	BOOL   bAlertable   // �Ƿ����ھ���״̬�����ΪFALSE����һ��Ҫ�ȴ���ʱʱ�����֮��Ż᷵�أ�
		 *						����ϣ���ص�����һ��ɾ��ܷ��أ�����ͬWSAWaitForMultipleEventsһ����һ��Ҫ����ΪTRUE
		 *	);
		 */
		DWORD dwIndex = ::WSAWaitForMultipleEvents(1, s_eventArray, FALSE, 10, TRUE);

		// ����WAIT_IO_COMPLETION��ʾһ���ص������������������������Ϊ�����������̷���
		if(dwIndex == WAIT_IO_COMPLETION)
		{
			////////////////////���ڿ�ʼͶ�ݺ�����WSARecv����////////////////////////////

			// ǰһ��������̽����Ժ󣬿�ʼ�ڴ��׽�����Ͷ����һ��WSARecv, ���nCurSockIndex������ǰ��������̵õ����Ǹ�
			if (s_nSockIndex != NULL_SOCKET)
			{
				flags = 0;
				ZeroMemory(&s_acceptOverlapped[s_nSockIndex], sizeof(WSAOVERLAPPED));

				char* buffer = new char[COMPLETE_ROUTINE_DATA_BUFSIZE];
				ZeroMemory(buffer, COMPLETE_ROUTINE_DATA_BUFSIZE);

				s_dataBuf[s_nSockIndex].len = COMPLETE_ROUTINE_DATA_BUFSIZE;
				s_dataBuf[s_nSockIndex].buf = buffer;

				// ��WSAOVERLAPPED�ṹָ��Ϊһ������,���׽�����Ͷ��һ���첽WSARecv()����
				// ���ṩ�������Ϊ������̵�CompletionRoutine�ص�����
				if (::WSARecv(s_sockArray[s_nSockIndex], &s_dataBuf[s_nSockIndex], 1, &dwRecvBytes, &flags, &s_acceptOverlapped[s_nSockIndex], CompletionRoutine) == SOCKET_ERROR)
				{
					if (::WSAGetLastError() != WSA_IO_PENDING)
					{
						ReleaseSocket(s_nSockIndex);
						continue;
					}
				}

				// ȥ����������Ч��SOCKET
				// ��Ϊ�������رյĿͻ��ˣ�����ε����ߵȣ������ǲ���ӵ�֪ͨ��
			}

			continue;
		}
		else
		{
			if (dwIndex == WAIT_TIMEOUT)
				continue;	 // �����ȴ�
			else
			{
				OutputDebugString("_WaitForCompletionThread �����쳣���߳̽��˳���");
				break;
			}
		}
	}

	return 0;
}

int CCompletionRoutineServer::GetEmptySocket()
{
	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
	{
		if (s_sockArray[i] == INVALID_SOCKET)
			return i;
	}

	return -1;
}
