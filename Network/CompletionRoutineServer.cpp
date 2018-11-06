#include "CompletionRoutineServer.h"
#include <process.h>
#include <tchar.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

// 接收缓冲区
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
	nRet = ::listen(s_sockListen, 1);
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

	// 开始 OVERLAPPED I/O线程, 先休眠，因为没有需要等待的事件
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

		// 尚有线程在运行，强制终止
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

		// 尚有线程在运行，强制终止
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
 * DWORD dwError, // 标志投递的OVERLAPPED操作，比如WSARecv，完成的状态是什么 
 * DWORD cbTransferred, // 指明了在OVERLAPPED操作期间，实际传输的字节量是多大
 * LPWSAOVERLAPPED lpOverlapped, // 参数指明传递到最初的IO调用内的一个OVERLAPPED 结构
 * DWORD dwFlags  // 返回操作结束时可能用的标志(一般没用)); 
 */
VOID CALLBACK CCompletionRoutineServer::CompletionRoutine(DWORD error, DWORD bytesTransfered, LPWSAOVERLAPPED overlapped, DWORD inFlags)
{
	// 根据传入的重叠结构， 来寻找究竟是哪个SOCKET上触发了事件
	s_nCurSockIndex = GetCurrentSocketIndex(overlapped);

	// 错误处理：可能是对方关闭套接字，或者发生一个严重错误
	if (error != 0 || bytesTransfered == 0)
	{
		ReleaseSocket(s_nCurSockIndex);
		return;
	}

	// 数据
	OutputDebugString(s_dataBuf[s_nCurSockIndex].buf);
}

// 根据回调函数中的Overlapped参数，判断是哪个SOCKET上发生了事件
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
	// 客户端断开连接, 关闭SOCKET
	::closesocket(s_sockArray[index]);

	s_nSockTotal--;

	s_nCurSockIndex = NULL_SOCKET;

	// 如果SOCKET总数为0，则处理线程休眠
	if (s_nCurSockIndex <= 0)
		::SuspendThread(s_waitForCompletionThread);
}

UINT CALLBACK CCompletionRoutineServer::_ServerListenThread(LPVOID lParam)
{
	CCompletionRoutineServer* pServer = (CCompletionRoutineServer*)lParam;

	SOCKADDR_IN clientAddr;		// 客户端地址结构
	int addrLen = sizeof(clientAddr);

	while (TRUE)
	{
		// 接收连入的客户端
		SOCKET sockTmp = ::accept(s_sockListen, (SOCKADDR*)&clientAddr, &addrLen);
		if (sockTmp == INVALID_SOCKET)
		{
			OutputDebugString("Accept Conection failed!");
			continue;
		}

		// 获得一个空闲的SOCKET索引号
		s_nSockIndex = pServer->GetEmptySocket();

		s_sockArray[s_nSockIndex] = sockTmp;

		// 这里可以取得客户端的IP和端口，但是我们只取其中的SOCKET编号
		//LPCTSTR lpIP =  inet_ntoa(clientAddr.sin_addr);     // IP
		//UINT nPort = ntohs(clientAddr.sin_port);            // PORT

		s_nSockTotal++;

		// 标志投递一个新的WSARecv请求
		s_bNewSocket = TRUE;

		// SOCKET数量不为空时激活处理线程
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
		// 如果标志为True，则表示投递第一个WSARecv请求！
		if (s_bNewSocket)
		{
			s_bNewSocket = FALSE;

			////////////////////////////现在开始投递第一个WSARecv请求！ /////////////////////////////////
			flags = 0;
			ZeroMemory(&s_acceptOverlapped[s_nSockIndex], sizeof(WSAOVERLAPPED));

			char* buffer = new char[COMPLETE_ROUTINE_DATA_BUFSIZE];
			ZeroMemory(buffer, COMPLETE_ROUTINE_DATA_BUFSIZE);

			s_dataBuf[s_nSockIndex].len = COMPLETE_ROUTINE_DATA_BUFSIZE;
			s_dataBuf[s_nSockIndex].buf = buffer;

			// 将WSAOVERLAPPED结构指定为一个参数,在套接字上投递一个异步WSARecv()请求
			// 并提供下面的作为完成例程的CompletionRoutine回调函数
			if (::WSARecv(s_sockArray[s_nSockIndex], &s_dataBuf[s_nSockIndex], 1, &dwRecvBytes, &flags, &s_acceptOverlapped[s_nSockIndex], CompletionRoutine) == SOCKET_ERROR)
			{
				if (::WSAGetLastError() != WSA_IO_PENDING)
				{
					ReleaseSocket(s_nSockIndex);
					continue;
				}
			}
		}

		// 等待重叠请求完成，自动回调完成例程函数
		/*
		 * DWORD SleepEx(
		 *	DWORD dwMilliseconds,  // 等待的超时时间，如果设置为INFINITE就会一直等待下去
		 *	BOOL   bAlertable   // 是否置于警觉状态，如果为FALSE，则一定要等待超时时间完毕之后才会返回，
		 *						这里希望重叠操作一完成就能返回，所以同WSAWaitForMultipleEvents一样，一定要设置为TRUE
		 *	);
		 */
		DWORD dwIndex = ::WSAWaitForMultipleEvents(1, s_eventArray, FALSE, 10, TRUE);

		// 返回WAIT_IO_COMPLETION表示一个重叠请求完成例程例结束。继续为更多的完成例程服务
		if(dwIndex == WAIT_IO_COMPLETION)
		{
			////////////////////现在开始投递后续的WSARecv请求！////////////////////////////

			// 前一个完成例程结束以后，开始在此套接字上投递下一个WSARecv, 这个nCurSockIndex来自于前面完成例程得到的那个
			if (s_nSockIndex != NULL_SOCKET)
			{
				flags = 0;
				ZeroMemory(&s_acceptOverlapped[s_nSockIndex], sizeof(WSAOVERLAPPED));

				char* buffer = new char[COMPLETE_ROUTINE_DATA_BUFSIZE];
				ZeroMemory(buffer, COMPLETE_ROUTINE_DATA_BUFSIZE);

				s_dataBuf[s_nSockIndex].len = COMPLETE_ROUTINE_DATA_BUFSIZE;
				s_dataBuf[s_nSockIndex].buf = buffer;

				// 将WSAOVERLAPPED结构指定为一个参数,在套接字上投递一个异步WSARecv()请求
				// 并提供下面的作为完成例程的CompletionRoutine回调函数
				if (::WSARecv(s_sockArray[s_nSockIndex], &s_dataBuf[s_nSockIndex], 1, &dwRecvBytes, &flags, &s_acceptOverlapped[s_nSockIndex], CompletionRoutine) == SOCKET_ERROR)
				{
					if (::WSAGetLastError() != WSA_IO_PENDING)
					{
						ReleaseSocket(s_nSockIndex);
						continue;
					}
				}

				// 去掉数组中无效的SOCKET
				// 因为非正常关闭的客户端，比如拔掉网线等，这里是不会接到通知的
			}

			continue;
		}
		else
		{
			if (dwIndex == WAIT_TIMEOUT)
				continue;	 // 继续等待
			else
			{
				OutputDebugString("_WaitForCompletionThread 发生异常，线程将退出！");
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
