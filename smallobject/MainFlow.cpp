#include "stdafx.h"
#include "MainFlow.h"
#include "SchAssistantFuncs.h"
#include "SingleConvCalcV3.h"
#include "SingleObvCalcV3.h"
#include "Scheduler.h"
#include "SBHAPI.h"
#include "DisobeyRestrict.h"
#include "ResourceMgr.h"
#include "..\LanguagePack\LanguagePack.h"
#include <algorithm>

//#ifdef RELEASETESTOUTPUT
#include "clock.h"
#include "Tools.h"
#include "DataApi.h"
//#endif
#include "MaxIntervalDisposeManager.h"
#include "LockedDispose.h"
#include "SingleConvCalc.h"

#include "WorkBoundV3.h"

#include "PriorityMgr.h"
#include "SingleObvCalc.h"
#include "..\YKBizModel\ForAlgTemp.h"
#include "..\LanguagePack\LanguagePack.h"

#include "CGraphSort.h"

#ifdef _DEBUG_USING_CLOCK
#include "clock.h"
#include "Tools.h"
#define CLOCK_MARK(timer) timer.Pause();
#else
#define CLOCK_MARK(timer) 
#endif

#define UM_SCHUPDATECURVIEW	WM_USER+20007

CMainFlow::CMainFlow(void)
{
	CSingleObvCalcV3::Initialize();
	CSingleConvCalcV3::Initialize();
	CDisobeyRestrictDirctionMgr::Initialize();
}

CMainFlow::~CMainFlow(void)
{
	CSingleObvCalcV3::Release();
	CSingleConvCalcV3::Release();
	CDisobeyRestrictDirctionMgr::Release();
}

SBH_BOOL CMainFlow::Schedule(HWND pMainWnd /* = NULL */)
{
	//1、数据预处理
	Pretreatment();

	//2、工作筛选
	SchObjectFilter();

	//3、确定工作分派方向
	EnsureDirection();

	//4、排程计算
	Scheduling(pMainWnd);

	//5、违反约束处理
	//delete by yp 2011.1.31	
	DisobeyRestrictTreatment();

#ifdef WORKSEQUENCEOUTPUT
	TimeLog::WriteLog("\n");
#endif

	pData->newRegulation.SetLastWorkId(0);
	return true;
}

//1、数据预处理
void CMainFlow::Pretreatment()
{
	if (AMAPI::GetSchLevel() == SL_Level3)
	{
		//新启发式规则
		vector<unsigned int> rule;
		vector<wstring> vecWKParam;
		AMAPI::GetHeristicRuleList(rule, vecWKParam);
		pData->newRegulation.Init(rule, vecWKParam);
	}

	pData->workFilter.Clear();

	//2、算法参数初始化
	int time = 1800;
	time = AMAPI::GetMaxTm();
	SetParameSBH( 1, 1, time);
	//规则初始化
	AMAPI::ResetResSelectRuleMap();

	CWorkBoundV3 wb;
	wb.ResetMarginAmount();
	//6、资源能力初始化
	if(AMAPI::GetParameter()->GetInitCapacityFlg())
	{
		//4、普通工作信息表初始化
		//清除算法模型工作临时表
		WorkPtrMap* pWorkMap = AMAPI::GetWorkPtrMap();
		pWorkMap->Clear();

		//5、工作排程范围初始化
		pData->appendWorkDao.Init();

		if (!pData->mgRemainCapacityDao.IsEmpty())
		{
			pData->mgRemainCapacityDao.Clear();
		}
		pData->mgRemainCapacityDao.IniResCapacityDao();

		AMAPI::GetParameter()->SetInitCapacityFlg(false);

		//只有第一级排程或者全排,才再次初始化
		CPriorityMgr priMgr;
		priMgr.CreatePriority();
	}
	else
	{
		AMAPI::GetWorkPtrMap()->Clear();
		pData->appendWorkDao.ClearWorkPoint();
		pData->appendWorkDao.InitParticipateSch();
	}
}

//2、工作筛选
void CMainFlow::SchObjectFilter()
{
	SchLevelType sl;
	switch(AMAPI::GetSchLevel())
	{
	case 0:
		sl = SL_ALL;break;
	case 1:
		sl = SL_Level0;break;
	case 2:
		sl = SL_Level1;break;
	case 3:
		sl = SL_Level2;break;
	case 4:
		sl = SL_Level3;break;
	case 5:
		sl = SL_Level4;break;

	}
	pData->workFilter.Filter(sl);
}

//3、确定工作分派方向
void CMainFlow::EnsureDirection()
{
	pData->workFilter.EnsureDirection();
}

//4、排程计算
void CMainFlow::Scheduling(HWND pMainWnd /* = NULL */)
{
	//计算静态匹配程度
	pData->parame.m_bMoveWork = false;
	pData->appendWorkDao.ResetCaculateWorkBoundFlg();
	int i = 0;
	int startPose = *(pData->GetSBHProcess());
	ScheduleMethod GlobalSm = AMAPI::GetLimitSchFlg() ? SM_LIMITARY : SM_INFINITY;

	//选择排程工作
	while(1)
	{
		//选择工作
		list<SBH_INT>	schWorkList;
		if (!pData->workFilter.GetNextSchWork(schWorkList))
		{
			CScheduler sched;
			sched.DelWithDisobeyRestict_LV2();
			return;
		}
		pData->uniteWorkDao.Start();

		list<SBH_INT>::iterator iSchWork = schWorkList.begin();
		for (;iSchWork != schWorkList.end();)
		{
			CScheduler scheduling;
#ifdef WORKSEQUENCEOUTPUT
			char buf[102];
			sprintf(buf,"%d ",*iSchWork);
			TimeLog::WriteLog(buf);
#endif

			////对有最大间隔工作的处理
			//CLOCK_MARK(mainFlow_Scheduling3)
			//if(!g_MaxValRegister.BExist(*iSchWork))
			//{
			//	set<SBH_INT> maxvalGroup;
			//	AMAPI::GetMaxvalGroup(*iSchWork,maxvalGroup);
			//	if(maxvalGroup.size() > 1)
			//	{
			//		g_MaxValRegister.InitGather(maxvalGroup);
			//	}
			//	g_MaxValRegister.SetSchedFlg(*iSchWork,true);
			//}
			//else
			//{
			//	g_MaxValRegister.SetSchedFlg(*iSchWork,true);
			//}
			//CLOCK_MARK(mainFlow_Scheduling3)
			const SBH_INT nSchWorkId = *iSchWork;
			AMAPI::SetWorkSequence(i,*iSchWork);
			//普通排程才仅处理流程
			pData->parame.m_curSchMethod = GlobalSm;
			if(scheduling.Scheduling(*iSchWork))
			{
				if (pData->uniteWorkDao.IsUniteSchFailed())
				{
					if (scheduling.UnitWorkSchFailed(schWorkList))
					{
						break;
					}
				}
				//一级处理
				if(scheduling.DelWithDisobeyRestict_LV1(*iSchWork))
				{
					////切换正方向处理
					//if (scheduling.ChangeDirection(schWorkList))
					//{
					//	i++;
					//	AMAPI::SetWorkSequence(i,*iSchWork);
					//	pData->SetSBHProcess(i*100/pData->appendWorkDao.GetCount());
					//	break;
					//}
				}
			}

			i++;
			pData->SetSBHProcess(startPose + int((float)100*i/pData->appendWorkDao.GetCount()));

			if (pData->BShiftDown())
			{
				if (pData->GetOtherKey() == 0)
				{
					pData->SetFlashStata(true);
				}

				BIZAPI::LeaveDataCriSel();	// 解锁刷新界面
				::PostMessage(pMainWnd, UM_SCHUPDATECURVIEW, 0, 0);
				while(pData->BShiftDown() && pData->GetOtherKey() == 0)
				{
					Sleep(100);//等待0.1s
				}
				BIZAPI::EnterDataCriSel(INFINITE);

				//空格
				if (pData->GetOtherKey() == 1)
				{
					pData->ClearOtherKey();
				}
			}

			++iSchWork;
		}

		//最大间隔约束处理
		if (!pData->workFilter.GetMaxIntervalGp().empty())
		{
			const SBH_INT nWorkId = (*(pData->workFilter.GetMaxIntervalGp().begin()))[0];
			AppendWork* pComWork = pData->GetAppendWork(nWorkId);
			if(pComWork != NULL)
			{
				g_MaxValRegister.SetSchedGroup2Check();
				g_MaxValRegister.ProMaxInterval();
// 				CMaxIntervalDisposeManager* pMDM = CMaxIntervalDisposeManager::GetMaxDisManager();
// 				DirType dirType = Dir_Obverse;
// 				switch (AMAPI::GetDirectionFlg())
// 				{
// 				case Sch_Dir_Force_Obv:
// 					break;
// 				case Sch_Dir_Force_Conv:
// 					dirType = Dir_Converse;
// 					break;
// 				}
// 				pMDM->SetDirType(dirType);
// 				//pMDM->MaxIntervalCheck();
// 				pMDM->ProMaxInterval();
				pData->workFilter.ClearMaxIntervalGp();
			}
			g_MaxValRegister.ClearMaxValRegister();
		}


		// 锁定处理
		if (!pData->workFilter.GetLockedGp().empty())
		{
			CLockedDispose lockDis;
			lockDis.ResLockedPro();
			pData->workFilter.ClearLockedGp();
		}
	}
}

//5、违反约束处理
void CMainFlow::DisobeyRestrictTreatment()
{

	//ScheduleHelper::UpdateIOTakeTm();
	CWorkBoundV3 workBound;
	workBound.UpdateTimeBoundV3();
	workBound.UpadteMarginAmount();
	pData->appendWorkDao.OutputBound();
}

void CMainFlow::SetParameSBH(int excessearlymode, /* 超过最早处理 绞?*/ 
							 int excesslatemode, /* 超过最晚处理 绞?*/ 
							 int maxruntime /* 最长运行时间 */ )
{
	//WorkPtrMap* pmwork = AMAPI::GetWorkPtrMap();
	//WorkPtr ptrwork;
	pData->parame.machinegroupnumber = 1;
	//p_datamanager->parame.bottleneckinterval = intervaltime * 60;
	pData->parame.excessearlystarttimemode = excessearlymode;
	pData->parame.excesslateststarttimemode = excesslatemode;
	//list<WorkPtr> worklist;
	////根据工作表，查找“排程标志=1，且工作类型=1或2”的待排工作ID列表
	//AMAPI::GetCommonWorks(worklist);
	pData->parame.maxRunTime = maxruntime;
	//SBH最大循环次数＝排程最长时间×（2/3）\（待排工作总数×1秒\ 10＋1）＋1；
	//最长运行时间
	pData->parame.sbhMaxLoopNumber = 10;
	pData->parame.sbhMaxStep = 2;

	//SBH最大运行时间＝排程最长时间×（2/3）；
	pData->parame.sbhMaxRunTime = int(maxruntime * (2.0 / 3.0));

}

