#include "stdafx.h"
#include "Scheduler.h"

#include "SingleObvCalc.h"
#include "SingleConvCalc.h"
#include "PriorityMgr.h"
#include "DisobeyRestrict.h"
#include "SingleConvCalcV3.h"
#include "WorkBoundV3.h"

#ifdef RELEASETESTOUTPUT
#include "clock.h"
#include "Tools.h"
#endif

#ifdef _DEBUG_USING_CLOCK
#include "clock.h"
#define CLOCK_MARK(timer) timer.Pause();
#else
#define CLOCK_MARK(timer) 
#endif

#define WORKSCHEMODE_IMMOVABILITY		1		//计划不动
#define WORKSCHEMODE_TIMEFIX			2		//固定工作处理
#define WORKSCHEMODE_COMMONSCHE			3		//普通排程
#define	WORKSCHEMODE_NONE				4		//不处理
#define WORKSCHEMODE_DISOBEY_RESTRICT	5		//违反约束处理


CScheduler::CScheduler(void)
{
}

CScheduler::~CScheduler(void)
{
}

SBH_BOOL CScheduler::Scheduling( SBH_INT workId )
{
	if(workId <= 0)
		return false;

#ifdef RELEASETESTOUTPUT
	TimeLog::WriteLog(TOOLS::IToString(workId,10).c_str());
#endif
	SBH_BOOL rtFlg = false;

	AppendWork* pComWork = pData->GetAppendWork(workId);
	if(pComWork != NULL)
	{
		//建立优先关系
		//CPriorityMgr priMgr;
		//priMgr.CreatePriority(workId);
		pData->schTools.SetCurAppWork(pComWork);
		//判断工作是否有计划
		switch(GetWorkScheMode(workId,pComWork))
		{
		case WORKSCHEMODE_IMMOVABILITY:
			{
				if(!pComWork->GetISSchedFlg())
					ImmovableWorkScheduling(workId);
				pComWork->SetISSucessedFlg(true);
				pComWork->SetISSchedFlg(true);
				break;
			}
		case WORKSCHEMODE_TIMEFIX:
			{
				if(!pComWork->GetISSchedFlg())
					TimefixSchdule(workId,pComWork->GetDirection());
				pComWork->SetISSucessedFlg(true);
				pComWork->SetISSchedFlg(true);
				break;
			}
		case WORKSCHEMODE_COMMONSCHE:
			{
				//如果是已排,删除计划,修改方向
				bool changeDir = false;
				DirType dt;
				if (pComWork->GetISSchedFlg())
				{
					changeDir = true;
					dt = pComWork->GetDirection();
					pData->RetractWorkPlan(workId);
					pComWork->SetISSchedFlg(false);
					pComWork->SetDirection(pComWork->GetDirection() == Dir_Obverse ? Dir_Converse :
						Dir_Obverse);
					pComWork->ResetTm();
				}
				CommonSchdule(workId,pComWork->GetDirection());
				if (changeDir)
				{
					pComWork->SetDirection(dt);
				}
				rtFlg = true;
				break;
			}
		case WORKSCHEMODE_DISOBEY_RESTRICT:
			{
				DisobeyRestrictSch(workId,pComWork->GetDirection());
				break;
			}
		}
		pComWork->SetISSchedFlg(true);
		pData->schTools.SetCurAppWork(NULL);
	}

	return rtFlg ;
}

//取工作计划开始，结束时刻及资源
bool PrepareResCap(const list<WorkSchPlanPtr>& planList,map<SBH_ULONG,SBH_DOUBLE>& resCap,time_t& stTm,time_t& endTm,int type)
{
	 stTm = MAXINT64;
	 endTm = 0;
	 resCap.clear();
	for(list<WorkSchPlanPtr>::const_iterator iter = planList.begin();
		iter != planList.end();iter++)
	{
		const WorkSchPlanPtr& workPlan = *iter;
		if(type == workPlan->GetType())
		{
			resCap.insert(make_pair(workPlan->GetPlanSelResId(),(SBH_DOUBLE)workPlan->GetUsedQuantity()));
			stTm = min(stTm,workPlan->GetStartTime());
			endTm = max(endTm,workPlan->GetEndTime());
		}
	}

	if(resCap.empty())
		return false;

	return true;
}

#include "SingleObvShiftCalc.h"

int CScheduler::PlanUse(SBH_INT workId,list<WorkSchPlanPtr> planList,SBH_INT type,bool bShiftCal )
{
	time_t stTm(MAXINT64),endTm(0);
	map<SBH_ULONG,SBH_DOUBLE> resIds;

	if(PrepareResCap( planList, resIds, stTm, endTm, type))
	{
		UnionFindMode findMode;
		findMode.m_outStartTime = stTm;
		findMode.m_outEndTime = endTm;
		findMode.m_resCaps = resIds;
		findMode.m_nType = type;
		if(type == TYPE_LOCKTM)
		{
			findMode.m_bLock = true;
		}
		if(pData->mgRemainCapacityDao.CalCalendarRange(findMode))
		{
			//放入新计划
			if (!bShiftCal)
			{
				TestSchedulerInfo scheInfo;
				scheInfo.InsertAddPlan(workId,type,findMode.m_timeRanges);
				scheInfo.SurePlan();
				pData->mgRemainCapacityDao.UnionUse(workId,type,findMode.m_timeRanges);
				return 0;
			}
			else
			{
				int pdLen = 0;
				for(map<SBH_ULONG,TimeRangeList>::iterator i_resRange = findMode.m_timeRanges.begin();
					i_resRange != findMode.m_timeRanges.end();i_resRange++)
				{
					for (list<TimeRange>::iterator i_time = i_resRange->second.begin();
						i_time != i_resRange->second.end();i_time++)
					{
						pdLen += (int)(i_time->m_endTime - i_time->m_startTime);
					}
					if (pdLen > 0)
						break;
				}
				return pdLen;
			}
		}
		else
		{
			TestSchedulerInfo scheInfo;
			for(map<SBH_ULONG,SBH_DOUBLE>::iterator iter = findMode.m_resCaps.begin();iter != findMode.m_resCaps.end();++iter)
			{
				findMode.m_timeRanges[iter->first].push_back(TimeRange(findMode.m_outStartTime,findMode.m_outEndTime,iter->second));
			}
			scheInfo.InsertAddPlan(workId,type,findMode.m_timeRanges);
			scheInfo.SurePlan();
		}
	}//if(PrepareResCap
	return 0;
}

