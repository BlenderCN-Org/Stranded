#include "NetManager.h"
#include "../Common/SException.h"
#include <process.h>
#include "CGHeartBeat.h"

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

#define MAX_CONNECT_TIME (10*1000)	// 最长连接时间10s

CNetManager* CNetManager::s_pMe = nullptr;
extern char Error[_ESIZE];

CNetManager::CNetManager() : m_socketInputStream(&m_socket), m_socketOutputStream(&m_socket)
{
	s_pMe = this;
	m_hConnectThread = nullptr;
}

CNetManager::~CNetManager()
{

}

VOID CNetManager::ConnectToServer(LPCTSTR szServerAddr, INT nServerPort)
{
	// 已经有登录线程在运行中
	if (m_hConnectThread)
		return;

	m_strServerAddr = szServerAddr;
	m_nServerPort = nServerPort;

	// 创建登录线程
	UINT nThreadID;
	m_hConnectThread = (HANDLE)::_beginthreadex(nullptr, 0, _ConnectThread, this, CREATE_SUSPENDED|THREAD_QUERY_INFORMATION, &nThreadID);
	if (m_hConnectThread == nullptr)
		SThrow(_T("(CNetManager::ConnectToServer) Can't create connect thread!"));

	// 开始运行登录线程
	m_timeConnectBegin = 0;	// GetTimeNow();
	::ResumeThread(m_hConnectThread);
}

VOID CNetManager::SendPacket(Packet* pPacket)
{
	assert(pPacket == nullptr);

	if (m_socket.isValid())
	{
		output_debug("发送包信息：id=[%d], size=[%d] 服务器端口：[%d]", pPacket->GetPacketID(), pPacket->GetPacketSize(), m_socketOutputStream.GetSocket()->GetPort());

		UINT nSizeBefore = m_socketOutputStream.Length();

		// 查询当前包尾位置，记录写包前位置
		UINT nTailBegin = m_socketOutputStream.GetTail();

		PacketID_t packetID = pPacket->GetPacketID();

		UINT w = m_socketOutputStream.Write((CHAR*)&packetID, sizeof(PacketID_t));

		UINT packetTick = 0;
		w = m_socketOutputStream.Write((CHAR*)&packetTick, sizeof(UINT));

		UINT packetUINT;
		UINT packetSize = pPacket->GetPacketSize();
		UINT packetIndex = pPacket->GetPacketIndex();

		SET_PACKET_INDEX(packetUINT, packetIndex);
		SET_PACKET_LEN(packetUINT, packetSize);

		w = m_socketOutputStream.Write((CHAR*)&packetUINT, sizeof(UINT));

		BOOL ret = pPacket->Write(m_socketOutputStream);

		UINT nSizeAfter = m_socketOutputStream.Length();

		// 查询当前包尾位置，记录写包后位置
		UINT nTailEnd = m_socketOutputStream.GetTail();

		// 消息加密处理 -- Begin
		{
			UINT nSize = nTailEnd - nTailBegin;
			UINT nHead = m_socketOutputStream.GetHead();
			UINT nTail = m_socketOutputStream.GetTail();
			UINT nBufferLen = m_socketOutputStream.GetBuffLen();
			CHAR* szBuffer = m_socketOutputStream.GetBuffer();
			if (nHead < nTail)
			{
				ENCRYPT(&(szBuffer[nTailBegin]), nSize, CLIENT_TO_LOGIN_KEY, 0);
			}
			else
			{
				UINT rightLen = nBufferLen - nHead;
				if (nSize <= rightLen)
				{
					ENCRYPT(&(szBuffer[nTailBegin]), nSize, CLIENT_TO_LOGIN_KEY, 0);
				}
				else
				{
					ENCRYPT(&(szBuffer[nTailBegin]), rightLen, CLIENT_TO_LOGIN_KEY, 0);
					ENCRYPT(szBuffer, nSize-rightLen, CLIENT_TO_LOGIN_KEY, rightLen);
				}
			}
		}
		// 消息加密处理 -- End

		if (pPacket->GetPacketSize() != nSizeAfter - nSizeBefore - PACKET_HEADER_SIZE)
		{
			output_debug("Packet size error. id=[%d], stream=[%d], size=[%d]",
				pPacket->GetPacketID(), nSizeAfter-nSizeBefore-PACKET_HEADER_SIZE, pPacket->GetPacketSize());
		}
	}
}