SBH_BOOL CMainFlow::ScheduleMaintain(HWND pMainWnd)
{
	SBH_BOOL bOK = false;
	WorkPtrMap* pWorkMap = AMAPI::GetWorkPtrMap();
	if (pWorkMap == NULL)
		return bOK;

	ResourcePtrMap* pResPtrMap = AMAPI::GetResourcePtrMap();
	if (pResPtrMap == NULL)
		return bOK;
	//将所有维护工作的计划及占用资源能力清除
	for(pWorkMap->Begin();pWorkMap->NotEnd();pWorkMap->Step())
	{
		WorkPtr& workPtr = pWorkMap->Get();
		if(BMaintainWork(workPtr->GetId()))
		{
			if(workPtr->IsNonSchWorkD() || workPtr->IsNonSchWorkE()
				||workPtr->IsSchWorkD())
			pData->RetractWorkPlan(workPtr->GetId());
		}
	}

	//备份工作计划
	pWorkMap->BackupWorkPlan();

	list<SBH_INT> workSeq;
	list<pair<SBH_INT,SBH_INT>> lerr;
	bool rt = GetWorkSequence(workSeq, lerr);

	set<unsigned long>& schWorkList = AMAPI::GetAlgWorkSchList();
	for(list<SBH_INT>::iterator iterSeq = workSeq.begin();iterSeq != workSeq.end();)
	{
		if(schWorkList.find(*iterSeq) == schWorkList.end())
		{
			iterSeq = workSeq.erase(iterSeq);
		}
		else
			++iterSeq;
	}

	for (pResPtrMap->Begin(); pResPtrMap->NotEnd(); pResPtrMap->Step())
	{
		Resource resPtr = pResPtrMap->Get();
		if (resPtr.ValidPointer())
		{
			list<SBH_INT> listWorks,finalListWorks;
			const SBH_INT nResId = resPtr.GetId();
			pData->mgRemainCapacityDao.GetResWorkList(resPtr.GetId(), listWorks);
			for (list<pair<SBH_INT,SBH_INT>>::iterator iterErr = lerr.begin();
				iterErr != lerr.end(); ++iterErr)
			{
				list<SBH_INT>::iterator iterFErr = find(listWorks.begin(), listWorks.end(), iterErr->first);
				list<SBH_INT>::iterator iterSErr = find(listWorks.begin(), listWorks.end(), iterErr->second);
				if (iterFErr != listWorks.end() && iterSErr != listWorks.end())
				{
					if (distance(listWorks.begin(), iterFErr) < distance(listWorks.begin(), iterSErr))
					{
						iter_swap(iterFErr, iterSErr);
					}
				}
			}
			for(list<SBH_INT>::iterator iter = listWorks.begin();iter != listWorks.end(); ++iter)
			{
				const SBH_INT nhtWorkId = *iter;
				WorkPtr& workPtr = pWorkMap->Get(*iter);
				if (!workPtr.ValidPointer())
					continue;

				CIPWorkStatus workStatus;
				workStatus.nId = *iter;
				if (workPtr->IsNonSchWorkA() || workPtr->IsNonSchWorkB() || workPtr->IsNonSchWorkC()
					||workPtr->IsSchWorkA() || workPtr->IsSchWorkB() || workPtr->IsSchWorkC())
				{
					workStatus.bFixed = true;
				}

				m_listResMaintainStatus[nResId].m_listWorks.push_back(workStatus);
			}


			map<SBH_INT, ResMaintainStatus>::iterator iter = m_listResMaintainStatus.find(nResId);
			if (iter != m_listResMaintainStatus.end() && !m_listResMaintainStatus[nResId].m_listWorks.empty())
				m_listResMaintainStatus[nResId].workSchBeg = m_listResMaintainStatus[nResId].m_listWorks.front().nId;
		}
	}
	// 收集所有工作排程信息，清除资源能力
	pWorkMap->Clear();

	//5、工作排程范围初始化
	pData->appendWorkDao.Init();
	pData->mgRemainCapacityDao.Clear();
	pData->mgRemainCapacityDao.IniResCapacityDao();
	AMAPI::GetParameter()->SetInitCapacityFlg(false);

	//只有第一级排程或者全排,才再次初始化
	CPriorityMgr priMgr;
	priMgr.CreatePriority();

	const int nCurLevel = AMAPI::GetSchLevel();
	AMAPI::SetSchLevel(SL_Level1);
	Schedule();
	AMAPI::SetSchLevel(SL_Level2);
	Schedule();
	AMAPI::SetSchLevel(nCurLevel);
 	
	int i = 0;
	int startPose = *(pData->GetSBHProcess());
	pData->EnableSchMaintain();

	ScheduleMethod backUp = pData->parame.m_curSchMethod;
	pData->parame.m_curSchMethod = SM_IGNORE;
	for (list<SBH_INT>::iterator iterWK = workSeq.begin(); iterWK != workSeq.end();)
	{
		const SBH_INT nCurWorkId = *iterWK;
		assert(FitPrepareSchWorks(nCurWorkId));
		//除去维护工作
		if(BMaintainWork(nCurWorkId))
		{
			iterWK = workSeq.erase(iterWK);
			continue;
		}

		WorkPtr curWork = pWorkMap->Get(nCurWorkId);
		assert(curWork != NULL);
		if(curWork == NULL) continue;

		WorkResInfo workResInfo;
		curWork.GetPlanResRelaCom(workResInfo.resRelaID, workResInfo.assistResRelaList, true);
		assert(workResInfo.resRelaID.ValidPointer());

		int nResId(0);
		if(workResInfo.resRelaID.ValidPointer())
			nResId = workResInfo.resRelaID.GetResId();
#ifdef TESTWORKSEQ
		{
			char buf[100];
			sprintf_s(buf,"%d-%d ",nResId,nCurWorkId);
			TimeLog::WriteLog(buf);
		}
#endif

		//工作状态为普通工作，进入排程
		CSchPrepareCollection schColl;
		if (curWork.GetSchedulerFlg() < SCHEDULEREDFINISH)
		{
			SBH_TIME tmCurWKBeg(0), tmPreResWKEnd(0);
			AMAPI::GetMinStartTime(curWork, WorkPlan_Produce, tmCurWKBeg);

			list<SBH_INT> listResPreWK;
			pData->mgRemainCapacityDao.GetResWorkList(nResId, listResPreWK, 0, tmCurWKBeg-1);
			if (!listResPreWK.empty())
			{
				const int nWKTemp = *listResPreWK.rbegin();
				WorkPtr wkTemp = pWorkMap->Get(nWKTemp);
				assert(wkTemp.ValidPointer());
				AMAPI::GetMinStartTime(wkTemp, WorkPlan_Produce, tmPreResWKEnd);
			}

			tmCurWKBeg = max(tmCurWKBeg, tmPreResWKEnd);
			pData->GetAppendWork(nCurWorkId)->SetSchLevel(SL_Level3);

			ProLockedDispatch(curWork, nResId, &schColl);
			bool fixState = curWork.GetResFix();
			curWork.SetIsFixRes(true);
			SingleObvCalc singleObv;
			singleObv.SingleObvSchedule(nCurWorkId, tmCurWKBeg);
			curWork.SetIsFixRes(fixState);

			workResInfo.resRelaID = NULL;
			workResInfo.assistResRelaList.clear();
			curWork.GetPlanResRelaCom(workResInfo.resRelaID, workResInfo.assistResRelaList);
			if(!workResInfo.resRelaID.ValidPointer())
				continue;

			//////////////////////////////////////////////////////////////////////////
			map<SBH_INT,SBH_TIME> tmPreEnd,tmPreSt;
			if(RevocationTimes(curWork,tmPreEnd,tmPreSt))	//判断是否有打乱顺序的情况
			{

				pData->RetractWorkPlan(nCurWorkId, true);
				if(workResInfo.resRelaID != NULL)
				{
					RevocationWorks(workResInfo.resRelaID->GetResId(), tmPreEnd[workResInfo.resRelaID->GetResId()] + 1, 0, 0, nCurWorkId);
					for(list<WorkResRelaPtr>::iterator iterAss = workResInfo.assistResRelaList.begin();
						iterAss != workResInfo.assistResRelaList.end();++iterAss)
					{
						RevocationWorks(iterAss->GetResId(), tmPreEnd[iterAss->GetResId()] + 1, 0, 0, nCurWorkId);
					}
					//求排程开始时刻
					for(map<SBH_INT,SBH_TIME>::iterator iterMaxTm = tmPreSt.begin();iterMaxTm != tmPreSt.end();++iterMaxTm)
					{
						tmCurWKBeg = max(tmCurWKBeg,iterMaxTm->second);
					}

					bool fixState = curWork.GetResFix();
					curWork.SetIsFixRes(true);
					singleObv.SingleObvSchedule(nCurWorkId, tmCurWKBeg);
					curWork.SetIsFixRes(fixState);
				}
			}
			//////////////////////////////////////////////////////////////////////////
		}//第三级工作排程结束

		//维护处理开始
		CIPWorkStatus workStatus;
		workStatus.nId = nCurWorkId;
		if (curWork->IsNonSchWorkA() || curWork->IsNonSchWorkB() || curWork->IsNonSchWorkC()
			||curWork->IsSchWorkA() || curWork->IsSchWorkB() || curWork->IsSchWorkC())
		{
			workStatus.bFixed = true;
		}
		ProMaintainOrder(nResId, workStatus);
		for (list<WorkResRelaPtr>::iterator iterRes = workResInfo.assistResRelaList.begin();
			iterRes != workResInfo.assistResRelaList.end(); ++iterRes)
		{
			ProMaintainOrder(iterRes->GetResId(), workStatus);
		}

		SBH_TIME tmNewBeg(0),tmNewEnd(0);
		AMAPI::GetMaxEndTime(curWork, WorkPlan_Produce, tmNewEnd);
		if(tmNewEnd < AMAPI::GetSchEndTime())
		{
			//最大间隔
			if(AMAPI::GetParameter()->GetConsiderMaxVal())
				ProMaxIntval(curWork, nResId);
			if (!schColl.GetLockedGp().empty())
			{
				CLockedDispose lockDispatch;
				set<vector<SBH_INT> >& setLockGp = schColl.GetLockedGp();
				for (set<vector<SBH_INT> >::iterator iterLock = setLockGp.begin();
					iterLock != setLockGp.end(); ++iterLock)
				{
					lockDispatch.ResLockedPairPro(*iterLock);
				}
			}
		}

		if (false == m_listReSchWk.empty())
		{
			list<SBH_INT>::iterator iterNew = find_first_of(workSeq.begin(), workSeq.end(), m_listReSchWk.begin(), m_listReSchWk.end());
			if (distance(workSeq.begin(), iterNew) < distance(workSeq.begin(), iterWK))
				iterWK = iterNew;
			else
				++iterWK;
			m_listReSchWk.clear();
		}
		else
		{
			++iterWK;
		}

		i++;
		pData->SetSBHProcess(startPose + int((float)100*i/pData->appendWorkDao.GetCount()));

		if (pData->BShiftDown())
		{
			if (pData->GetOtherKey() == 0)
			{
				pData->SetFlashStata(true);
			}

			BIZAPI::LeaveDataCriSel();	// 解锁刷新界面
			::PostMessage(pMainWnd, UM_SCHUPDATECURVIEW, 0, 0);
			while(pData->BShiftDown() && pData->GetOtherKey() == 0)
			{
				Sleep(100);//等待0.1s
			}
			BIZAPI::EnterDataCriSel(INFINITE);

			//空格
			if (pData->GetOtherKey() == 1)
			{
				pData->ClearOtherKey();
			}
		}
	}

	pData->EnableSchMaintain(false);


	AfterLastWork();

	pData->parame.m_curSchMethod = backUp;
	DisobeyRestrictTreatment();
	ClearSpilthMaintainWork();
	m_listResMaintainStatus.clear();
	return bOK;
}

