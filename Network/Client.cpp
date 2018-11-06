#include "Client.h"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "../Debug/Common.lib")
// 屏蔽deprecation警告
#pragma warning(push)
#pragma warning(disable:4996)
#endif

// 释放指针宏
#define RELEASE(x)			{if(x != NULL) {delete x; x=NULL;}}
// 释放指针
#define RELEASE_ARRAY(x)	{if(x != NULL) {delete[] x, x = NULL;}}
// 释放句柄宏
#define RELEASE_HANDLE(x)	{if(x != NULL && x != INVALID_HANDLE_VALUE) { ::CloseHandle(x); x = NULL; }}

CClient::CClient() : m_strServerIP(CLIENT_DEFAULT_IP), m_strLocalIP(CLIENT_DEFAULT_IP), m_nThreads(CLIENT_DEFAULT_THREADS),
	m_nPort(CLIENT_DEFAULT_PORT), m_strMessage(CLIENT_DEFAULT_MESSAGE),
	m_phWorkerThreads(nullptr), m_hConnectionThread(nullptr), m_hShutdownEvent(nullptr)
{

}

CClient::~CClient()
{
	this->Stop();
}

bool CClient::LoadSocketLib()
{
	WSAData wsaData;
	int nResult = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (NO_ERROR != nResult)
	{
		OutputDebugString(_T("初始化WinSock 2.2失败！\n"));
		return false;
	}

	return true;
}

// 开始监听
bool CClient::Start()
{
	// 建立系统退出的事件通知
	m_hShutdownEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	// 启动连接线程
	DWORD nThreadID;
	THREADPARAMS_CONNECTION* pThreadParams = new THREADPARAMS_CONNECTION;
	pThreadParams->pClient = this;

	m_hConnectionThread = ::CreateThread(0, 0, _ConnectionThread, (void*)pThreadParams, 0, &nThreadID);

	return true;
}

// 停止监听
void CClient::Stop()
{
	if (m_hShutdownEvent == nullptr) return;

	::SetEvent(m_hShutdownEvent);
	// 等待Connection线程退出
	::WaitForSingleObject(m_hConnectionThread, INFINITE);

	// 关闭所有的Socket
	for (int i = 0; i < m_nThreads; ++i)
	{
		::closesocket(m_pParamsWorker[i].sock);
	}

	// 等待所有的工作者线程退出
	::WaitForMultipleObjects(m_nThreads, m_phWorkerThreads, TRUE, INFINITE);

	// 清空资源
	CleanUp();

	OutputDebugString("测试停止.\n");
}

std::string CClient::GetLocalIP()
{
	// 获得本机主机名
	char hostname[MAX_PATH] = { 0 };
	::gethostname(hostname, MAX_PATH);
	struct hostent FAR* lpHostEnt = ::gethostbyname(hostname);
	if (lpHostEnt == nullptr)
	{
		return CLIENT_DEFAULT_IP;
	}

	// 取得IP地址列表中的第一个为返回的IP(因为一台主机可能会绑定多个IP)
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];

	// 将IP地址转化成字符串形式
	struct in_addr inAddr;
	memmove(&inAddr, lpAddr, 4);
	m_strLocalIP = std::string(::inet_ntoa(inAddr));	// 转化成标准的IP地址形式

	return m_strLocalIP;
}

// 建立连接
bool CClient::EstablishConnection()
{
	DWORD nThreadID;

	m_phWorkerThreads = new HANDLE[m_nThreads];
	m_pParamsWorker = new THREADPARAMS_WORKER[m_nThreads];

	// 根据用户设置的线程数量，生成每一个线程连接至服务器，并生成线程发送数据
	for (int i = 0; i < m_nThreads; ++i)
	{
		// 监听用户的停止事件
		if (WAIT_OBJECT_0 == ::WaitForSingleObject(m_hShutdownEvent, 0))
		{
			OutputDebugString(_T("接收到用户停止命令.\n"));
			return true;
		}

		// 向服务器进行连接
		if (!this->ConnectToServer(&m_pParamsWorker[i].sock, m_strServerIP, m_nPort))
		{
			OutputDebugString(_T("连接服务器失败！"));
			CleanUp();
			return false;
		}

		m_pParamsWorker[i].nThreadNo = i + 1;
		sprintf(m_pParamsWorker[i].szBuffer, "[%d]号线程 发送数据 [%s]", i + 1, m_strMessage.c_str());

		Sleep(10);

		// 如果连接服务器成功，就开始建立工作者线程，向服务器发送指定数据
		m_pParamsWorker[i].pClient = this;
		m_phWorkerThreads[i] = ::CreateThread(0, 0, _WorkerThread, (void*)(&m_pParamsWorker[i]), 0, &nThreadID);
	}

	return true;
}

