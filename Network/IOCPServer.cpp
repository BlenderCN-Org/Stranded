#include "IOCPServer.h"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "../lib/Common.lib")
#endif

/*
 * 每一个处理器上产生多少个线程 (为了最大限度的提升服务器性能).
 * 最好是建立CPU核心数量 * 2那么多的线程，充分利用CPU资源，因为完成端口的调度是非常智能的.
 * Worker线程有的时候可能会有Sleep()或者WaitForSingleObject()之类的情况，
 * 同一个CPU核心上的另一个线程就可以代替这个Sleep的线程执行了；因为完成端口的目标是要使得CPU满负荷的工作。
 */
#define WORKER_THREADS_PER_PROCESSOR	2
// 同时投递的Accept请求的数量(这个要根据实际的情况灵活设置)
#define MAX_POST_ACCEPT	10
// 传递给Worker线程的退出信号
#define EXIT_CODE	NULL

// 释放指针和句柄资源的宏

// 释放指针宏
#define RELEASE(x)			{if(x != NULL) {delete x; x=NULL;}}
// 释放指针
#define RELEASE_ARRAY(x)	{if(x != NULL) {delete[] x, x = NULL;}}
// 释放句柄宏
#define RELEASE_HANDLE(x)	{if(x != NULL && x != INVALID_HANDLE_VALUE) { ::CloseHandle(x); x = NULL; }}
// 释放Socket宏
#define RELEASE_SOCKET(x)	{if(x != INVALID_SOCKET) { ::closesocket(x); x = INVALID_SOCKET; }}

CIOCPServer::CIOCPServer() : m_nThreads(0), m_hShutdownEvent(NULL), m_hIOCompletionPort(NULL),
							m_phWorkerThreads(NULL), m_strIP(IOCP_DEFAULT_IP), m_nPort(IOCP_DEFAULT_PORT),
							m_lpfnAcceptEx(NULL), m_pListenContext(NULL), m_lpfnGetAcceptExSockAddrs(NULL)
{

}

CIOCPServer::~CIOCPServer()
{
	// 确保资源彻底释放
	this->Stop();
}

// 启动服务器
bool CIOCPServer::Start()
{
	// 初始化线程互斥量
	::InitializeCriticalSection(&m_csContextList);

	// 建立系统退出的事件通知
	m_hShutdownEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	// 初始化IOCP
	if (false == InitializeIOCP())
	{
		OutputDebugString(_T("初始化IOCP失败！\n"));
		return false;
	}
	else
	{
		OutputDebugString("\nIOCP初始化完毕\n.");
	}

	// 初始化Socket
	if (false == InitializeListenSocket())
	{
		OutputDebugString(_T("Listen Socket初始化失败！\n"));
		this->DeInitialize();
		return false;
	}
	else
	{
		OutputDebugString(_T("Listen Socket初始化完毕."));
	}

	OutputDebugString(_T("系统准备就绪，等候连接....\n"));
	
	return true;
}

/*
 * 开始发送系统退出消息，退出完成端口和线程资源
 * Worker线程一旦进入了GetQueuedCompletionStatus()的阶段，就会进入睡眠状态，INFINITE的等待完成端口中，
 * 如果完成端口上一直都没有已经完成的I/O请求，那么这些线程将无法被唤醒，这也意味着线程没法正常退出。
 * 如果在线程睡眠的时候，直接把线程关闭掉的话，很多线程体内很多资源都来不及释放掉
 * 确保每一个Worker线程正好就收到一个PostQueuedCompletionStatus(), 只调用一次GetQueuedCompletionStatus()，
 * 然后所有的线程都正好退出
 */