SBH_BOOL CMainFlow::ProMaintainOrder(SBH_INT nResId, CIPWorkStatus& workStatus)
{
	SBH_BOOL bOK = false;
	Resource resPtr = AMAPI::GetResourcePtrMap()->Get(nResId);
	WorkPtr pCurWork = AMAPI::GetWorkPtrMap()->Get(workStatus.nId);
	if (!resPtr.ValidPointer() || !pCurWork.ValidPointer())
		return bOK;

	SBH_TIME stTm(0), stInput(0);
	AMAPI::GetMinStartTime(pCurWork, WorkPlan_Produce, stTm);
	stTm = max(stTm,AMAPI::GetSchStartTime());
	const SBH_INT nPreWorkId = pData->mgRemainCapacityDao.GetNearByWork(nResId, stTm, Dir_Converse, pCurWork.GetId());

	WorkPtr pPreWork = AMAPI::GetWorkPtrMap()->Get(nPreWorkId);

	// 收集指定资源的全部维护信息
	list<SBH_INT> listMT;
	AM_CreateMTOrderParamPtrMap* pMTParamMap = AMAPI::GetCreateMTOrderParamPtrMap();
	for (pMTParamMap->Begin(); pMTParamMap->NotEnd(); pMTParamMap->Step())
	{
		AM_CreateMTOrderParam mtParam = pMTParamMap->Get();
		if (mtParam.ValidPointer() && mtParam.GetMTIsValid() && mtParam.GetResId() == nResId)
		{
			listMT.push_back(mtParam.GetId());
		}
	}

	if (listMT.empty())
		return bOK;
	listMT.sort(SortMaintainParam);

	bool bMTDir = false;	// 维护工作位置标志，true为工作后，false为工作前
	FindSuitMainMode findMode = CommWork;
	for (list<SBH_INT>::iterator iter = listMT.begin(); iter != listMT.end(); ++iter)
	{
		AM_CreateMTOrderParam mtParam = AMAPI::GetCreateMTOrderParamPtrMap()->Get(*iter);
		if (!mtParam.ValidPointer())
			continue;

		if ( pPreWork.ValidPointer() && mtParam.GetOpPosition() == GetRemarkFiled(ID_CREATEMT_OPPOSITION_2))
		{
			if (false == AMAPI::GetMaxEndTime(pPreWork, workPlan_NextSet, stInput,nResId))
				if (false == AMAPI::GetMaxEndTime(pPreWork, WorkPlan_Lock, stInput,nResId))
					AMAPI::GetMaxEndTime(pPreWork, WorkPlan_Produce, stInput,nResId);
			if(stInput < AMAPI::GetSchStartTime())
			{
				SBH_TIME curWorkStTm(0);
				if(AMAPI::GetMinStartTime(pCurWork, WorkPlan_Produce, curWorkStTm))
				{
					if(curWorkStTm > AMAPI::GetSchStartTime())
						stInput = AMAPI::GetSchStartTime();
				}

			}
			bMTDir = true;
		}
		else
		{
			if (false == AMAPI::GetMinStartTime(pCurWork, WorkPlan_PreSet, stInput,nResId))
				AMAPI::GetMinStartTime(pCurWork, WorkPlan_Produce, stInput,nResId);
			bMTDir = false;
		}

		// 找到一条符合条件的维护信息
		if (FindSuitMaintainInfo(nResId, *iter, stInput, pCurWork, pPreWork,findMode))
		{
			const int nCurId = pCurWork.GetId();
			SBH_INT nMTWorkId = GetMaintainWork(*iter,nResId);
			if(nMTWorkId == 0)
			{
				findMode = IdleMaintWork;
				continue;
			}

			WorkPtr mtWorkPtr = AMAPI::GetWorkPtrMap()->Get(nMTWorkId);
			if (!mtWorkPtr.ValidPointer())
			{
				findMode = IdleMaintWork;
				continue;
			}

			AppendWork* pAppWork = pData->appendWorkDao.Get(nMTWorkId);
			if(pAppWork == NULL)
			{
				AppendWork appWork;
				appWork.id = nMTWorkId;
				appWork.SetWorkPtr(mtWorkPtr);
				appWork.initEarlyStartTime = AMAPI::GetSchStartTime();
				appWork.initLatestEndTime = AMAPI::GetSchEndTime();
				appWork.earlieststarttime = AMAPI::GetSchStartTime();
				appWork.latestendtime = AMAPI::GetSchEndTime();
				appWork.SetSchLevel(SL_Level3);
				pData->appendWorkDao.Add(appWork);
			}
			else
			{
				pAppWork->SetSchLevel(SL_Level3);
			}

			bool fixState = pCurWork.GetResFix();
			if(!workStatus.bFixed)
			{
				pCurWork.SetIsFixRes(true);
			}

			list<WorkResInfo> workResInfoList;
			CResourceMgr resourceMgr(Dir_Obverse);
			resourceMgr.GetResComb(mtWorkPtr,workResInfoList);
			WorkResInfo workResInfo;
			pCurWork.GetPlanResRelaCom(workResInfo.resRelaID, workResInfo.assistResRelaList);
			if(workResInfoList.empty())
			{
				workResInfoList.push_back(workResInfo);
				workResInfoList.front().assistResRelaList.clear();
			}

			//判断时间点是否在排程范围内
			if(stInput >= AMAPI::GetSchStartTime() && stInput <= AMAPI::GetSchEndTime())
			{
				if (false == ScheduleWork(mtWorkPtr, nResId, stInput, bMTDir ? Dir_Obverse : Dir_Converse, stTm,pCurWork))
				{
					//SingleCalcV3 singleCal;

					//TimeLenInfo tmLen;
					//singleCal.GetPrdTm(mtWorkPtr, workResInfoList.front(), tmLen);

					//SBH_TIME rtStTm, rtEndTm;
					//TestSchedulerInfo tsInfo;
					////使用F1进行试排
					//bool bF1OK = singleCal.CalWorkTime(mtWorkPtr,  workResInfoList.front(), TYPE_PRODUCETM, stInput
					//	,tmLen, tsInfo, rtStTm, rtEndTm, FUC_F1, bMTDir ? Dir_Obverse : Dir_Converse);
					//if(bF1OK)	//试排成功
					//{
					//	mtWorkPtr.ClearSchPlan();
					//	tsInfo.SurePlan();
					//	//取计划结束查找维护与原计划之间是否有其他工作
					//	SBH_TIME mtLimitTm(0);
					//	if(bMTDir)	//正向
					//	{
					//		AMAPI::GetMinStartTime(mtWorkPtr,TYPE_PRODUCETM,mtLimitTm);
					//		SBH_TIME curStTm(0);
					//		AMAPI::GetMinStartTime(pCurWork,TYPE_PRODUCETM,curStTm);
					//		if(mtLimitTm >= curStTm)
					//		{
					//			pData->RetractWorkPlan(nMTWorkId);
					//			bF1OK = false;
					//		}
					//		else
					//		{
					//			list<SBH_INT> listWorks;
					//			pData->mgRemainCapacityDao.GetResWorkList(nResId, listWorks, stInput, mtLimitTm);
					//			listWorks.remove(workStatus.nId);
					//			listWorks.remove(nMTWorkId);
					//			if(!listWorks.empty())
					//			{
					//				//清除维护工作
					//				pData->RetractWorkPlan(nMTWorkId);
					//				bF1OK = false;
					//			}
					//		}

					//	}
					//	else
					//	{
					//		AMAPI::GetMaxEndTime(mtWorkPtr,TYPE_PRODUCETM,mtLimitTm);
					//		list<SBH_INT> listWorks;
					//		pData->mgRemainCapacityDao.GetResWorkList(nResId, listWorks,mtLimitTm, stInput);
					//		listWorks.remove(workStatus.nId);
					//		listWorks.remove(nMTWorkId);
					//		if(!listWorks.empty())
					//		{
					//			//清除维护工作
					//			pData->RetractWorkPlan(nMTWorkId);
					//			bF1OK = false;
					//		}
					//	}
					//}

					//if(!bF1OK)
					{
						if (false == workStatus.bFixed)
						{
							map<SBH_INT,SBH_TIME> tmPreEnd,tmPreSt;
							RevocationTimes(pCurWork,tmPreEnd,tmPreSt);	//判断是否有打乱顺序的情况,求取每个资源上的撤销时间点

							//这句是什么意思？
							//if (tmPreEnd.find(workResInfo.resRelaID->GetResId()) == tmPreEnd.end())
							//	return bOK;
							bOK = RevocationWorks(workResInfo.resRelaID->GetResId(), tmPreEnd[workResInfo.resRelaID->GetResId()]+1, 0, 0, pCurWork.GetId());
							for(list<WorkResRelaPtr>::iterator iterAss = workResInfo.assistResRelaList.begin();
								iterAss != workResInfo.assistResRelaList.end();++iterAss)
							{
								bOK &= RevocationWorks(iterAss->GetResId(), tmPreEnd[iterAss->GetResId()] + 1, 0, 0, pCurWork.GetId());
							}
							//pData->RetractWorkPlan(pCurWork->GetId());
						}

						SingleObvCalc singleObv;
						if (false == workStatus.bFixed)
						{
							if (false == AMAPI::GetMaxEndTime(pPreWork, workPlan_NextSet, stTm))
								if (false == AMAPI::GetMaxEndTime(pPreWork, WorkPlan_Lock, stTm))
									AMAPI::GetMaxEndTime(pPreWork, WorkPlan_Produce, stTm);

							stTm = max(stTm,AMAPI::GetSchStartTime());
							singleObv.SingleObvSchedule(mtWorkPtr.GetId(), stTm);
							AMAPI::GetMaxEndTime(mtWorkPtr, WorkPlan_Produce, stTm);
							singleObv.SingleObvSchedule(pCurWork.GetId(), stTm);
						}
						else
						{
							ScheduleMethod oldSchMethod = pData->parame.m_curSchMethod;
							pData->parame.m_curSchMethod = SM_FORCE;
							singleObv.SingleObvSchedule(mtWorkPtr.GetId(), stTm);
							pData->parame.m_curSchMethod = oldSchMethod;
						}
					}
					//else
					//{
					//	AppendWork* pAppWork = pData->appendWorkDao.Get(nMTWorkId);
					//	if(pAppWork != NULL)
					//	{
					//		pAppWork->SetISSchedFlg(true);
					//		pAppWork->SetISSucessedFlg(true);
					//	}
					//}


				}
			}

			if(!workStatus.bFixed)
			{
				pCurWork.SetIsFixRes(fixState);
			}

			findMode = IdleMaintWork;;
		}
	}

	return bOK;
}

