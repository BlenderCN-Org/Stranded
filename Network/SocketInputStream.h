#ifndef __SOCKET_INPUT_STREAM_H__
#define __SOCKET_INPUT_STREAM_H__

/*
 * @brief: 消息数据的接收缓存，提供数据的接收和格式化读取功能
 */

#include "SSocket.h"


// 初始化的接收缓存长度
#define DEFAULTSOCKETINPUTBUFFERSIZE 64*1024
// 最大可以允许的缓存长度，如果超过此数值，则断开连接
#define DISCONNECTSOCKETINPUTSIZE 96* 1024

class Packet;
class SocketInputStream
{
public:
	SocketInputStream(Socket* sock, UINT bufferSize = DEFAULTSOCKETINPUTBUFFERSIZE, UINT maxBufferSize = DISCONNECTSOCKETINPUTSIZE);
	virtual ~SocketInputStream();

public:
	UINT Read(CHAR* buf, UINT len);
	BOOL ReadPacket(Packet* p);

	BOOL Peek(CHAR* buf, UINT len);

	BOOL Find(CHAR* buf);

	BOOL Skip(UINT len);

	UINT Fill();

	VOID Initsize();
	BOOL Resize(INT size);

	UINT Capacity() const { return m_bufferLen; }

	UINT Length() const;
	UINT Size() const { return Length(); }

	BOOL IsEmpty() const { return m_head == m_tail; }

	Socket* GetSocket() const { return m_pSocket; }

	VOID Cleanup();

	UINT GetHead() { return m_head; }
	UINT GetTail() { return m_tail; }
	UINT GetBufferLen() { return m_bufferLen; }
	CHAR* GetBuffer() { return m_buffer; }

protected:
	Socket*		m_pSocket;

	CHAR*		m_buffer;

	UINT		m_bufferLen;
	UINT		m_maxBufferLen;

	UINT		m_head;
	UINT		m_tail;
};

#endif
