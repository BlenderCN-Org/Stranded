#ifndef __GAME_STRUCT_ITEM_H__
#define __GAME_STRUCT_ITEM_H__

#include "../Common/define.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

#define MAX_ITEM_CREATOR_NAME 13
#define MAX_ITEM_PARAM 3
// ��Ʒ����Ƕ��ʯ��������
#define MAX_ITEM_GEM 3
// �����Ʒ����
#define MAX_ITEM_ATTR (9+MAX_ITEM_GEM)
// ��Ч��IDֵ
#define INVALID_ID		-1

static BYTE GetSerialClass(UINT Serial)
{
	return (BYTE)(Serial / 10000000);
}

static BYTE GetSerialType(UINT Serial)
{
	return (BYTE)((Serial % 100000) / 1000);
}

static BYTE GetSerialQual(UINT Serial)
{
	return	(BYTE)((Serial % 10000000) / 100000);
}

UINT GetSerialIndex(UINT Serial)
{
	return (Serial % 1000);
}

enum ItemParamValue
{
	IPV_CHAR = 0,
	IPV_SHORT = 1,
	IPV_INT = 2
};

//��Ʒ��Ϣλ����
enum ITEM_EXT_INFO
{
	IEI_BIND_INFO = 0x00000001,	//����Ϣ
	IEI_IDEN_INFO = 0x00000002, //������Ϣ
	IEI_PLOCK_INFO = 0x00000004, //���������Ѿ�����
	IEI_BLUE_ATTR = 0x00000008, //�Ƿ���������
	IEL_CREATOR = 0x00000010,	//�Ƿ��д�����
};

enum ITEM_CLASS
{
	ICLASS_EQUIP = 1,	// ����WEAPON������DEFENCE������ADORN
	ICLASS_MATERIAL = 2,	// ԭ��
	ICLASS_COMITEM = 3,	// ҩƷ
	ICLASS_TASKITEM = 4,	// ������Ʒ
	ICLASS_GEM = 5, // ��ʯ
	ICLASS_STOREMAP = 6,	// �ر�ͼ
	ICLASS_TALISMAN = 7,	// ����---???
	ICLASS_GUILDITEM = 8,	// �����Ʒ
	ICLASS_NUMBER, // ��Ʒ���������
};

enum ITEM_ATTRIBUTE
{
	IATTRIBUTE_POINT_MAXHP = 0,		//����������HP������
	IATTRIBUTE_RATE_MAXHP = 1,		//���ٷֱ�����HP������
	IATTRIBUTE_RESTORE_HP = 2,		//�ӿ�HP�Ļظ��ٶ�

	IATTRIBUTE_POINT_MAXMP = 3,		//����������MP������
	IATTRIBUTE_RATE_MAXMP = 4,		//���ٷֱ�����MP������
	IATTRIBUTE_RESTORE_MP = 5,		//�ӿ�MP�Ļظ��ٶ�

	IATTRIBUTE_COLD_ATTACK = 6,		//������
	IATTRIBUTE_COLD_RESIST = 7,		//���ֿ�
	IATTRIBUTE_COLD_TIME = 8,		//���ٱ����ٻ�ʱ��
	IATTRIBUTE_FIRE_ATTACK = 9,		//�𹥻�
	IATTRIBUTE_FIRE_RESIST = 10,	//��ֿ�
	IATTRIBUTE_FIRE_TIME = 11,	//���ٻ��ճ���ʱ��
	IATTRIBUTE_LIGHT_ATTACK = 12,	//�繥��
	IATTRIBUTE_LIGHT_RESIST = 13,	//��ֿ�
	IATTRIBUTE_LIGHT_TIME = 14,	//���ٵ��ѣ��ʱ��
	IATTRIBUTE_POISON_ATTACK = 15,	//������
	IATTRIBUTE_POISON_RESIST = 16,	//���ֿ�
	IATTRIBUTE_POISON_TIME = 17,	//�����ж�ʱ��
	IATTRIBUTE_RESIST_ALL = 18,	//���ٷֱȵ����������Թ���

	IATTRIBUTE_ATTACK_P = 19,	//������
	IATTRIBUTE_RATE_ATTACK_P = 20,	//���ٷֱ�����������
	IATTRIBUTE_RATE_ATTACK_EP = 21,	//��װ�������������ٷֱȼӳ�
	IATTRIBUTE_DEFENCE_P = 22,	//�������
	IATTRIBUTE_RATE_DEFENCE_P = 23,	//���ٷֱ������������
	IATTRIBUTE_RATE_DEFENCE_EP = 24,	//��װ��������������ٷֱȼӳ�
	IATTRIBUTE_IMMUNITY_P = 25,	//���ٷֱȵ��������˺�

	IATTRIBUTE_ATTACK_M = 26,	//ħ������
	IATTRIBUTE_RATE_ATTACK_M = 27,	//���ٷֱ�����ħ������
	IATTRIBUTE_RATE_ATTACK_EM = 28,	//��װ������ħ�������ٷֱȼӳ�
	IATTRIBUTE_DEFENCE_M = 29,	//ħ������
	IATTRIBUTE_RATE_DEFENCE_M = 30,	//���ٷֱ�����ħ������
	IATTRIBUTE_RATE_DEFENCE_EM = 31,	//��װ������ħ�������ٷֱȼӳ�
	IATTRIBUTE_IMMUNITY_M = 32,	//���ٷֱȵ���ħ���˺�

	IATTRIBUTE_ATTACK_SPEED = 33,	//�����ٶ�
	IATTRIBUTE_SKILL_TIME = 34,	//ħ����ȴ�ٶ�

	IATTRIBUTE_HIT = 35,	//����
	IATTRIBUTE_MISS = 36,	//����
	IATTRIBUTE_2ATTACK_RATE = 37,	//����һ����˫���������İٷֱ�
	IATTRIBUTE_NO_DEFENCE_RATE = 38,	//���ӶԷ������ĸ���

