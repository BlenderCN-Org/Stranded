#include "IOCPServer.h"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "../lib/Common.lib")
#endif

/*
 * ÿһ���������ϲ������ٸ��߳� (Ϊ������޶ȵ���������������).
 * ����ǽ���CPU�������� * 2��ô����̣߳��������CPU��Դ����Ϊ��ɶ˿ڵĵ����Ƿǳ����ܵ�.
 * Worker�߳��е�ʱ����ܻ���Sleep()����WaitForSingleObject()֮��������
 * ͬһ��CPU�����ϵ���һ���߳̾Ϳ��Դ������Sleep���߳�ִ���ˣ���Ϊ��ɶ˿ڵ�Ŀ����Ҫʹ��CPU�����ɵĹ�����
 */
#define WORKER_THREADS_PER_PROCESSOR	2
// ͬʱͶ�ݵ�Accept���������(���Ҫ����ʵ�ʵ�����������)
#define MAX_POST_ACCEPT	10
// ���ݸ�Worker�̵߳��˳��ź�
#define EXIT_CODE	NULL

// �ͷ�ָ��;����Դ�ĺ�

// �ͷ�ָ���
#define RELEASE(x)			{if(x != NULL) {delete x; x=NULL;}}
// �ͷ�ָ��
#define RELEASE_ARRAY(x)	{if(x != NULL) {delete[] x, x = NULL;}}
// �ͷž����
#define RELEASE_HANDLE(x)	{if(x != NULL && x != INVALID_HANDLE_VALUE) { ::CloseHandle(x); x = NULL; }}
// �ͷ�Socket��
#define RELEASE_SOCKET(x)	{if(x != INVALID_SOCKET) { ::closesocket(x); x = INVALID_SOCKET; }}

CIOCPServer::CIOCPServer() : m_nThreads(0), m_hShutdownEvent(NULL), m_hIOCompletionPort(NULL),
							m_phWorkerThreads(NULL), m_strIP(IOCP_DEFAULT_IP), m_nPort(IOCP_DEFAULT_PORT),
							m_lpfnAcceptEx(NULL), m_pListenContext(NULL), m_lpfnGetAcceptExSockAddrs(NULL)
{

}

CIOCPServer::~CIOCPServer()
{
	// ȷ����Դ�����ͷ�
	this->Stop();
}

// ����������
bool CIOCPServer::Start()
{
	// ��ʼ���̻߳�����
	::InitializeCriticalSection(&m_csContextList);

	// ����ϵͳ�˳����¼�֪ͨ
	m_hShutdownEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	// ��ʼ��IOCP
	if (false == InitializeIOCP())
	{
		OutputDebugString(_T("��ʼ��IOCPʧ�ܣ�\n"));
		return false;
	}
	else
	{
		OutputDebugString("\nIOCP��ʼ�����\n.");
	}

	// ��ʼ��Socket
	if (false == InitializeListenSocket())
	{
		OutputDebugString(_T("Listen Socket��ʼ��ʧ�ܣ�\n"));
		this->DeInitialize();
		return false;
	}
	else
	{
		OutputDebugString(_T("Listen Socket��ʼ�����."));
	}

	OutputDebugString(_T("ϵͳ׼���������Ⱥ�����....\n"));
	
	return true;
}

/*
 * ��ʼ����ϵͳ�˳���Ϣ���˳���ɶ˿ں��߳���Դ
 * Worker�߳�һ��������GetQueuedCompletionStatus()�Ľ׶Σ��ͻ����˯��״̬��INFINITE�ĵȴ���ɶ˿��У�
 * �����ɶ˿���һֱ��û���Ѿ���ɵ�I/O������ô��Щ�߳̽��޷������ѣ���Ҳ��ζ���߳�û�������˳���
 * ������߳�˯�ߵ�ʱ��ֱ�Ӱ��̹߳رյ��Ļ����ܶ��߳����ںܶ���Դ���������ͷŵ�
 * ȷ��ÿһ��Worker�߳����þ��յ�һ��PostQueuedCompletionStatus(), ֻ����һ��GetQueuedCompletionStatus()��
 * Ȼ�����е��̶߳������˳�
 */
