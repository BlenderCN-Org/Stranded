#ifndef __PACKET_H__
#define __PACKET_H__

#include "PacketDefine.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

class Socket;
class Player;

#define GET_PACKET_INDEX(a) ((a)>>24)
#define SET_PACKET_INDEX(a, index) ((a)=(((a)&0xFFFFFF)+((index)<<24)))
#define GET_PACKET_LEN(a) ((a)&0xFFFFFF)
#define SET_PACKET_LEN(a, len) ((a)=((a)&0xFF000000)+(len))
/*
 * 消息头中包括：PacketID_t-2字节; UINT-4字节中高位一个字节为消息序列号，其余
 * 三个字节为消息长度
 * 通过GET_PACKET_INDEX 和GET_PACKET_LEN 宏，可以取得UINT数据里面的消息序列号和长度
 * 通过SET_PACKET_INDEX 和SET_PACKET_LEN 宏，可以设置UINT数据里面的消息序列号和长度
 */
#define PACKET_HEADER_SIZE (sizeof(PacketID_t)+sizeof(UINT)+sizeof(UINT))

// Packet::Execute(...) 的返回值
enum PACKET_EXE
{
	PACKET_EXE_ERROR = 0,
	PACKET_EXE_BREAK,
	PACKET_EXE_CONTINUE,
	PACKET_EXE_NOTREMOVE,
	PACKET_EXE_NOTREMOVE_ERROR,
};

class Packet
{
protected:
	BYTE		m_index;
	BYTE		m_status;
public:
	Packet();
	virtual ~Packet();

	virtual VOID CleanUp() {}

	virtual BOOL Read(SocketInputStream& iStream) = 0;

	virtual BOOL Write(SocketOutputStream& oStream) const = 0;

	/*
	 * 返回值为: PACKET_EXE 中的内容.
	 * PACKET_EXE_ERROR 表示出现严重错误，当前连接需要被强制断开
	 * PACKET_EXE_BREAK 表示返回后剩下的消息将不在当前处理循环里处理
	 * PACKET_EXE_CONTINUE 表示继续在当前循环里执行剩下的消息
	 * PACKET_EXE_NOTREMOVE 表示继续在当前循环里执行剩下的消息，但是不回收当前消息
	 */
	virtual UINT Execute(Player* pPlayer) = 0;

	virtual PacketID_t GetPacketID() const = 0;

	virtual UINT GetPacketSize() const = 0;

	virtual BOOL CheckPacket() { return TRUE; }

	BYTE GetPacketIndex() const { return m_index; }
	VOID SetPacketIndex(BYTE index) { m_index = index; }

	BYTE GetPacketStatus() const { return m_status; }
	VOID SetPacketStatus(BYTE status) { m_status = status; }
};

#endif // !__PACKET_H__

