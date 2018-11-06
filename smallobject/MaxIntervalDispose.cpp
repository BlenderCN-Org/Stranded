#include "MaxIntervalDispose.h"
#include "SingleObvCalcV3.h"
#include "SingleConvCalcV3.h"
#include "GlobalVarSBH.h"
#include "ResourceMgr.h"
#include "AM_API.h"

long GetResId(WorkResInfo& workResInfo)
{
	if(workResInfo.resRelaID != NULL)
		return workResInfo.resRelaID->GetResId();
	else 
		return 0;
}



CMaxIntervalDispose::CMaxIntervalDispose(void)
{
}

CMaxIntervalDispose::~CMaxIntervalDispose(void)
{
}

//计算两工作间最大间隔
SBH_INT CMaxIntervalDispose::CalcMaxInterval( SBH_INT preWorkId,SBH_INT postWorkId,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo,DirType dir ,SBH_BOOL bFirst )
{
	WorkPtr& preWorkPtr = AMAPI::GetWorkPtrMap()->Get(preWorkId);
	WorkPtr& postWorkPtr = AMAPI::GetWorkPtrMap()->Get(postWorkId);
	if(preWorkPtr == NULL || postWorkPtr == NULL)
		return CANNOTRESOLVE;

	//首先清理类变量
	Init();

	if(bFirst)
		m_changeRes = true;

	//取得原计划备份的副本
	GetWorkPlan(preWorkId,m_preWorkPlan);
	GetWorkPlan(postWorkId,m_postWorkPlan);

	//清除资源计划
	AMAPI::ClearWorkPlan(preWorkId);
	AMAPI::ClearWorkPlan(postWorkId);
	//pData->RetractWorkPlan(preWorkId);
	//pData->RetractWorkPlan(postWorkId);

	if(CalcSingleLoop(preWorkPtr,postWorkPtr,preWorkResInfo,postWorkResInfo,dir))
	{	
		return m_checkflg;
	}
	else
	{
		return CANNOTRESOLVE;
	}

}

void CMaxIntervalDispose::GetWorkPlan( SBH_INT workId,TestSchedulerInfo& workSchInfo )
{
	WorkPtr& workPtr = AMAPI::GetWorkPtrMap()->Get(workId);
	if(workPtr == NULL)
		return;

	//工作计划列表
	list<WorkSchPlanPtr> schPlanlanPtrList = workPtr->GetSchPlanList();

	map<SBH_INT, map<SBH_ULONG,TimeRangeList> > 	planList;

	//遍历计划，取对应信息
	for(list<WorkSchPlanPtr>::iterator iter = schPlanlanPtrList.begin();
		iter != schPlanlanPtrList.end();iter++)
	{
		WorkSchPlanPtr& schPlan = *iter;
		planList[schPlan->GetType()][schPlan->GetPlanSelResId()].push_back(TimeRange(schPlan->GetStartTime(),schPlan->GetEndTime(),(SBH_DOUBLE)schPlan->GetUsedQuantity()));
	}

	//设置删除计划，撤销资源能力
	for(map<SBH_INT, map<SBH_ULONG,TimeRangeList> >::iterator iter = planList.begin();
		iter != planList.end();iter++)
	{
		workSchInfo.InsertAddPlan(workPtr->GetId(),iter->first,iter->second);
	}
}

//单资源组合最大间隔递归
SBH_BOOL CMaxIntervalDispose::CalcSingleLoop( WorkPtr& preWorkPtr ,WorkPtr& postWorkPtr,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo ,DirType dir )
{
	SBH_BOOL trFlg = false;
	if(dir == Dir_Obverse)//正向
	{
		trFlg = CalcSingleLoopObv(preWorkPtr,postWorkPtr,preWorkResInfo,postWorkResInfo);
	}
	else	//逆向
	{
		trFlg = CalcSingleLoopConv(preWorkPtr,postWorkPtr,preWorkResInfo,postWorkResInfo);
	}

	//确认计划
	m_preWorkPlan.SurePlan();
	m_postWorkPlan.SurePlan();

	return trFlg;
}