void CIOCPServer::Stop()
{
	if (m_pListenContext != nullptr && m_pListenContext->m_socket != INVALID_SOCKET)
	{
		// ����ر���Ϣ֪ͨ[������һ���˳���Event�����˳���ʱ��SetEventһ�£���ȷ��Worker�߳�ÿ�ξ�ֻ�����һ�� GetQueuedCompletionStatus()���Ƚϰ�ȫ]
		::SetEvent(m_hShutdownEvent);

		for (int i = 0; i < m_nThreads; ++i)
		{
			/*
			 * ֪ͨ���е���ɶ˿ڲ����˳�
			 * �ֶ������һ����ɶ˿�I/O����������˯�ߵȴ���״̬���߳̾ͻ���һ�������ѣ�
			 * ���Ϊ����ÿһ��Worker�̶߳�����һ��PostQueuedCompletionStatus()�Ļ�����ô���е��߳�Ҳ�ͻ���˶��������ˡ�
			 *
			 *    BOOL WINAPI PostQueuedCompletionStatus(
					  __in      HANDLE CompletionPort,
					  __in      DWORD dwNumberOfBytesTransferred,
					  __in      ULONG_PTR dwCompletionKey,
					  __in_opt  LPOVERLAPPED lpOverlapped
			 * );
			 *
			 * ��������£�GetQueuedCompletionStatus()��ȡ�����Ĳ���������Ӧ����ϵͳ���������ģ�
			 * �������ڰ���ɶ˿�ʱ���еģ�
			 * ������������ȴ����ֱ��ʹ��PostQueuedCompletionStatus()ֱ�ӽ����������������ݸ�GetQueuedCompletionStatus()
			 * ֪ͨ�߳��˳�Լ����dwCompletionKey��������ΪNULL(��������£���������Ƿ�NULL��ָ��)
			 */
			::PostQueuedCompletionStatus(m_hIOCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
		}

		// �ȴ����еĿͻ�����Դ�˳�
		::WaitForMultipleObjects(m_nThreads, m_phWorkerThreads, TRUE, INFINITE);

		// ����ͻ����б���Ϣ
		this->ClearContextList();

		// �ͷ�������Դ
		this->DeInitialize();

		OutputDebugString("ֹͣ����\n");
	}
}

/************************************************************************/
/*                     ϵͳ��ʼ������ֹ                                 */
/************************************************************************/

// ��ʼ��WinSock 2.2
bool CIOCPServer::LoadSocketLib()
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

/************************************************************************/
/*                        ����������������                              */
/************************************************************************/

// ��ñ�����IP��ַ
std::string CIOCPServer::GetLocalIP()
{
	// ��ñ���������
	char hostname[MAX_PATH] = { 0 };
	::gethostname(hostname, MAX_PATH);
	struct hostent FAR* lpHostEnt = ::gethostbyname(hostname);
	if (lpHostEnt == nullptr)
	{
		return IOCP_DEFAULT_IP;
	}

	// ȡ��IP��ַ�б��еĵ�һ��Ϊ���ص�IP(��Ϊһ̨�������ܻ�󶨶��IP)
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];

	// ��IP��ַת�����ַ�����ʽ
	struct in_addr inAddr;
	memmove(&inAddr, lpAddr, 4);
	m_strIP = std::string(::inet_ntoa(inAddr));

	return m_strIP;
}

