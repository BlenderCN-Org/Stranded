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
	// �ⲿ����ͨ�ýӿ�

	// ��ʼ���ӿ�
	BOOL Init();
	// ������Ϣ���ʹ��ڴ��������Ϣʵ������(������߳�ͬʱ����)
	Packet* CreatePacket(PacketID_t packetID);
	// ������Ϣ����ȡ�ö�Ӧ��Ϣ�����ߴ�(������߳�ͬʱ����)
	UINT GetPacketMaxSize(PacketID_t packetID);
	// ɾ����Ϣʵ��(������߳�ͬʱ����)
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
