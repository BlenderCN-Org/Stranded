#ifndef __IOCP_CLIENT_H__
#define __IOCP_CLIENT_H__

/*
 * 类CClient: 用于产生用于指定的并发线程向指定服务器发送
 * 信息，测试服务器的响应及资源占用率情况.
 * 
 * Notes:
 * 客户端使用的是最简单的多线程阻塞式Socket，而且每个线程只发送一次数据
 * 如果需要可以修改成发送多次数据的情况
 */

#include <WinSock2.h>
#include "../Common/common.h"

// 缓冲区长度(8*1024字节)
#define CLIENT_MAX_BUFFER_LEN	8196
#define CLIENT_DEFAULT_PORT		12345				// 默认端口
#define CLIENT_DEFAULT_IP		_T("127.0.0.1")		// 默认IP地址
#define CLIENT_DEFAULT_THREADS	100					// 默认并发线程数
#define CLIENT_DEFAULT_MESSAGE	_T("Hello!")		// 默认的发送信息

class CClient;
// 用于发送数据的线程参数
typedef struct _tagThreadParams_WORKER
{
	CClient*	pClient;							// 类指针，用于调用类中的函数
	SOCKET		sock;								// 每个线程使用的Socket
	int			nThreadNo;							// 线程编号
	char		szBuffer[CLIENT_MAX_BUFFER_LEN];
} THREADPARAMS_WORKER, *PTHREADPARAMS_WORKER;

// 产生Socket连接的线程
typedef struct _tagThreadParams_CONNECTION
{
	CClient* pClient;	// 类指针，用于调用类中的函数
} THREADPARAMS_CONNECTION, *PTHREADPARAMS_CONNECTION;

class CClient
{
public:
	CClient();
	~CClient();

public:
	// 加载Socket库
	bool LoadSocketLib();
	// 卸载Socket库
	inline void UnloadsocketLib() { ::WSACleanup(); }

	// 开始测试
	bool Start();
	// 停止测试
	void Stop();

	// 获得本机的IP地址
	std::string GetLocalIP();

	// 设置连接IP地址
	inline void SetIP(const std::string& strIP) { m_strServerIP = strIP; }
	// 设置监听端口
	inline void SetPort(const int nPort) { m_nPort = nPort; }
	// 设置并发线程数量
	inline void SetThreads(const int n) { m_nThreads = n; }
	// 设置要按发送的信息
	inline void SetMessage(const std::string& strMessage) { m_strMessage = strMessage; }

private:
	// 建立连接
	bool EstablishConnection();
	// 向服务器进行连接
	bool ConnectToServer(SOCKET *pSocket, const std::string& strServer, int nPort);
	// 用于建立连接的线程
	static DWORD WINAPI _ConnectionThread(LPVOID lpParam);
	// 用于发送信息的线程
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);

	// 释放资源
	void CleanUp();

private:
	std::string					m_strServerIP;			// 服务器端的IP地址
	std::string					m_strLocalIP;			// 本机IP地址
	std::string					m_strMessage;			// 发给服务器的信息
	int							m_nPort;				// 监听端口
	int							m_nThreads;				// 并发线程数量

	HANDLE						*m_phWorkerThreads;
	HANDLE						m_hConnectionThread;	// 接受连接的线程句柄
	HANDLE						m_hShutdownEvent;		// 用来通知线程系统退出的事件，为了能够更好的退出线程

	THREADPARAMS_WORKER			*m_pParamsWorker;		// 线程参数
};

#endif