void CScheduler::CalcInterruptedTime( SBH_INT workId )
{
	WorkPtr& workPtr = AMAPI::GetWorkPtrMap()->Get(workId);
	if(workPtr == NULL)	return;

	//获取工作计划列表	
	list<WorkSchPlanPtr> planList;
	GetWorkPlanList(planList,workPtr);

	//清除原计划
	AMAPI::ClearWorkPlan(workId);

	list<int> types;
	types.push_back(TYPE_PRESET);
	types.push_back(TYPE_PRODUCETM);
	types.push_back(TYPE_LOCKTM);
	types.push_back(TYPE_POSTSET);	
	
	for(list<int>::iterator iter = types.begin();
		iter != types.end();iter++)
	{
		if(AMAPI::GetParameter()->GetSystemSchFlg() == Refined_Sch)
		{			
			PlanUse(workId,planList,*iter);
		}
		else if(*iter == TYPE_PRODUCETM)	//按日排程,仅排制造
		{
			WorkResInfo workResInfo;

			//遍历计划，取使用过的资源
			time_t stTm(MAXINT64),endTm(0);
			set<int> planResIdList;
			for(list<WorkSchPlanPtr>::iterator iter1 = planList.begin();
				iter1 != planList.end();iter1++)
			{
				WorkSchPlanPtr& planPtr = *iter1;
				//判断是否为
				planResIdList.insert(planPtr->GetPlanSelResId());
				stTm = min(stTm,ScheduleHelper::GetGlancingStartTime(planPtr->GetStartTime(),planPtr->GetPlanSelResId()));
				endTm = max(endTm,ScheduleHelper::GetGlancingStartTime(planPtr->GetEndTime(),planPtr->GetPlanSelResId()));
			}

			//工作跨天
			if(stTm != endTm)
			{
				PlanUse(workId,planList,*iter);
				break;
			}

			//获取资源
			workPtr->MatchingMainAssRes(planResIdList,workResInfo.resRelaID,workResInfo.assistResRelaList);

			if(workResInfo.resRelaID == NULL || workResInfo.resRelaID->GetRes() == NULL)
				break;

			SingleCalcV3* pCalc = NULL;

			pCalc = CSingleObvShiftCalc::Get();

			//pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen,0);

			
			int prdTm = PlanUse(workId,planList,*iter,true);//AMAPI::GetWorkPlanLength(planList,0,MAXINT64);

			pData->schTools.m_prdTimeLen.tmPairList.clear();
			pData->schTools.m_prdTimeLen.tmPairList.insert(make_pair(workResInfo.resRelaID,prdTm ));
			for(list<WorkResRelaPtr>::iterator iterAss = workResInfo.assistResRelaList.begin();
				iterAss != workResInfo.assistResRelaList.end();iterAss++)
			{
				pData->schTools.m_prdTimeLen.tmPairList.insert(make_pair(*iterAss,prdTm ));
			}

			if(!pCalc->Schedule(workPtr,stTm,workResInfo,TYPE_PRODUCETM))
			{
				pData->schTools.m_lastSucessTestSchInfo.ClearPlan();
				return;
			}
			else
			{
				//跨天
				if(ScheduleHelper::GetGlancingStartTime(pData->schTools.m_lastSucessTestSchInfo.GetPdEndTime(workId),workResInfo.resRelaID->GetResId()) != stTm)
				{
					pData->schTools.m_lastSucessTestSchInfo.ClearPlan();
					//从这天结束逆向排
					TestSchedulerInfo tsInfo;
					SBH_TIME rtStTm,rtEndTm; 
					endTm = ScheduleHelper::GetGlancingEndTime(stTm,workResInfo.resRelaID->GetResId());
					//无限能力逆排
					ScheduleMethod temp = pData->parame.m_curSchMethod;
					pData->parame.m_curSchMethod = SM_INFINITY;	//无限能力
					if(pCalc->CalWorkTime(workPtr,workResInfo,TYPE_PRODUCETM,endTm
						,pData->schTools.m_prdTimeLen,tsInfo,rtStTm,rtEndTm,FUC_F1,Dir_Converse))
					{
						if(ScheduleHelper::GetGlancingStartTime(rtStTm,workResInfo.resRelaID->GetResId()) != stTm)
						{
							//待排工作超过一天，无限能力正排
							tsInfo.ClearPlan();
							

							pCalc->CalWorkTime(workPtr,workResInfo,TYPE_PRODUCETM,stTm
								,pData->schTools.m_prdTimeLen,tsInfo,rtStTm,rtEndTm,FUC_F1,Dir_Obverse);

						}

						pData->schTools.m_lastSucessTestSchInfo = tsInfo;
					}

					pData->parame.m_curSchMethod = temp;
				}

				pData->schTools.m_lastSucessTestSchInfo.SurePlan();

			}

		}
	}//for

}

void CScheduler::ImmovableWorkScheduling( SBH_INT workId )
{
	//依据计划，在现有日历基础上分段(中断)
	CalcInterruptedTime(workId);

	map<SBH_ULONG,TimeRangeList> prePlanList;	//前设置
	map<SBH_ULONG,TimeRangeList> prdPlanList;	//制造
	map<SBH_ULONG,TimeRangeList> postPlanList;	//后设置
	map<SBH_ULONG,TimeRangeList> lockPlanList;	//锁定

	pData->GetWorkPlan(workId,prePlanList,prdPlanList,postPlanList,lockPlanList);

	//if(!prdPlanList.empty())
	//{
	//	pData->mgRemainCapacityDao.UnionUse(workId,TYPE_PRODUCETM,prdPlanList);
	//}
	//if(!postPlanList.empty())
	//{
	//	pData->mgRemainCapacityDao.UnionUse(workId,TYPE_POSTSET,postPlanList);
	//}
	//if(!lockPlanList.empty())
	//{
	//	pData->mgRemainCapacityDao.UnionUse(workId,TYPE_LOCKTM,lockPlanList);
	//}
	////插入计划
	//if(!prePlanList.empty())
	//{
	//	pData->mgRemainCapacityDao.UnionUse(workId,TYPE_PRESET,prePlanList);
	//}

	//最大间隔相关工作需要将计划和资源放入最大间隔检验类
	if(pData->workFilter.ExistInMaxIntervalGp(workId)/*g_MaxValRegister.BExist(workId)*/)
	{
		//求取工作
		WorkPtr&  workPtr = AMAPI::GetWorkPtrMap()->Get(workId);
		//求取资源组合
		WorkResInfo workResInfo;
		AMAPI::GetPlanResRelaCom(workPtr,workResInfo.resRelaID,workResInfo.assistResRelaList);
		list<WorkResInfo> workResInfoList;
		workResInfoList.push_back(workResInfo);
		//求取工作计划
		TestSchedulerInfo testSchedulerInfo;
		testSchedulerInfo.InsertAddPlan(workId,TYPE_PRODUCETM,prdPlanList);
		testSchedulerInfo.InsertAddPlan(workId,TYPE_PRESET,prePlanList);
		testSchedulerInfo.InsertAddPlan(workId,TYPE_POSTSET,postPlanList);
		testSchedulerInfo.InsertAddPlan(workId,TYPE_LOCKTM,lockPlanList);

		vector<TestSchedulerInfo> workPlanList;
		workPlanList.push_back(testSchedulerInfo);

		g_MaxValRegister.InsertWorkInfo(workId,workResInfoList,workPlanList);
	}
}

