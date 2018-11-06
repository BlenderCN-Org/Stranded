#include "LockedDispose.h"

#include "SingleObvCalc.h"
#include "SingleConvCalc.h"
#include "CGraphSort.h"
#include "DisobeyRestrict.h"

CLockedDispose::CLockedDispose(void)
	: m_direction(Dir_Obverse)
	, m_tmRSchBound(0)
	, m_nAdjustCount(0)
	, m_bAdjustFailed(false)
	, m_bFixNexWK(false)
	, m_bAdjusted(false)
{
}

CLockedDispose::~CLockedDispose(void)
{
}

SBH_BOOL CLockedDispose::ResLockedPro()
{
	SBH_BOOL bOK = true;

	SBH_INT nPreWorkId(0), nNexWorkId(0);
	set<vector<SBH_INT> > setWorkPair = pData->workFilter.GetLockedGp();

	list<pair<SBH_INT, SBH_INT> > listPrepareWP, listError;
	for (set<vector<SBH_INT> >::iterator iterWP = setWorkPair.begin();
		iterWP != setWorkPair.end(); ++iterWP)
	{
		assert(iterWP->size() == 3);
		listPrepareWP.push_back(make_pair((*iterWP)[0], (*iterWP)[1]));
	}

	Graph g;
	list<SBH_INT> listSorted;
	g.sort(listPrepareWP, listSorted, listError);
	list<vector<SBH_INT> > listSortedWorkPair;
	for (list<SBH_INT>::iterator iterWK = listSorted.begin();
		iterWK != listSorted.end(); ++iterWK)
	{
		for (set<vector<SBH_INT> >::iterator iterWP = setWorkPair.begin();
			iterWP != setWorkPair.end(); ++iterWP)
		{
			if (*iterWK == (*iterWP)[0])
			{
				listSortedWorkPair.push_back(*iterWP);
			}
		}
	}

	if (AMAPI::GetDirectionFlg() == Sch_Dir_Force_Conv)
		listSortedWorkPair.reverse();

	m_bAdjusted = false;
	m_iterCur = listSortedWorkPair.end();
	for (list<vector<SBH_INT> >::iterator iter = listSortedWorkPair.begin();
		iter != listSortedWorkPair.end();)
	{
		nPreWorkId = (*iter)[0];
		nNexWorkId = (*iter)[1];
		if (false == m_bAdjusted)
			m_iterCur = iter;
		//获取两个工作的状态，将未排工作跳过
		AppendWork* pPreAppWork = pData->GetAppendWork(nPreWorkId);
		AppendWork* pNxtAppWork = pData->GetAppendWork(nNexWorkId);
		if(pPreAppWork!=NULL && pNxtAppWork != NULL
			&& pPreAppWork->GetISSchedFlg() && pPreAppWork->GetISSucessedFlg()
			&& pNxtAppWork->GetISSchedFlg() && pNxtAppWork->GetISSucessedFlg())
		{
			if (ResLockedPairProNeedAdj(iter, listSortedWorkPair))
				++iter;
		}
		else
		{
			++iter;
		}
	}

	return bOK;
}

