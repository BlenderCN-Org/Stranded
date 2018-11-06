#ifndef __COMPLETION_ROUTINE_SERVER_H__
#define __COMPLETION_ROUTINE_SERVER_H__

#include <WinSock2.h>


// �����������SOCKET������OVERLAPPED I/Oģ����P4����������Ͽ��Դ�������������������SOCKET����
#define MAX_SOCKET 100


class CCompletionRoutineServer
{
public:
	CCompletionRoutineServer();
	virtual ~CCompletionRoutineServer();

	int StartListening(const UINT port);
	int StopListening();

private:
	// ������Ͷ�ݵ�WSARecv������ɵ�ʱ��ϵͳ�Զ����õĻص�����
	static VOID CALLBACK CompletionRoutine(DWORD error, DWORD bytesTransfered, LPWSAOVERLAPPED overlapped, DWORD inFlags);

	static int GetCurrentSocketIndex(LPWSAOVERLAPPED overlapped);
	// �ر�ָ��SOCKET���ͷ���Դ
	static void ReleaseSocket(const int index);
	// �����˿ڣ��������������
	static UINT CALLBACK _ServerListenThread(LPVOID lParam);
	// ����Ͷ�ݵ�һ��WSARecv���󣬲��ȴ�ϵͳ��ɵ�֪ͨ��Ȼ�����Ͷ�ݺ���������
	static UINT CALLBACK _WaitForCompletionThread(LPVOID lParam);

	int GetEmptySocket();

private:
	UINT m_nPort;

	static HANDLE s_serverListenThread;
	static HANDLE s_waitForCompletionThread;

	// ����SOCKET�����Խ��տͻ�������
	static SOCKET s_sockListen;
	// ��ͻ���ͨ�ŵ�SOCKET
	static SOCKET s_sockArray[MAX_SOCKET];
	// OVERLAPPED�ṹ��ÿ���¼���Ӧһ��
	static WSAOVERLAPPED s_acceptOverlapped[MAX_SOCKET];
	// ��������WSARecv����
	static WSABUF s_dataBuf[MAX_SOCKET];
	// ��ΪWSAWaitForMultipleEvents() APIҪ����һ�������¼������ϵȴ�,
	// ��˲��ò�����һ��α�¼�����.��������¼������Ѿ����Ǻ�SOCKET���������
	static WSAEVENT s_eventArray[1];

	// �Ƿ��ǵ�һ��Ͷ��SOCKET�ϵ�WSARecv����
	static BOOL s_bNewSocket;

	// ��ǰ���overlapped������Socket
	static int s_nCurSockIndex;
	// Socket ���
	static int s_nSockIndex;
	// Socket ����
	static int s_nSockTotal;
};

#endif