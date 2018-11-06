#ifndef __SOCKET_OUTPUT_STREAM_H__
#define __SOCKET_OUTPUT_STREAM_H__

/*
 * @brief: ��Ϣ���ݵ�������棬�ṩ���ݵĸ�ʽ��д��ͷ��͹���
 */

#include "SSocket.h"


// ��ʼ���ķ��ͻ��泤��
#define DEFAULTSOCKETOUTPUTBUFFERSIZE 8192
// ����������Ļ��泤�ȣ������������ֵ����Ͽ�����
#define DISCONNECTSOCKETOUTPUTSIZE 100*1024

class Packet;
class SocketOutputStream
{
public:
	SocketOutputStream(Socket* sock, UINT bufferSize = DEFAULTSOCKETOUTPUTBUFFERSIZE, UINT maxBufferSize = DISCONNECTSOCKETOUTPUTSIZE);
	virtual ~SocketOutputStream();

public:
	UINT Write(const CHAR* buf, UINT len);
	BOOL WritePacket(const Packet* pPacket);

	UINT Flush();

	VOID Initsize();
	BOOL Resize(INT size);

	INT Capacity() const { return m_bufferLen; }

	UINT Length() const;
	UINT Size() const { return Length(); }

	CHAR* GetBuffer() const { return m_buffer; }

	CHAR* GetTail() const { return &(m_buffer[m_tail]); }

	BOOL IsEmpty() const { return m_head == m_tail; }

	Socket* GetSocket() const { return m_pSocket; }

	VOID CleanUp();

	UINT GetHead() { return m_head; }
	UINT GetTail() { return m_tail; }
	UINT GetBuffLen() { return m_bufferLen; }
	CHAR* GetBuff() { return m_buffer; }
protected:
	Socket*			m_pSocket;

	CHAR*			m_buffer;

	UINT			m_bufferLen;
	UINT			m_maxBufferLen;

	UINT			m_head;
	UINT			m_tail;
};

#endif