SBH_BOOL CLockedDispose::ResLockedPairPro(vector<SBH_INT>& workPair, SBH_BOOL bAdjust /* = false */)
{
	SBH_BOOL bOK = true;

	const SBH_INT nPreWorkId = workPair[0];
	const SBH_INT nNxtWorkId = workPair[1];

	WorkPtr& workPtr = pData->GetAppendWork(nPreWorkId)->GetWorkPtr();
	WorkResInfo workResInfo;
	list<WorkResRelaPtr>::iterator iterWRR;
	workPtr.GetPlanResRelaCom(workResInfo.resRelaID, workResInfo.assistResRelaList);
	if(workResInfo.resRelaID == NULL)
		return bOK;

	list<Resource> lockRes;
	vector<SBH_DOUBLE> vecLockResCnt;
	if (workResInfo.resRelaID.GetRes().GetResLckType() == 3 || workResInfo.resRelaID.GetRes().GetResLckType() == 4)
	{
		lockRes.push_back(workResInfo.resRelaID.GetRes());
		vecLockResCnt.push_back(workResInfo.resRelaID.GetPdtResCnt());
	}

	for (iterWRR = workResInfo.assistResRelaList.begin(); 
		iterWRR != workResInfo.assistResRelaList.end(); ++iterWRR)
	{
		if (iterWRR->GetRes().GetResLckType() == 3 || iterWRR->GetRes().GetResLckType() == 4)
		{
			lockRes.push_back(iterWRR->GetRes());
			vecLockResCnt.push_back(iterWRR->GetPdtResCnt());
		}
	}

	if (lockRes.empty())
		return bOK;

	ScheduleMethod temp = pData->parame.m_curSchMethod;
	pData->parame.m_curSchMethod = SM_IGNORE;
	
	list<WorkSchPlanPtr> listWSP = workPtr.GetSchPlanList();
	map<SBH_ULONG,TimeRangeList>	mapBackupNexSet;
	map<SBH_ULONG,TimeRangeList>	mapBackupLock;
	TestSchedulerInfo tsBackup,nxtWorkPreBackup;
	SBH_TIME lockSt(0),lockEd(0);
	for (list<WorkSchPlanPtr>::iterator iter = listWSP.begin();
		iter != listWSP.end(); ++iter)
	{
		if ((*iter)->GetType() == workPlan_NextSet)
		{
			mapBackupNexSet[(*iter)->GetPlanSelResId()].push_back(TimeRange((*iter)->GetStartTime(), (*iter)->GetEndTime(),(SBH_DOUBLE)(*iter)->GetUsedQuantity()));
		}

		if ((*iter)->GetType() == WorkPlan_Lock)
		{
			mapBackupLock[(*iter)->GetPlanSelResId()].push_back(TimeRange((*iter)->GetStartTime(), (*iter)->GetEndTime(),(SBH_DOUBLE)(*iter)->GetUsedQuantity()));
			lockSt = (*iter)->GetStartTime();
			lockEd = (*iter)->GetEndTime();
		}
	}
	tsBackup.InsertAddPlan(nPreWorkId, workPlan_NextSet, mapBackupNexSet);
	tsBackup.InsertAddPlan(nPreWorkId, WorkPlan_Lock, mapBackupLock);

	WorkPtr& nxtWork = pData->GetAppendWork(nNxtWorkId)->GetWorkPtr();

	SBH_TIME stTm(0), endTm(0);
	AMAPI::GetMaxEndTime(workPtr, WorkPlan_Produce, stTm);
	switch (lockRes.front()->GetResLckType())
	{
	case 3:
		AMAPI::GetMinStartTime(nxtWork, WorkPlan_Produce, endTm);
		break;
	case 4:
		AMAPI::GetMaxEndTime(nxtWork, WorkPlan_Produce, endTm);
		break;
	default:
		break;
	}

	endTm += AMAPI::GetResLockTime(lockRes.front());
	const SBH_TIME nTmLen = endTm - stTm;

	if (nTmLen > 0 /*&& (lockEd <= endTm )*/)
	{
		map<SBH_ULONG,TimeRangeList>	mapBackupnxtWorkPtrSet;
		list<WorkSchPlanPtr> listNWP = nxtWork.GetSchPlanList();
		for (list<WorkSchPlanPtr>::iterator iter = listNWP.begin();
			iter != listNWP.end(); ++iter)
		{
			if ((*iter)->GetType() == WorkPlan_PreSet)
			{
				mapBackupnxtWorkPtrSet[(*iter)->GetPlanSelResId()].push_back(TimeRange((*iter)->GetStartTime(), (*iter)->GetEndTime(),(SBH_DOUBLE)(*iter)->GetUsedQuantity()));
			}

		}

		nxtWork.ClearSchPlan(WorkPlan_PreSet);
		pData->mgRemainCapacityDao.UnionFree(nNxtWorkId, WorkPlan_PreSet, mapBackupnxtWorkPtrSet);
		nxtWorkPreBackup.InsertAddPlan(nNxtWorkId, WorkPlan_PreSet, mapBackupnxtWorkPtrSet);

		// 撤销前工作的后设置，撤销资源能力
		if (!listWSP.empty())
		{
			workPtr.ClearSchPlan(workPlan_NextSet);
			workPtr.ClearSchPlan(WorkPlan_Lock);
			pData->mgRemainCapacityDao.UnionFree(workPtr->GetId(), workPlan_NextSet, mapBackupNexSet);
			pData->mgRemainCapacityDao.UnionFree(workPtr->GetId(), WorkPlan_Lock, mapBackupLock);
		}

		UnionFindMode unFindMode;
		SBH_INT nIndex = 0;
		for (list<Resource>::iterator iterRes = lockRes.begin();
			iterRes != lockRes.end(); ++iterRes, ++nIndex)
		{
			unFindMode.m_resCaps[iterRes->GetId()] = vecLockResCnt[nIndex];
		}
		list<WorkResRelaPtr>& listResRela = workPtr.GetWorkResRela();
// 		for (iterWRR = listResRela.begin(); iterWRR != listResRela.end(); ++iterWRR)
// 		{
// // 			set<int> setResIds;
// // 			workPtr->GetPlanResList(setResIds);
// 			if (/*setResIds*/lockRes.find(iterWRR->GetRes()) != /*setResIds*/lockRes.end())
// 				unFindMode.m_resCaps[iterWRR->GetResId()] = iterWRR->GetPdtResCnt();
// 		}
		//unFindMode.m_interrupt = -1;//不可中断
		unFindMode.m_time = stTm;
		unFindMode.m_len = nTmLen;
		unFindMode.m_bLock = true;
		unFindMode.m_nType = TYPE_LOCKTM;
		if (!pData->mgRemainCapacityDao.UnionFindIdleF2(unFindMode, Dir_Obverse))
		{
			m_tmRSchBound = endTm;
			tsBackup.FinallySurePlan();
			nxtWorkPreBackup.FinallySurePlan();

			if (bAdjust)
			{
				//判断两个工作级别不做调整
				if (!(workPtr->IsNonSchWorkA() || workPtr->IsNonSchWorkB() || workPtr->IsNonSchWorkC()
					||workPtr->IsSchWorkA() || workPtr->IsSchWorkB() || workPtr->IsSchWorkC()
					||nxtWork->IsNonSchWorkA() || nxtWork->IsNonSchWorkB() || nxtWork->IsNonSchWorkC()
					||nxtWork->IsSchWorkA() || workPtr->IsSchWorkB() || nxtWork->IsSchWorkC()))
				{
					bOK = ResLockedFailedAdjust(unFindMode,workPair,stTm,endTm);
				}
			}
		}
		else
		{
			nxtWorkPreBackup.FinallySurePlan();
			map<SBH_ULONG,TimeRangeList> mapNew;
			set<int> setResIds;
			workPtr->GetPlanResList(setResIds);
			//list<WorkResRelaPtr>& listResRela = workPtr.GetWorkResRela();

			// 设置后设置
			SBH_ULONG workId = pData->mgRemainCapacityDao.GetNearByWork(workResInfo.resRelaID.GetResId(), endTm, Dir_Obverse);
	
			TimeLenInfo	preTm, nexTm;
			SBH_TIME rtStTm, rtEndTm;
			SingleCalcV3 singleCal;
			TestSchedulerInfo tsInfo;
			singleCal.GetSetTimes(workId,workPtr, workResInfo, Dir_Obverse, preTm, nexTm);
			if (singleCal.VaildTmLen(nexTm))
			{
				if (singleCal.CalWorkTime(workPtr, workResInfo, TYPE_POSTSET, endTm
					,nexTm, tsInfo, rtStTm, rtEndTm, FUC_F2, Dir_Obverse))
				{
					tsInfo.SurePlan();

					for (iterWRR = listResRela.begin(); iterWRR != listResRela.end(); ++iterWRR)
					{
						if (find(lockRes.begin(),lockRes.end(),iterWRR->GetRes()) != lockRes.end())
						{
							ScheduleHelper::InsertWorkSchPlanItem(workPtr, iterWRR->GetRes(), iterWRR->GetResSelector(), WorkPlan_Lock, stTm, endTm, iterWRR->GetPdtResCnt(), 0);
							mapNew[iterWRR->GetResId()].push_back(TimeRange(stTm, endTm, iterWRR->GetPdtResCnt()));
						}
					}
					pData->mgRemainCapacityDao.UnionUse(workPtr.GetId(), WorkPlan_Lock, mapNew);
				}
				else
				{
					tsInfo.ClearPlan();
					TestSchedulerInfo tsInfo1;
					if (singleCal.CalWorkTime(workPtr, workResInfo, TYPE_POSTSET, rtEndTm
						,nexTm, tsInfo1, rtStTm, rtEndTm, FUC_F2, Dir_Converse))
					{
						tsInfo1.SurePlan();

						for (iterWRR = listResRela.begin(); iterWRR != listResRela.end(); ++iterWRR)
						{
							if (find(lockRes.begin(),lockRes.end(),iterWRR->GetRes()) != lockRes.end())
							{
								ScheduleHelper::InsertWorkSchPlanItem(workPtr, iterWRR->GetRes(), iterWRR->GetResSelector(), WorkPlan_Lock, stTm, rtStTm, iterWRR->GetPdtResCnt(), 0);
								mapNew[iterWRR->GetResId()].push_back(TimeRange(stTm, rtStTm, iterWRR->GetPdtResCnt()));
							}
						}
						pData->mgRemainCapacityDao.UnionUse(workPtr.GetId(), WorkPlan_Lock, mapNew);
					}
					else
						tsBackup.FinallySurePlan();
				}
			}
			else
			{
				for (iterWRR = listResRela.begin(); iterWRR != listResRela.end(); ++iterWRR)
				{
					//if (curRes->GetId() == iterWRR->GetResId())
					if (find(lockRes.begin(),lockRes.end(),iterWRR->GetRes()) != lockRes.end())
					{
						ScheduleHelper::InsertWorkSchPlanItem(workPtr, iterWRR->GetRes(), iterWRR->GetResSelector(), WorkPlan_Lock, stTm, endTm, iterWRR->GetPdtResCnt(), 0);
						mapNew[iterWRR->GetResId()].push_back(TimeRange(stTm, endTm, iterWRR->GetPdtResCnt()));
					}
				}
				pData->mgRemainCapacityDao.UnionUse(workPtr.GetId(), WorkPlan_Lock, mapNew);
			}
		}
	}

	pData->parame.m_curSchMethod = temp;

	return bOK;
}