SBH_BOOL CMainFlow::FindSuitMaintainInfo(SBH_INT nResId, SBH_INT nMTId, SBH_TIME stInput, WorkPtr& pCurWork, WorkPtr& pPreWork,FindSuitMainMode findMode )
{
	if(findMode != LastWork)
	{
		if (!pCurWork.ValidPointer())
			return false;
	}

	AM_CreateMTOrderParam mtParam = AMAPI::GetCreateMTOrderParamPtrMap()->Get(nMTId);
	if (!mtParam.ValidPointer())
		return false;

	list<UnitJudgeFun> listUnitJudge;
	AnalyMaintainStr(mtParam.GetCondition(), listUnitJudge);
	if (listUnitJudge.empty())
		return false;

	//如果是空闲，做排除逻辑
	if(findMode == IdleMaintWork)
	{
		bool bdleMaintWork = false;
		for (list<UnitJudgeFun>::iterator iter = listUnitJudge.begin();
			iter != listUnitJudge.end(); ++iter)
		{
			if(iter->nType == 3)
			{
				bdleMaintWork = true;
				break;
			}
		}
		if(!bdleMaintWork)
			return false;
	}

	//前工作为维护工作则不在做维护处理
	bool preMaintainWorkFlg = BMaintainWork(pPreWork->GetId());

	SBH_BOOL bOK = true, bFlgTemp = false;
	for (list<UnitJudgeFun>::iterator iter = listUnitJudge.begin();
		iter != listUnitJudge.end(); ++iter)
	{
		bFlgTemp = false;
		try
		{
			switch (iter->nType)
			{
			case 1:			// 是否指定时刻
				{
					SBH_TIME tmSpec = boost::any_cast<SBH_TIME>(iter->anyVal);
					if(findMode == CommWork )
					{
						if(CheckWorkCondition(pCurWork->GetId(),iter->bWorkCondition))
						{
							SBH_TIME stStart(0), stNeed(0);
							AMAPI::GetMinStartTime(pCurWork, WorkPlan_Produce, stStart);
							while (1)
							{
								SBH_INT nMTWorkId = pData->mgRemainCapacityDao.GetNearByWork(nResId, stStart, Dir_Converse, pCurWork.GetId());
								WorkPtr workMT = AMAPI::GetWorkPtrMap()->Get(nMTWorkId);
								if (!workMT.ValidPointer())
									break;

								if (BMaintainWork(nMTWorkId) && CheckWorkCondition(nMTWorkId,iter->wstrCondition))
								{
									AMAPI::GetMaxEndTime(workMT, WorkPlan_Produce, stNeed);
									break;
								}

								AMAPI::GetMinStartTime(workMT, WorkPlan_Produce, stStart);
							}
							// 指定时刻大于等于输入时刻
							if (tmSpec > stNeed)
								bFlgTemp = (stInput >= tmSpec);
						}
						else
							bFlgTemp = false;
					}
				}
				break;
			case 3:			// 空闲时间
				{
					if(findMode != LastWork)
					{
						if(CheckWorkCondition(pCurWork->GetId(),iter->bWorkCondition))
						{
							SBH_INT stSpace = boost::any_cast<SBH_INT>(iter->anyVal);

							//求取当前工作的前工作
							const SBH_INT nPreWorkId = pData->mgRemainCapacityDao.GetNearByWork(nResId, stInput, Dir_Converse, pCurWork.GetId());

							WorkPtr idlePreWork = AMAPI::GetWorkPtrMap()->Get(nPreWorkId);

							SBH_TIME stEnd(0);
							if (mtParam.GetOpPosition() == GetRemarkFiled(ID_CREATEMT_OPPOSITION_2))	// 工作后
							{
								//if (pPreWork.ValidPointer())
								//{
								//	if (false == AMAPI::GetMinStartTime(pCurWork, WorkPlan_PreSet, stEnd, nResId))
								//		AMAPI::GetMinStartTime(pCurWork, WorkPlan_Produce, stEnd, nResId);

								//	bFlgTemp = ((stEnd - stInput) > stSpace);
								//}
								//else
								//{
								//	bFlgTemp = ((stInput - 0) > stSpace);
								//}
								bFlgTemp = false;
							}
							else	// 相对位置为工作前
							{
								if (idlePreWork.ValidPointer())
								{
									if (false == AMAPI::GetMaxEndTime(idlePreWork, workPlan_NextSet, stEnd))
										AMAPI::GetMaxEndTime(idlePreWork, WorkPlan_Produce, stEnd);

									bFlgTemp = ((stInput - stEnd) > stSpace);
								}
								else
								{
									bFlgTemp = ((stInput - 0) > stSpace);
								}
							}
						}
						else
							bFlgTemp = false;
					}
				}
				break;
			case 2:			// 工作状态变化
				{
					//前工作为维护工作
					if(preMaintainWorkFlg || findMode == LastWork /*|| findMode == IdleMaintWork*/)
					{
						return false;
					}

					wstring changeCondition = boost::any_cast<wstring>(iter->anyVal);
					//找到前个对比工作
					SBH_TIME tmBeg(0);
					AMAPI::GetMinStartTime(pCurWork, WorkPlan_Produce, tmBeg);
					list<SBH_INT> preWorkList;
					pData->mgRemainCapacityDao.GetResWorkList(nResId,preWorkList,0,tmBeg);
					if(findMode != LastWork)
						preWorkList.remove(pCurWork.GetId());

					WorkPtr preWorkPtr;
					for(list<SBH_INT>::const_reverse_iterator iterWork = preWorkList.rbegin();iterWork != preWorkList.rend();++iterWork)
					{	
						if(BMaintainWork(*iterWork) && CheckWorkCondition(*iterWork,iter->wstrCondition))
							break;
						else if(CheckWorkCondition(*iterWork,iter->bWorkCondition))
						{
							preWorkPtr = AMAPI::GetWorkPtrMap()->Get(*iterWork);
							break;
						}
					}

					if (preWorkPtr.ValidPointer())
						bFlgTemp = CalWorkStateCGEx(preWorkPtr.GetId(), pCurWork.GetId(), changeCondition);


				}
				break;
			case 4:			// 未维护时间长度
				{
					if( findMode == LastWork/* || findMode == IdleMaintWork*/)
					{
						return false;
					}
					SBH_INT nLen = boost::any_cast<SBH_INT>(iter->anyVal);
					if (nLen > 0)
					{
						if(CheckWorkCondition(pCurWork.GetId(),iter->bWorkCondition))
						{
 							SBH_TIME stStart(0), stNeed(0);
 							AMAPI::GetMinStartTime(pCurWork, WorkPlan_Produce, stStart);
 							while (1)
 							{
 								SBH_INT nMTWorkId = pData->mgRemainCapacityDao.GetNearByWork(nResId, stStart, Dir_Converse, pCurWork.GetId());
 								WorkPtr workMT = AMAPI::GetWorkPtrMap()->Get(nMTWorkId);
 								if (!workMT.ValidPointer())
 									break;
	 
								//找到满足条件的维护工作为止
 								if (BMaintainWork(nMTWorkId) && CheckWorkCondition(nMTWorkId,iter->wstrCondition))
 								{
 									AMAPI::GetMaxEndTime(workMT, WorkPlan_Produce, stNeed);
 									break;
 								}
	 
 								AMAPI::GetMinStartTime(workMT, WorkPlan_Produce, stStart);
 							}
	 
 							bFlgTemp = (stInput - stNeed) >= nLen;
						}
						else
							bFlgTemp = false;
					}
				}
				break;
			case 5:			// 工作批次（工作数），在此暂且没有统计重叠的工作--Note
				{
					//前工作为维护工作
					if(preMaintainWorkFlg /*|| findMode == IdleMaintWork*/)
					{
						return false;
					}
					SBH_INT nCount = boost::any_cast<SBH_INT>(iter->anyVal);
					if (nCount > 0)
					{

						SBH_DOUBLE nWorkCount = 0;
						SBH_TIME tmBeg(0);
						if( findMode == LastWork)
						{
							tmBeg = stInput;
						}
						else
							AMAPI::GetMinStartTime(pCurWork, WorkPlan_Produce, tmBeg);

						list<SBH_INT> preWorkList;
						pData->mgRemainCapacityDao.GetResWorkList(nResId,preWorkList,0,tmBeg);
						if(findMode != LastWork)
							preWorkList.remove(pCurWork.GetId());

						for(list<SBH_INT>::const_reverse_iterator iterWork = preWorkList.rbegin();iterWork != preWorkList.rend();++iterWork)
						{
							WorkPtr workPtr = AMAPI::GetWorkPtrMap()->Get(*iterWork);
							if (workPtr.ValidPointer() )
							{
								if (BMaintainWork(*iterWork) )
								{
									if(CheckWorkCondition(*iterWork,iter->wstrCondition))
										break;
								}
								else
								{
									if(CheckWorkCondition(*iterWork,iter->bWorkCondition))
										++nWorkCount;
								}
							}
						}
						bFlgTemp = nWorkCount >= nCount;
					}
				}
				break;
			case 6:			// 是否达到制造数量，在此暂且没有统计重叠的工作--Note
				{
					//前工作为维护工作
					if(preMaintainWorkFlg /*|| findMode == IdleMaintWork*/)
					{
						return false;
					}

					SBH_DOUBLE dCount = boost::any_cast<SBH_DOUBLE>(iter->anyVal);
					if (dCount > 0)
					{
						SBH_DOUBLE dPduCount = 0;
						SBH_TIME tmBeg(0);
						if( findMode == LastWork)
						{
							tmBeg = stInput;
						}
						else
							AMAPI::GetMinStartTime(pCurWork, WorkPlan_Produce, tmBeg);

						list<SBH_INT> preWorkList;
						pData->mgRemainCapacityDao.GetResWorkList(nResId,preWorkList,0,tmBeg);
						if(findMode != LastWork)
							preWorkList.remove(pCurWork.GetId());

						for(list<SBH_INT>::const_reverse_iterator iterWork = preWorkList.rbegin();iterWork != preWorkList.rend();++iterWork)
						{
							WorkPtr workPtr = AMAPI::GetWorkPtrMap()->Get(*iterWork);
							if (workPtr.ValidPointer() )
							{
								if (BMaintainWork(*iterWork) )
								{
									if(CheckWorkCondition(*iterWork,iter->wstrCondition))
										break;
								}
								else
								{
									if(CheckWorkCondition(*iterWork,iter->bWorkCondition))
										dPduCount += workPtr.GetProcessAmount();
								}
							}
						}

						bFlgTemp = dPduCount >= dCount;
					}
				}
			case 7:			// 牛迪西亚特殊功能
				{
					//前工作为维护工作
					if(preMaintainWorkFlg || findMode == LastWork /*|| findMode == IdleMaintWork*/)
					{
						return false;
					}

					wstring changeCondition = boost::any_cast<wstring>(iter->anyVal);
		
					SBH_INT nWorkCount = 0;
					SBH_TIME tmBeg(0);
					AMAPI::GetMinStartTime(pCurWork, WorkPlan_Produce, tmBeg);
					WorkPtr preFirstWork,preSecondWork;
					list<SBH_INT> preWorkList;
					pData->mgRemainCapacityDao.GetResWorkList(nResId,preWorkList,0,tmBeg);
					preWorkList.remove(pCurWork.GetId());

					for(list<SBH_INT>::const_reverse_iterator iterWork = preWorkList.rbegin();iterWork != preWorkList.rend();++iterWork)
					{
						WorkPtr workPtr = AMAPI::GetWorkPtrMap()->Get(*iterWork);
						if (workPtr.ValidPointer() )
						{
							if (!BMaintainWork(*iterWork))
							{
								if(CheckWorkCondition(*iterWork,iter->bWorkCondition))
								{
									if(preSecondWork == NULL)
										preSecondWork = workPtr;
									else if(preFirstWork == NULL)
										preFirstWork = workPtr;
									else
										break;
								}
							}
							else if(CheckWorkCondition(*iterWork,iter->wstrCondition))
								break;
						}
					}

					if(preSecondWork == NULL)
					{
						bFlgTemp = false;
					}
					else
					{
						bFlgTemp = CalWorkStateCGEx(preFirstWork.GetId(), preSecondWork.GetId(), changeCondition);
					}
					
				}
				break;
			default:
				break;
			}
		}
		catch (const boost::bad_any_cast& )
		{
			return false;
		}

		if (iter->bResult)		// 结果且操作
			bOK &= bFlgTemp;
		else					// 结果或操作
			bOK |= bFlgTemp;
	}

	return bOK;
}