SBH_INT CScheduler::GetWorkScheMode( SBH_INT workId ,AppendWork* pAppWork)
{
//	WorkPtr&  workPtr = AMAPI::GetWorkPtrMap()->Get(workId);
	if(pAppWork == NULL)
		return WORKSCHEMODE_NONE;

	if(pAppWork->GetSchLevel() == SL_Level0)
		return WORKSCHEMODE_IMMOVABILITY;

	if(pAppWork->GetSchLevel() == SL_Level1)
	{
		if (pAppWork->GetWorkPtr() != NULL && pAppWork->GetWorkPtr()->IsNonSchWorkB())
			return WORKSCHEMODE_TIMEFIX;
		return WORKSCHEMODE_IMMOVABILITY;
	}

	if(pAppWork->GetSchLevel() == SL_Level2)
	{
		return WORKSCHEMODE_TIMEFIX;
	}

	//已排并且未成功排程/违反约束处理
	if (pAppWork != NULL && pAppWork->GetISSchedFlg()
		&& !pAppWork->GetISSucessedFlg())
		return WORKSCHEMODE_DISOBEY_RESTRICT;

	return WORKSCHEMODE_COMMONSCHE;
}

void CScheduler::DisobeyRestrictSch( SBH_INT workId,DirType dt)
{
	if (pData->parame.m_curDisRestrcitType != DRT_NO)
	{
		//删除虚拟计划
		pData->RetractWorkPlan(workId);

		AppendWork* pComWork = pData->GetAppendWork(workId);

		//不分派
		if (pData->parame.m_curSchMethod == SM_UNDISPATCH)
		{
			pData->parame.m_curSchMethod = SM_LIMITARY;
			pData->parame.m_curDisRestrcitType = DRT_NO;

			//modify by 喻鹏 2010.8.10
			//需要添加参与排程条件
			if(pComWork != NULL)
				pComWork->SetISSucessedFlg(true);
			return ;
		}
		else
		{
			CommonSchdule(workId,dt);
		}
		
	}
	pData->parame.m_curSchMethod = SM_LIMITARY;
	pData->parame.m_curDisRestrcitType = DRT_NO;
}

SBH_BOOL CScheduler::DelWithDisobeyRestict_LV1( SBH_INT workId )
{
	if (AMAPI::GetOffendRestMode() >= 4) return false;
	if (workId <= 0) return false;
	AppendWork* pComWork = pData->GetAppendWork(workId);
	//modify by 喻鹏 2010.8.10
	//需要添加参与排程条件
	bool flg = false;
	if(pComWork != NULL 
		&& ( pComWork->GetSchLevel() == SL_Level3 || pComWork->GetSchLevel() == SL_Level4 )
		&& pComWork->GetISSchedFlg()
		&& !pComWork->GetISSucessedFlg()
		&& AMAPI::IsParticipateSchWork( workId))
	{
		//检查违反约束并获取违反约束处理方式
		CDisobeyRestrict disobey;
		pData->parame.m_curDisRestrcitType = disobey.GetDisobeyRestrictType(pComWork,workId,false);
		pData->parame.m_curSchMethod = disobey.GetDisobeyRestrictWay(
			pData->parame.m_curDisRestrcitType);
		if (pData->parame.m_curDisRestrcitType != DRT_NO)
		{
			if (pData->parame.m_curSchMethod != SM_CHANGEDIRECTION)
			{
				//备份方向
				DirType saveDirType = pComWork->GetDirection();

				SBH_SHORT curState = pData->parame.m_curSchMethod;

				disobey.DirectionMgr(pComWork,pData->parame.m_curDisRestrcitType
					,pData->parame.m_curSchMethod);


				//违反约束排程
				Scheduling(workId);
				//如果排程依旧失败
				//disobey.RecordeDisobyRestrict(pData->parame.m_curDisRestrcitType,workId);		
				if(curState == SM_OVERTIME)
				{
					pComWork->GetWorkPtr().SetWorkFlg(SchFlg_Sched_OverTime);
				}
				else if(curState == SM_FORCE)					//还原方向
				{
					pComWork->GetWorkPtr().SetWorkFlg(SchFlg_Sched_Force);
					pComWork->SetDirection(saveDirType);
				}

				//放在虚拟排程
				if(!pComWork->GetISSucessedFlg())
				{
					pData->parame.m_curSchMethod = SM_VIRTUAL;
					CommonSchdule(workId,pComWork->GetDirection());
//					pComWork->GetWorkPtr().SetWorkFlg(SchFlg_Sched_Dum);
					pData->parame.m_curSchMethod = SM_LIMITARY;
				}
				pComWork->SetIsDisobeyRestrict(true);
			}
			else
			{
				disobey.DirectionMgr(pComWork,pData->parame.m_curDisRestrcitType
					,pData->parame.m_curSchMethod);
				pData->parame.m_curSchMethod = SM_LIMITARY;
			}
		}
		flg = true;
	}
	//订单切换方向
	if (CDisobeyRestrictDirctionMgr::Get()->ChangeDirection())
	{
		ChangeDirection();
	}
	return flg;
}

