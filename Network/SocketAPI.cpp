#include "SocketAPI.h"
#include "FileAPI.h"

#if WIN32
#elif __LINUX__
#include <sys/types.h>			// for accept()
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>			// for inet_xxx()
#include <netinet/in.h>
#include <errno.h>				// for errno
#endif

// external variable
#if __LINUX__
extern INT errno;
#endif

CHAR Error[_ESIZE];

/*
 * @brief: exception version of socket()
 *
 * @param:
 *		domain - AF_INET(internet socket), AF_UNIX(internal socket), ...
 *		type - SOCKET_STREAM(TCP), SOCK_DGRAM(UDP), ...
 *		protocol - 0
 * @return:
 *		socket descriptor
 */
SOCKET SocketAPI::socket_ex(INT domain, INT type, INT protocol)
{
	SOCKET s = ::socket(domain, type, protocol);

	if (s == INVALID_SOCKET)
	{
#if __LINUX__
		switch (errno)
		{
		case EPROTONOSUPPORT:
		case EMFILE:
		case ENFILE:
		case EACCES:
		case ENOBUFS:
		default:
			break;
		}
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
		INT iErr = ::WSAGetLastError();
		switch (iErr)
		{
		case WSANOTINITIALISED:
			strncpy(Error, "WSANOTINITIALISED", _ESIZE);
			break;
		case WSAENETDOWN:
			strncpy(Error, "WSAENETDOWN", _ESIZE);
			break;
		case WSAEAFNOSUPPORT:
			strncpy(Error, "WSAEAFNOSUPPORT", _ESIZE);
			break;
		case WSAEINPROGRESS:
			strncpy(Error, "WSAEINPROGRESS", _ESIZE);
			break;
		case WSAEMFILE:
			strncpy(Error, "WSAEMFILE", _ESIZE);
			break;
		case WSAENOBUFS:
			strncpy(Error, "WSAENOBUFS", _ESIZE);
			break;
		case WSAEPROTONOSUPPORT:
			strncpy(Error, "WSAEPROTONOSUPPORT", _ESIZE);
			break;
		case WSAEPROTOTYPE:
			strncpy(Error, "WSAEPROTOTYPE", _ESIZE);
			break;
		case WSAESOCKTNOSUPPORT:
			strncpy(Error, "WSAESOCKTNOSUPPORT", _ESIZE);
			break;
		default:
			strncpy(Error, "UNKNOWN", _ESIZE);
			break;
		}
#pragma warning(pop)
#endif
	}

	return s;
}

/*
 * @brief: exception version of bind()
 *
 * @param:
 *		s - socket descriptor
 *		addr - socket address structure (normally struct sockaddr_in)
 *		addrlen - length of socket address structure
 * @return:
 *		none
 */
BOOL SocketAPI::bind_ex(SOCKET s, const struct sockaddr* addr, UINT addrlen)
{
	if (::bind(s, addr, addrlen) == SOCKET_ERROR)
	{
#if __LINUX__
		switch (errno)
		{
		case EADDRINUSE:
		case EINVAL:
		case EACCES:
		case ENOTSOCK:
		case EBADF:
		case EROFS:
		case EFAULT:
		case ENAMETOOLONG:
		case ENOENT:
		case ENOMEM:
		case ENOTDIR:
		case ELOOP:
		default:
			break;
		}
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
		INT iErr = ::WSAGetLastError();
		switch (iErr)
		{
		case WSANOTINITIALISED:
			strncpy(Error, "WSANOTINITIALISED", _ESIZE);
			break;
		case WSAENETDOWN:
			strncpy(Error, "WSAENETDOWN", _ESIZE);
			break;
		case WSAEADDRINUSE:
			strncpy(Error, "WSAEADDRINUSE", _ESIZE);
			break;
		case WSAEADDRNOTAVAIL:
			strncpy(Error, "WSAEADDRNOTAVAIL", _ESIZE);
			break;
		case WSAEFAULT:
			strncpy(Error, "WSAEFAULT", _ESIZE);
			break;
		case WSAEINPROGRESS:
			strncpy(Error, "WSAEINPROGRESS", _ESIZE);
			break;
		case WSAEINVAL:
			strncpy(Error, "WSAEINVAL", _ESIZE);
			break;
		case WSAENOBUFS:
			strncpy(Error, "WSAENOBUFS", _ESIZE);
			break;
		default:
			strncpy(Error, "UNKOWN", _ESIZE);
			break;
		}
#pragma warning(pop)
#endif
		return FALSE;
	}

	return TRUE;
}

