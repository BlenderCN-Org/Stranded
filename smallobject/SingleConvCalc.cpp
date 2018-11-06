
#include "SingleConvCalc.h"
#include "Csv.h"

using namespace ScheduleHelper;


#include "SingleConvCalcSplit.h"
#include "WorkBoundV3.h"
#include "SingleConvShiftCalc.h"
#include "MaxIntervalDisposeManager.h"

#include "Scheduler.h"
#include "Tools.h"
#ifdef _DEBUG_USING_CLOCK
#include "clock.h"

#define CLOCK_MARK(timer) timer.Pause();
#else
#define CLOCK_MARK(timer) 
#endif


SingleConvCalc::SingleConvCalc()
{
	// None logic
}

SingleConvCalc::~SingleConvCalc()
{
	// None logic
}






#include "stdafx.h"
#include "SplitWorkMgr.h"
#include "ResourceMgr.h"

void SingleConvCalc::SingleConvSchedule(int curWorkId, time_t rightBound)
{
	SBH_INT splitedWorkId = 0;

	SingleConvSchedule( curWorkId, rightBound, splitedWorkId );

	while(splitedWorkId > 0)
	{
		curWorkId = splitedWorkId;
		SingleConvSchedule( curWorkId, rightBound, splitedWorkId );

		CScheduler sch;
		sch.DelWithDisobeyRestict_LV1(curWorkId);
	}
}

