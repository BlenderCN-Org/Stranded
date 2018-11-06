
#define SHBAPI_EXPORT

#include "SBHAPI.h"
#include "GlobalVarSBH.h"
#include "AM_API.h"
#include "CalcBaseStruct.h"
#include "SchAssistantFuncs.h"
#include "Csv.h"
#include "SingleObvCalc.h"
#include "SingleConvCalc.h"
#include "SBHDragV3.h"
#include "MainFlow.h"
#include "Scheduler.h"
#include "MaxIntervalDisposeManager.h"
#include "LockedDispose.h"
#include "SingleCalcV3.h"

#ifndef  _BZERO_X
#define  _BZERO_X
#define BZERO(x) ((x) > -0.0000001 && (x) < 0.0000001)
#endif

#ifdef _DEBUG_USING_CLOCK
#include "clock.h"
static Timer T_RunSBH(L"RunSBH");
#define CLOCK_MARK(timer) timer.Pause();
#else
#define CLOCK_MARK(timer) 
#endif

void  SetSlider(int* currProcess)
{
	if (pData != NULL && currProcess != NULL)
	{
		pData->SetSBHProcess(currProcess);
	}
}

bool StopSBH()
{
	stop = true;
	return true;
}

DWORD GetProgressSBH()
{
	DWORD progress = 0;
	double rate = 0;
	if(pData != NULL)
	{
		double timeRate = 0;
		double loopRate = 0;
		double stepRate = 0;

		if(pData->parame.sbhMaxRunTime > 0)
		{
			timeRate = (double)pData->parame.runTime / pData->parame.sbhMaxRunTime;
		}
		if(pData->parame.machinegroupnumber > 0)
		{
			loopRate = (double)pData->parame.loopNumber / pData->parame.sbhMaxLoopNumber;
		}
		if(pData->parame.sbhMaxStep > 0)
		{
			stepRate = (double)pData->parame.step / pData->parame.sbhMaxStep;
		}
		rate = max(timeRate, max(loopRate, stepRate));		
	}
	if(rate > 1)
	{
		rate = 1;
	}
	rate *= 100;
	if(pData == NULL)
	{
		rate = 100;
	}
	WORD loword = MAKEWORD(rate, 0);
	progress = (DWORD)MAKEWPARAM(loword, 100);
	return progress;
}

//动态分配全局变量
bool CreateGlobalVar()
{
	if(pData == NULL)
	{
		CMemMgr::Initialize();
		pData = new DataManager;
		if(pData != NULL)
		{
			return true;
		}
	}
	return false;
}

//释放全局变量
bool FreeGlobalVar()
{
	if(pData != NULL)
	{
		CMemMgr::Clear();
		pData->ClearMap();
		delete pData;
		pData = NULL;

		CMaxIntervalDisposeManager* pMDM = CMaxIntervalDisposeManager::GetMaxDisManager();
		pMDM->MaxDisManagerRelese();
		return true;
	}
	return false;
}



void SetResUpdateWorks(const list<int>& resList)
{
	ResourcePtrMap* pmres = AMAPI::GetResourcePtrMap();	

	list<int> resourceList;
	resourceList = resList;
	list<int>::iterator iterRes, iterSecRes;

	//将resList的上层或下层资源放入resList中
	iterRes = resourceList.begin();
	while (iterRes != resourceList.end())
	{
		int topResId = 0;
		ResourcePtr ptrRes = pmres->Get(*iterRes);
		if (ptrRes != NULL)
		{
			topResId = ptrRes->GetTopResId();
			if (topResId > 0)
			{
				iterSecRes = std::find(resourceList.begin(), resourceList.end(), topResId);
				if (iterSecRes == resourceList.end())
				{
					resourceList.push_back(topResId);
				}
			}
			list<ResourcePtr>& bottomResList = ptrRes->GetBottomResList();
			list<ResourcePtr>::iterator iterBottom;
			if (!bottomResList.empty())
			{
				iterBottom = bottomResList.begin();
				while (iterBottom != bottomResList.end())
				{
					if (*iterBottom != NULL)
					{
						iterSecRes = std::find(resourceList.begin(), resourceList.end(), (*iterBottom)->GetId());
						if (iterSecRes == resourceList.end())
						{
							resourceList.push_back((*iterBottom)->GetId());
						}
					}
					iterBottom++;
				}
			}

		}
		iterRes++;
	}
	iterRes = resourceList.begin();
	while (iterRes != resourceList.end())
	{
		ResourcePtr resPtr = pmres->Get(*iterRes);
		if (resPtr != NULL)
		{
			pData->mgRemainCapacityDao.UpdateResCalendar(resPtr->GetId());
			
		}
		iterRes++;
	}
}

void SetRelevantWorks(const list<int>& relevantWorks)
{
	//int schType = AMAPI::GetParameter()->GetSchType();
	////调整功能
	//if (schType == 2)
	//{
	//	//SetAdjustWorks(relevantWorks);
	//}
	////紧急插单功能
	//else if (schType == 3)
	//{
	//	//SetFlyWorks(relevantWorks);
	//}
	////加班功能
	//else if (schType == 4) 
	//{
	//	//SetOverTimeWorks(relevantWorks);
	//}
	////变更资源（工厂日历、资源删除、资源增加）
	//else if (schType == 5)
	{
		SetResUpdateWorks(relevantWorks);
	}
}


