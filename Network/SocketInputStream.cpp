#include "SocketInputStream.h"
#include "Packet.h"

SocketInputStream::SocketInputStream(Socket* sock, UINT bufferSize /*= DEFAULTSOCKETINPUTBUFFERSIZE*/, UINT maxBufferSize /*= DISCONNECTSOCKETINPUTSIZE*/)
{
	m_pSocket = sock;
	m_bufferLen = bufferSize;
	m_maxBufferLen = maxBufferSize;

	m_head = 0;
	m_tail = 0;

	m_buffer = new CHAR[m_bufferLen];
	memset(m_buffer, 0, m_bufferLen);
}

SocketInputStream::~SocketInputStream()
{
	SAFE_DELETE_ARRAY(m_buffer);
}

// 返回0表示没有读到数据
UINT SocketInputStream::Read(CHAR* buf, UINT len)
{
	assert(buf != NULL);

	if (len == 0)
		return 0;

	if (len > Length())
		return 0;

	if (m_head < m_tail)
	{
		memcpy(buf, &m_buffer[m_head], len);
	}
	else
	{
		UINT rightLen = m_bufferLen - m_head;
		if (len <= rightLen)
		{
			memcpy(buf, &m_buffer[m_head], len);
		}
		else
		{
			memcpy(buf, &m_buffer[m_head], rightLen);
			memcpy(&buf[rightLen], m_buffer, len - rightLen);
		}
	}

	m_head = (m_head + len) % m_bufferLen;

	return len;
}

BOOL SocketInputStream::ReadPacket(Packet* p)
{
	assert(p != nullptr);

	BOOL ret;

	ret = Skip(PACKET_HEADER_SIZE);
	if (!ret)
		return FALSE;

	return p->Read(*this);
}

BOOL SocketInputStream::Peek(CHAR* buf, UINT len)
{
	assert(buf != nullptr);

	if (len == 0)
		return FALSE;

	if (len > Length())
		return FALSE;

	if (m_head < m_tail)
	{
		memcpy(buf, &m_buffer[m_head], len);
	}
	else
	{
		UINT rightLen = m_bufferLen - m_head;
		if (len <= rightLen)
		{
			memcpy(&buf[0], &m_buffer[m_head], len);
		}
		else
		{
			memcpy(&buf[0], &m_buffer[m_head], rightLen);
			memcpy(&buf[rightLen], &m_buffer[0], len - rightLen);
		}
	}

	return TRUE;
}

BOOL SocketInputStream::Find(CHAR* buf)
{
	assert(buf != nullptr);

	BOOL isFnd = FALSE;
	while (Length() > PACK_COMPART_SIZE)
	{
		if (m_head < m_tail)
		{
			memcpy(buf, &m_buffer[m_head], PACK_COMPART_SIZE);
		}
		else
		{
			UINT rightLen = m_bufferLen - m_head;
			if (PACK_COMPART_SIZE <= rightLen)
			{
				memcpy(&buf[0], &m_buffer[m_head], PACK_COMPART_SIZE);
			}
			else
			{
				memcpy(&buf[0], &m_buffer[m_head], rightLen);
				memcpy(&buf[rightLen], &m_buffer[0], PACK_COMPART_SIZE - rightLen);
			}
		}

		if (strstr(buf, PACK_COMPART) == NULL)
		{
			m_head++;
			if (m_head >= m_bufferLen)
				m_head -= m_bufferLen;

			continue;
		}
		else
		{
			isFnd = TRUE;
			break;
		}
	}

	if (!isFnd)
		return FALSE;

	m_head += (UINT)PACK_COMPART_SIZE;
	if (m_head >= m_bufferLen)
		m_head -= m_bufferLen;

	return TRUE;
}

BOOL SocketInputStream::Skip(UINT len)
{
	if (len == 0)
		return FALSE;

	if (len > Length())
		return FALSE;

	m_head = (m_head + len) % m_bufferLen;

	return TRUE;
}