// ��ʼ����ɶ˿�
bool CIOCPServer::InitializeIOCP()
{
	/*
	 * (1) ���� CreateIoCompletionPort() ��������һ����ɶ˿�, һ������½���һ��
	 * CompetionPort: һЩ����Э����������I/O�Ķ���
	 *
	 *      HANDLE WINAPI CreateIoCompletionPort(
		__in      HANDLE  FileHandle,             // ���ﵱȻ�����������׽��־����
		 __in_opt  HANDLE  ExistingCompletionPort, // �������ǰ�洴�����Ǹ���ɶ˿�
		 __in      ULONG_PTR CompletionKey,        // ������������������̲߳���һ������
												   // �󶨵�ʱ����Լ�����Ľṹ��ָ�봫��
												   // ��������Worker�߳��У�Ҳ����ʹ�����
												   // �ṹ��������ˣ��൱�ڲ����Ĵ���
		 __in      DWORD NumberOfConcurrentThreads // ����ͬ����0
	 * );
	 * NumberOfConcurrentThreads: ����Ӧ�ó���ͬʱִ�е��߳�����, �����������л����������״̬����ÿ����������ֻ����һ���߳��ˣ�
	 *	������������Ϊ0������˵�ж��ٸ���������������ͬʱ���ٸ��߳����С�
	 *
	 */
	m_hIOCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (nullptr == m_hIOCompletionPort)
	{
		output_debug(_T("������ɶ˿�ʧ�ܣ��������: %d!\n"), WSAGetLastError());
		return false;
	}

	//  (2) ����ϵͳ���ж��ٸ����������ͽ������ٸ�Worker�߳� (ר�������Ϳͻ��˽���ͨ�ŵ�)

	// ���ݱ����еĴ�����������������Ӧ���߳���
	m_nThreads = WORKER_THREADS_PER_PROCESSOR * GetNoOfProcessors();

	// Ϊ�������̳߳�ʼ�����
	m_phWorkerThreads = new HANDLE[m_nThreads];

	// ���ݼ�����������������������߳�
	DWORD nThreadID;
	for (int i = 0; i < m_nThreads; ++i)
	{
		THREADPARAMS_WORKER* pThreadParams = new THREADPARAMS_WORKER;
		pThreadParams->pIOCPServer = this;
		pThreadParams->nThreadNo = i + 1;
		m_phWorkerThreads[i] = ::CreateThread(0, 0, _WorkerThread, (void*)pThreadParams, 0, &nThreadID);
	}

	output_debug(" ���� _WorkerThread %d ��.\n", m_nThreads);

	return true;
}

/*
 * ��ʼ��Socket
 * (3) ������ǽ��������Socket����
 * ����ʵ�ַ�ʽ��һ�Ǻͱ�ı��ģ��һ��������Ҫ����һ���������̣߳�ר������accept�ͻ��˵��������󣻶��������ܸ��߸��õ��첽AcceptEx()����
 */
bool CIOCPServer::InitializeListenSocket()
{
	// AcceptEx �� GetAcceptExSockaddrs ��GUID�����ڵ�������ָ��
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	GUID guidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

	// ��������ַ��Ϣ�����ڰ�Socket
	struct sockaddr_in serverAddress;

	// �������ڼ�����Socket����Ϣ
	m_pListenContext = new PER_SOCKET_CONTEXT;

	// ��Ҫʹ���ص�IO�������ʹ��WSASocket������Socket���ſ���֧���ص�IO����
	m_pListenContext->m_socket = ::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_pListenContext->m_socket)
	{
		output_debug("��ʼ��Socketʧ�ܣ��������: %d.\n", WSAGetLastError());
		return false;
	}
	else
	{
		OutputDebugString("WSASocket() ���.\n");
	}

	// ����ַ��Ϣ
	::ZeroMemory((char*)&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	// ������԰��κο��õ�IP��ַ�����߰�һ��ָ����IP��ַ 
	serverAddress.sin_addr.S_un.S_addr = ::inet_addr(m_strIP.c_str());
	serverAddress.sin_port = ::htons(m_nPort);

	// �󶨵�ַ�Ͷ˿�
	if (SOCKET_ERROR == ::bind(m_pListenContext->m_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)))
	{
		OutputDebugString("bind()����ִ�д���.\n");
		return false;
	}
	else
	{
		OutputDebugString("bind() ���.\n");
	}

	/*
	 * ��ȡAcceptEx����ָ��
	 *
	 *
	 * LPFN_ACCEPTEX     m_lpfnAcceptEx;         // AcceptEx����ָ��
	 * GUID GuidAcceptEx = WSAID_ACCEPTEX;        // GUID�������ʶ��AcceptEx���������
	 * DWORD dwBytes = 0;

		WSAIoctl(
			m_pListenContext->m_Socket,	// ���⴫�ݸ�WSAIoctl()һ����Ч��SOCKET���ɣ���Socket�����Ͳ���Ӱ���ȡ��AcceptEx����ָ��
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidAcceptEx,
			sizeof(GuidAcceptEx),
			&m_lpfnAcceptEx,
			sizeof(m_lpfnAcceptEx),
			&dwBytes,
			NULL,
			NULL);
	 *
	 */
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == ::WSAIoctl(m_pListenContext->m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(guidAcceptEx), &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx), &dwBytes, NULL, NULL))
	{
		output_debug("WSAIoctl δ�ܻ�ȡAcceptEx����ָ�롣�������: %d\n", WSAGetLastError());
		DeInitialize();
		return false;
	}

	// ��ȡGetAcceptExSockAddrs����ָ��
	if (SOCKET_ERROR == ::WSAIoctl(m_pListenContext->m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidGetAcceptExSockAddrs, sizeof(guidGetAcceptExSockAddrs), &m_lpfnGetAcceptExSockAddrs, sizeof(m_lpfnGetAcceptExSockAddrs), &dwBytes, NULL, NULL))
	{
		output_debug("WSAIoctl δ�ܻ�ȡGuidGetAcceptExSockAddrs����ָ�롣�������: %d\n", WSAGetLastError());
		DeInitialize();
		return false;
	}

	// ΪAcceptEx ׼��������Ȼ��Ͷ��AcceptEx I/O ����
	for (int i = 0; i < MAX_POST_ACCEPT; ++i)
	{
		// �½�һ��IO_CONTEXT
		PER_IO_CONTEXT* pAcceptIoContext = m_pListenContext->GetNewIoContext();

		if (false == this->PostAccept(pAcceptIoContext))
		{
			m_pListenContext->RemoveContext(pAcceptIoContext);
			return false;
		}
	}

	output_debug(_T("Ͷ�� %d ��AcceptEx�������"), MAX_POST_ACCEPT);

	return true;
}

