#ifndef __IOCP_SERVER_H__
#define __IOCP_SERVER_H__

/*
 * CompletionPort
 *
 * @Point:
 * �����ļ���������һ������ȴ�һ���ļ���Ȼ�󲻶ϵ�ѭ������ReadFile()��ȡһ��֮��Ȼ���ٵ���WSASend ()ȥ�����͡�
 * ReadFile()��ʱ������Ҫ����ϵͳͨ�����̵��������򣬵�ʵ�ʵ�����Ӳ����ȥ��ȡ�ļ��ģ�
 * ��ͻ�ʹ�ò���ϵͳ���û�̬ת�����ں�̬ȥ������������Ȼ���ٰѶ�ȡ�Ľ���������û�̬��
 * ͬ���ĵ���WSARecv()Ҳ���漰�����û�̬���ں�̬�л������� --- ���ò�Ƶ�������û�̬���ں�̬֮��ת����Ч�ʵ�
 *
 * ��һ���ǳ��õĽ��������ʹ��΢���ṩ����չ����TransmitFile()�������ļ���
 * ��Ϊֻ��Ҫ���ݸ�TransmitFile()һ���ļ��ľ������Ҫ������ֽ���������ͻ������л����ں�̬��
 * �����Ƕ�ȡ���ݻ��Ƿ����ļ�������ֱ�����ں�̬��ִ�еģ�ֱ���ļ�������ϲŻ᷵�����û�̬�������̷���֪ͨ��Ч�ʸߡ�
 */

#include <WinSock2.h>
/*
 * ׼��AcceptEx
 * 
 * AcceptEx΢����չ����
 * ΢���ʵ����ͨ��mswsock.dll���ṩ, ����ͨ����̬����mswsock.lib��ʹ��AcceptEx
 * ��Ϊ������δȡ�ú���ָ�������¾͵���AcceptEx�Ŀ����Ǻܴ�ģ�
 * ��ΪAcceptEx ʵ�����Ǵ�����Winsock2�ṹ��ϵ֮���(��Ϊ��΢�������ṩ��)��
 * �����������ֱ�ӵ���AcceptEx�Ļ����������ǵĴ����ֻ����΢���ƽ̨�����ˣ�
 * û�а취������ƽ̨�ϵ��õ���ƽ̨�ṩ��AcceptEx�İ汾(����еĻ�)�� 
 * ���Ҹ������ǣ�����ÿ�ε���AcceptExʱ��Service Provider����Ҫͨ��WSAIoctl()��ȡһ�θú���ָ�룬Ч�ʵ�, ֱ�ӻ�ȡָ�롣
 *
 * �Ƽ�: ��WSAIoctl ���SIO_GET_EXTENSION_FUNCTION_POINTER��������ȡ������ָ�룬Ȼ���ٵ���AcceptEx��
 */
#include <MSWSock.h>
#include "../Common/common.h"

/*
 * ��x86����ϵ�У��ڴ�ҳ������4KBΪ��λ�������ģ�
 * Ҳ����˵����������Ͷ��WSARecv()��ʱ��ֻ����1KB��С�Ļ�������ϵͳ���ǵø����4KB���ڴ档
 * Ϊ�˱��������˷ѣ�����ǰѷ��ͺͽ������ݵĻ�����ֱ�����ó�4KB�ı�����
 * ���������� (1024*8)������ͻ��˹���ÿ�����ݶ��Ƚ�С����������СЩ
 */
#define IOCP_MAX_BUFFER_LEN	8192
// Ĭ�϶˿�
#define IOCP_DEFAULT_PORT 12345
// Ĭ��IP��ַ
#define IOCP_DEFAULT_IP	_T("127.0.0.1")

// ��CompletionPort��Ͷ�ݵ�I/O��������
typedef enum _OPERATION_TYPE
{
	ACCEPT_POSTED,				// ��־Ͷ�ݵ�Accept����
	SEND_POSTED,				// ��־Ͷ�ݵ��Ƿ��Ͳ���
	RECV_POSTED,				// ��־Ͷ�ݵ��ǽ��ղ���
	NULL_POSTED					// ���ڳ�ʼ����������
} OPERATION_TYPE;

// ��IO���ݽṹ�嶨��(ÿһ��I/O�������ж�Ӧ��PER_IO_CONTEXT�ṹ, ����ÿһ��Overlapped�����Ĳ���)
typedef struct _PER_IO_CONTEXT
{
	// ȷ���ڽṹ��PER_IO_CONTEXT�����ʱ�򣬰�Overlapped����������Ϊ�ṹ���еĵ�һ����Ա��
	OVERLAPPED		m_overlapped;					// ÿһ���ص�����������ص��ṹ(���ÿһ��Socket��ÿһ����������Ҫ��һ��)
	SOCKET			m_sockAccept;					// ������������ʹ�õ�Socket
	WSABUF			m_wsaBuf;						// WSA���͵Ļ����������ڸ��ص�������������
	char			m_szbuff[IOCP_MAX_BUFFER_LEN];	// �����WSABUF�������ַ��Ļ�����
	OPERATION_TYPE	m_opType;						// ��ʶ�������������(��Ӧ�����ö��)

	// ��ʼ��
	_PER_IO_CONTEXT()
	{
		ZeroMemory(&m_overlapped, sizeof(OVERLAPPED));
		ZeroMemory(m_szbuff, IOCP_MAX_BUFFER_LEN);

		m_sockAccept = INVALID_SOCKET;
		m_wsaBuf.buf = m_szbuff;
		m_wsaBuf.len = IOCP_MAX_BUFFER_LEN;
		m_opType = NULL_POSTED;
	}

	// �ͷŵ�Socket
	~_PER_IO_CONTEXT()
	{
		if (m_sockAccept != INVALID_SOCKET)
		{
			::closesocket(m_sockAccept);
			m_sockAccept = INVALID_SOCKET;
		}
	}

	// ���û���������
	void ResetBuffer()
	{
		ZeroMemory(m_szbuff, IOCP_MAX_BUFFER_LEN);
	}
} PER_IO_CONTEXT, *PPER_IO_CONTEXT;

