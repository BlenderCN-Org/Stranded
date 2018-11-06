#ifndef __GAME_STRUCT_ITEM_H__
#define __GAME_STRUCT_ITEM_H__

#include "../Common/define.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

#define MAX_ITEM_CREATOR_NAME 13
#define MAX_ITEM_PARAM 3
// 物品可镶嵌宝石的最大个数
#define MAX_ITEM_GEM 3
// 最大物品属性
#define MAX_ITEM_ATTR (9+MAX_ITEM_GEM)
// 无效的ID值
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

//物品信息位定义
enum ITEM_EXT_INFO
{
	IEI_BIND_INFO = 0x00000001,	//绑定信息
	IEI_IDEN_INFO = 0x00000002, //鉴定信息
	IEI_PLOCK_INFO = 0x00000004, //二级密码已经处理
	IEI_BLUE_ATTR = 0x00000008, //是否有蓝属性
	IEL_CREATOR = 0x00000010,	//是否有创造者
};

enum ITEM_CLASS
{
	ICLASS_EQUIP = 1,	// 武器WEAPON、防具DEFENCE、饰物ADORN
	ICLASS_MATERIAL = 2,	// 原料
	ICLASS_COMITEM = 3,	// 药品
	ICLASS_TASKITEM = 4,	// 任务物品
	ICLASS_GEM = 5, // 宝石
	ICLASS_STOREMAP = 6,	// 藏宝图
	ICLASS_TALISMAN = 7,	// 法宝---???
	ICLASS_GUILDITEM = 8,	// 帮会物品
	ICLASS_NUMBER, // 物品的类别数量
};

enum ITEM_ATTRIBUTE
{
	IATTRIBUTE_POINT_MAXHP = 0,		//按点数增加HP的上限
	IATTRIBUTE_RATE_MAXHP = 1,		//按百分比增加HP的上限
	IATTRIBUTE_RESTORE_HP = 2,		//加快HP的回复速度

	IATTRIBUTE_POINT_MAXMP = 3,		//按点数增加MP的上限
	IATTRIBUTE_RATE_MAXMP = 4,		//按百分比增加MP的上限
	IATTRIBUTE_RESTORE_MP = 5,		//加快MP的回复速度

	IATTRIBUTE_COLD_ATTACK = 6,		//冰攻击
	IATTRIBUTE_COLD_RESIST = 7,		//冰抵抗
	IATTRIBUTE_COLD_TIME = 8,		//减少冰冻迟缓时间
	IATTRIBUTE_FIRE_ATTACK = 9,		//火攻击
	IATTRIBUTE_FIRE_RESIST = 10,	//火抵抗
	IATTRIBUTE_FIRE_TIME = 11,	//减少火烧持续时间
	IATTRIBUTE_LIGHT_ATTACK = 12,	//电攻击
	IATTRIBUTE_LIGHT_RESIST = 13,	//电抵抗
	IATTRIBUTE_LIGHT_TIME = 14,	//减少电击眩晕时间
	IATTRIBUTE_POISON_ATTACK = 15,	//毒攻击
	IATTRIBUTE_POISON_RESIST = 16,	//毒抵抗
	IATTRIBUTE_POISON_TIME = 17,	//减少中毒时间
	IATTRIBUTE_RESIST_ALL = 18,	//按百分比抵消所有属性攻击

	IATTRIBUTE_ATTACK_P = 19,	//物理攻击
	IATTRIBUTE_RATE_ATTACK_P = 20,	//按百分比增加物理攻击
	IATTRIBUTE_RATE_ATTACK_EP = 21,	//对装备基础物理攻击百分比加成
	IATTRIBUTE_DEFENCE_P = 22,	//物理防御
	IATTRIBUTE_RATE_DEFENCE_P = 23,	//按百分比增加物理防御
	IATTRIBUTE_RATE_DEFENCE_EP = 24,	//对装备基础物理防御百分比加成
	IATTRIBUTE_IMMUNITY_P = 25,	//按百分比抵消物理伤害

	IATTRIBUTE_ATTACK_M = 26,	//魔法攻击
	IATTRIBUTE_RATE_ATTACK_M = 27,	//按百分比增加魔法攻击
	IATTRIBUTE_RATE_ATTACK_EM = 28,	//对装备基础魔法攻击百分比加成
	IATTRIBUTE_DEFENCE_M = 29,	//魔法防御
	IATTRIBUTE_RATE_DEFENCE_M = 30,	//按百分比增加魔法防御
	IATTRIBUTE_RATE_DEFENCE_EM = 31,	//对装备基础魔法防御百分比加成
	IATTRIBUTE_IMMUNITY_M = 32,	//按百分比抵消魔法伤害

	IATTRIBUTE_ATTACK_SPEED = 33,	//攻击速度
	IATTRIBUTE_SKILL_TIME = 34,	//魔法冷却速度