// ����ͷŵ�������Դ
void CIOCPServer::DeInitialize()
{
	// ɾ���ͻ����б�Ļ�����
	::DeleteCriticalSection(&m_csContextList);

	// �ر�ϵͳ�˳��¼����
	RELEASE_HANDLE(m_hShutdownEvent);

	// �ͷŹ������߳̾��ָ��
	for (int i = 0; i < m_nThreads; ++i)
	{
		RELEASE_HANDLE(m_phWorkerThreads[i]);
	}

	RELEASE_ARRAY(m_phWorkerThreads);

	// �ر�IOCP���
	RELEASE_HANDLE(m_hIOCompletionPort);

	// �رռ���Socket
	RELEASE(m_pListenContext);

	OutputDebugString("�ͷ���Դ���.\n");
}

/************************************************************************/
/*                       Ͷ����ɶ˿�����                               */
/************************************************************************/

// Ͷ��Accept����
bool CIOCPServer::PostAccept(PER_IO_CONTEXT* pAcceptIoContext)
{
	assert(pAcceptIoContext != nullptr);
	assert(INVALID_SOCKET != m_pListenContext->m_socket);

	// ׼������
	DWORD dwBytes = 0;
	pAcceptIoContext->m_opType = ACCEPT_POSTED;
	WSABUF* p_wbuf = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED* p_ol = &pAcceptIoContext->m_overlapped;

	// Ϊ�Ժ�������Ŀͻ�����׼����Socket( ������봫ͳaccept�������� )
	pAcceptIoContext->m_sockAccept = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == pAcceptIoContext->m_sockAccept)
	{
		output_debug("��������Accept��Socketʧ�ܣ��������: %d", WSAGetLastError());
		return false;
	}

	/*
	 * Ͷ��AcceptEx
	 *    BOOL AcceptEx (
				  SOCKET sListenSocket,
				  SOCKET sAcceptSocket,
				  PVOID lpOutputBuffer,
				  DWORD dwReceiveDataLength,
				  DWORD dwLocalAddressLength,
				  DWORD dwRemoteAddressLength,
				  LPDWORD lpdwBytesReceived,
				  LPOVERLAPPED lpOverlapped
	 * );
	 *
	 * ����1--sListenSocket, ��������Ǹ�Ψһ������������Socket
	 * ����2--sAcceptSocket, ���ڽ������ӵ�socket������Ҫ�Ƚ��ã����пͻ������ӽ�����Socket�ø����õģ���AcceptEx�����ܵĹؼ����ڡ�
	 * ����3--lpOutputBuffer, ���ջ�����. ������������Ϣ��һ�ǿͻ��˷����ĵ�һ�����ݣ�����server�ĵ�ַ������client��ַ��
	 * ����4--dwReceiveDataLength������lpOutputBuffer�����ڴ�����ݵĿռ��С������˲���=0����Acceptʱ����������ݵ�������ֱ�ӷ���;
	 *	����˲�����Ϊ0����ôһ���õȽ��յ������˲Ż᷵��. 
	 *	����ͨ������ҪAccept��������ʱ������Ҫ���ò������Ϊ��sizeof(lpOutputBuffer) - 2*(sizeof sockaddr_in +16)�����ܳ��ȼ�ȥ������ַ�ռ�ĳ���
	 * ����5--dwLocalAddressLength����ű���ַ��ַ��Ϣ�Ŀռ��С��
	 * ����6--dwRemoteAddressLength�����Զ�˵�ַ��Ϣ�Ŀռ��С��
	 * ����7--lpdwBytesReceived��out����
	 * ����8--lpOverlapped�������ص�I/O��Ҫ�õ����ص��ṹ��
	 */
	if (FALSE == m_lpfnAcceptEx(m_pListenContext->m_socket, pAcceptIoContext->m_sockAccept, p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN) + 16) * 2), sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, p_ol))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			output_debug("Ͷ�� AcceptEx ����ʧ�ܣ��������: %d", WSAGetLastError());
			return false;
		}
	}

	return true;
}