SBH_BOOL CMainFlow::ProLockedDispatch( WorkPtr& pCurWork, SBH_INT nCurResId, CSchPrepareCollection* pSchColl)
{
	SBH_BOOL bOK = false;

	list<Work*> listPreWorks;
	pCurWork.GetPreworks(listPreWorks);
	for (list<Work*>::iterator iterPre = listPreWorks.begin();
		iterPre != listPreWorks.end(); ++iterPre)
	{
		Work* pPreWork = *iterPre;
		if (pSchColl->FitLockCondition(pPreWork, pCurWork.GetId()))
		{
			SBH_TIME tmBeg(0), tmNeed(0);
			WorkResInfo workResInfo;
			pPreWork->GetPlanResRelaCom(workResInfo.resRelaID, workResInfo.assistResRelaList);
			if(workResInfo.resRelaID == NULL)
			{
				assert(0);
				continue;
			}

			list<Resource> lockRes;
			if (workResInfo.resRelaID.GetRes().GetResLckType() == 3 || workResInfo.resRelaID.GetRes().GetResLckType() == 4)
			{
				lockRes.push_back(workResInfo.resRelaID.GetRes());
			}

			for (list<WorkResRelaPtr>::iterator iterWRR = workResInfo.assistResRelaList.begin(); 
				iterWRR != workResInfo.assistResRelaList.end(); ++iterWRR)
			{
				if (iterWRR->GetRes().GetResLckType() == 3 || iterWRR->GetRes().GetResLckType() == 4)
				{
					lockRes.push_back(iterWRR->GetRes());
				}
			}

			AMAPI::GetMaxEndTime(*pPreWork, WorkPlan_Produce, tmBeg);
			for (list<Resource>::iterator iterRes = lockRes.begin();
				iterRes != lockRes.end(); ++iterRes)
			{
				const SBH_INT nPreResId = iterRes->GetId();
				if (nPreResId == nCurResId)
					continue;

				switch (iterRes->GetResLckType())
				{
				case 3:
					AMAPI::GetMinStartTime(pCurWork, WorkPlan_Produce, tmNeed);
					break;
				case 4:
					AMAPI::GetMaxEndTime(pCurWork, WorkPlan_Produce, tmNeed);
					break;
				default:
					break;
				}
				

				if (RevocationWorks(nPreResId, tmBeg, tmNeed, nCurResId, pCurWork.GetId()))
				{
				//	m_listReSchWk.push_back(pPreWork->GetId());
					bOK = true;
				}
			}
		}
	}

	return bOK;
}

SBH_BOOL CMainFlow::ProMaxIntval( WorkPtr& pCurWork, SBH_INT nCurResId )
{
	SBH_BOOL bOK = false;	

	ScheduleMethod temp = pData->parame.m_curSchMethod;
	pData->parame.m_curSchMethod = SM_IGNOREBOUND;
	//将当前工作固定
	short state = pCurWork->GetState();
	pCurWork->SetState(WorkState_Finish);
	map<WorkPriRelaPtr, Work*>  mapPreWorks;
	AMAPI::GetPreWorks(pCurWork, mapPreWorks);
	for (map<WorkPriRelaPtr, Work*>::iterator iterPre = mapPreWorks.begin();
		iterPre != mapPreWorks.end(); ++iterPre)
	{
		Work* pPreWork = iterPre->second;
		const int nMaxTimes = iterPre->first.GetMaxVal().GetTime();
		const int nMinTimes = iterPre->first.GetMinVal().GetTime();
		SBH_TIME tmBeg(0), tmEnd(0);
		AMAPI::GetMaxEndTime(*pPreWork, WorkPlan_Produce, tmBeg);
		AMAPI::GetMinStartTime(pCurWork, WorkPlan_Produce, tmEnd);
		if (nMaxTimes > 0 && nMaxTimes < (tmEnd - tmBeg))	// 不满足最大时间间隔
		{

			// 非固定工作
			if (!(pPreWork->IsNonSchWorkA() || pPreWork->IsNonSchWorkB() || pPreWork->IsNonSchWorkC()
				||pPreWork->IsSchWorkA() || pPreWork->IsSchWorkB() || pPreWork->IsSchWorkC()))
			{
				WorkResInfo workResInfo;
				pPreWork->GetPlanResRelaCom(workResInfo.resRelaID, workResInfo.assistResRelaList);
				if(workResInfo.resRelaID == NULL)
					continue;

				map<SBH_ULONG,TimeRangeList> prePlanList;	//前设置
				map<SBH_ULONG,TimeRangeList> prdPlanList;	//制造
				map<SBH_ULONG,TimeRangeList> postPlanList;	//后设置
				map<SBH_ULONG,TimeRangeList> lockPlanList;	//锁定
				pData->GetWorkPlan(pPreWork->GetId(),prePlanList,prdPlanList,postPlanList,lockPlanList);
				TestSchedulerInfo tsBackup;
				tsBackup.InsertAddPlan(pPreWork->GetId(),TYPE_PRODUCETM,prdPlanList);
				tsBackup.InsertAddPlan(pPreWork->GetId(),TYPE_PRESET,prePlanList);
				tsBackup.InsertAddPlan(pPreWork->GetId(),TYPE_POSTSET,postPlanList);
				tsBackup.InsertAddPlan(pPreWork->GetId(),TYPE_LOCKTM,lockPlanList);

				bool preFixFlg = pPreWork->GetResFix();
				pPreWork->SetIsFixRes(true);

				SingleConvCalc singleConv;
				pData->RetractWorkPlan(pPreWork->GetId(), true);
				singleConv.SingleConvSchedule(pPreWork->GetId(), tmEnd - nMaxTimes);
				AMAPI::GetMaxEndTime(*pPreWork, WorkPlan_Produce, tmBeg);
				if (nMaxTimes < (tmEnd - tmBeg))
				{
					pData->RetractWorkPlan(pPreWork->GetId(), true);
					singleConv.SingleConvSchedule(pPreWork->GetId(), tmEnd - nMinTimes);
					AMAPI::GetMaxEndTime(*pPreWork, WorkPlan_Produce, tmBeg);
					if (nMaxTimes < (tmEnd - tmBeg))
					{
						pData->RetractWorkPlan(pPreWork->GetId());
						tsBackup.FinallySurePlan();
						AppendWork* pAppWork = pData->appendWorkDao.Get(pPreWork->GetId());
						if(pAppWork!=NULL)
						{
							pAppWork->SetISSchedFlg(true);
							pAppWork->SetISSucessedFlg(true);
						}
					}
				}
				else
				{
					SBH_TIME tmNewBeg(0);
					if (false == AMAPI::GetMinStartTime(*pPreWork, WorkPlan_PreSet, tmNewBeg))
						AMAPI::GetMinStartTime(*pPreWork, WorkPlan_Produce, tmNewBeg);
					UpdatePrepareSchWorks(pPreWork, tmBeg, tmNewBeg);
				}

				pPreWork->SetIsFixRes(preFixFlg);

				list<Work*> listPreWork;
				pPreWork->GetPreworks(listPreWork);
				for (list<Work*>::iterator iterPre = listPreWork.begin();
					iterPre != listPreWork.end(); ++iterPre)
				{
					pPreWork = *iterPre;
					ProDragLocked(pPreWork);
				}
			}
		}
	}

	pCurWork->SetState(state);
	pData->parame.m_curSchMethod = temp;
	return bOK;
}


