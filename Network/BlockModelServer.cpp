#include "BlockModelServer.h"
#include "../Common/common.h"
#include <process.h>

#ifdef _MSC_VER
#pragma comment (lib,"ws2_32.lib")
#endif

#define BLOCK_BUF_SIZE 8192


UINT CALLBACK _ServerListenThread(LPVOID lParam)
{
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);

	CBlockModelServer* psvSock = (CBlockModelServer*)lParam;

	while (TRUE)
	{
		psvSock->m_sockComm = ::accept(psvSock->m_sockListen, (SOCKADDR*)&clientAddr, &addrLen);
		if (psvSock->m_sockComm == INVALID_SOCKET)
		{
			OutputDebugString("Accept Connection failed!");
			return 1;
		}

		while (TRUE)
		{
			char pRevMsg[BLOCK_BUF_SIZE] = { 0 };

			int iLen = ::recv(psvSock->m_sockComm, pRevMsg, BLOCK_BUF_SIZE, 0);
			if (iLen > 0)
			{
				// �����˳�?
				if (strcmp((LPCSTR)pRevMsg, "[EXIT]") == 0)
					return 0;
				// ��ȡ������
			}
			else if (iLen == SOCKET_ERROR)
			{
				// �����Ͽ�����
				break;
			}
		}
	}

	return 0;
}

HANDLE CBlockModelServer::s_serverListenThread = nullptr;

CBlockModelServer::CBlockModelServer()
{

}

CBlockModelServer::CBlockModelServer(const UINT port)
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
		return;
	}

	// ��������Socket(��ʽ)
	m_sockListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// ����˿ڼ�Э���岢��
	SOCKADDR_IN serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(port);

	// ���׽���
	nRet = ::bind(m_sockListen, (LPSOCKADDR)&serverAddr, sizeof(serverAddr));
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("Bind Socket Fail!");
		::closesocket(m_sockListen);
		return;
	}

	// ��ʼ�����������ü����ͻ�������
	nRet = ::listen(m_sockListen, 1);
	if (nRet == SOCKET_ERROR)
	{
		OutputDebugString("Listening fail!");
		return ;
	}
}

void CBlockModelServer::StartListening()
{
	// ��ʼ�����߳�
	UINT nThreadID;
	s_serverListenThread = (HANDLE)::_beginthreadex(nullptr, 0, _ServerListenThread, this, CREATE_SUSPENDED | THREAD_QUERY_INFORMATION, &nThreadID);
	if (s_serverListenThread == NULL)
	{
		OutputDebugString("Can't create connect thread!");
		return;
	}

	// ��ʼ���м����߳�
	::ResumeThread(s_serverListenThread);
}

void CBlockModelServer::DisConnect()
{
	if (s_serverListenThread != NULL)
	{
		DWORD dwStatus;
		::GetExitCodeThread(s_serverListenThread, &dwStatus);

		// �����߳������У�ǿ����ֹ
		if (dwStatus == STILL_ACTIVE)
		{
			SOCKET tempSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			SOCKADDR_IN serverAddr;

			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(m_nPort);
			serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

			int nRet = ::connect(tempSocket, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR));

			char szExit[64] = { 0 };
			sprintf(szExit, "[EXIT]");
			nRet = ::send(tempSocket, szExit, 64, 0);
		}

		s_serverListenThread = NULL;
	}
}

CBlockModelServer::~CBlockModelServer()
{
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

	::WSACleanup();
}