void CIOCPServer::Stop()
{
	if (m_pListenContext != nullptr && m_pListenContext->m_socket != INVALID_SOCKET)
	{
		// 激活关闭消息通知[搭配了一个退出的Event，在退出的时候SetEvent一下，来确保Worker线程每次就只会调用一轮 GetQueuedCompletionStatus()，比较安全]
		::SetEvent(m_hShutdownEvent);

		for (int i = 0; i < m_nThreads; ++i)
		{
			/*
			 * 通知所有的完成端口操作退出
			 * 手动的添加一个完成端口I/O操作，处于睡眠等待的状态的线程就会有一个被唤醒，
			 * 如果为我们每一个Worker线程都调用一次PostQueuedCompletionStatus()的话，那么所有的线程也就会因此而被唤醒了。
			 *
			 *    BOOL WINAPI PostQueuedCompletionStatus(
					  __in      HANDLE CompletionPort,
					  __in      DWORD dwNumberOfBytesTransferred,
					  __in      ULONG_PTR dwCompletionKey,
					  __in_opt  LPOVERLAPPED lpOverlapped
			 * );
			 *
			 * 正常情况下，GetQueuedCompletionStatus()获取回来的参数本来是应该是系统帮我们填充的，
			 * 或者是在绑定完成端口时就有的，
			 * 但是我们这里却可以直接使用PostQueuedCompletionStatus()直接将后面三个参数传递给GetQueuedCompletionStatus()
			 * 通知线程退出约定：dwCompletionKey参数设置为NULL(正常情况下，这个参数是非NULL的指针)
			 */
			::PostQueuedCompletionStatus(m_hIOCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
		}

		// 等待所有的客户端资源退出
		::WaitForMultipleObjects(m_nThreads, m_phWorkerThreads, TRUE, INFINITE);

		// 清除客户端列表信息
		this->ClearContextList();

		// 释放其他资源
		this->DeInitialize();

		OutputDebugString("停止监听\n");
	}
}

/************************************************************************/
/*                     系统初始化和终止                                 */
/************************************************************************/

// 初始化WinSock 2.2
bool CIOCPServer::LoadSocketLib()
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

/************************************************************************/
/*                        其他辅助函数定义                              */
/************************************************************************/

// 获得本机的IP地址
std::string CIOCPServer::GetLocalIP()
{
	// 获得本机主机名
	char hostname[MAX_PATH] = { 0 };
	::gethostname(hostname, MAX_PATH);
	struct hostent FAR* lpHostEnt = ::gethostbyname(hostname);
	if (lpHostEnt == nullptr)
	{
		return IOCP_DEFAULT_IP;
	}

	// 取得IP地址列表中的第一个为返回的IP(因为一台主机可能会绑定多个IP)
	LPSTR lpAddr = lpHostEnt->h_addr_list[0];

	// 将IP地址转化成字符串形式
	struct in_addr inAddr;
	memmove(&inAddr, lpAddr, 4);
	m_strIP = std::string(::inet_ntoa(inAddr));

	return m_strIP;
}

// 初始化完成端口
bool CIOCPServer::InitializeIOCP()
{
	/*
	 * (1) 调用 CreateIoCompletionPort() 函数创建一个完成端口, 一般情况下仅需一个
	 * CompetionPort: 一些用来协调各种网络I/O的队列
	 *
	 *      HANDLE WINAPI CreateIoCompletionPort(
		__in      HANDLE  FileHandle,             // 这里当然是连入的这个套接字句柄了
		 __in_opt  HANDLE  ExistingCompletionPort, // 这个就是前面创建的那个完成端口
		 __in      ULONG_PTR CompletionKey,        // 这个参数就是类似于线程参数一样，在
												   // 绑定的时候把自己定义的结构体指针传递
												   // 这样到了Worker线程中，也可以使用这个
												   // 结构体的数据了，相当于参数的传递
		 __in      DWORD NumberOfConcurrentThreads // 这里同样置0
	 * );
	 * NumberOfConcurrentThreads: 允许应用程序同时执行的线程数量, 避免上下文切换，最理想的状态就是每个处理器上只运行一个线程了，
	 *	所以我们设置为0，就是说有多少个处理器，就允许同时多少个线程运行。
	 *
	 */
	m_hIOCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (nullptr == m_hIOCompletionPort)
	{
		output_debug(_T("建立完成端口失败！错误代码: %d!\n"), WSAGetLastError());
		return false;
	}

	//  (2) 根据系统中有多少个处理器，就建立多少个Worker线程 (专门用来和客户端进行通信的)

	// 根据本机中的处理器数量，建立对应的线程数
	m_nThreads = WORKER_THREADS_PER_PROCESSOR * GetNoOfProcessors();

	// 为工作者线程初始化句柄
	m_phWorkerThreads = new HANDLE[m_nThreads];

	// 根据计算出来的数量建立工作者线程
	DWORD nThreadID;
	for (int i = 0; i < m_nThreads; ++i)
	{
		THREADPARAMS_WORKER* pThreadParams = new THREADPARAMS_WORKER;
		pThreadParams->pIOCPServer = this;
		pThreadParams->nThreadNo = i + 1;
		m_phWorkerThreads[i] = ::CreateThread(0, 0, _WorkerThread, (void*)pThreadParams, 0, &nThreadID);
	}

	output_debug(" 建立 _WorkerThread %d 个.\n", m_nThreads);

	return true;
}

/*
 * 初始化Socket
 * (3) 下面就是接收连入的Socket连接
 * 两种实现方式：一是和别的编程模型一样，还需要启动一个独立的线程，专门用来accept客户端的连接请求；二是用性能更高更好的异步AcceptEx()请求
 */
bool CIOCPServer::InitializeListenSocket()
{
	// AcceptEx 和 GetAcceptExSockaddrs 的GUID，用于导出函数指针
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	GUID guidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

	// 服务器地址信息，用于绑定Socket
	struct sockaddr_in serverAddress;

	// 生成用于监听的Socket的信息
	m_pListenContext = new PER_SOCKET_CONTEXT;

	// 需要使用重叠IO，必须得使用WSASocket来建立Socket，才可以支持重叠IO操作
	m_pListenContext->m_socket = ::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_pListenContext->m_socket)
	{
		output_debug("初始化Socket失败，错误代码: %d.\n", WSAGetLastError());
		return false;
	}
	else
	{
		OutputDebugString("WSASocket() 完成.\n");
	}

	// 填充地址信息
	::ZeroMemory((char*)&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	// 这里可以绑定任何可用的IP地址，或者绑定一个指定的IP地址 
	serverAddress.sin_addr.S_un.S_addr = ::inet_addr(m_strIP.c_str());
	serverAddress.sin_port = ::htons(m_nPort);

	// 绑定地址和端口
	if (SOCKET_ERROR == ::bind(m_pListenContext->m_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)))
	{
		OutputDebugString("bind()函数执行错误.\n");
		return false;
	}
	else
	{
		OutputDebugString("bind() 完成.\n");
	}

	/*
	 * 获取AcceptEx函数指针
	 *
	 *
	 * LPFN_ACCEPTEX     m_lpfnAcceptEx;         // AcceptEx函数指针
	 * GUID GuidAcceptEx = WSAID_ACCEPTEX;        // GUID，这个是识别AcceptEx函数必须的
	 * DWORD dwBytes = 0;

		WSAIoctl(
			m_pListenContext->m_Socket,	// 任意传递给WSAIoctl()一个有效的SOCKET即可，该Socket的类型不会影响获取的AcceptEx函数指针
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
		output_debug("WSAIoctl 未能获取AcceptEx函数指针。错误代码: %d\n", WSAGetLastError());
		DeInitialize();
		return false;
	}

	// 获取GetAcceptExSockAddrs函数指针
	if (SOCKET_ERROR == ::WSAIoctl(m_pListenContext->m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidGetAcceptExSockAddrs, sizeof(guidGetAcceptExSockAddrs), &m_lpfnGetAcceptExSockAddrs, sizeof(m_lpfnGetAcceptExSockAddrs), &dwBytes, NULL, NULL))
	{
		output_debug("WSAIoctl 未能获取GuidGetAcceptExSockAddrs函数指针。错误代码: %d\n", WSAGetLastError());
		DeInitialize();
		return false;
	}

	// 为AcceptEx 准备参数，然后投递AcceptEx I/O 请求
	for (int i = 0; i < MAX_POST_ACCEPT; ++i)
	{
		// 新建一个IO_CONTEXT
		PER_IO_CONTEXT* pAcceptIoContext = m_pListenContext->GetNewIoContext();

		if (false == this->PostAccept(pAcceptIoContext))
		{
			m_pListenContext->RemoveContext(pAcceptIoContext);
			return false;
		}
	}

	output_debug(_T("投递 %d 个AcceptEx请求完毕"), MAX_POST_ACCEPT);

	return true;
}

// 最后释放掉所有资源
void CIOCPServer::DeInitialize()
{
	// 删除客户端列表的互斥量
	::DeleteCriticalSection(&m_csContextList);

	// 关闭系统退出事件句柄
	RELEASE_HANDLE(m_hShutdownEvent);

	// 释放工作者线程句柄指针
	for (int i = 0; i < m_nThreads; ++i)
	{
		RELEASE_HANDLE(m_phWorkerThreads[i]);
	}

	RELEASE_ARRAY(m_phWorkerThreads);

	// 关闭IOCP句柄
	RELEASE_HANDLE(m_hIOCompletionPort);

	// 关闭监听Socket
	RELEASE(m_pListenContext);

	OutputDebugString("释放资源完毕.\n");
}

/************************************************************************/
/*                       投递完成端口请求                               */
/************************************************************************/

// 投递Accept请求
bool CIOCPServer::PostAccept(PER_IO_CONTEXT* pAcceptIoContext)
{
	assert(pAcceptIoContext != nullptr);
	assert(INVALID_SOCKET != m_pListenContext->m_socket);

	// 准备参数
	DWORD dwBytes = 0;
	pAcceptIoContext->m_opType = ACCEPT_POSTED;
	WSABUF* p_wbuf = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED* p_ol = &pAcceptIoContext->m_overlapped;

	// 为以后新连入的客户端先准备好Socket( 这个是与传统accept最大的区别 )
	pAcceptIoContext->m_sockAccept = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == pAcceptIoContext->m_sockAccept)
	{
		output_debug("创建用于Accept的Socket失败！错误代码: %d", WSAGetLastError());
		return false;
	}

	/*
	 * 投递AcceptEx
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
	 * 参数1--sListenSocket, 这个就是那个唯一的用来监听的Socket
	 * 参数2--sAcceptSocket, 用于接受连接的socket，是需要先建好，等有客户端连接进来把Socket拿给它用的，是AcceptEx高性能的关键所在。
	 * 参数3--lpOutputBuffer, 接收缓冲区. 包含了三个信息：一是客户端发来的第一组数据，二是server的地址，三是client地址。
	 * 参数4--dwReceiveDataLength，参数lpOutputBuffer中用于存放数据的空间大小。如果此参数=0，则Accept时将不会待数据到来，而直接返回;
	 *	如果此参数不为0，那么一定得等接收到数据了才会返回. 
	 *	所以通常当需要Accept接收数据时，就需要将该参数设成为：sizeof(lpOutputBuffer) - 2*(sizeof sockaddr_in +16)，即总长度减去两个地址空间的长度
	 * 参数5--dwLocalAddressLength，存放本地址地址信息的空间大小；
	 * 参数6--dwRemoteAddressLength，存放远端地址信息的空间大小；
	 * 参数7--lpdwBytesReceived，out参数
	 * 参数8--lpOverlapped，本次重叠I/O所要用到的重叠结构。
	 */
	if (FALSE == m_lpfnAcceptEx(m_pListenContext->m_socket, pAcceptIoContext->m_sockAccept, p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN) + 16) * 2), sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, p_ol))
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			output_debug("投递 AcceptEx 请求失败，错误代码: %d", WSAGetLastError());
			return false;
		}
	}

	return true;
}

