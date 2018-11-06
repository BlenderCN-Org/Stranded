#include "GCItemInfo.h"

BOOL GCItemInfo::Read(SocketInputStream& iStream)
{
	iStream.Read((CHAR*)(&m_BagIndex), sizeof(WORD));
	iStream.Read((CHAR*)(&m_nsNull), sizeof(BOOL));

	m_ITEM.Read(iStream);

	return TRUE;
}

BOOL GCItemInfo::Write(SocketOutputStream& oStream) const
{
	oStream.Write((CHAR*)(&m_BagIndex), sizeof(WORD));
	oStream.Write((CHAR*)(&m_nsNull), sizeof(BOOL));

	m_ITEM.Write(oStream);

	return TRUE;
}

UINT	GCItemInfo::Execute(Player* pPlayer)
{
	return GCItemInfoHandler::Execute(this, pPlayer);
}