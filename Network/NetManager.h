/*
 * 网络管理器
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
	// 连接到服务器
	VOID ConnectToServer(LPCTSTR szServerAddr, INT nServerPort);
	// 发送数据包
	VOID SendPacket(Packet* pPacket);
	// 断开连接
	VOID Close();
	// 处理消息缓冲
	VOID ProcessInputStream(SocketInputStream& inputStream);
	// 得到消息工厂
	PacketFactoryManager& GetPacketFactory() { return m_packetFactoryManager; }
public:
	// 节点初始化
	virtual VOID Initial(VOID*);
	// 逻辑轮询函数
	virtual VOID Tick();
	// 释放自己所拥有的资源
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
	// Tick不同的游戏流程
	//////////////////////////////////////////////////////////////////////////
	// 设置网络状态
	VOID SetNetStatus(NETMANAGER_STATUS netStatus);
protected:
	// 连接线程，专门用来连接服务器用
	static UINT CALLBACK _ConnectThread(LPVOID pParam);
	INT ConnectThread();
	// 连接线程句柄
	HANDLE					m_hConnectThread;
	UINT					m_timeConnectBegin;
private:
	VOID WaitConnecting();
	VOID WaitPacket();
	VOID SendHeartBeat();

	// SOCKET操作类
	BOOL Select();
	BOOL ProcessExcept();
	BOOL ProcessInput();
	BOOL ProcessOutput();
	VOID ProcessCommands();

	// 消息包执行
	UINT ExecutePacketGenException(Packet* pPacket);
	UINT ExecutePacketCppException(Packet* pPacket);
protected:
	static CNetManager*		s_pMe;
	// 服务器信息
	std::string				m_strServerAddr;
	INT						m_nServerPort;
private:
	// 可读句柄
	fd_set					m_readFD;
	// 可写句柄
	fd_set					m_writeFD;
	// 异常句柄
	fd_set					m_exceptFD;
	// SOCKET
	Socket					m_socket;
	// 输入缓冲区
	SocketInputStream		m_socketInputStream;
	// 输出缓冲区
	SocketOutputStream		m_socketOutputStream;
	// SOCKET包管理器
	PacketFactoryManager	m_packetFactoryManager;
};