// 投递接收数据请求
bool CIOCPServer::PostRecv(PER_IO_CONTEXT* pIoContext)
{
	assert(pIoContext != nullptr);

	// 初始化变量
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF *p_wbuf = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->m_overlapped;

	pIoContext->ResetBuffer();
	pIoContext->m_opType = RECV_POSTED;

	// 初始化完成后，投递WSARecv请求
	int nBytesRecv = ::WSARecv(pIoContext->m_sockAccept, p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL);

	// 如果返回值错误，并且错误的代码并非是Pending的话，那就说明这个重叠请求失败了
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		OutputDebugString("投递第一个WSARecv失败！");
		return false;
	}

	return true;
}

/*
 * 在有客户端连入的时候，进行处理
 * 传入的是ListenSocket的Context，复制一份出来给新连入的Socket用
 * 原来的Context还是要在上面继续投递下一个Accept请求
 */
bool CIOCPServer::DoAccept(PER_SOCKET_CONTEXT* pSocketcontext, PER_IO_CONTEXT* pIoContext)
{
	SOCKADDR_IN* clientAddr = nullptr;
	SOCKADDR_IN* localAddr = nullptr;
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);

	/*
	 * 1. 首先取得连入客户端的地址信息
	 * m_lpfnGetAcceptExSockAddrs 可取得客户端和本地端的地址信息，还能取出客户端发来的第一组数据
	 */
	this->m_lpfnGetAcceptExSockAddrs(pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2), sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&localAddr, &localLen, (LPSOCKADDR*)&clientAddr, &remoteLen);

	// 客户端IP地址 客户端连入的端口 客户端发来第一组数据的缓冲区
	output_debug(_T("客户端 [%s]:[%d] 信息：[%s]."), inet_ntoa(clientAddr->sin_addr), ntohs(clientAddr->sin_port), pIoContext->m_wsaBuf.buf);
	// 本地IP地址 本地通讯的端口
	output_debug(_T("本地 [%s]:[%d]"), inet_ntoa(localAddr->sin_addr), ntohs(localAddr->sin_port));

	/*
	 * 2. 这里传入的这个是ListenSocket上的Context，这个Context还需要用于监听下一个连接
	 * 所以还要将ListenSocket上的Context复制出来一份为新连入的Socket新建一个SocketContext
	 */
	PER_SOCKET_CONTEXT* pNewSocketContext = new PER_SOCKET_CONTEXT;
	pNewSocketContext->m_socket = pIoContext->m_sockAccept;
	memcpy(&(pNewSocketContext->m_clientAddr), clientAddr, sizeof(SOCKADDR_IN));

	// 参数设置完毕，将这个Socket和完成端口绑定
	if (false == this->AssociateWithIOCP(pNewSocketContext))
	{
		RELEASE(pNewSocketContext);
		return false;
	}

	// 3. 继续，建立其下的IoContext，用于在这个Socket上投递第一个Recv数据请求
	PER_IO_CONTEXT* pNewIoContext = pNewSocketContext->GetNewIoContext();
	pNewIoContext->m_opType = RECV_POSTED;
	pNewIoContext->m_sockAccept = pNewSocketContext->m_socket;
	// 如果Buffer需要保留，就拷贝一份出来
	//memcpy( pNewIoContext->m_szBuffer,pIoContext->m_szBuffer,IOCP_MAX_BUFFER_LEN );

	// 绑定完毕之后，就可以开始在这个Socket上投递完成请求了
	if (false == this->PostRecv(pNewIoContext))
	{
		pNewSocketContext->RemoveContext(pNewIoContext);
		return false;
	}

	// 4. 如果投递成功，那么就把这个有效的客户端信息，加入到ContextList中去(需要统一管理，方便释放资源)
	this->AddToContextList(pNewSocketContext);

	// 5. 使用完毕之后，把Listen Socket的那个IoContext重置，然后准备投递新的AcceptEx
	pIoContext->ResetBuffer();

	return this->PostAccept(pIoContext);
}