//清除指定区间内工作
void CScheduler::ClearPlan(int workId,SBH_TIME leftLimit,SBH_TIME rightLimit)
{
	pData->FreeWorkCap(workId);

	WorkPtr& workPtr = AMAPI::GetWorkPtrMap()->Get(workId);
	list<WorkSchPlanPtr> delWorkPlan = workPtr->ClearSchPlan(leftLimit,rightLimit);

	//清除资源能力信息表中计划
	map<SBH_ULONG,TimeRangeList> prdPlanList ; /*制造 */ 

	//取工作计划
	for(list<WorkSchPlanPtr>::iterator iter = delWorkPlan.begin();
		iter != delWorkPlan.end();iter++)
	{
		WorkSchPlanPtr& workPlan = *iter;
/*			if(workPlan->GetType() == TYPE_PRESET)
		{
			prePlanList[workPlan->GetPlanSelResId()].push_back(TimeRange(workPlan->GetStartTime(),workPlan->GetEndTime(),(SBH_DOUBLE)workPlan->GetUsedQuantity()));
		}
		else */
		if(workPlan->GetType() == TYPE_PRODUCETM)
		{
			prdPlanList[workPlan->GetPlanSelResId()].push_back(TimeRange(workPlan->GetStartTime(),workPlan->GetEndTime(),(SBH_DOUBLE)workPlan->GetUsedQuantity()));
		}
		//else if(workPlan->GetType() == TYPE_POSTSET)
		//{
		//	postPlanList[workPlan->GetPlanSelResId()].push_back(TimeRange(workPlan->GetStartTime(),workPlan->GetEndTime(),(SBH_DOUBLE)workPlan->GetUsedQuantity()));
		//}
		//else if(workPlan->GetType() == TYPE_LOCKTM)
		//{
		//	lockPlanList[workPlan->GetPlanSelResId()].push_back(TimeRange(workPlan->GetStartTime(),workPlan->GetEndTime(),(SBH_DOUBLE)workPlan->GetUsedQuantity()));
		//}
	}
	pData->mgRemainCapacityDao.UnionUse(workId,TYPE_PRODUCETM,prdPlanList);
}

void CScheduler::CutFinelyPlan( SBH_INT workId,DirType dt )
{
	//排程类型判断
	if(AMAPI::GetParameter()->GetSystemSchFlg() == Refined_Sch)
		return ;
	if (AMAPI::GetParameter()->GetIgnoreTime() <= 0)
		return;

	//求取工作制造开始及结束时刻
	SBH_TIME stTm(0),endTm(0);
	AMAPI::GetWorkPlanEnd(workId,TYPE_PRODUCETM,Dir_Left,stTm);
	AMAPI::GetWorkPlanEnd(workId,TYPE_PRODUCETM,Dir_Right,endTm);

	//判断有效性
	if(stTm == 0|| endTm == 0)
		return;

	AppendWork* pAppWork = pData->appendWorkDao.Get(workId);
	if (pAppWork == NULL) return;

	WorkResInfo workResInfo;
	AMAPI::GetPlanResRelaCom(pAppWork->GetWorkPtr(),workResInfo.resRelaID,workResInfo.assistResRelaList);
	if(workResInfo.resRelaID == NULL)
		return;
	SBH_TIME stLeft = ScheduleHelper::GetGlancingStartTime(stTm,workResInfo.resRelaID->GetResId());
	SBH_TIME endLeft = ScheduleHelper::GetGlancingStartTime(endTm,workResInfo.resRelaID->GetResId());
	if(stLeft == endLeft)
		return;

	SBH_TIME stRight = ScheduleHelper::GetGlancingEndTime(stTm,workResInfo.resRelaID->GetResId());
	SBH_TIME endRight = ScheduleHelper::GetGlancingEndTime(endTm,workResInfo.resRelaID->GetResId());

	//根据区间，求取区间内对应工作的计划时间
	int prdTmLeft = AMAPI::GetWorkPlanLength(workId,stLeft,stRight);
	int prdTmRight = AMAPI::GetWorkPlanLength(workId,endLeft,endRight);

	
	//小于可忽略时间
	if(prdTmLeft <= AMAPI::GetParameter()->GetIgnoreTime())
	{
		//右边的小于左边的
		if(prdTmRight < prdTmLeft )
		{
			//清除时间段内工作计划
			ClearPlan(workId,endLeft,endRight);
			dt == Dir_Converse ?
				pAppWork->SetWorkPlanLeftCutFlg():pAppWork->SetWorkPlanRightCutFlg();
			//工作只存在于2个区间
			AMAPI::GetWorkPlanEnd(workId,TYPE_PRODUCETM,Dir_Right,endTm);
			if(ScheduleHelper::GetGlancingStartTime(endTm-1,workResInfo.resRelaID->GetResId()) == stLeft)
				return;
		}

		//清除时间段内工作计划
		ClearPlan(workId,stLeft,stRight);
		dt == Dir_Converse ?
			pAppWork->SetWorkPlanLeftCutFlg():pAppWork->SetWorkPlanRightCutFlg();

		//工作只存在于2个区间
		AMAPI::GetWorkPlanEnd(workId,TYPE_PRODUCETM,Dir_Left,stTm);
		if(ScheduleHelper::GetGlancingStartTime(stTm,workResInfo.resRelaID->GetResId()) == endLeft)
			return;

	}

	if(prdTmRight <= AMAPI::GetParameter()->GetIgnoreTime())
	{
		//清除时间段内工作计划
		ClearPlan(workId,endLeft,endRight);
		dt == Dir_Converse ?
			pAppWork->SetWorkPlanLeftCutFlg():pAppWork->SetWorkPlanRightCutFlg();

	}
}

void CScheduler::CommonSchdule( SBH_INT workId,DirType dt )
{
	if(dt == Dir_Obverse)
	{
		SingleObvCalc singleObv;
		singleObv.SingleObvSchedule(workId, 0);
	}
	else
	{
		SingleConvCalc singleConv;
		singleConv.SingleConvSchedule(workId, 0);
	}

	//圆整排程结果
	CutFinelyPlan(workId,dt);
}