SBH_BOOL CMainFlow::RevocationWorks(SBH_INT nPreResId, SBH_TIME tmBeg, SBH_TIME tmNeed /* = 0 */, SBH_INT nCurResId /* = 0 */, SBH_INT nCurWorkId /* = 0 */)
{
	//测试一下，仅撤销本工作的效果
	pData->RetractWorkPlan(nCurWorkId,true);
	return true;

	WorkPtr curWork = AMAPI::GetWorkPtrMap()->Get(nCurWorkId);
	if (nCurResId > 0 && curWork.ValidPointer())
	{
		const SBH_INT nNexWorkId = pData->mgRemainCapacityDao.GetNearByWork(nPreResId, tmBeg, Dir_Obverse, nCurWorkId);
		WorkPtr nexWork = AMAPI::GetWorkPtrMap()->Get(nNexWorkId);
		if (nexWork.ValidPointer())
		{
			SBH_TIME tmFind(0);
			AMAPI::GetMinStartTime(nexWork, WorkPlan_Produce, tmFind);
			if (tmNeed <= tmFind)
				return false;
		}
		else
		{
			return false;
		}
	}

	list<SBH_INT> coverWorks;
	pData->mgRemainCapacityDao.GetCoverWorks(nPreResId, tmBeg, tmBeg, coverWorks);
	if (coverWorks.size() > 1)
		return false;
	map<SBH_INT, SBH_TIME> mapUndoTime;
	list<SBH_INT> listWorks;
	pData->mgRemainCapacityDao.GetResWorkList(nPreResId, listWorks, tmBeg+1);
	for (list<CIPWorkStatus>::iterator iterCIP = m_listResMaintainStatus[nPreResId].m_listWorks.begin();
		iterCIP != m_listResMaintainStatus[nPreResId].m_listWorks.end(); ++iterCIP)
	{
		if (iterCIP->nId == nCurWorkId)
			break;

		list<SBH_INT>::iterator iterWK = find(listWorks.begin(), listWorks.end(), iterCIP->nId);
		if (iterWK != listWorks.end())
			listWorks.erase(iterWK);
	}

	for (list<SBH_INT>::iterator iterWK = listWorks.begin();
		iterWK != listWorks.end(); ++iterWK)
	{
		WorkPtr workPtr = AMAPI::GetWorkPtrMap()->Get(*iterWK);
		if (workPtr.ValidPointer())
		{
			if (*iterWK == nCurWorkId && nCurResId > 0)
				continue;

			RecordAllPostWorkInfo(&workPtr, mapUndoTime, nCurResId);
		}
	}

	for (map<SBH_INT, SBH_TIME>::iterator iter = mapUndoTime.begin();
		iter != mapUndoTime.end(); ++iter)
	{
		tmBeg = iter->second;
		assert(tmBeg > 0);
		// 重叠的工作不进行撤销
		list<SBH_INT> coverWorks;
		pData->mgRemainCapacityDao.GetCoverWorks(nPreResId, tmBeg, tmBeg, coverWorks);
		if (coverWorks.size() > 1)
			continue;
		if (m_listResMaintainStatus[iter->first].tmSchBeg > tmBeg || m_listResMaintainStatus[iter->first].tmSchBeg == 0)
			m_listResMaintainStatus[iter->first].tmSchBeg = tmBeg;
		listWorks.clear();
		pData->mgRemainCapacityDao.GetResWorkList(iter->first, listWorks, tmBeg+1);

		for (list<SBH_INT>::iterator iterWK = listWorks.begin();
			iterWK != listWorks.end(); ++iterWK)
		{
			if(!BMaintainWork(*iterWK) && nCurWorkId != *iterWK)
			{
				for (list<CIPWorkStatus>::iterator iterIWK = m_listResMaintainStatus[iter->first].m_listWorks.begin();
					iterIWK != m_listResMaintainStatus[iter->first].m_listWorks.end(); ++iterIWK)
				{
					if(iterIWK->nId == m_listResMaintainStatus[iter->first].workSchBeg)
						break;
					else if(iterIWK->nId == *iterWK)
					{
						m_listResMaintainStatus[iter->first].workSchBeg = *iterWK;
						break;
					}
				}
				break;
			}
		}
	}

	// 将收集到得开始撤销时间点后的所有工作撤销，固定工作除外
	for (map<SBH_INT, SBH_TIME>::iterator iter = mapUndoTime.begin();
		iter != mapUndoTime.end(); ++iter)
	{
		tmBeg = iter->second;
		assert(tmBeg > 0);
		//if (m_listResMaintainStatus[iter->first].tmSchBeg > tmBeg)
		//	m_listResMaintainStatus[iter->first].tmSchBeg = tmBeg;
		listWorks.clear();
		pData->mgRemainCapacityDao.GetResWorkList(iter->first, listWorks, tmBeg+1);
		for (list<CIPWorkStatus>::iterator iterCIP = m_listResMaintainStatus[nPreResId].m_listWorks.begin();
			iterCIP != m_listResMaintainStatus[nPreResId].m_listWorks.end(); ++iterCIP)
		{
			if (iterCIP->nId == nCurWorkId)
				break;

			list<SBH_INT>::iterator iterWK = find(listWorks.begin(), listWorks.end(), iterCIP->nId);
			if (iterWK != listWorks.end())
				listWorks.erase(iterWK);
		}
		if (!listWorks.empty() && listWorks.front() == nCurWorkId)
		{
			pData->RetractWorkPlan(nCurWorkId, true);
			listWorks.pop_front();
		}

		//if (!listWorks.empty())
		//for (list<SBH_INT>::iterator iterWK = listWorks.begin();
		//	iterWK != listWorks.end(); ++iterWK)
		//{
		//	if(!BMaintainWork(*iterWK))
		//	{
		//		m_listResMaintainStatus[iter->first].workSchBeg = *iterWK;
		//		break;
		//	}
		//}

		for (list<SBH_INT>::iterator iterWK = listWorks.begin();
			iterWK != listWorks.end(); ++iterWK)
		{
			WorkPtr workPtr = AMAPI::GetWorkPtrMap()->Get(*iterWK);
			if (false == workPtr.ValidPointer() || nCurWorkId == *iterWK)
				continue;

			if (!(workPtr->IsNonSchWorkA() || workPtr->IsNonSchWorkB() || workPtr->IsNonSchWorkC()
				|| workPtr->IsSchWorkA() || workPtr->IsSchWorkB() || workPtr->IsSchWorkC()))
			{
				pData->RetractWorkPlan(*iterWK, true);
				if (!BMaintainWork(*iterWK))
					m_listReSchWk.push_back(*iterWK);
			}
		}
	}

	return true;
}

SBH_BOOL CMainFlow::FitPrepareSchWorks(SBH_INT nWorkId)
{
	WorkPtr pCurWork = AMAPI::GetWorkPtrMap()->Get(nWorkId);
	if (!pCurWork.ValidPointer())
		return false;

	SBH_BOOL bOK = true;

	list<Work*> listPreWorks;
	pCurWork.GetPreworks(listPreWorks);
	for (list<Work*>::iterator iter = listPreWorks.begin();
		iter != listPreWorks.end(); ++iter)
	{
		Work* pPreWork = *iter;
		if (!(pPreWork->GetSchedulerFlg() >= SCHEDULEREDFINISH || 
			(pPreWork->IsNonSchWorkA() || pPreWork->IsNonSchWorkB() || pPreWork->IsNonSchWorkC()
			||pPreWork->IsSchWorkA() || pPreWork->IsSchWorkB() || pPreWork->IsSchWorkC())))
		{
			bOK = false;
			break;
		}
	}
	return bOK;
}

void CMainFlow::RecordAllSecondResWork(SBH_INT workId,SBH_INT nPreResId,SBH_TIME tmBeg,map<SBH_INT, SBH_TIME>& mapUndoTime,SBH_INT nCurResId)
{
	
	list<SBH_INT> listWorks;
	pData->mgRemainCapacityDao.GetResWorkList(nPreResId, listWorks, tmBeg);
	for (list<SBH_INT>::iterator iterWK = listWorks.begin();
		iterWK != listWorks.end(); ++iterWK)
	{
		Work workPtr = AMAPI::GetWorkPtrMap()->Get(*iterWK);
		if (workPtr.ValidPointer())
		{
			RecordAllPostWorkInfo(&workPtr, mapUndoTime, nCurResId);
		}
	}
}


void CMainFlow::RecordAllPostWorkInfo( Work* pCurWork, map<SBH_INT, SBH_TIME>& mapUndoTime, SBH_INT nCurResId )
{
	list<Work*> listWK;
	pCurWork->GetPostworks(listWK);
	// 找当前工作的主副资源，并记录开始撤销时间
	WorkResInfo workResInfo;
	pCurWork->GetPlanResRelaCom(workResInfo.resRelaID, workResInfo.assistResRelaList);
	if (workResInfo.resRelaID.ValidPointer())
	{
		SBH_TIME tmBeg(0);
		AMAPI::GetMinStartTime(*pCurWork, WorkPlan_Produce, tmBeg);
		SBH_INT nResId = workResInfo.resRelaID.GetResId();
		if (nResId != nCurResId)
		{
			if (mapUndoTime.find(nResId) == mapUndoTime.end())
			{
				mapUndoTime.insert(make_pair(nResId, tmBeg));
				RecordAllSecondResWork(pCurWork->GetId(),nResId,tmBeg,mapUndoTime,nCurResId);
			}
			else if (tmBeg > 0 && mapUndoTime[nResId] > tmBeg)
			{
				mapUndoTime[nResId] = tmBeg;
				RecordAllSecondResWork(pCurWork->GetId(),nResId,tmBeg,mapUndoTime,nCurResId);
			}
		}

		for (list<WorkResRelaPtr>::iterator iter = workResInfo.assistResRelaList.begin();
			iter != workResInfo.assistResRelaList.end(); ++iter)
		{
			nResId = iter->GetResId();
			if (nResId != nCurResId)
			{
				if (mapUndoTime.find(nResId) == mapUndoTime.end())
				{
					mapUndoTime.insert(make_pair(nResId, tmBeg));
					RecordAllSecondResWork(pCurWork->GetId(),nResId,tmBeg,mapUndoTime,nCurResId);
				}
				else if (tmBeg > 0 && mapUndoTime[nResId] > tmBeg)
				{
					mapUndoTime[nResId] = tmBeg;
					RecordAllSecondResWork(pCurWork->GetId(),nResId,tmBeg,mapUndoTime,nCurResId);
				}
			}
		}
	}

	WorkResInfo workResInfo2;
	for (list<Work*>::iterator iter = listWK.begin();
		iter != listWK.end(); ++iter)
	{
		(*iter)->GetPlanResRelaCom(workResInfo2.resRelaID, workResInfo2.assistResRelaList);
		if (workResInfo2.resRelaID.ValidPointer())
		{
			const SBH_INT nResId = workResInfo2.resRelaID.GetResId();
			SBH_TIME tmBeg(0);
			AMAPI::GetMinStartTime(*(*iter), WorkPlan_Produce, tmBeg);
	
			if (nResId != nCurResId)
			{
				if (mapUndoTime.find(nResId) == mapUndoTime.end())
					mapUndoTime.insert(make_pair(nResId, tmBeg));
				else if (tmBeg > 0 && mapUndoTime[nResId] > tmBeg)
					mapUndoTime[nResId] = tmBeg;
			}

			RecordAllPostWorkInfo(*iter, mapUndoTime, nCurResId);
		}
	}
}

void CMainFlow::UpdatePrepareSchWorks(Work* pWork, SBH_TIME tmBeg, SBH_TIME tmEnd)
{
	WorkPtrMap* pWorkMap = AMAPI::GetWorkPtrMap();
	if (pWorkMap == NULL)
		return;

	WorkResInfo workResInfo;
	pWork->GetPlanResRelaCom(workResInfo.resRelaID, workResInfo.assistResRelaList);
	if (workResInfo.resRelaID.ValidPointer())
	{
		list<SBH_INT> listWorks;
		SBH_INT nResId = workResInfo.resRelaID.GetResId();
		pData->mgRemainCapacityDao.GetResWorkList(nResId, listWorks, tmBeg, tmEnd);
		m_listResMaintainStatus[nResId].tmSchBeg = tmEnd;
// 		if (!listWorks.empty())
// 		{
// 			const SBH_INT nWKIdBeg = listWorks.front();
// 			const SBH_INT nWKIdEnd = listWorks.back();
// 			listWorks.insert()
// 		}

		for (list<WorkResRelaPtr>::iterator iter = workResInfo.assistResRelaList.begin();
			iter != workResInfo.assistResRelaList.end(); ++iter)
		{
			nResId = iter->GetResId();
			listWorks.clear();
			pData->mgRemainCapacityDao.GetResWorkList(nResId, listWorks, tmBeg, tmEnd);
			m_listResMaintainStatus[nResId].tmSchBeg = tmEnd;
		}
	}
}