//正向单资源组合最大间隔求解
SBH_BOOL CMaxIntervalDispose::CalcSingleLoopObv( WorkPtr& preWorkPtr ,WorkPtr& postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo )
{
	//求取最大最小间隔
	int minInterval = AMAPI::GetMinMaxInterval(preWorkPtr,postWorkPtr,MININTELVAL,preWorkResInfo.resRelaID,postWorkResInfo.resRelaID);
	int maxInterval = AMAPI::GetMinMaxInterval(preWorkPtr,postWorkPtr,MAXINTELVAL,preWorkResInfo.resRelaID,postWorkResInfo.resRelaID);

	AppendWork* pPreWork = pData->GetAppendWork(preWorkPtr->GetId());

	AppendWork* pPostWork = pData->GetAppendWork(postWorkPtr->GetId());

	if(pPreWork == NULL || pPostWork == NULL)
		return false;

	SBH_BOOL rtFlg = ObvMaxInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
		, minInterval, maxInterval, pPreWork, pPostWork);

	if(rtFlg && m_checkflg == NOTDISOBEY)
	{
		return ObvMinInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
			, minInterval, maxInterval, pPreWork, pPostWork);
	}
	else
	{
		return rtFlg;
	}

}

//逆向单资源组合最大间隔求解
SBH_BOOL CMaxIntervalDispose::CalcSingleLoopConv( WorkPtr& preWorkPtr ,WorkPtr& postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo)
{
	//求取最大最小间隔
	int minInterval = AMAPI::GetMinMaxInterval(preWorkPtr,postWorkPtr,MININTELVAL,preWorkResInfo.resRelaID,postWorkResInfo.resRelaID);
	int maxInterval = AMAPI::GetMinMaxInterval(preWorkPtr,postWorkPtr,MAXINTELVAL,preWorkResInfo.resRelaID,postWorkResInfo.resRelaID);

	AppendWork* pPreWork = pData->GetAppendWork(preWorkPtr->GetId());

	AppendWork* pPostWork = pData->GetAppendWork(postWorkPtr->GetId());

	if(pPreWork == NULL || pPostWork == NULL)
		return false;

	SBH_BOOL rtFlg = ConvMaxInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
		, minInterval, maxInterval, pPreWork, pPostWork);

	if(rtFlg && m_checkflg == NOTDISOBEY)
	{
		return ConvMinInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
			, minInterval, maxInterval, pPreWork, pPostWork);
	}
	else
	{
		return rtFlg;
	}
}

//获取前工作计算时间点
time_t CMaxIntervalDispose::GetPreWorkPdEndTm(SBH_LONG workId,SBH_LONG resId)
{
	if(resId != 0) 
		return m_preWorkPlan.GetPdEndTime(workId,resId);
	else 
		return m_preWorkPlan.GetPdEndTime(workId);
}

//获取后工作计算时间点
time_t CMaxIntervalDispose::GetPreWorkPdStTm(SBH_LONG workId,SBH_LONG resId)
{
	if(resId != 0) 
		return m_preWorkPlan.GetPdStTime(workId,resId);
	else
		return m_preWorkPlan.GetPdStTime(workId);
}

//获取前工作计算时间点
time_t CMaxIntervalDispose::GetPostWorkPdEndTm(SBH_LONG workId,SBH_LONG resId)
{
	if(resId != 0) 
		return m_postWorkPlan.GetPdEndTime(workId,resId);
	else 
		return m_postWorkPlan.GetPdEndTime(workId);
}

//获取后工作计算时间点
time_t CMaxIntervalDispose::GetPostWorkPdStTm(SBH_LONG workId,SBH_LONG resId)
{
	if(resId != 0) 
		return m_postWorkPlan.GetPdStTime(workId,resId);
	else
		return m_postWorkPlan.GetPdStTime(workId);
}



