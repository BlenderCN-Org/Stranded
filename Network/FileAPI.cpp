#include "FileAPI.h"

#if WIN32
#include <io.h>			// for _open()
#include <fcntl.h>		// for _open()/_close()/_read()/_write() ...
#include <string.h>		// for memcpy()
#elif __LINUX__
#include <sys/types.h>	// for open()
#include <sys/stat.h>	// for open()
#include <unistd.h>		// for fcntl()
#include <fcntl.h>		// for fcntl()
#include <sys/ioctl.h>	// for ioctl()
#include <errno.h>		// for errno
#endif

// external variables
#if __LINUX__
extern INT errno;
#endif

// exception version of open ()
INT FileAPI::open_ex(const CHAR* filename, INT flags)
{
#if __LINUX__
	INT fd = open(filename, flags);
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
	INT fd = ::_open(filename, flags);
#pragma warning(pop)
#endif
	if (fd < 0)
	{
#if __LINUX__
		switch (errno)
		{
		case EEXIST:
		case ENOENT:
		case EISDIR:
		case EACCES:
		case ENAMETOOLONG:
		case ENOTDIR:
		case ENXIO:
		case ENODEV:
		case EROFS:
		case ETXTBSY:
		case EFAULT:
		case ELOOP:
		case ENOSPC:
		case ENOMEM:
		case EMFILE:
		case ENFILE:
		default:
			break;
		}
#elif WIN32
#endif
	}
	return fd;
}

INT FileAPI::open_ex(const CHAR* filename, INT flags, INT mode)
{
#if __LINUX__
	INT fd = open(filename, flags, mode);
#elif WIN32
#pragma warning(push)
#pragma warning(disable:4996)
	INT fd = ::_open(filename, flags, mode);
#pragma warning(pop)
#endif
	if (fd < 0)
	{
#if __LINUX__
		switch (errno)
		{
		case EEXIST:
		case EISDIR:
		case EACCES:
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
		case ENXIO:
		case ENODEV:
		case EROFS:
		case ETXTBSY:
		case EFAULT:
		case ELOOP:
		case ENOSPC:
		case ENOMEM:
		case EMFILE:
		case ENFILE:
		default:
			break;
		}
#elif WIN32
#endif
	}
	return fd;
}

/*
 * @brief: exception version of close()
 * @param:
 *		fd - file descriptor
 */
VOID FileAPI::close_ex(INT fd)
{
#pragma warning(push)
#pragma warning(disable:4996)
	if (::close(fd) < 0)
#pragma warning(pop)
	{
#if __LINUX__
		switch (errno)
		{
		case EBADF:
		default:
			break;
		}
#elif WIN32
#endif
	}
}

/*
 * @brief: exception version of read()
 * @param:
 *		fd	- file descriptor
 *		buf	- reading buffer
 *		len	- reading length
 * @return:
 *		length of reading bytes
 */
UINT FileAPI::read_ex(INT fd, VOID* buf, UINT len)
{
#if __LINUX__
	INT result = read(fd, buf, len);
#elif WIN32
	INT result = _read(fd, buf, len);
#endif
	if (result < 0)
	{
#if __LINUX__
		switch (errno)
		{
		case EINTR:
		case EAGAIN:
		case EBADF:
		case EIO:
		case EISDIR:
		case EINVAL:
		case EFAULT:
		case ECONNRESET:
		default:
			break;
		}
#elif WIN32
#endif
	}
	else if (result == 0)
	{

	}
	return result;
}

/*
 * @brief: exception version of write()
 * @param:
 *		fd	- file descriptor
 *		buf	- writing buffer
 *		len	- writing length
 * @return:
 *		length of reading bytes
 */
UINT FileAPI::write_ex(INT fd, const VOID* buf, UINT len)
{
#if __LINUX__
	INT result = write(fd, buf, len);
#elif WIN32
	INT result = ::_write(fd, buf, len);
#endif
	if (result < 0)
	{
#if __LINUX__
		switch (errno)
		{
		case EAGAIN:
		case EINTR:
		case EBADF:
		case EPIPE:
		case EINVAL:
		case EFAULT:
		case ENOSPC:
		case EIO:
		case ECONNRESET:
		default:
			break;
		}
#elif WIN32
#endif
	}
	return result;
}

/*
 * @brief: exception version of fcntl()
 * @param:
 *		fd	- file descriptor
 *		cmd	- file control command
 * @return:
 *		various according to cmd
 */
