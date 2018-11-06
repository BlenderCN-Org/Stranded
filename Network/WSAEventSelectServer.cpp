#include "WSAEventSelectServer.h"
#include <process.h>

#ifdef _MSC_VER
#pragma comment (lib,"ws2_32.lib")
#endif

#define WSA_EVENT_SELECT_DATA_BUF 4096


DWORD CWSAEventSelectServer::s_dwEventTotal = 0;

SOCKET CWSAEventSelectServer::s_sockListen = INVALID_SOCKET;
SOCKET CWSAEventSelectServer::s_sockAccept = INVALID_SOCKET;
SOCKET CWSAEventSelectServer::s_sockAcceptArray[WSA_MAXIMUM_WAIT_EVENTS];

WSAEVENT CWSAEventSelectServer::s_eventArray[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT CWSAEventSelectServer::s_newEvent = INVALID_HANDLE_VALUE;

WSANETWORKEVENTS CWSAEventSelectServer::s_networkEvents;

HANDLE CWSAEventSelectServer::s_serverListenThread = nullptr;

// 监听线程
static UINT CALLBACK _ServerListenThread(LPVOID lParam)
{
	DWORD index = 0, i = 0;
	while (TRUE)
	{
		index = ::WSAWaitForMultipleEvents(CWSAEventSelectServer::s_dwEventTotal, CWSAEventSelectServer::s_eventArray, FALSE, WSA_INFINITE, FALSE);

		index = index - WSA_WAIT_EVENT_0;

		for (i = index; i < CWSAEventSelectServer::s_dwEventTotal; ++i)
		{
			index = ::WSAWaitForMultipleEvents(1, &CWSAEventSelectServer::s_eventArray[i], TRUE, 1000, FALSE);

			if((index == WSA_WAIT_FAILED) || (index == WSA_WAIT_TIMEOUT))
				continue;
			else
			{
				index = i;

				::WSAEnumNetworkEvents(CWSAEventSelectServer::s_sockAcceptArray[index], CWSAEventSelectServer::s_eventArray[index], &CWSAEventSelectServer::s_networkEvents);

				if (CWSAEventSelectServer::s_networkEvents.lNetworkEvents & FD_ACCEPT)
				{
					if (CWSAEventSelectServer::s_networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
					{
						OutputDebugString("accept error!");
						return -1;
					}

					CWSAEventSelectServer::s_sockAccept = ::accept(CWSAEventSelectServer::s_sockAcceptArray[index], NULL, NULL);

					CWSAEventSelectServer::s_newEvent = ::WSACreateEvent();

					::WSAEventSelect(CWSAEventSelectServer::s_sockAccept, CWSAEventSelectServer::s_newEvent, FD_READ | FD_CLOSE);

					CWSAEventSelectServer::s_eventArray[CWSAEventSelectServer::s_dwEventTotal] = CWSAEventSelectServer::s_newEvent;
					CWSAEventSelectServer::s_sockAcceptArray[CWSAEventSelectServer::s_dwEventTotal] = CWSAEventSelectServer::s_sockAccept;

					CWSAEventSelectServer::s_dwEventTotal++;
				}

				if (CWSAEventSelectServer::s_networkEvents.lNetworkEvents & FD_READ)
				{
					if (CWSAEventSelectServer::s_networkEvents.iErrorCode[FD_READ_BIT] != 0)
					{
						OutputDebugString("Read error!");
						return -1;
					}

					char buffer[WSA_EVENT_SELECT_DATA_BUF];
					::recv(CWSAEventSelectServer::s_sockAcceptArray[index - WSA_WAIT_EVENT_0], buffer, WSA_EVENT_SELECT_DATA_BUF, 0);
				}

				if (CWSAEventSelectServer::s_networkEvents.lNetworkEvents & FD_CLOSE)
				{
					if (CWSAEventSelectServer::s_networkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
					{
						OutputDebugString("Close error!");
						return -1;
					}

					::closesocket(CWSAEventSelectServer::s_sockAcceptArray[index - WSA_WAIT_EVENT_0]);

					::WSACloseEvent(CWSAEventSelectServer::s_eventArray[index - WSA_WAIT_EVENT_0]);

					CWSAEventSelectServer::s_dwEventTotal--;
				}
			}
		}
	}

	return 0;
}

CWSAEventSelectServer::CWSAEventSelectServer()
{
	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
	{
		s_eventArray[i] = INVALID_HANDLE_VALUE;
	}
}

CWSAEventSelectServer::~CWSAEventSelectServer()
{
	if (s_serverListenThread != NULL)
	{
		DWORD dwStatus;
		::GetExitCodeThread(s_serverListenThread, &dwStatus);

		// 尚有线程在运行，强制终止
		if (dwStatus == STILL_ACTIVE)
		{
			::TerminateThread(s_serverListenThread, 0);
			::CloseHandle(s_serverListenThread);
		}

		s_serverListenThread = NULL;
	}

	::WSACleanup();
}

int CWSAEventSelectServer::StartListening(const UINT port)
{
	m_nPort = port;

	WSADATA wsaData;

	int nRet;
	// 开启winsock.dll
	nRet = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nRet != 0)
	{
		OutputDebugString("Load winsock2 failed!");
		::WSACleanup();
		return -1;
	}

	// 创建服务Socket(流式)
	s_sockListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// 分配端口及协议族并绑定
	SOCKADDR_IN serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(port);

	// 绑定套接字
	nRet = ::bind(s_sockListen, (LPSOCKADDR)&serverAddr, sizeof(serverAddr));
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("Bind Socket Fail!");
		::closesocket(s_sockListen);
		return -2;
	}

	s_newEvent = ::WSACreateEvent();

	::WSAEventSelect(s_sockListen, s_newEvent, FD_ACCEPT | FD_CLOSE);

	// 开始监听，并设置监听客户端数量
	nRet = ::listen(s_sockListen, 5);
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("Listening fail!");
		return -3;
	}

	s_sockAcceptArray[s_dwEventTotal] = s_sockListen;
	s_eventArray[s_dwEventTotal] = s_newEvent;

	s_dwEventTotal++;

	// 开始监听线程
	UINT nThreadID;
	s_serverListenThread = (HANDLE)::_beginthreadex(nullptr, 0, _ServerListenThread, this, CREATE_SUSPENDED | THREAD_QUERY_INFORMATION, &nThreadID);
	if (s_serverListenThread == NULL)
	{
		OutputDebugString("Can't create connect thread!");
		return -4;
	}

	// 开始运行监听线程
	::ResumeThread(s_serverListenThread);

	return 0;
}

int CWSAEventSelectServer::StopListening()
{
	s_dwEventTotal = 0;

	return 0;
}