/*
 * @brief: exception version of connect() system call
 *
 * @param:
 *		s - socket descriptor
 *		addr - socket address structure
 *		addrlen - length of socket address structure
 */
BOOL SocketAPI::connect_ex(SOCKET s, const struct sockaddr* addr, UINT addrlen)
{
	if (::connect(s, addr, addrlen) == SOCKET_ERROR)
	{
#if __LINUX__
		switch (errno)
		{
			switch (errno)
			{
			case EALREADY:
			case EINPROGRESS:
			case ECONNREFUSED:
			case EISCONN:
			case ETIMEDOUT:
			case ENETUNREACH:
			case EADDRINUSE:
			case EBADF:
			case EFAULT:
			case ENOTSOCK:
			default:
				break;
			}
		}
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
		INT iErr = ::WSAGetLastError();
		switch (iErr)
		{
		case WSANOTINITIALISED:
			strncpy(Error, "WSANOTINITIALISED", _ESIZE);
			break;
		case WSAENETDOWN:
			strncpy(Error, "WSAENETDOWN", _ESIZE);
			break;
		case WSAEADDRINUSE:
			strncpy(Error, "WSAEADDRINUSE", _ESIZE);
			break;
		case WSAEINTR:
			strncpy(Error, "WSAEINTR", _ESIZE);
			break;
		case WSAEINPROGRESS:
			strncpy(Error, "WSAEINPROGRESS", _ESIZE);
			break;
		case WSAEALREADY:
			strncpy(Error, "WSAEALREADY", _ESIZE);
			break;
		case WSAEADDRNOTAVAIL:
			strncpy(Error, "WSAEADDRNOTAVAIL", _ESIZE);
			break;
		case WSAEAFNOSUPPORT:
			strncpy(Error, "WSAEAFNOSUPPORT", _ESIZE);
			break;
		case WSAECONNREFUSED:
			strncpy(Error, "WSAECONNREFUSED", _ESIZE);
			break;
		case WSAEFAULT:
			strncpy(Error, "WSAEFAULT", _ESIZE);
			break;
		case WSAEINVAL:
			strncpy(Error, "WSAEINVAL", _ESIZE);
			break;
		case WSAEISCONN:
			strncpy(Error, "WSAEISCONN", _ESIZE);
			break;
		case WSAENETUNREACH:
			strncpy(Error, "WSAENETUNREACH", _ESIZE);
			break;
		case WSAENOBUFS:
			strncpy(Error, "WSAENOBUFS", _ESIZE);
			break;
		case WSAENOTSOCK:
			strncpy(Error, "WSAENOTSOCK", _ESIZE);
			break;
		case WSAETIMEDOUT:
			strncpy(Error, "WSAETIMEDOUT", _ESIZE);
			break;
		case WSAEWOULDBLOCK:
			strncpy(Error, "WSAEWOULDBLOCK", _ESIZE);
			break;
		default:
			strncpy(Error, "UNKNOWN", _ESIZE);
			break;
		}
#pragma warning(pop)
#endif
		return FALSE;
	}
	return TRUE;
}

/*
 * @brief: exception version of listern()
 *
 * @param:
 *		s - socket descriptor
 *		backlog - waiting queue length
 * @return:
 *		none
 */
