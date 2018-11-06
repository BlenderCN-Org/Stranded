#include "SocketOutputStream.h"
#include "Packet.h"

SocketOutputStream::SocketOutputStream(Socket* sock, UINT bufferSize /*= DEFAULTSOCKETOUTPUTBUFFERSIZE*/, UINT maxBufferSize /*= DISCONNECTSOCKETOUTPUTSIZE*/)
{
	m_pSocket = sock;
	m_bufferLen = bufferSize;
	m_maxBufferLen = maxBufferSize;

	m_head = 0;
	m_tail = 0;

	m_buffer = new CHAR[m_bufferLen];
	memset(m_buffer, 0, m_bufferLen);
}

SocketOutputStream::~SocketOutputStream()
{
	SAFE_DELETE_ARRAY(m_buffer);
}

UINT SocketOutputStream::Write(const CHAR* buf, UINT len)
{
	//					//
	//     T  H			//    H   T			LEN=10
	// 0123456789		// 0123456789
	// abcd...efg		// ...abcd...
	//					//

	UINT nFree = ((m_head<=m_tail)?(m_bufferLen-m_tail+m_head-1):(m_head-m_tail-1));

	if (len >= nFree)
	{
		if (!Resize(len - nFree + 1))
			return 0;
	}

	if (m_head <= m_tail)
	{
		if (m_head == 0)
		{
			nFree = m_bufferLen - m_tail - 1;
			memcpy(&m_buffer[m_tail], buf, len);
		}
		else
		{
			nFree = m_bufferLen - m_tail;
			if (len <= nFree)
			{
				memcpy(&m_buffer[m_tail], buf, len);
			}
			else
			{
				memcpy(&m_buffer[m_tail], buf, nFree);
				memcpy(m_buffer, &buf[nFree], len - nFree);
			}
		}
	}
	else
	{
		memcpy(&m_buffer[m_tail], buf, len);
	}

	m_tail = (m_tail+len)%m_bufferLen;

	return len;
}

BOOL SocketOutputStream::WritePacket(const Packet* pPacket)
{
	assert(pPacket != nullptr);

	PacketID_t packetID = pPacket->GetPacketID();
	UINT w = Write((CHAR*)&packetID, sizeof(PacketID_t));

	UINT packetUINT;

	UINT packetSize = pPacket->GetPacketSize();
	UINT packetIndex = pPacket->GetPacketIndex();

	SET_PACKET_INDEX(packetUINT, packetIndex);
	SET_PACKET_LEN(packetUINT, packetSize);

	w = Write((CHAR*)&packetUINT, sizeof(UINT));

	return pPacket->Write(*this);
}

UINT SocketOutputStream::Flush()
{
	assert(m_pSocket != nullptr);

	UINT nFlushed = 0;
	UINT nSent = 0;
	UINT nLeft;

	if (m_bufferLen > m_maxBufferLen)
	{
		// 如果单个客户端的缓存太大，则重新设置缓存，并将此客户端断开连接
		Initsize();

		return SOCKET_ERROR - 1;
	}

#if defined(WIN32)
	UINT flag = MSG_DONTROUTE;
#elif defined(__LINUX__)
	UINT flag = MSG_NOSIGNAL;
#endif

	try
	{
		if (m_head < m_tail)
		{
			nLeft = m_tail - m_head;
			while (nLeft>0)
			{
				nSent = m_pSocket->send(&m_buffer[m_head], nLeft, flag);
				if (nSent == SOCKET_ERROR_WOULDBLOCK) return 0;
				if (nSent == SOCKET_ERROR) return SOCKET_ERROR - 2;
				if (nSent == 0) return 0;

				nFlushed += nSent;
				nLeft -= nSent;
				m_head += nSent;
			}
		}
		else if (m_head > m_tail)
		{
			nLeft = m_bufferLen - m_head;

			while (nLeft > 0)
			{
				nSent = m_pSocket->send(&m_buffer[m_head], nLeft, flag);
				if (nSent == SOCKET_ERROR_WOULDBLOCK) return 0;
				if (nSent == SOCKET_ERROR) return SOCKET_ERROR - 3;
				if (nSent == 0) return 0;

				nFlushed += nSent;
				nLeft -= nSent;
				m_head += nSent;
			}

			m_head = 0;

			nLeft = m_tail;

			while (nLeft > 0)
			{
				nSent = m_pSocket->send(&m_buffer[m_head], nLeft, flag);
				if (nSent == SOCKET_ERROR_WOULDBLOCK) return 0;
				if (nSent == SOCKET_ERROR) return SOCKET_ERROR - 4;
				if (nSent == 0) return 0;

				nFlushed += nSent;
				nLeft -= nSent;
				m_head += nSent;
			}
		}

		if (m_head != m_tail)
		{
			assert(m_head == m_tail);
		}
	}
	catch (...)
	{
		if (nSent > 0)
			m_head += nSent;
	}

	m_head = m_tail = 0;

	return nFlushed;
}

VOID SocketOutputStream::Initsize()
{
	m_head = 0;
	m_tail = 0;

	SAFE_DELETE_ARRAY(m_buffer);

	m_buffer = new CHAR[DEFAULTSOCKETOUTPUTBUFFERSIZE];

	m_bufferLen = DEFAULTSOCKETOUTPUTBUFFERSIZE;

	memset(m_buffer, 0, m_bufferLen);
}

BOOL SocketOutputStream::Resize(INT size)
{
	INT orgSize = size;

	size = max(size, (int)(m_bufferLen>>1));
	UINT newBufferLen = m_bufferLen + size;
	UINT len = Length();

	if (size < 0)
	{
		if (newBufferLen < 0 || newBufferLen < len)
			return FALSE;
	}

	CHAR* newBuffer = new CHAR[newBufferLen];
	if (newBuffer == NULL)
		return FALSE;

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

UINT SocketOutputStream::Length() const
{
	if (m_head < m_tail)
		return m_tail - m_head;
	else if (m_head>m_tail)
		return m_bufferLen - m_head + m_tail;

	return 0;
}

VOID SocketOutputStream::CleanUp()
{
	m_head = 0;
	m_tail = 0;
}
