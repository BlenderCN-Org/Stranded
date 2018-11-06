#include "OverlappedServer.h"
#include <process.h>

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

// ���ջ�������С
#define OVERLAPPED_DATA_BUFSIZE 4096

DWORD COverlappedServer::s_dwEventTotal = 0;

SOCKET COverlappedServer::s_sockListen = INVALID_SOCKET;
SOCKET COverlappedServer::s_sockAcceptArray[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT COverlappedServer::s_eventArray[WSA_MAXIMUM_WAIT_EVENTS];
WSAOVERLAPPED COverlappedServer::s_acceptOverlapped[WSA_MAXIMUM_WAIT_EVENTS];
WSABUF COverlappedServer::s_dataBuf[WSA_MAXIMUM_WAIT_EVENTS];

HANDLE COverlappedServer::s_overlappedThread = NULL;
HANDLE COverlappedServer::s_serverListenThread = NULL;

BOOL COverlappedServer::s_bOverlapped = FALSE;

SLock COverlappedServer::s_lock;

COverlappedServer::COverlappedServer()
{
	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
	{
		s_sockAcceptArray[i] = INVALID_SOCKET;
		s_eventArray[i] = INVALID_HANDLE_VALUE;
	}
}

COverlappedServer::~COverlappedServer()
{
	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
	{
		if (s_dataBuf[i].buf)
		{
			delete[] s_dataBuf[i].buf;
			s_dataBuf[i].buf = nullptr;
			s_dataBuf[i].len = 0;
		}
	}

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

	if (s_overlappedThread != NULL)
	{
		DWORD dwStatus;
		::GetExitCodeThread(s_overlappedThread, &dwStatus);

		// �����߳������У�ǿ����ֹ
		if (dwStatus == STILL_ACTIVE)
		{
			::TerminateThread(s_overlappedThread, 0);
			::CloseHandle(s_overlappedThread);
		}

		s_overlappedThread = NULL;
	}

	::WSACleanup();
}

// ��ʼ����
int COverlappedServer::StartListening(const UINT port)
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
	nRet = ::listen(s_sockListen, 5);
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

	s_bOverlapped = TRUE;

	// ��ʼ OVERLAPPED I/O�߳�, �����ߣ���Ϊû����Ҫ�ȴ����¼�
	s_overlappedThread = (HANDLE)::_beginthreadex(nullptr, 0, _OverlappedThread, this, CREATE_SUSPENDED | THREAD_QUERY_INFORMATION, &nThreadID);
	if (s_overlappedThread == NULL)
	{
		OutputDebugString(_T("Can't create connect thread!"));
		::closesocket(s_sockListen);
		return -1;
	}

	return 0;
}

int COverlappedServer::StopListening()
{
	s_overlappedThread = FALSE;

	s_dwEventTotal = 0;

	return 0;
}

int COverlappedServer::GetLocalIP(char* szIp)
{
	assert(szIp == nullptr);

	WSADATA wsaData;
	
	// ����winsock.dll
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR)
	{
		OutputDebugString("Load winsock2 failed!");
		::WSACleanup();
		return -1;
	}

	int nLen = 20;
	char hostname[20];
	// ��ñ���������
	::gethostname(hostname, nLen);

	struct hostent FAR* lpHostEnt = ::gethostbyname(hostname);
	if (lpHostEnt == nullptr)
	{
		memcpy(szIp, "0.0.0.0", sizeof("0.0.0.0"));
		return 0;
	}

	// ȡ��IP��ַ�б��еĵ�һ��Ϊ���ص�IP
	char* lpAddr = lpHostEnt->h_addr_list[0];

	struct in_addr inAddr;
	memmove(&inAddr, lpAddr, 4);

	// ת���ɱ�׼IP��ʽ
	char* lpTmp = ::inet_ntoa(inAddr);
	memcpy(szIp, lpTmp, sizeof(lpTmp));
	return 0;
}

// ��Socket�����л��һ�����е�socket
int COverlappedServer::GetEmptySocket()
{
	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
	{
		if (s_sockAcceptArray[i] == INVALID_SOCKET)
			return i;
	}

	return -1;
}