BOOL SocketAPI::listen_ex(SOCKET s, UINT backlog)
{
	if (::listen(s, backlog) == SOCKET_ERROR)
	{
#if __LINUX__
		switch (errno)
		{
		case EBADF:
		case ENOTSOCK:
		case EOPNOTSUPP:
		default:
			break;
		}
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
		INT iErr = ::WSAGetLastError();
		switch (iErr)
		{
		case WSANOTINITIALISED:
			strncpy(Error, "WSANOTINITIALISED", _ESIZE);
			break;
		case WSAENETDOWN:
			strncpy(Error, "WSAENETDOWN", _ESIZE);
			break;
		case WSAEADDRINUSE:
			strncpy(Error, "WSAEADDRINUSE", _ESIZE);
			break;
		case WSAEINPROGRESS:
			strncpy(Error, "WSAEINPROGRESS", _ESIZE);
			break;
		case WSAEINVAL:
			strncpy(Error, "WSAEINVAL", _ESIZE);
			break;
		case WSAEISCONN:
			strncpy(Error, "WSAEISCONN", _ESIZE);
			break;
		case WSAEMFILE:
			strncpy(Error, "WSAEMFILE", _ESIZE);
			break;
		case WSAENOBUFS:
			strncpy(Error, "WSAENOBUFS", _ESIZE);
			break;
		case WSAENOTSOCK:
			strncpy(Error, "WSAENOTSOCK", _ESIZE);
			break;
		case WSAEOPNOTSUPP:
			strncpy(Error, "WSAEOPNOTSUPP", _ESIZE);
			break;
		default:
			strncpy(Error, "UNKNOWN", _ESIZE);
			break;
		}
#pragma warning(pop)
#endif
		return FALSE;
	}
	return TRUE;
}

/*
 * @brief: exception version of accept()
 *
 * @param:
 *		s - socket descriptor
 *		addr - socket address structure
 *		addrlen - length of socket address structure
 */
SOCKET SocketAPI::accept_ex(SOCKET s, struct sockaddr* addr, UINT* addrlen)
{
#if __LINUX__
	SOCKET client = accept(s, addr, addrlen);
#elif WIN32
	SOCKET client = ::accept(s, addr, (int*)addrlen);
#endif
	if (client == INVALID_SOCKET)
	{
#if __LINUX__
		switch (errno)
		{
		case EWOULDBLOCK:

		case ECONNRESET:
		case EPROTO:
		case EINTR:
			// from UNIX Network Programming 2nd, 15.6
			// with nonblocking-socket, ignore above errors
		case EBADF:
		case ENOTSOCK:
		case EOPNOTSUPP:
		case EFAULT:
		default:
			break;
		}
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
		INT iErr = ::WSAGetLastError();
		switch (iErr)
		{
		case WSANOTINITIALISED:
			strncpy(Error, "WSANOTINITIALISED", _ESIZE);
			break;
		case WSAENETDOWN:
			strncpy(Error, "WSAENETDOWN", _ESIZE);
			break;
		case WSAEFAULT:
			strncpy(Error, "WSAEFAULT", _ESIZE);
			break;
		case WSAEINTR:
			strncpy(Error, "WSAEINTR", _ESIZE);
			break;
		case WSAEINPROGRESS:
			strncpy(Error, "WSAEINPROGRESS", _ESIZE);
			break;
		case WSAEINVAL:
			strncpy(Error, "WSAEINVAL", _ESIZE);
			break;
		case WSAEMFILE:
			strncpy(Error, "WSAEMFILE", _ESIZE);
			break;
		case WSAENOBUFS:
			strncpy(Error, "WSAENOBUFS", _ESIZE);
			break;
		case WSAENOTSOCK:
			strncpy(Error, "WSAENOTSOCK", _ESIZE);
			break;
		case WSAEOPNOTSUPP:
			strncpy(Error, "WSAEOPNOTSUPP", _ESIZE);
			break;
		case WSAEWOULDBLOCK:
			strncpy(Error, "WSAEWOULDBLOCK", _ESIZE);
			break;
		default:
			strncpy(Error, "UNKNOWN", _ESIZE);
			break;
		}
#pragma warning(pop)
#endif
	}
	return client;
}

/*
 * @brief: exception version of getsockopt()
 *
 * @param:
 *		s		- socket descriptor
 *		level	- socket option level (SOL_SOCKET, ...)
 *		optname	- socket option name (SO_REUSEADDR, SO_LNGER, ...)
 *		optval	- pointer to contain option value
 *		optlen	- length of optval
 */