UINT SocketInputStream::Fill()
{
	assert(m_pSocket != nullptr);

	UINT nFilled = 0;
	UINT nReceived = 0;
	UINT nFree = 0;

	if (m_head <= m_tail)
	{
		if (m_head == 0)
		{
			//
			// H   T		LEN=10
			// 0123456789
			// abcd......
			//

			nReceived = 0;
			nFree = m_bufferLen - m_tail - 1;
			if (nFree != 0)
			{
				nReceived = m_pSocket->receive(&m_buffer[m_tail], nFree);
				if (nReceived == SOCKET_ERROR_WOULDBLOCK) return 0;
				if (nReceived == SOCKET_ERROR) return SOCKET_ERROR - 1;
				if (nReceived == 0) return SOCKET_ERROR - 2;

				m_tail += nReceived;
				nFilled += nReceived;
			}

			if (nReceived == nFree)
			{
				UINT available = m_pSocket->available();
				if (available > 0)
				{
					if (m_bufferLen + available + 1 > m_maxBufferLen)
					{
						Initsize();
						return SOCKET_ERROR - 3;
					}

					if (!Resize(available + 1))
						return 0;

					nReceived = m_pSocket->receive(&m_buffer[m_tail], available);
					if (nReceived == SOCKET_ERROR_WOULDBLOCK) return 0;
					if (nReceived == SOCKET_ERROR) return SOCKET_ERROR - 4;
					if (nReceived == 0) return SOCKET_ERROR - 5;

					m_tail += nReceived;
					nFilled += nReceived;
				}
			}
		}
		else
		{
			//
			//    H   T		LEN=10
			// 0123456789
			// ...abcd...
			//

			nFree = m_bufferLen - m_tail;
			nReceived = m_pSocket->receive(&m_buffer[m_tail], nFree);
			if (nReceived == SOCKET_ERROR_WOULDBLOCK) return 0;
			if (nReceived == SOCKET_ERROR) return SOCKET_ERROR - 6;
			if (nReceived == 0) return SOCKET_ERROR - 7;

			m_tail = (m_tail + nReceived) % m_bufferLen;
			nFilled += nReceived;

			if (nReceived == nFree)
			{
				nReceived = 0;
				nFree = m_head - 1;
				if (nFree != 0)
				{
					nReceived = m_pSocket->receive(&m_buffer[0], nFree);
					if (nReceived == SOCKET_ERROR_WOULDBLOCK) return 0;
					if (nReceived == SOCKET_ERROR) return SOCKET_ERROR - 8;
					if (nReceived == 0) return SOCKET_ERROR - 9;

					m_tail += nReceived;
					nFilled += nReceived;
				}

				if (nReceived == nFree)
				{
					UINT available = m_pSocket->available();
					if (available > 0)
					{
						if ((m_bufferLen + available + 1) > m_maxBufferLen)
						{
							Initsize();
							return SOCKET_ERROR - 10;
						}

						if (!Resize(available + 1))
							return 0;

						nReceived = m_pSocket->receive(&m_buffer[m_tail], available);
						if (nReceived == SOCKET_ERROR_WOULDBLOCK) return 0;
						if (nReceived == SOCKET_ERROR) return SOCKET_ERROR - 11;
						if (nReceived == 0) return SOCKET_ERROR - 12;

						m_tail += nReceived;
						nFilled += nReceived;
					}
				}
			}
		}
	}
	else
	{
		//
		//     T  H		LEN=10
		// 0123456789
		// abcd...efg
		//

		nReceived = 0;
		nFree = m_head - m_tail - 1;
		if (nFree != 0)
		{
			nReceived = m_pSocket->receive(&m_buffer[m_tail], nFree);
			if (nReceived == SOCKET_ERROR_WOULDBLOCK) return 0;
			if (nReceived == SOCKET_ERROR) return SOCKET_ERROR - 13;
			if (nReceived == 0) return SOCKET_ERROR - 14;

			m_tail += nReceived;
			nFilled += nReceived;
		}

		if (nReceived == nFree)
		{
			UINT available = m_pSocket->available();
			if (available > 0)
			{
				if ((m_bufferLen + available + 1) > m_maxBufferLen)
				{
					Initsize();
					return SOCKET_ERROR - 15;
				}

				if (!Resize(available + 1))
					return 0;

				nReceived = m_pSocket->receive(&m_buffer[m_tail], available);
				if (nReceived == SOCKET_ERROR_WOULDBLOCK) return 0;
				if (nReceived == SOCKET_ERROR) return SOCKET_ERROR - 16;
				if (nReceived == 0) return SOCKET_ERROR - 17;

				m_tail += nReceived;
				nFilled += nReceived;
			}
		}
	}

	return nFilled;
}

VOID SocketInputStream::Initsize()
{
	m_head = 0;
	m_tail = 0;

	SAFE_DELETE_ARRAY(m_buffer);

	m_buffer = new CHAR[DEFAULTSOCKETINPUTBUFFERSIZE];

	m_bufferLen = DEFAULTSOCKETINPUTBUFFERSIZE;

	memset(m_buffer, 0, m_bufferLen);
}

BOOL SocketInputStream::Resize(INT size)
{
	size = max(size, (int)(m_bufferLen >> 1));
	UINT newBufferLen = m_bufferLen + size;
	UINT len = Length();

	if (size > 0)
	{
		if (newBufferLen < 0 || newBufferLen < len)
			return FALSE;
	}

	CHAR* newBuffer = new CHAR[newBufferLen];

	if (m_head < m_tail)
	{
		memcpy(newBuffer, &m_buffer[m_head], m_tail - m_head);
	}
	else if (m_head > m_tail)
	{
		memcpy(newBuffer, &m_buffer[m_head], m_bufferLen - m_head);
		memcpy(&newBuffer[m_bufferLen - m_head], m_buffer, m_tail);
	}

	delete[] m_buffer;

	m_buffer = newBuffer;
	m_bufferLen = newBufferLen;
	m_head = 0;
	m_tail = len;

	return TRUE;
}

UINT SocketInputStream::Length() const
{
	if (m_head < m_tail)
		return (m_tail - m_head);
	else if (m_head > m_tail)
		return (m_bufferLen - m_head + m_tail);

	return 0;
}

VOID SocketInputStream::Cleanup()
{
	m_head = 0;
	m_tail = 0;
}