//界面拖动函数
//1.orgMouseStartTm为上次鼠标位置
//2.newMouseStartTm为当前鼠标位置
//3.<mainResId, assistResList>为workId为指定资源
//4.newSchPlanList为新的排程计划
//5.reltIntervlRate为相对‘前设置开始时刻’百分比：在某次拖动过程中，第一次调时，初始值为-1；以后调用，都采用第一次的返回值；
bool GetMoveWorkSchPlan(int workId, int mainResId, int origResId, 
						time_t orgMouseStartTm, time_t newMouseStartTm
						, list<int>& newSchPlanList, double& reltIntervlRate,int nGridRange)
{
	int origType = AMAPI::GetParameter()->GetMouseMoveType();

	if(AMAPI::GetParameter()->GetSystemSchFlg() != Refined_Sch)	//非精细排程
	{
		pData->parame.m_curSchMethod = SM_INFINITY;
	}
	else
	{
		pData->parame.m_curSchMethod = SM_LIMITARY;
	}

	bool rtFlg = GetMoveWorkSchPlanInner( workId,  mainResId,  origResId, 
		 orgMouseStartTm,  newMouseStartTm, newSchPlanList,  reltIntervlRate,nGridRange);
	/*bool rtFlg =GetNewMoveWorkSchPlanInner(  workId,  mainResId,   orgMouseStartTm,  newMouseStartTm, 
		 newSchPlanList,  reltIntervlRate );*/

	AMAPI::GetParameter()->SetMouseMoveType(origType);

	return rtFlg;
}

//  左插入/右插入需求：拖动结束，释放鼠标后，调用该函数；
//  表格拖动；
//	workId为拖动工作
//	mouseResId为拖动结束后鼠标落点所在资源
//  mainResId为拖动工作将要排程的主资源,仅为“表格拖动”使用，其它情况为－1;
//  assistResList为拖动工作将要排程的副资源列表,仅为“表格拖动”使用，其它情况为－1;
//	mouseStartTm为拖动结束后鼠标落点所在时刻，仅为“表格拖动”使用，其它情况为－1；
//  gridStartTm为鼠标落点所在单元格的开始时刻，仅为“表格拖动”使用，其它情况为－1；
//  gridEndTm为鼠标落点所在单元格的结束时刻，仅为“表格拖动”使用，其它情况为－1；
//	newSchPlanList为新的排程计划
bool GetPushWorkSchPlan(int workId, int mouseResId, int origResId, list<int>& assistResList, time_t mouseStartTm, time_t gridStartTm, time_t gridEndTm, list<int>& newSchPlanList)
{
	return GetNewPushWorkSchPlan( workId,  mouseResId, origResId, mouseStartTm,  gridStartTm,  gridEndTm,  newSchPlanList);
}
bool RunSBH(HWND pMainWnd /* = NULL */, bool bMaintain /* = false */)
{
	CLOCK_MARK(T_RunSBH)
	bool bOK = false;
	CMainFlow mainFlow;
	if (bMaintain)
		bOK = mainFlow.ScheduleMaintain(pMainWnd);
	else
		bOK = mainFlow.Schedule(pMainWnd);
	CLOCK_MARK(T_RunSBH)
	return bOK;
}

bool RunScheduleByCalendar()
{
	CMainFlow mainFlow;
	return mainFlow.ScheduleByCalendar();
}


//void DeleteAlgPlanList(list<int>& workList)
//{
//	CLOCK_MARK(gDeleteAlgPlanList);
//	WorkPtrMap* pmwork = AMAPI::GetWorkPtrMap();
//	WorkSchPlanPtrMap* pmworksch = AMAPI::GetWorkSchPlanPtrMap();
//
//	list<int>::iterator iterWork = workList.begin();
//	while (iterWork != workList.end())
//	{
//		WorkPtr& ptrwork = pmwork->Get(*iterWork);
//		if (ptrwork != NULL && (ptrwork->GetSchedulerFlg() == SCHEDULEREDNOPLANT || ptrwork->GetSchedulerFlg() == SCHEDULINGWILLORDER) && ptrwork->GetType() < 3)
//		{
//			list<int> tempWorkList;
//			tempWorkList.push_back(*iterWork);
//			list<WorkSchPlanPtr> currWorkPlanList = ptrwork->GetSchPlanList();
//			list<WorkSchPlanPtr>::iterator iterCurrWorkPlan = currWorkPlanList.begin();
//			while (iterCurrWorkPlan != currWorkPlanList.end())
//			{
//				if (*iterCurrWorkPlan != NULL)
//				{
//					if ((*iterCurrWorkPlan)->GetPlanSelResId() > 0 && (*iterCurrWorkPlan)->GetUsedQuantity() > 0)
//					{						
//						//删除剩余能力
//						pData->mgRemainCapacityDao.Resume(tempWorkList, (*iterCurrWorkPlan)->GetPlanSelResId(), (*iterCurrWorkPlan)->GetStartTime(), 
//							(*iterCurrWorkPlan)->GetEndTime(), (*iterCurrWorkPlan)->GetUsedQuantity(), (*iterCurrWorkPlan)->GetType());
//					}
//					AMAPI::ClearBefoDelWorkSchPlan(*iterCurrWorkPlan);
//					//删除工作排程的排程计划
//					pmworksch->Delete(*iterCurrWorkPlan);
//				}
//				iterCurrWorkPlan++;
//			}
//		}
//		iterWork++;
//	}
//	CLOCK_MARK(gDeleteAlgPlanList);
//}

bool DrawOrCompWorkPrud( int workId, time_t startTm, time_t endTm, list<int>& newSchPlanList )
{
	
	return DrawWorkLen(  workId,  startTm,  endTm,  newSchPlanList );
}


#include "MainFlow.h"

//排程计算
bool RunSBHV3()
{
	return false;
}

