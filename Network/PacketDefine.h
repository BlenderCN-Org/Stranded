#ifndef __PACKET_DEFINE_H__
#define __PACKET_DEFINE_H__

namespace Packets
{
	// 消息类型值描述格式：PACKET_XX_YYYYYY
	// XX可以描述为：GC、 CG、CL、LC、GL、LG
	// G-游戏服务器端、C-客户端、L-登录服务器端、S-服务器端程序(可以是登录，也可以是游戏)
	// YYYYYY表示消息内容
	// 例如：PACKET_CG_ATTACK 表示客户端发给服务器端关于攻击的消息
	enum PACKET_DEFINE
	{
		PACKET_NONE = 0,								// 0, 空
		/************************************************************************/
		PACKET_CG_HEARTBEAT,							// 心跳消息
		/************************************************************************/
		PACKET_GC_ITEMINFO,								// 服务器返回某件装备的详细信息
		/************************************************************************/
		PACKET_MAX										// 消息类型的最大值
	};
}

#endif