VOID CNetManager::Close()
{
	m_socket.close();
}

VOID CNetManager::ProcessInputStream(SocketInputStream& inputStream)
{
	static USHORT nPacketIDSaved = 0;

	BOOL ret;

	char packCompart[PACKET_HEADER_SIZE];
	char header[PACKET_HEADER_SIZE];
	PacketID_t packetID;
	UINT packetuint, packetSize, packetIndex, packetTick;
	Packet* pPacket = nullptr;
	
	static PacketID_t packetIDList[10];
	static int packetIDListIdx = 0;

	{
		for (;;)
		{
			if (!m_socketInputStream.Find(packCompart))
			{
				// 数据没有消息分割点
				break;
			}

			if (!m_socketInputStream.Peek(&header[0], PACKET_HEADER_SIZE))
			{
				// 数据不能填充消息头
				break;
			}

			// 封包头解密
			ENCRYPT_HEAD(header, LOGIN_TO_CLIENT_KEY);

			memcpy(&packetID, &header[0], sizeof(PacketID_t));
			memcpy(&packetTick, &header[sizeof(UINT)], sizeof(UINT));
			memcpy(&packetuint, &header[sizeof(UINT) + sizeof(PacketID_t)], sizeof(UINT));
			packetSize = GET_PACKET_LEN(packetuint);
			packetIndex = GET_PACKET_INDEX(packetuint);

			if(packetID >= (PacketID_t)PACKET_MAX)
			{
				// 无效的消息类型
				output_debug("Invalid Packet, Old Packet List [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d], Idx=[%d]", packetIDList[0], packetIDList[1],
					packetIDList[2], packetIDList[3], packetIDList[4], packetIDList[5], packetIDList[6], packetIDList[7], packetIDList[8],
					packetIDList[9], packetIDListIdx);
				output_debug("Invalid Packet[%d]", packetID);
				return;
			}

			// 消息解密处理 -- Begin
			{
				UINT nSize = packetSize + PACKET_HEADER_SIZE;
				UINT nHead = m_socketInputStream.GetHead();
				UINT nTail = m_socketInputStream.GetTail();
				UINT nBufferLen = m_socketInputStream.GetBufferLen();
				CHAR* szBuffer = m_socketInputStream.GetBuffer();
				if (nHead < nTail)
				{
					ENCRYPT(&szBuffer[nHead], nSize, LOGIN_TO_CLIENT_KEY, 0);
				}
				else
				{
					UINT rightLen = nBufferLen - nHead;
					if (nSize <= rightLen)
					{
						ENCRYPT(&szBuffer[nHead], nSize, LOGIN_TO_CLIENT_KEY, 0);
					}
					else
					{
						ENCRYPT(&szBuffer[nHead], rightLen, LOGIN_TO_CLIENT_KEY, 0);
						ENCRYPT(szBuffer, nSize - rightLen, LOGIN_TO_CLIENT_KEY, rightLen);
					}
				}
			}
			// 消息解密处理 -- End

			{
				if (m_socketInputStream.Length() < PACKET_HEADER_SIZE + packetSize)
				{
					// 消息没有接收全
					break;
				}

				if (packetSize > m_packetFactoryManager.GetPacketMaxSize(packetID))
				{
					output_debug("Packet size error, Old Packet List [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d], Idx=[%d]", packetIDList[0], packetIDList[1],
						packetIDList[2], packetIDList[3], packetIDList[4], packetIDList[5], packetIDList[6], packetIDList[7], packetIDList[8],
						packetIDList[9], packetIDListIdx);
					// 消息的大小出现异常，收到的消息比预定义消息的最大值还要大
					output_debug("Packet size error! Id=[%d]([%d]>[%d])", packetID, packetSize, m_packetFactoryManager.GetPacketMaxSize(packetID));
					return;
				}

				Packet* pPacket = m_packetFactoryManager.CreatePacket(packetID);
				assert(pPacket);
				if (pPacket == nullptr)
				{
					// 不能分配到足够的内存
					output_debug("Create Packet error, Old Packet List [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d], Idx=[%d]", packetIDList[0], packetIDList[1],
						packetIDList[2], packetIDList[3], packetIDList[4], packetIDList[5], packetIDList[6], packetIDList[7], packetIDList[8],
						packetIDList[9], packetIDListIdx);
					SThrow("Create Packet error.[%d]!", packetID);
					return;
				}

				// 设置消息序列号
				pPacket->SetPacketIndex(packetIndex);

				ret = m_socketInputStream.ReadPacket(pPacket);
				if (ret == FALSE)
				{
					// 读取消息内容错误
					output_debug("Read Packet error, Old Packet List [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d], Idx=[%d]", packetIDList[0], packetIDList[1],
						packetIDList[2], packetIDList[3], packetIDList[4], packetIDList[5], packetIDList[6], packetIDList[7], packetIDList[8],
						packetIDList[9], packetIDListIdx);
					output_debug("Read Packet error!");
					continue;
				}

				packetIDList[packetIDListIdx++] = packetID;
				if (packetIDListIdx == 10) packetIDListIdx = 0;

				output_debug("接收包信息 id=[%d], size=[%d] 服务器端口: [%d]", pPacket->GetPacketID(), pPacket->GetPacketSize(), m_socketInputStream.GetSocket()->GetPort());
				// 消息包执行
				ret = ExecutePacketGenException(pPacket);
				if (ret != PACKET_EXE_NOTREMOVE)
				{
					m_packetFactoryManager.RemovePacket(pPacket);
				}

				nPacketIDSaved = packetID;
			}
		}
	}
}