bool GetMoveWorkSchPlanInner( int workId, int mainResId, int origResId, time_t orgMouseStartTm, time_t newMouseStartTm, 
							 list<int>& newSchPlanList, double& reltIntervlRate ,int nGridRange)
{

	return GetNewMoveWorkSchPlanInner( workId,  mainResId, origResId,  orgMouseStartTm,  newMouseStartTm, 
		newSchPlanList,  reltIntervlRate ,nGridRange);

}
bool GetNewMoveWorkSchPlanInner( int workId, int resId,int origResId,  time_t orgMouseStartTm, time_t newMouseStartTm, 
							 list<int>& newSchPlanList, double& reltIntervlRate ,int nGridRange)
{


	CSBHDragV3* pDrag = CSBHDragV3::Get();
    pDrag->SetWorkResId(workId,resId,origResId);
    pDrag->InitWorkPtr();

	if (!pDrag->IfValidPtr()) return false;

	pDrag->SetSameRes(false);

	if(orgMouseStartTm!=newMouseStartTm)
		pDrag->SetSameRes();
	pDrag->GetChangeRes(resId,origResId);

	//无可用资源组合
	if(resId == 0)
		return false;


      
	if (reltIntervlRate==-1)
	{
		reltIntervlRate=pDrag->CalTimeLen(orgMouseStartTm, newMouseStartTm);
		reltIntervlRate =0;
	}

	//工作排程标志、时间范围是否在工作时间范围内
	if (!pDrag->IfDrag()
		/*||!pDrag->IfSchedulerTimeRand(orgMouseStartTm)*/)
	{
		return false;
	}


	if (orgMouseStartTm==newMouseStartTm&&pDrag->IfSameRes())//时间相同,资源相同
	{
		return  false;
	}
   
	/*判断资源是否在上班时间段*/
	list<SBH_INT>combList = pDrag->GetResComb();
	for ( list<SBH_INT>::iterator iter =combList.begin();
		iter !=combList.end(); iter++)
	{
		if (!pData->mgRemainCapacityDao.IfOffDutyTimePoint(*iter/*resId*/,newMouseStartTm))  //是否在下班时间段
		{
			return false;
		}
	}


    if (pDrag->StartScheduler(orgMouseStartTm, newMouseStartTm,newSchPlanList,reltIntervlRate,nGridRange))
	{
		AppendWork* pCurAppWork = pData->GetAppendWork(workId);
		if(pCurAppWork != NULL )
		{
			pCurAppWork->SetISSchedFlg(true);
			pCurAppWork->SetISSucessedFlg(true);

			pDrag->UpdateTakeTimePoint(newSchPlanList);

			Work* pPreWork = &pCurAppWork->GetWorkPtr();
			list<Work*> listPreWork;
			pPreWork->GetPreworks(listPreWork);
			for (list<Work*>::iterator iterPre = listPreWork.begin();
				iterPre != listPreWork.end(); ++iterPre)
			{
				pPreWork = *iterPre;
				ProDragLocked(pPreWork);
			}

			ProDragLocked(&pCurAppWork->GetWorkPtr());
		}
		
		return true;
	}
	
	return false;
}

//表格拖动功能函数
bool GetNewPushWorkSchPlan(int workId, int mouseResId, int origResId,time_t mouseStartTm, time_t gridStartTm, time_t gridEndTm, list<int>& newSchPlanList)
{
	CSBHDragV3* pDrag = CSBHDragV3::Get();
	pDrag->SetWorkResId(workId,mouseResId);
	pDrag->InitWorkPtr();
	if (!pDrag->IfValidPtr()) return false;

	//设置资源是否为不同资源
	if(mouseResId!=origResId)
		pDrag->SetSameRes(false);
	else
		pDrag->SetSameRes(true);

	pDrag->GetChangeRes(mouseResId,origResId);


	//工作排程标志、时间范围是否在工作时间范围内
	if (!pDrag->IfDrag())
	{
		return false;
	}
	if (mouseStartTm==-1)
	{
		if (AMAPI::GetParameter()->GetMouseMoveType()==4)
		{
			//左插入
			 if ( pDrag->LeftInsert( newSchPlanList) )
			 {
				 pDrag->UpdateTakeTimePoint(newSchPlanList);
				 return true;
			 }
			 else
				 return false;
			 
			//return pDrag->LeftInsert( newSchPlanList);
		}
		else
		{
			//右插入
			if( pDrag->RightInsert(  newSchPlanList) )
			{
				pDrag->UpdateTakeTimePoint(newSchPlanList);
				return true;
			}
			else
				return false;
			//return pDrag->RightInsert(  newSchPlanList);
		}
	}
	else
	{
		if (pDrag->TblDrag( mouseStartTm,  gridStartTm,  gridEndTm,  newSchPlanList))
		{
			pDrag->UpdateTakeTimePoint(newSchPlanList);
			return true;
		}
		else
			return false;

           //return pDrag->TblDrag( mouseStartTm,  gridStartTm,  gridEndTm,  newSchPlanList);
	}

}

//工作制造长度拖动
bool DrawWorkLen( int workId, time_t startTm, time_t endTm, list<int>& newSchPlanList )
{
	CSBHDragV3* pDrag = CSBHDragV3::Get();
	pDrag->SetWorkResId(workId,0);
	pDrag->InitWorkPtr();
	if (!pDrag->IfValidPtr()) return false;
    return pDrag->DrawWorkLen( startTm, endTm,newSchPlanList);
}
//工作拆分
bool GetSplitWorkPlan(SBH_INT srcWorkId,SBH_INT fDevWorkId,SBH_INT sDevWorkId,double rate, list<SBH_INT>& newSchPlanList)
{
	CSBHDragV3* pDrag = CSBHDragV3::Get();
	return pDrag->GetSplitWorkPlan(srcWorkId,fDevWorkId,sDevWorkId,rate, newSchPlanList);
}
//工作合并
bool GetCombWorkPlan(int srcWorkId,double rate,list<unsigned long> devWorkIdList,  list<int>& newSchPlanList,bool combParent)
{
	CSBHDragV3* pDrag = CSBHDragV3::Get();
	return pDrag->GetCombWorkPlan(srcWorkId,rate, devWorkIdList, newSchPlanList,combParent);
}
//订单分割
bool GetSplitOrderPlan(map<unsigned long,unsigned long> parallelismWkMap, list<int>&newSchPlanList)
{
	CSBHDragV3* pDrag = CSBHDragV3::Get();
	return pDrag->GetSplitOrderPlan(parallelismWkMap, newSchPlanList);
}
bool GetCombOrderPlan(unsigned long srcOrderId,list<unsigned long> orderList , list<int>&newSchPlanList)
{
	CSBHDragV3* pDrag = CSBHDragV3::Get();
	return pDrag->GetCombOrderPlan(srcOrderId, orderList,newSchPlanList);
}

