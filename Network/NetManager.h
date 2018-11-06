/*
 * ���������
 */
#pragma once

#include <WinSock2.h>
#include "PacketFactoryManager.h"

class CNetManager
{
public:
	CNetManager();
	virtual ~CNetManager();
	static CNetManager* GetMe() { return s_pMe; }
public:
	// ���ӵ�������
	VOID ConnectToServer(LPCTSTR szServerAddr, INT nServerPort);
	// �������ݰ�
	VOID SendPacket(Packet* pPacket);
	// �Ͽ�����
	VOID Close();
	// ������Ϣ����
	VOID ProcessInputStream(SocketInputStream& inputStream);
	// �õ���Ϣ����
	PacketFactoryManager& GetPacketFactory() { return m_packetFactoryManager; }
public:
	// �ڵ��ʼ��
	virtual VOID Initial(VOID*);
	// �߼���ѯ����
	virtual VOID Tick();
	// �ͷ��Լ���ӵ�е���Դ
	virtual VOID Release();
public:
	enum NETMANAGER_STATUS
	{
		CONNECT_SUCESS = 0,
		CONNECT_FAILED_CREATE_SOCKET_ERROR,
		CONNECT_FAILED_CONNECT_ERROR,
		CONNECT_FAILED_TIME_OUT,
	};
	//////////////////////////////////////////////////////////////////////////
	// Tick��ͬ����Ϸ����
	//////////////////////////////////////////////////////////////////////////
	// ��������״̬
	VOID SetNetStatus(NETMANAGER_STATUS netStatus);
protected:
	// �����̣߳�ר���������ӷ�������
	static UINT CALLBACK _ConnectThread(LPVOID pParam);
	INT ConnectThread();
	// �����߳̾��
	HANDLE					m_hConnectThread;
	UINT					m_timeConnectBegin;
private:
	VOID WaitConnecting();
	VOID WaitPacket();
	VOID SendHeartBeat();

	// SOCKET������
	BOOL Select();
	BOOL ProcessExcept();
	BOOL ProcessInput();
	BOOL ProcessOutput();
	VOID ProcessCommands();

	// ��Ϣ��ִ��
	UINT ExecutePacketGenException(Packet* pPacket);
	UINT ExecutePacketCppException(Packet* pPacket);
protected:
	static CNetManager*		s_pMe;
	// ��������Ϣ
	std::string				m_strServerAddr;
	INT						m_nServerPort;
private:
	// �ɶ����
	fd_set					m_readFD;
	// ��д���
	fd_set					m_writeFD;
	// �쳣���
	fd_set					m_exceptFD;
	// SOCKET
	Socket					m_socket;
	// ���뻺����
	SocketInputStream		m_socketInputStream;
	// ���������
	SocketOutputStream		m_socketOutputStream;
	// SOCKET��������
	PacketFactoryManager	m_packetFactoryManager;
};