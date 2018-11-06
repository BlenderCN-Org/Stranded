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
	unsigned short msgLen;			// ��Ϣ����(��������Ϣͷ)��ȡ����ʵ��
	unsigned char msgFlags;			// ��Ϣ��־λ
	unsigned char msgCls;			// ��Ϣ���
	unsigned short msgId;			// ��Ϣid
	unsigned char crcCode;			// CRCУ����
	unsigned long long context;		// ����������
};

struct AppMsgServerList
{
	unsigned short msgLen;			// ��Ϣ����(��������Ϣͷ)��ȡ����ʵ��
	unsigned char msgFlags;			// ��Ϣ��־λ
	unsigned char msgCls;			// ��Ϣ���
	unsigned short msgId;			// ��Ϣid
	unsigned char crcCode;			// CRCУ����
	unsigned long long context;		// ����������
};

struct AppMsgSmallSC
{
	unsigned short msgLen;			// ��Ϣ����(��������Ϣͷ)��ȡ����ʵ��
	unsigned char msgFlags;			// ��Ϣ��־λ
	unsigned short msgId;			// ��Ϣid
};

struct AppMsgSmallCS
{
	unsigned short msgLen;			// ��Ϣ����(��������Ϣͷ)��ȡ����ʵ��
	unsigned char msgFlags;			// ��Ϣ��־λ
	unsigned char msgCls;			// ��Ϣ���
	unsigned short msgId;			// ��Ϣid
	unsigned char crcCode;			// CRCУ����
};

const int SIZEOF_APPMSG = sizeof(AppMsg);
const int SIZEOF_APPMSG_SERVERLIST = sizeof(AppMsgServerList);
const int SIZEOF_APPMSG_SMALL_SC = sizeof(AppMsgSmallSC);
const int SIZEOF_APPMSG_SMALL_CS = sizeof(AppMsgSmallCS);

// ���鳣������ AppMsg::msgCls.
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