SBH_BOOL CMaxIntervalDispose::ObvMaxInterValCheck(WorkPtr&  preWorkPtr ,WorkPtr&  postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo
												  ,SBH_INT minInterval,SBH_INT maxInterval,AppendWork* pPreWork,AppendWork* pPostWork)
{
	//获取前工作制造结束时刻
//	time_t preWorkTmPoint = GetPreWorkTmPoint(preWorkPtr->GetId(),GetResId(preWorkResInfo));
//	time_t postWorkTmPoint = GetPostWorkTmPoint(postWorkPtr->GetId(),GetResId(postWorkResInfo));
	InitWorkTm( preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo);

	//违反最大间隔时间
	SBH_INT maxValTm(0);

	//申明正向计算
	CSingleObvCalcV3* pCalc = CSingleObvCalcV3::Get();

	//判断是否存在最大间隔,作约束检验
	if(m_workBoundForMaxVal.CheckMinMaxIntval(preWorkPtr,postWorkPtr,false,maxValTm))//违反
	{
		//发现违反
		m_checkflg = RESOLVEPROB;

		//正向最大间隔，切换资源后第一次进最大间隔检验，直接进最小间隔
		if(m_changeRes && pPostWork->GetSchLevel() >= SL_Level3)
		{
			return ObvMinInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
				, minInterval, maxInterval, pPreWork, pPostWork);
		}

		//判断前工作状态
		if( pPreWork->GetSchLevel() < SL_Level3)
		{
			return false;
		}

		m_changeRes = false;

		//初始化制造时间
		//pData->schTools.m_lastSucessTestSchInfo.Init();
		pData->schTools.Init();
		pCalc->GetPrdTm(preWorkPtr,preWorkResInfo,pData->schTools.m_prdTimeLen);

		//更新前工作左边界，排程
		//边界起始点为后工作开始时刻-最大间隔
		//前工作左边界
//		SBH_TIME newLeftLimit = CalcLimitStTm(GetWorkPdStTm(preWorkPtr,GetResId(preWorkResInfo)) + maxValTm,preWorkResInfo,Dir_Right);
		//前工作左边界
//		SBH_TIME newLeftLimit = pCalc->GetNewLimit(limitStTm,preWorkResInfo,pData->schTools.m_prdTimeLen,Dir_Left);

		SBH_TIME newLeftLimit = GetPreWorkPdStTm(preWorkPtr->GetId(),GetResId(preWorkResInfo)) + maxValTm;
		//取消前工作计划
		m_preWorkPlan.ClearPlan();

		if(pCalc->Schedule(preWorkPtr,newLeftLimit,preWorkResInfo,TYPE_PRODUCETM))
		{
			pData->schTools.MeragePlan();
			//排程成功
			m_preWorkPlan = pData->schTools.m_lastSucessTestSchInfo;
			//pData->schTools.m_lastSucessTestSchInfo.Init();
			pData->schTools.Init();

			return ObvMinInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
				, minInterval, maxInterval, pPreWork, pPostWork);
		}
		//排程失败，返回
		return false;

	}

	m_changeRes = false;
	return true;

}

SBH_BOOL CMaxIntervalDispose::ObvMinInterValCheck(WorkPtr&  preWorkPtr ,WorkPtr&  postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo
												  ,SBH_INT minInterval,SBH_INT maxInterval,AppendWork* pPreWork,AppendWork* pPostWork)
{
	//获取前工作制造结束时刻
//	time_t preWorkTmPoint = GetPreWorkTmPoint(preWorkPtr->GetId(),GetResId(preWorkResInfo));
//	time_t postWorkTmPoint = GetPostWorkTmPoint(postWorkPtr->GetId(),GetResId(postWorkResInfo));
	InitWorkTm( preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo);

	//申明正向计算
	CSingleObvCalcV3* pCalc = CSingleObvCalcV3::Get();

	SBH_INT minValTm(0);

	//判断是否存在最小间隔,作约束检验
	if(m_workBoundForMaxVal.CheckMinMaxIntval(preWorkPtr,postWorkPtr,true,minValTm) || m_changeRes)//违反
	{
		m_checkflg = RESOLVEPROB;

		//判断前工作状态
		if(pPostWork->GetSchLevel() < SL_Level3)
		{
			return false;
		}

		//更新后工作左边界，排程
		SBH_TIME newLeftLimit = GetPostWorkPdStTm(postWorkPtr->GetId(),GetResId(postWorkResInfo)) + minValTm;

		//交换资源后第一此最小间隔检验
		if(	m_changeRes )
		{
			m_changeRes = false;
			//newLeftLimit = GetPostWorkPdStTm(postWorkPtr,GetResId(postWorkResInfo)) + minValTm;
		}

		//取消后工作计划
		m_postWorkPlan.ClearPlan();

		//初始化制造时间
		//pData->schTools.m_lastSucessTestSchInfo.Init();
		pData->schTools.Init();
		pCalc->GetPrdTm(postWorkPtr,postWorkResInfo,pData->schTools.m_prdTimeLen);
		if(pCalc->Schedule(postWorkPtr,newLeftLimit,postWorkResInfo,TYPE_PRODUCETM))
		{
			pData->schTools.MeragePlan();
			//排程成功
			m_postWorkPlan = pData->schTools.m_lastSucessTestSchInfo;
			//pData->schTools.m_lastSucessTestSchInfo.Init();
			pData->schTools.Init();

			return ObvMaxInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
				, minInterval, maxInterval, pPreWork, pPostWork);
		}
		//排程失败，返回
		return false;

	}

	return true;
}

