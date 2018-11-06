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

#define WORKSCHEMODE_IMMOVABILITY		1		//�ƻ�����
#define WORKSCHEMODE_TIMEFIX			2		//�̶���������
#define WORKSCHEMODE_COMMONSCHE			3		//��ͨ�ų�
#define	WORKSCHEMODE_NONE				4		//������
#define WORKSCHEMODE_DISOBEY_RESTRICT	5		//Υ��Լ������


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
		//�������ȹ�ϵ
		//CPriorityMgr priMgr;
		//priMgr.CreatePriority(workId);
		pData->schTools.SetCurAppWork(pComWork);
		//�жϹ����Ƿ��мƻ�
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
				//���������,ɾ���ƻ�,�޸ķ���
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

//ȡ�����ƻ���ʼ������ʱ�̼���Դ
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
			//�����¼ƻ�
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

	//��ȡ�����ƻ��б�	
	list<WorkSchPlanPtr> planList;
	GetWorkPlanList(planList,workPtr);

	//���ԭ�ƻ�
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
		else if(*iter == TYPE_PRODUCETM)	//�����ų�,��������
		{
			WorkResInfo workResInfo;

			//�����ƻ���ȡʹ�ù�����Դ
			time_t stTm(MAXINT64),endTm(0);
			set<int> planResIdList;
			for(list<WorkSchPlanPtr>::iterator iter1 = planList.begin();
				iter1 != planList.end();iter1++)
			{
				WorkSchPlanPtr& planPtr = *iter1;
				//�ж��Ƿ�Ϊ
				planResIdList.insert(planPtr->GetPlanSelResId());
				stTm = min(stTm,ScheduleHelper::GetGlancingStartTime(planPtr->GetStartTime(),planPtr->GetPlanSelResId()));
				endTm = max(endTm,ScheduleHelper::GetGlancingStartTime(planPtr->GetEndTime(),planPtr->GetPlanSelResId()));
			}

			//��������
			if(stTm != endTm)
			{
				PlanUse(workId,planList,*iter);
				break;
			}

			//��ȡ��Դ
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
				//����
				if(ScheduleHelper::GetGlancingStartTime(pData->schTools.m_lastSucessTestSchInfo.GetPdEndTime(workId),workResInfo.resRelaID->GetResId()) != stTm)
				{
					pData->schTools.m_lastSucessTestSchInfo.ClearPlan();
					//���������������
					TestSchedulerInfo tsInfo;
					SBH_TIME rtStTm,rtEndTm; 
					endTm = ScheduleHelper::GetGlancingEndTime(stTm,workResInfo.resRelaID->GetResId());
					//������������
					ScheduleMethod temp = pData->parame.m_curSchMethod;
					pData->parame.m_curSchMethod = SM_INFINITY;	//��������
					if(pCalc->CalWorkTime(workPtr,workResInfo,TYPE_PRODUCETM,endTm
						,pData->schTools.m_prdTimeLen,tsInfo,rtStTm,rtEndTm,FUC_F1,Dir_Converse))
					{
						if(ScheduleHelper::GetGlancingStartTime(rtStTm,workResInfo.resRelaID->GetResId()) != stTm)
						{
							//���Ź�������һ�죬������������
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
	//���ݼƻ������������������Ϸֶ�(�ж�)
	CalcInterruptedTime(workId);

	map<SBH_ULONG,TimeRangeList> prePlanList;	//ǰ����
	map<SBH_ULONG,TimeRangeList> prdPlanList;	//����
	map<SBH_ULONG,TimeRangeList> postPlanList;	//������
	map<SBH_ULONG,TimeRangeList> lockPlanList;	//����

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
	////����ƻ�
	//if(!prePlanList.empty())
	//{
	//	pData->mgRemainCapacityDao.UnionUse(workId,TYPE_PRESET,prePlanList);
	//}

	//�������ع�����Ҫ���ƻ�����Դ���������������
	if(pData->workFilter.ExistInMaxIntervalGp(workId)/*g_MaxValRegister.BExist(workId)*/)
	{
		//��ȡ����
		WorkPtr&  workPtr = AMAPI::GetWorkPtrMap()->Get(workId);
		//��ȡ��Դ���
		WorkResInfo workResInfo;
		AMAPI::GetPlanResRelaCom(workPtr,workResInfo.resRelaID,workResInfo.assistResRelaList);
		list<WorkResInfo> workResInfoList;
		workResInfoList.push_back(workResInfo);
		//��ȡ�����ƻ�
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

	//���Ų���δ�ɹ��ų�/Υ��Լ������
	if (pAppWork != NULL && pAppWork->GetISSchedFlg()
		&& !pAppWork->GetISSucessedFlg())
		return WORKSCHEMODE_DISOBEY_RESTRICT;

	return WORKSCHEMODE_COMMONSCHE;
}

void CScheduler::DisobeyRestrictSch( SBH_INT workId,DirType dt)
{
	if (pData->parame.m_curDisRestrcitType != DRT_NO)
	{
		//ɾ������ƻ�
		pData->RetractWorkPlan(workId);

		AppendWork* pComWork = pData->GetAppendWork(workId);

		//������
		if (pData->parame.m_curSchMethod == SM_UNDISPATCH)
		{
			pData->parame.m_curSchMethod = SM_LIMITARY;
			pData->parame.m_curDisRestrcitType = DRT_NO;

			//modify by ���� 2010.8.10
			//��Ҫ��Ӳ����ų�����
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
	//modify by ���� 2010.8.10
	//��Ҫ��Ӳ����ų�����
	bool flg = false;
	if(pComWork != NULL 
		&& ( pComWork->GetSchLevel() == SL_Level3 || pComWork->GetSchLevel() == SL_Level4 )
		&& pComWork->GetISSchedFlg()
		&& !pComWork->GetISSucessedFlg()
		&& AMAPI::IsParticipateSchWork( workId))
	{
		//���Υ��Լ������ȡΥ��Լ������ʽ
		CDisobeyRestrict disobey;
		pData->parame.m_curDisRestrcitType = disobey.GetDisobeyRestrictType(pComWork,workId,false);
		pData->parame.m_curSchMethod = disobey.GetDisobeyRestrictWay(
			pData->parame.m_curDisRestrcitType);
		if (pData->parame.m_curDisRestrcitType != DRT_NO)
		{
			if (pData->parame.m_curSchMethod != SM_CHANGEDIRECTION)
			{
				//���ݷ���
				DirType saveDirType = pComWork->GetDirection();

				SBH_SHORT curState = pData->parame.m_curSchMethod;

				disobey.DirectionMgr(pComWork,pData->parame.m_curDisRestrcitType
					,pData->parame.m_curSchMethod);


				//Υ��Լ���ų�
				Scheduling(workId);
				//����ų�����ʧ��
				//disobey.RecordeDisobyRestrict(pData->parame.m_curDisRestrcitType,workId);		
				if(curState == SM_OVERTIME)
				{
					pComWork->GetWorkPtr().SetWorkFlg(SchFlg_Sched_OverTime);
				}
				else if(curState == SM_FORCE)					//��ԭ����
				{
					pComWork->GetWorkPtr().SetWorkFlg(SchFlg_Sched_Force);
					pComWork->SetDirection(saveDirType);
				}

				//���������ų�
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
	//�����л�����
	if (CDisobeyRestrictDirctionMgr::Get()->ChangeDirection())
	{
		ChangeDirection();
	}
	return flg;
}

//���ָ�������ڹ���
void CScheduler::ClearPlan(int workId,SBH_TIME leftLimit,SBH_TIME rightLimit)
{
	pData->FreeWorkCap(workId);

	WorkPtr& workPtr = AMAPI::GetWorkPtrMap()->Get(workId);
	list<WorkSchPlanPtr> delWorkPlan = workPtr->ClearSchPlan(leftLimit,rightLimit);

	//�����Դ������Ϣ���мƻ�
	map<SBH_ULONG,TimeRangeList> prdPlanList ; /*���� */ 

	//ȡ�����ƻ�
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
	//�ų������ж�
	if(AMAPI::GetParameter()->GetSystemSchFlg() == Refined_Sch)
		return ;
	if (AMAPI::GetParameter()->GetIgnoreTime() <= 0)
		return;

	//��ȡ�������쿪ʼ������ʱ��
	SBH_TIME stTm(0),endTm(0);
	AMAPI::GetWorkPlanEnd(workId,TYPE_PRODUCETM,Dir_Left,stTm);
	AMAPI::GetWorkPlanEnd(workId,TYPE_PRODUCETM,Dir_Right,endTm);

	//�ж���Ч��
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

	//�������䣬��ȡ�����ڶ�Ӧ�����ļƻ�ʱ��
	int prdTmLeft = AMAPI::GetWorkPlanLength(workId,stLeft,stRight);
	int prdTmRight = AMAPI::GetWorkPlanLength(workId,endLeft,endRight);

	
	//С�ڿɺ���ʱ��
	if(prdTmLeft <= AMAPI::GetParameter()->GetIgnoreTime())
	{
		//�ұߵ�С����ߵ�
		if(prdTmRight < prdTmLeft )
		{
			//���ʱ����ڹ����ƻ�
			ClearPlan(workId,endLeft,endRight);
			dt == Dir_Converse ?
				pAppWork->SetWorkPlanLeftCutFlg():pAppWork->SetWorkPlanRightCutFlg();
			//����ֻ������2������
			AMAPI::GetWorkPlanEnd(workId,TYPE_PRODUCETM,Dir_Right,endTm);
			if(ScheduleHelper::GetGlancingStartTime(endTm-1,workResInfo.resRelaID->GetResId()) == stLeft)
				return;
		}

		//���ʱ����ڹ����ƻ�
		ClearPlan(workId,stLeft,stRight);
		dt == Dir_Converse ?
			pAppWork->SetWorkPlanLeftCutFlg():pAppWork->SetWorkPlanRightCutFlg();

		//����ֻ������2������
		AMAPI::GetWorkPlanEnd(workId,TYPE_PRODUCETM,Dir_Left,stTm);
		if(ScheduleHelper::GetGlancingStartTime(stTm,workResInfo.resRelaID->GetResId()) == endLeft)
			return;

	}

	if(prdTmRight <= AMAPI::GetParameter()->GetIgnoreTime())
	{
		//���ʱ����ڹ����ƻ�
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

	//Բ���ų̽��
	CutFinelyPlan(workId,dt);
}

void CScheduler::TimefixSchdule( SBH_INT workId,DirType dt )
{
	//ȷ�Ϲ���ָ����ʼʱ��
	time_t startAt = AMAPI::GetWorkLimit(workId,STARTAT);
	if(startAt > AMAPI::GetSchStartTime())
	{
		//��߽�̶��ų�
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

	//����
	if(dt == Dir_Obverse   )
	{
		//ȡ����������߽�
		time_t prdStTm(0);
		AMAPI::GetWorkPlanEnd(workId,TYPE_PRODUCETM,Dir_Left,prdStTm);

		prdStTm = max(prdStTm,AMAPI::GetSchStartTime());
		prdStTm = max(prdStTm,AMAPI::GetWorkLimit(workId,STARTAFTER));
		//��߽�̶��ų�
		SingleObvCalc singleObv;
		singleObv.SingleObvSchedule(workId, prdStTm);
	}
	else
	{
		//ȡ���������ұ߽�
		time_t prdEndTm(LLONG_MAX);
		AMAPI::GetWorkPlanEnd(workId,TYPE_PRODUCETM,Dir_Right,prdEndTm);
		prdEndTm = min(prdEndTm,AMAPI::GetSchEndTime());
		prdEndTm = min(prdEndTm,AMAPI::GetWorkLimit(workId,ENDBEFORE));
		SingleConvCalc singleConv;
		singleConv.SingleConvSchedule(workId, prdEndTm);
	}
}

#include "SingleObvCalcV3.h"
//�л�������
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

		//���¼��㹤���߽�
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
		//ɾ���ƻ�
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
	//���¹�����Χ

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

//��ȡ��Դ��Ӧ����ʱ�䳤��
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
	//��ȡ����
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();
	assert(workResInfo.resRelaID != NULL);
	TimeLenInfo timeLenInfo;
	GetSetTmLength(origPlanList,workResInfo,TYPE_POSTSET,timeLenInfo);
	//�����ã��ӿ�ʼ�����
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
	//�����ã��ӿ�ʼ�����
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
//������������������ǰ�����ã��������ƻ�
void CreateSetLockPlan(list<WorkSchPlanPtr>& planList ,WorkPtr&  workPtr,WorkResInfo& workResInfo,const SBH_TIME& stTm,const SBH_TIME& endTm)
{
	//ȡԭ�ƻ����ж��Ƿ����
	list<WorkSchPlanPtr> origPlanList = workPtr->GetSchPlanList();

	if(origPlanList.empty())
		return;

	//��ȡ����
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();

	//��ʱ��Ϣ���ݶ���
	TestSchedulerInfo testSchedulerInfo;

	//�ж������Ƿ����
	TimeLenInfo lockTm ;

	SBH_TIME lockStartTime(endTm),lockEndTime(endTm);

	//��������ʱ�䳤��
	AMAPI::GetResLockTime(workPtr,workResInfo.resRelaID,workResInfo.assistResRelaList,lockTm.tmPairList);
	//��̬����ʱ��ļ���
	CalcDynamicLock(lockTm,origPlanList);
	if(pCalc->VaildTmLen(lockTm))//�ж������Ƿ����
	{
		//���������ʼ��������
		//�����ų̱߽�
		ScheduleMethod temp = pData->parame.m_curSchMethod ;
		pData->parame.m_curSchMethod = SM_FORCE;
		if (pCalc->CalWorkTime(workPtr,workResInfo,TYPE_LOCKTM,
			lockStartTime,lockTm,testSchedulerInfo,lockStartTime,lockEndTime
			,FUC_F2,Dir_Obverse))
		{
			//���ԭ�ƻ�
			AMAPI::ClearWorkPlan(workPtr->GetId());
			//�����ųɹ�
			testSchedulerInfo.SurePlan();

			pData->FreeWorkCap(workPtr->GetId());

			list<WorkSchPlanPtr>& tempPlanList = workPtr->GetSchPlanList();

			planList.insert(planList.end(),tempPlanList.begin(),tempPlanList.end());
		}
		pData->parame.m_curSchMethod = temp;
	}

	//����ʱ��
	//������Դ
	//����Դ
	CreateSetPlan(planList,origPlanList,workPtr,workResInfo,stTm,lockStartTime);

}


//���ɹ����ƻ�
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
		//��������Դ����ʱ��ƻ�
		CreatWorkSchPlan(planList,workPtr,workResInfo.resRelaID->GetResId(),stTm,endTm);
	}

	//���ɸ���Դ����ʱ��ƻ�
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

	//���������������ų�
	CreateSetLockPlan(planList,workPtr,workResInfo,stTm,endTm);
}

//��ȡ������Դ
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

//����߽翪ʼʱ��
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

		//д��Դ��(��������Դ)
		findInfo.m_resCaps[workResInfo.resRelaID->GetResId()] = 0;
		//����Դ
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


//��ȡ�����ƻ�
void CScheduler::GetWorkPlanList( list<WorkSchPlanPtr>& planList ,WorkPtr&  workPtr)
{
	//���ݹ������ͽ����ж�
	if(AMAPI::GetParameter()->GetSchLevel() == 1)
	{
		planList = workPtr->GetSchPlanList();
	}
	else
	{
		if(workPtr->GetSchedulerFlg() == SCHEDULINGFINISH
			|| workPtr->GetSchedulerFlg() == SCHEDULEREDFINISH)		//���깤����
		{
			//�����깤����״̬
			CalcFinishWork(planList,workPtr);

		}
		else if(workPtr->GetSchedulerFlg() == SCHEDULINGSTARTPD
			|| workPtr->GetSchedulerFlg() == SCHEDULEREDSTARTPD)	//�ѿ�ʼδ�깤����
		{
			CalcStartPdWork(  planList , workPtr );
		}
		else
		{
			planList = workPtr->GetSchPlanList();
		}
	}

}

//�����깤����״̬
void CScheduler::CalcFinishWork(list<WorkSchPlanPtr>& planList ,WorkPtr&  workPtr)
{
	//��ȡ����
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();

	//ȡʵ���ֶ�
	FactWorkPtr factWorkPtr = workPtr->GetFactData();
	if(factWorkPtr != NULL)
	{
		//��ȡ��Դ
		WorkResInfo workResInfo;
		if(GetWorkRes(workResInfo,workPtr))
		{
			SBH_TIME stTm = factWorkPtr->GetFactStTm();
			SBH_TIME endTm = factWorkPtr->GetFactEndTm();
			//�ж�ʵ�ʿ�ʼʵ�ʽ����Ƿ������
			if(stTm > 0 && endTm > 0)
			{
				CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);
			}
			else if(stTm > 0)	//ʵ�ʿ�ʼ����
			{
				//��������ʱ��
				pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen);
				//ʵ�ʿ�ʼ��,����
				endTm = pCalc->GetNewLimit(CalcLimitStTm(stTm,workResInfo,Dir_Right),workResInfo,pData->schTools.m_prdTimeLen,Dir_Right);
				CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);
			
			}
			else if(endTm > 0)	//ʵ�ʽ�������
			{

				//��������ʱ��
				pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen);
				//ʵ�ʽ�����,����
				stTm = pCalc->GetNewLimit(CalcLimitStTm(endTm,workResInfo,Dir_Left),workResInfo,pData->schTools.m_prdTimeLen,Dir_Left);
				CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);

			}
			else if(factWorkPtr->GetFactGetTm() > 0) //ʵ��ȡ��ʱ�̴���
			{

				//��������ʱ��
				pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen);
				//ʵ��ȡ����,����
				endTm = CalcLimitStTm(factWorkPtr->GetFactGetTm(),workResInfo,Dir_Left);
				stTm = pCalc->GetNewLimit(endTm,workResInfo,pData->schTools.m_prdTimeLen,Dir_Left);
				CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);

			}
			else if(factWorkPtr->GetFactMainResId() > 0)	//ʵ�ʿ�ʼ��ȡ�ö�������
			{
				//ȡ�ƻ�����Ŀ�ʼ����
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
	else	//δ����ʵ��
	{
		planList = workPtr->GetSchPlanList();
	}

}

//���㿪ʼ�����Ĺ���
void CScheduler::CalcStartPdWork( list<WorkSchPlanPtr>& planList ,WorkPtr&  workPtr )
{
	//��ȡ����
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();

	//ȡʵ���ֶ�
	FactWorkPtr factWorkPtr = workPtr->GetFactData();
	if(factWorkPtr != NULL)
	{
		//��ȡ��Դ
		WorkResInfo workResInfo;
		if(GetWorkRes(workResInfo,workPtr))
		{
			SBH_TIME stTm = factWorkPtr->GetFactStTm();
			SBH_TIME endTm = factWorkPtr->GetFactEndTm();
			SBH_TIME getTm = factWorkPtr->GetFactGetTm();
			//�ж�ʵ�ʿ�ʼʵ��ȡ��ʱ���Ƿ��������
/*			if(stTm <= 0 && getTm <= 0)
			{
				planList = workPtr->GetSchPlanList();
			}
			else */
			if(stTm > 0)	//ʵ�ʿ�ʼ����
			{
				//�ж�ʵ��ȡ���Ƿ�Ϊ��
				if(getTm <= 0)
				{

					//��������ʱ��
					pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen);
					//ʵ�ʿ�ʼ��,����
					endTm = pCalc->GetNewLimit(CalcLimitStTm(stTm,workResInfo,Dir_Right),workResInfo,pData->schTools.m_prdTimeLen,Dir_Right);
					CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);

				}
				else	//ʵ��ȡ�ô���
				{
					if(endTm == 0)
						AMAPI::GetMaxEndTime(workPtr,TYPE_PRODUCETM,endTm);

					//����ʣ������
					SBH_DOUBLE remainAcount(0),rate(0);
					if(factWorkPtr->GetFactAcount() > PRECISION)
						remainAcount = workPtr->GetProcessAmount() - factWorkPtr->GetFactAcount();
					else if(getTm > stTm && getTm < endTm)
						rate = double(endTm - getTm)/(endTm - stTm);
					if(remainAcount > PRECISION || rate > PRECISION)
					{
						//��������ʱ��
						pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen,remainAcount,rate);
						//ʵ�ʿ�ʼ��,����
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
			else 	//ʵ�ʿ�ʼ������
			{
				if(getTm > 0) //ʵ��ȡ��ʱ�̴���
				{
					if(factWorkPtr->GetFactAcount() < 0.00001)		//ʵ������Ϊ��
					{
						//��������ʱ��
						pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen);
						//ʵ��ȡ����,����
						stTm = CalcLimitStTm(getTm,workResInfo,Dir_Right);
						endTm = pCalc->GetNewLimit(stTm,workResInfo,pData->schTools.m_prdTimeLen,Dir_Right);
						CreatWorkSchPlans(planList,workPtr,workResInfo,getTm,endTm);

					}
					else
					{
	
						//��������ʱ��
						pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen,factWorkPtr->GetFactAcount());
						//ʵ��ȡ����,�����ſ�ʼ
						stTm = pCalc->GetNewLimit(CalcLimitStTm(getTm,workResInfo,Dir_Left),workResInfo,pData->schTools.m_prdTimeLen,Dir_Left);

						SBH_DOUBLE remainAcount = workPtr->GetProcessAmount() - factWorkPtr->GetFactAcount();
						if(remainAcount > 0)
						{
							//��������ʱ��
							pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen,remainAcount);
							//ʵ��ȡ����,�����ſ�ʼ
							endTm = pCalc->GetNewLimit(CalcLimitStTm(getTm,workResInfo,Dir_Right),workResInfo,pData->schTools.m_prdTimeLen,Dir_Right);
						}
						else
						{
							endTm = CalcLimitStTm(getTm,workResInfo,Dir_Left);
						}

						//�������ɼƻ�
						CreatWorkSchPlans(planList,workPtr,workResInfo,stTm,endTm);
	
					}
		
				}
				else if(factWorkPtr->GetFactMainResId() > 0)	//ʵ�ʿ�ʼ��ȡ�ö�������
				{
					//ȡ�ƻ�����Ŀ�ʼ����
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
	else	//δ����ʵ��
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