BOOL SocketAPI::getsockopt_ex(SOCKET s, INT level, INT optname, VOID* optval, UINT* optlen)
{
#if __LINUX__
	if (getsockopt(s, level, optname, optval, optlen) == SOCKET_ERROR)
	{
		switch (errno)
		{
		case EBADF:
		case ENOTSOCK:
		case ENOPROTOOPT:
		case EFAULT:
		default:
			break;
		}
		return FALSE;
	}
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
	if (::getsockopt(s, level, optname, (CHAR*)optval, (INT*)optlen) == SOCKET_ERROR)
	{
		INT iErr = ::WSAGetLastError();
		switch (iErr)
		{
		case WSANOTINITIALISED:
			strncpy(Error, "WSANOTINITIALISED", _ESIZE);
			break;
		case WSAENETDOWN:
			strncpy(Error, "WSAENETDOWN", _ESIZE);
			break;
		case WSAEFAULT:
			strncpy(Error, "WSAEFAULT", _ESIZE);
			break;
		case WSAEINPROGRESS:
			strncpy(Error, "WSAEINPROGRESS", _ESIZE);
			break;
		case WSAEINVAL:
			strncpy(Error, "WSAEINVAL", _ESIZE);
			break;
		case WSAENOPROTOOPT:
			strncpy(Error, "WSAENOPROTOOPT", _ESIZE);
			break;
		case WSAENOTSOCK:
			strncpy(Error, "WSAENOTSOCK", _ESIZE);
			break;
		default:
			strncpy(Error, "UNKNOWN", _ESIZE);
			break;
		}
		return FALSE;
	}
#pragma warning(pop)
#endif
	return TRUE;
}

UINT SocketAPI::getsockopt_ex2(SOCKET s, INT level, INT optname, VOID* optval, UINT* optlen)
{
#if __LINUX__
	if (getsockopt(s, level, optname, optval, optlen) == SOCKET_ERROR)
	{
		switch (errno)
		{
		case EBADF:
			return 1;
		case ENOTSOCK:
			return 2;
		case ENOPROTOOPT:
			return 3;
		case EFAULT:
			return 4;
		default:
			return 5;
		}
	}
	return 0;
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
	if (::getsockopt(s, level, optname, (CHAR*)optval, (int*)optlen) == SOCKET_ERROR)
	{
		INT iErr = ::WSAGetLastError();
		switch (iErr)
		{
		case WSANOTINITIALISED:
			strncpy(Error, "WSANOTINITIALISED", _ESIZE);
			break;
		case WSAENETDOWN:
			strncpy(Error, "WSAENETDOWN", _ESIZE);
			break;
		case WSAEFAULT:
			strncpy(Error, "WSAEFAULT", _ESIZE);
			break;
		case WSAEINPROGRESS:
			strncpy(Error, "WSAEINPROGRESS", _ESIZE);
			break;
		case WSAEINVAL:
			strncpy(Error, "WSAEINVAL", _ESIZE);
			break;
		case WSAENOPROTOOPT:
			strncpy(Error, "WSAENOPROTOOPT", _ESIZE);
			break;
		case WSAENOTSOCK:
			strncpy(Error, "WSAENOTSOCK", _ESIZE);
			break;
		default:
			strncpy(Error, "UNKNOWN", _ESIZE);
			break;
		}
	}
#pragma warning(pop)
#endif
	return 0;
}

/*
 * @brief: exception version of setsockopt()
 * @param:
 *		s		- socket descriptor
 *		level	- socket option level (SOL_SOCKET, ...)
 *		optname	- socket option name (SO_REUSEADDR, SO_LINGER, ...)
 *		optval	- pointer to contain option value
 *		optlen	- length of optval
 */