INT FileAPI::fcntl_ex(INT fd, INT cmd)
{
#if __LINUX__
	INT result = fcntl(fd, cmd);
	if (result < 0)
	{
		switch (errno)
		{
		case EINTR:
		case EBADF:
		case EACCES:
		case EAGAIN:
		case EDEADLK:
		case EMFILE:
		case ENOLCK:
		default:
			break;
		}
	}
	return result;
#elif WIN32
	return 0;
#endif
}

/*
* @brief: exception version of fcntl()
* @param:
*		fd	- file descriptor
*		cmd	- file control command
*		arg	- command argument
* @return:
*		various according to cmd
*/
INT FileAPI::fcntl_ex(INT fd, INT cmd, LONG arg)
{
#if __LINUX__
	INT result = fcntl(fd, cmd, arg);
	if (result < 0)
	{
		switch (errno)
		{
		case EINTR:
		case EINVAL:
		case EBADF:
		case EACCES:
		case EAGAIN:
		case EDEADLK:
		case EMFILE:
		case ENOLCK:
		default:
			break;
		}
	}
	return result;
#elif WIN32
	return 0;
#endif
}

/*
 * @brief: is this stream is nonblocking?
 *			using fcntl_ex()
 * @param:
 *		fd - file descriptor
 * @return:
 *		TRUE if nonblocking, FALSE if blocking
 */
BOOL FileAPI::getfilenonblocking_ex(INT fd)
{
#if __LINUX__
	INT flags = fcntl_ex(fd, F_GETFL, 0);
	return flags | O_NONBLOCK;
#elif WIN32
	return FALSE;
#endif
}

/*
 * @brief: make this stream blocking/nonblocking
 *			using fcntl_ex()
 * @param:
 *		fd	- file descriptor
 *		on	- TRUE if nonblocking, FALSE if blocking
 */
VOID FileAPI::setfilenonblocking_ex(INT fd, BOOL on)
{
#if __LINUX__
	INT flags = fcntl_ex(fd, F_GETFL, 0);
	if (on)
		// make nonblocking fd
		flags |= O_NONBLOCK;
	else
		// make blocking fd
		flags &= ~O_NONBLOCK;

	fcntl_ex(fd, F_SETFL, flags);
#elif WIN32
#endif
}

/*
 * @brief: exception version of ioctl()
 * @param:
 *		fd		- file descriptor
 *		request	- i/o control request
 *		argp	- argument
 */
VOID FileAPI::ioctl_ex(INT fd, INT request, VOID* argp)
{
#if __LINUX__
	if (ioctl(fd, request, argp) < 0)
	{
		switch (errno)
		{
		case EBADF:
		case ENOTTY:
		case EINVAL:
		default:
			break;
		}
	}
#elif WIN32
#endif
}

/*
 * @brief: make this stream blocking/nonblocking using ioctl_ex()
 * @param:
 *		fd	- file descriptor
 *		on	- TRUE if nonblocking, FALSE else
 */
VOID FileAPI::setfilenonblocking_ex2(INT fd, BOOL on)
{
#if __LINUX__
	ULONG arg = (on==TRUE ? 1 : 0);
	ioctl_ex(fd, FIONBIO, &arg);
#elif WIN32
#endif
}

/*
 * @brief: how much bytes available in this stream? using ioctl_ex()
 * @param:
 *		fd - file descriptor
 * @return:
 *		#bytes avaiable
 */
UINT FileAPI::availablefile_ex(INT fd)
{
#if __LINUX__
	UINT arg = 0;
	ioctl_ex(fd, FIONREAD, &arg);
	return arg;
#elif WIN32
	return 0;
#endif
}

// exception version of dup()
INT FileAPI::dup_ex(INT fd)
{
#if __LINUX__
	INT newfd = dup(fd);
#elif WIN32
	INT newfd = _dup(fd);
#endif
	if (newfd < 0)
	{
#if __LINUX__
		switch (errno)
		{
		case EBADF:
		case EMFILE:
		default:
			break;
		}
#elif WIN32
#endif
	}
	return newfd;
}

// exception version of lseek()
LONG FileAPI::lseek_ex(INT fd, LONG offset, INT whence)
{
#if __LINUX__
	LONG result = lseek(fd, offset, whence);
	if (result < 0)
	{
		switch (errno)
		{
		case EBADF:
		case ESPIPE:
		case EINVAL:
		default:
			break;
		}
	}
#elif WIN32
	LONG result = _lseek(fd, offset, whence);
	if(result < 0)
	{ }
#endif
	return result;
}

LONG FileAPI::tell_ex(INT fd)
{
#if __LINUX__
	LONG result = 0L;
#elif WIN32
	LONG result = _tell(fd);
	if(result < 0)
	{ }
#endif
	return result;
}
