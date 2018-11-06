#ifndef __BLOCK_MODEL_SERVER_H__
#define __BLOCK_MODEL_SERVER_H__

#include <WinSock2.h>

class CBlockModelServer
{
friend UINT CALLBACK _ServerListenThread(LPVOID lParam);
public:
	CBlockModelServer();
	virtual ~CBlockModelServer();

	CBlockModelServer(const UINT port);

	void StartListening();

	void DisConnect();
private:
	static HANDLE s_serverListenThread;

	SOCKET m_sockListen;
	SOCKET m_sockComm;
	UINT m_nPort;
};

#endif