// �����߳�
UINT CALLBACK COverlappedServer::_ServerListenThread(LPVOID lParam)
{
	COverlappedServer* pServer = (COverlappedServer*)lParam;

	SOCKADDR_IN clientAddr;		// �ͻ��˵�ַ�ṹ
	int addrLen = sizeof(clientAddr);

	// WSARecv����
	DWORD flags = 0;
	// ���յ����ַ�����
	DWORD dwRecvBytes = 0;
	// socket����ı��
	int nSockIndex = -1;

	while (TRUE)
	{
		// ����windows�����ȴ��¼�����
		if (s_dwEventTotal >= WSA_MAXIMUM_WAIT_EVENTS)
		{
			OutputDebugString("�Ѵﵽ���������!");
			continue;
		}

		SOCKET sockTmp = ::accept(s_sockListen, (SOCKADDR*)&clientAddr, &addrLen);
		if (sockTmp == INVALID_SOCKET)
		{
			OutputDebugString("Accept Conection failed!");
			continue;
		}

		// client IP
		//LPCTSTR lpIP = ::inet_ntoa(clientAddr.sin_addr);
		// client Port
		//UINT nPort = clientAddr.sin_port;

		nSockIndex = pServer->GetEmptySocket();

		s_sockAcceptArray[nSockIndex] = sockTmp;

		// ��ȡ�ͻ��˵�IP�Ͷ˿�
		LPCTSTR lpIP = ::inet_ntoa(clientAddr.sin_addr);
		UINT nPort = clientAddr.sin_port;

		//char szSock[256] = { 0 };
		//sprintf(szSock, "SOCKET���:[%d]", s_sockAcceptArray[nSockIndex]);
		//::SendMessage(pServer->m_hNotifyWnd, WM_MSG_NEW_SOCKET, (LPARAM)(LPCTSTR)szSock, (LPARAM)(LPCTSTR)"�ͻ��˽�������!");

		// ���տͻ��������Ժ�Ϊÿһ�������SOCKET����ʼ������һ��WSAOVERLAPPED

		// ����һ���¼�������
		s_eventArray[nSockIndex] = ::WSACreateEvent();
		// ��ʼ��
		ZeroMemory(&s_acceptOverlapped[nSockIndex], sizeof(WSAOVERLAPPED));

		char* buffer = new char[OVERLAPPED_DATA_BUFSIZE];
		ZeroMemory(buffer, OVERLAPPED_DATA_BUFSIZE);

		// �����¼�
		s_acceptOverlapped[nSockIndex].hEvent = s_eventArray[nSockIndex];

		s_dataBuf[nSockIndex].len = OVERLAPPED_DATA_BUFSIZE;
		s_dataBuf[nSockIndex].buf = buffer;

		flags = 0;
		// Ͷ�ݵ�һ��WSARecv�����Ա㿪ʼ��SOCKET�Ͻ�������
		if (::WSARecv(s_sockAcceptArray[nSockIndex], &s_dataBuf[nSockIndex], 1, &dwRecvBytes, &flags, &s_acceptOverlapped[nSockIndex], NULL) == SOCKET_ERROR)
		{
			// WSARecv�����ɹ���I/O������δ��ɣ�����¼�֪ͨ�������
			if (::WSAGetLastError() != WSA_IO_PENDING)
			{
				OutputDebugString("����: ��һ��Ͷ��Recv����ʧ��!! ��SSOCKET�����ر�");
				::closesocket(s_sockAcceptArray[nSockIndex]);
				s_sockAcceptArray[nSockIndex] = INVALID_SOCKET;

				::WSACloseEvent(s_eventArray[nSockIndex]);

				MoveEventWhenClose(nSockIndex, false);

				continue;
			}
		}

		s_dwEventTotal++;

		// ���+1==1˵���߳������ˣ�����
		if (s_dwEventTotal == 1)
			::ResumeThread(s_overlappedThread);
	}

	return 0;
}