BOOL SocketAPI::setsockopt_ex(SOCKET s, INT level, INT optname, const VOID* optval, UINT optlen)
{
#if __LINUX__
	if (setsockopt(s, level, optname, optval, optlen) == SOCKET_ERROR)
	{
		switch (errno)
		{
		case EBADF:
		case ENOTSOCK:
		case ENOPROTOOPT:
		case EFAULT:
		default:
			break;
		}
		return FALSE;
	}
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
	if (::setsockopt(s, level, optname, (CHAR*)optval, optlen) == SOCKET_ERROR)
	{
		INT iErr = ::WSAGetLastError();
		switch (iErr)
		{
		case WSANOTINITIALISED:
			strncpy(Error, "WSANOTINITIALISED", _ESIZE);
			break;
		case WSAENETDOWN:
			strncpy(Error, "WSAENETDOWN", _ESIZE);
			break;
		case WSAEFAULT:
			strncpy(Error, "WSAEFAULT", _ESIZE);
			break;
		case WSAEINPROGRESS:
			strncpy(Error, "WSAEINPROGRESS", _ESIZE);
			break;
		case EINVAL:
			strncpy(Error, "EINVAL", _ESIZE);
			break;
		case WSAENETRESET:
			strncpy(Error, "WSAENETRESET", _ESIZE);
			break;
		case WSAENOPROTOOPT:
			strncpy(Error, "WSAENOPROTOOPT", _ESIZE);
			break;
		case WSAENOTCONN:
			strncpy(Error, "WSAENOTCONN", _ESIZE);
			break;
		case WSAENOTSOCK:
			strncpy(Error, "WSAENOTSOCK", _ESIZE);
			break;
		default:
			strncpy(Error, "UNKNOWN", _ESIZE);
			break;
		}
		return FALSE;
	}
#pragma warning(pop)
#endif
	return TRUE;
}

/*
 * @brief: exception version of send()
 * @param:
 *		s		- socket descriptor
 *		buf		- input buffer
 *		len		- input data length
 *		flags	- send flag (MSG_OOB, MSG_DONTROUTE)
 * @return
 *		length of bytes sent
 */
UINT SocketAPI::send_ex(SOCKET s, const VOID* buf, UINT len, UINT flags)
{
	INT nSent;
	try
	{
#if __LINUX__
		nSent = send(s, buf, len, flags);
#elif WIN32
		nSent = ::send(s, (const CHAR*)buf, len, flags);
#endif
		if (nSent == SOCKET_ERROR)
		{
#if __LINUX__
			switch (errno)
			{
			case EWOULDBLOCK:
				return SOCKET_ERROR_WOULDBLOCK;
			case ECONNRESET:
			case EPIPE:

			case EBADF:
			case ENOTSOCK:
			case EFAULT:
			case EMSGSIZE:
			case ENOBUFS:

			default:
				break;
			}
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
			INT iErr = ::WSAGetLastError();
			switch (iErr)
			{
			case WSANOTINITIALISED:
				strncpy(Error, "WSANOTINITIALISED", _ESIZE);
				break;
			case WSAENETDOWN:
				strncpy(Error, "WSAENETDOWN", _ESIZE);
				break;
			case WSAEACCES:
				strncpy(Error, "WSAEACCES", _ESIZE);
				break;
			case WSAEINTR:
				strncpy(Error, "WSAEINTR", _ESIZE);
				break;
			case WSAEINPROGRESS:
				strncpy(Error, "WSAEINPROGRESS", _ESIZE);
				break;
			case WSAEFAULT:
				strncpy(Error, "WSAEFAULT", _ESIZE);
				break;
			case WSAENETRESET:
				strncpy(Error, "WSAENETRESET", _ESIZE);
				break;
			case WSAENOBUFS:
				strncpy(Error, "WSAENOBUFS", _ESIZE);
				break;
			case WSAENOTCONN:
				strncpy(Error, "WSAENOTCONN", _ESIZE);
				break;
			case WSAENOTSOCK:
				strncpy(Error, "WSAENOTSOCK", _ESIZE);
				break;
			case WSAEOPNOTSUPP:
				strncpy(Error, "WSAEOPNOTSUPP", _ESIZE);
				break;
			case WSAESHUTDOWN:
				strncpy(Error, "WSAESHUTDOWN", _ESIZE);
				break;
			case WSAEWOULDBLOCK:
				return SOCKET_ERROR_WOULDBLOCK;
			case WSAEMSGSIZE:
				strncpy(Error, "WSAEMSGSIZE", _ESIZE);
				break;
			case WSAEHOSTUNREACH:
				strncpy(Error, "WSAEHOSTUNREACH", _ESIZE);
				break;
			case WSAEINVAL:
				strncpy(Error, "WSAEINVAL", _ESIZE);
				break;
			case WSAECONNABORTED:
				strncpy(Error, "WSAECONNABORTED", _ESIZE);
				break;
			case WSAECONNRESET:
				strncpy(Error, "WSAECONNRESET", _ESIZE);
				break;
			case WSAETIMEDOUT:
				strncpy(Error, "WSAETIMEDOUT", _ESIZE);
				break;
			default:
				strncpy(Error, "UNKNOWN", _ESIZE);
				break;
			}
#pragma warning(pop)
#endif
		}
		else if (nSent == 0)
		{
		}
	}
	catch (...)
	{

	}

	return nSent;
}