void CScheduler::TimefixSchdule( SBH_INT workId,DirType dt )
{
	//确认工作指定开始时刻
	time_t startAt = AMAPI::GetWorkLimit(workId,STARTAT);
	if(startAt > AMAPI::GetSchStartTime())
	{
		//左边界固定排程
		SingleObvCalc singleObv;
		singleObv.SingleObvSchedule(workId, startAt);
		return;
	}
	else 
	{
		time_t endAt = AMAPI::GetWorkLimit(workId,ENDAT);
		if(endAt > AMAPI::GetSchStartTime())
		{
			SingleConvCalc singleConv;
			singleConv.SingleConvSchedule(workId, endAt);
			return;
		}
	}

	//正排
	if(dt == Dir_Obverse   )
	{
		//取已排制造左边界
		time_t prdStTm(0);
		AMAPI::GetWorkPlanEnd(workId,TYPE_PRODUCETM,Dir_Left,prdStTm);

		prdStTm = max(prdStTm,AMAPI::GetSchStartTime());
		prdStTm = max(prdStTm,AMAPI::GetWorkLimit(workId,STARTAFTER));
		//左边界固定排程
		SingleObvCalc singleObv;
		singleObv.SingleObvSchedule(workId, prdStTm);
	}
	else
	{
		//取已排制造右边界
		time_t prdEndTm(LLONG_MAX);
		AMAPI::GetWorkPlanEnd(workId,TYPE_PRODUCETM,Dir_Right,prdEndTm);
		prdEndTm = min(prdEndTm,AMAPI::GetSchEndTime());
		prdEndTm = min(prdEndTm,AMAPI::GetWorkLimit(workId,ENDBEFORE));
		SingleConvCalc singleConv;
		singleConv.SingleConvSchedule(workId, prdEndTm);
	}
}

#include "SingleObvCalcV3.h"
//切换正方向
SBH_BOOL CScheduler::ChangeDirection()
{
	map<SBH_INT,AppendWork*>* pLst = CDisobeyRestrictDirctionMgr::Get()->GetChangeDirWorkList();
	if (pLst != NULL)
	{
		CWorkBoundV3 workBound;
		CSingleObvCalcV3* pCalc = CSingleObvCalcV3::Get();
		map<SBH_INT,AppendWork*> tempList;
		map<SBH_INT,AppendWork*>::iterator iter;
		for (iter = pLst->begin(); iter != pLst->end();iter++)
		{
			if (iter->second != NULL)
			{
				WorkPtr& workPtr = iter->second->GetWorkPtr();
				if (workPtr != NULL && workPtr->GetLastSequence() >= 0)
				{
					tempList[workPtr->GetLastSequence()] = iter->second;
				}
			}
		}
		pData->parame.m_curSchMethod = SM_CHANGEDIRECTION;
		map<SBH_INT,AppendWork*>::reverse_iterator iLst;
		for (iLst = tempList.rbegin(); iLst != tempList.rend();iLst++)
		{
			if (iLst->second != NULL)
			{
				iLst->second->SetDirection(Dir_Obverse);
				WorkPtr& workPtr = iLst->second->GetWorkPtr();
				if (workPtr != NULL)
				{
					pData->schTools.Init();
					WorkResInfo workResInfo;
					AMAPI::GetPlanResRelaCom(workPtr,workResInfo.resRelaID,workResInfo.assistResRelaList);
					SBH_TIME oldPlanStartTime(0);
					AMAPI::GetMinStartTime(workPtr,TYPE_PRODUCETM ,oldPlanStartTime);
					SBH_TIME leftLimitTm(0);
					SBH_TIME rightTime(LLONG_MAX);
					workBound.CalWorkBound(workPtr,workResInfo,leftLimitTm,rightTime,Dir_Obverse);
					if (iLst->second->GetISSucessedFlg())
					{
						if (leftLimitTm <= oldPlanStartTime)
							continue;
					}
					pData->RetractWorkPlan(workPtr->GetId());

					iLst->second->SetISSchedFlg(false);
					pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen);

					leftLimitTm = max(leftLimitTm,AMAPI::GetSchStartTime());

					if(pCalc->Schedule(workPtr,leftLimitTm,workResInfo,TYPE_PRODUCETM))
					{
						pData->schTools.MeragePlan();
						pData->schTools.m_lastSucessTestSchInfo.SurePlan();
					}
					else
					{
						pCalc->VurtualResScheduler(workPtr->GetId(),Dir_Obverse);
					}
					iLst->second->SetISSucessedFlg(true);
					iLst->second->SetISSchedFlg(true);
				}
				
			}
		}

		//重新计算工作边界
		for (iLst = tempList.rbegin(); iLst != tempList.rend();iLst++)
		{
			if (iLst->second != NULL)
			{
				WorkPtr& workPtr = iLst->second->GetWorkPtr();
				if (workPtr != NULL)
				{
					WorkResInfo workResInfo;
					AMAPI::GetPlanResRelaCom(workPtr,workResInfo.resRelaID,workResInfo.assistResRelaList);
					SBH_TIME leftLimitTm(0);
					SBH_TIME rightTime(LLONG_MAX);
					workBound.CalWorkBound(workPtr,workResInfo,leftLimitTm,rightTime,Dir_Obverse);
				}
			}
		}

		pData->parame.m_curSchMethod = SM_LIMITARY;
	}

	return false;
}

SBH_BOOL CScheduler::UnitWorkSchFailed( list<SBH_INT>& workList )
{
	set<SBH_INT> temp;
	for (list<SBH_INT>::iterator iWork = workList.begin();
		iWork != workList.end();iWork++)
	{
		SBH_INT workId = *iWork;
		//删除计划
		pData->RetractWorkPlan(workId);
		AppendWork* pAppWork = pData->GetAppendWork(workId);
		if(pAppWork != NULL)
		{
			pAppWork->ResetTm();
			pAppWork->SetISSchedFlg(false);
			pAppWork->SetISSucessedFlg(false);
			g_MaxValRegister.SetSchedFlg(pAppWork->id,false);
			
		}
		temp.insert(workId);
	}
	
	pData->workFilter.ReAdd(temp);
	return true;
}

void CScheduler::UpdateTimeBoundDynamic( SBH_INT workId ,SBH_BOOL bSpread)
{
	//更新工作范围

	AppendWork* pCurAppWork = pData->GetAppendWork(workId);
	if((pCurAppWork != NULL && pCurAppWork->GetISSchedFlg()))
	{
		CWorkBoundV3 workBoundV3;
		workBoundV3.UpdateTaskTime(workId);
		workBoundV3.UpdateTimeBoundV3(workId);

		AMAPI::InsertWorkBound(pCurAppWork->id,pCurAppWork->earlieststarttime,pCurAppWork->latestendtime);
	}
}