// 在有接收的数据到达的时候，进行处理
bool CIOCPServer::DoRecv(PER_SOCKET_CONTEXT* pSocketcontext, PER_IO_CONTEXT* pIoContext)
{
	assert(pSocketcontext != nullptr);
	assert(pIoContext != nullptr);

	// 先把上一次的数据显示出现，然后就重置状态，发出下一个Recv请求
	SOCKADDR_IN *clientAddr = &pSocketcontext->m_clientAddr;
	output_debug(_T("收到  %s:%d 信息：%s"), inet_ntoa(clientAddr->sin_addr), ntohs(clientAddr->sin_port), pIoContext->m_wsaBuf.buf);

	// 然后开始投递下一个WSARecv请求
	return this->PostRecv(pIoContext);
}

/************************************************************************/
/*                    ContextList 相关操作                              */
/************************************************************************/

// 将客户端的相关信息存储到数组中
void CIOCPServer::AddToContextList(PER_SOCKET_CONTEXT* pSocketContext)
{
	::EnterCriticalSection(&m_csContextList);

	m_arrayClientContext.push_back(pSocketContext);

	::LeaveCriticalSection(&m_csContextList);
}

// 移除某个特定的Context
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

// 清空客户端信息
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

// 将句柄(Socket)绑定到完成端口中
bool CIOCPServer::AssociateWithIOCP(PER_SOCKET_CONTEXT* pContext)
{
	assert(pContext!= nullptr);

	// 将用于和客户端通信的SOCKET绑定到完成端口中
	HANDLE hTemp = ::CreateIoCompletionPort((HANDLE)pContext->m_socket, m_hIOCompletionPort, (DWORD)pContext, 0);

	if (NULL == hTemp)
	{
		output_debug("执行CreateIoCompletionPort()出现错误.错误代码：%d", GetLastError());
		return false;
	}

	return true;
}

