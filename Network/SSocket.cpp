#include "SSocket.h"

Socket::Socket()
{
	m_socketID = INVALID_SOCKET;
	memset(&m_sockAddr, 0, sizeof(SOCKADDR_IN));
	memset(m_host, 0, IP_SIZE);
	m_port = 0;
}

Socket::Socket(const CHAR* host, UINT port)
{
	memcpy(m_host, host, IP_SIZE - 1);
	m_port = port;

	create();
}

Socket::~Socket()
{
	close();
}

BOOL Socket::create()
{
	m_socketID = SocketAPI::socket_ex(AF_INET, SOCK_STREAM, 0);

	memset(&m_sockAddr, 0, sizeof(m_sockAddr));
	m_sockAddr.sin_family = AF_INET;

	if (isValid())
		return TRUE;
	else
		return FALSE;
}

VOID Socket::close()
{
	if (isValid() && !isSocketError())
	{
		try
		{
			SocketAPI::closesocket_ex(m_socketID);
		}
		catch (...)
		{

		}
	}

	m_socketID = INVALID_SOCKET;
	memset(&m_sockAddr, 0, sizeof(SOCKADDR_IN));
	memset(m_host, 0, IP_SIZE);
	m_port = 0;
}

BOOL Socket::connect()
{
	m_sockAddr.sin_addr.s_addr = ::inet_addr(m_host);

	// set sockaddr's port
	m_sockAddr.sin_port = ::htons(m_port);

	// try to connect to peer host
	BOOL result = SocketAPI::connect_ex(m_socketID, (const struct sockaddr*)&m_sockAddr, sizeof(m_sockAddr));
	if (result)
		return TRUE;
	else
		return FALSE;
}

BOOL Socket::connect(const CHAR* host, UINT port)
{
	strncpy(m_host, host, IP_SIZE - 1);
	m_port = port;

	return connect();
}

BOOL Socket::reconnect(const CHAR* host, UINT port)
{
	// delete old socket impl object
	close();

	// create new socket impl object
	strncpy(m_host, host, IP_SIZE - 1);
	m_port = port;

	create();

	// try to connect
	return connect();
}

UINT Socket::send(const VOID* buf, UINT len, UINT flags /*= 0*/)
{
	return SocketAPI::send_ex(m_socketID, buf, len, flags);
}

UINT Socket::receive(VOID* buf, UINT len, UINT flags /*= 0*/)
{
	return SocketAPI::recv_ex(m_socketID, buf, len, flags);
}

UINT Socket::available() const
{
	return SocketAPI::availablesocket_ex(m_socketID);
}

SOCKET Socket::accept(struct sockaddr* addr, UINT* addrlen)
{
	return SocketAPI::accept_ex(m_socketID, addr, addrlen);
}

BOOL Socket::bind()
{
	m_sockAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	m_sockAddr.sin_port = ::htons(m_port);

	BOOL result = SocketAPI::bind_ex(m_socketID, (const struct sockaddr*)&m_sockAddr, sizeof(m_sockAddr));
	if (result)
		return TRUE;
	else
		return FALSE;
}

BOOL Socket::bind(UINT port)
{
	m_port = port;

	return bind();
}

BOOL Socket::listen(INT backlog)
{
	return SocketAPI::listen_ex(m_socketID, backlog);
}

UINT Socket::getLinger() const
{
	struct linger ling;
	UINT len = sizeof(ling);

	SocketAPI::getsockopt_ex(m_socketID, SOL_SOCKET, SO_LINGER, &ling, &len);

	return ling.l_linger;
}

BOOL Socket::setLinger(UINT lingertime)
{
	struct linger ling;

	ling.l_onoff = lingertime > 0 ? 1 : 0;
	ling.l_linger = lingertime;

	return SocketAPI::setsockopt_ex(m_socketID, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
}

BOOL Socket::isReuseAddr() const
{
	INT reuse;
	UINT len = sizeof(reuse);

	SocketAPI::getsockopt_ex(m_socketID, SOL_SOCKET, SO_REUSEADDR, &reuse, &len);

	return reuse == 1;
}

BOOL Socket::setReuseAddr(BOOL on /*= TRUE*/)
{
	INT opt = (on==TRUE ? 1 : 0);

	return SocketAPI::setsockopt_ex(m_socketID, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

UINT Socket::getSockError() const
{
	return isSocketError();
}

BOOL Socket::isNonBlocking() const
{
	return SocketAPI::getsocketnonblocking_ex(m_socketID);
}

BOOL Socket::setNonBlocking(BOOL on /*= TRUE*/)
{
	return SocketAPI::setsocketnonblocking_ex(m_socketID, on);
}

UINT Socket::getReceiveBufferSize() const
{
	UINT receiveBufferSize;
	UINT size = sizeof(receiveBufferSize);

	SocketAPI::getsockopt_ex(m_socketID, SOL_SOCKET, SO_RCVBUF, &receiveBufferSize, &size);

	return receiveBufferSize;
}

BOOL Socket::setReceiveBufferSize(UINT size)
{
	return (BOOL)(SocketAPI::setsockopt_ex(m_socketID, SOL_SOCKET, SO_RCVBUF, &size, sizeof(UINT)));
}

UINT Socket::getSendBufferSize() const
{
	UINT sendBufferSize;
	UINT size = sizeof(sendBufferSize);

	SocketAPI::getsockopt_ex(m_socketID, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, &size);

	return sendBufferSize;
}

BOOL Socket::setSendBufferSize(UINT size)
{
	return (BOOL)(SocketAPI::setsockopt_ex(m_socketID, SOL_SOCKET, SO_SNDBUF, &size, sizeof(UINT)));
}

UINT Socket::getPort() const
{
	return m_port;
}

ULONG Socket::getHostIP() const
{
	return (ULONG)(m_sockAddr.sin_addr.s_addr);
}

BOOL Socket::isValid() const
{
	return m_socketID != INVALID_SOCKET;
}

SOCKET Socket::getSocket() const
{
	return m_socketID;
}

BOOL Socket::isSocketError() const
{
	INT error;
	UINT len = sizeof(error);

	INT result = SocketAPI::getsockopt_ex2(m_socketID, SOL_SOCKET, SO_ERROR, &error, &len);

	if (result == 0)
		return FALSE;
	else
		return TRUE;
}
