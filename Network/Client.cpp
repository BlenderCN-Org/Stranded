#include "Client.h"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "../Debug/Common.lib")
// ����deprecation����
#pragma warning(push)
#pragma warning(disable:4996)
#endif

// �ͷ�ָ���
#define RELEASE(x)			{if(x != NULL) {delete x; x=NULL;}}
// �ͷ�ָ��
#define RELEASE_ARRAY(x)	{if(x != NULL) {delete[] x, x = NULL;}}
// �ͷž����
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
		OutputDebugString(_T("��ʼ��WinSock 2.2ʧ�ܣ�\n"));
		return false;
	}

	return true;
}

// ��ʼ����
bool CClient::Start()
{
	// ����ϵͳ�˳����¼�֪ͨ
	m_hShutdownEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	// ���������߳�
	DWORD nThreadID;
	THREADPARAMS_CONNECTION* pThreadParams = new THREADPARAMS_CONNECTION;
	pThreadParams->pClient = this;

	m_hConnectionThread = ::CreateThread(0, 0, _ConnectionThread, (void*)pThreadParams, 0, &nThreadID);

	return true;
}

// ֹͣ����
void CClient::Stop()
{
	if (m_hShutdownEvent == nullptr) return;

	::SetEvent(m_hShutdownEvent);
	// �ȴ�Connection�߳��˳�
	::WaitForSingleObject(m_hConnectionThread, INFINITE);

	// �ر����е�Socket
	for (int i = 0; i < m_nThreads; ++i)
	{
		::closesocket(m_pParamsWorker[i].sock);
	}

	// �ȴ����еĹ������߳��˳�
	::WaitForMultipleObjects(m_nThreads, m_phWorkerThreads, TRUE, INFINITE);

	// �����Դ
	CleanUp();

	OutputDebugString("����ֹͣ.\n");
}

std::string CClient::GetLocalIP()
{
	// ��ñ���������
	char hostname[MAX_PATH] = { 0 };
	::gethostname(hostname, MAX_PATH);
	struct hostent FAR* lpHostEnt = ::gethostbyname(hostname);
	if (lpHostEnt == nullptr)
	{
		return CLIENT_DEFAULT_IP;
	}

	// ȡ��IP��ַ�б��еĵ�һ��Ϊ���ص�IP(��Ϊһ̨�������ܻ�󶨶��IP)
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];

	// ��IP��ַת�����ַ�����ʽ
	struct in_addr inAddr;
	memmove(&inAddr, lpAddr, 4);
	m_strLocalIP = std::string(::inet_ntoa(inAddr));	// ת���ɱ�׼��IP��ַ��ʽ

	return m_strLocalIP;
}

// ��������
bool CClient::EstablishConnection()
{
	DWORD nThreadID;

	m_phWorkerThreads = new HANDLE[m_nThreads];
	m_pParamsWorker = new THREADPARAMS_WORKER[m_nThreads];

	// �����û����õ��߳�����������ÿһ���߳����������������������̷߳�������
	for (int i = 0; i < m_nThreads; ++i)
	{
		// �����û���ֹͣ�¼�
		if (WAIT_OBJECT_0 == ::WaitForSingleObject(m_hShutdownEvent, 0))
		{
			OutputDebugString(_T("���յ��û�ֹͣ����.\n"));
			return true;
		}

		// ���������������
		if (!this->ConnectToServer(&m_pParamsWorker[i].sock, m_strServerIP, m_nPort))
		{
			OutputDebugString(_T("���ӷ�����ʧ�ܣ�"));
			CleanUp();
			return false;
		}

		m_pParamsWorker[i].nThreadNo = i + 1;
		sprintf(m_pParamsWorker[i].szBuffer, "[%d]���߳� �������� [%s]", i + 1, m_strMessage.c_str());

		Sleep(10);

		// ������ӷ������ɹ����Ϳ�ʼ�����������̣߳������������ָ������
		m_pParamsWorker[i].pClient = this;
		m_phWorkerThreads[i] = ::CreateThread(0, 0, _WorkerThread, (void*)(&m_pParamsWorker[i]), 0, &nThreadID);
	}

	return true;
}