// Ͷ�ݽ�����������
bool CIOCPServer::PostRecv(PER_IO_CONTEXT* pIoContext)
{
	assert(pIoContext != nullptr);

	// ��ʼ������
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF *p_wbuf = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->m_overlapped;

	pIoContext->ResetBuffer();
	pIoContext->m_opType = RECV_POSTED;

	// ��ʼ����ɺ�Ͷ��WSARecv����
	int nBytesRecv = ::WSARecv(pIoContext->m_sockAccept, p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL);

	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		OutputDebugString("Ͷ�ݵ�һ��WSARecvʧ�ܣ�");
		return false;
	}

	return true;
}

/*
 * ���пͻ��������ʱ�򣬽��д���
 * �������ListenSocket��Context������һ�ݳ������������Socket��
 * ԭ����Context����Ҫ���������Ͷ����һ��Accept����
 */
bool CIOCPServer::DoAccept(PER_SOCKET_CONTEXT* pSocketcontext, PER_IO_CONTEXT* pIoContext)
{
	SOCKADDR_IN* clientAddr = nullptr;
	SOCKADDR_IN* localAddr = nullptr;
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);

	/*
	 * 1. ����ȡ������ͻ��˵ĵ�ַ��Ϣ
	 * m_lpfnGetAcceptExSockAddrs ��ȡ�ÿͻ��˺ͱ��ض˵ĵ�ַ��Ϣ������ȡ���ͻ��˷����ĵ�һ������
	 */
	this->m_lpfnGetAcceptExSockAddrs(pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2), sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&localAddr, &localLen, (LPSOCKADDR*)&clientAddr, &remoteLen);

	// �ͻ���IP��ַ �ͻ�������Ķ˿� �ͻ��˷�����һ�����ݵĻ�����
	output_debug(_T("�ͻ��� [%s]:[%d] ��Ϣ��[%s]."), inet_ntoa(clientAddr->sin_addr), ntohs(clientAddr->sin_port), pIoContext->m_wsaBuf.buf);
	// ����IP��ַ ����ͨѶ�Ķ˿�
	output_debug(_T("���� [%s]:[%d]"), inet_ntoa(localAddr->sin_addr), ntohs(localAddr->sin_port));

	/*
	 * 2. ���ﴫ��������ListenSocket�ϵ�Context�����Context����Ҫ���ڼ�����һ������
	 * ���Ի�Ҫ��ListenSocket�ϵ�Context���Ƴ���һ��Ϊ�������Socket�½�һ��SocketContext
	 */
	PER_SOCKET_CONTEXT* pNewSocketContext = new PER_SOCKET_CONTEXT;
	pNewSocketContext->m_socket = pIoContext->m_sockAccept;
	memcpy(&(pNewSocketContext->m_clientAddr), clientAddr, sizeof(SOCKADDR_IN));

	// ����������ϣ������Socket����ɶ˿ڰ�
	if (false == this->AssociateWithIOCP(pNewSocketContext))
	{
		RELEASE(pNewSocketContext);
		return false;
	}

	// 3. �������������µ�IoContext�����������Socket��Ͷ�ݵ�һ��Recv��������
	PER_IO_CONTEXT* pNewIoContext = pNewSocketContext->GetNewIoContext();
	pNewIoContext->m_opType = RECV_POSTED;
	pNewIoContext->m_sockAccept = pNewSocketContext->m_socket;
	// ���Buffer��Ҫ�������Ϳ���һ�ݳ���
	//memcpy( pNewIoContext->m_szBuffer,pIoContext->m_szBuffer,IOCP_MAX_BUFFER_LEN );

	// �����֮�󣬾Ϳ��Կ�ʼ�����Socket��Ͷ�����������
	if (false == this->PostRecv(pNewIoContext))
	{
		pNewSocketContext->RemoveContext(pNewIoContext);
		return false;
	}

	// 4. ���Ͷ�ݳɹ�����ô�Ͱ������Ч�Ŀͻ�����Ϣ�����뵽ContextList��ȥ(��Ҫͳһ���������ͷ���Դ)
	this->AddToContextList(pNewSocketContext);

	// 5. ʹ�����֮�󣬰�Listen Socket���Ǹ�IoContext���ã�Ȼ��׼��Ͷ���µ�AcceptEx
	pIoContext->ResetBuffer();

	return this->PostAccept(pIoContext);
}

