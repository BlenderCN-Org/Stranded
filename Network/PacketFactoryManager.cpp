#include "PacketFactoryManager.h"
#include "PacketDefine.h"

#include "PacketInclude.h"

using namespace Packets;

PacketFactoryManager* g_pPacketFactoryManager = nullptr;

PacketFactoryManager::PacketFactoryManager()
{
	m_factories = nullptr;
	m_size = PACKET_MAX;

	assert(m_size > 0);

	m_factories = new PacketFactory*[m_size];
	assert(m_factories);
	m_pPacketAllocCount = new UINT[m_size];
	assert(m_pPacketAllocCount);

	for (INT i = 0; i < m_size; ++i)
	{
		m_factories[i] = nullptr;
		m_pPacketAllocCount[i] = 0;
	}
}

PacketFactoryManager::~PacketFactoryManager()
{
	assert(m_factories != nullptr);

	for (INT i = 0; i < m_size; ++i)
	{
		SAFE_DELETE(m_factories[i]);
	}

	SAFE_DELETE_ARRAY(m_factories);
	SAFE_DELETE_ARRAY(m_pPacketAllocCount);
}

VOID PacketFactoryManager::AddFactory(PacketFactory* pFactory)
{
	assert(pFactory);

	if (m_factories[pFactory->GetPacketID()] != nullptr)
	{
		// 重复设定
		assert(FALSE);
		return;
	}

	m_factories[pFactory->GetPacketID()] = pFactory;
}

BOOL PacketFactoryManager::Init()
{
	// 此处添加每种消息的Factory信息
	// 例如：
	AddFactory(new CGHeartBeatFactory());

	return TRUE;
}

Packet* PacketFactoryManager::CreatePacket(PacketID_t packetID)
{
	if (packetID >= m_size || m_factories[packetID] == nullptr)
	{
		assert(FALSE);
		return nullptr;
	}

	Packet* pPacket = nullptr;
	Lock();
	try
	{
		pPacket = m_factories[packetID]->CreatePacket();
		m_pPacketAllocCount[packetID]++;
	}
	catch (...)
	{
		pPacket = nullptr;
	}
	Unlock();

	return pPacket;
}

UINT PacketFactoryManager::GetPacketMaxSize(PacketID_t packetID)
{
	if (packetID >= m_size || m_factories[packetID] == nullptr)
	{
		char buf[256] = { 0 };
		sprintf(buf, "PacketID=[%d] 消息没有注册到PacketFactoryManager上", packetID);
		OutputDebugString(buf);
		assert(FALSE);
		return 0;
	}

	Lock();
	UINT iRet = m_factories[packetID]->GetPacketMaxSize();
	Unlock();

	return iRet;
}

VOID PacketFactoryManager::RemovePacket(Packet* pPacket)
{
	if (pPacket == nullptr)
	{
		assert(FALSE);
		return;
	}

	PacketID_t packetID = pPacket->GetPacketID();
	if (packetID >= m_size)
	{
		assert(FALSE);
		return;
	}

	Lock();
	try
	{
		SAFE_DELETE(pPacket);
		m_pPacketAllocCount[packetID]--;
	}
	catch(...)
	{ }
	Unlock();
}
