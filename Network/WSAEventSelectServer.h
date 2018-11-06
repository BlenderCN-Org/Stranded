#ifndef __WSA_EVENT_SELECT_SERVER_H__
#define __WSA_EVENT_SELECT_SERVER_H__

#include <WinSock2.h>


class CWSAEventSelectServer
{
	friend UINT CALLBACK _ServerListenThread(LPVOID lParam);
public:
	CWSAEventSelectServer();
	virtual ~CWSAEventSelectServer();

	int StartListening(const UINT port);
	int StopListening();
private:
	static DWORD s_dwEventTotal;

	static SOCKET s_sockListen;
	static SOCKET s_sockAccept;
	// ��ͻ���ͨ�ŵ�SOCKET
	static SOCKET s_sockAcceptArray[WSA_MAXIMUM_WAIT_EVENTS];
	// ��socket���׵��¼���
	static WSAEVENT s_eventArray[WSA_MAXIMUM_WAIT_EVENTS];
	static WSAEVENT s_newEvent;

	static WSANETWORKEVENTS s_networkEvents;

	static HANDLE s_serverListenThread;

	UINT m_nPort;
};

#endif