	IATTRIBUTE_SPEED_RATE = 39,	//�ƶ��ٶȰٷֱ�

	IATTRIBUTE_DAMAGE_RET = 40,	//�˺�����
	IATTRIBUTE_DAMAGE2MANA = 41,	//�˺�����������

	IATTRIBUTE_STR = 42,	//��������
	IATTRIBUTE_SPR = 43,	//��������
	IATTRIBUTE_CON = 44,	//��������
	IATTRIBUTE_INT = 45,	//���Ӷ���
	IATTRIBUTE_DEX = 46,	//������
	IATTRIBUTE_LUK = 47,	//��������
	IATTRIBUTE_ALL = 48,	//�������е�����һ������

	IATTRIBUTE_HP_THIEVE = 49,	//����͵ȡ(���˺���)
	IATTRIBUTE_MP_THIEVE = 50,	//����͵ȡ(���˺���)

	IATTRIBUTE_USESKILL = 51,	//����ĳ��ʹ�ü���
	IATTRIBUTE_RAND_SKILL = 52,	//����ĳ���������
	IATTRIBUTE_SKILL_RATE = 53,	//������ܷ�������

	IATTRIBUTE_BASE_ATTACK_P = 54,	//����������
	IATTRIBUTE_BASE_ATTACK_M = 55,	//����ħ������
	IATTRIBUTE_BASE_ATTACK_TIME = 56,	//���������ٶȣ�ֻ����ͨ������
	IATTRIBUTE_BASE_DEFENCE_P = 57,	//�����������
	IATTRIBUTE_BASE_DEFENCE_M = 58,	//����ħ������
	IATTRIBUTE_BASE_MISS = 59,	//��������



	IATTRIBUTE_NUMBER, //��Ʒ������������
};


// ��ƷΨһID
struct _ITEM_GUID
{
	BYTE		m_world;			// �����: (��)101
	BYTE		m_server;			// �����������: (��)5
	INT			m_serial;			// ��Ʒ���к�: (��)123429

	_ITEM_GUID()
	{
		m_world = 0;
		m_server = 0;
		m_serial = 0;
	}

	VOID Reset()
	{
		m_world = 0;
		m_server = 0;
		m_serial = 0;
	}

	_ITEM_GUID& operator=(const _ITEM_GUID& rhs)
	{
		this->m_serial = rhs.m_serial;
		this->m_server = rhs.m_server;
		this->m_world = rhs.m_world;
		return *this;
	}

	BOOL operator==(_ITEM_GUID& ref) const
	{
		return (ref.m_serial==m_serial)&&(ref.m_server==m_server)&&(ref.m_world==m_world);
	}

	BOOL operator==(const _ITEM_GUID& ref) const
	{
		return (ref.m_serial == m_serial) && (ref.m_server == m_server) && (ref.m_world == m_world);
	}

	BOOL IsNull() const
	{
		return (m_world==0)&&(m_serial==0)&&(m_server==0);
	}
};

struct MEDIC_INFO
{

	BYTE					m_nCount;			// ��ǰ����
	BYTE					m_nLevel;
	BYTE					m_nLayedNum;		// ��������
	BYTE					m_nReqSkillLevel;

	UINT					m_nBasePrice;
	INT						m_nScriptID;
	INT						m_nSkillID;
	BOOL					m_bCosSelf;			// �Ƿ������Լ�
	INT						m_nReqSkill;
	BYTE					m_TargetType;		// ��Ʒѡ������

	INT GetTileNum() { return m_nCount; }

	INT GetMaxTileNum() { return m_nLayedNum; }

	VOID CleanUp()
	{
		m_nLevel = 0;
		m_nBasePrice = 0;
		m_nLayedNum = 0;
		m_nScriptID = INVALID_ID;
		m_nSkillID = 0;
		m_nCount = 0;
		m_bCosSelf = FALSE;
		m_nReqSkill = -1;
		m_nReqSkillLevel = -1;
		m_TargetType = 0;
	}
};


// ��Ƕ��ʯ��Ϣ
struct _ITEM_GEMINFO
{
	UINT m_GemType;
};

struct GEM_INFO
{

	_ITEM_ATTR	m_Attr;
	UINT		m_nPrice;


	VOID CleanUp()
	{
		m_nPrice = 0;

		m_Attr.CleanUp();
	}
};

struct STORE_MAP_INFO
{
	INT						m_nLevel;
	FLOAT					m_xPos;
	FLOAT					m_zPos;
	INT						m_SceneID;
	INT						m_GrowType;
};

// ��Ʒ����ֵ
struct _ITEM_VALUE
{
	SHORT m_Value;

	VOID CleanUp()
	{
		memset(this, 0, sizeof(*this));
	}

	BOOL operator == (_ITEM_VALUE& iV)
	{
		return iV.m_Value = m_Value;
	}
};

// ��Ʒ����
struct _ITEM_ATTR
{
	BYTE				m_AttrType;		// �������ͨװ������m_AttrType��enum ITEM_ATTRIBUTE
										//    ����ʾ�������ͣ�m_Value��ʾ��������ֵ
										// �������ɫ����װ����m_AttrType��ʾ��ǰ��װ��
										//    ������ϵ�װ��������m_Value��ʾ��װ��������
	_ITEM_VALUE			m_Value;

	VOID CleanUp()
	{
		memset(this, 0, sizeof(*this));
	}

	BOOL operator==(_ITEM_ATTR& iA)
	{
		return	(iA.m_AttrType == m_AttrType) && (iA.m_Value == m_Value);
	}
};