// OVERLAPPED I/O �����߳�
UINT CALLBACK COverlappedServer::_OverlappedThread(LPVOID lParam)
{
	COverlappedServer* pServer = (COverlappedServer*)lParam;

	// ���յ����ַ�����
	DWORD dwRecvBytes = 0;
	// WSARecv����
	DWORD flags = 0;
	DWORD dwIndex = 0;
	DWORD dwBytesTransferred = 0;

	// ѭ�����ʱ�������е��¼������Խ��յ����ݽ��д���
	while (s_bOverlapped)
	{
		// �Ⱥ�Overlapped I/O���ý���, ͨ����Overlapped�󶨵�ʱ�䣬������ɻ���յ��¼�֪ͨ
		dwIndex = ::WSAWaitForMultipleEvents(s_dwEventTotal, s_eventArray, FALSE, 10, FALSE);

		if(dwIndex == WSA_WAIT_TIMEOUT)
			continue;

		// ���ּ�������
		if (dwIndex == WSA_WAIT_FAILED)
		{
			int nErrCode = ::WSAGetLastError();

			if (nErrCode == WSA_INVALID_HANDLE)
			{
				OutputDebugString("�������ִ���: ��Ч�� lphEvents ����!");
			}
			else
			{
				if (nErrCode == WSA_INVALID_PARAMETER)
				{
					OutputDebugString("�������ִ���: ��Ч�� CEvents ����!");
				}
			}

			continue;
		}

		// ȡ������ֵ, ��ȡ�¼���������
		dwIndex = dwIndex - WSA_WAIT_EVENT_0;

		// ���ø��¼�
		::WSAResetEvent(s_eventArray[dwIndex]);

		// ȷ����ǰ�����ŵ�SOCKET��overlapped����״̬
		WSAOVERLAPPED& currOverlapped = s_acceptOverlapped[dwIndex];
		SOCKET& currSock = s_sockAcceptArray[dwIndex];

		::WSAGetOverlappedResult(currSock, &currOverlapped, &dwBytesTransferred, FALSE, &flags);

		// �ȼ��ͨ�ŶԷ��Ƿ��Ѿ��ر�����
		if (dwBytesTransferred == 0)
		{
			//char szSock[256] = { 0 };
			//sprintf(szSock, "SOCKET���:[%d]", s_sockAcceptArray[dwIndex]);
			//::SendMessage(pServer->m_hNotifyWnd, WM_MSG_NEW_SOCKET, (LPARAM)(LPCTSTR)szSock, (LPARAM)(LPCTSTR)"�ͻ��˶Ͽ����ӣ�");

			::closesocket(currSock);
			currSock = INVALID_SOCKET;

			s_dwEventTotal--;
			if (s_dwEventTotal <= 0)
			{
				// ���û��ʱ��ȴ������߳�����
				::SuspendThread(s_overlappedThread);
			}

			continue;
		}
		
		// s_dataBuf�а������յ�������
		//char szSock[256] = { 0 };
		//sprintf(szSock, "SOCKET���:[%d]", currSock);
		//::SendMessage(pServer->m_hNotifyWnd, WM_MSG_NEW_SOCKET, (LPARAM)(LPCTSTR)szSock, (LPARAM)(LPCTSTR)s_dataBuf[dwIndex].buf);

		// Ȼ�����׽�����Ͷ����һ��WSARecv����
		flags = 0;
		ZeroMemory(&currOverlapped, sizeof(WSAOVERLAPPED));

		char* buffer = new char[OVERLAPPED_DATA_BUFSIZE];
		ZeroMemory(buffer, OVERLAPPED_DATA_BUFSIZE);

		currOverlapped.hEvent = s_eventArray[dwIndex];
		s_dataBuf[dwIndex].len = OVERLAPPED_DATA_BUFSIZE;
		s_dataBuf[dwIndex].buf = buffer;

		// ��ʼ����һ��WSARecv
		if (::WSARecv(currSock, &s_dataBuf[dwIndex], 1, &dwRecvBytes, &flags, &currOverlapped, NULL) == SOCKET_ERROR)
		{
			if (::WSAGetLastError() != WSA_IO_PENDING)
			{
				OutputDebugString("����: Ͷ��Recv ����ʧ��!! ��SOCKET�����ر�!");

				::closesocket(currSock);
				currSock = INVALID_SOCKET;

				::WSACloseEvent(s_eventArray[dwIndex]);

				MoveEventWhenClose(dwIndex, true);

				continue;
			}
		}
	}

	return 0;
}

void COverlappedServer::MoveEventWhenClose(DWORD index, bool needDec)
{
	s_lock.Lock();

	if (index + 1 != s_dwEventTotal)
	{
		for (int i = index; i < s_dwEventTotal; ++i)
		{
			s_eventArray[i] = s_eventArray[i + 1];
			s_sockAcceptArray[i] = s_sockAcceptArray[i + 1];
		}
	}

	if (needDec)
		s_dwEventTotal--;

	s_lock.Unlock();
}