SBH_BOOL CMaxIntervalDispose::ConvMaxInterValCheck( WorkPtr&  preWorkPtr ,WorkPtr&  postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo 
												   ,SBH_INT minInterval,SBH_INT maxInterval,AppendWork* pPreWork,AppendWork* pPostWork )
{
	//获取前工作制造结束时刻
//	time_t preWorkTmPoint = GetPreWorkTmPoint(preWorkPtr->GetId(),GetResId(preWorkResInfo));
//	time_t postWorkTmPoint = GetPostWorkTmPoint(postWorkPtr->GetId(),GetResId(postWorkResInfo));
	InitWorkTm( preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo);

	//申明正向计算
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();

	SBH_INT maxValTm(0);
	//判断是否存在最大间隔,作约束检验
	if(m_workBoundForMaxVal.CheckMinMaxIntval(preWorkPtr,postWorkPtr,false,maxValTm))//违反
	{
		//发现违反
		m_checkflg = RESOLVEPROB;

		//正向最大间隔，切换资源后第一次进最大间隔检验，直接进最小间隔
		if(m_changeRes && pPreWork->GetSchLevel() >= SL_Level3)
		{
			return ConvMinInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
				, minInterval, maxInterval, pPreWork, pPostWork);
		}

		//判断后工作状态
		if(pPostWork->GetSchLevel() < SL_Level3)
		{
			return false;
		}

		m_changeRes = false;

		SBH_TIME newRightLimit = GetPostWorkPdEndTm(postWorkPtr->GetId(),GetResId(postWorkResInfo)) - maxValTm;

		//取消后工作计划
		m_postWorkPlan.ClearPlan();

		//初始化制造时间
		//pData->schTools.m_lastSucessTestSchInfo.Init();
		pData->schTools.Init();
		pCalc->GetPrdTm(postWorkPtr,postWorkResInfo,pData->schTools.m_prdTimeLen);

		//更新后工作右边界，排程
//		SBH_TIME limitStTm = CalcLimitStTm(preWorkTmPoint + maxInterval,postWorkResInfo,Dir_Left);
//		SBH_TIME newRightLimit = pCalc->GetNewLimit(limitStTm,postWorkResInfo,pData->schTools.m_prdTimeLen,Dir_Right);

		if(pCalc->Schedule(postWorkPtr,newRightLimit,postWorkResInfo,TYPE_PRODUCETM))
		{
			pData->schTools.MeragePlan();
			//排程成功
			m_postWorkPlan = pData->schTools.m_lastSucessTestSchInfo;
			//pData->schTools.m_lastSucessTestSchInfo.Init();
			pData->schTools.Init();

			return ConvMinInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
				, minInterval, maxInterval, pPreWork, pPostWork);
		}
		//排程失败，返回
		return false;

	}

	m_changeRes = false;
	return true;
}