struct EQUIP_INFO
{
	SHORT				m_SetNum;					// ��װ���
	UINT				m_BasePrice;				// �۳��۸�
	CHAR				m_MaxNum;					// ��װ
	BYTE				m_EquipPoint;				// ��Ʒװ���
	BYTE				m_MaxDurPoint;				// ����;�ֵ
	BYTE				m_NeedLevel;				// ����ȼ�
	BYTE				m_GemMax;					// ���ʯ����
	//////////////////////////////////////////////////////////////////////////
	// ����Ϊ�̶�����
	BYTE				m_FaileTimes;				// ����ʧ�ܴ���
	BYTE				m_CurDurPoint;				// ��ǰ�;�ֵ
	USHORT				m_CurDamagePoint;			// ��ǰ���˶�
	BYTE				m_AttrCount;				// ���Ե�����
	BYTE				m_StoneCount;				// ��Ƕ��ʯ������
	_ITEM_ATTR			m_pAttr[MAX_ITEM_ATTR];		// ��Ʒ����
	_ITEM_GEMINFO		m_pGemInfo[MAX_ITEM_GEM];	// ��Ʒ����Ƕ�ı�ʯ

	VOID CleanUp()
	{
		memset(this, 0, sizeof(EQUIP_INFO));
	};
};

struct _ITEM
{
	_ITEM_GUID				m_ItemGuid;						// ��Ʒ�̶���Ϣ(���ܸı�)
	UINT					m_ItemIndex;					// ��Ʒ����
															/*
															|	    1 ~ 10000		��ɫװ��
															|	10001 ~ 20000		��ɫװ��
															|	20001 ~ 30000		��ɫװ��
															|	30001 ~ 35000		ҩƿ
															|	35001 ~ 40000		��ʯ
															|						...
															*/
	CHAR					m_rulerId;
	CHAR					m_nsBind;
	CHAR					m_creator[MAX_ITEM_CREATOR_NAME];
	INT						m_param[MAX_ITEM_PARAM];

	union
	{
		EQUIP_INFO			m_Equip;
		GEM_INFO			m_Gem;
		MEDIC_INFO			m_Medic;
		STORE_MAP_INFO		m_StoreMap;
	};

	_ITEM()
	{
		CleanUp();
	}

	VOID CleanUp()
	{
		memset(this, 0, sizeof(*this));
	}

	BYTE ItemClass() const
	{
		return GetSerialClass(m_ItemIndex);
	};

	BYTE ItemType() const
	{
		return GetSerialType(m_ItemIndex);
	}

	BYTE GetQual() const
	{
		return GetSerialQual(m_ItemIndex);
	}

	BYTE GetIndex() const
	{
		return GetSerialIndex(m_ItemIndex);
	}

	EQUIP_INFO* GetEquipData() const
	{
		return	(EQUIP_INFO*)(&m_Equip);
	}

	GEM_INFO* GetGemData()	const
	{
		return	(GEM_INFO*)(&m_Gem);
	}

	MEDIC_INFO* GetMedicData()	const
	{
		return (MEDIC_INFO*)(&m_Medic);
	}

	STORE_MAP_INFO*	GetStoreMapData() const
	{
		return (STORE_MAP_INFO*)(&m_StoreMap);
	}
	//////////////////////////////////////////////////////////////////////////
	// ��Ʒ��������
	// �ܵ�����Ʒ		���� ʵ�ʴ�С
	// ���ܵ�����Ʒ		���� 1
	//
	BYTE GetItemCount() const
	{
		BYTE bClass = ItemClass();

		if (bClass == ICLASS_EQUIP)
		{
			return 1;
		}
		else if (bClass == ICLASS_GEM)
		{
			return 1;
		}
		else if (bClass == ICLASS_COMITEM)
		{
			return GetMedicData()->m_nCount;
		}
		else if (bClass == ICLASS_MATERIAL)
		{
			return GetMedicData()->m_nCount;
		}
		else if (bClass == ICLASS_TASKITEM)
		{
			return GetMedicData()->m_nCount;
		}
		else
			return 0;
	}

	VOID SetItemCount(INT nCount)
	{
		UCHAR bClass = ItemClass();

		if (bClass == ICLASS_COMITEM)
		{
			GetMedicData()->m_nCount = nCount;
		}
		else if (bClass == ICLASS_MATERIAL)
		{
			GetMedicData()->m_nCount = nCount;
		}
		else if (bClass == ICLASS_TASKITEM)
		{
			GetMedicData()->m_nCount = nCount;
		}
		else
		{
			assert(FALSE);
			return;
		}
	}

	BYTE	GetItemTileMax()	const
	{
		BYTE	bClass = ItemClass();

		switch (ItemClass())
		{
		case ICLASS_EQUIP:
			return 1;
			break;
		case ICLASS_GEM:
			return 1;
			break;
		case ICLASS_COMITEM:
		case ICLASS_MATERIAL:
		case ICLASS_TASKITEM:
		{
			return GetMedicData()->m_nLayedNum;
		}
		break;
		case ICLASS_STOREMAP:
		{
			return 1;
		}
		default:
		{
			// not implement class
			assert(FALSE);
			return 0;
		}
		}
	}

	// �Ƿ�ﵽ����������
	BOOL isFullTile() const
	{
		return GetItemCount() >= GetItemTileMax();
	}


	// ���ӵ�������
	BOOL IncCount(UINT nCount = 1)
	{
		const	BYTE	bClass = ItemClass();

		switch (bClass) {
		case ICLASS_EQUIP:
		{
			return FALSE;
		}
		break;
		case ICLASS_GEM:
		{
			return FALSE;
		}
		break;
		case ICLASS_COMITEM:
		case ICLASS_MATERIAL:
		case ICLASS_TASKITEM:
		{
			assert(!isFullTile());
			GetMedicData()->m_nCount += nCount;
			return TRUE;
		}
		break;
		default:
			return FALSE;
			break;
		}
	}
	// ���ٵ�������
	BOOL	DecCount(UINT nCount = 1)
	{
		const	BYTE	bClass = ItemClass();
		switch (bClass)
		{
		case ICLASS_EQUIP:
		{
			return FALSE;
		}
		break;
		case ICLASS_GEM:
		{
			return FALSE;
		}
		break;
		case ICLASS_TASKITEM:
		case ICLASS_MATERIAL:
		case ICLASS_COMITEM:
		{
			assert(GetItemCount() > 0);
			GetMedicData()->m_nCount -= nCount;
			return TRUE;
		}
		break;
		default:
			return FALSE;
			break;
		}
	}