// ���н��յ����ݵ����ʱ�򣬽��д���
bool CIOCPServer::DoRecv(PER_SOCKET_CONTEXT* pSocketcontext, PER_IO_CONTEXT* pIoContext)
{
	assert(pSocketcontext != nullptr);
	assert(pIoContext != nullptr);

	// �Ȱ���һ�ε�������ʾ���֣�Ȼ�������״̬��������һ��Recv����
	SOCKADDR_IN *clientAddr = &pSocketcontext->m_clientAddr;
	output_debug(_T("�յ�  %s:%d ��Ϣ��%s"), inet_ntoa(clientAddr->sin_addr), ntohs(clientAddr->sin_port), pIoContext->m_wsaBuf.buf);

	// Ȼ��ʼͶ����һ��WSARecv����
	return this->PostRecv(pIoContext);
}

/************************************************************************/
/*                    ContextList ��ز���                              */
/************************************************************************/

// ���ͻ��˵������Ϣ�洢��������
void CIOCPServer::AddToContextList(PER_SOCKET_CONTEXT* pSocketContext)
{
	::EnterCriticalSection(&m_csContextList);

	m_arrayClientContext.push_back(pSocketContext);

	::LeaveCriticalSection(&m_csContextList);
}

// �Ƴ�ĳ���ض���Context
void CIOCPServer::RemoveContext(PER_SOCKET_CONTEXT* pSocketContext)
{
	::EnterCriticalSection(&m_csContextList);

	for (std::vector<PER_SOCKET_CONTEXT*>::iterator itr = m_arrayClientContext.begin(); itr != m_arrayClientContext.end(); ++itr)
	{
		if (pSocketContext == *itr)
		{
			RELEASE(pSocketContext);
			m_arrayClientContext.erase(itr);
			break;
		}
	}

	::LeaveCriticalSection(&m_csContextList);
}

// ��տͻ�����Ϣ
void CIOCPServer::ClearContextList()
{
	::EnterCriticalSection(&m_csContextList);

	for (int i = 0; i < m_arrayClientContext.size(); ++i)
	{
		RELEASE(m_arrayClientContext[i]);
	}

	m_arrayClientContext.clear();

	::LeaveCriticalSection(&m_csContextList);
}