//加班功能
bool SBHOverTime( int workId,list<int>& newSchPlanList )
{
	AppendWork* pComWork = pData->GetAppendWork(workId);
	if(pComWork == NULL)	return false;
	SchLevelType tempType = pComWork->GetSchLevel();
	pComWork->SetSchLevel(SL_Level3);
	
	if(pComWork != NULL 
		&& ( pComWork->GetSchLevel() == SL_Level3 || pComWork->GetSchLevel() == SL_Level4 )
		//&& pComWork->GetISSchedFlg()
		//&& !pComWork->GetISSucessedFlg()
		&& AMAPI::IsParticipateSchWork( workId))
	{
		WorkPtr& workPtr  = pComWork->GetWorkPtr();
		if (workPtr == NULL)
		{
			pComWork->SetSchLevel(tempType);
			return false;
		}
		//检查违反约束并获取违反约束处理方式,加班
		pData->parame.m_curSchMethod = SM_OVERTIME;
		if(pComWork->GetISSchedFlg())
		{
			pComWork->SetISSchedFlg(false);
			pComWork->SetISSucessedFlg(false);
		}
		//备份排程计划
		list<WorkSchPlanPtr> schPlanIDList = workPtr->GetSchPlanList();

		//违反约束排程
		CScheduler scheduling;
		scheduling.CommonSchdule(workId,pComWork->GetDirection());
		//如果排程依旧失败
		if(!pComWork->GetISSucessedFlg())
		{
			pComWork->SetSchLevel(tempType);
			return false;
		}
		map<SBH_ULONG,TimeRangeList> prePlanList;	//前设置
		map<SBH_ULONG,TimeRangeList> prdPlanList;	//制造
		map<SBH_ULONG,TimeRangeList> postPlanList;	//后设置
		map<SBH_ULONG,TimeRangeList> lockPlanList;	//锁定
		//删除工作原排程计划和资源量
		for(list<WorkSchPlanPtr>::iterator iter = schPlanIDList.begin();
			iter != schPlanIDList.end();iter++)
		{
			WorkSchPlanPtr& workPlan = *iter;
			if(workPlan->GetType() == TYPE_PRESET)
			{
				prePlanList[workPlan->GetPlanSelResId()].push_back(TimeRange(workPlan->GetStartTime(),workPlan->GetEndTime(),(double)workPlan->GetUsedQuantity()));
			}
			else if(workPlan->GetType() == TYPE_PRODUCETM)
			{
				prdPlanList[workPlan->GetPlanSelResId()].push_back(TimeRange(workPlan->GetStartTime(),workPlan->GetEndTime(),(double)workPlan->GetUsedQuantity()));
			}
			else if(workPlan->GetType() == TYPE_POSTSET)
			{
				postPlanList[workPlan->GetPlanSelResId()].push_back(TimeRange(workPlan->GetStartTime(),workPlan->GetEndTime(),(double)workPlan->GetUsedQuantity()));
			}
			else if(workPlan->GetType() == TYPE_LOCKTM)
			{
				lockPlanList[workPlan->GetPlanSelResId()].push_back(TimeRange(workPlan->GetStartTime(),workPlan->GetEndTime(),(double)workPlan->GetUsedQuantity()));
			}
		}
		if(!prdPlanList.empty())
		{
			pData->mgRemainCapacityDao.UnionFree(workId,TYPE_PRODUCETM,prdPlanList);
		}
		if(!postPlanList.empty())
		{
			pData->mgRemainCapacityDao.UnionFree(workId,TYPE_POSTSET,postPlanList);
		}
		if(!lockPlanList.empty())
		{
			pData->mgRemainCapacityDao.UnionFree(workId,TYPE_LOCKTM,lockPlanList);
		}
		if(!prePlanList.empty())
		{
			pData->mgRemainCapacityDao.UnionFree(workId,TYPE_PRESET,prePlanList);
		}
		scheduling.UpdateTimeBoundDynamic(workId,true);

		//取计划列表id
		
			list<WorkSchPlanPtr> planList = workPtr->GetSchPlanList();
			for(list<WorkSchPlanPtr>::iterator iter = planList.begin();
				iter != planList.end();iter++)
			{
				newSchPlanList.push_back((*iter)->GetId());
			}
			pComWork->SetSchLevel(tempType);
			return true;
	}

	pComWork->SetSchLevel(tempType);
	return false;
}