SBH_BOOL ResExist(list<WorkSchPlanPtr>& planList,int resId)
{
	for(list<WorkSchPlanPtr>::iterator iter = planList.begin();iter != planList.end();iter++)
	{
		WorkSchPlanPtr& workSchPlanPtr = *iter;
		if(workSchPlanPtr->GetPlanSelResId() == resId)
			return true;
	}

	return false;
}

//获取资源对应设置时间长度
void GetSetTmLength(const list<WorkSchPlanPtr>& planList,const WorkResInfo& workRes,int type,TimeLenInfo& timeLenInfo)
{

	for(list<WorkSchPlanPtr>::const_iterator iter = planList.begin();iter != planList.end();iter++)
	{
		const WorkSchPlanPtr& workSchPlanPtr = *iter;
		if(workSchPlanPtr->GetPlanSelResId() == workRes.resRelaID.GetResId() && workSchPlanPtr->GetType() == type)
		{
			timeLenInfo.tmPairList[workRes.resRelaID] += SBH_INT(workSchPlanPtr->GetEndTime() - workSchPlanPtr->GetStartTime());
		}

		for(list<WorkResRelaPtr>::const_iterator iterAss = workRes.assistResRelaList.begin();
			iterAss != workRes.assistResRelaList.end();++iterAss)
		{
			if(workSchPlanPtr->GetPlanSelResId() == (*iterAss).GetResId() && workSchPlanPtr->GetType() == type)
			{
				timeLenInfo.tmPairList[*iterAss] += SBH_INT(workSchPlanPtr->GetEndTime() - workSchPlanPtr->GetStartTime());
			}
		}
	}

}

void CreatWorkSchPlan(list<WorkSchPlanPtr>& planList ,WorkPtr&  workPtr,int resId,const SBH_TIME& stTm,const SBH_TIME& endTm,int type = TYPE_PRODUCETM)
{
	WorkSchPlanPtr workSchPlanPtr(new WorkSchPlan);
	workSchPlanPtr->SetWorkId(workPtr->GetId());
	workSchPlanPtr->SetStartTime(stTm);
	workSchPlanPtr->SetEndTime(endTm);
	workSchPlanPtr->SetType(type);
	workSchPlanPtr->SetUsedQuantity(1);
	workSchPlanPtr->SetPlanSelResId(resId);
	workSchPlanPtr->SetSchMdfFlg(2);

	planList.push_back(workSchPlanPtr);
}

void CreateSetPlan(list<WorkSchPlanPtr>& planList ,list<WorkSchPlanPtr>& origPlanList
				   ,WorkPtr&  workPtr,WorkResInfo& workResInfo
				   ,const SBH_TIME& prdStTm,const SBH_TIME& prdEndTm)
{
	//获取计算
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();
	assert(workResInfo.resRelaID != NULL);
	TimeLenInfo timeLenInfo;
	GetSetTmLength(origPlanList,workResInfo,TYPE_POSTSET,timeLenInfo);
	//后设置，从开始求结束
	if(pCalc->VaildTmLen(timeLenInfo))
	{
		SBH_TIME endTm = pCalc->GetNewLimit(prdEndTm,workResInfo,timeLenInfo,Dir_Right);
		SBH_TIME stTm = max(prdEndTm,pCalc->GetNewLimit(endTm,workResInfo,timeLenInfo,Dir_Left));

		for(map<WorkResRelaPtr,int>::const_iterator iter = timeLenInfo.tmPairList.begin();
			iter != timeLenInfo.tmPairList.end();++iter)
		{
			CreatWorkSchPlan(planList , workPtr,iter->first.GetResId(),stTm,endTm,TYPE_POSTSET);
		}

	}

	timeLenInfo.tmPairList.clear();
	GetSetTmLength(origPlanList,workResInfo,TYPE_PRESET,timeLenInfo );
	//后设置，从开始求结束
	if(pCalc->VaildTmLen(timeLenInfo))
	{
		SBH_TIME stTm = pCalc->GetNewLimit(prdStTm,workResInfo,timeLenInfo,Dir_Left);
		SBH_TIME endTm = min(prdStTm,pCalc->GetNewLimit(stTm,workResInfo,timeLenInfo,Dir_Right));
		for(map<WorkResRelaPtr,int>::const_iterator iter = timeLenInfo.tmPairList.begin();
			iter != timeLenInfo.tmPairList.end();++iter)
		{
			CreatWorkSchPlan(planList , workPtr,iter->first.GetResId(),stTm,endTm,TYPE_PRESET);
		}
	}

}

void CalcDynamicLock(TimeLenInfo& lockTm,list<WorkSchPlanPtr> origPlanList)
{
	for(map<WorkResRelaPtr,int>::iterator iter = lockTm.tmPairList.begin();
		iter != lockTm.tmPairList.end();++iter)
	{
		WorkResRelaPtr resRela = iter->first;
		if(resRela->GetRes().GetResLckType() == 3||resRela->GetRes().GetResLckType() == 4)
		{
			for(list<WorkSchPlanPtr>::iterator iter2 = origPlanList.begin();
				iter2 != origPlanList.end();++iter2)
			{
				if((*iter2)->GetPlanSelResId() == resRela->GetResId() && (*iter2)->GetType() == 4)
				{
					iter->second += (*iter2)->GetEndTime() - (*iter2)->GetStartTime();
				}
			}
		}
	}

}
//产生工作的其他任务（前后设置，锁定）计划
void CreateSetLockPlan(list<WorkSchPlanPtr>& planList ,WorkPtr&  workPtr,WorkResInfo& workResInfo,const SBH_TIME& stTm,const SBH_TIME& endTm)
{
	//取原计划，判断是否存在
	list<WorkSchPlanPtr> origPlanList = workPtr->GetSchPlanList();

	if(origPlanList.empty())
		return;

	//获取计算
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();

	//临时信息备份对象
	TestSchedulerInfo testSchedulerInfo;

	//判断锁定是否存在
	TimeLenInfo lockTm ;

	SBH_TIME lockStartTime(endTm),lockEndTime(endTm);

	//计算锁定时间长度
	AMAPI::GetResLockTime(workPtr,workResInfo.resRelaID,workResInfo.assistResRelaList,lockTm.tmPairList);
	//动态锁定时间的计算
	CalcDynamicLock(lockTm,origPlanList);
	if(pCalc->VaildTmLen(lockTm))//判断锁定是否存在
	{
		//制造结束开始正排锁定
		//锁定排程边界
		ScheduleMethod temp = pData->parame.m_curSchMethod ;
		pData->parame.m_curSchMethod = SM_FORCE;
		if (pCalc->CalWorkTime(workPtr,workResInfo,TYPE_LOCKTM,
			lockStartTime,lockTm,testSchedulerInfo,lockStartTime,lockEndTime
			,FUC_F2,Dir_Obverse))
		{
			//清除原计划
			AMAPI::ClearWorkPlan(workPtr->GetId());
			//锁定排成功
			testSchedulerInfo.SurePlan();

			pData->FreeWorkCap(workPtr->GetId());

			list<WorkSchPlanPtr>& tempPlanList = workPtr->GetSchPlanList();

			planList.insert(planList.end(),tempPlanList.begin(),tempPlanList.end());
		}
		pData->parame.m_curSchMethod = temp;
	}

	//设置时间
	//遍历资源
	//主资源
	CreateSetPlan(planList,origPlanList,workPtr,workResInfo,stTm,lockStartTime);

}