// �����(Socket)�󶨵���ɶ˿���
bool CIOCPServer::AssociateWithIOCP(PER_SOCKET_CONTEXT* pContext)
{
	assert(pContext!= nullptr);

	// �����ںͿͻ���ͨ�ŵ�SOCKET�󶨵���ɶ˿���
	HANDLE hTemp = ::CreateIoCompletionPort((HANDLE)pContext->m_socket, m_hIOCompletionPort, (DWORD)pContext, 0);

	if (NULL == hTemp)
	{
		output_debug("ִ��CreateIoCompletionPort()���ִ���.������룺%d", GetLastError());
		return false;
	}

	return true;
}

// ��ʾ��������ɶ˿��ϵĴ���
bool CIOCPServer::HandleError(PER_SOCKET_CONTEXT* pContext, const DWORD dwErr)
{
	assert(pContext != nullptr);

	// ����ǳ�ʱ�ˣ����ټ����Ȱ� 
	if (WAIT_TIMEOUT == dwErr)
	{
		// ȷ�Ͽͻ����Ƿ񻹻���...
		if (!IsSocketAlive(pContext->m_socket))
		{
			OutputDebugString(_T("��⵽�ͻ����쳣�˳���"));
			this->RemoveContext(pContext);
			return true;
		}
		else
		{
			OutputDebugString(_T("���������ʱ��������..."));
			return true;
		}
	}
	else if (ERROR_NETNAME_DELETED == dwErr)	// �����ǿͻ����쳣�˳���
	{
		OutputDebugString(_T("��⵽�ͻ����쳣�˳���"));
		this->RemoveContext(pContext);
		return true;
	}
	else
	{
		output_debug(_T("��ɶ˿ڲ������ִ����߳��˳���������룺%d"), dwErr);
		return false;
	}
}

/*
 * �������߳�: ΪIOCP�������Ĺ������߳�
 *		Ҳ����ÿ����ɶ˿��ϳ�����������ݰ����ͽ�֮ȡ�������д�����߳�
 */
