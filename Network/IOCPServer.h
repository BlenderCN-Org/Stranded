#ifndef __IOCP_SERVER_H__
#define __IOCP_SERVER_H__

/*
 * CompletionPort
 *
 * @Point:
 * 发送文件的做法，一般会是先打开一个文件，然后不断的循环调用ReadFile()读取一块之后，然后再调用WSASend ()去发发送。
 * ReadFile()的时候，是需要操作系统通过磁盘的驱动程序，到实际的物理硬盘上去读取文件的，
 * 这就会使得操作系统从用户态转换到内核态去调用驱动程序，然后再把读取的结果返回至用户态；
 * 同样的道理，WSARecv()也会涉及到从用户态到内核态切换的问题 --- 不得不频繁的在用户态到内核态之间转换，效率低
 *
 * 而一个非常好的解决方案是使用微软提供的扩展函数TransmitFile()来传输文件，
 * 因为只需要传递给TransmitFile()一个文件的句柄和需要传输的字节数，程序就会整个切换至内核态，
 * 无论是读取数据还是发送文件，都是直接在内核态中执行的，直到文件传输完毕才会返回至用户态给主进程发送通知。效率高。
 */

#include <WinSock2.h>
/*
 * 准备AcceptEx
 * 
 * AcceptEx微软扩展函数
 * 微软的实现是通过mswsock.dll中提供, 可以通过静态链接mswsock.lib来使用AcceptEx
 * 因为我们在未取得函数指针的情况下就调用AcceptEx的开销是很大的，
 * 因为AcceptEx 实际上是存在于Winsock2结构体系之外的(因为是微软另外提供的)，
 * 所以如果我们直接调用AcceptEx的话，首先我们的代码就只能在微软的平台上用了，
 * 没有办法在其他平台上调用到该平台提供的AcceptEx的版本(如果有的话)， 
 * 而且更糟糕的是，我们每次调用AcceptEx时，Service Provider都得要通过WSAIoctl()获取一次该函数指针，效率低, 直接获取指针。
 *
 * 推荐: 用WSAIoctl 配合SIO_GET_EXTENSION_FUNCTION_POINTER参数来获取函数的指针，然后再调用AcceptEx。
 */
#include <MSWSock.h>
#include "../Common/common.h"

/*
 * 在x86的体系中，内存页面是以4KB为单位来锁定的，
 * 也就是说，就算是你投递WSARecv()的时候只用了1KB大小的缓冲区，系统还是得给你分4KB的内存。
 * 为了避免这种浪费，最好是把发送和接收数据的缓冲区直接设置成4KB的倍数。
 * 缓冲区长度 (1024*8)，如果客户端过来每组数据都比较小，可以设置小些
 */
#define IOCP_MAX_BUFFER_LEN	8192
// 默认端口
#define IOCP_DEFAULT_PORT 12345
// 默认IP地址
#define IOCP_DEFAULT_IP	_T("127.0.0.1")

// 在CompletionPort上投递的I/O操作类型
typedef enum _OPERATION_TYPE
{
	ACCEPT_POSTED,				// 标志投递的Accept操作
	SEND_POSTED,				// 标志投递的是发送操作
	RECV_POSTED,				// 标志投递的是接收操作
	NULL_POSTED					// 用于初始化，无意义
} OPERATION_TYPE;

// 单IO数据结构体定义(每一个I/O操作都有对应的PER_IO_CONTEXT结构, 用于每一个Overlapped操作的参数)
typedef struct _PER_IO_CONTEXT
{
	// 确保在结构体PER_IO_CONTEXT定义的时候，把Overlapped变量，定义为结构体中的第一个成员。
	OVERLAPPED		m_overlapped;					// 每一个重叠网络操作的重叠结构(针对每一个Socket的每一个操作，都要有一个)
	SOCKET			m_sockAccept;					// 这个网络操作所使用的Socket
	WSABUF			m_wsaBuf;						// WSA类型的缓冲区，用于给重叠操作传参数的
	char			m_szbuff[IOCP_MAX_BUFFER_LEN];	// 这个是WSABUF里具体存字符的缓冲区
	OPERATION_TYPE	m_opType;						// 标识网络操作的类型(对应上面的枚举)

	// 初始化
	_PER_IO_CONTEXT()
	{
		ZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
		ZeroMemory(m_szbuff, IOCP_MAX_BUFFER_LEN);

		m_sockAccept = INVALID_SOCKET;
		m_wsaBuf.buf = m_szbuff;
		m_wsaBuf.len = IOCP_MAX_BUFFER_LEN;
		m_opType = NULL_POSTED;
	}

	// 释放掉Socket
	~_PER_IO_CONTEXT()
	{
		if (m_sockAccept != INVALID_SOCKET)
		{
			::closesocket(m_sockAccept);
			m_sockAccept = INVALID_SOCKET;
		}
	}

	// 重置缓冲区内容
	void ResetBuffer()
	{
		ZeroMemory(m_szbuff, IOCP_MAX_BUFFER_LEN);
	}
} PER_IO_CONTEXT, *PPER_IO_CONTEXT;