//生成工作计划
void CScheduler::CreatWorkSchPlans(list<WorkSchPlanPtr>& planList ,WorkPtr&  workPtr,WorkResInfo& workResInfo,const SBH_TIME& stTm,const SBH_TIME& endTm)
{
	TimeLenInfo timeLenInfo;
	AMAPI::GetStdProductTime(workPtr,workResInfo.resRelaID
		,workResInfo.assistResRelaList
		,timeLenInfo.tmPairList);
	map<WorkResRelaPtr,int>::iterator iTm = 
		timeLenInfo.tmPairList.find(workResInfo.resRelaID);
	if (iTm != timeLenInfo.tmPairList.end() && 
		iTm->second > 0)
	{
		//生成主资源制造时间计划
		CreatWorkSchPlan(planList,workPtr,workResInfo.resRelaID->GetResId(),stTm,endTm);
	}

	//生成副资源制造时间计划
	for(list<WorkResRelaPtr>::iterator iter = workResInfo.assistResRelaList.begin();
		iter != workResInfo.assistResRelaList.end();iter++)
	{
		iTm = timeLenInfo.tmPairList.find(*iter);
		if (iTm != timeLenInfo.tmPairList.end() && 
			iTm->second > 0)
		{
			CreatWorkSchPlan(planList,workPtr,(*iter)->GetResId(),stTm,endTm);
		}
	}

	//工作的其他任务排程
	CreateSetLockPlan(planList,workPtr,workResInfo,stTm,endTm);
}

//获取可用资源
SBH_BOOL GetWorkRes(WorkResInfo& workResInfo,WorkPtr& workPtr)
{
	//multimap<WorkResRelaPtr, list<WorkResRelaPtr>> useableResRelaComb;
	//AMAPI::GetAvailableSrcGroup(workPtr,useableResRelaComb);
	workPtr->GetFinishWorkResComb(workResInfo.resRelaID,workResInfo.assistResRelaList);
	if(workResInfo.resRelaID != NULL)
	{
		return true;
	}

	return false;
}

//计算边界开始时刻
SBH_TIME CScheduler::CalcLimitStTm( const SBH_TIME& startTm ,WorkResInfo& workResInfo, SBH_INT dir)
{
//	SBH_TIME rtTm(0);
	if(dir == Dir_Right)
	{
		return pData->mgRemainCapacityDao.GetOnDutyTime(workResInfo,startTm);
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

		findInfo.m_len = pData->schTools.m_prdTimeLen.Max();

		findInfo.m_time = startTm;

		return pData->mgRemainCapacityDao.GetOnPreDutyTime(findInfo,startTm);

	}
}


//获取工作计划
void CScheduler::GetWorkPlanList( list<WorkSchPlanPtr>& planList ,WorkPtr&  workPtr)
{
	//根据工作类型进行判断
	if(AMAPI::GetParameter()->GetSchLevel() == 1)
	{
		planList = workPtr->GetSchPlanList();
	}
	else
	{
		if(workPtr->GetSchedulerFlg() == SCHEDULINGFINISH
			|| workPtr->GetSchedulerFlg() == SCHEDULEREDFINISH)		//已完工工作
		{
			//计算完工工作状态
			CalcFinishWork(planList,workPtr);

		}
		else if(workPtr->GetSchedulerFlg() == SCHEDULINGSTARTPD
			|| workPtr->GetSchedulerFlg() == SCHEDULEREDSTARTPD)	//已开始未完工工作
		{
			CalcStartPdWork(  planList , workPtr );
		}
		else
		{
			planList = workPtr->GetSchPlanList();
		}
	}

}

//计算完工工作状态
void CScheduler::CalcFinishWork(list<WorkSchPlanPtr>& planList ,WorkPtr&  workPtr)
{
	//获取计算
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();

	//取实际字段
	FactWorkPtr factWorkPtr = workPtr->GetFactData();
	if(factWorkPtr != NULL)
	{
		//获取资源
		WorkResInfo workResInfo;
		if(GetWorkRes(workResInfo,workPtr))
		{
			SBH_TIME stTm = factWorkPtr->GetFactStTm();
			SBH_TIME endTm = factWorkPtr->GetFactEndTm();
			//判断实际开始实际结束是否均存在
			if(stTm > 0 && endTm > 0)
			{
				CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);
			}
			else if(stTm > 0)	//实际开始存在
			{
				//计算制造时间
				pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen);
				//实际开始排,正向
				endTm = pCalc->GetNewLimit(CalcLimitStTm(stTm,workResInfo,Dir_Right),workResInfo,pData->schTools.m_prdTimeLen,Dir_Right);
				CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);
			
			}
			else if(endTm > 0)	//实际结束存在
			{

				//计算制造时间
				pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen);
				//实际结束排,逆向
				stTm = pCalc->GetNewLimit(CalcLimitStTm(endTm,workResInfo,Dir_Left),workResInfo,pData->schTools.m_prdTimeLen,Dir_Left);
				CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);

			}
			else if(factWorkPtr->GetFactGetTm() > 0) //实际取得时刻存在
			{

				//计算制造时间
				pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen);
				//实际取的排,逆向
				endTm = CalcLimitStTm(factWorkPtr->GetFactGetTm(),workResInfo,Dir_Left);
				stTm = pCalc->GetNewLimit(endTm,workResInfo,pData->schTools.m_prdTimeLen,Dir_Left);
				CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);

			}
			else if(factWorkPtr->GetFactMainResId() > 0)	//实际开始和取得都不存在
			{
				//取计划制造的开始结束
				time_t planStTm(MAXINT64),planEndTm(0);
				map<SBH_ULONG,SBH_DOUBLE> resIds;
				list<WorkSchPlanPtr> tempPlanList = workPtr->GetSchPlanList();

				if(PrepareResCap( tempPlanList, resIds, planStTm, planEndTm, TYPE_PRODUCETM))
				{
					CreatWorkSchPlans(planList,workPtr,workResInfo,planStTm,planEndTm);
				}
				else
				{
					planList = workPtr->GetSchPlanList();
				}
			}
			else
			{
				planList = workPtr->GetSchPlanList();
			}
		}
	}
	else	//未输入实际
	{
		planList = workPtr->GetSchPlanList();
	}

}