	BOOL	IsNullType()	const
	{
		return m_ItemIndex == 0;
	}

	VOID	Read(SocketInputStream& iStream)
	{
		iStream.Read((CHAR*)(&m_ItemGuid), sizeof(_ITEM_GUID));
		iStream.Read((CHAR*)(&m_ItemIndex), sizeof(UINT));
		iStream.Read((CHAR*)(&m_rulerId), sizeof(CHAR));
		iStream.Read((CHAR*)(&m_nsBind), sizeof(CHAR));
		iStream.Read((CHAR*)(m_param), sizeof(INT)*MAX_ITEM_PARAM);

		if (GetCreatorVar())
		{
			iStream.Read(m_creator, sizeof(CHAR)*MAX_ITEM_CREATOR_NAME);
		}
		if (ItemClass() == ICLASS_EQUIP)
		{
			iStream.Read((CHAR*)(&GetEquipData()->m_CurDurPoint), sizeof(BYTE));
			iStream.Read((CHAR*)(&GetEquipData()->m_CurDamagePoint), sizeof(USHORT));
			iStream.Read((CHAR*)(&GetEquipData()->m_MaxDurPoint), sizeof(BYTE));
			iStream.Read((CHAR*)(&GetEquipData()->m_BasePrice), sizeof(UINT));
			iStream.Read((CHAR*)(&GetEquipData()->m_EquipPoint), sizeof(BYTE));
			iStream.Read((CHAR*)(&GetEquipData()->m_AttrCount), sizeof(BYTE));
			iStream.Read((CHAR*)(&GetEquipData()->m_SetNum), sizeof(SHORT));
			iStream.Read((CHAR*)(&GetEquipData()->m_MaxNum), sizeof(CHAR));
			if (GetEquipData()->m_AttrCount > MAX_ITEM_ATTR)
				GetEquipData()->m_AttrCount = MAX_ITEM_ATTR;

			for (INT j = 0; j < GetEquipData()->m_AttrCount; j++)
			{
				iStream.Read((CHAR*)(&GetEquipData()->m_pAttr[j]), sizeof(_ITEM_ATTR));
			}

			iStream.Read((CHAR*)(&GetEquipData()->m_StoneCount), sizeof(BYTE));

			if (GetEquipData()->m_StoneCount > MAX_ITEM_GEM)
				GetEquipData()->m_StoneCount = MAX_ITEM_GEM;

			for (INT j = 0; j < GetEquipData()->m_StoneCount; j++)
			{
				iStream.Read((CHAR*)(&GetEquipData()->m_pGemInfo[j]), sizeof(_ITEM_GEMINFO));
			}

			iStream.Read((CHAR*)(&GetEquipData()->m_NeedLevel), sizeof(BYTE));
			iStream.Read((CHAR*)(&GetEquipData()->m_GemMax), sizeof(BYTE));
			iStream.Read((CHAR*)(&GetEquipData()->m_FaileTimes), sizeof(BYTE));
		}
		else if (ItemClass() == ICLASS_GEM)
		{
			iStream.Read((CHAR*)(GetGemData()), sizeof(GEM_INFO));
		}
		else if (ItemClass() == ICLASS_COMITEM)
		{
			iStream.Read((CHAR*)(GetMedicData()), sizeof(MEDIC_INFO));
		}
		else if (ItemClass() == ICLASS_MATERIAL)
		{
			iStream.Read((CHAR*)(GetMedicData()), sizeof(MEDIC_INFO));
		}
		else if (ItemClass() == ICLASS_TASKITEM)
		{
			iStream.Read((CHAR*)(GetMedicData()), sizeof(MEDIC_INFO));
		}
		else if (ItemClass() == ICLASS_STOREMAP)
		{
			iStream.Read((CHAR*)(GetStoreMapData()), sizeof(STORE_MAP_INFO));
		}
	}
	VOID	Write(SocketOutputStream& oStream) const
	{
		oStream.Write((CHAR*)(&m_ItemGuid), sizeof(_ITEM_GUID));
		oStream.Write((CHAR*)(&m_ItemIndex), sizeof(UINT));
		oStream.Write((CHAR*)(&m_rulerId), sizeof(CHAR));
		oStream.Write((CHAR*)(&m_nsBind), sizeof(CHAR));
		oStream.Write((CHAR*)(m_param), sizeof(INT)*MAX_ITEM_PARAM);

		if (GetCreatorVar())
		{
			oStream.Write(m_creator, sizeof(CHAR)*MAX_ITEM_CREATOR_NAME);
		}

		BYTE bClass = ItemClass();

		if (bClass == ICLASS_EQUIP)
		{
			oStream.Write((CHAR*)(&GetEquipData()->m_CurDurPoint), sizeof(BYTE));
			oStream.Write((CHAR*)(&GetEquipData()->m_CurDamagePoint), sizeof(USHORT));
			oStream.Write((CHAR*)(&GetEquipData()->m_MaxDurPoint), sizeof(BYTE));
			oStream.Write((CHAR*)(&GetEquipData()->m_BasePrice), sizeof(UINT));
			oStream.Write((CHAR*)(&GetEquipData()->m_EquipPoint), sizeof(BYTE));
			oStream.Write((CHAR*)(&GetEquipData()->m_AttrCount), sizeof(BYTE));
			oStream.Write((CHAR*)(&GetEquipData()->m_SetNum), sizeof(SHORT));
			oStream.Write((CHAR*)(&GetEquipData()->m_MaxNum), sizeof(BYTE));

			for (INT j = 0; j < GetEquipData()->m_AttrCount; j++)
			{
				oStream.Write((CHAR*)(&GetEquipData()->m_pAttr[j]), sizeof(_ITEM_ATTR));
			}

			oStream.Write((CHAR*)(&GetEquipData()->m_StoneCount), sizeof(BYTE));

			for (INT j = 0; j < GetEquipData()->m_StoneCount; j++)
			{
				oStream.Write((CHAR*)(&GetEquipData()->m_pGemInfo[j]), sizeof(_ITEM_GEMINFO));
			}

			oStream.Write((CHAR*)(&GetEquipData()->m_NeedLevel), sizeof(BYTE));
			oStream.Write((CHAR*)(&GetEquipData()->m_GemMax), sizeof(BYTE));
			oStream.Write((CHAR*)(&GetEquipData()->m_FaileTimes), sizeof(BYTE));
		}
		else if (bClass == ICLASS_GEM)
		{
			oStream.Write((CHAR*)(GetGemData()), sizeof(GEM_INFO));
		}
		else if (bClass == ICLASS_COMITEM)
		{
			oStream.Write((CHAR*)(GetMedicData()), sizeof(MEDIC_INFO));
		}
		else if (bClass == ICLASS_MATERIAL)
		{
			oStream.Write((CHAR*)(GetMedicData()), sizeof(MEDIC_INFO));
		}
		else if (bClass == ICLASS_TASKITEM)
		{
			oStream.Write((CHAR*)(GetMedicData()), sizeof(MEDIC_INFO));
		}
		else if (bClass == ICLASS_STOREMAP)
		{
			oStream.Write((CHAR*)(GetStoreMapData()), sizeof(STORE_MAP_INFO));
		}
	}

