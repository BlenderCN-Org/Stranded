#ifndef __OVERLAPPED_SERVER_H__
#define __OVERLAPPED_SERVER_H__

/*
 * WSAOVERLAPPED
 */

#include <WinSock2.h>
#include "../Common/common.h"


class COverlappedServer
{
private:
	static UINT CALLBACK _ServerListenThread(LPVOID lParam);
	static UINT CALLBACK _OverlappedThread(LPVOID lParam);
	static void MoveEventWhenClose(DWORD index, bool needDec);

public:
	COverlappedServer();
	virtual ~COverlappedServer();

	int StartListening(const UINT port);
	int StopListening();
	int GetLocalIP(char* szIp);
private:
	int GetEmptySocket();

private:
	UINT m_nPort;

	// 事件总数
	static DWORD s_dwEventTotal;

	// 监听SOCKET
	static SOCKET s_sockListen;
	
	// 与客户端通信的SOCKET
	static SOCKET s_sockAcceptArray[WSA_MAXIMUM_WAIT_EVENTS];
	// 配套的事件组
	static WSAEVENT s_eventArray[WSA_MAXIMUM_WAIT_EVENTS];
	// WSAOVERLAPPED结构，每个SOCKET操作对应一个
	static WSAOVERLAPPED s_acceptOverlapped[WSA_MAXIMUM_WAIT_EVENTS];
	// 接收缓冲区, WSARecv的参数，每个SOCKET对应一个
	static WSABUF s_dataBuf[WSA_MAXIMUM_WAIT_EVENTS];

	static HANDLE s_overlappedThread;
	static HANDLE s_serverListenThread;

	// 是否处理overlapped请求
	static BOOL s_bOverlapped;

	// 共享锁
	static SLock s_lock;
};

#endif