void SingleConvCalc::SingleConvSchedule( SBH_INT curWorkId, time_t rightBound, SBH_INT& splitedWorkId )
{
	bool bSplitedWork = false;
	if (splitedWorkId > 0)
		bSplitedWork = true;
	splitedWorkId = 0;
	time_t leftStartTm = 0;								//	左开始时刻
	time_t rightCutoffTm = LLONG_MAX;					//	右截至时刻


	AppendWork* pAppWork = pData->GetAppendWork(curWorkId) ;
	if (pAppWork == NULL)
		return;

	WorkPtr& workPtr = pAppWork->GetWorkPtr();
	if (workPtr == NULL)
	{
		return;
	}

	pData->schTools.SetCurAppWork(pAppWork);

	//虚拟排程
	if (pData->parame.m_curSchMethod == SM_VIRTUAL)
	{
		//清理原计划
		pData->RetractWorkPlan(curWorkId);
		CSingleConvCalcV3::Get()->VurtualResScheduler(curWorkId,Dir_Converse);
		pAppWork->SetISSucessedFlg(true);
		pAppWork->SetISSchedFlg(true);
		return;
	}

	//	获得可用资源组合1
	/************************************************************************/
	/* 调用接口                                                             */
	/************************************************************************/
	//资源选择：
	list<WorkResInfo> workResInfoList;

	CResourceMgr resourceMgr(Dir_Converse);
	resourceMgr.GetResComb(workPtr,workResInfoList);

	//清理原计划,固定工作求取资源组合逻辑之前不可以清除计划
	workPtr->ClearSchPlan();

	vector<TestSchedulerInfo> workPlanList;
	
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();
	//如果没有可用资源直接放虚拟资源上
	if (workResInfoList.empty() )
	{
		pCalc->VurtualResScheduler(curWorkId,Dir_Converse);
		//pAppWork->schFlag += 10000;	
		pData->uniteWorkDao.Start();
		pAppWork->SetISSchedFlg(true);
		pAppWork->SetISSucessedFlg(true);
		return;
	}

#ifndef PMCMODE
	if(!pCalc->WorkComb(workPtr,workResInfoList,rightBound))
#endif
	{		
		//遍历所有资源组合
		vector<SchdulerFlg> schFlgList;
		SBH_BOOL succeededFlg = false;

		//TestSchedulerInfo workPlanInfo;
		//TestSchedulerInfo tempSchInfo;
		//SBH_BOOL schFlg = false;
		//SBH_BOOL bSplited = false;
		//SBH_DOUBLE workPlanAmount(0.0f);//工作加工数量
		//SBH_DOUBLE lastProcessAmount(0.0f); //加工数量(用于拆分最后一个工作）
		//WorkPtr&  oldWorkPtr = NULL;

		list<WorkResInfo>::iterator i_workResInfo = workResInfoList.begin();
		//list<SBH_TIME>::iterator iLimit = resLimitTimes.begin();
		while (i_workResInfo != workResInfoList.end())
		{
			WorkResInfo& workResInfo = *i_workResInfo;
			//TestSchedulerInfo tschInfo;
			//SBH_DOUBLE  curProcessAmount(0.0f); //加工数量(用于拆分最后一个工作）
			//SBH_BOOL curSchFlg = false;
			SchdulerFlg schFlg;
			CSplitWorkMgr splitMgr;

			pData->schTools.Init();
			SBH_BOOL curSplitFlg = splitMgr.SplitWork(workPtr,workResInfo);
			if (curSplitFlg)
			{
				pData->schTools.m_splitWorkAmout = splitMgr.GetSplitWorkAmount();
				schFlg.workPlanAmount = pData->schTools.m_splitWorkAmout;
			}
			else if (bSplitedWork)
			{
				schFlg.workPlanAmount = TOOLS::CeilDouble(workPtr->GetProcessAmount(),workResInfo.resRelaID->GetPdtBatchUnit());
			}
			pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen
				,schFlg.workPlanAmount);

			if(!Scheduler( workPtr, leftStartTm,  rightBound, *i_workResInfo ))
			{
				//记录所有资源组合上的排程计划
				workPlanList.push_back(pData->schTools.m_lastSucessTestSchInfo);
				pData->schTools.m_lastSucessTestSchInfo.ClearPlan();


				if(AMAPI::GetSchedulerType() == 2 && pAppWork->GetSchLevel() > SL_Level2)
				{
					if(!BZERO(pData->schTools.m_splitWorkAmout))
					{
						schFlg.workPlanAmount = pData->schTools.m_splitWorkAmout;
						schFlg.bSplited = true;
					}
				}
				else 
				{
					schFlg.bSplited = curSplitFlg;
				}
				//schFlg.bSplited = curSplitFlg;
				schFlg.schFlg = true;
				schFlgList.push_back(schFlg);
				succeededFlg = true;
			}
			else
			{
				workPlanList.push_back(pData->schTools.m_lastSucessTestSchInfo);
				schFlg.bSplited = curSplitFlg;
				schFlg.schFlg = false;
				schFlgList.push_back(schFlg);
			}
			i_workResInfo++;
		}
		assert(pAppWork != NULL);

		//把计划保存数据加入资源能力和排程计划中
		if (succeededFlg && (!workPlanList.empty()) &&workPlanList.size() == schFlgList.size())
		{
			SchdulerFlg bestSchFlg;
			TestSchedulerInfo workPlanInfo;
			//资源评估
			resourceMgr.ResEvaluate(workPtr,workResInfoList,schFlgList,workPlanList
				,bestSchFlg,workPlanInfo,Dir_Converse);

			SBH_INT checkWorkId = workPtr->GetId();
			if (!workPtr->GetSchPlanList().empty())
				AMAPI::ClearWorkPlan(workPtr->GetId());

			//////////////////////////////////////////////////////////////////////////
			//检查先行工作有效性
			//拆分不考虑一起生产检测
			if (!(bestSchFlg.bSplited && bestSchFlg.workPlanAmount >= 0.000001f)
				&& !bSplitedWork)
			{
				SBH_TIME pdSt,pdEt;
				workPlanInfo.GetMinMaxTime(checkWorkId,0,pdSt,pdEt);
				if (!pData->uniteWorkDao.CheckSch_First(checkWorkId,workPlanInfo,pdSt,pdEt,Dir_Converse))
				{
					pAppWork->SetISSucessedFlg(false);
					return;
				}
				else if (!pData->uniteWorkDao.CheckSch_Seq(checkWorkId,workPlanInfo,pdSt,pdEt,Dir_Converse))
				{
					pAppWork->SetISSucessedFlg(false);
					return;
				}
			}
			//////////////////////////////////////////////////////////////////////////
			//拆分
			if (bestSchFlg.bSplited && bestSchFlg.workPlanAmount >= 0.000001f)
			{
				//实拆分
				CSplitWorkMgr splitWorkMgr;
				if (splitWorkMgr.SplitWork(workPtr,bestSchFlg.workPlanAmount))
				{
					if (splitWorkMgr.GetSplitedWorkId1() > 0)
					{
						checkWorkId = splitWorkMgr.GetSplitedWorkId1();
						workPlanInfo.FinallySurePlan(workPtr->GetId(),
							splitWorkMgr.GetSplitedWorkId1());
						splitedWorkId = splitWorkMgr.GetSplitedWorkId2();

						//替换工作Id
						pData->uniteWorkDao.ChangeLastWork(workPtr->GetId(),checkWorkId);
						pData->uniteWorkDao.AddWork(splitedWorkId);

						AppendWork* psplitAppwork = pData->GetAppendWork(splitWorkMgr.GetSplitedWorkId1());
						if (psplitAppwork != NULL)
						{
							psplitAppwork->SetISSucessedFlg(true);
							psplitAppwork->SetISSchedFlg(true);

						}
					}
					else
					{
						workPlanInfo.FinallySurePlan();
						pAppWork->SetISSucessedFlg(true);
					}
				}
			}
			else
			{
				if (bestSchFlg.workPlanAmount > 0.0000001f
					&& bSplitedWork)
				{
					//最后一个拆分工作尾数调整（待修改）
					//workPtr->SetProcessAmount(bestSchFlg.workPlanAmount);
				}
				workPlanInfo.FinallySurePlan();
				pAppWork->SetISSucessedFlg(true);
			}
			//获取变化
			pData->uniteWorkDao.GetWorkCondition(checkWorkId);
		}
		else
		{
			//CSingleConvCalcV3::Get()->PushIntoVirtualRes(curWorkId,Dir_Converse);
			CSingleConvCalcV3::Get()->AllResFailed(curWorkId,workResInfoList,Dir_Converse);
			pAppWork->SetISSucessedFlg(false);
		}
	}	