	BOOL	WriteFixAttr(CHAR* pIn, INT BuffLength)
	{
		assert(pIn);

		BYTE bClass = ItemClass();

		CHAR*	pBuff = pIn;
		m_rulerId = *pBuff;
		pBuff++;

		BYTE	FixAttrCount = *((BYTE*)pBuff);
		pBuff += sizeof(BYTE);

		switch (bClass)
		{
		case ICLASS_EQUIP:
		{
			GetEquipData()->m_SetNum = *((SHORT*)pBuff);					//��װ���
			pBuff += sizeof(SHORT);

			GetEquipData()->m_BasePrice = *((UINT*)pBuff);					//�۳��۸�
			pBuff += sizeof(UINT);

			GetEquipData()->m_MaxNum = *pBuff;								//��װ
			pBuff += sizeof(CHAR);


			GetEquipData()->m_EquipPoint = *((BYTE*)pBuff);				//��Ʒװ���
			pBuff += sizeof(BYTE);

			GetEquipData()->m_MaxDurPoint = *((BYTE*)pBuff);				//����;�ֵ
			pBuff += sizeof(BYTE);

			GetEquipData()->m_NeedLevel = *((BYTE*)pBuff);					//����ȼ�
			pBuff += sizeof(BYTE);

			//GetEquipData()->m_GemMax	=	*((BYTE*)pBuff);					//���ʯ����
			//pBuff		+=	sizeof(BYTE);

			//�̶�������ǰ��
			for (INT i = 0; i < FixAttrCount; i++)
			{
				GetEquipData()->m_pAttr[i].m_AttrType = *((BYTE*)pBuff);
				pBuff += sizeof(BYTE);

				GetEquipData()->m_pAttr[i].m_Value.m_Value = *((SHORT*)pBuff);
				pBuff += sizeof(SHORT);
			}

			GetEquipData()->m_AttrCount = FixAttrCount; //�ȱ���FixAttrCount;

		}
		break;
		case ICLASS_GEM:
		{
			GetGemData()->m_nPrice = *((INT*)pBuff);
			pBuff += sizeof(INT);

			GetGemData()->m_Attr.m_AttrType = *((BYTE*)pBuff);
			pBuff += sizeof(BYTE);

			GetGemData()->m_Attr.m_Value.m_Value = *((SHORT*)pBuff);
			pBuff += sizeof(SHORT);


		}
		break;
		case ICLASS_COMITEM:
		case ICLASS_MATERIAL:
		case ICLASS_TASKITEM:
		{
			GetMedicData()->m_bCosSelf = *((BOOL*)pBuff);
			pBuff += sizeof(BOOL);
			GetMedicData()->m_nBasePrice = *((UINT*)pBuff);
			pBuff += sizeof(UINT);
			GetMedicData()->m_nLayedNum = *((BYTE*)pBuff);
			pBuff += sizeof(BYTE);
			GetMedicData()->m_nLevel = *((BYTE*)pBuff);
			pBuff += sizeof(BYTE);
			GetMedicData()->m_nReqSkill = *((INT*)pBuff);
			pBuff += sizeof(INT);
			GetMedicData()->m_nReqSkillLevel = *((BYTE*)pBuff);
			pBuff += sizeof(BYTE);
			GetMedicData()->m_nScriptID = *((INT*)pBuff);
			pBuff += sizeof(INT);
			GetMedicData()->m_nSkillID = *((INT*)pBuff);
			pBuff += sizeof(INT);
			GetMedicData()->m_TargetType = *((BYTE*)pBuff);
			pBuff += sizeof(BYTE);
		}
		break;
		case ICLASS_STOREMAP:
		{
			GetStoreMapData()->m_GrowType = *((INT*)pBuff);
			pBuff += sizeof(INT);
			GetStoreMapData()->m_nLevel = *((INT*)pBuff);
			pBuff += sizeof(INT);
			GetStoreMapData()->m_SceneID = *((INT*)pBuff);
			pBuff += sizeof(INT);
			GetStoreMapData()->m_xPos = *((FLOAT*)pBuff);
			pBuff += sizeof(FLOAT);
			GetStoreMapData()->m_zPos = *((FLOAT*)pBuff);
			pBuff += sizeof(FLOAT);
		}
		break;
		default:
		{
			assert(FALSE);
		}
		}

		assert(BuffLength >= (pBuff - pIn));

		return TRUE;
	}
	BOOL	ReadFixAttr(CHAR* pOut, INT& OutLength, INT BuffLength)
	{
		assert(pOut);

		BYTE bClass = ItemClass();

		CHAR*	pBuff = pOut;
		*pBuff = m_rulerId;
		pBuff++;

		BYTE	FixAttrCount = GetFixAttrCount();

		*((BYTE*)pBuff) = FixAttrCount;
		pBuff++;
		switch (bClass)
		{
		case ICLASS_EQUIP:
		{


			*((SHORT*)pBuff) = GetEquipData()->m_SetNum;					//��װ���
			pBuff += sizeof(SHORT);
			*((UINT*)pBuff) = GetEquipData()->m_BasePrice;				//�۳��۸�
			pBuff += sizeof(UINT);
			*pBuff = GetEquipData()->m_MaxNum;					//��װ
			pBuff += sizeof(CHAR);
			*((BYTE*)pBuff) = GetEquipData()->m_EquipPoint;				//��Ʒװ���
			pBuff += sizeof(BYTE);
			*((BYTE*)pBuff) = GetEquipData()->m_MaxDurPoint;				//����;�ֵ
			pBuff += sizeof(BYTE);
			*((BYTE*)pBuff) = GetEquipData()->m_NeedLevel;					//����ȼ�
			pBuff += sizeof(BYTE);
			//*((BYTE*)pBuff)  =	GetEquipData()->m_GemMax;					//���ʯ����
			//pBuff		+=	sizeof(BYTE);

			// �̶�������ǰ��
			for (INT i = 0; i < FixAttrCount; i++)
			{
				*((BYTE*)pBuff) = GetEquipData()->m_pAttr[i].m_AttrType;
				pBuff += sizeof(BYTE);

				*((SHORT*)pBuff) = GetEquipData()->m_pAttr[i].m_Value.m_Value;
				pBuff += sizeof(SHORT);
			}

		}
		break;
		case ICLASS_GEM:
		{
			*((INT*)pBuff) = GetGemData()->m_nPrice;
			pBuff += sizeof(INT);
			*((BYTE*)pBuff) = GetGemData()->m_Attr.m_AttrType;
			pBuff += sizeof(BYTE);
			*((SHORT*)pBuff) = GetGemData()->m_Attr.m_Value.m_Value;
			pBuff += sizeof(SHORT);


		}
		break;
		case ICLASS_COMITEM:
		case ICLASS_MATERIAL:
		case ICLASS_TASKITEM:
		{
			*((BOOL*)pBuff) = GetMedicData()->m_bCosSelf;
			pBuff += sizeof(BOOL);
			*((UINT*)pBuff) = GetMedicData()->m_nBasePrice;
			pBuff += sizeof(UINT);
			*((BYTE*)pBuff) = GetMedicData()->m_nLayedNum;
			pBuff += sizeof(BYTE);
			*((BYTE*)pBuff) = GetMedicData()->m_nLevel;
			pBuff += sizeof(BYTE);
			*((INT*)pBuff) = GetMedicData()->m_nReqSkill;
			pBuff += sizeof(INT);
			*((BYTE*)pBuff) = GetMedicData()->m_nReqSkillLevel;
			pBuff += sizeof(BYTE);
			*((INT*)pBuff) = GetMedicData()->m_nScriptID;
			pBuff += sizeof(INT);
			*((INT*)pBuff) = GetMedicData()->m_nSkillID;
			pBuff += sizeof(INT);
			*((BYTE*)pBuff) = GetMedicData()->m_TargetType;
			pBuff += sizeof(BYTE);
		}
		break;
		case ICLASS_STOREMAP:
		{
			*((INT*)pBuff) = GetStoreMapData()->m_GrowType;
			pBuff += sizeof(INT);
			*((INT*)pBuff) = GetStoreMapData()->m_nLevel;
			pBuff += sizeof(INT);
			*((INT*)pBuff) = GetStoreMapData()->m_SceneID;
			pBuff += sizeof(INT);
			*((FLOAT*)pBuff) = GetStoreMapData()->m_xPos;
			pBuff += sizeof(FLOAT);
			*((FLOAT*)pBuff) = GetStoreMapData()->m_zPos;
			pBuff += sizeof(FLOAT);
		}
		break;
		default:
		{
			assert(FALSE);
		}
		}
		OutLength = (INT)(pBuff - pOut);
		assert(BuffLength >= OutLength);

		return TRUE;
	}