SBH_BOOL CMainFlow::ScheduleWork(WorkPtr& workPtr, SBH_INT resId, SBH_TIME tmBeg, DirType dirSch, SBH_TIME& rtTmBeg,Work& curWork)
{
	rtTmBeg = tmBeg;
	//SBH_BOOL bOK = true;

	//SingleCalcV3 singleCal;

	//TimeLenInfo tmLen;
	//singleCal.GetPrdTm(workPtr, resInfo, tmLen);
	//
	//SBH_TIME rtStTm, rtEndTm;
	//TestSchedulerInfo tsInfo;
	//bOK = singleCal.CalWorkTime(workPtr, resInfo, TYPE_PRODUCETM, tmBeg
	//	,tmLen, tsInfo, rtStTm, rtEndTm, FUC_F2, dirSch);
	//if (bOK)
	//{
	//	workPtr.ClearSchPlan();
	//	tsInfo.SurePlan();
	//	AppendWork* pAppWork = pData->appendWorkDao.Get(workPtr->GetId());
	//	if(pAppWork != NULL)
	//	{
	//		pAppWork->SetISSchedFlg(true);
	//		pAppWork->SetISSucessedFlg(true);
	//	}
	//}
	//else
	//{
	//	rtTmBeg = min(rtStTm, rtEndTm);
	//}

	AppendWork* pAppWork = pData->appendWorkDao.Get(workPtr->GetId());
	if(pAppWork == NULL) return false;

	SBH_BOOL bOK = true;
	if(dirSch == Dir_Obverse)	//正向
	{
		SingleObvCalc singleObv;
		singleObv.SingleObvSchedule(workPtr.GetId(),tmBeg);

		if(!(pAppWork->GetISSucessedFlg() && pAppWork->GetISSchedFlg()))
		{
			pData->RetractWorkPlan(workPtr.GetId());
			bOK = false;
		}
		else
		{
			SBH_TIME mtLimitTm(0);
			SBH_TIME curStTm(0);
			if(AMAPI::GetMaxEndTime(workPtr,TYPE_PRODUCETM,mtLimitTm) && AMAPI::GetMinStartTime(curWork,TYPE_PRODUCETM,curStTm))
			{
				if(mtLimitTm >= curStTm)
				{
					pData->RetractWorkPlan(workPtr.GetId());
					bOK = false;
				}
				else
				{
					list<SBH_INT> listWorks;
					pData->mgRemainCapacityDao.GetResWorkList(resId, listWorks, mtLimitTm, curStTm);
					listWorks.remove(workPtr.GetId());
					listWorks.remove(curWork.GetId());
					if(!listWorks.empty())
					{
						//清除维护工作
						pData->RetractWorkPlan(workPtr.GetId());
						bOK = false;
					}
				}
			}
			else
			{
				pData->RetractWorkPlan(workPtr.GetId());
				bOK = false;
			}
		}
	}
	else
	{
		SingleConvCalc singleConv;
		singleConv.SingleConvSchedule(workPtr.GetId(),tmBeg);

		if(!(pAppWork->GetISSucessedFlg() && pAppWork->GetISSchedFlg()))
		{
			pData->RetractWorkPlan(workPtr.GetId());
			bOK = false;
		}
		else
		{
			SBH_TIME mtLimitTm(0);
			SBH_TIME curStTm(0);
			if(AMAPI::GetMaxEndTime(workPtr,TYPE_PRODUCETM,mtLimitTm) && AMAPI::GetMinStartTime(curWork,TYPE_PRODUCETM,curStTm))
			{
				list<SBH_INT> listWorks;
				pData->mgRemainCapacityDao.GetResWorkList(resId, listWorks,mtLimitTm, curStTm);
				listWorks.remove(workPtr.GetId());
				listWorks.remove(curWork.GetId());
				if(!listWorks.empty())
				{
					//清除维护工作
					pData->RetractWorkPlan(workPtr.GetId());
					bOK = false;
				}
			}
			else
			{
				pData->RetractWorkPlan(workPtr.GetId());
				bOK = false;
			}
		}
	}

	return bOK;
}
bool SortMaintainParam( SBH_INT nLeftId, SBH_INT nRightId )
{
	bool bOK = false;

	AM_CreateMTOrderParam mtLeft = AMAPI::GetCreateMTOrderParamPtrMap()->Get(nLeftId);
	AM_CreateMTOrderParam mtRight = AMAPI::GetCreateMTOrderParamPtrMap()->Get(nRightId);
	if (!mtLeft.ValidPointer() && !mtRight.ValidPointer())
		return nLeftId > nRightId;
	else if(!mtLeft.ValidPointer())
		return false;
	else if(!mtRight.ValidPointer())
		return true;

	bOK = mtLeft.GetProprity() > mtRight.GetProprity();

	return bOK;
}

// 函数说明:查找范围it1-it2(不包含) 如果查找到里面任意一个数与data内数相等 则返回这个元素的迭代器
// 如果it1==it2 直接返回it2
list<pair<SBH_INT,SBH_INT> >::iterator findPair(list<pair<SBH_INT,SBH_INT> >::iterator it1, list<pair<SBH_INT,SBH_INT> >::iterator it2, const pair<SBH_INT,SBH_INT>& data)
{
	while(it1 != it2)
	{
		if((it1->first == data.first) ||
			(it1->first == data.second) ||
			(it1->second == data.first) ||
			(it1->second == data.second))
			return it1;
		++it1;
	}
	return it1;
}
list<pair<SBH_INT,SBH_INT> >::iterator findPair(list<pair<SBH_INT,SBH_INT> >::iterator it1, list<pair<SBH_INT,SBH_INT> >::iterator it2, SBH_INT data)
{
	while(it1 != it2)
	{
		if((it1->first == data) || (it1->second == data))
			return it1;
		++it1;
	}
	return it1;
}

bool CMainFlow::GetWorkSequence( list<SBH_INT>& workSeq ,list<pair<int,int>>& lerr)
{
	list<pair<SBH_INT,SBH_INT> > inList;

	bool bOK = false;

	WorkPtrMap* pWorkMap = AMAPI::GetWorkPtrMap();
	if (pWorkMap == NULL)
		return bOK;

	ResourcePtrMap* pResPtrMap = AMAPI::GetResourcePtrMap();
	if (pResPtrMap == NULL)
		return bOK;
	// 建立临时链表list<list<SBH_INT>> 存储所有链表
	// 建立list<SBH_INT>::iteartor对应inList第一个for循环的最后一次插入时的位置
	list<list<pair<SBH_INT,SBH_INT> > > allList;
	list<pair<SBH_INT,SBH_INT> >::iterator itInList;
	//建立两两关系网
	for(pWorkMap->Begin();pWorkMap->NotEnd();pWorkMap->Step())
	{
		WorkPtr& workPtr = pWorkMap->Get();
		if(!BMaintainWork(workPtr->GetId()))
		{
			list<Work*> postWorkList;
			workPtr.GetPostworks(postWorkList);
			for(list<Work*>::iterator iter =postWorkList.begin();
				iter != postWorkList.end();++iter)
			{
				inList.push_back(make_pair(workPtr.GetId(),(*iter)->GetId()));
			}
		}
	}
	// 让itInList 指向此时的inList最后一个元素
	itInList = inList.end();
	--itInList;
	// 给inList中所有的数据分组放到groupList中
	{
		list<pair<SBH_INT,SBH_INT> > groupList; // 作为allList 存储的临时变量
		list<pair<SBH_INT,SBH_INT> > lpList;
		list<pair<SBH_INT,SBH_INT> >::iterator it1, it2;
		for(it1=inList.begin(); it1!=inList.end(); ++it1)
			lpList.push_back(*it1);
		list<pair<SBH_INT,SBH_INT> >::iterator itGroup;
		while(!lpList.empty())
		{
			groupList.push_back(*lpList.begin());
			lpList.erase(lpList.begin());
			itGroup = groupList.end();
			--itGroup;
			while(itGroup != groupList.end())
			{
				it1 = lpList.begin();

				while((it1=findPair(it1, lpList.end(), *itGroup)) != lpList.end())
				{
					groupList.push_back(*it1);
					it2 = it1++;
					lpList.erase(it2);
					if(lpList.empty()) break;
				}
				++itGroup;
			}
			// 分组是无序的
			allList.push_back(groupList);
			groupList.clear();
		}
	}

	for (pResPtrMap->Begin(); pResPtrMap->NotEnd(); pResPtrMap->Step())
	{
		Resource resPtr = pResPtrMap->Get();
		if (resPtr.ValidPointer())
		{
			list<SBH_INT> listWorks;
			const SBH_INT nResId = resPtr.GetId();
			pData->mgRemainCapacityDao.GetResWorkList(resPtr.GetId(), listWorks);
			for(list<SBH_INT>::iterator iter = listWorks.begin();iter != listWorks.end(); )
			{
				if(BMaintainWork(*iter))
				{
					++iter;
					continue;
				}

				list<SBH_INT>::iterator iter2 = iter;
				do{
				++iter2;
				if(iter2 == listWorks.end())
					break;
				}while(BMaintainWork(*iter2));

				if(iter2 == listWorks.end())
					break;

				inList.push_back(make_pair(*iter,*iter2));
				++iter;
			}
		}
	}
	// 找出不合规矩的规则 从inList中删除
	list<SBH_INT> resListFirst;
	{
		Graph* g = new Graph;
		list<pair<SBH_INT,SBH_INT>> errList;
		list<pair<SBH_INT,SBH_INT>>::iterator itDel;
		g->sort(inList, resListFirst, errList);
		delete g;
		while(!errList.empty())
		{
			lerr.push_back(errList.front());
			if((itDel=find(itInList, inList.end(), errList.front())) != inList.end())
				inList.erase(itDel);
			errList.pop_front();
		}
	}
	list<pair<SBH_INT,SBH_INT> > newRule;
	{
		// allList itInList 建一个临时链表存储新的规则list<pair<SBH_INT,SBH_INT> > newRule;最后将newRule合并到inList
		// 建立一个临时链表allListIt 存储allList的迭代器
		// 以确定是否已经存储过
		list<list<pair<SBH_INT,SBH_INT>> >::iterator itAllList1, itAllList2;
		itAllList2 = itAllList1 = allList.end();
		// 两个迭代器指针allListIt1 allListIt2 判断两组数据是否存过;
		list<list<list<pair<SBH_INT,SBH_INT>>>::iterator> allListIt1;
		list<list<list<pair<SBH_INT,SBH_INT>>>::iterator> allListIt2;
		graph* g_group = new graph;
		list<SBH_INT> lsInt;
		list<SBH_INT>::iterator itInt;
		list<pair<SBH_INT,SBH_INT> >::iterator itlpInt;
		list<SBH_INT>::iterator itRes1, itRes2, itBackRes=resListFirst.end();
		--itBackRes;
		for(itRes1=resListFirst.begin(); itRes1!=itBackRes; ++itRes1)
		{
			itRes2 = itRes1;
			++itRes2;
			itAllList1 = itAllList2 = allList.end();
			for(list<list<pair<SBH_INT,SBH_INT>> >::iterator it1=allList.begin(); it1!=allList.end(); ++it1)
			{
				if((itAllList1 == allList.end()) && (findPair(it1->begin(), it1->end(), *itRes1) != it1->end()))
					itAllList1 = it1;
				if((itAllList2 == allList.end()) && (findPair(it1->begin(), it1->end(), *itRes2) != it1->end()))
					itAllList2 = it1;
				if((itAllList1 != allList.end()) && (itAllList2 != allList.end()))
					break;
			}
			if(itAllList1 == itAllList2)
				continue;
			if(find(allListIt1.begin(), allListIt1.end(), itAllList1) != allListIt1.end())
				continue;
			if(find(allListIt2.begin(), allListIt2.end(), itAllList2) != allListIt2.end())
				continue;
			allListIt1.push_back(itAllList1);allListIt2.push_back(itAllList2);
			list<pair<SBH_INT,SBH_INT> >::iterator itlpInt = itAllList2->begin();
			while(itlpInt != itAllList2->end())
			{
				g_group->insert(itlpInt->first, itlpInt->second);
				++itlpInt;
			}
			lsInt.clear();
			g_group->GetHeads(lsInt);
			g_group->clear();

			for(list<pair<SBH_INT,SBH_INT>>::iterator it=itAllList1->begin(); it!=itAllList1->end(); ++it)
			{
				for(itInt=lsInt.begin(); itInt!=lsInt.end(); ++itInt)
				{
					newRule.push_back(make_pair(it->first, *itInt));
					newRule.push_back(make_pair(it->second, *itInt));
				}
			}
		}
		delete g_group;
	}
	newRule.sort();
	newRule.unique();
	inList.splice(inList.end(), newRule);
#ifdef TESTWORKSEQ
	{
		TimeLog::WriteLog("输入排序工作顺序");
		for(list<pair<SBH_INT,SBH_INT> >::iterator iter = inList.begin();iter != inList.end();++iter)
		{
			char buf[20];
			sprintf_s(buf,"%d-%d ",iter->first,iter->second);
			TimeLog::WriteLog(buf);
		}
		TimeLog::WriteLog("工作排序结果");
	}
#endif


	workSeq.clear();



	Graph g;
	return g.sort(inList, workSeq, lerr);

//	return SortPair(inList,workSeq);

}