// �����������Socket����
bool CClient::ConnectToServer(SOCKET *pSocket, const std::string& strServer, int nPort)
{
	struct sockaddr_in serverAddress;
	struct hostent *server;

	// ����SOCKET
	*pSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == *pSocket)
	{
		output_debug("���󣺳�ʼ��Socketʧ�ܣ�������Ϣ��%d\n", WSAGetLastError());
		return false;
	}

	// ���ɵ�ַ��Ϣ
	server = ::gethostbyname(strServer.c_str());
	if (server == nullptr)
	{
		::closesocket(*pSocket);
		OutputDebugString("������Ч�ķ�������ַ.\n");
		return false;
	}

	ZeroMemory((char*)&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	CopyMemory((char*)&serverAddress.sin_addr.S_un.S_addr, (char*)server->h_addr, server->h_length);

	serverAddress.sin_port = ::htons(m_nPort);

	// ��ʼ���ӷ�����
	if (SOCKET_ERROR == ::connect(*pSocket, reinterpret_cast<const struct sockaddr*>(&serverAddress), sizeof(serverAddress)))
	{
		::closesocket(*pSocket);
		OutputDebugString("����������������ʧ�ܣ�\n");
		return false;
	}

	return true;
}

// �������ӵ��߳�
DWORD WINAPI CClient::_ConnectionThread(LPVOID lpParam)
{
	THREADPARAMS_CONNECTION* pParams = (THREADPARAMS_CONNECTION*)lpParam;
	assert(pParams != nullptr);
	CClient* pClient = (CClient*)pParams->pClient;
	assert(pClient != nullptr);

	OutputDebugString("_ConnectionThread ��ʼ��������.. \n");

	pClient->EstablishConnection();

	OutputDebugString(_T("_ConnectionThread�߳̽���.\n"));

	RELEASE(pParams);

	return 0;
}

// ���ڷ�����Ϣ���߳�
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

	// �������������Ϣ
	sprintf(szTemp, ("��1����Ϣ: [%s]"), pParams->szBuffer);
	nByteSent = ::send(pParams->sock, szTemp, strlen(szTemp), 0);
	if (SOCKET_ERROR == nByteSent)
	{
		output_debug("���󣺷��͵�1����Ϣʧ�ܣ�������룺[%ld]\n", WSAGetLastError());
		return 1;
	}

	output_debug("�������������Ϣ�ɹ�: [%s]\n", szTemp);

	Sleep(3000);

	// �ٷ���һ����Ϣ
	memset(szTemp, 0, sizeof(szTemp));
	sprintf(szTemp, ("��2����Ϣ: [%s]"), pParams->szBuffer);
	nByteSent = ::send(pParams->sock, szTemp, strlen(szTemp), 0);
	if (SOCKET_ERROR == nByteSent)
	{
		output_debug("���󣺷��͵�2����Ϣʧ�ܣ�������룺[%ld]\n", WSAGetLastError());
		return 1;
	}

	output_debug("�������������Ϣ�ɹ�: [%s]\n", szTemp);

	Sleep(3000);

	// ����3����Ϣ
	memset(szTemp, 0, sizeof(szTemp));
	sprintf(szTemp, ("��3����Ϣ: [%s]"), pParams->szBuffer);
	nByteSent = ::send(pParams->sock, szTemp, strlen(szTemp), 0);
	if (SOCKET_ERROR == nByteSent)
	{
		output_debug("���󣺷��͵�3����Ϣʧ�ܣ�������룺[%ld]\n", WSAGetLastError());
		return 1;
	}

	output_debug("�������������Ϣ�ɹ�: [%s]\n", szTemp);

	if (pParams->nThreadNo == pClient->m_nThreads)
	{
		output_debug(_T("���Բ��� [%d] ���߳����."), pClient->m_nThreads);
	}

	return 0;
}

// �����Դ
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