#ifndef PMCMODE
	else
	{
		workPlanList.push_back(pData->schTools.m_lastSucessTestSchInfo);
	}
#endif
	if (pAppWork == NULL)
		pAppWork = pData->GetAppendWork(workPtr->GetId());

	if(pAppWork != NULL)
	{
		pAppWork->SetISSchedFlg(true);

		//排程成功，放入最大约束检验
		if(pAppWork->GetISSucessedFlg() && splitedWorkId == 0 && pData->workFilter.ExistInMaxIntervalGp(curWorkId)/*g_MaxValRegister.BExist(curWorkId)*/)
		{
			g_MaxValRegister.InsertWorkInfo(curWorkId,workResInfoList,workPlanList);
		}
	}

}

#include "clock.h"
#include "Tools.h"
bool SingleConvCalc::Scheduler(WorkPtr& workPtr, time_t leftStartTm, time_t rightCutoffTm,	WorkResInfo& workResInfo )
{
	if(rightCutoffTm <= 0)
		rightCutoffTm = LLONG_MAX;
	else
	{
		if(AMAPI::GetParameter()->GetSystemSchFlg() != Refined_Sch)
			rightCutoffTm = ScheduleHelper::GetGlancingEndTime(rightCutoffTm,workResInfo.resRelaID->GetResId());
	}

	AppendWork* pAppWork = pData->GetAppendWork(workPtr->GetId());
	if(pAppWork == NULL) return true;

	if(pAppWork->GetSchLevel() > SL_Level2)
	//if(workPtr->GetSchedulerFlg() == SCHEDULINGNOPLANT || workPtr->GetSchedulerFlg() == SCHEDULINGWILLORDER)
	{
		SBH_TIME rightTime(LLONG_MAX);
		if(AMAPI::GetParameter()->GetSystemSchFlg() == Refined_Sch)
		{
			CWorkBoundV3 workBound;

			workBound.CalWorkBound(workPtr,workResInfo,leftStartTm,rightTime,Dir_Converse);
		}
		else
		{
			CWorkBoundGlancing workBound;

			workBound.CalWorkBound(workPtr,workResInfo,leftStartTm,rightTime,Dir_Converse);
		}


		rightCutoffTm = min(rightTime,rightCutoffTm);
	}

	pData->schTools.m_limitTime = leftStartTm;

	if(rightCutoffTm == LLONG_MAX)
		rightCutoffTm = AMAPI::GetSchEndTime();

#ifdef WORKSEQUENCEOUTPUT
	char buf[1024];
	sprintf_s(buf,"workId:%d  resourceID:%d LeftBound: %S  RightBound: %S ",
				workPtr->GetId(),workResInfo.resRelaID->GetResId(),
				TOOLS::FormatTime(CDealTime(leftStartTm)).c_str()
				,TOOLS::FormatTime(CDealTime(rightCutoffTm)).c_str());
	TimeLog::WriteLog(buf);
#endif



	SBH_TIME newTime = pData->uniteWorkDao.GetNewLimits(workPtr->GetId());
	if (newTime > 0)
	{
		rightCutoffTm = min(newTime,rightCutoffTm);
	}

	SingleCalcV3* pCalc = NULL;

	if(AMAPI::GetParameter()->GetSystemSchFlg() == Refined_Sch)
	{
	if(AMAPI::GetSchedulerType() == 2 && pAppWork->GetSchLevel() > SL_Level2)
		pCalc = CSingleConvCalcSplit::Get();
	else if(AMAPI::GetSchedulerType() == 1)
		pCalc = CSingleConvCalcV3::Get();
	}
	else
	{
		pCalc = CSingleConvShiftCalc::Get();
	}

	if(pCalc->Schedule(workPtr,rightCutoffTm,workResInfo,TYPE_PRODUCETM))
	{
		pData->schTools.MeragePlan();
		return false;
	}

	return true;
}