	// һ��Ҫ�����л��̶�����
	// CHAR* pIn ʵ��������������
	BOOL	WriteVarAttr(CHAR* pIn, INT BuffLength)
	{
		assert(pIn);

		BYTE bClass = ItemClass();

		CHAR*	pBuff = pIn;
		m_nsBind = *pBuff;
		pBuff++;
		switch (bClass)
		{
		case ICLASS_EQUIP:
		{
			GetEquipData()->m_CurDurPoint = *((BYTE*)pBuff);
			pBuff += sizeof(BYTE);
			GetEquipData()->m_FaileTimes = *((BYTE*)pBuff);
			pBuff += sizeof(BYTE);

			BYTE FixAttrCount = GetEquipData()->m_AttrCount;
			GetEquipData()->m_AttrCount = *((BYTE*)pBuff);
			pBuff += sizeof(BYTE);
			//��һ���������

			assert(GetEquipData()->m_AttrCount <= MAX_ITEM_ATTR);
			//ÿ���������һ��INT
			for (INT i = FixAttrCount; i < GetEquipData()->m_AttrCount; i++)
			{
				GetEquipData()->m_pAttr[i].m_AttrType = (BYTE)(*((SHORT*)pBuff));	//����ռһ��Short
				pBuff += sizeof(SHORT);

				GetEquipData()->m_pAttr[i].m_Value.m_Value = *((SHORT*)pBuff);//����ֵռһ��Short
				pBuff += sizeof(SHORT);
			}

			GetEquipData()->m_StoneCount = (BYTE)(*((BYTE*)pBuff)); //��ʯ����һ��SHORT
			pBuff += sizeof(SHORT);

			GetEquipData()->m_GemMax = (BYTE)(*((BYTE*)pBuff)); //��ʯ����һ��SHORT
			pBuff += sizeof(BYTE);

			GetEquipData()->m_CurDamagePoint = *((USHORT*)pBuff); //���˶�һ��SHORT
			pBuff += sizeof(USHORT);

			assert(GetEquipData()->m_StoneCount <= MAX_ITEM_GEM);

			//ÿ����ʯ����һ��INT
			for (int i = 0; i < MAX_ITEM_GEM; i++)
			{
				GetEquipData()->m_pGemInfo[i].m_GemType = *((UINT*)pBuff);
				pBuff += sizeof(UINT);
			}

		}
		break;
		case ICLASS_GEM:
		{

		}
		break;
		case ICLASS_COMITEM:
		case ICLASS_MATERIAL:
		case ICLASS_TASKITEM:
		{
			GetMedicData()->m_nCount = *((BYTE*)pBuff);
		}
		break;
		case ICLASS_STOREMAP:
		{

		}
		break;
		default:
		{
			assert(FALSE);
		}
		}

		assert(BuffLength >= (pBuff - pIn));

		return TRUE;
	}
	BOOL	ReadVarAttr(CHAR* pOut, INT& OutLength, INT BuffLength)
	{
		assert(pOut);

		BYTE bClass = ItemClass();

		CHAR*	pBuff = pOut;
		*pBuff = m_nsBind;
		pBuff++;
		switch (bClass)
		{
		case ICLASS_EQUIP:
		{
			*((BYTE*)pBuff) = GetEquipData()->m_CurDurPoint;
			pBuff += sizeof(BYTE);

			*((BYTE*)pBuff) = GetEquipData()->m_FaileTimes;
			pBuff += sizeof(BYTE);

			*((BYTE*)pBuff) = GetEquipData()->m_AttrCount;
			pBuff += sizeof(BYTE);

			BYTE FixAttrCount = GetFixAttrCount();

			assert(GetEquipData()->m_AttrCount <= MAX_ITEM_ATTR);
			for (INT i = FixAttrCount; i < GetEquipData()->m_AttrCount; i++)
			{
				*((SHORT*)pBuff) = GetEquipData()->m_pAttr[i].m_AttrType;
				pBuff += sizeof(SHORT);

				*((SHORT*)pBuff) = GetEquipData()->m_pAttr[i].m_Value.m_Value;
				pBuff += sizeof(SHORT);
			}

			*((SHORT*)pBuff) = GetEquipData()->m_StoneCount;
			pBuff += sizeof(SHORT);

			*((BYTE*)pBuff) = GetEquipData()->m_GemMax;
			pBuff += sizeof(BYTE);

			*((USHORT*)pBuff) = GetEquipData()->m_CurDamagePoint;
			pBuff += sizeof(USHORT);

			assert(GetEquipData()->m_StoneCount <= MAX_ITEM_GEM);

			for (int i = 0; i < MAX_ITEM_GEM; i++)
			{
				*((UINT*)pBuff) = GetEquipData()->m_pGemInfo[i].m_GemType;
				pBuff += sizeof(UINT);
			}

		}
		break;
		case ICLASS_GEM:
		{

		}
		break;
		case ICLASS_COMITEM:
		case ICLASS_MATERIAL:
		case ICLASS_TASKITEM:
		{
			*((BYTE*)pBuff) = GetMedicData()->m_nCount;
			pBuff += sizeof(BYTE);
		}
		break;
		case ICLASS_STOREMAP:
		{

		}
		break;
		default:
		{
			assert(FALSE);
		}
		}

		OutLength = (INT)(pBuff - pOut);
		assert(BuffLength >= OutLength);
		return TRUE;
	}