//强制逻辑
bool SBHForceWork( int workId,list<int>&newSchPlanList )
{
	AppendWork* pComWork = pData->GetAppendWork(workId);
	if(pComWork == NULL) return false;
	SchLevelType tempType = pComWork->GetSchLevel();
	WorkPtr& workPtr = pComWork->GetWorkPtr();
	pComWork->SetSchLevel(SL_Level3);
	if(pComWork != NULL && workPtr != NULL
		&& ( pComWork->GetSchLevel() == SL_Level3 || pComWork->GetSchLevel() == SL_Level4 )
		&& AMAPI::IsParticipateSchWork( workId))
	{
		pData->parame.m_curSchMethod = SM_INFINITY;
		if(pComWork->GetISSchedFlg())
		{
			pComWork->SetISSchedFlg(false);
			pComWork->SetISSucessedFlg(false);
		}
		//备份排程计划
		list<WorkSchPlanPtr> schPlanIDList = workPtr->GetSchPlanList();
		//违反约束排程
		CScheduler scheduling;
		scheduling.CommonSchdule(workId,pComWork->GetDirection());
		//如果排程依旧失败
		if(!pComWork->GetISSucessedFlg())
		{
			pComWork->SetSchLevel(tempType);
			return false;
		}
		map<SBH_ULONG,TimeRangeList> prePlanList;	//前设置
		map<SBH_ULONG,TimeRangeList> prdPlanList;	//制造
		map<SBH_ULONG,TimeRangeList> postPlanList;	//后设置
		map<SBH_ULONG,TimeRangeList> lockPlanList;	//锁定
		//删除工作原排程计划和资源量
		for(list<WorkSchPlanPtr>::iterator iter = schPlanIDList.begin();
			iter != schPlanIDList.end();iter++)
		{
			WorkSchPlanPtr& workPlan = *iter;
			if(workPlan->GetType() == TYPE_PRESET)
			{
				prePlanList[workPlan->GetPlanSelResId()].push_back(TimeRange(workPlan->GetStartTime(),workPlan->GetEndTime(),(double)workPlan->GetUsedQuantity()));
			}
			else if(workPlan->GetType() == TYPE_PRODUCETM)
			{
				prdPlanList[workPlan->GetPlanSelResId()].push_back(TimeRange(workPlan->GetStartTime(),workPlan->GetEndTime(),(double)workPlan->GetUsedQuantity()));
			}
			else if(workPlan->GetType() == TYPE_POSTSET)
			{
				postPlanList[workPlan->GetPlanSelResId()].push_back(TimeRange(workPlan->GetStartTime(),workPlan->GetEndTime(),(double)workPlan->GetUsedQuantity()));
			}
			else if(workPlan->GetType() == TYPE_LOCKTM)
			{
				lockPlanList[workPlan->GetPlanSelResId()].push_back(TimeRange(workPlan->GetStartTime(),workPlan->GetEndTime(),(double)workPlan->GetUsedQuantity()));
			}
		}
		if(!prdPlanList.empty())
		{
			pData->mgRemainCapacityDao.UnionFree(workId,TYPE_PRODUCETM,prdPlanList);
		}
		if(!postPlanList.empty())
		{
			pData->mgRemainCapacityDao.UnionFree(workId,TYPE_POSTSET,postPlanList);
		}
		if(!lockPlanList.empty())
		{
			pData->mgRemainCapacityDao.UnionFree(workId,TYPE_LOCKTM,lockPlanList);
		}
		if(!prePlanList.empty())
		{
			pData->mgRemainCapacityDao.UnionFree(workId,TYPE_PRESET,prePlanList);
		}
		scheduling.UpdateTimeBoundDynamic(workId,true);

		//取计划列表id
		list<WorkSchPlanPtr> planList = workPtr->GetSchPlanList();
		for(list<WorkSchPlanPtr>::iterator iter = planList.begin();
			iter != planList.end();iter++)
		{
			newSchPlanList.push_back((*iter)->GetId());
		}
		pComWork->SetSchLevel(tempType);
		return true;

	}

	
	pComWork->SetSchLevel(tempType);
	return false;
}
void DeleteModelWorkResCal(list<int>wkIdList)
{
	CSBHDragV3* pDrag = CSBHDragV3::Get();
	pDrag->DeleteModelWorkResCal(wkIdList);
}
void DeleteModelWork(list<int>dWkIdList)
{
	CSBHDragV3* pDrag = CSBHDragV3::Get();
	pDrag->DeleteModelWork(dWkIdList);
}