SBH_BOOL SingleConvCalc::SingleConvSchWKPair(SBH_INT nWKPre, SBH_INT nWKNex, time_t leftBound, SBH_BOOL bFixNexWK /* = false */)
{
	WorkPtr wkPre = AMAPI::GetWorkPtrMap()->Get(nWKPre);
	WorkPtr wkNex = AMAPI::GetWorkPtrMap()->Get(nWKNex);
	AppendWork* pAppWorkPre = pData->GetAppendWork(nWKPre);
	AppendWork* pAppWorkNex = pData->GetAppendWork(nWKNex);
	assert(wkPre.ValidPointer() && wkNex.ValidPointer() && pAppWorkPre && pAppWorkNex);

	SBH_TIME tmNexOri(0);

	//取消两个工作计划，从新起点开始排	
	WorkResInfo workResInfo;
	wkPre.GetPlanResRelaCom(workResInfo.resRelaID, workResInfo.assistResRelaList);
	AMAPI::GetMaxEndTime(wkNex, WorkPlan_Produce, tmNexOri);


	wkPre.BackupWorkPlan();
	wkNex.BackupWorkPlan();
	pData->RetractWorkPlan(nWKPre,true);
	pData->RetractWorkPlan(nWKNex,true);

	vector<pair<WorkResInfo, WorkResInfo> > vecWorkRes;
	vector<pair<TestSchedulerInfo, TestSchedulerInfo> > vecTestSchPlan;
	list<WorkResInfo> listResInfoPre, listResInfoNex;
	CResourceMgr resourceMgr(Dir_Converse),resourceMgrNxt(Dir_Converse);
	resourceMgr.GetResComb(wkPre, listResInfoPre);
	AMAPI::ClearWorkPlan(nWKPre);

	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();
	for (list<WorkResInfo>::iterator iter1 = listResInfoPre.begin(); 
		iter1 != listResInfoPre.end(); ++iter1)
	{
		TestSchedulerInfo preTestSchInfo;
		pData->schTools.SetCurAppWork(pAppWorkPre);
		pData->schTools.Init();
		pCalc->GetPrdTm(wkPre, *iter1, pData->schTools.m_prdTimeLen);
		time_t tempTm = ((*iter1) == workResInfo) ? leftBound : 0;
		//排程计算
		if(!Scheduler(wkPre, 0, tempTm, *iter1))
		{
			preTestSchInfo = pData->schTools.m_lastSucessTestSchInfo;
			pData->schTools.m_lastSucessTestSchInfo.ClearPlan();
			preTestSchInfo.FinallySurePlan();
			pAppWorkPre->SetISSucessedFlg(true);
			pAppWorkPre->SetISSchedFlg(true);

			SBH_TIME tmBeg(0), tmEnd(0);
			AMAPI::GetMinStartTime(wkPre, WorkPlan_Produce, tmEnd);

			listResInfoNex.clear();
			wkNex.RecoverWorkPlan();
			resourceMgrNxt.GetResComb(wkNex, listResInfoNex);
			AMAPI::ClearWorkPlan(nWKNex);
			for (list<WorkResInfo>::iterator iter2 = listResInfoNex.begin();
				iter2 != listResInfoNex.end(); ++iter2)
			{
				TestSchedulerInfo nexTestSchInfo;
				pData->schTools.SetCurAppWork(pAppWorkNex);
				pData->schTools.Init();
				pCalc->GetPrdTm(wkNex, *iter2, pData->schTools.m_prdTimeLen);
				tempTm = (bFixNexWK/*&&(*iter2)==nxtWorkResInfo*/) ? tmNexOri : 0;
				if (!Scheduler(wkNex, 0, tempTm, *iter2))
				{
					nexTestSchInfo = pData->schTools.m_lastSucessTestSchInfo;
					pData->schTools.m_lastSucessTestSchInfo.ClearPlan();
					nexTestSchInfo.FinallySurePlan();
					pAppWorkNex->SetISSucessedFlg(true);
					pAppWorkNex->SetISSchedFlg(true);

					list<Resource> lockRes;
					if (iter2->resRelaID.GetRes().GetResLckType() == 3 || iter2->resRelaID.GetRes().GetResLckType() == 4)
					{
						lockRes.push_back(iter1->resRelaID.GetRes());
					}

					for (list<WorkResRelaPtr>::iterator iterWRR = iter2->assistResRelaList.begin();
						iterWRR != iter2->assistResRelaList.end(); ++iterWRR)
					{
						if (iterWRR->GetRes().GetResLckType() == 3 || iterWRR->GetRes().GetResLckType() == 4)
						{
							lockRes.push_back(iterWRR->GetRes());
						}
					}

					bool bAdjusted = false;
					if (!lockRes.empty())
					{
						AMAPI::GetMaxEndTime(wkNex, WorkPlan_Produce, tmBeg);
						SBH_TIME newSt(MAXTIME), tempSt(MAXTIME);
						SBH_BOOL bAdjust(false);
						for (list<Resource>::iterator iterLockRes = lockRes.begin();
							iterLockRes != lockRes.end(); ++iterLockRes)
						{
							list<SBH_INT> workIdList;
							pData->mgRemainCapacityDao.GetResWorkList(iterLockRes->GetId(), workIdList, tmBeg+1, tmEnd-1);
							workIdList.remove(nWKPre);
							workIdList.remove(nWKNex);
							if (!workIdList.empty())
								bAdjust = true;

							for(list<SBH_INT>::iterator iterWorkList = workIdList.begin();
								iterWorkList != workIdList.end();++iterWorkList)
							{
								if (false == AMAPI::GetMinStartTime(AMAPI::GetWorkPtrMap()->Get(*iterWorkList), WorkPlan_PreSet, tempSt))
									AMAPI::GetMinStartTime(AMAPI::GetWorkPtrMap()->Get(*iterWorkList), WorkPlan_Produce, tempSt);
								newSt = min(tempSt, newSt);
							}
						}

						if (bAdjust)
						{
							pData->RetractWorkPlan(nWKPre);
							pData->RetractWorkPlan(nWKNex);

							pData->schTools.SetCurAppWork(pAppWorkPre);
							pData->schTools.Init();
							pCalc->GetPrdTm(wkPre, *iter1, pData->schTools.m_prdTimeLen);
							TimeLenInfo tmLen;
							SingleCalcV3 singleCal;
							singleCal.GetPrdTm(wkPre, *iter1, tmLen);
							SBH_TIME rtStTm(MAXTIME), rtEndTm(MAXTIME);
							TestSchedulerInfo tsInfo;
							singleCal.CalWorkTime(wkPre, *iter1, TYPE_PRODUCETM, newSt
								,tmLen, tsInfo, rtStTm, rtEndTm, FUC_F2, Dir_Obverse, false, false);
							if (!Scheduler(wkPre, 0, rtEndTm, *iter1))
							{
								TestSchedulerInfo newPreTSInfo = pData->schTools.m_lastSucessTestSchInfo;
								pData->schTools.m_lastSucessTestSchInfo.ClearPlan();
								//newPreTSInfo.FinallySurePlan();

								pData->schTools.SetCurAppWork(pAppWorkNex);
								pData->schTools.Init();
								pCalc->GetPrdTm(wkNex, *iter2, pData->schTools.m_prdTimeLen);
								rtEndTm = bFixNexWK ? tmNexOri : rtEndTm;
								if (!Scheduler(wkNex, 0, rtEndTm, *iter2))
								{
									nexTestSchInfo = pData->schTools.m_lastSucessTestSchInfo;
									pData->schTools.m_lastSucessTestSchInfo.ClearPlan();
									//nexTestSchInfo.FinallySurePlan();
									vecTestSchPlan.push_back(make_pair(newPreTSInfo, nexTestSchInfo));
									vecWorkRes.push_back(make_pair(*iter1, *iter2));
									bAdjusted = true;
								}
							}
						}
					}

					if (false == bAdjusted)
					{
						pData->RetractWorkPlan(nWKNex);
						vecTestSchPlan.push_back(make_pair(preTestSchInfo, nexTestSchInfo));
						vecWorkRes.push_back(make_pair(*iter1, *iter2));
					}
					else
					{
						preTestSchInfo.FinallySurePlan();
						pAppWorkPre->SetISSucessedFlg(true);
						pAppWorkPre->SetISSchedFlg(true);
					}
				}
			}
			pData->RetractWorkPlan(nWKPre);
		}
	}

	vector<TestSchedulerInfo> vecPreTS, vecNexTs;
	assert(vecTestSchPlan.size() == vecWorkRes.size());
	listResInfoPre.clear();
	SBH_INT nIndex = 0;
	for (vector<pair<TestSchedulerInfo, TestSchedulerInfo> >::iterator iter = vecTestSchPlan.begin();
		iter != vecTestSchPlan.end(); ++iter, ++nIndex)
	{
		vecPreTS.push_back(iter->first);
		listResInfoPre.push_back(vecWorkRes[nIndex].first);
	}
	SchdulerFlg bestSchFlg;
	TestSchedulerInfo preWorkPlan;
	vector<SchdulerFlg> listSchFlgPre, listSchFlgNex;
	SchdulerFlg tempFlg;
	tempFlg.schFlg = true;
	listSchFlgPre.resize(listResInfoPre.size(),tempFlg);
	resourceMgr.ResEvaluate(wkPre, listResInfoPre, listSchFlgPre, vecPreTS
		,bestSchFlg, preWorkPlan, Dir_Converse);

	if (!preWorkPlan.Empty())
	{
		listResInfoNex.clear();
		nIndex = 0;
		for (vector<pair<TestSchedulerInfo, TestSchedulerInfo> >::iterator iter = vecTestSchPlan.begin();
			iter != vecTestSchPlan.end(); ++iter, ++nIndex)
		{
			if (iter->first == preWorkPlan)
			{
				vecNexTs.push_back(iter->second);
				listResInfoNex.push_back(vecWorkRes[nIndex].second);
			}
		}

		listSchFlgNex.resize(vecNexTs.size(),tempFlg);
		SchdulerFlg nexSchFlg;
		TestSchedulerInfo nexWorkPlan;
		resourceMgrNxt.ResEvaluate(wkNex, listResInfoNex, listSchFlgNex, vecNexTs
			,nexSchFlg, nexWorkPlan, Dir_Converse);

		if (!nexWorkPlan.Empty())
		{
			preWorkPlan.FinallySurePlan();
			nexWorkPlan.FinallySurePlan();
			pAppWorkPre->SetISSucessedFlg(true);
			pAppWorkPre->SetISSchedFlg(true);
			pAppWorkNex->SetISSucessedFlg(true);
			pAppWorkNex->SetISSchedFlg(true);

			if (bFixNexWK)	// 后工作限制了排程时间点，结果可能不够优
			{
				WorkResInfo newWRInfo;
				wkPre.GetPlanResRelaCom(newWRInfo.resRelaID, newWRInfo.assistResRelaList);
				if (!(newWRInfo == workResInfo))	// 前工作换资源，后工作在资源固定下重排
				{
					SBH_TIME tmPreWKStart(0);
					AMAPI::GetMinStartTime(wkPre, WorkPlan_Produce, tmPreWKStart);
					wkNex.BackupWorkPlan();
					pData->RetractWorkPlan(nWKNex, true);
					const bool fixFlg = wkNex.GetResFix();
					wkNex.SetIsFixRes(true);
					SingleConvSchedule(nWKNex, tmPreWKStart);
					wkNex.SetIsFixRes(fixFlg);
				}
			}
			return true;
		}
	}

	return false;
}