// exception version of sendto()
UINT SocketAPI::sendto_ex(SOCKET s, const VOID* buf, INT len, UINT flags, const struct sockaddr* to, INT tolen)
{
#if __LINUX__
	INT nSent = sendto(s, buf, len, flags, to, tolen);
#elif WIN32
	INT nSent = ::sendto(s, (const CHAR*)buf, len, flags, to, tolen);
#endif
	if (nSent == SOCKET_ERROR)
	{
#if __LINUX__
		switch (errno)
		{
		case EWOULDBLOCK:
			return 0;
		case ECONNRESET:
		case EPIPE:

		case EBADF:
		case ENOTSOCK:
		case EFAULT:
		case EMSGSIZE:
		case ENOBUFS:

		default:
			break;
		}
#elif WIN32
#endif
	}
	return nSent;
}

/*
 * @brief: exception version of recv()
 * @param:
 *		s		- socket descriptor
 *		buf		- input buffer
 *		len		- input data length
 *		flags	- send flag (MSG_OOB, MSG_DONTROUTE)
 * @return:
 *		length of bytes received
 */
UINT SocketAPI::recv_ex(SOCKET s, VOID* buf, UINT len, UINT flags)
{
#if __LINUX__
	INT nRecv = recv(s, buf, len, float);
#elif WIN32
	INT nRecv = ::recv(s, (CHAR*)buf, len, flags);
#endif
	if (nRecv == SOCKET_ERROR)
	{
#if __LINUX__
		switch (errno)
		{
		case EWOULDBLOCK:
			return SOCKET_ERROR_WOULDBLOCK;
		case ECONNRESET:
		case EPIPE:

		case EBADF:
		case ENOTCONN:
		case ENOTSOCK:
		case EINTR:
		case EFAULT:

		default:
			break;
		}
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
		INT iErr = ::WSAGetLastError();
		switch (iErr)
		{
		case WSANOTINITIALISED:
			strncpy(Error, "WSANOTINITIALISED", _ESIZE);
			break;
		case WSAENETDOWN:
			strncpy(Error, "WSAENETDOWN", _ESIZE);
			break;
		case WSAEFAULT:
			strncpy(Error, "WSAEFAULT", _ESIZE);
			break;
		case WSAENOTCONN:
			strncpy(Error, "WSAENOTCONN", _ESIZE);
			break;
		case WSAEINTR:
			strncpy(Error, "WSAEINTR", _ESIZE);
			break;
		case WSAEINPROGRESS:
			strncpy(Error, "WSAEINPROGRESS", _ESIZE);
			break;
		case WSAENETRESET:
			strncpy(Error, "WSAENETRESET", _ESIZE);
			break;
		case WSAENOTSOCK:
			strncpy(Error, "WSAENOTSOCK", _ESIZE);
			break;
		case WSAEOPNOTSUPP:
			strncpy(Error, "WSAEOPNOTSUPP", _ESIZE);
			break;
		case WSAESHUTDOWN:
			strncpy(Error, "WSAESHUTDOWN", _ESIZE);
			break;
		case WSAEWOULDBLOCK:
			return SOCKET_ERROR_WOULDBLOCK;
		case WSAECONNABORTED:
			strncpy(Error, "WSAECONNABORTED", _ESIZE);
			break;
		case WSAETIMEDOUT:
			strncpy(Error, "WSAETIMEDOUT", _ESIZE);
			break;
		case WSAECONNRESET:
			strncpy(Error, "WSAECONNRESET", _ESIZE);
			break;
		default:
			strncpy(Error, "UNKNOWN", _ESIZE);
			break;
		}
#pragma warning(pop)
#endif
	}
	else if (nRecv == 0)
	{

	}

	return nRecv;
}