void Prepare_Work_FGT( unsigned long resId,__int64 workStartTm,__int64 workEndTm )
{
	pData->forGantt.Prepare(resId,workStartTm,workEndTm);
}

 bool Loop_Work_FGT()
{
	return pData->forGantt.Loop();
}

 bool Get_Work_FGT(unsigned long& workId)
{
	 return pData->forGantt.Get(workId);
 }

 bool HitTest_FGT( unsigned long resId, __int64 time, __int64& workStartTm )
 {
	 return pData->forGantt.HitTest(resId,time,workStartTm);
 }

 bool PrepareGetCoverWork_FGT( unsigned long resId,__int64 st,__int64 et )
 {
	 return pData->forGantt.PrepareGetCoverWork(resId,st,et);
 }

 bool GetCoverWork_FGT( set<int>& coverWorkList,__int64& st,__int64& et ,int& nWorkAmount)
 {
	 CoverWorkInfo cwi;
	 if (pData->forGantt.GetCoverWork(cwi))
	 {
		 coverWorkList = cwi.workList;
		 st = cwi.startTime;
		 et = cwi.endTime;
		 nWorkAmount = cwi.maxWorkAmount;
		 return true;
	 }
	 return false;
 }

  bool PrepareResLoad_FGT( unsigned long resId )
 {
	 return pData->forGantt.PrepareResLoad(resId);
 }

  bool GetResLoad_FGT( __int64 st,__int64 et, double& fCap,bool bConsiderSetTmFlg)
 {
	 return pData->forGantt.GetResLoad(st,et,fCap,bConsiderSetTmFlg);
  }

   bool GetResLoad_FGT( __int64 st,__int64 et,double& use,double& all,bool bConsiderSetTmFlg /*= true*/ )
  {
	  return pData->forGantt.GetResLoad(st,et,use,all,bConsiderSetTmFlg);
  }
  int GetVirtualWorkCount_FGT()
  {
	  return pData->forGantt.GetVirtualWorkCount();
  }

  void GetResourceList_FGT( list<unsigned long>& resList )
  {
	  return pData->forGantt.GetResourceList(resList);
  }

   __int64 GetResCurStartTime_FGT( unsigned long resId,__int64 timePoint )
  {
	  return pData->forGantt.GetResCurStartTime(resId,timePoint);
   }

    bool PrepareGetOverTopRange_FGT( unsigned long resId,__int64 st,__int64 et )
   {
	   return pData->forGantt.PrepareOverTopRange(resId,st,et);
   }

    bool GetOverTopRange_FGT( __int64& st,__int64& et,double& fCap )
   {
	   OvertopRangeInfo ori;
	   if (pData->forGantt.GetOverTopRange(ori)) 
	   {
		   st = ori.startTime;
		   et = ori.endTime;
		   fCap = ori.remainCap;
		   return true;
	   }
	   return false;
	}

	 bool GetMinStartBetweenAB_FGT( unsigned long resId,__int64 A,__int64 B,__int64& newTime )
	{
		return pData->forGantt.GetMinStartBetweenAB(resId,A,B,newTime);
	 }

	 bool GetMinStartBetweenABEx_FGT( unsigned long resId,__int64 A,__int64 B,__int64& newTime,int& workId,list<int>& exWorkList )
	 {
		 return pData->forGantt.GetMinStartBetweenABEx(resId,A,B,newTime,workId,exWorkList);
	 }

  bool DrawOrCompressWork( int workId, int taskType, time_t startTm, time_t endTm, list<int>& newSchPlanList )
 {

	 //违反操作	
	 if (startTm > endTm)
	 {
		 return false;
	 }

	 WorkPtrMap* workpm = AMAPI::GetWorkPtrMap();
	 WorkSchPlanPtrMap* wschpm = AMAPI::GetWorkSchPlanPtrMap();
	 WorkPtr coveredWorkPtr = NULL;
	 ResourcePtr resPtr = NULL;
	 WorkResRelaPtr mainResRela = NULL;
	 list<WorkResRelaPtr> assistResList;


	 list<WorkSchPlanPtr> workSchPlanList;
	 list<WorkSchPlanPtr>::iterator iterWorkPlan;

	 map<time_t,TimeInfo> globalTimeInfo;
	 map<time_t,TimeInfo>::iterator iterGlobalTmInfo;

	 //中断时间
	 double presetInterruptTm = -2;
	 double pdtInterruptTm = -2;
	 double postsetInterruptTm = -2;

	 //备份开始结束时刻
	 time_t minStartTm = LLONG_MAX;
	 time_t maxEndTm = 0;

	 coveredWorkPtr = workpm->Get(workId);
	 if (coveredWorkPtr !=NULL)
	 {
		 //获得所在资源
		// AMAPI::GetPlanResRelaCom(coveredWorkPtr, mainResRela, assistResList);
		 AMAPI::GetIncludePlanResRelaCom(coveredWorkPtr, mainResRela, assistResList);
		 if (mainResRela == NULL)
		 {
			 return false;
		 }				 
		 resPtr = mainResRela->GetRes();
		 if (resPtr != NULL)
		 { 					
			 if (taskType == PRESET_TYPE)//前设置
			 {							
			 }
			 else if (taskType == PRODUCT_TYPE)//制造
			 {

				 return DrawOrCompWorkPrud(workId,  startTm,  endTm, newSchPlanList );

			 }
			 else if (taskType == POSTSET_TYPE)//后设置
			 {
			 }
		 }
	 }	
	 return true;
  }

  // bool CalWorkTimeBound( int workId,time_t presetSt,time_t presetEnd,time_t pudSt ,time_t pudEnd,time_t nextSt,time_t nextEnd,list<int>& newSchPlanList ,int resId , list<int>& assistRes )
  //{
	 // return false;
  //}

   #include "SequenceDispatchMgr.h"
   void HorizontalCombineAPI( const list<int>& combineWorks, CConsecutiveCond& combConditionList,list<int>& newPlanList )
  {
	  AMAPI::SetSchedulingFlg(true);
	  CSequenceDispatchMgr sm;
	  sm.SetConsecutiveCond(combConditionList);
	  sm.Dispatch(combineWorks);
	  sm.Ending(newPlanList);
	  AMAPI::SetSchedulingFlg(false);
   }

   bool Loop_Res_FGT()
   {
	   return pData->forGantt.Loop_Res();
   }

   void GetNodeInfo_FGT( __int64& time,double& maxCap,double& lfCap,short& type )
   {
	   return pData->forGantt.GetNodeInfo(time,maxCap,lfCap,type);
   }

   bool Loop_Node_FGT()
   {
	   return pData->forGantt.Loop_Node();
   }

   void GetNodeCap_FGT( unsigned long& workId,double& usedCap,int& usedType )
   {
	   return pData->forGantt.GetNodeCap(workId,usedCap,usedType);
   }

   void FlashWorkSchFlg()
   {
	   pData->FlashWorkSchFlg();
   }

   void SetKeyDown( int* shiftKey,int* otherKey )
   {
	   pData->SetSBHKeyState(shiftKey,otherKey);
   }

     void SetFlashFlg( bool* flash )
   {
	   pData->SetFlashStata(flash);
	 }

	 bool GetWorkListInArea_FGT( unsigned long resId,__int64 st,__int64 et,set<unsigned long>& workLst )
	 {
		 return pData->forGantt.GetWorkList(resId,st,et,workLst);
	 }

	 //界面任务表拖动
	 bool CollectTableDrag( int taskId,double dragCount,time_t srcTime,time_t desTime )
	 {
		 //获取天（班次开始时刻）
		 CDealTime srcDate = CDealTime(srcTime).GetDate();
		 CDealTime desDate = CDealTime(desTime).GetDate();

		 //输入班次相同，不处理
		 if(srcDate == desDate)
			 return false;

		 //判断方向：true 为正向
		 bool direct = (desDate>srcDate)?true:false;

		 //获取任务对应的工作列表，以工作开始时刻升序排列
		 list<int> workList; // = GetWorkList(taskId);

		 //累加数量记录
		 double totalCount(0);

		 //遍历工作列表
		 for(list<int>::iterator iter = workList.begin();
			 iter != workList.end();iter++)
		 {
			 //获取对应工作
			 WorkPtr& work = AMAPI::GetWorkPtrMap()->Get(*iter);

			 //工作拖动数量
			 double workDragCount(0);
			 //获取工作数量
			 totalCount += work->GetProcessAmount();
			 if(totalCount > dragCount)
			 {
				 workDragCount = work->GetProcessAmount() - (totalCount - dragCount);
			 }
			 else
			 {
				 workDragCount = work->GetProcessAmount();
			 }

			 //调用工作拖动排程
			 if(CollectTableWorkDrag(*iter,workDragCount,direct,desDate.GetTime()))
			 {
				 //工作拖动失败，工作计划还原了
				 totalCount -= work->GetProcessAmount();
			 }

			 if((totalCount - dragCount) > 0.00001)
				 break;
		 }

		 return true;
	 }

	 bool CollectTableWorkDrag( int workId,double dragCount,int direct,time_t desTime )
	 {
		 //计算目标班次剩余总工时
		 double remainManHour(0);

		 //计算所需工时
		 WorkPtr& work = AMAPI::GetWorkPtrMap()->Get(workId);

		 //计算工作所需总工时
		 double workManHour(1);

		 CSBHDragV3* pDrag = CSBHDragV3::Get();

		 //判断排程模式
		 if(remainManHour >= workManHour && BZERO(dragCount - work.GetProcessAmount()))
		 {
			 //工作产能可以放入这一天，无限能力，最大资源量排程
			 return pDrag->SingleWorkSch(work,direct,desTime);

		 }
		 else
		 {
			 //工作产能大于这一天剩余产能，将工作的一部分排入这一天
			 //计算数量
			 if(!BZERO(workManHour * work.GetProcessAmount()))
			 {
				 double count = remainManHour / workManHour * work.GetProcessAmount();

				 return pDrag->SingleWorkSch(work,direct,desTime,min(count,dragCount));
			 }
		 }

		 return false;
	 }

	 bool RetractWorkPlan( int workId )
	 {
		 pData->DeleteWork(workId);
		 return true;
	 }

	 static map<time_t,list<int> > effectWorkList;

	 void RetractWorkPlan(  TestSchedulerInfo& tsInfo)
	 {
		 CSBHDragV3* pDrag = CSBHDragV3::Get();

		 //超负荷时，还原所有工作计划,清理排程数据
		 TestSchedulerInfo tempInfo;
		 pDrag->BackupWorks(effectWorkList,tempInfo);

		 pDrag->ResumePlan(tsInfo);

		 //分别排每个区间内的工作
		 for(map<time_t,list<SBH_INT> >::iterator iter = effectWorkList.begin();
			 iter != effectWorkList.end();iter++)
		 {
			 pDrag->UpdateTakeTimePoint(iter->second);
		 }

		 effectWorkList.clear();
	 }

	 bool DaySchWorkDragAdjust( int workId ,SBH_INT resId)
	 {
		 if(AMAPI::GetParameter()->GetSystemSchFlg() == Refined_Sch)
			 return false;

		 //取输入工作计划开始，结束时刻影响到的工作列表；
	
		 pData->parame.m_bMoveWork = true;
		 CSBHDragV3* pDrag = CSBHDragV3::Get();
		 pDrag->GetEffectWorkList(workId,resId,effectWorkList);

		 //备份所有待处理工作计划
		 TestSchedulerInfo tsInfo;
		 pDrag->BackupWorks(effectWorkList,tsInfo,false);

		 //分别排每个区间内的工作
		 for(map<time_t,list<SBH_INT> >::iterator iter = effectWorkList.begin();
			 iter != effectWorkList.end();iter++)
		 {
			 //对每个区间单独排程，出现超负荷的情况，返回false
			 if(!iter->second.empty() && !pDrag->ScheSingleInterzone(iter->second,iter->first,0))
			 {
				 ////超负荷时，还原所有工作计划,清理排程数据
				 // TestSchedulerInfo tempInfo;
				 // pDrag->BackupWorks(effectWorkList,tempInfo);

				 //pDrag->ResumePlan(tsInfo);
				 RetractWorkPlan(tsInfo);
				   pData->parame.m_bMoveWork = false;
				 return false;
			 }
		 }
		 effectWorkList.clear();
		  pData->parame.m_bMoveWork = false;
		 return true;
	 }

	 void BackupEffectWorkList(SBH_INT workId ,SBH_INT resId)
	 {
		 if(AMAPI::GetParameter()->GetSystemSchFlg() == Refined_Sch)
			 return ;

		 CSBHDragV3* pDrag = CSBHDragV3::Get();
		 pDrag->GetEffectWorkList(workId,resId,effectWorkList);

		 //删除本工作
		 //分别排每个区间内的工作
		 for(map<time_t,list<SBH_INT> >::iterator iter = effectWorkList.begin();
			 iter != effectWorkList.end();iter++)
		 {
			 list<SBH_INT>& workList = iter->second;
			 //对每个区间单独排程，出现超负荷的情况，返回false
			 list<SBH_INT>::iterator iter1 = find(workList.begin(),workList.end(),workId);
			 if(iter1 != workList.end())
			 {
				 workList.erase(iter1);
			 }
		 }
	 }
	 //开始生产的工作先取出主副资源组合
	 void GetStPlanPlanResRelaCom(int srcWorkId)
	 {
		 CSBHDragV3* pDrag = CSBHDragV3::Get();
		 return pDrag->GetStPlanPlanResRelaCom(srcWorkId);
	 }

	 void Prepare_DayShift_FGT( int resId )
	 {
		 pData->forGantt.Prepare_DayShift(resId);
	 }
	 bool Loop_DayShift_FGT(){
		 return pData->forGantt.Loop_DayShift();
	 }
	 int  Get_DayShift_ResId_FGT(){
		 return pData->forGantt.Get_DayShift_ResId();
	 }
	 __int64 Get_DayShift_Time_FGT(){
		 return pData->forGantt.Get_DayShift_Time();
	 }
	 int  Get_DayShift_ShiftId_FGT(){
		  return pData->forGantt.Get_DayShift_ShiftId();
	 }
	 int  Get_DayShift_WorkId_FGT(){
		  return pData->forGantt.Get_DayShift_WorkId();
	 }
	 double Get_DayShift_WorkAmount_FGT(){
		  return pData->forGantt.Get_DayShift_WorkAmount();
	 }

	 int Get_DayShift_WorkType_FGT()
	 {
		  return pData->forGantt.Get_DayShift_WorkType();
	 }

	 void ProDragLocked( Work* pPreWork )
	 {
		set<vector<SBH_INT> > setLockPairs;
		 list<WorkResRelaPtr>& listWR =  pPreWork->GetWorkResRela();
		 for (list<WorkResRelaPtr>::iterator iter = listWR.begin();
			 iter != listWR.end(); ++iter)
		 {
			 const SBH_TIME fixedLockTime = AMAPI::GetResLockTime(iter->GetRes());
			 const SBH_INT resLckType = iter->GetRes()->GetResLckType();
			 list<Work*> listPostWork;
			 pPreWork->GetPostworks(listPostWork);
			 SBH_TIME maxTime = 0, curTime = 0;
			 Work* pSelWork = 0;
			 AMAPI::GetMaxEndTime(iter->GetWork(), WorkPlan_Produce, maxTime);
			 for (list<Work*>::iterator iterPost = listPostWork.begin();
				 iterPost != listPostWork.end(); ++iterPost)
			 {
				 switch (resLckType)
				 {
				 case 3:
					 AMAPI::GetMinStartTime(*(*iterPost), WorkPlan_Produce, curTime);
					 break;
				 case 4:
					 AMAPI::GetMaxEndTime(*(*iterPost), WorkPlan_Produce, curTime);
					 break;
				 }
				 if (maxTime < (curTime + fixedLockTime))
				 {
					 maxTime = curTime;
					 pSelWork = *iterPost;
				 }
			 }

			 if (pSelWork)
			 {
				 vector<SBH_INT> vecPair;
				 vecPair.push_back(pPreWork->GetId());
				 vecPair.push_back(pSelWork->GetId());
				 vecPair.push_back(resLckType);
				 setLockPairs.insert(vecPair);
			 }
			 else if (fixedLockTime > 0)
			 {
				 // 清除锁定及后设置
				 map<SBH_ULONG,TimeRangeList>	mapBackupLock;
				 map<SBH_ULONG,TimeRangeList>	mapBackupNexSet;
				 TestSchedulerInfo tsBackup;
				 list<WorkSchPlanPtr> listWSP = pPreWork->GetSchPlanList();
				 for (list<WorkSchPlanPtr>::iterator iterWSP = listWSP.begin();
					 iterWSP != listWSP.end(); ++iterWSP)
				 {
					 if ((*iterWSP)->GetType() == WorkPlan_Lock)
					 {
						 mapBackupLock[(*iterWSP)->GetPlanSelResId()].push_back(TimeRange((*iterWSP)->GetStartTime(), (*iterWSP)->GetEndTime(),(SBH_DOUBLE)(*iterWSP)->GetUsedQuantity()));
					 }

					 if ((*iterWSP)->GetType() == workPlan_NextSet)
					 {
						 mapBackupNexSet[(*iterWSP)->GetPlanSelResId()].push_back(TimeRange((*iterWSP)->GetStartTime(), (*iterWSP)->GetEndTime(),(SBH_DOUBLE)(*iterWSP)->GetUsedQuantity()));
					 }
				 }
				 tsBackup.InsertAddPlan(pPreWork->GetId(), workPlan_NextSet, mapBackupNexSet);
				 tsBackup.InsertAddPlan(pPreWork->GetId(), WorkPlan_Lock, mapBackupLock);
				 pPreWork->ClearSchPlan(WorkPlan_Lock);
				 pPreWork->ClearSchPlan(workPlan_NextSet);
				 pData->mgRemainCapacityDao.UnionFree(pPreWork->GetId(), WorkPlan_Lock, mapBackupLock);
				 pData->mgRemainCapacityDao.UnionFree(pPreWork->GetId(), workPlan_NextSet, mapBackupNexSet);

				 // 重新设置后设置
				 WorkResInfo workResInfo;
				 pPreWork->GetPlanResRelaCom(workResInfo.resRelaID, workResInfo.assistResRelaList);
				 const SBH_ULONG workId = pData->mgRemainCapacityDao.GetNearByWork(workResInfo.resRelaID.GetResId(), maxTime, Dir_Obverse);

				 TimeLenInfo	preTm, nexTm;
				 SBH_TIME rtStTm, rtEndTm;
				 SingleCalcV3 singleCal;
				 TestSchedulerInfo tsInfo;
				 singleCal.GetSetTimes(workId, *pPreWork, workResInfo, Dir_Obverse, preTm, nexTm);
				 if (singleCal.VaildTmLen(nexTm)  &&
					 singleCal.CalWorkTime(*pPreWork, workResInfo, TYPE_POSTSET, maxTime
					 ,nexTm, tsInfo, rtStTm, rtEndTm, FUC_F2, Dir_Obverse))
				 {
					 tsInfo.SurePlan();
				 }
				 else
				 {
					 tsBackup.FinallySurePlan();
				 }
			 }
		 }

		 if (!setLockPairs.empty())
		 {
			 CLockedDispose lockDis;
			 for (set<vector<SBH_INT> >::iterator iterLock = setLockPairs.begin();
				 iterLock != setLockPairs.end(); ++iterLock)
			 {
				 lockDis.ResLockedPairPro(*iterLock);
			 }
			 setLockPairs.clear();
		 }
	 }

	 void CheckDisobeyRestrict()
	 {
		pData->appendWorkDao.CheckDisobeyRestrict();
	 }