// 单句柄(SOCKET)数据结构体定义(用于每一个完成端口，即每一个Socket的参数. 在每一个Socket上，投递多个I/O请求)
typedef struct _PER_SOCKET_CONTEXT
{
	SOCKET							m_socket;					// 每一个客户端连接的Socket
	SOCKADDR_IN						m_clientAddr;				// 客户端的地址
	std::vector<_PER_IO_CONTEXT*>	m_arrayIoContext;			// 也就是说对于每一个客户端Socket，是可以在上面同时投递多个IO请求的

	// 初始化
	_PER_SOCKET_CONTEXT()
	{
		m_socket = INVALID_SOCKET;
		memset(&m_clientAddr, 0, sizeof(SOCKADDR_IN));
	}

	// 释放资源
	~_PER_SOCKET_CONTEXT()
	{
		if (m_socket != INVALID_SOCKET)
		{
			::closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
		// 释放掉所有的IO上下文数据
		for (int i = 0; i < m_arrayIoContext.size(); ++i)
		{
			if (m_arrayIoContext[i])
			{
				delete m_arrayIoContext[i];
				m_arrayIoContext[i] = nullptr;
			}
		}
		m_arrayIoContext.clear();
	}

	// 获取一个新的IoContext
	_PER_IO_CONTEXT* GetNewIoContext()
	{
		_PER_IO_CONTEXT* p = new _PER_IO_CONTEXT;

		m_arrayIoContext.push_back(p);

		return p;
	}

	// 从数组中移除一个指定的IoContext
	void RemoveContext(_PER_IO_CONTEXT* pContext)
	{
		assert(pContext != nullptr);

		for (std::vector<_PER_IO_CONTEXT*>::iterator itr = m_arrayIoContext.begin(); itr != m_arrayIoContext.end(); ++itr)
		{
			if (pContext == *itr)
			{
				delete pContext;
				pContext = nullptr;
				m_arrayIoContext.erase(itr);
				break;
			}
		}
	}
} PER_SOCKET_CONTEXT, *PPER_SOCKET_CONTEXT;

// 工作者线程的线程参数
class CIOCPServer;
typedef struct _tagThreadParams_WORKER
{
	CIOCPServer*	pIOCPServer;		// 类指针，用于调用类中的函数
	int				nThreadNo;			// 线程编号
} THREADPARAMS_WORKER, *PTHREADPARAMS_WORKER;

class CIOCPServer
{
public:
	CIOCPServer();
	virtual ~CIOCPServer();

public:
	// 启动服务器
	bool Start();

	// 停止服务器
	void Stop();

	// 加载Socket库
	bool LoadSocketLib();

	// 卸载Socket库，彻底完事
	void UnloadSocketLib() { ::WSACleanup(); }

	// 获得本地的IP地址
	std::string GetLocalIP();

	// 设置监听端口
	void SetPort(const int nPort) { m_nPort = nPort; }

protected:
	// 初始化IOCP
	bool InitializeIOCP();

	// 初始化Socket
	bool InitializeListenSocket();

	// 最后释放资源
	void DeInitialize();

	// 投递Accept请求
	bool PostAccept(PER_IO_CONTEXT* pAcceptIoContext);

	// 投递接收数据请求
	bool PostRecv(PER_IO_CONTEXT* pIoContext);

	// 在有客户端连入的时候，进行处理
	bool DoAccept(PER_SOCKET_CONTEXT* pSocketcontext, PER_IO_CONTEXT* pIoContext);

	// 在有接收的数据到达的时候，进行处理
	bool DoRecv(PER_SOCKET_CONTEXT* pSocketcontext, PER_IO_CONTEXT* pIoContext);

	// 将客户端的相关信息存储到数组中
	void AddToContextList(PER_SOCKET_CONTEXT* pSocketContext);

	// 将客户端的信息从数组中移除
	void RemoveContext(PER_SOCKET_CONTEXT* pSocketContext);

	// 清空客户端信息
	void ClearContextList();

	// 将句柄绑定到完成端口中
	bool AssociateWithIOCP(PER_SOCKET_CONTEXT* pContext);

	// 处理完成端口上的错误
	bool HandleError(PER_SOCKET_CONTEXT* pContext, const DWORD dwErr);

	// 线程函数，为IOCP请求服务的工作者线程
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);

	// 获得本机的处理器数量
	int GetNoOfProcessors();

	// 判断客户端Socket是否已经断开
	bool IsSocketAlive(SOCKET s);

private:
	HANDLE									m_hShutdownEvent;				// 用来通知线程系统退出的事件，为了能够更好的退出线程
	HANDLE									m_hIOCompletionPort;			// 完成端口的句柄
	HANDLE*									m_phWorkerThreads;				// 工作者线程的句柄指针
	int										m_nThreads;						// 生成的线程数量
	std::string								m_strIP;						// 服务器端的IP地址
	int										m_nPort;						// 服务器端的监听端口
	CRITICAL_SECTION						m_csContextList;				// 用于Worker线程同步的互斥量
	std::vector<PER_SOCKET_CONTEXT*>		m_arrayClientContext;			// 客户端Socket的Context信息
	PER_SOCKET_CONTEXT*						m_pListenContext;				// 用于监听的Socket的Context信息

	LPFN_ACCEPTEX							m_lpfnAcceptEx;					// AcceptEx 和 GetAcceptExSockaddrs 的函数指针，用于调用这两个扩展函数
	LPFN_GETACCEPTEXSOCKADDRS				m_lpfnGetAcceptExSockAddrs;
};

#endif