SBH_BOOL CMaxIntervalDispose::ConvMinInterValCheck( WorkPtr&  preWorkPtr ,WorkPtr&  postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo 
												   ,SBH_INT minInterval,SBH_INT maxInterval,AppendWork* pPreWork,AppendWork* pPostWork )
{
	//获取前工作制造结束时刻
//	time_t preWorkTmPoint = GetPreWorkTmPoint(preWorkPtr->GetId(),GetResId(preWorkResInfo));
//	time_t postWorkTmPoint = GetPostWorkTmPoint(postWorkPtr->GetId(),GetResId(postWorkResInfo));
	InitWorkTm( preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo);

	//申明正向计算
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();

	SBH_INT minValTm(0);
	//判断是否存在最小间隔,作约束检验
	if(m_workBoundForMaxVal.CheckMinMaxIntval(preWorkPtr,postWorkPtr,true,minValTm)|| m_changeRes)//违反
	{
		//发现违反
		m_checkflg = RESOLVEPROB;

		//判断前工作状态
		if(pPreWork->GetSchLevel() < SL_Level3)
		{
			return false;
		}

		//更新前工作右边界，排程
		//SBH_TIME newRightLimit = m_postWorkPlan.GetPdStTime(postWorkPtr->GetId(),postWorkResInfo.resRelaID->GetResId()) - minInterval ;
		SBH_TIME newRightLimit = GetPreWorkPdEndTm(preWorkPtr->GetId(),GetResId(preWorkResInfo)) - minValTm;

		//交换资源后第一次最小间隔检验
		if(	m_changeRes )
		{
			m_changeRes = false;
			//newRightLimit = postWorkTmPoint - minInterval;
		}

		//取消前工作计划
		m_preWorkPlan.ClearPlan();

		//初始化制造时间
		//pData->schTools.m_lastSucessTestSchInfo.Init();
		pData->schTools.Init();
		pCalc->GetPrdTm(preWorkPtr,preWorkResInfo,pData->schTools.m_prdTimeLen);
		if(pCalc->Schedule(preWorkPtr,newRightLimit,preWorkResInfo,TYPE_PRODUCETM))
		{
			pData->schTools.MeragePlan();
			//排程成功
			m_preWorkPlan = pData->schTools.m_lastSucessTestSchInfo;
			//pData->schTools.m_lastSucessTestSchInfo.Init();
			pData->schTools.Init();

			return ConvMaxInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
				, minInterval, maxInterval, pPreWork, pPostWork);
		}
		//排程失败，返回
		return false;

	}

	return true;
}

//SBH_TIME CMaxIntervalDispose::GetNewLimit(const SBH_TIME& startTm,WorkResInfo& workResInfo,const TimeLenInfo& pdLen, SBH_INT dir)
//{
//	UnionFindMode findInfo;
//
//	//写资源量(主、副资源)
//	findInfo.m_resCaps[workResInfo.resRelaID->GetResId()] = 0;
//	//副资源
//	for(list<WorkResRelaPtr>::iterator iter = workResInfo.assistResRelaList.begin();
//		iter != workResInfo.assistResRelaList.end();iter++)
//	{
//		findInfo.m_resCaps[(*iter)->GetResId()] = 0;
//	}
//
//	findInfo.m_time = startTm;
//
//	findInfo.m_len = pdLen.Max();//标准加工时间
//	findInfo.m_interrupt = 0;//最大中断，-1：不中断，0：任意中断
//	findInfo.m_bLock = false;//是否为锁定
//	findInfo.m_bStrideWork = false;//是否可以跨工作
//	findInfo.m_bOverTime = false;//是否可以加班
//	findInfo.m_bInfinity = true;//是否无限能力排程
//	findInfo.m_bIgnoreFurnaceRes = true;//是否忽略熔炉资源
//	findInfo.m_nType = TYPE_PRODUCETM;//类型 TYPE_PRODUCETM ....
//
//	if(dir == Dir_Left)	//左工作，求左时间点逆向
//	{
//		if(pData->mgRemainCapacityDao.UnionFindIdleF2(findInfo,Dir_Converse))
//			return max(findInfo.m_outStartTime,AMAPI::GetSchStartTime());
//	}
//	else
//	{
//		if(pData->mgRemainCapacityDao.UnionFindIdleF2(findInfo,Dir_Obverse))
//			return min(findInfo.m_outEndTime,AMAPI::GetSchEndTime());
//	}
//
//	return startTm;
//}

void CMaxIntervalDispose::Init()
{
	m_preWorkPlan.Init();
	m_postWorkPlan.Init();
	m_workBoundForMaxVal.Clear();

	m_checkflg = NOTDISOBEY;
	m_changeRes = false;
}