SBH_BOOL CLockedDispose::ResLockedPairProNeedAdj(list<vector<SBH_INT> >::iterator& iterStart, list<vector<SBH_INT> >& listPairs)
{
	assert(iterStart != listPairs.end());
	
	m_bAdjustFailed = false;
	do 
	{
		if (false == ResLockedPairPro(*iterStart, true))
		{
			if (m_bAdjustFailed)
			{
				iterStart = listPairs.erase(iterStart);
				return false;
			}

			// 检测出循环处理
			if (m_bAdjusted && m_iterCur == iterStart)
			{
				//map<SBH_INT,SBH_TIME>	templastTime;
				//for(list<vector<SBH_INT> >::iterator iterAllWK = listPairs.begin();iterAllWK != listPairs.end();++iterAllWK)
				//{
				//	WorkPtr& workPtr = pData->GetAppendWork((*iterAllWK)[0])->GetWorkPtr();
				//	SBH_TIME stTm(0);
				//	if(AMAPI::GetMinStartTime(workPtr,TYPE_PRODUCETM,stTm))
				//		templastTime[(*iterAllWK)[0]] = stTm;
				//	WorkPtr& nxtWorkPtr = pData->GetAppendWork((*iterAllWK)[1])->GetWorkPtr();
				//	if(AMAPI::GetMinStartTime(nxtWorkPtr,TYPE_PRODUCETM,stTm))
				//		templastTime[(*iterAllWK)[1]] = stTm;
				//}

				//if(m_lastTime != templastTime)
				//	m_lastTime = templastTime;
				//else
				//m_bFixNexWK = true;

				m_bFixNexWK = true;
			}

			m_bAdjusted = true;
			if (iterStart == listPairs.begin())
				break;
			--iterStart;
		}
		else
		{
			// 没有出现循环处理。
 			if (m_iterCur == iterStart && m_bAdjusted)
			{
				m_bFixNexWK = false;
 				m_bAdjusted = false;
			}
			break;
		}
	} while (true);

	return true;
}

