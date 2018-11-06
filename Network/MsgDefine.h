#pragma once

#pragma pack(push, 1)

typedef unsigned char			uchar;		// 8 bit
typedef unsigned short			ushort;		// 16 bit
typedef unsigned int			uint;		// 32 bit
typedef long long				int64;		// 64 bit
typedef int						int32;		// 32 bit
typedef short					int16;		// 16 bit
typedef char					int8;		// 8 bit
typedef unsigned long long		uint64;		// 6 bit
typedef unsigned int			uint32;		// 32 bit
typedef unsigned short			uint16;		// 16 bit
typedef unsigned char			uint8;		// 8 bit

struct AppMsg
{
	unsigned short msgLen;			// 消息长度(包括本消息头)，取决于实现
	unsigned char msgFlags;			// 消息标志位
	unsigned char msgCls;			// 消息类别
	unsigned short msgId;			// 消息id
	unsigned char crcCode;			// CRC校验码
	unsigned long long context;		// 保留上下文
};

struct AppMsgServerList
{
	unsigned short msgLen;			// 消息长度(包括本消息头)，取决于实现
	unsigned char msgFlags;			// 消息标志位
	unsigned char msgCls;			// 消息类别
	unsigned short msgId;			// 消息id
	unsigned char crcCode;			// CRC校验码
	unsigned long long context;		// 保留上下文
};

struct AppMsgSmallSC
{
	unsigned short msgLen;			// 消息长度(包括本消息头)，取决于实现
	unsigned char msgFlags;			// 消息标志位
	unsigned short msgId;			// 消息id
};

struct AppMsgSmallCS
{
	unsigned short msgLen;			// 消息长度(包括本消息头)，取决于实现
	unsigned char msgFlags;			// 消息标志位
	unsigned char msgCls;			// 消息类别
	unsigned short msgId;			// 消息id
	unsigned char crcCode;			// CRC校验码
};

const int SIZEOF_APPMSG = sizeof(AppMsg);
const int SIZEOF_APPMSG_SERVERLIST = sizeof(AppMsgServerList);
const int SIZEOF_APPMSG_SMALL_SC = sizeof(AppMsgSmallSC);
const int SIZEOF_APPMSG_SMALL_CS = sizeof(AppMsgSmallCS);

// 本组常量用于 AppMsg::msgCls.
enum _AppMsgClass
{
	MSG_VS_DEFAULT = 0,

	MSG_CLASS_MAX = 255
};

enum _AppMsgFlagTo
{
	MSG_FLAG_ENCRYPT = 0x01,
	MSG_FLAG_PART = 0x10,
	MSG_FLAG_COMPRESS = 0x20,
	MSG_FLAG_LITTLE = 0x40,
	MSG_FLAG_SERVER = 0x80,
};

#pragma pack(pop)