	IATTRIBUTE_HIT = 35,	//命中
	IATTRIBUTE_MISS = 36,	//闪避
	IATTRIBUTE_2ATTACK_RATE = 37,	//会心一击（双倍攻击）的百分比
	IATTRIBUTE_NO_DEFENCE_RATE = 38,	//无视对方防御的概率

	IATTRIBUTE_SPEED_RATE = 39,	//移动速度百分比

	IATTRIBUTE_DAMAGE_RET = 40,	//伤害反射
	IATTRIBUTE_DAMAGE2MANA = 41,	//伤害由内力抵消

	IATTRIBUTE_STR = 42,	//增加力量
	IATTRIBUTE_SPR = 43,	//增加灵气
	IATTRIBUTE_CON = 44,	//增加体制
	IATTRIBUTE_INT = 45,	//增加定力
	IATTRIBUTE_DEX = 46,	//增加身法
	IATTRIBUTE_LUK = 47,	//增加悟性
	IATTRIBUTE_ALL = 48,	//增加所有的人物一级属性

	IATTRIBUTE_HP_THIEVE = 49,	//生命偷取(从伤害里)
	IATTRIBUTE_MP_THIEVE = 50,	//内力偷取(从伤害里)

	IATTRIBUTE_USESKILL = 51,	//增加某个使用技能
	IATTRIBUTE_RAND_SKILL = 52,	//增加某个随机技能
	IATTRIBUTE_SKILL_RATE = 53,	//随机技能发动概率

	IATTRIBUTE_BASE_ATTACK_P = 54,	//基础物理攻击
	IATTRIBUTE_BASE_ATTACK_M = 55,	//基础魔法攻击
	IATTRIBUTE_BASE_ATTACK_TIME = 56,	//基础攻击速度（只对普通攻击）
	IATTRIBUTE_BASE_DEFENCE_P = 57,	//基础物理防御
	IATTRIBUTE_BASE_DEFENCE_M = 58,	//基础魔法防御
	IATTRIBUTE_BASE_MISS = 59,	//基础闪避



	IATTRIBUTE_NUMBER, //物品属性类型总数
};


// 物品唯一ID
struct _ITEM_GUID
{
	BYTE		m_world;			// 世界号: (例)101
	BYTE		m_server;			// 服务器程序号: (例)5
	INT			m_serial;			// 物品序列号: (例)123429

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

	BYTE					m_nCount;			// 当前数量
	BYTE					m_nLevel;
	BYTE					m_nLayedNum;		// 叠放数量
	BYTE					m_nReqSkillLevel;

	UINT					m_nBasePrice;
	INT						m_nScriptID;
	INT						m_nSkillID;
	BOOL					m_bCosSelf;			// 是否消耗自己
	INT						m_nReqSkill;
	BYTE					m_TargetType;		// 物品选择类型

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


// 镶嵌宝石信息
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

// 物品属性值
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

// 物品属性
struct _ITEM_ATTR
{
	BYTE				m_AttrType;		// 如果是普通装备，则m_AttrType用enum ITEM_ATTRIBUTE
										//    来表示属性类型，m_Value表示具体属性值
										// 如果是绿色的套装，则m_AttrType表示当前套装在
										//    玩家身上的装备件数，m_Value表示套装的索引号
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
	SHORT				m_SetNum;					// 套装编号
	UINT				m_BasePrice;				// 售出价格
	CHAR				m_MaxNum;					// 套装
	BYTE				m_EquipPoint;				// 物品装配点
	BYTE				m_MaxDurPoint;				// 最大耐久值
	BYTE				m_NeedLevel;				// 需求等级
	BYTE				m_GemMax;					// 最大宝石个数
	//////////////////////////////////////////////////////////////////////////
	// 以上为固定属性
	BYTE				m_FaileTimes;				// 修理失败次数
	BYTE				m_CurDurPoint;				// 当前耐久值
	USHORT				m_CurDamagePoint;			// 当前损伤度
	BYTE				m_AttrCount;				// 属性的数量
	BYTE				m_StoneCount;				// 镶嵌宝石的数量
	_ITEM_ATTR			m_pAttr[MAX_ITEM_ATTR];		// 物品属性
	_ITEM_GEMINFO		m_pGemInfo[MAX_ITEM_GEM];	// 物品里镶嵌的宝石

