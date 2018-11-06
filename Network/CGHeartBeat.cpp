#include "CGHeartBeat.h"

BOOL Packets::CGHeartBeat::Read(SocketInputStream & iStream)
{
	return TRUE;
}

BOOL Packets::CGHeartBeat::Write(SocketOutputStream& oStream) const
{
	return TRUE;
}

UINT Packets::CGHeartBeat::Execute(Player* pPlayer)
{
	return CGHeartBeatHandler::Execute(this, pPlayer);
}