// exception version of recvfrom()
UINT SocketAPI::recvfrom_ex(SOCKET s, VOID* buf, INT len, UINT flags, struct sockaddr* from, UINT* fromlen)
{
#if __LINUX__
	INT nReceived = recvfrom(s, buf, len, flags, from, fromlen);
#elif WIN32
	INT nReceived = ::recvfrom(s, (CHAR*)buf, len, flags, from, (int*)fromlen);
#endif
	if (nReceived == SOCKET_ERROR)
	{
#if __LINUX__
		switch (errno)
		{
		case EWOULDBLOCK:
			return SOCKET_ERROR_WOULDBLOCK;
		case ECONNRESET:
		case EPIPE:

		case EBADF:
		case ENOTCONN:
		case ENOTSOCK:
		case EINTR:
		case EFAULT:

		default:
			break;
		}
#elif WIN32
#endif
	}
	return nReceived;
}

/*
 * @brief: exception version of closesocket()
 * *CAUTION*
 *		in UNIX, close() used instead
 * *param:
 *		s - socket descriptor
 */
BOOL SocketAPI::closesocket_ex(SOCKET s)
{
#if __LINUX__
	close(s);
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
	if (::closesocket(s) == SOCKET_ERROR)
	{
		INT iErr = ::WSAGetLastError();
		switch (iErr)
		{
		case WSANOTINITIALISED:
			strncpy(Error, "WSANOTINITIALISED", _ESIZE);
			break;
		case WSAENETDOWN:
			strncpy(Error, "WSAENETDOWN", _ESIZE);
			break;
		case WSAENOTSOCK:
			strncpy(Error, "WSAENOTSOCK", _ESIZE);
			break;
		case WSAEINPROGRESS:
			strncpy(Error, "WSAEINPROGRESS", _ESIZE);
			break;
		case WSAEINTR:
			strncpy(Error, "WSAEINTR", _ESIZE);
			break;
		case WSAEWOULDBLOCK:
			strncpy(Error, "WSAEWOULDBLOCK", _ESIZE);
			break;
		default:
			strncpy(Error, "UNKNOWN", _ESIZE);
			break;
		}
		return FALSE;
	}
#pragma warning(pop)
#endif
	return TRUE;
}

/*
 * @brief: exception version of ioctlsocket()
 * *CAUTION*
 *		in UNIX, ioctl() used instead
 */
BOOL SocketAPI::ioctlsocket_ex(SOCKET s, LONG cmd, ULONG* argp)
{
#if __LINUX__
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
	if (::ioctlsocket(s, cmd, argp) == SOCKET_ERROR)
	{
		INT iErr = ::WSAGetLastError();
		switch (iErr)
		{
		case WSANOTINITIALISED:
			strncpy(Error, "WSANOTINITIALISED", _ESIZE);
			break;
		case WSAENETDOWN:
			strncpy(Error, "WSAENETDOWN", _ESIZE);
			break;
		case WSAEINPROGRESS:
			strncpy(Error, "WSAEINPROGRESS", _ESIZE);
			break;
		case WSAENOTSOCK:
			strncpy(Error, "WSAENOTSOCK", _ESIZE);
			break;
		case WSAEFAULT:
			strncpy(Error, "WSAEFAULT", _ESIZE);
			break;
		default:
			strncpy(Error, "UNKNOWN", _ESIZE);
			break;
		}
		return FALSE;
	}
#pragma warning(pop)
#endif
	return TRUE;
}

/*
 * @brief: check if this socket is nonblocking mode
 * @param:
 *		s - socket descriptor
 * @return:
 *		TRUE if nonblocking, FALSE if blocking
 */
BOOL SocketAPI::getsocketnonblocking_ex(SOCKET s)
{
#if __LINUX__
	INT flags = FileAPI::fcntl_ex(s, F_GETFL, 0);
	return flags | O_NONBLOCK;
#elif WIN32
	return FALSE;
#endif
}