VOID CNetManager::Initial(VOID*)
{
	// 初始化WinSock
	WSADATA wsaData;
	if (0 != ::WSAStartup(MAKEWORD(2, 2), &wsaData) || (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2))
	{
		::WSACleanup();
		SThrow(_T("Could not find a useable WinSock Dll!"));
		return;
	}

	m_hConnectThread = nullptr;
	m_packetFactoryManager.Init();
}

VOID CNetManager::Tick()
{

}

VOID CNetManager::Release()
{
	// 尚有登陆线程在运行，强制终止
	if (m_hConnectThread)
	{
		::TerminateThread(m_hConnectThread, 0);
		::CloseHandle(m_hConnectThread);
		m_hConnectThread = nullptr;
	}

	m_socket.close();

	::WSACleanup();
}

VOID CNetManager::SetNetStatus(NETMANAGER_STATUS netStatus)
{

}

UINT CALLBACK CNetManager::_ConnectThread(LPVOID pParam)
{
	SAssert(pParam);

	CNetManager* pNetManager = (CNetManager*)pParam;

	return pNetManager->ConnectThread();
}

/*
 * 连接线程返回值
 * 0：尚未连接
 * 1：成功连接到服务器
 * -1：创建socket发生错误
 * -2：无法连接到目的服务器
 * -3：超时错误
 */
INT CNetManager::ConnectThread()
{
	// 关闭socket
	m_socket.close();
	//创建新的socket
	if (!m_socket.create())
	{
		return -1;
	}

	// 连接到服务器
	if (!m_socket.connect(m_strServerAddr.c_str(), m_nServerPort))
	{
		m_socket.close();
		return -2;
	}

	// 成功连接
	return 1;
}

VOID CNetManager::WaitConnecting()
{
	// 监测登陆线程是否结束
	int nExitcode = 0;

	if(::GetExitCodeThread(m_hConnectThread, (DWORD*)&nExitcode))
	{ }

	// 登陆线程未结束
	if (STILL_ACTIVE == nExitcode)
	{
		// 检查超时
		UINT dwTimeNow = 0;	// GetTimeNow();
		UINT dwUsed = 0;	// CalSubTime(m_timeConnectBegin, dwTimeNow);
		// 超时
		if (dwUsed >= MAX_CONNECT_TIME)
		{
			// 强制结束登陆线程
			::TerminateThread(m_hConnectThread, 0);
			nExitcode = -3;
		}
		// 继续等待
		else
		{
			return;
		}
	}

	// 登陆线程已经结束，关闭句柄
	if (::CloseHandle(m_hConnectThread))
	{
		m_hConnectThread = nullptr;
	}

	// 登录过程中发生错误
	if (nExitcode < 0)
	{
		switch (nExitcode)
		{
		case -1:
		{
			// 创建网络连接失败
			SetNetStatus(CONNECT_FAILED_CREATE_SOCKET_ERROR);
			break;
		}
		case -2:
		{
			// 目的服务器可能关闭
			SetNetStatus(CONNECT_FAILED_CONNECT_ERROR);
			break;
		}
		case -3:
		{
			// 连接超时
			SetNetStatus(CONNECT_FAILED_TIME_OUT);
			break;
		}
		default:
		{
			// 未知错误
			SetNetStatus(CONNECT_FAILED_CONNECT_ERROR);
			break;
		}
		}

		this->Close();
		return;
	}

	// 连接成功后设置为非阻塞模式，设置Linger参数
	if (!m_socket.setNonBlocking() || !m_socket.setLinger(0))
	{
		SetNetStatus(CONNECT_FAILED_CONNECT_ERROR);
		SThrow(_T("(CNetManager::Tick) SetSocket Error."));
		return;
	}

	// 通知登陆流程，SOCKET连接成功
	SetNetStatus(CONNECT_SUCESS);

	return;
}

