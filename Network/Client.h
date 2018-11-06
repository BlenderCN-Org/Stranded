#ifndef __IOCP_CLIENT_H__
#define __IOCP_CLIENT_H__

/*
 * ��CClient: ���ڲ�������ָ���Ĳ����߳���ָ������������
 * ��Ϣ�����Է���������Ӧ����Դռ�������.
 * 
 * Notes:
 * �ͻ���ʹ�õ�����򵥵Ķ��߳�����ʽSocket������ÿ���߳�ֻ����һ������
 * �����Ҫ�����޸ĳɷ��Ͷ�����ݵ����
 */

#include <WinSock2.h>
#include "../Common/common.h"

// ����������(8*1024�ֽ�)
#define CLIENT_MAX_BUFFER_LEN	8196
#define CLIENT_DEFAULT_PORT		12345				// Ĭ�϶˿�
#define CLIENT_DEFAULT_IP		_T("127.0.0.1")		// Ĭ��IP��ַ
#define CLIENT_DEFAULT_THREADS	100					// Ĭ�ϲ����߳���
#define CLIENT_DEFAULT_MESSAGE	_T("Hello!")		// Ĭ�ϵķ�����Ϣ

class CClient;
// ���ڷ������ݵ��̲߳���
typedef struct _tagThreadParams_WORKER
{
	CClient*	pClient;							// ��ָ�룬���ڵ������еĺ���
	SOCKET		sock;								// ÿ���߳�ʹ�õ�Socket
	int			nThreadNo;							// �̱߳��
	char		szBuffer[CLIENT_MAX_BUFFER_LEN];
} THREADPARAMS_WORKER, *PTHREADPARAMS_WORKER;

// ����Socket���ӵ��߳�
typedef struct _tagThreadParams_CONNECTION
{
	CClient* pClient;	// ��ָ�룬���ڵ������еĺ���
} THREADPARAMS_CONNECTION, *PTHREADPARAMS_CONNECTION;

class CClient
{
public:
	CClient();
	~CClient();

public:
	// ����Socket��
	bool LoadSocketLib();
	// ж��Socket��
	inline void UnloadsocketLib() { ::WSACleanup(); }

	// ��ʼ����
	bool Start();
	// ֹͣ����
	void Stop();

	// ��ñ�����IP��ַ
	std::string GetLocalIP();

	// ��������IP��ַ
	inline void SetIP(const std::string& strIP) { m_strServerIP = strIP; }
	// ���ü����˿�
	inline void SetPort(const int nPort) { m_nPort = nPort; }
	// ���ò����߳�����
	inline void SetThreads(const int n) { m_nThreads = n; }
	// ����Ҫ�����͵���Ϣ
	inline void SetMessage(const std::string& strMessage) { m_strMessage = strMessage; }

private:
	// ��������
	bool EstablishConnection();
	// ���������������
	bool ConnectToServer(SOCKET *pSocket, const std::string& strServer, int nPort);
	// ���ڽ������ӵ��߳�
	static DWORD WINAPI _ConnectionThread(LPVOID lpParam);
	// ���ڷ�����Ϣ���߳�
	static DWORD WINAPI _WorkerThread(LPVOID lpParam);

	// �ͷ���Դ
	void CleanUp();

private:
	std::string					m_strServerIP;			// �������˵�IP��ַ
	std::string					m_strLocalIP;			// ����IP��ַ
	std::string					m_strMessage;			// ��������������Ϣ
	int							m_nPort;				// �����˿�
	int							m_nThreads;				// �����߳�����

	HANDLE						*m_phWorkerThreads;
	HANDLE						m_hConnectionThread;	// �������ӵ��߳̾��
	HANDLE						m_hShutdownEvent;		// ����֪ͨ�߳�ϵͳ�˳����¼���Ϊ���ܹ����õ��˳��߳�

	THREADPARAMS_WORKER			*m_pParamsWorker;		// �̲߳���
};

#endif
