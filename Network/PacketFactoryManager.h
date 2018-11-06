#ifndef __PACKET_FACTORY_MANAGER_H__
#define __PACKET_FACTORY_MANAGER_H__

#include "../Common/pubutil.h"
#include "Packet.h"
#include "PacketFactory.h"

class PacketFactoryManager
{
public:
	PacketFactoryManager();
	~PacketFactoryManager();
private:
	VOID AddFactory(PacketFactory* pFactory);
public:
	// 外部调用通用接口

	// 初始化接口
	BOOL Init();
	// 根据消息类型从内存里分配消息实体数据(允许多线程同时调用)
	Packet* CreatePacket(PacketID_t packetID);
	// 根据消息类型取得对应消息的最大尺寸(允许多线程同时调用)
	UINT GetPacketMaxSize(PacketID_t packetID);
	// 删除消息实体(允许多线程同时调用)
	VOID RemovePacket(Packet* pPacket);

	VOID Lock() { m_lock.Lock(); }
	VOID Unlock() { m_lock.Unlock(); }
private:
	PacketFactory**		m_factories;

	USHORT				m_size;

	SLock				m_lock;
public:
	UINT*				m_pPacketAllocCount;
};

extern PacketFactoryManager* g_pPacketFactoryManager;

#endif