//计算开始生产的工作
void CScheduler::CalcStartPdWork( list<WorkSchPlanPtr>& planList ,WorkPtr&  workPtr )
{
	//获取计算
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();

	//取实际字段
	FactWorkPtr factWorkPtr = workPtr->GetFactData();
	if(factWorkPtr != NULL)
	{
		//获取资源
		WorkResInfo workResInfo;
		if(GetWorkRes(workResInfo,workPtr))
		{
			SBH_TIME stTm = factWorkPtr->GetFactStTm();
			SBH_TIME endTm = factWorkPtr->GetFactEndTm();
			SBH_TIME getTm = factWorkPtr->GetFactGetTm();
			//判断实际开始实际取得时刻是否均不存在
/*			if(stTm <= 0 && getTm <= 0)
			{
				planList = workPtr->GetSchPlanList();
			}
			else */
			if(stTm > 0)	//实际开始存在
			{
				//判断实际取得是否为空
				if(getTm <= 0)
				{

					//计算制造时间
					pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen);
					//实际开始排,正向
					endTm = pCalc->GetNewLimit(CalcLimitStTm(stTm,workResInfo,Dir_Right),workResInfo,pData->schTools.m_prdTimeLen,Dir_Right);
					CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);

				}
				else	//实际取得存在
				{
					if(endTm == 0)
						AMAPI::GetMaxEndTime(workPtr,TYPE_PRODUCETM,endTm);

					//计算剩余数量
					SBH_DOUBLE remainAcount(0),rate(0);
					if(factWorkPtr->GetFactAcount() > PRECISION)
						remainAcount = workPtr->GetProcessAmount() - factWorkPtr->GetFactAcount();
					else if(getTm > stTm && getTm < endTm)
						rate = double(endTm - getTm)/(endTm - stTm);
					if(remainAcount > PRECISION || rate > PRECISION)
					{
						//计算制造时间
						pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen,remainAcount,rate);
						//实际开始排,正向
						getTm = max(getTm,stTm);
						endTm = pCalc->GetNewLimit(CalcLimitStTm(getTm,workResInfo,Dir_Right),workResInfo,pData->schTools.m_prdTimeLen,Dir_Right);
					}
					else
					{
						stTm = min(stTm,getTm);
						endTm = max(stTm,getTm);
					}

					CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);
	
				}

			}
			else 	//实际开始不存在
			{
				if(getTm > 0) //实际取得时刻存在
				{
					if(factWorkPtr->GetFactAcount() < 0.00001)		//实际数量为空
					{
						//计算制造时间
						pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen);
						//实际取的排,正向
						stTm = CalcLimitStTm(getTm,workResInfo,Dir_Right);
						endTm = pCalc->GetNewLimit(stTm,workResInfo,pData->schTools.m_prdTimeLen,Dir_Right);
						CreatWorkSchPlans(planList,workPtr,workResInfo,getTm,endTm);

					}
					else
					{
	
						//计算制造时间
						pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen,factWorkPtr->GetFactAcount());
						//实际取的排,逆向排开始
						stTm = pCalc->GetNewLimit(CalcLimitStTm(getTm,workResInfo,Dir_Left),workResInfo,pData->schTools.m_prdTimeLen,Dir_Left);

						SBH_DOUBLE remainAcount = workPtr->GetProcessAmount() - factWorkPtr->GetFactAcount();
						if(remainAcount > 0)
						{
							//计算制造时间
							pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen,remainAcount);
							//实际取的排,逆向排开始
							endTm = pCalc->GetNewLimit(CalcLimitStTm(getTm,workResInfo,Dir_Right),workResInfo,pData->schTools.m_prdTimeLen,Dir_Right);
						}
						else
						{
							endTm = CalcLimitStTm(getTm,workResInfo,Dir_Left);
						}

						//计算生成计划
						CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);
	
					}
		
				}
				else if(factWorkPtr->GetFactMainResId() > 0)	//实际开始和取得都不存在
				{
					//取计划制造的开始结束
					time_t planStTm(MAXINT64),planEndTm(0);
					map<SBH_ULONG,SBH_DOUBLE> resIds;
					list<WorkSchPlanPtr> tempPlanList = workPtr->GetSchPlanList();

					if(PrepareResCap( tempPlanList, resIds, planStTm, planEndTm, TYPE_PRODUCETM))
					{
						CreatWorkSchPlans(planList,workPtr,workResInfo,planStTm,planEndTm);
					}
					else
					{
						planList = workPtr->GetSchPlanList();
					}
				}
				else
				{
					planList = workPtr->GetSchPlanList();
				}
			}
		}
	}
	else	//未输入实际
	{
		planList = workPtr->GetSchPlanList();
	}
}

SBH_BOOL CScheduler::DelWithDisobeyRestict_LV2( )
{
	pData->appendWorkDao.StartLoop();
	AppendWork* pAppWork = NULL;
	CDisobeyRestrict disobey;
	while (pData->appendWorkDao.Get(&pAppWork))
	{
		if (pAppWork != NULL && pAppWork->GetIsDisobeyRestrict())
		{
			disobey.GetDisobeyRestrictType(pAppWork,pAppWork->id,false);
		}
		//if (pAppWork != NULL && pAppWork->GetISSchedFlg())
		//{
		//	WorkPtr& ptr = pAppWork->GetWorkPtr();
		//	if (ptr != NULL)
		//		ptr->SetWorkSchedulered();
		//}
	}
	return true;
}