	VOID CleanUp()
	{
		memset(this, 0, sizeof(EQUIP_INFO));
	};
};

struct _ITEM
{
	_ITEM_GUID				m_ItemGuid;						// 物品固定信息(不能改变)
	UINT					m_ItemIndex;					// 物品类型
															/*
															|	    1 ~ 10000		白色装备
															|	10001 ~ 20000		绿色装备
															|	20001 ~ 30000		蓝色装备
															|	30001 ~ 35000		药瓶
															|	35001 ~ 40000		宝石
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
	// 物品个数方法
	// 能叠放物品		返回 实际大小
	// 不能叠放物品		返回 1
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

	// 是否达到单叠最大个数
	BOOL isFullTile() const
	{
		return GetItemCount() >= GetItemTileMax();
	}


	// 增加叠放数量
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
	// 减少叠放数量
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
			GetEquipData()->m_SetNum = *((SHORT*)pBuff);					//套装编号
			pBuff += sizeof(SHORT);

			GetEquipData()->m_BasePrice = *((UINT*)pBuff);					//售出价格
			pBuff += sizeof(UINT);

			GetEquipData()->m_MaxNum = *pBuff;								//套装
			pBuff += sizeof(CHAR);


			GetEquipData()->m_EquipPoint = *((BYTE*)pBuff);				//物品装佩点
			pBuff += sizeof(BYTE);

			GetEquipData()->m_MaxDurPoint = *((BYTE*)pBuff);				//最大耐久值
			pBuff += sizeof(BYTE);

			GetEquipData()->m_NeedLevel = *((BYTE*)pBuff);					//需求等级
			pBuff += sizeof(BYTE);

			//GetEquipData()->m_GemMax	=	*((BYTE*)pBuff);					//最大宝石个数
			//pBuff		+=	sizeof(BYTE);

			//固定属性在前面
			for (INT i = 0; i < FixAttrCount; i++)
			{
				GetEquipData()->m_pAttr[i].m_AttrType = *((BYTE*)pBuff);
				pBuff += sizeof(BYTE);

				GetEquipData()->m_pAttr[i].m_Value.m_Value = *((SHORT*)pBuff);
				pBuff += sizeof(SHORT);
			}

			GetEquipData()->m_AttrCount = FixAttrCount; //先保存FixAttrCount;

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


			*((SHORT*)pBuff) = GetEquipData()->m_SetNum;					//套装编号
			pBuff += sizeof(SHORT);
			*((UINT*)pBuff) = GetEquipData()->m_BasePrice;				//售出价格
			pBuff += sizeof(UINT);
			*pBuff = GetEquipData()->m_MaxNum;					//套装
			pBuff += sizeof(CHAR);
			*((BYTE*)pBuff) = GetEquipData()->m_EquipPoint;				//物品装佩点
			pBuff += sizeof(BYTE);
			*((BYTE*)pBuff) = GetEquipData()->m_MaxDurPoint;				//最大耐久值
			pBuff += sizeof(BYTE);
			*((BYTE*)pBuff) = GetEquipData()->m_NeedLevel;					//需求等级
			pBuff += sizeof(BYTE);
			//*((BYTE*)pBuff)  =	GetEquipData()->m_GemMax;					//最大宝石个数
			//pBuff		+=	sizeof(BYTE);

			// 固定属性在前面
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

	// 一定要先序列化固定属性
	// CHAR* pIn 实际上是整数序列
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
			//第一个整数完毕

			assert(GetEquipData()->m_AttrCount <= MAX_ITEM_ATTR);
			//每个随机属性一个INT
			for (INT i = FixAttrCount; i < GetEquipData()->m_AttrCount; i++)
			{
				GetEquipData()->m_pAttr[i].m_AttrType = (BYTE)(*((SHORT*)pBuff));	//类型占一个Short
				pBuff += sizeof(SHORT);

				GetEquipData()->m_pAttr[i].m_Value.m_Value = *((SHORT*)pBuff);//类型值占一个Short
				pBuff += sizeof(SHORT);
			}

			GetEquipData()->m_StoneCount = (BYTE)(*((BYTE*)pBuff)); //宝石个数一个SHORT
			pBuff += sizeof(SHORT);

			GetEquipData()->m_GemMax = (BYTE)(*((BYTE*)pBuff)); //宝石个数一个SHORT
			pBuff += sizeof(BYTE);

			GetEquipData()->m_CurDamagePoint = *((USHORT*)pBuff); //损伤度一个SHORT
			pBuff += sizeof(USHORT);

			assert(GetEquipData()->m_StoneCount <= MAX_ITEM_GEM);

			//每个宝石类型一个INT
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
			m_nsBind &= (~(IEI_BIND_INFO));	// 这个操作一般不会执行
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
			m_nsBind &= (~(IEI_IDEN_INFO));	// 这个操作一般不会执行
	}

	BOOL	GetItemPLock() const	//密码锁定
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
			m_nsBind &= (~(IEI_PLOCK_INFO));	// 这个操作一般不会执行
	}

	INT		GetItemFailTimes() const
	{
		return m_Equip.m_FaileTimes;
	}
	VOID	SetItemFailTimes(INT nTimes)
	{
		m_Equip.m_FaileTimes = nTimes;
	}
	//创建者
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
			m_nsBind &= (~(IEL_CREATOR));//这个操作一般不会执行
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

// 获得物品有效数据大小
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