// 向服务器进行Socket连接
bool CClient::ConnectToServer(SOCKET *pSocket, const std::string& strServer, int nPort)
{
	struct sockaddr_in serverAddress;
	struct hostent *server;

	// 生成SOCKET
	*pSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == *pSocket)
	{
		output_debug("错误：初始化Socket失败，错误信息：%d\n", WSAGetLastError());
		return false;
	}

	// 生成地址信息
	server = ::gethostbyname(strServer.c_str());
	if (server == nullptr)
	{
		::closesocket(*pSocket);
		OutputDebugString("错误：无效的服务器地址.\n");
		return false;
	}

	ZeroMemory((char*)&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	CopyMemory((char*)&serverAddress.sin_addr.S_un.S_addr, (char*)server->h_addr, server->h_length);

	serverAddress.sin_port = ::htons(m_nPort);

	// 开始连接服务器
	if (SOCKET_ERROR == ::connect(*pSocket, reinterpret_cast<const struct sockaddr*>(&serverAddress), sizeof(serverAddress)))
	{
		::closesocket(*pSocket);
		OutputDebugString("错误：连接至服务器失败！\n");
		return false;
	}

	return true;
}

// 建立连接的线程
DWORD WINAPI CClient::_ConnectionThread(LPVOID lpParam)
{
	THREADPARAMS_CONNECTION* pParams = (THREADPARAMS_CONNECTION*)lpParam;
	assert(pParams != nullptr);
	CClient* pClient = (CClient*)pParams->pClient;
	assert(pClient != nullptr);

	OutputDebugString("_ConnectionThread 开始建立连接.. \n");

	pClient->EstablishConnection();

	OutputDebugString(_T("_ConnectionThread线程结束.\n"));

	RELEASE(pParams);

	return 0;
}

// 用于发送信息的线程
DWORD WINAPI CClient::_WorkerThread(LPVOID lpParam)
{
	THREADPARAMS_WORKER *pParams = (THREADPARAMS_WORKER*)lpParam;
	assert(pParams != nullptr);
	CClient* pClient = (CClient*)pParams->pClient;
	assert(pClient != nullptr);

	char szTemp[CLIENT_MAX_BUFFER_LEN];
	memset(szTemp, 0, sizeof(szTemp));
	char szRecv[CLIENT_MAX_BUFFER_LEN];
	memset(szRecv, 0, sizeof(szRecv));

	int nByteSent = 0, nByteRecv = 0;

	//CopyMemory(szTemp,pParams->szBuffer,sizeof(pParams->szBuffer));

	// 向服务器发送信息
	sprintf(szTemp, ("第1条信息: [%s]"), pParams->szBuffer);
	nByteSent = ::send(pParams->sock, szTemp, strlen(szTemp), 0);
	if (SOCKET_ERROR == nByteSent)
	{
		output_debug("错误：发送第1次信息失败，错误代码：[%ld]\n", WSAGetLastError());
		return 1;
	}

	output_debug("向服务器发送信息成功: [%s]\n", szTemp);

	Sleep(3000);

	// 再发送一条信息
	memset(szTemp, 0, sizeof(szTemp));
	sprintf(szTemp, ("第2条信息: [%s]"), pParams->szBuffer);
	nByteSent = ::send(pParams->sock, szTemp, strlen(szTemp), 0);
	if (SOCKET_ERROR == nByteSent)
	{
		output_debug("错误：发送第2次信息失败，错误代码：[%ld]\n", WSAGetLastError());
		return 1;
	}

	output_debug("向服务器发送信息成功: [%s]\n", szTemp);

	Sleep(3000);

	// 发第3条信息
	memset(szTemp, 0, sizeof(szTemp));
	sprintf(szTemp, ("第3条信息: [%s]"), pParams->szBuffer);
	nByteSent = ::send(pParams->sock, szTemp, strlen(szTemp), 0);
	if (SOCKET_ERROR == nByteSent)
	{
		output_debug("错误：发送第3次信息失败，错误代码：[%ld]\n", WSAGetLastError());
		return 1;
	}

	output_debug("向服务器发送信息成功: [%s]\n", szTemp);

	if (pParams->nThreadNo == pClient->m_nThreads)
	{
		output_debug(_T("测试并发 [%d] 个线程完毕."), pClient->m_nThreads);
	}

	return 0;
}

// 清空资源
void CClient::CleanUp()
{
	if (m_hShutdownEvent == nullptr) return;

	RELEASE_ARRAY(m_phWorkerThreads);

	RELEASE_HANDLE(m_hConnectionThread);

	RELEASE_ARRAY(m_pParamsWorker);

	RELEASE_HANDLE(m_hShutdownEvent);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
