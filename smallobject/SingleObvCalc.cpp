
#include "SingleObvCalc.h"
#include "AM_Def.h"
#include "AM_WorkResRela.h"
#include "Csv.h"
using namespace ScheduleHelper;

#include "SingleObvCalcSplit.h"
#include "WorkBoundV3.h"
#include "SingleObvShiftCalc.h"
#include "MaxIntervalDispose.h"

#include "MaxIntervalDisposeManager.h"

#include "Scheduler.h"
#include "Tools.h"
#include "..\..\YKBizModel\ForAlgTemp.h"
#ifdef _DEBUG_USING_CLOCK
#include "clock.h"
static Timer T_CalcScheduling(L"CalcScheduling");
#define CLOCK_MARK(timer) timer.Pause();
#else
#define CLOCK_MARK(timer) NULL;
#endif

#ifndef BZERO
#define BZERO(x) ((x) > -0.00001 && (x) < 0.00001)
#endif

SingleObvCalc::SingleObvCalc(void)
{
}

SingleObvCalc::~SingleObvCalc(void){}


#include "SplitWorkMgr.h"
#include "ResourceMgr.h"

#ifdef RELEASETESTOUTPUT
#include "clock.h"
#include "Tools.h"
#endif
void SingleObvCalc::SingleObvSchedule(SBH_INT curWorkId, time_t leftBound)
{
	SBH_INT splitedWorkId = 0;

	SingleObvSchedule( curWorkId, leftBound, splitedWorkId );

	while(splitedWorkId > 0)
	{
		curWorkId = splitedWorkId;
		

		SingleObvSchedule( curWorkId, leftBound, splitedWorkId );

		CScheduler sch;
		sch.DelWithDisobeyRestict_LV1(curWorkId);
	
	}
}