/*
 * @brief: make this socket blocking/nonblocking
 * @param:
 *		s	- socket descriptor
 *		on	- TURE if nonblocking, FALSE if blocking
 */
BOOL SocketAPI::setsocketnonblocking_ex(SOCKET s, BOOL on)
{
#if __LINUX__
	INT flags = FileAPI::fcntl_ex(s, F_GETFL, 0);
	if (on)
		// make nonblocking fd
		flags |= O_NONBLOCK;
	else
		// make blocking fd
		flags &= ~O_NONBLOCK;

	FileAPI::fcntl_ex(s, F_SETFL, flags);

	return TRUE;
#elif WIN32
	ULONG argp = (on==TRUE) ? 1 : 0;
	return ioctlsocket_ex(s, FIONBIO, &argp);
#endif
}

/*
 * @brief: get amount of data in socket input buffer
 * @param:
 *		s - socket descriptor
 * @return:
 *		amount of data in socket input buffer
 */
UINT SocketAPI::availablesocket_ex(SOCKET s)
{
#if __LINUX__
	return FileAPI::availablefile_ex(s);
#elif WIN32
	ULONG argp = 0;
	ioctlsocket_ex(s, FIONREAD, &argp);
	return argp;
#endif
}

/*
 * @brief: shutdown all or part of connection of socket
 * @param:
 *		s	- socket descriptor
 *		how	- how to close (all, send, receive)
 */
BOOL SocketAPI::shutdown_ex(SOCKET s, UINT how)
{
	if (::shutdown(s, how) < 0)
	{
#if __LINUX__
		switch (errno)
		{
		case EBADF:
		case ENOTSOCK:
		case ENOTCONN:
		default:
			break;
		}
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
		INT iErr = ::WSAGetLastError();
		switch (iErr)
		{
		case WSANOTINITIALISED:
			strncpy(Error, "WSANOTINITIALISED", _ESIZE);
			break;
		case WSAENETDOWN:
			strncpy(Error, "WSAENETDOWN", _ESIZE);
			break;
		case WSAEINVAL:
			strncpy(Error, "WSAEINVAL", _ESIZE);
			break;
		case WSAEINPROGRESS:
			strncpy(Error, "WSAEINPROGRESS", _ESIZE);
			break;
		case WSAENOTCONN:
			strncpy(Error, "WSAENOTCONN", _ESIZE);
			break;
		case WSAENOTSOCK:
			strncpy(Error, "WSAENOTSOCK", _ESIZE);
			break;
		default:
			strncpy(Error, "UNKNOWN", _ESIZE);
			break;
		}
#pragma warning(pop)
#endif
		return FALSE;
	}
	return TRUE;
}

/*
 * @brief: system call for I/O multiplexing
 * @return:
 *	positive count of ready descriptors
 */
INT SocketAPI::select_ex(INT maxfdp1, fd_set* readset, fd_set* writeset, fd_set* exceptset, struct timeval* timeout)
{
	INT result;
	try
	{
		result = ::select(maxfdp1, readset, writeset, exceptset, timeout);
		if (result == SOCKET_ERROR)
		{
#if __LINUX__
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
			INT iErr = ::WSAGetLastError();
			switch (iErr)
			{
			case WSANOTINITIALISED:
				strncpy(Error, "WSANOTINITIALISED", _ESIZE);
				break;
			case WSAEFAULT:
				strncpy(Error, "WSAEFAULT", _ESIZE);
				break;
			case WSAENETDOWN:
				strncpy(Error, "WSAENETDOWN", _ESIZE);
				break;
			case WSAEINVAL:
				strncpy(Error, "WSAEINVAL", _ESIZE);
				break;
			case WSAEINTR:
				strncpy(Error, "WSAEINTR", _ESIZE);
				break;
			case WSAEINPROGRESS:
				strncpy(Error, "WSAEINPROGRESS", _ESIZE);
				break;
			case WSAENOTSOCK:
				strncpy(Error, "WSAENOTSOCK", _ESIZE);
				break;
			default:
				strncpy(Error, "UNKNOWN", _ESIZE);
				break;
			}
#pragma warning(pop)
#endif
		}
	}
	catch (...)
	{
	}

	return result;
}