VOID CNetManager::WaitPacket()
{
	if (!m_socket.isValid())
		return;

	// 网络数据流操作
	if (!Select() || !ProcessExcept() || !ProcessInput() || !ProcessOutput())
	{
		// NET_CLOSE
	}

	// Packet操作
	ProcessCommands();
}

VOID CNetManager::SendHeartBeat()
{
	static UINT s_dwLastSendTime = 0;
	const UINT HEART_BEAT_RATE = 60 * 1000;		// 1 Minutes

	UINT dwTimeNow = 0;	// GetTimeNow();
	if (dwTimeNow - s_dwLastSendTime >= HEART_BEAT_RATE)
	{
		CGHeartBeat msg;
		this->SendPacket(&msg);

		s_dwLastSendTime = dwTimeNow;
	}
}

BOOL CNetManager::Select()
{
	FD_ZERO(&m_readFD);
	FD_ZERO(&m_writeFD);
	FD_ZERO(&m_exceptFD);

	FD_SET(m_socket.getSocket(), &m_readFD);
	FD_SET(m_socket.getSocket(), &m_writeFD);
	FD_SET(m_socket.getSocket(), &m_exceptFD);

	timeval timeOut;
	timeOut.tv_sec = 0;
	timeOut.tv_usec = 0;

	if (SOCKET_ERROR == SocketAPI::select_ex(0, &m_readFD, &m_writeFD, &m_exceptFD, &timeOut))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CNetManager::ProcessExcept()
{
	if (FD_ISSET(m_socket.getSocket(), &m_exceptFD))
	{
		m_socket.close();
		return FALSE;
	}
	return TRUE;
}

BOOL CNetManager::ProcessInput()
{
	if (FD_ISSET(m_socket.getSocket(), &m_readFD))
	{
		UINT ret = m_socketInputStream.Fill();
		if ((INT)ret <= SOCKET_ERROR)
		{
			m_socket.close();
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CNetManager::ProcessOutput()
{
	if (FD_ISSET(m_socket.getSocket(), &m_writeFD))
	{
		UINT ret = m_socketOutputStream.Flush();
		if ((INT)ret <= SOCKET_ERROR)
		{
			m_socket.close();
			return FALSE;
		}
	}

	return TRUE;
}

VOID CNetManager::ProcessCommands()
{
	ProcessInputStream(m_socketInputStream);
}

UINT CNetManager::ExecutePacketGenException(Packet* pPacket)
{
	assert(pPacket);

	CHAR szTitle[MAX_PATH];
	
	_snprintf(szTitle, MAX_PATH, "Packet: %d", pPacket->GetPacketID());

	__try
	{
		return ExecutePacketCppException(pPacket);
	}
	__except(CreateDumpHelpFile(GetExceptionInformation(), szTitle), EXCEPTION_EXECUTE_HANDLER) {}

	return PACKET_EXE_CONTINUE;
}

UINT CNetManager::ExecutePacketCppException(Packet* pPacket)
{
	try
	{
		return pPacket->Execute((Player*)this);
	}
	catch (const std::exception& e)
	{
		static std::string strCppException;
		strCppException = e.what();

		// 转化为结构化异常
		LPVOID pException = &strCppException;
		::RaiseException(
			0xE000C0DE,
			EXCEPTION_NONCONTINUABLE,			// 不可继续的严重错误
			1,									// 1个参数
			(CONST ULONG_PTR*)&pException		// 指向静态异常内存
			);
	}
	catch (...)
	{
		throw;
	}

	return PACKET_EXE_CONTINUE;
}