//�������ų�
void SingleObvCalc::SingleObvSchedule( SBH_INT curWorkId, time_t leftBound, SBH_INT& splitedWorkId )
{
	bool bSplitedWork = (splitedWorkId > 0);
	splitedWorkId = 0;

	time_t leftStartTm = leftBound;								//	��ʼʱ��
	time_t rightCutoffTm = LLONG_MAX;							//	�ҽ���ʱ��

	AppendWork* pAppWork = pData->GetAppendWork(curWorkId);
	if (pAppWork == NULL) return;

	WorkPtr& workPtr = pAppWork->GetWorkPtr();
	if (false == workPtr.ValidPointer()) return;
	pData->schTools.SetCurAppWork(pAppWork);

	//�����ų�
	if (pData->parame.m_curSchMethod == SM_VIRTUAL)
	{
		pData->RetractWorkPlan(curWorkId);
		CSingleObvCalcV3::Get()->VurtualResScheduler(curWorkId,Dir_Obverse);
		pAppWork->SetISSucessedFlg(true);
		pAppWork->SetISSchedFlg(true);
		return;
	}

	/************************************************************************/
	//	��ÿ�����Դ���1
	/************************************************************************/
	list<WorkResInfo> workResInfoList;
	CResourceMgr resourceMgr(Dir_Obverse);
	if (pData->GetSchMaintainFlg() && !BMaintainWork(curWorkId))
	{
		WorkResInfo workResForMT;
		workPtr.GetPlanResRelaCom(workResForMT.resRelaID, workResForMT.assistResRelaList, true);
		if(workResForMT.resRelaID != NULL)
			workResInfoList.push_back(workResForMT);
	}
	else
	{
		resourceMgr.GetResComb(workPtr,workResInfoList);
	}
	//����ԭ�ƻ�,�̶�������ȡ��Դ����߼�֮ǰ����������ƻ�
	workPtr->ClearSchPlan();

	vector<TestSchedulerInfo> workPlanList;
	CSingleObvCalcV3* pCalc = CSingleObvCalcV3::Get();
	if (workResInfoList.empty() )
	{
		pCalc->VurtualResScheduler(curWorkId,Dir_Obverse);
	
		pData->uniteWorkDao.Start();
		pAppWork->SetISSchedFlg(true);
		pAppWork->SetISSucessedFlg(true);
		return;
	}

	//���� 
#ifndef PMCMODE
	if(!pCalc->WorkComb(workPtr,workResInfoList,leftStartTm))
#endif
	{	
		//����������Դ���
		SBH_BOOL succeededFlg = false;
		vector<SchdulerFlg> schFlgList;
		schFlgList.reserve(workResInfoList.size());
		list<WorkResInfo>::iterator i_workResInfo = workResInfoList.begin();
		while (i_workResInfo != workResInfoList.end())
		{
			WorkResInfo& workResInfo = *i_workResInfo;
			SchdulerFlg schFlg;
			CSplitWorkMgr splitMgr;
			pData->schTools.Init();
			//��������ʱ�䳤��
			SBH_BOOL curSplitFlg  = splitMgr.SplitWork(workPtr,workResInfo);
			if (curSplitFlg)
			{
				pData->schTools.m_splitWorkAmout = splitMgr.GetSplitWorkAmount();
				schFlg.workPlanAmount = pData->schTools.m_splitWorkAmout;
			}
			else if (bSplitedWork) //��ֳ����Ĺ��������һ������������������
			{
				schFlg.workPlanAmount = TOOLS::CeilDouble(workPtr->GetProcessAmount(),workResInfo.resRelaID->GetPdtBatchUnit());
			}
			pCalc->GetPrdTm(workPtr,workResInfo,pData->schTools.m_prdTimeLen
				,schFlg.workPlanAmount);

			//�ų̼���
			if(!Scheduler( workPtr, leftStartTm, 0, workResInfo ))
			{
				//��¼������Դ����ϵ��ų̼ƻ�
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
		if (succeededFlg &&!workPlanList.empty()&&workPlanList.size() == schFlgList.size())
		{
			SchdulerFlg bestSchFlg;
			TestSchedulerInfo workPlanInfo;
			//��Դ����
			resourceMgr.ResEvaluate(workPtr,workResInfoList,schFlgList,workPlanList
				,bestSchFlg,workPlanInfo,Dir_Obverse);

			SBH_INT checkWorkId = workPtr->GetId();
			if (!workPtr->GetSchPlanList().empty())
				AMAPI::ClearWorkPlan(checkWorkId);

			//��ֲ�����һ���������
			if (!(bestSchFlg.bSplited && bestSchFlg.workPlanAmount >= 0.000001f)
				&& !bSplitedWork)
			{
				SBH_TIME pdSt,pdEt;
				workPlanInfo.GetMinMaxTime(checkWorkId,0,pdSt,pdEt);
				if (!pData->uniteWorkDao.CheckSch_First(checkWorkId,workPlanInfo,pdSt,pdEt,Dir_Obverse))
				{
					pAppWork->SetISSucessedFlg(false);
					return;
				}
				else if (!pData->uniteWorkDao.CheckSch_Seq(checkWorkId,workPlanInfo,pdSt,pdEt,Dir_Obverse))
				{
					pAppWork->SetISSucessedFlg(false);
					return;
				}
			}
			
			//�Ѽƻ��������ݼ�����Դ�������ų̼ƻ���
			if (bestSchFlg.bSplited && bestSchFlg.workPlanAmount >= 0.000001f)
			{
				//ʵ���
				CSplitWorkMgr splitWorkMgr;
				if (splitWorkMgr.SplitWork(workPtr,bestSchFlg.workPlanAmount))
				{
					if (splitWorkMgr.GetSplitedWorkId1() > 0)
					{
						checkWorkId = splitWorkMgr.GetSplitedWorkId1();						
						
						workPlanInfo.FinallySurePlan(workPtr->GetId(),
							splitWorkMgr.GetSplitedWorkId1());
						splitedWorkId = splitWorkMgr.GetSplitedWorkId2();

						//�滻����Id
						if(pData->uniteWorkDao.ChangeLastWork(workPtr->GetId(),checkWorkId))
						{
							if (splitedWorkId > 0)
								pData->uniteWorkDao.AddWork(splitedWorkId);
						}
						
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
					//���һ����ֹ���β�����������޸ģ�
					//workPtr->SetProcessAmount(bestSchFlg.workPlanAmount);
				}
				workPlanInfo.FinallySurePlan();
				pAppWork->SetISSucessedFlg(true);
			}

			//��ȡ�仯
			pData->uniteWorkDao.GetWorkCondition(checkWorkId);
		}
		else
		{
			CSingleObvCalcV3::Get()->AllResFailed(curWorkId,workResInfoList,Dir_Obverse);
			pAppWork->SetISSucessedFlg(false);
		}
	}
#ifndef PMCMODE
	else
	{
		workPlanList.push_back(pData->schTools.m_lastSucessTestSchInfo);
	}
#endif
	//����appendwork���ų̱��Ϊ2���������ţ�
	pAppWork->SetISSchedFlg(true);

	if(pAppWork->GetISSucessedFlg() && splitedWorkId == 0 && pData->workFilter.ExistInMaxIntervalGp(curWorkId)/*g_MaxValRegister.BExist(curWorkId)*/)
	{
		g_MaxValRegister.InsertWorkInfo(curWorkId,workResInfoList,workPlanList);
	}
}

#include "clock.h"
#include "Tools.h"

bool SingleObvCalc::Scheduler(WorkPtr& workPtr, time_t leftStartTm, time_t rightCutoffTm,	WorkResInfo& workResInfo )
{
	bool bOK = true;
	if(AMAPI::GetParameter()->GetSystemSchFlg() != Refined_Sch)
		leftStartTm = ScheduleHelper::GetGlancingStartTime(leftStartTm,workResInfo.resRelaID->GetResId());

	AppendWork* pAppWork = pData->schTools.GetCurAppWork();//appendWorkDao.Get(workPtr->GetId());

	if(pAppWork->GetSchLevel() > SL_Level2)
	{
		SBH_TIME leftLimitTm(0);
		SBH_TIME rightTime(LLONG_MAX);

		if(AMAPI::GetParameter()->GetSystemSchFlg() == Refined_Sch)
		{
			CWorkBoundV3 workBound;	
			workBound.CalWorkBound(workPtr,workResInfo,leftLimitTm,rightTime,Dir_Obverse);
		}
		else
		{
			CWorkBoundGlancing workBound;
			workBound.CalWorkBound(workPtr,workResInfo,leftLimitTm,rightTime,Dir_Obverse);
		}


		leftStartTm = max(leftLimitTm,leftStartTm);
#ifdef WORKSEQUENCEOUTPUT
		if(workPtr->GetId() == 3470690)
		{
			char buf[1024];
			sprintf_s(buf,"workId:%d  resourceID:%d LeftBound: %S  RightBound: %S ",
				workPtr->GetId(),workResInfo.resRelaID->GetResId(),
				TOOLS::FormatTime(CDealTime(leftStartTm)).c_str()
				,TOOLS::FormatTime(CDealTime(rightCutoffTm)).c_str());
			TimeLog::WriteLog(buf);
		}
#endif
	}

	pData->schTools.m_limitTime = rightCutoffTm;
	

	SBH_TIME newTime = pData->uniteWorkDao.GetNewLimits(workPtr->GetId());
	if (newTime > 0)
	{
		leftStartTm = max(newTime,leftStartTm);
	}
#ifdef WORKSEQUENCEOUTPUT
	char buf[1024];
	sprintf_s(buf,"workId:%d  resourceID:%d LeftBound: %S  RightBound: %S ",
		workPtr->GetId(),workResInfo.resRelaID->GetResId(),
		TOOLS::FormatTime(CDealTime(leftStartTm)).c_str()
		,TOOLS::FormatTime(CDealTime(rightCutoffTm)).c_str());
	TimeLog::WriteLog(buf);
#endif

	SingleCalcV3* pCalc = NULL;

	if(AMAPI::GetParameter()->GetSystemSchFlg() == Refined_Sch)
	{
		if(AMAPI::GetSchedulerType() == 2 && pAppWork->GetSchLevel() > SL_Level2)
			pCalc = CSingleObvCalcSplit::Get();
		else if(AMAPI::GetSchedulerType() == 1)
			pCalc = CSingleObvCalcV3::Get();
	}
	else
	{
		pCalc = CSingleObvShiftCalc::Get();
	}

	CLOCK_MARK(T_CalcScheduling)
	if(pCalc->Schedule(workPtr,leftStartTm,workResInfo,TYPE_PRODUCETM))
	{
		pData->schTools.MeragePlan();
		bOK = false;
	}
	CLOCK_MARK(T_CalcScheduling)
	return bOK;
}

SBH_BOOL SingleObvCalc::SingleObvSchWKPair(SBH_INT nWKPre, SBH_INT nWKNex, time_t leftBound, SBH_BOOL bFixNexWK /* = false */)
{
	WorkPtr wkPre = AMAPI::GetWorkPtrMap()->Get(nWKPre);
	WorkPtr wkNex = AMAPI::GetWorkPtrMap()->Get(nWKNex);
	AppendWork* pAppWorkPre = pData->GetAppendWork(nWKPre);
	AppendWork* pAppWorkNex = pData->GetAppendWork(nWKNex);
	assert(wkPre.ValidPointer() && wkNex.ValidPointer() && pAppWorkPre && pAppWorkNex);

	time_t leftStartTm = leftBound;								//	��ʼʱ��
	time_t rightCutoffTm = LLONG_MAX;							//	�ҽ���ʱ��
	SBH_TIME tmNexOri(0);

	//ȡ�����������ƻ���������㿪ʼ��	
	WorkResInfo workResInfo;
	wkPre.GetPlanResRelaCom(workResInfo.resRelaID, workResInfo.assistResRelaList);
	AMAPI::GetMinStartTime(wkNex, WorkPlan_Produce, tmNexOri);

	wkPre.BackupWorkPlan();
	wkNex.BackupWorkPlan();
	pData->RetractWorkPlan(nWKPre,true);
	pData->RetractWorkPlan(nWKNex,true);

	vector<pair<WorkResInfo, WorkResInfo> > vecWorkRes;
	vector<pair<TestSchedulerInfo, TestSchedulerInfo> > vecTestSchPlan;
	list<WorkResInfo> listResInfoPre, listResInfoNex;
	CResourceMgr resourceMgr(Dir_Obverse),resourceMgrNxt(Dir_Obverse);
	resourceMgr.GetResComb(wkPre, listResInfoPre);
	AMAPI::ClearWorkPlan(nWKPre);

	CSingleObvCalcV3* pCalc = CSingleObvCalcV3::Get();

	for (list<WorkResInfo>::iterator iter1 = listResInfoPre.begin(); 
		iter1 != listResInfoPre.end(); ++iter1)
	{
		TestSchedulerInfo preTestSchInfo;
		pData->schTools.SetCurAppWork(pAppWorkPre);
		pData->schTools.Init();
		pCalc->GetPrdTm(wkPre, *iter1, pData->schTools.m_prdTimeLen);
		time_t tempTm = ((*iter1) == workResInfo) ? leftBound : 0;
		//�ų̼���
		if(!Scheduler(wkPre, tempTm, 0, *iter1))
		{
			preTestSchInfo = pData->schTools.m_lastSucessTestSchInfo;
			pData->schTools.m_lastSucessTestSchInfo.ClearPlan();
			preTestSchInfo.FinallySurePlan();
			pAppWorkPre->SetISSucessedFlg(true);
			pAppWorkPre->SetISSchedFlg(true);

			list<Resource> lockRes;
			if (iter1->resRelaID.GetRes().GetResLckType() == 3 || iter1->resRelaID.GetRes().GetResLckType() == 4)
			{
				lockRes.push_back(iter1->resRelaID.GetRes());
			}

			for (list<WorkResRelaPtr>::iterator iterWRR = iter1->assistResRelaList.begin();
				iterWRR != iter1->assistResRelaList.end(); ++iterWRR)
			{
				if (iterWRR->GetRes().GetResLckType() == 3 || iterWRR->GetRes().GetResLckType() == 4)
				{
					lockRes.push_back(iterWRR->GetRes());
				}
			}

			SBH_TIME tmBeg(0), tmEnd(0);
			AMAPI::GetMaxEndTime(wkPre, WorkPlan_Produce, tmBeg);

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
				tempTm = (bFixNexWK /*&& (*iter2)==nxtWorkResInfo*/) ? tmNexOri : 0;
				if (!Scheduler(wkNex, tempTm, 0, *iter2))
				{
					nexTestSchInfo = pData->schTools.m_lastSucessTestSchInfo;
					pData->schTools.m_lastSucessTestSchInfo.ClearPlan();
					nexTestSchInfo.FinallySurePlan();
					pAppWorkNex->SetISSucessedFlg(true);
					pAppWorkNex->SetISSchedFlg(true);

					bool bAdjusted = false;
					if (!lockRes.empty())
					{
						AMAPI::GetMinStartTime(wkNex, WorkPlan_Produce, tmEnd);
						SBH_TIME newSt(0), tempSt(0);
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
								if (false == AMAPI::GetMaxEndTime(AMAPI::GetWorkPtrMap()->Get(*iterWorkList), workPlan_NextSet, tempSt))
									AMAPI::GetMaxEndTime(AMAPI::GetWorkPtrMap()->Get(*iterWorkList), WorkPlan_Produce, tempSt);
								newSt = max(tempSt, newSt);
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
							SBH_TIME rtStTm(0), rtEndTm(0);
							TestSchedulerInfo tsInfo;
							singleCal.CalWorkTime(wkPre, *iter1, TYPE_PRODUCETM, newSt
								,tmLen, tsInfo, rtStTm, rtEndTm, FUC_F2, Dir_Converse, false, false);
							if (!Scheduler(wkPre, rtStTm, 0, *iter1))
							{
								TestSchedulerInfo newPreTSInfo = pData->schTools.m_lastSucessTestSchInfo;
								pData->schTools.m_lastSucessTestSchInfo.ClearPlan();
								newPreTSInfo.SurePlan();
								pAppWorkPre->SetISSucessedFlg(true);
								pAppWorkPre->SetISSchedFlg(true);

								pData->schTools.SetCurAppWork(pAppWorkNex);
								pData->schTools.Init();
								pCalc->GetPrdTm(wkNex, *iter2, pData->schTools.m_prdTimeLen);
								rtStTm = bFixNexWK ? tmNexOri : rtStTm;
								if (!Scheduler(wkNex, rtStTm, 0, *iter2))
								{
									nexTestSchInfo = pData->schTools.m_lastSucessTestSchInfo;
									pData->schTools.m_lastSucessTestSchInfo.ClearPlan();
									//nexTestSchInfo.SurePlan();
									vecTestSchPlan.push_back(make_pair(newPreTSInfo, nexTestSchInfo));
									vecWorkRes.push_back(make_pair(*iter1, *iter2));
									bAdjusted = true;
								}
								AMAPI::ClearWorkPlan(nWKPre);
								pAppWorkPre->SetISSucessedFlg(false);
								pAppWorkPre->SetISSchedFlg(false);
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
		,bestSchFlg, preWorkPlan, Dir_Obverse);

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
			,nexSchFlg, nexWorkPlan, Dir_Obverse);

		if (!nexWorkPlan.Empty())
		{
			preWorkPlan.FinallySurePlan();
			nexWorkPlan.FinallySurePlan();
			pAppWorkPre->SetISSucessedFlg(true);
			pAppWorkPre->SetISSchedFlg(true);
			pAppWorkNex->SetISSucessedFlg(true);
			pAppWorkNex->SetISSchedFlg(true);

			if (bFixNexWK)	// �����������ų�ʱ��㣬������ܲ�����
			{
				WorkResInfo newWRInfo;
				wkPre.GetPlanResRelaCom(newWRInfo.resRelaID, newWRInfo.assistResRelaList);
				if (!(newWRInfo == workResInfo))	// ǰ��������Դ����������Դ�̶�������
				{
					SBH_TIME tmPreWKEnd(0);
					AMAPI::GetMaxEndTime(wkPre, WorkPlan_Produce, tmPreWKEnd);
					wkNex.BackupWorkPlan();
					pData->RetractWorkPlan(nWKNex, true);
					const bool fixFlg = wkNex.GetResFix();
					wkNex.SetIsFixRes(true);
					SingleObvSchedule(nWKNex, tmPreWKEnd);
					wkNex.SetIsFixRes(fixFlg);
				}
			}
			return true;
		}
	}

	return false;
}