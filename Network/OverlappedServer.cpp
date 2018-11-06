#include "OverlappedServer.h"
#include <process.h>

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

// 接收缓冲区大小
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

		// 尚有线程在运行，强制终止
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

		// 尚有线程在运行，强制终止
		if (dwStatus == STILL_ACTIVE)
		{
			::TerminateThread(s_overlappedThread, 0);
			::CloseHandle(s_overlappedThread);
		}

		s_overlappedThread = NULL;
	}

	::WSACleanup();
}

// 开始监听
int COverlappedServer::StartListening(const UINT port)
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
		return -1;
	}

	// 开始监听，并设置监听客户端数量
	nRet = ::listen(s_sockListen, 5);
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("Listening fail!");
		return -1;
	}

	// 开始监听线程
	UINT nThreadID;
	s_serverListenThread = (HANDLE)::_beginthreadex(nullptr, 0, _ServerListenThread, this, CREATE_SUSPENDED | THREAD_QUERY_INFORMATION, &nThreadID);
	if (s_serverListenThread == NULL)
	{
		OutputDebugString(_T("Can't create connect thread!"));
		::closesocket(s_sockListen);
		return -1;
	}

	// 开始运行监听线程
	::ResumeThread(s_serverListenThread);

	s_bOverlapped = TRUE;

	// 开始 OVERLAPPED I/O线程, 先休眠，因为没有需要等待的事件
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
	
	// 开启winsock.dll
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR)
	{
		OutputDebugString("Load winsock2 failed!");
		::WSACleanup();
		return -1;
	}

	int nLen = 20;
	char hostname[20];
	// 获得本机主机名
	::gethostname(hostname, nLen);

	struct hostent FAR* lpHostEnt = ::gethostbyname(hostname);
	if (lpHostEnt == nullptr)
	{
		memcpy(szIp, "0.0.0.0", sizeof("0.0.0.0"));
		return 0;
	}

	// 取得IP地址列表中的第一个为返回的IP
	char* lpAddr = lpHostEnt->h_addr_list[0];

	struct in_addr inAddr;
	memmove(&inAddr, lpAddr, 4);

	// 转换成标准IP形式
	char* lpTmp = ::inet_ntoa(inAddr);
	memcpy(szIp, lpTmp, sizeof(lpTmp));
	return 0;
}

// 从Socket数组中获得一个空闲的socket
int COverlappedServer::GetEmptySocket()
{
	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; ++i)
	{
		if (s_sockAcceptArray[i] == INVALID_SOCKET)
			return i;
	}

	return -1;
}