DWORD WINAPI CIOCPServer::_WorkerThread(LPVOID lpParam)
{
	THREADPARAMS_WORKER* pParam = (THREADPARAMS_WORKER*)lpParam;
	assert(pParam != nullptr);
	CIOCPServer* pIOCPServer = (CIOCPServer*)pParam->pIOCPServer;
	assert(pIOCPServer != nullptr);
	int nThreadNo = (int)pParam->nThreadNo;

	// �������߳�����

	OVERLAPPED* pOverlapped = nullptr;
	PER_SOCKET_CONTEXT* pSocketContext = nullptr;
	DWORD dwBytesTransfered = 0;

	// ѭ����������֪�����յ�Shutdown��ϢΪֹ
	while (WAIT_OBJECT_0 != ::WaitForSingleObject(pIOCPServer->m_hShutdownEvent, 0))
	{
		/*
		 * ����Worker�߳̽��벻ռ��CPU��˯��״̬��ֱ����ɶ˿��ϳ�������Ҫ���������������߳����˵ȴ���ʱ������Ϊֹ��
		 * һ����ɶ˿��ϳ���������ɵ�I/O����
		 * ��ô�ȴ����̻߳ᱻ���̻���[�����ȳ��ķ�ʽ, �����������ֻ��һ��I/O���������Ƕ��������ɵĻ���
		 * �ں˾�ֻ��Ҫ����ͬһ���߳̾Ϳ����ˣ�������Ҫ���Ż��Ѷ���̣߳���Լ����Դ��
		 * ���ҿ��԰�������ʱ��˯�ߵ��̻߳����ڴ棬�ᵽ��Դ�����ʡ�]��
		 * (���ʹ�����ݵ�ʱ������Ҫע��������Ⱥ����)
		 * Ȼ�����ִ�к����Ĵ��롣
		 *
		 * BOOL WINAPI GetQueuedCompletionStatus(
		 * __in   HANDLE          CompletionPort,    // ������Ψһ����ɶ˿�
		 * __out  LPDWORD         lpNumberOfBytes,   // ������ɺ󷵻ص��ֽ���
		 * __out  PULONG_PTR      lpCompletionKey,   // ������ɶ˿ڵ�ʱ��󶨵��Զ���ṹ�����
		 * __out  LPOVERLAPPED    *lpOverlapped,     // ������Socket��ʱ��һ�������ص��ṹ, ����ʹ���PER_IO_CONTEXT (Ͷ��AcceptEx���󴫵��ص�����)
		 * __in   DWORD           dwMilliseconds     // �ȴ���ɶ˿ڵĳ�ʱʱ�䣬����̲߳���Ҫ�����������飬�Ǿ�INFINITE������
		 * );

		 */
		BOOL bReturn = ::GetQueuedCompletionStatus(pIOCPServer->m_hIOCompletionPort, &dwBytesTransfered, (PULONG_PTR)&pSocketContext, &pOverlapped, INFINITE);
		// ����յ������˳���־����ֱ���˳�
		if (EXIT_CODE == (DWORD)pSocketContext)
		{
			break;
		}

		// �ж��Ƿ�����˴���
		if (!bReturn)
		{
			DWORD dwErr = ::GetLastError();

			// ��ʾһ����ʾ��Ϣ
			if (!pIOCPServer->HandleError(pSocketContext, dwErr))
			{
				break;
			}

			continue;
		}
		else
		{
			/*
			 * ��ȡ����Ĳ���
			 * ȥ�����lpOverlapped������ҵ��ͽṹ����PER_IO_CONTEXT��m_overlapped��Ա��ص����ݡ�
			 * ȷ���ڽṹ��PER_IO_CONTEXT�����ʱ�򣬰�Overlapped����������Ϊ�ṹ���еĵ�һ����Ա��
			 */
			PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, PER_IO_CONTEXT, m_overlapped);
			assert(pIoContext != nullptr);

			// �ж��Ƿ��пͻ��˶Ͽ�
			if ((0 == dwBytesTransfered) && (RECV_POSTED == pIoContext->m_opType || SEND_POSTED == pIoContext->m_opType))
			{
				// �ͻ��˶Ͽ�����
				::inet_ntoa(pSocketContext->m_clientAddr.sin_addr);
				::ntohs(pSocketContext->m_clientAddr.sin_port);

				// �ͷŵ���Ӧ����Դ
				pIOCPServer->RemoveContext(pSocketContext);

				continue;
			}
			else
			{
				switch (pIoContext->m_opType)
				{
					// Accept
				case ACCEPT_POSTED:
				{
					pIOCPServer->DoAccept(pSocketContext, pIoContext);
				}
				break;
				// RECV
				case RECV_POSTED:
				{
					pIOCPServer->DoRecv(pSocketContext, pIoContext);
				}
				break;
				// SEND
				case SEND_POSTED:
				{

				}
				break;
				default:
				{
					OutputDebugString(_T("_WorkThread�е� pIoContext->m_OpType �����쳣.\n"));
				}
					break;
				}
			}
		}
	}

	output_debug(_T("�������߳� %d ���˳�.\n"), nThreadNo);

	// �ͷ��̲߳���
	RELEASE(lpParam);

	return 0;
}

// ��ñ����д�����������
int CIOCPServer::GetNoOfProcessors()
{
	SYSTEM_INFO si;

	::GetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}

/*
 * �жϿͻ���Socket�Ƿ��Ѿ��Ͽ���������һ����Ч��Socket��Ͷ��WSARecv����������쳣
 * ʹ�õķ����ǳ��������socket�������ݣ��ж����socket���õķ���ֵ
 * ��Ϊ����ͻ��������쳣�Ͽ�(����ͻ��˱������߰ε����ߵ�)��ʱ�򣬷����������޷��յ��ͻ��˶Ͽ���֪ͨ��
 */
bool CIOCPServer::IsSocketAlive(SOCKET s)
{
	int nByteSent = ::send(s, "", 0, 0);
	if (-1 == nByteSent)
		return false;

	return true;
}