SBH_BOOL CMaxIntervalDispose::MaxInterValCheck( SBH_INT preWorkId,SBH_INT postWorkId )
{
	//首先清理类变量
	//Init();

	//获取前后工作智能指针
	WorkPtr&  preWorkPtr = AMAPI::GetWorkPtrMap()->Get(preWorkId);
	WorkPtr&  postWorkPtr = AMAPI::GetWorkPtrMap()->Get(postWorkId);

	//有工作不存在。。。不输出违反信息
	if(!(preWorkPtr != NULL && postWorkPtr != NULL))
		return true;

	//WorkResRelaPtr preMainRes = NULL,postMainRes = NULL;
	//list<WorkResRelaPtr> preAssRes,postAssRes;

	////取得原计划的资源
	//AMAPI::GetPlanResRelaCom(preWorkPtr,preMainRes,preAssRes);
	//AMAPI::GetPlanResRelaCom(postWorkPtr,postMainRes,postAssRes);

	////求取最大最小间隔
	////	int minInterval = AMAPI::GetMinMaxInterval(preWorkPtr,postWorkPtr,MININTELVAL,preWorkResInfo.resRelaID,postWorkResInfo.resRelaID);
	//int maxInterval = AMAPI::GetMinMaxInterval(preWorkPtr,postWorkPtr,MAXINTELVAL,preMainRes,postMainRes);

	//time_t postWorkTmPoint(0),preWorkTmPoint(0);

	CWorkBoundV3 workBoundV3;
	SBH_INT balance;

	//判断是否存在最大间隔,作约束检验
	//if(maxInterval > 0 && postWorkTmPoint - preWorkTmPoint > maxInterval)//违反
	if(workBoundV3.CheckMinMaxIntval(preWorkPtr,postWorkPtr,false,balance))
	{
		return true;
	}

	return false;
}

//计算边界开始时刻
SBH_TIME CMaxIntervalDispose::CalcLimitStTm( const SBH_TIME& startTm ,WorkResInfo& workResInfo, SBH_INT dir)
{
	SBH_TIME rtTm(0);
	if(dir == Dir_Left)
	{
		SBH_TIME stTm = pData->mgRemainCapacityDao.GetOnDutyTime(workResInfo,startTm);
		if(stTm > startTm)
		{

			UnionFindMode findInfo;

			//写资源量(主、副资源)
			findInfo.m_resCaps[workResInfo.resRelaID->GetResId()] = 0;
			//副资源
			for(list<WorkResRelaPtr>::iterator iter = workResInfo.assistResRelaList.begin();
				iter != workResInfo.assistResRelaList.end();iter++)
			{
				findInfo.m_resCaps[(*iter)->GetResId()] = 0;
			}

			rtTm = pData->mgRemainCapacityDao.GetOnPreDutyTime(findInfo,startTm) - 1;
		}
		else
		{
			//判断是否为下班开始
			if(pData->mgRemainCapacityDao.IfOffDutyTimePoint(workResInfo.resRelaID->GetResId(),startTm+1) /*> (startTm+1)*/)
			{
				rtTm = startTm - 1;
			}
			else
				rtTm = stTm;//开始时间点
		}
	}
	else
	{
		UnionFindMode findInfo;

		//写资源量(主、副资源)
		findInfo.m_resCaps[workResInfo.resRelaID->GetResId()] = 0;
		//副资源
		for(list<WorkResRelaPtr>::iterator iter = workResInfo.assistResRelaList.begin();
			iter != workResInfo.assistResRelaList.end();iter++)
		{
			findInfo.m_resCaps[(*iter)->GetResId()] = 0;
		}

		SBH_TIME stTm = pData->mgRemainCapacityDao.GetOnPreDutyTime(findInfo,startTm);
		if(stTm < startTm)
		{
			rtTm = pData->mgRemainCapacityDao.GetOnDutyTime(workResInfo,startTm) + 1;

		}
		else
		{
			//判断是否为上班开始
			if(pData->mgRemainCapacityDao.GetOnPreDutyTime(findInfo,startTm-1) < (startTm-1))
			{
				rtTm = startTm + 1;//开始时间点
			}
			else
				rtTm = startTm ;//开始时间点
		}
	}

	return rtTm;
}

void CMaxIntervalDispose::InitWorkTm(WorkPtr&  preWorkPtr ,WorkPtr&  postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo)
{
	m_workBoundForMaxVal.InsertWorkTm(preWorkPtr->GetId(),STTMFORMAXVAL,GetPreWorkPdStTm(preWorkPtr->GetId(),GetResId(preWorkResInfo)));
	m_workBoundForMaxVal.InsertWorkTm(preWorkPtr->GetId(),ENDTMFORMAXVAL,GetPreWorkPdEndTm(preWorkPtr->GetId(),GetResId(preWorkResInfo)));
	m_workBoundForMaxVal.InsertWorkTm(postWorkPtr->GetId(),STTMFORMAXVAL,GetPostWorkPdStTm(postWorkPtr->GetId(),GetResId(postWorkResInfo)));
	m_workBoundForMaxVal.InsertWorkTm(postWorkPtr->GetId(),ENDTMFORMAXVAL,GetPostWorkPdEndTm(postWorkPtr->GetId(),GetResId(postWorkResInfo)));
}
