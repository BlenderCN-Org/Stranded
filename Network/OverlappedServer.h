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

	// �¼�����
	static DWORD s_dwEventTotal;

	// ����SOCKET
	static SOCKET s_sockListen;
	
	// ��ͻ���ͨ�ŵ�SOCKET
	static SOCKET s_sockAcceptArray[WSA_MAXIMUM_WAIT_EVENTS];
	// ���׵��¼���
	static WSAEVENT s_eventArray[WSA_MAXIMUM_WAIT_EVENTS];
	// WSAOVERLAPPED�ṹ��ÿ��SOCKET������Ӧһ��
	static WSAOVERLAPPED s_acceptOverlapped[WSA_MAXIMUM_WAIT_EVENTS];
	// ���ջ�����, WSARecv�Ĳ�����ÿ��SOCKET��Ӧһ��
	static WSABUF s_dataBuf[WSA_MAXIMUM_WAIT_EVENTS];

	static HANDLE s_overlappedThread;
	static HANDLE s_serverListenThread;

	// �Ƿ���overlapped����
	static BOOL s_bOverlapped;

	// ������
	static SLock s_lock;
};

#endif