// 监听线程
UINT CALLBACK COverlappedServer::_ServerListenThread(LPVOID lParam)
{
	COverlappedServer* pServer = (COverlappedServer*)lParam;

	SOCKADDR_IN clientAddr;		// 客户端地址结构
	int addrLen = sizeof(clientAddr);

	// WSARecv参数
	DWORD flags = 0;
	// 接收到的字符长度
	DWORD dwRecvBytes = 0;
	// socket数组的编号
	int nSockIndex = -1;

	while (TRUE)
	{
		// 超出windows的最大等待事件数量
		if (s_dwEventTotal >= WSA_MAXIMUM_WAIT_EVENTS)
		{
			OutputDebugString("已达到最大连接数!");
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

		// 获取客户端的IP和端口
		LPCTSTR lpIP = ::inet_ntoa(clientAddr.sin_addr);
		UINT nPort = clientAddr.sin_port;

		//char szSock[256] = { 0 };
		//sprintf(szSock, "SOCKET编号:[%d]", s_sockAcceptArray[nSockIndex]);
		//::SendMessage(pServer->m_hNotifyWnd, WM_MSG_NEW_SOCKET, (LPARAM)(LPCTSTR)szSock, (LPARAM)(LPCTSTR)"客户端建立连接!");

		// 接收客户端连接以后，为每一个连入的SOCKET都初始化建立一个WSAOVERLAPPED

		// 建立一个事件对象句柄
		s_eventArray[nSockIndex] = ::WSACreateEvent();
		// 初始化
		ZeroMemory(&s_acceptOverlapped[nSockIndex], sizeof(WSAOVERLAPPED));

		char* buffer = new char[OVERLAPPED_DATA_BUFSIZE];
		ZeroMemory(buffer, OVERLAPPED_DATA_BUFSIZE);

		// 关联事件
		s_acceptOverlapped[nSockIndex].hEvent = s_eventArray[nSockIndex];

		s_dataBuf[nSockIndex].len = OVERLAPPED_DATA_BUFSIZE;
		s_dataBuf[nSockIndex].buf = buffer;

		flags = 0;
		// 投递第一个WSARecv请求，以便开始在SOCKET上接收数据
		if (::WSARecv(s_sockAcceptArray[nSockIndex], &s_dataBuf[nSockIndex], 1, &dwRecvBytes, &flags, &s_acceptOverlapped[nSockIndex], NULL) == SOCKET_ERROR)
		{
			// WSARecv操作成功，I/O操作还未完成，需绑定事件通知操作完成
			if (::WSAGetLastError() != WSA_IO_PENDING)
			{
				OutputDebugString("错误: 第一次投递Recv操作失败!! 此SSOCKET将被关闭");
				::closesocket(s_sockAcceptArray[nSockIndex]);
				s_sockAcceptArray[nSockIndex] = INVALID_SOCKET;

				::WSACloseEvent(s_eventArray[nSockIndex]);

				MoveEventWhenClose(nSockIndex, false);

				continue;
			}
		}

		s_dwEventTotal++;

		// 如果+1==1说明线程休眠了，唤醒
		if (s_dwEventTotal == 1)
			::ResumeThread(s_overlappedThread);
	}

	return 0;
}

// OVERLAPPED I/O 处理线程
UINT CALLBACK COverlappedServer::_OverlappedThread(LPVOID lParam)
{
	COverlappedServer* pServer = (COverlappedServer*)lParam;

	// 接收到的字符长度
	DWORD dwRecvBytes = 0;
	// WSARecv参数
	DWORD flags = 0;
	DWORD dwIndex = 0;
	DWORD dwBytesTransferred = 0;

	// 循环检测时间数组中的事件，并对接收的数据进行处理
	while (s_bOverlapped)
	{
		// 等候Overlapped I/O调用结束, 通过和Overlapped绑定的时间，操作完成会接收到事件通知
		dwIndex = ::WSAWaitForMultipleEvents(s_dwEventTotal, s_eventArray, FALSE, 10, FALSE);

		if(dwIndex == WSA_WAIT_TIMEOUT)
			continue;

		// 出现监听错误
		if (dwIndex == WSA_WAIT_FAILED)
		{
			int nErrCode = ::WSAGetLastError();

			if (nErrCode == WSA_INVALID_HANDLE)
			{
				OutputDebugString("监听出现错误: 无效的 lphEvents 参数!");
			}
			else
			{
				if (nErrCode == WSA_INVALID_PARAMETER)
				{
					OutputDebugString("监听出现错误: 无效的 CEvents 参数!");
				}
			}

			continue;
		}

		// 取得索引值, 获取事件的索引号
		dwIndex = dwIndex - WSA_WAIT_EVENT_0;

		// 重置该事件
		::WSAResetEvent(s_eventArray[dwIndex]);

		// 确定当前索引号的SOCKET的overlapped请求状态
		WSAOVERLAPPED& currOverlapped = s_acceptOverlapped[dwIndex];
		SOCKET& currSock = s_sockAcceptArray[dwIndex];

		::WSAGetOverlappedResult(currSock, &currOverlapped, &dwBytesTransferred, FALSE, &flags);

		// 先检查通信对方是否已经关闭连接
		if (dwBytesTransferred == 0)
		{
			//char szSock[256] = { 0 };
			//sprintf(szSock, "SOCKET编号:[%d]", s_sockAcceptArray[dwIndex]);
			//::SendMessage(pServer->m_hNotifyWnd, WM_MSG_NEW_SOCKET, (LPARAM)(LPCTSTR)szSock, (LPARAM)(LPCTSTR)"客户端断开连接！");

			::closesocket(currSock);
			currSock = INVALID_SOCKET;

			s_dwEventTotal--;
			if (s_dwEventTotal <= 0)
			{
				// 如果没有时间等待，则线程休眠
				::SuspendThread(s_overlappedThread);
			}

			continue;
		}
		
		// s_dataBuf中包含接收到的数据
		//char szSock[256] = { 0 };
		//sprintf(szSock, "SOCKET编号:[%d]", currSock);
		//::SendMessage(pServer->m_hNotifyWnd, WM_MSG_NEW_SOCKET, (LPARAM)(LPCTSTR)szSock, (LPARAM)(LPCTSTR)s_dataBuf[dwIndex].buf);

		// 然后在套接字上投递另一个WSARecv请求
		flags = 0;
		ZeroMemory(&currOverlapped, sizeof(WSAOVERLAPPED));

		char* buffer = new char[OVERLAPPED_DATA_BUFSIZE];
		ZeroMemory(buffer, OVERLAPPED_DATA_BUFSIZE);

		currOverlapped.hEvent = s_eventArray[dwIndex];
		s_dataBuf[dwIndex].len = OVERLAPPED_DATA_BUFSIZE;
		s_dataBuf[dwIndex].buf = buffer;

		// 开始另外一个WSARecv
		if (::WSARecv(currSock, &s_dataBuf[dwIndex], 1, &dwRecvBytes, &flags, &currOverlapped, NULL) == SOCKET_ERROR)
		{
			if (::WSAGetLastError() != WSA_IO_PENDING)
			{
				OutputDebugString("错误: 投递Recv 操作失败!! 此SOCKET将被关闭!");

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