// �����(SOCKET)���ݽṹ�嶨��(����ÿһ����ɶ˿ڣ���ÿһ��Socket�Ĳ���. ��ÿһ��Socket�ϣ�Ͷ�ݶ��I/O����)
typedef struct _PER_SOCKET_CONTEXT
{
	SOCKET							m_socket;					// ÿһ���ͻ������ӵ�Socket
	SOCKADDR_IN						m_clientAddr;				// �ͻ��˵ĵ�ַ
	std::vector<_PER_IO_CONTEXT*>	m_arrayIoContext;			// Ҳ����˵����ÿһ���ͻ���Socket���ǿ���������ͬʱͶ�ݶ��IO�����

	// ��ʼ��
	_PER_SOCKET_CONTEXT()
	{
		m_socket = INVALID_SOCKET;
		memset(&m_clientAddr, 0, sizeof(SOCKADDR_IN));
	}

	// �ͷ���Դ
	~_PER_SOCKET_CONTEXT()
	{
		if (m_socket != INVALID_SOCKET)
		{
			::closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
		// �ͷŵ����е�IO����������
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

	// ��ȡһ���µ�IoContext
	_PER_IO_CONTEXT* GetNewIoContext()
	{
		_PER_IO_CONTEXT* p = new _PER_IO_CONTEXT;

		m_arrayIoContext.push_back(p);

		return p;
	}

	// ���������Ƴ�һ��ָ����IoContext
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

// �������̵߳��̲߳���
class CIOCPServer;
typedef struct _tagThreadParams_WORKER
{
	CIOCPServer*	pIOCPServer;		// ��ָ�룬���ڵ������еĺ���
	int				nThreadNo;			// �̱߳��
} THREADPARAMS_WORKER, *PTHREADPARAMS_WORKER;

class CIOCPServer
{
public:
	CIOCPServer();
	virtual ~CIOCPServer();

public:
	// ����������
	bool Start();

	// ֹͣ������
	void Stop();

	// ����Socket��
	bool LoadSocketLib();

	// ж��Socket�⣬��������
	void UnloadSocketLib() { ::WSACleanup(); }

	// ��ñ��ص�IP��ַ
	std::string GetLocalIP();

	// ���ü����˿�
	void SetPort(const int nPort) { m_nPort = nPort; }

protected:
	// ��ʼ��IOCP
	bool InitializeIOCP();

	// ��ʼ��Socket
	bool InitializeListenSocket();

	// ����ͷ���Դ
	void DeInitialize();

	// Ͷ��Accept����
	bool PostAccept(PER_IO_CONTEXT* pAcceptIoContext);

	// Ͷ�ݽ�����������
	bool PostRecv(PER_IO_CONTEXT* pIoContext);

	// ���пͻ��������ʱ�򣬽��д���
	bool DoAccept(PER_SOCKET_CONTEXT* pSocketcontext, PER_IO_CONTEXT* pIoContext);

	// ���н��յ����ݵ����ʱ�򣬽��д���
	bool DoRecv(PER_SOCKET_CONTEXT* pSocketcontext, PER_IO_CONTEXT* pIoContext);

	// ���ͻ��˵������Ϣ�洢��������
	void AddToContextList(PER_SOCKET_CONTEXT* pSocketContext);

	// ���ͻ��˵���Ϣ���������Ƴ�
	void RemoveContext(PER_SOCKET_CONTEXT* pSocketContext);

	// ��տͻ�����Ϣ
	void ClearContextList();

	// ������󶨵���ɶ˿���
	bool AssociateWithIOCP(PER_SOCKET_CONTEXT* pContext);

	// ������ɶ˿��ϵĴ���
	bool HandleError(PER_SOCKET_CONTEXT* pContext, const DWORD dwErr);

	// �̺߳�����ΪIOCP�������Ĺ������߳�
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);

	// ��ñ����Ĵ���������
	int GetNoOfProcessors();

	// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ�
	bool IsSocketAlive(SOCKET s);

private:
	HANDLE									m_hShutdownEvent;				// ����֪ͨ�߳�ϵͳ�˳����¼���Ϊ���ܹ����õ��˳��߳�
	HANDLE									m_hIOCompletionPort;			// ��ɶ˿ڵľ��
	HANDLE*									m_phWorkerThreads;				// �������̵߳ľ��ָ��
	int										m_nThreads;						// ���ɵ��߳�����
	std::string								m_strIP;						// �������˵�IP��ַ
	int										m_nPort;						// �������˵ļ����˿�
	CRITICAL_SECTION						m_csContextList;				// ����Worker�߳�ͬ���Ļ�����
	std::vector<PER_SOCKET_CONTEXT*>		m_arrayClientContext;			// �ͻ���Socket��Context��Ϣ
	PER_SOCKET_CONTEXT*						m_pListenContext;				// ���ڼ�����Socket��Context��Ϣ

	LPFN_ACCEPTEX							m_lpfnAcceptEx;					// AcceptEx �� GetAcceptExSockaddrs �ĺ���ָ�룬���ڵ�����������չ����
	LPFN_GETACCEPTEXSOCKADDRS				m_lpfnGetAcceptExSockAddrs;
};

#endif