bool CMainFlow::RevocationTimes(Work& curWork,map<SBH_INT,SBH_TIME>& tmPreEnd,map<SBH_INT,SBH_TIME>& tmPreSt)
{
	SBH_BOOL bSwap = false;

	SBH_TIME tmNewBeg(0),tmNewEnd(0);
	AMAPI::GetMaxEndTime(curWork, WorkPlan_Produce, tmNewEnd);
	if(tmNewEnd >= AMAPI::GetSchEndTime())
		return bSwap;

	if (false == AMAPI::GetMinStartTime(curWork, WorkPlan_PreSet, tmNewBeg))
		AMAPI::GetMinStartTime(curWork, WorkPlan_Produce, tmNewBeg);


	set<SBH_INT> resSet;
	SBH_BOOL coverWorkFlg(true);
	curWork->GetPlanResList(resSet);
	for(set<SBH_INT>::iterator iterResSet = resSet.begin();iterResSet != resSet.end();++iterResSet)
	{
		list<SBH_INT> preWorkList;
		pData->mgRemainCapacityDao.GetResWorkList(*iterResSet,preWorkList,0,tmNewBeg);
		preWorkList.remove(curWork.GetId());

		//查询此资源上工作是否重叠（区间使用计划的制造开始和结束时刻就行）
		list<SBH_INT> coverWorkList;
		pData->mgRemainCapacityDao.GetCoverWorks(*iterResSet,tmNewBeg,tmNewEnd,coverWorkList);
		if(!coverWorkList.empty())	//有重叠，此资源上撤销时刻为所有重叠工作结束时刻
		{
			for(list<SBH_INT>::iterator iterCover = coverWorkList.begin();iterCover != coverWorkList.end();++iterCover)
			{
				Work coverWork = AMAPI::GetWorkPtrMap()->Get(*iterCover); 
				SBH_TIME tempTm(0);
				if (false == AMAPI::GetMaxEndTime(coverWork, workPlan_NextSet, tempTm))
					if (false == AMAPI::GetMaxEndTime(coverWork, WorkPlan_Lock, tempTm))
						AMAPI::GetMaxEndTime(coverWork, WorkPlan_Produce, tempTm);

				//coverWorkEdTm = max(tempTm,coverWorkEdTm);
				tmPreEnd[*iterResSet] = max(tempTm,tmPreEnd[*iterResSet]);

			}
			coverWorkFlg = false;
		}

				list<CIPWorkStatus>& listWorks = m_listResMaintainStatus[*iterResSet].m_listWorks;
		for (list<CIPWorkStatus>::iterator iterCIP = listWorks.begin();
			iterCIP != listWorks.end(); ++iterCIP)
		{
			if (iterCIP->nId == curWork->GetId())
			{
				break;
			}
			else
			{
				//找前面的工作
				list<SBH_INT>::iterator iterPreWork = find(preWorkList.begin(),preWorkList.end(),iterCIP->nId);
				if(iterPreWork == preWorkList.end())	//没有找到
				{
					//前工作是否已排
					//if()
					WorkPtr undoWKPre = AMAPI::GetWorkPtrMap()->Get(iterCIP->nId);

					if(undoWKPre!=NULL && undoWKPre.GetSchedulerFlg() >= SCHEDULEREDFINISH)
					{
						SBH_TIME tempTmPreEnd(0),tempTmPreSt(0);
						if (false == AMAPI::GetMaxEndTime(undoWKPre, workPlan_NextSet, tempTmPreEnd))
							AMAPI::GetMaxEndTime(undoWKPre, WorkPlan_Produce, tempTmPreEnd);
						tmPreEnd[*iterResSet] = max(tmPreEnd[*iterResSet],tempTmPreEnd);

						if (false == AMAPI::GetMinStartTime(undoWKPre, WorkPlan_PreSet, tempTmPreSt))
							AMAPI::GetMinStartTime(undoWKPre, WorkPlan_Produce, tempTmPreSt);
						tmPreSt[*iterResSet] = max(tmPreSt[*iterResSet],tempTmPreSt);
						bSwap = true;
					}
				}
				else
				{
					preWorkList.erase(iterPreWork);
					WorkPtr undoWKPre = AMAPI::GetWorkPtrMap()->Get(iterCIP->nId);
					if(undoWKPre!=NULL && undoWKPre.GetSchedulerFlg() >= SCHEDULEREDFINISH)
					{
						SBH_TIME tempTmPreEnd(0),tempTmPreSt(0);;
						if (false == AMAPI::GetMaxEndTime(undoWKPre, workPlan_NextSet, tempTmPreEnd))
							AMAPI::GetMaxEndTime(undoWKPre, WorkPlan_Produce, tempTmPreEnd);

						tmPreEnd[*iterResSet] = max(tmPreEnd[*iterResSet],tempTmPreEnd);

						if (false == AMAPI::GetMinStartTime(undoWKPre, WorkPlan_PreSet, tempTmPreSt))
							AMAPI::GetMinStartTime(undoWKPre, WorkPlan_Produce, tempTmPreSt);
						tmPreSt[*iterResSet] = max(tmPreSt[*iterResSet],tempTmPreSt);
					}


				}


			}
		}

		for(list<SBH_INT>::iterator iterPreWork = preWorkList.begin();iterPreWork != preWorkList.end();++iterPreWork)
		{
			if(BMaintainWork(*iterPreWork))
			{
				WorkPtr undoWKPre = AMAPI::GetWorkPtrMap()->Get(*iterPreWork);
				if(undoWKPre!=NULL && undoWKPre.GetSchedulerFlg() >= SCHEDULEREDFINISH)
				{
					SBH_TIME tempTmPreEnd(0),tempTmPreSt(0);;
					if (false == AMAPI::GetMaxEndTime(undoWKPre, workPlan_NextSet, tempTmPreEnd))
						AMAPI::GetMaxEndTime(undoWKPre, WorkPlan_Produce, tempTmPreEnd);
					tmPreEnd[*iterResSet] = max(tmPreEnd[*iterResSet],tempTmPreEnd);

					if (false == AMAPI::GetMinStartTime(undoWKPre, WorkPlan_PreSet, tempTmPreSt))
						AMAPI::GetMinStartTime(undoWKPre, WorkPlan_Produce, tempTmPreSt);
					tmPreSt[*iterResSet] = max(tmPreSt[*iterResSet],tempTmPreSt);
				}

			}
			else
			{
				bSwap = true;
				break;
			}

		}
	}
	return bSwap & coverWorkFlg;
}

void CMainFlow::AfterLastWork()
{
	for (map<SBH_INT, ResMaintainStatus>::iterator iterRes = m_listResMaintainStatus.begin();
		iterRes != m_listResMaintainStatus.end(); ++iterRes)
	{
		const SBH_INT nResId = iterRes->first;

		list<SBH_INT> listMT;
		AM_CreateMTOrderParamPtrMap* pMTParamMap = AMAPI::GetCreateMTOrderParamPtrMap();
		for (pMTParamMap->Begin(); pMTParamMap->NotEnd(); pMTParamMap->Step())
		{
			AM_CreateMTOrderParam mtParam = pMTParamMap->Get();
			if (mtParam.ValidPointer() && mtParam.GetMTIsValid() && mtParam.GetResId() == nResId && mtParam.GetOpPosition() == GetRemarkFiled(ID_CREATEMT_OPPOSITION_2))
			{
				listMT.push_back(mtParam.GetId());
			}
		}

		if (listMT.empty())
			continue ;

		listMT.sort(SortMaintainParam);

		list<SBH_INT> preWorkList;
		pData->mgRemainCapacityDao.GetResWorkList(nResId,preWorkList);
		if(preWorkList.empty())
			continue;

		Work pPreWork,pCurWork;
		SBH_TIME stInput(0);
		list<SBH_INT>::const_reverse_iterator iterWork = preWorkList.rbegin();
		if(BMaintainWork(*iterWork))	return;
		pPreWork = AMAPI::GetWorkPtrMap()->Get(*iterWork);
		if(pPreWork != NULL)
		{
			if (false == AMAPI::GetMaxEndTime(pPreWork, workPlan_NextSet, stInput))
				if (false == AMAPI::GetMaxEndTime(pPreWork, WorkPlan_Lock, stInput))
					AMAPI::GetMaxEndTime(pPreWork,TYPE_PRODUCETM,stInput);
		}
		else
			continue;

		for (list<SBH_INT>::iterator iter = listMT.begin(); iter != listMT.end(); ++iter)
		{
			// 找到一条符合条件的维护信息
			if (FindSuitMaintainInfo(nResId, *iter, stInput, pCurWork, pPreWork,LastWork))
			{
				SBH_INT nMTWorkId = GetMaintainWork(*iter,nResId);
				WorkPtr mtWorkPtr = AMAPI::GetWorkPtrMap()->Get(nMTWorkId);
				if (!mtWorkPtr.ValidPointer())
					break;

				if (pData->appendWorkDao.Get(nMTWorkId) == NULL)
				{
					AppendWork pAppWork;
					pAppWork.id = nMTWorkId;
					pAppWork.SetWorkPtr(mtWorkPtr);
					pAppWork.initEarlyStartTime = AMAPI::GetSchStartTime();
					pAppWork.initLatestEndTime = AMAPI::GetSchEndTime();
					pAppWork.earlieststarttime = AMAPI::GetSchStartTime();
					pAppWork.latestendtime = AMAPI::GetSchEndTime();
					pData->appendWorkDao.Add(pAppWork);
				}

				SingleObvCalc singleObv;
				ScheduleMethod oldSchMethod = pData->parame.m_curSchMethod;
				//为什么要强制排？
				//pData->parame.m_curSchMethod = SM_FORCE;
				singleObv.SingleObvSchedule(nMTWorkId, stInput);
				pData->parame.m_curSchMethod = oldSchMethod;
				break;
			}
		}
	}
}

SBH_BOOL CMainFlow::ScheduleByCalendar()
{
	AMDayShiftPtrMap* pDayShiftMap = AMAPI::GetAMDayShiftPtrMap();
	for (pDayShiftMap->Begin(); pDayShiftMap->NotEnd(); pDayShiftMap->Step())
	{
		AMDayShift dayShift = pDayShiftMap->Get();
		wstring wstr = dayShift.GetWorkTmSectStr();
		int num = 0;
	}
	return true;
}