void CLockedDispose::ReSchWork( WorkPtr& work,SBH_TIME limitTm ,DirType dir)
{
	if(dir == Dir_Obverse)
	{
		SingleObvCalc singleObv;
		bool fixFlg = work.GetResFix();
		//work.SetIsFixRes(true);
		singleObv.SingleObvSchedule(work.GetId(),limitTm);
		//work.SetIsFixRes(fixFlg);
	}
	else
	{
		SingleConvCalc singleConv;
		bool fixFlg = work.GetResFix();
		//work.SetIsFixRes(true);
		singleConv.SingleConvSchedule(work.GetId(),limitTm);
		//work.SetIsFixRes(fixFlg);
	}
}

void CLockedDispose::GetSchedPreOrPostWorks(AppendWork* pCurWork,list<int>& workList,DirType dir)
{
	if(pCurWork == NULL)	return;
	map<WorkPriRelaPtr, Work*> mapWorks;
	//正向
	if(dir == Dir_Obverse)
	{
		AMAPI::GetPostWorks(pCurWork->GetWorkPtr(), mapWorks);
	}
	else
	{
		AMAPI::GetPreWorks(pCurWork->GetWorkPtr(), mapWorks);
	}

	for (map<WorkPriRelaPtr, Work*>::iterator iter = mapWorks.begin();
		iter != mapWorks.end(); ++iter)
	{
		AppendWork* pDiffWork = pData->GetAppendWork(iter->second->GetId());
		// 处于不同级别的工作，进行约束设置。
		if (pDiffWork && pDiffWork->GetISSchedFlg())
		{
			workList.push_back(pDiffWork->GetWorkId());
			GetSchedPreOrPostWorks( pDiffWork, workList, dir);
		}
	}
}

