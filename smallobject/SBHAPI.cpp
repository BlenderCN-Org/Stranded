
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

//��̬����ȫ�ֱ���
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

//�ͷ�ȫ�ֱ���
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

	//��resList���ϲ���²���Դ����resList��
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
	////��������
	//if (schType == 2)
	//{
	//	//SetAdjustWorks(relevantWorks);
	//}
	////�����嵥����
	//else if (schType == 3)
	//{
	//	//SetFlyWorks(relevantWorks);
	//}
	////�Ӱ๦��
	//else if (schType == 4) 
	//{
	//	//SetOverTimeWorks(relevantWorks);
	//}
	////�����Դ��������������Դɾ������Դ���ӣ�
	//else if (schType == 5)
	{
		SetResUpdateWorks(relevantWorks);
	}
}


//�����϶�����
//1.orgMouseStartTmΪ�ϴ����λ��
//2.newMouseStartTmΪ��ǰ���λ��
//3.<mainResId, assistResList>ΪworkIdΪָ����Դ
//4.newSchPlanListΪ�µ��ų̼ƻ�
//5.reltIntervlRateΪ��ԡ�ǰ���ÿ�ʼʱ�̡��ٷֱȣ���ĳ���϶������У���һ�ε�ʱ����ʼֵΪ-1���Ժ���ã������õ�һ�εķ���ֵ��
bool GetMoveWorkSchPlan(int workId, int mainResId, int origResId, 
						time_t orgMouseStartTm, time_t newMouseStartTm
						, list<int>& newSchPlanList, double& reltIntervlRate,int nGridRange)
{
	int origType = AMAPI::GetParameter()->GetMouseMoveType();

	if(AMAPI::GetParameter()->GetSystemSchFlg() != Refined_Sch)	//�Ǿ�ϸ�ų�
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

//  �����/�Ҳ��������϶��������ͷ����󣬵��øú�����
//  ����϶���
//	workIdΪ�϶�����
//	mouseResIdΪ�϶�������������������Դ
//  mainResIdΪ�϶�������Ҫ�ų̵�����Դ,��Ϊ������϶���ʹ�ã��������Ϊ��1;
//  assistResListΪ�϶�������Ҫ�ų̵ĸ���Դ�б�,��Ϊ������϶���ʹ�ã��������Ϊ��1;
//	mouseStartTmΪ�϶�����������������ʱ�̣���Ϊ������϶���ʹ�ã��������Ϊ��1��
//  gridStartTmΪ���������ڵ�Ԫ��Ŀ�ʼʱ�̣���Ϊ������϶���ʹ�ã��������Ϊ��1��
//  gridEndTmΪ���������ڵ�Ԫ��Ľ���ʱ�̣���Ϊ������϶���ʹ�ã��������Ϊ��1��
//	newSchPlanListΪ�µ��ų̼ƻ�
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
//						//ɾ��ʣ������
//						pData->mgRemainCapacityDao.Resume(tempWorkList, (*iterCurrWorkPlan)->GetPlanSelResId(), (*iterCurrWorkPlan)->GetStartTime(), 
//							(*iterCurrWorkPlan)->GetEndTime(), (*iterCurrWorkPlan)->GetUsedQuantity(), (*iterCurrWorkPlan)->GetType());
//					}
//					AMAPI::ClearBefoDelWorkSchPlan(*iterCurrWorkPlan);
//					//ɾ�������ų̵��ų̼ƻ�
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

//�ų̼���
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

	//�޿�����Դ���
	if(resId == 0)
		return false;


      
	if (reltIntervlRate==-1)
	{
		reltIntervlRate=pDrag->CalTimeLen(orgMouseStartTm, newMouseStartTm);
		reltIntervlRate =0;
	}

	//�����ų̱�־��ʱ�䷶Χ�Ƿ��ڹ���ʱ�䷶Χ��
	if (!pDrag->IfDrag()
		/*||!pDrag->IfSchedulerTimeRand(orgMouseStartTm)*/)
	{
		return false;
	}


	if (orgMouseStartTm==newMouseStartTm&&pDrag->IfSameRes())//ʱ����ͬ,��Դ��ͬ
	{
		return  false;
	}
   
	/*�ж���Դ�Ƿ����ϰ�ʱ���*/
	list<SBH_INT>combList = pDrag->GetResComb();
	for ( list<SBH_INT>::iterator iter =combList.begin();
		iter !=combList.end(); iter++)
	{
		if (!pData->mgRemainCapacityDao.IfOffDutyTimePoint(*iter/*resId*/,newMouseStartTm))  //�Ƿ����°�ʱ���
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

//����϶����ܺ���
bool GetNewPushWorkSchPlan(int workId, int mouseResId, int origResId,time_t mouseStartTm, time_t gridStartTm, time_t gridEndTm, list<int>& newSchPlanList)
{
	CSBHDragV3* pDrag = CSBHDragV3::Get();
	pDrag->SetWorkResId(workId,mouseResId);
	pDrag->InitWorkPtr();
	if (!pDrag->IfValidPtr()) return false;

	//������Դ�Ƿ�Ϊ��ͬ��Դ
	if(mouseResId!=origResId)
		pDrag->SetSameRes(false);
	else
		pDrag->SetSameRes(true);

	pDrag->GetChangeRes(mouseResId,origResId);


	//�����ų̱�־��ʱ�䷶Χ�Ƿ��ڹ���ʱ�䷶Χ��
	if (!pDrag->IfDrag())
	{
		return false;
	}
	if (mouseStartTm==-1)
	{
		if (AMAPI::GetParameter()->GetMouseMoveType()==4)
		{
			//�����
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
			//�Ҳ���
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

//�������쳤���϶�
bool DrawWorkLen( int workId, time_t startTm, time_t endTm, list<int>& newSchPlanList )
{
	CSBHDragV3* pDrag = CSBHDragV3::Get();
	pDrag->SetWorkResId(workId,0);
	pDrag->InitWorkPtr();
	if (!pDrag->IfValidPtr()) return false;
    return pDrag->DrawWorkLen( startTm, endTm,newSchPlanList);
}
//�������
bool GetSplitWorkPlan(SBH_INT srcWorkId,SBH_INT fDevWorkId,SBH_INT sDevWorkId,double rate, list<SBH_INT>& newSchPlanList)
{
	CSBHDragV3* pDrag = CSBHDragV3::Get();
	return pDrag->GetSplitWorkPlan(srcWorkId,fDevWorkId,sDevWorkId,rate, newSchPlanList);
}
//�����ϲ�
bool GetCombWorkPlan(int srcWorkId,double rate,list<unsigned long> devWorkIdList,  list<int>& newSchPlanList,bool combParent)
{
	CSBHDragV3* pDrag = CSBHDragV3::Get();
	return pDrag->GetCombWorkPlan(srcWorkId,rate, devWorkIdList, newSchPlanList,combParent);
}
//�����ָ�
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

//�Ӱ๦��
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
		//���Υ��Լ������ȡΥ��Լ������ʽ,�Ӱ�
		pData->parame.m_curSchMethod = SM_OVERTIME;
		if(pComWork->GetISSchedFlg())
		{
			pComWork->SetISSchedFlg(false);
			pComWork->SetISSucessedFlg(false);
		}
		//�����ų̼ƻ�
		list<WorkSchPlanPtr> schPlanIDList = workPtr->GetSchPlanList();

		//Υ��Լ���ų�
		CScheduler scheduling;
		scheduling.CommonSchdule(workId,pComWork->GetDirection());
		//����ų�����ʧ��
		if(!pComWork->GetISSucessedFlg())
		{
			pComWork->SetSchLevel(tempType);
			return false;
		}
		map<SBH_ULONG,TimeRangeList> prePlanList;	//ǰ����
		map<SBH_ULONG,TimeRangeList> prdPlanList;	//����
		map<SBH_ULONG,TimeRangeList> postPlanList;	//������
		map<SBH_ULONG,TimeRangeList> lockPlanList;	//����
		//ɾ������ԭ�ų̼ƻ�����Դ��
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

		//ȡ�ƻ��б�id
		
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

//ǿ���߼�
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
		//�����ų̼ƻ�
		list<WorkSchPlanPtr> schPlanIDList = workPtr->GetSchPlanList();
		//Υ��Լ���ų�
		CScheduler scheduling;
		scheduling.CommonSchdule(workId,pComWork->GetDirection());
		//����ų�����ʧ��
		if(!pComWork->GetISSucessedFlg())
		{
			pComWork->SetSchLevel(tempType);
			return false;
		}
		map<SBH_ULONG,TimeRangeList> prePlanList;	//ǰ����
		map<SBH_ULONG,TimeRangeList> prdPlanList;	//����
		map<SBH_ULONG,TimeRangeList> postPlanList;	//������
		map<SBH_ULONG,TimeRangeList> lockPlanList;	//����
		//ɾ������ԭ�ų̼ƻ�����Դ��
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

		//ȡ�ƻ��б�id
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

	 //Υ������	
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

	 //�ж�ʱ��
	 double presetInterruptTm = -2;
	 double pdtInterruptTm = -2;
	 double postsetInterruptTm = -2;

	 //���ݿ�ʼ����ʱ��
	 time_t minStartTm = LLONG_MAX;
	 time_t maxEndTm = 0;

	 coveredWorkPtr = workpm->Get(workId);
	 if (coveredWorkPtr !=NULL)
	 {
		 //���������Դ
		// AMAPI::GetPlanResRelaCom(coveredWorkPtr, mainResRela, assistResList);
		 AMAPI::GetIncludePlanResRelaCom(coveredWorkPtr, mainResRela, assistResList);
		 if (mainResRela == NULL)
		 {
			 return false;
		 }				 
		 resPtr = mainResRela->GetRes();
		 if (resPtr != NULL)
		 { 					
			 if (taskType == PRESET_TYPE)//ǰ����
			 {							
			 }
			 else if (taskType == PRODUCT_TYPE)//����
			 {

				 return DrawOrCompWorkPrud(workId,  startTm,  endTm, newSchPlanList );

			 }
			 else if (taskType == POSTSET_TYPE)//������
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

	 //����������϶�
	 bool CollectTableDrag( int taskId,double dragCount,time_t srcTime,time_t desTime )
	 {
		 //��ȡ�죨��ο�ʼʱ�̣�
		 CDealTime srcDate = CDealTime(srcTime).GetDate();
		 CDealTime desDate = CDealTime(desTime).GetDate();

		 //��������ͬ��������
		 if(srcDate == desDate)
			 return false;

		 //�жϷ���true Ϊ����
		 bool direct = (desDate>srcDate)?true:false;

		 //��ȡ�����Ӧ�Ĺ����б��Թ�����ʼʱ����������
		 list<int> workList; // = GetWorkList(taskId);

		 //�ۼ�������¼
		 double totalCount(0);

		 //���������б�
		 for(list<int>::iterator iter = workList.begin();
			 iter != workList.end();iter++)
		 {
			 //��ȡ��Ӧ����
			 WorkPtr& work = AMAPI::GetWorkPtrMap()->Get(*iter);

			 //�����϶�����
			 double workDragCount(0);
			 //��ȡ��������
			 totalCount += work->GetProcessAmount();
			 if(totalCount > dragCount)
			 {
				 workDragCount = work->GetProcessAmount() - (totalCount - dragCount);
			 }
			 else
			 {
				 workDragCount = work->GetProcessAmount();
			 }

			 //���ù����϶��ų�
			 if(CollectTableWorkDrag(*iter,workDragCount,direct,desDate.GetTime()))
			 {
				 //�����϶�ʧ�ܣ������ƻ���ԭ��
				 totalCount -= work->GetProcessAmount();
			 }

			 if((totalCount - dragCount) > 0.00001)
				 break;
		 }

		 return true;
	 }

	 bool CollectTableWorkDrag( int workId,double dragCount,int direct,time_t desTime )
	 {
		 //����Ŀ����ʣ���ܹ�ʱ
		 double remainManHour(0);

		 //�������蹤ʱ
		 WorkPtr& work = AMAPI::GetWorkPtrMap()->Get(workId);

		 //���㹤�������ܹ�ʱ
		 double workManHour(1);

		 CSBHDragV3* pDrag = CSBHDragV3::Get();

		 //�ж��ų�ģʽ
		 if(remainManHour >= workManHour && BZERO(dragCount - work.GetProcessAmount()))
		 {
			 //�������ܿ��Է�����һ�죬���������������Դ���ų�
			 return pDrag->SingleWorkSch(work,direct,desTime);

		 }
		 else
		 {
			 //�������ܴ�����һ��ʣ����ܣ���������һ����������һ��
			 //��������
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

		 //������ʱ����ԭ���й����ƻ�,�����ų�����
		 TestSchedulerInfo tempInfo;
		 pDrag->BackupWorks(effectWorkList,tempInfo);

		 pDrag->ResumePlan(tsInfo);

		 //�ֱ���ÿ�������ڵĹ���
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

		 //ȡ���빤���ƻ���ʼ������ʱ��Ӱ�쵽�Ĺ����б�
	
		 pData->parame.m_bMoveWork = true;
		 CSBHDragV3* pDrag = CSBHDragV3::Get();
		 pDrag->GetEffectWorkList(workId,resId,effectWorkList);

		 //�������д��������ƻ�
		 TestSchedulerInfo tsInfo;
		 pDrag->BackupWorks(effectWorkList,tsInfo,false);

		 //�ֱ���ÿ�������ڵĹ���
		 for(map<time_t,list<SBH_INT> >::iterator iter = effectWorkList.begin();
			 iter != effectWorkList.end();iter++)
		 {
			 //��ÿ�����䵥���ų̣����ֳ����ɵ����������false
			 if(!iter->second.empty() && !pDrag->ScheSingleInterzone(iter->second,iter->first,0))
			 {
				 ////������ʱ����ԭ���й����ƻ�,�����ų�����
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

		 //ɾ��������
		 //�ֱ���ÿ�������ڵĹ���
		 for(map<time_t,list<SBH_INT> >::iterator iter = effectWorkList.begin();
			 iter != effectWorkList.end();iter++)
		 {
			 list<SBH_INT>& workList = iter->second;
			 //��ÿ�����䵥���ų̣����ֳ����ɵ����������false
			 list<SBH_INT>::iterator iter1 = find(workList.begin(),workList.end(),workId);
			 if(iter1 != workList.end())
			 {
				 workList.erase(iter1);
			 }
		 }
	 }
	 //��ʼ�����Ĺ�����ȡ��������Դ���
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
				 // ���������������
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

				 // �������ú�����
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