	BOOL	IsFixAttr(BYTE attrType)
	{
		switch (attrType)
		{

		case IATTRIBUTE_BASE_ATTACK_P:
		case IATTRIBUTE_BASE_ATTACK_M:
		case IATTRIBUTE_BASE_ATTACK_TIME:
		case IATTRIBUTE_BASE_DEFENCE_P:
		case IATTRIBUTE_BASE_DEFENCE_M:
		case IATTRIBUTE_BASE_MISS:
		{
			return TRUE;
		}
		break;
		default:
			return FALSE;
		}
	}
	BYTE	GetFixAttrCount()
	{
		INT iAttrCount = GetEquipData()->m_AttrCount - GetEquipData()->m_StoneCount;

		INT i = 0;
		INT FixAttrCount = 0;
		for (; i < iAttrCount; i++)
		{
			if (IsFixAttr(GetEquipData()->m_pAttr[i].m_AttrType)
				&& GetEquipData()->m_pAttr[i].m_Value.m_Value>0)
				FixAttrCount++;

		}
		return  FixAttrCount;
	}

	BOOL	GetItemBind() const
	{
		if (m_nsBind & IEI_BIND_INFO)
			return TRUE;
		return FALSE;
	}
	VOID	SetItemBind(BOOL bBind)
	{
		if (bBind)
			m_nsBind |= IEI_BIND_INFO;
		else
			m_nsBind &= (~(IEI_BIND_INFO));	// �������һ�㲻��ִ��
	}

	BOOL	GetItemIdent() const
	{
		if (m_nsBind & IEI_IDEN_INFO)
			return TRUE;
		return FALSE;
	}
	VOID	SetItemIdent(BOOL bIdent)
	{
		if (bIdent)
			m_nsBind |= IEI_IDEN_INFO;
		else
			m_nsBind &= (~(IEI_IDEN_INFO));	// �������һ�㲻��ִ��
	}