// 显示并处理完成端口上的错误
bool CIOCPServer::HandleError(PER_SOCKET_CONTEXT* pContext, const DWORD dwErr)
{
	assert(pContext != nullptr);

	// 如果是超时了，就再继续等吧 
	if (WAIT_TIMEOUT == dwErr)
	{
		// 确认客户端是否还活着...
		if (!IsSocketAlive(pContext->m_socket))
		{
			OutputDebugString(_T("检测到客户端异常退出！"));
			this->RemoveContext(pContext);
			return true;
		}
		else
		{
			OutputDebugString(_T("网络操作超时！重试中..."));
			return true;
		}
	}
	else if (ERROR_NETNAME_DELETED == dwErr)	// 可能是客户端异常退出了
	{
		OutputDebugString(_T("检测到客户端异常退出！"));
		this->RemoveContext(pContext);
		return true;
	}
	else
	{
		output_debug(_T("完成端口操作出现错误，线程退出。错误代码：%d"), dwErr);
		return false;
	}
}

/*
 * 工作者线程: 为IOCP请求服务的工作者线程
 *		也就是每当完成端口上出现了完成数据包，就将之取出来进行处理的线程
 */
DWORD WINAPI CIOCPServer::_WorkerThread(LPVOID lpParam)
{
	THREADPARAMS_WORKER* pParam = (THREADPARAMS_WORKER*)lpParam;
	assert(pParam != nullptr);
	CIOCPServer* pIOCPServer = (CIOCPServer*)pParam->pIOCPServer;
	assert(pIOCPServer != nullptr);
	int nThreadNo = (int)pParam->nThreadNo;

	// 工作者线程启动

	OVERLAPPED* pOverlapped = nullptr;
	PER_SOCKET_CONTEXT* pSocketContext = nullptr;
	DWORD dwBytesTransfered = 0;

	// 循环处理请求，知道接收到Shutdown信息为止
	while (WAIT_OBJECT_0 != ::WaitForSingleObject(pIOCPServer->m_hShutdownEvent, 0))
	{
		/*
		 * 会让Worker线程进入不占用CPU的睡眠状态，直到完成端口上出现了需要处理的网络操作或者超出了等待的时间限制为止。
		 * 一旦完成端口上出现了已完成的I/O请求，
		 * 那么等待的线程会被立刻唤醒[后入先出的方式, 这样如果反复只有一个I/O操作而不是多个操作完成的话，
		 * 内核就只需要唤醒同一个线程就可以了，而不需要轮着唤醒多个线程，节约了资源，
		 * 而且可以把其他长时间睡眠的线程换出内存，提到资源利用率。]，
		 * (传送大块数据的时候，是需要注意下这个先后次序)
		 * 然后继续执行后续的代码。
		 *
		 * BOOL WINAPI GetQueuedCompletionStatus(
		 * __in   HANDLE          CompletionPort,    // 建立的唯一的完成端口
		 * __out  LPDWORD         lpNumberOfBytes,   // 操作完成后返回的字节数
		 * __out  PULONG_PTR      lpCompletionKey,   // 建立完成端口的时候绑定的自定义结构体参数
		 * __out  LPOVERLAPPED    *lpOverlapped,     // 在连入Socket的时候一起建立的重叠结构, 里面就带有PER_IO_CONTEXT (投递AcceptEx请求传的重叠参数)
		 * __in   DWORD           dwMilliseconds     // 等待完成端口的超时时间，如果线程不需要做其他的事情，那就INFINITE就行了
		 * );

		 */
		BOOL bReturn = ::GetQueuedCompletionStatus(pIOCPServer->m_hIOCompletionPort, &dwBytesTransfered, (PULONG_PTR)&pSocketContext, &pOverlapped, INFINITE);
		// 如果收到的是退出标志，则直接退出
		if (EXIT_CODE == (DWORD)pSocketContext)
		{
			break;
		}

		// 判断是否出现了错误
		if (!bReturn)
		{
			DWORD dwErr = ::GetLastError();

			// 显示一下提示信息
			if (!pIOCPServer->HandleError(pSocketContext, dwErr))
			{
				break;
			}

			continue;
		}
		else
		{
			/*
			 * 读取传入的参数
			 * 去传入的lpOverlapped变量里，找到和结构体中PER_IO_CONTEXT中m_overlapped成员相关的数据。
			 * 确保在结构体PER_IO_CONTEXT定义的时候，把Overlapped变量，定义为结构体中的第一个成员。
			 */
			PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, PER_IO_CONTEXT, m_overlapped);
			assert(pIoContext != nullptr);

			// 判断是否有客户端断开
			if ((0 == dwBytesTransfered) && (RECV_POSTED == pIoContext->m_opType || SEND_POSTED == pIoContext->m_opType))
			{
				// 客户端断开连接
				::inet_ntoa(pSocketContext->m_clientAddr.sin_addr);
				::ntohs(pSocketContext->m_clientAddr.sin_port);

				// 释放掉对应的资源
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
					OutputDebugString(_T("_WorkThread中的 pIoContext->m_OpType 参数异常.\n"));
				}
					break;
				}
			}
		}
	}

	output_debug(_T("工作者线程 %d 号退出.\n"), nThreadNo);

	// 释放线程参数
	RELEASE(lpParam);

	return 0;
}

// 获得本机中处理器的数量
int CIOCPServer::GetNoOfProcessors()
{
	SYSTEM_INFO si;

	::GetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}

/*
 * 判断客户端Socket是否已经断开，否则在一个无效的Socket上投递WSARecv操作会出现异常
 * 使用的方法是尝试向这个socket发送数据，判断这个socket调用的返回值
 * 因为如果客户端网络异常断开(例如客户端崩溃或者拔掉网线等)的时候，服务器端是无法收到客户端断开的通知的
 */
bool CIOCPServer::IsSocketAlive(SOCKET s)
{
	int nByteSent = ::send(s, "", 0, 0);
	if (-1 == nByteSent)
		return false;

	return true;
}
