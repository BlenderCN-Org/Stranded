#ifndef __COMPLETION_ROUTINE_SERVER_H__
#define __COMPLETION_ROUTINE_SERVER_H__

#include <WinSock2.h>


// 最大可以连入的SOCKET数量，OVERLAPPED I/O模型在P4级别的主机上可以处理上万个工作量不大的SOCKET连接
#define MAX_SOCKET 100


class CCompletionRoutineServer
{
public:
	CCompletionRoutineServer();
	virtual ~CCompletionRoutineServer();

	int StartListening(const UINT port);
	int StopListening();

private:
	// 在我们投递的WSARecv操作完成的时候，系统自动调用的回调函数
	static VOID CALLBACK CompletionRoutine(DWORD error, DWORD bytesTransfered, LPWSAOVERLAPPED overlapped, DWORD inFlags);

	static int GetCurrentSocketIndex(LPWSAOVERLAPPED overlapped);
	// 关闭指定SOCKET，释放资源
	static void ReleaseSocket(const int index);
	// 监听端口，接收连入的连接
	static UINT CALLBACK _ServerListenThread(LPVOID lParam);
	// 用于投递第一个WSARecv请求，并等待系统完成的通知，然后继续投递后续的请求
	static UINT CALLBACK _WaitForCompletionThread(LPVOID lParam);

	int GetEmptySocket();

private:
	UINT m_nPort;

	static HANDLE s_serverListenThread;
	static HANDLE s_waitForCompletionThread;

	// 监听SOCKET，用以接收客户端连接
	static SOCKET s_sockListen;
	// 与客户端通信的SOCKET
	static SOCKET s_sockArray[MAX_SOCKET];
	// OVERLAPPED结构，每个事件对应一个
	static WSAOVERLAPPED s_acceptOverlapped[MAX_SOCKET];
	// 缓冲区，WSARecv参数
	static WSABUF s_dataBuf[MAX_SOCKET];
	// 因为WSAWaitForMultipleEvents() API要求在一个或多个事件对象上等待,
	// 因此不得不创建一个伪事件对象.但是这个事件数组已经不是和SOCKET相关联的了
	static WSAEVENT s_eventArray[1];

	// 是否是第一次投递SOCKET上的WSARecv操作
	static BOOL s_bNewSocket;

	// 当前完成overlapped操作的Socket
	static int s_nCurSockIndex;
	// Socket 序号
	static int s_nSockIndex;
	// Socket 总数
	static int s_nSockTotal;
};

#endif