	BOOL	GetItemPLock() const	//��������
	{
		if (m_nsBind & IEI_PLOCK_INFO)
			return TRUE;
		return FALSE;
	}
	VOID	SetItemPLock(BOOL bLock)
	{
		if (bLock)
			m_nsBind |= IEI_PLOCK_INFO;
		else
			m_nsBind &= (~(IEI_PLOCK_INFO));	// �������һ�㲻��ִ��
	}

	INT		GetItemFailTimes() const
	{
		return m_Equip.m_FaileTimes;
	}
	VOID	SetItemFailTimes(INT nTimes)
	{
		m_Equip.m_FaileTimes = nTimes;
	}
	//������
	BOOL	GetCreatorVar() const
	{
		if (m_nsBind & IEL_CREATOR)
			return TRUE;
		return FALSE;
	}
	VOID	SetCreatorVar(BOOL bCreator)
	{
		if (bCreator)
			m_nsBind |= IEL_CREATOR;
		else
			m_nsBind &= (~(IEL_CREATOR));//�������һ�㲻��ִ��
	}

	INT		GetItemParamValue(UINT Start, const ItemParamValue& ipv)
	{
		INT TotalSize = sizeof(INT)*MAX_ITEM_PARAM;
		INT nSize;
		switch (ipv)
		{
		case IPV_CHAR:
		{
			nSize = 1;
			if (Start > ((UINT)TotalSize - (UINT)nSize))
			{
				assert(FALSE);
			}
			return *((CHAR*)m_param + Start);
		}
		break;
		case IPV_SHORT:
		{
			nSize = 2;
			if (Start > ((UINT)TotalSize - (UINT)nSize))
			{
				assert(FALSE);
			}
			return *((SHORT*)((CHAR*)m_param + Start));
		}
		break;
		case IPV_INT:
		{
			nSize = 4;
			if (Start > ((UINT)TotalSize - (UINT)nSize))
			{
				assert(FALSE);
			}
			return *((INT*)((CHAR*)m_param + Start));
		}
		break;
		default:
			assert(FALSE);
		}

		return -1;

	}
	VOID	SetItemParam(UINT start, ItemParamValue& ipv, INT value)
	{
		INT TotalSize = sizeof(INT)*MAX_ITEM_PARAM;
		INT size;

		switch (ipv)
		{
		case IPV_CHAR:
		{
			size = 1;
			if (start > ((UINT)TotalSize - (UINT)size))
			{
				assert(FALSE);
			}
			*((CHAR*)m_param + start) = (CHAR)value;
		}
		break;
		case IPV_SHORT:
		{
			size = 2;
			if (start > ((UINT)TotalSize - (UINT)size))
			{
				assert(FALSE);
			}
			*((SHORT*)((CHAR*)m_param + start)) = (SHORT)value;
		}
		break;
		case IPV_INT:
		{
			size = 4;
			if (start > ((UINT)TotalSize - (UINT)size))
			{
				assert(FALSE);
			}
			*((INT*)((CHAR*)m_param + start)) = (INT)value;
		}
		break;
		default:
			assert(FALSE);
		}
	}
};

// �����Ʒ��Ч���ݴ�С
static UINT	GetItemRealSize(const _ITEM& it)
{
	INT CreatorSize = it.GetCreatorVar() ? (sizeof(CHAR)*MAX_ITEM_CREATOR_NAME) : 0;
	if (it.ItemClass() == ICLASS_EQUIP)
	{

		return	sizeof(_ITEM_GUID) +
			sizeof(UINT) +
			sizeof(CHAR) +
			sizeof(CHAR) +
			sizeof(INT)*MAX_ITEM_PARAM +
			//////////////////////////////////////////////////////////////////////////
			sizeof(SHORT) * 2 +
			sizeof(UINT) +
			sizeof(_ITEM_ATTR)*it.m_Equip.m_AttrCount +
			sizeof(_ITEM_GEMINFO)*it.m_Equip.m_StoneCount +
			sizeof(BYTE) * 9 +
			CreatorSize;

	}
	else if (it.ItemClass() == ICLASS_GEM)
	{
		return	sizeof(_ITEM_GUID) +
			sizeof(UINT) +
			sizeof(CHAR) +
			sizeof(CHAR) +
			sizeof(INT)*MAX_ITEM_PARAM +
			CreatorSize +
			sizeof(GEM_INFO);
	}
	else if (it.ItemClass() == ICLASS_COMITEM)
	{

		return	sizeof(_ITEM_GUID) +
			sizeof(UINT) +
			sizeof(CHAR) +
			sizeof(CHAR) +
			sizeof(INT)*MAX_ITEM_PARAM +
			CreatorSize +
			sizeof(MEDIC_INFO);
	}
	else if (it.ItemClass() == ICLASS_MATERIAL)
	{
		return	sizeof(_ITEM_GUID) +
			sizeof(UINT) +
			sizeof(CHAR) +
			sizeof(CHAR) +
			sizeof(INT)*MAX_ITEM_PARAM +
			CreatorSize +
			sizeof(MEDIC_INFO);
	}
	else if (it.ItemClass() == ICLASS_TASKITEM)
	{
		return	sizeof(_ITEM_GUID) +
			sizeof(UINT) +
			sizeof(CHAR) +
			sizeof(CHAR) +
			sizeof(INT)*MAX_ITEM_PARAM +
			CreatorSize +
			sizeof(MEDIC_INFO);
	}
	else if (it.ItemClass() == ICLASS_STOREMAP)
	{
		return 	sizeof(_ITEM_GUID) +
			sizeof(UINT) +
			sizeof(CHAR) +
			sizeof(CHAR) +
			sizeof(INT)*MAX_ITEM_PARAM +
			CreatorSize +
			sizeof(STORE_MAP_INFO);

	}
	else
		return	sizeof(_ITEM_GUID) +
		sizeof(UINT) +
		sizeof(CHAR) +
		sizeof(INT)*MAX_ITEM_PARAM +
		CreatorSize +
		sizeof(CHAR);
}

#endif