void CLockedDispose::FreeCurLevelWorkListCap(list<SBH_INT>& workList)
{
	for(list<SBH_INT>::iterator iter = workList.begin();
		iter != workList.end();++iter)
	{
		AppendWork* pWork = pData->GetAppendWork(*iter);
		if(pWork && pWork->GetSchLevel() == AMAPI::GetSchLevel())
		{
			//pData->FreeWorkCap(*iter);
			pWork->GetWorkPtr()->BackupWorkPlan();
			pData->RetractWorkPlan(*iter,true);
		}
	}
}

void CLockedDispose::ReSchWorkList(list<int>& workList,SBH_TIME& limitTm, DirType dir)
{
	for(list<SBH_INT>::iterator iter = workList.begin();
		iter != workList.end();++iter)
	{
		AppendWork* pWork = pData->GetAppendWork(*iter);
		if(pWork && pWork->GetSchLevel() == AMAPI::GetSchLevel())
		{
			ReSchWork(pWork->GetWorkPtr(),limitTm,dir);
		}
	}

}

SBH_BOOL CLockedDispose::ResLockedFailedAdjust(UnionFindMode& unFindMode,vector<SBH_INT>& workPair,SBH_TIME& stTm,SBH_TIME& endTm)
{
	if (m_nAdjustCount > 10) 
	{
		m_bAdjustFailed = true;
		return false;
	}
	++m_nAdjustCount;
	const SBH_INT nPreWorkId = workPair[0];
	const SBH_INT nNxtWorkId = workPair[1];

	AppendWork* pPreAppWork = pData->GetAppendWork(nPreWorkId);
	AppendWork* pNxtAppWork = pData->GetAppendWork(nNxtWorkId);

	DirType dir = Dir_Obverse;
	if(AMAPI::GetDirectionFlg() == Sch_Dir_Force_Conv)
	{
		dir = Dir_Converse;
	}


	//若同资源后工作是工艺后工作，退出
	list<SBH_INT> workIdList;
	for(map<SBH_ULONG,SBH_DOUBLE>::iterator iter = unFindMode.m_resCaps.begin();
		iter != unFindMode.m_resCaps.end();iter++)
	{
		pData->mgRemainCapacityDao.GetResWorkList(iter->first,workIdList,stTm,endTm);
	}
	//两工作间没有其他工作，则无法调节

	workIdList.remove(nPreWorkId);
	workIdList.remove(nNxtWorkId);
	if(workIdList.empty())
	{
		return true;
	}

	//求出两工作所有已排后工作
	list<SBH_INT> workList1,workList2;
	GetSchedPreOrPostWorks(pPreAppWork,workList1,dir);
	GetSchedPreOrPostWorks(pNxtAppWork,workList2,dir);

	//将所有后工作合并到一起
	list<SBH_INT>::iterator iterPos = workList1.begin();
	for(list<SBH_INT>::iterator iter2 = workList2.begin();
		iter2 != workList2.end();)
	{
		list<SBH_INT>::iterator iter1 = find(iterPos,workList1.end(),*iter2);
		if(iter1 != workList1.end())
		{
			iterPos = iter1;
			workList1.insert(iterPos,workList2.begin(),iter2);
			iter2 = workList2.erase(iter2);
		}
		else
		{
			++iter2;
		}
	}
	workList1.remove(nPreWorkId);
	workList1.remove(nNxtWorkId);

	if (find_first_of(workIdList.begin(), workIdList.end(), workList1.begin(), workList1.end()) != workIdList.end())
	{
		m_bAdjustFailed = true;
		return false;
	}

	//清理所有后工作
	FreeCurLevelWorkListCap(workList1);

	WorkPtr& workPtr = pData->GetAppendWork(nPreWorkId)->GetWorkPtr();

	WorkPtr& nxtWork = pData->GetAppendWork(nNxtWorkId)->GetWorkPtr();

	if(dir == Dir_Converse)
	{
		//逆向排程
		SBH_TIME newSt(MAXTIME),tempSt(MAXTIME);
		for(list<SBH_INT>::iterator iterWorkList = workIdList.begin();
			iterWorkList != workIdList.end();++iterWorkList)
		{
			if (false == AMAPI::GetMinStartTime(AMAPI::GetWorkPtrMap()->Get(*iterWorkList), WorkPlan_PreSet, tempSt))
				AMAPI::GetMinStartTime(AMAPI::GetWorkPtrMap()->Get(*iterWorkList), WorkPlan_Produce, tempSt);
			
			newSt = min(tempSt,newSt);
		}

		WorkResInfo wkResInfo;
		nxtWork.GetPlanResRelaCom(wkResInfo.resRelaID, wkResInfo.assistResRelaList);
		assert(wkResInfo.resRelaID.ValidPointer());
		TimeLenInfo tmLen;
		SingleCalcV3 singleCal;
		singleCal.GetPrdTm(nxtWork, wkResInfo, tmLen);
		SBH_TIME rtStTm(MAXTIME), rtEndTm(MAXTIME), tmPreEndBK(0), tmNexEndBK(0);
		TestSchedulerInfo tsInfo;
		singleCal.CalWorkTime(nxtWork, wkResInfo, TYPE_PRODUCETM, newSt
			,tmLen, tsInfo, rtStTm, rtEndTm, FUC_F2, Dir_Obverse, false, false);

		AMAPI::GetMaxEndTime(workPtr, WorkPlan_Produce, tmPreEndBK);
		AMAPI::GetMaxEndTime(nxtWork, WorkPlan_Produce, tmNexEndBK);
		SingleConvCalc singleConv;
		if (false == singleConv.SingleConvSchWKPair(nNxtWorkId, nPreWorkId, rtEndTm, m_bFixNexWK))
		{
			ScheduleMethod temp = pData->parame.m_curSchMethod;
			pData->parame.m_curSchMethod = SM_IGNORE;
			singleConv.SingleConvSchedule(nNxtWorkId, tmNexEndBK);
			singleConv.SingleConvSchedule(nPreWorkId, tmPreEndBK);
			pData->parame.m_curSchMethod = temp;

			pPreAppWork->SetISSucessedFlg(true);
			pNxtAppWork->SetISSucessedFlg(true);
			m_bAdjustFailed = true;
			return false;
		}


	}
	else	//	if(AMAPI::GetDirectionFlg() == Sch_Dir_Force_Conv)
	{
		//正向排程
		SBH_TIME newSt(0),tempSt(0);
		for(list<SBH_INT>::iterator iterWorkList = workIdList.begin();
			iterWorkList != workIdList.end();++iterWorkList)
		{
			if (false == AMAPI::GetMaxEndTime(AMAPI::GetWorkPtrMap()->Get(*iterWorkList), workPlan_NextSet, tempSt))
				AMAPI::GetMaxEndTime(AMAPI::GetWorkPtrMap()->Get(*iterWorkList), WorkPlan_Produce, tempSt);
			newSt = max(tempSt,newSt);
		}

		WorkResInfo wkResInfo;
		workPtr.GetPlanResRelaCom(wkResInfo.resRelaID, wkResInfo.assistResRelaList);
		assert(wkResInfo.resRelaID.ValidPointer());
		TimeLenInfo tmLen;
		SingleCalcV3 singleCal;
		singleCal.GetPrdTm(workPtr, wkResInfo, tmLen);
		SBH_TIME rtStTm(0), rtEndTm(0), tmPreBegBK(0), tmNexBegBK(0);
		TestSchedulerInfo tsInfo;
		singleCal.CalWorkTime(workPtr, wkResInfo, TYPE_PRODUCETM, newSt
			,tmLen, tsInfo, rtStTm, rtEndTm, FUC_F2, Dir_Converse, false, false);

		AMAPI::GetMinStartTime(workPtr, WorkPlan_Produce, tmPreBegBK);
		AMAPI::GetMinStartTime(nxtWork, WorkPlan_Produce, tmNexBegBK);
		SingleObvCalc singleObv;
		if (false == singleObv.SingleObvSchWKPair(nPreWorkId, nNxtWorkId, rtStTm, m_bFixNexWK))
		{
			ScheduleMethod temp = pData->parame.m_curSchMethod;
			pData->parame.m_curSchMethod = SM_IGNORE;
			singleObv.SingleObvSchedule(nPreWorkId, tmPreBegBK);
			singleObv.SingleObvSchedule(nNxtWorkId, tmNexBegBK);
			pData->parame.m_curSchMethod = temp;

			pPreAppWork->SetISSucessedFlg(true);
			pNxtAppWork->SetISSucessedFlg(true);
			m_bAdjustFailed = true;	
			return false;
		}

	}

	ResLockedPairPro(workPair, true);

	//求取两工作的计划
	if(!workList1.empty())
	{
		SBH_TIME limitTm(0);
		//逆向
		if(dir == Dir_Converse)
		{
			AMAPI::GetMaxEndTime(nxtWork,TYPE_PRODUCETM,limitTm);
		}
		else	//正向
		{
			AMAPI::GetMinStartTime(workPtr,TYPE_PRODUCETM,limitTm);
		}

		ReSchWorkList(workList1,limitTm,dir);

	}

	--m_nAdjustCount;
	return false;
}