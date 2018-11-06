#ifndef __PACKET_DEFINE_H__
#define __PACKET_DEFINE_H__

namespace Packets
{
	// ��Ϣ����ֵ������ʽ��PACKET_XX_YYYYYY
	// XX��������Ϊ��GC�� CG��CL��LC��GL��LG
	// G-��Ϸ�������ˡ�C-�ͻ��ˡ�L-��¼�������ˡ�S-�������˳���(�����ǵ�¼��Ҳ��������Ϸ)
	// YYYYYY��ʾ��Ϣ����
	// ���磺PACKET_CG_ATTACK ��ʾ�ͻ��˷����������˹��ڹ�������Ϣ
	enum PACKET_DEFINE
	{
		PACKET_NONE = 0,								// 0, ��
		/************************************************************************/
		PACKET_CG_HEARTBEAT,							// ������Ϣ
		/************************************************************************/
		PACKET_GC_ITEMINFO,								// ����������ĳ��װ������ϸ��Ϣ
		/************************************************************************/
		PACKET_MAX										// ��Ϣ���͵����ֵ
	};
}

#endif