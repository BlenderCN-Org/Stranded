/*
* Copyright (c) 2008,永凯软件技术(上海)有限公司
* All rights reserved.
*/

/**********************************************************************
 *                         工作数据访问类                             *
 *                                                                    *
 * 功能描述  :   工作物料数据                           			  *
 * 输入参数  :   工作对象,工作标识                                    *
 * 返回参数  :	  详见各方法									      *
 * 作   者  :                                               		  *
 * 日   期  :                                  		                  *
*********************************************************************/
//#include "stdafx.h"
#include "AppendWorkDao.h"
#include <algorithm>
#include "AM_API.h"
#include "ArithDefine.h"
#include "AM_Global.h"
#include "SchAssistantFuncs.h"
#include "GlobalVarSBH.h"
#include <math.h>
using namespace AMAPI;

#ifdef _DEBUG_USING_CLOCK
#include "clock.h"

#define CLOCK_MARK(timer) timer.Pause();
#else
#define CLOCK_MARK(timer) NULL
#endif

#ifndef  _BZERO_X
#define  _BZERO_X
#define BZERO(x) ((x) > -0.0000001 && (x) < 0.0000001)
#endif

AppendWorkDao::AppendWorkDao()
:maxId(0)
{
	work_It = m_map.begin();
}


AppendWorkDao::~AppendWorkDao() 
{
	//if (!m_map.empty())
	m_map.clear();
	AppendWork::Destory();
}


// 初始化工作附表(要么全是覆盖工作，要么是普通工作初始化)
void AppendWorkDao::Create(list<WorkPtr>& works)
/*******************************************************************************
//输入：1、Work						（工作表）
//输出：1、AppendWork				（工作附表）							
*******************************************************************************/
{
	//（1）	输入：WORK
	//（2）	操作：
	//按如下过程建立“工作附表”的每条记录：
	//①	ID＝工作覆盖工作ID (工作类型＝4)；
	//②	工作最早开工时刻 ＝ 空；
	//③	工作最晚开工时刻 ＝ 空；
	//④	工作最早完工时刻 ＝ 空；
	//⑤	工作最晚完工时刻 ＝ 空；
	//⑥	初始工作最早开工时刻 ＝ 空；
	//⑦	初始工作最晚开工时刻 ＝ 空；
	//⑧	初始工作最早完工时刻 ＝ 空；
	//⑨	初始工作最晚完工时刻 ＝ 空；
	//⑩	排程标记：根据“工作表”，若工作覆盖工作ID的“排程标记＝1”，则排程标记＝1；否则，排程标记＝2； 
	//?	排程失败标记＝1（1.成功；2.失败）；
	//（3）	输出：工作附表（ADD_WORK）

	list<WorkPtr>::iterator it = works.begin();
	while(it != works.end())
	{
		WorkPtr& workPtr = *it;
		if (workPtr != NULL)
		{
			AppendWork appendwork;
			appendwork.id = workPtr->GetId();
			//appendwork.schFlag = workPtr->GetSchedulerFlg();
			appendwork.failFlag = 1;
			appendwork.earlieststarttime = 0;
			appendwork.earliestendtime = 0;
			appendwork.lateststarttime = LLONG_MAX;
			appendwork.latestendtime = LLONG_MAX;
			appendwork.initEarlyStartTime = 0;
			appendwork.initEarlyEndTime = 0;
			appendwork.initLatestStartTime = LLONG_MAX;
			appendwork.initLatestEndTime = LLONG_MAX;
			Add(appendwork);
			//map.insert(std::make_pair(appendwork.id, appendwork));
		}		
		it++;
	}
}


int AppendWorkDao::Add(const AppendWork& object) 
/*******************************************************************************
//输入：1、const Work& object				（需要添加到容器中的对象）
//输出：1、无							
*******************************************************************************/
{
	//尝试在容器中插入对象ID，如果此ID不存在，插入成功
	map<int, AppendWork>::iterator iter = m_map.find(object.id);
	if (iter == m_map.end())
	{
		m_map.insert(std::map<int, AppendWork>::value_type(object.id,object));
	}
// 	std::pair<std::map<int,AppendWork>::iterator,bool > ret =
// 		m_map.insert(std::map<int, AppendWork>::value_type(object.id,object));
// 	//如果此ID存在，使用下标方法插入
// 	if(!ret.second)
// 		m_map[object.id] = object;
	return object.id;
}


bool AppendWorkDao::Get(const int objectid, AppendWork& object) 
/*******************************************************************************
//输入：1、const unsigned int objectId			（取值对象的ID）
//输出：1、Work& object						（取值对象）
*******************************************************************************/
{
	//在容器中查找输入ID
	std::map<int, AppendWork>::iterator iter = m_map.find(objectid);
	if(iter == m_map.end())
	{
		//没找到，返回false
		return false;
	}
	else 
	{
		//找到，将需要对象赋值到object中
		object = iter->second;
		return true;
	}	
}

AppendWork * AppendWorkDao::Get(const int objectid) 
/*******************************************************************************
//输入：1、const unsigned int objectId			（取值对象的ID）
//输出：1、Work* object						（取值对象的指针）
*******************************************************************************/
{
	//在容器中查找输入ID
	AppendWork * p_object = NULL;
	std::map<int, AppendWork>::iterator iter = m_map.find(objectid);
	if(iter != m_map.end())
	{
		//找到，将需要对象赋值到object中
		p_object = &iter->second;
	}	
	return p_object;
}


bool AppendWorkDao::Update(const AppendWork& object) 
/*******************************************************************************
//输入：1、const unsigned int objectId			（待更新对象）
//输出：无
*******************************************************************************/
{
	//容器中查找对应ID
	if(m_map.count(object.id))
	{
		//存在则赋值，返回true
		m_map[object.id] = object;
		return true;
	}
	else
		//不存在返回fasle
		return false;
}


void AppendWorkDao::GetAll(std::map<int,AppendWork>& allobject) 
/*******************************************************************************
//输入：无
//输出：1、allobject			（将容器返回）
*******************************************************************************/
{
	allobject = m_map;
}

std::map<int,AppendWork>* AppendWorkDao::GetAll() 
/*******************************************************************************
//输入：无
//输出：1、p_allobject			（将容器指针返回）
*******************************************************************************/
{
	return &m_map;
}


void AppendWorkDao::StartLoop(void)
{
	//设置循环迭代器初值
	work_It = m_map.begin();
}

bool AppendWorkDao::Get(AppendWork & object)
/*******************************************************************************
//输入：无
//输出：1、const Work& object				（取值对象）
*******************************************************************************/
{
	//迭代器未指向容器尾，取值输出，并返回true
	if(work_It != m_map.end())
	{
		object = work_It->second;
		work_It++;
		return true;
	}
	//循环遍历结束，返回false
	else
		return false;
}

bool AppendWorkDao::Get(AppendWork** p_object)
/*******************************************************************************
//输入：无
//输出：1、const Work** p_object				（取值对象指针）
*******************************************************************************/
{
	//迭代器未指向容器尾，取值输出，并返回true
	if(work_It != m_map.end())
	{
		*p_object = &work_It->second;
		work_It++;
		return true;
	}
	//循环遍历结束，返回false
	else
		return false;
}

void AppendWorkDao::Clear(void)
{
	//清空容器
	m_map.clear();
	maxId = 0;
}


int AppendWorkDao::Create(AppendWork & object)
/*******************************************************************************
//输入：1、object									（需要产生id并添加的工作对象）
//输出：1、函数返回值								（产生的id号）
*******************************************************************************/
{
	//object.id = maxId + 1;
	return Add(object);
}

void AppendWorkDao::ClearAppendWork(const list<WorkPtr>& worklist)
/*******************************************************************************
//输入：1、list<int> worklist						（待清空工作列表）
//输出：1、无
*******************************************************************************/
{
	list<WorkPtr>::const_iterator it = worklist.begin();
	while(it != worklist.end())
	{
		if ((*it) != NULL)
		{
			Work temp = *it;
			AppendWork *p_appendwork = Get(temp->GetId());
			if(p_appendwork != NULL)
			{
				p_appendwork->failFlag = 1;
				//p_appendwork->regulationId.clear();
			}
		}
		it++;
	}
}

AppendWorkDao* AppendWorkDao::GetAppendWorkDao()
{
	return this;
}

//生成参与排程的工作
void AppendWorkDao::Init()
{
	Clear();
	m_bCaculateWorkBound = false;
	//粗略排程计算边界
	if (AMAPI::GetParameter()->GetSystemSchFlg() != Refined_Sch)
		return InitGlancing();

	time_t stTm = 0;//AMAPI::GetSchStartTime();
	time_t endTm = LLONG_MAX;//AMAPI::GetSchEndTime();
	WorkPtrMap* pWorkPtrMap = AMAPI::GetWorkPtrMap();
	if (pWorkPtrMap != NULL)
	{
		for (pWorkPtrMap->Begin();pWorkPtrMap->NotEnd();pWorkPtrMap->Step())
		{
			WorkPtr& workPtr = pWorkPtrMap->Get();
			if (workPtr.ValidPointer())
			{
				if (workPtr->IsParticipateSchWork())	//参与排程工作
				{
					AppendWork appendwork;
					appendwork.id = workPtr->GetId();
					appendwork.SetWorkPtr(workPtr);
					appendwork.failFlag = 1;
					appendwork.earlieststarttime = max(stTm,AMAPI::GetWorkLimit(appendwork.id,STARTAFTER));

					appendwork.latestendtime = min(endTm,AMAPI::GetWorkLimit(appendwork.id,ENDBEFORE));

					appendwork.initEarlyStartTime = appendwork.earlieststarttime;
					appendwork.initEarlyEndTime = appendwork.earliestendtime;
					appendwork.initLatestStartTime = appendwork.lateststarttime;
					appendwork.initLatestEndTime = appendwork.latestendtime;
					appendwork.SetRuleEarlyStartTm(appendwork.initEarlyStartTime);
					appendwork.SetRuleLatestEndTm(appendwork.initLatestEndTime);
					appendwork.SetOrigSchFlg(workPtr->GetSchedulerFlg());
					Add(appendwork);
				}
				else if (workPtr->GetType() == WORKTYPE_ORDER)		//订单
				{
					AppendWork appendwork;
					appendwork.id = workPtr->GetId();
					appendwork.SetWorkPtr(workPtr);
					appendwork.failFlag = 1;
					appendwork.earlieststarttime = max(stTm,AMAPI::GetWorkLimit(appendwork.id,STARTAFTER));

					appendwork.latestendtime = min(endTm,AMAPI::GetWorkLimit(appendwork.id,ENDBEFORE));
					appendwork.initEarlyStartTime = appendwork.earlieststarttime;
					appendwork.initEarlyEndTime = appendwork.earliestendtime;
					appendwork.initLatestStartTime = appendwork.lateststarttime;
					appendwork.initLatestEndTime = appendwork.latestendtime;
					appendwork.SetRuleEarlyStartTm(appendwork.initEarlyStartTime);
					appendwork.SetRuleLatestEndTm(appendwork.initLatestEndTime);
					appendwork.SetOrigSchFlg(workPtr->GetSchedulerFlg());
					Add(appendwork);
				}
			}
		}
	}

	//if(pData->newRegulation.IsNeedInitWorkBound())
	//{
	//	//计算边界(启发式规则用)
	//	CaculateWorkBound();
	//}
}

void AppendWorkDao::InitWorkBound()//初始化工作边界
{
	if (!m_bCaculateWorkBound)
	{
		m_bCaculateWorkBound = true;
		if(pData->newRegulation.IsNeedInitWorkBound())
		{
			//计算边界(启发式规则用)
			CaculateWorkBound();
		}
	}
}
//计算工作的初始边界
void AppendWorkDao::CaculateWorkBound()
{
	//遍历所有工作,取首工作集合和末工作集合
	set<Work*> firstWorkList,lastWorkList;
	std::map<int, AppendWork>::iterator iApp = m_map.begin();
	for (;iApp != m_map.end();iApp++)
	{
		AppendWork* pAppWork = &iApp->second;
		if (pAppWork != NULL)
		{
			WorkPtr&  workPtr = pAppWork->GetWorkPtr();//AMAPI::GetWorkPtrMap()->Get(pAppWork->id);
			if(workPtr != NULL)
			{
				//判断此工作是否为首工作
				list<Work*> workList;
				workPtr->GetPreworks(workList);
				if(workList.empty())
				{
					firstWorkList.insert(&workPtr);
					//list<int> workIdRecoder;
					//CalcEarlyBound(workPtr,pAppWork,workIdRecoder);
				}

				//判断此工作是否为末工作
				workList.clear();
				workPtr->GetPostworks(workList);
				if(workList.empty())
				{
					lastWorkList.insert(&workPtr);
					//list<int> workIdRecoder;
					//CalcLatestBound(workPtr,pAppWork,workIdRecoder);
				}
			}
		}//end if (pAppWork != NULL)
	}

	//从首工作开始遍历迭代,计算最早开始
	while(!firstWorkList.empty())
	{
		CalcEarlyBound(firstWorkList);
	}

	//从末工作计算最晚结束
	while(!lastWorkList.empty())
	{
		CalcLatestBound(lastWorkList);
	}
}

//最晚边界计算
void AppendWorkDao::CalcLatestBound( WorkPtr& workPtr,set<Work*>& preWorkList )
{
	////查自己是否已在列表中,防止循环
	AppendWork* pAppWork = Get(workPtr->GetId());

	//确认自己的边界
	if(pAppWork != NULL && pAppWork->GetISSucessedFlg())
	{
		time_t tempTm(LLONG_MAX);
		workPtr->GetWorkPlanEnd(TYPE_PRODUCETM,Dir_Left,pAppWork->initLatestStartTime);
		workPtr->GetWorkPlanEnd(TYPE_PRODUCETM,Dir_Right,tempTm);
		pAppWork->SetRuleLatestEndTm(tempTm);
	}
	else
	{
		pAppWork->initLatestStartTime = min(pAppWork->GetRuleLatestEndTm() - workPtr->GetDefaultPrdTm(),pAppWork->initLatestStartTime);
	}

	//取后工作
	list<Work*> workList;
	workPtr->GetPreworks(workList);
	//遍历，计算后工作的边界
	for(list<Work*>::iterator iter = workList.begin();
		iter != workList.end();iter++)
	{
		WorkPtr&  preWork = **iter;

		AppendWork* preAppWork = Get(preWork->GetId());

		if(preAppWork != NULL)
		{
			preAppWork->SetRuleLatestEndTm(min(preAppWork->GetRuleLatestEndTm(),pAppWork->initLatestStartTime)) ;

			////递归调用
			//CalcLatestBound(preWork,preAppWork,workIdRecoder);
			preWorkList.insert(&preWork);
		}
	}

}

void AppendWorkDao::CalcLatestBound( set<Work*>& workPtrList )
{
	set<Work*> tempWorkList;
	for(set<Work*>::iterator iter = workPtrList.begin();iter != workPtrList.end();iter++)
	{
		WorkPtr&  workPtr = **iter;

		CalcLatestBound(workPtr,tempWorkList);
	}

	workPtrList = tempWorkList;
}

//最早边界计算
void AppendWorkDao::CalcEarlyBound( WorkPtr& workPtr,set<Work*>& postWorkList )
{
	//查自己是否已在列表中,防止循环

	AppendWork* pAppWork = Get(workPtr->GetId());
	//确认自己的边界
	if(pAppWork != NULL && pAppWork->GetISSucessedFlg())
	{
		time_t tempTm(0);
		workPtr->GetWorkPlanEnd(TYPE_PRODUCETM,Dir_Left,tempTm);
		pAppWork->SetRuleEarlyStartTm(tempTm);

		workPtr->GetWorkPlanEnd(TYPE_PRODUCETM,Dir_Right,pAppWork->initEarlyEndTime);
	}
	else
	{
		pAppWork->initEarlyEndTime = max(pAppWork->GetRuleEarlyStartTm() + workPtr->GetDefaultPrdTm(),pAppWork->initEarlyEndTime);
	}

	//取后工作
	list<Work*> workList;
	workPtr->GetPostworks(workList);

	//遍历，计算后工作的边界
	for(list<Work*>::iterator iter = workList.begin();
		iter != workList.end();iter++)
	{
		WorkPtr&  postWork = **iter;

		AppendWork* postAppWork = Get(postWork->GetId());

		if(postAppWork != NULL)
		{
			postAppWork->SetRuleEarlyStartTm(max(postAppWork->GetRuleEarlyStartTm(),pAppWork->initEarlyEndTime)) ;

			//递归调用
			//CalcEarlyBound(postWork,postAppWork,workIdRecoder);
			postWorkList.insert(&postWork);
		}
	}

	//将自己从列表去掉
//	workIdRecoder.pop_back();
}

void AppendWorkDao::CalcEarlyBound( set<Work*>& workPtrList )
{
	set<Work*> tempWorkList;
	for(set<Work*>::iterator iter = workPtrList.begin();iter != workPtrList.end();iter++)
	{
		WorkPtr&  workPtr = **iter;

		CalcEarlyBound(workPtr,tempWorkList);
	}

	workPtrList = tempWorkList;
}

void AppendWorkDao::OutputBound()
{
	std::map<int, AppendWork>::iterator iApp = m_map.begin();
	for (;iApp != m_map.end();iApp++)
	{
		const AppendWork& appWork = iApp->second;
		AMAPI::InsertWorkBound(appWork.id,appWork.earlieststarttime,appWork.latestendtime);
	}
}

void AppendWorkDao::FlashWorkSchFlg()
{
	//数据,存放日历数据集合的容器        
	for(std::map<int, AppendWork>::iterator iter = m_map.begin();
		iter != m_map.end();iter++)
	{
		iter->second.SetOrigSchFlg(iter->second.GetWorkPtr().GetSchedulerFlg());
	}

}

void AppendWorkDao::InitGlancing()
{
	time_t stTm = 0;//AMAPI::GetSchStartTime();
	time_t endTm = LLONG_MAX;//AMAPI::GetSchEndTime();
	WorkPtrMap* pWorkPtrMap = AMAPI::GetWorkPtrMap();
	if (pWorkPtrMap != NULL)
	{
		for (pWorkPtrMap->Begin();pWorkPtrMap->NotEnd();pWorkPtrMap->Step())
		{
			WorkPtr& workPtr= pWorkPtrMap->Get();
			if (workPtr != NULL )
			{
				if (workPtr->IsParticipateSchWork())	//参与排程工作
				{
					AppendWork appendwork;
					appendwork.id = workPtr->GetId();
					appendwork.SetWorkPtr(workPtr);
					appendwork.failFlag = 1;
					appendwork.earlieststarttime = ScheduleHelper::GetGlancingStartTime(
						max(stTm,AMAPI::GetWorkLimit(appendwork.id,STARTAFTER)));
					appendwork.latestendtime = ScheduleHelper::GetGlancingEndTime(
						min(endTm,AMAPI::GetWorkLimit(appendwork.id,ENDBEFORE)));
					appendwork.initEarlyStartTime = appendwork.earlieststarttime;
					appendwork.initEarlyEndTime = appendwork.earliestendtime;
					appendwork.initLatestStartTime = appendwork.lateststarttime;
					appendwork.initLatestEndTime = appendwork.latestendtime;
					appendwork.SetRuleEarlyStartTm(appendwork.initEarlyStartTime);
					appendwork.SetRuleLatestEndTm(appendwork.initLatestEndTime);
					appendwork.SetOrigSchFlg(workPtr->GetSchedulerFlg());
					Add(appendwork);
				}
				else if (workPtr->GetType() == WORKTYPE_ORDER)		//订单
				{
					AppendWork appendwork;
					appendwork.id = workPtr->GetId();
					appendwork.SetWorkPtr(workPtr);
					appendwork.failFlag = 1;
					appendwork.earlieststarttime = ScheduleHelper::GetGlancingStartTime(
						max(stTm,AMAPI::GetWorkLimit(appendwork.id,STARTAFTER)));
					appendwork.latestendtime = ScheduleHelper::GetGlancingEndTime(
						min(endTm,AMAPI::GetWorkLimit(appendwork.id,ENDBEFORE)));
					appendwork.initEarlyStartTime = appendwork.earlieststarttime;
					appendwork.initEarlyEndTime = appendwork.earliestendtime;
					appendwork.initLatestStartTime = appendwork.lateststarttime;
					appendwork.initLatestEndTime = appendwork.latestendtime;
					appendwork.SetRuleEarlyStartTm(appendwork.initEarlyStartTime);
					appendwork.SetRuleLatestEndTm(appendwork.initLatestEndTime);
					appendwork.SetOrigSchFlg(workPtr->GetSchedulerFlg());
					Add(appendwork);
				}
			}
		}
	}
}

void AppendWorkDao::InitParticipateSch()
{
	time_t stTm = 0;//AMAPI::GetSchStartTime();
	time_t endTm = LLONG_MAX;//AMAPI::GetSchEndTime();
	set<unsigned long>& schWorkList = AMAPI::GetAlgWorkSchList();
	WorkPtrMap* pWorkPtrMap = AMAPI::GetWorkPtrMap();
	if (pWorkPtrMap != NULL)
	{
		for (set<unsigned long>::iterator iter = schWorkList.begin();iter != schWorkList.end();++iter)
		{
			WorkPtr& workPtr= pWorkPtrMap->Get(*iter);
			if (workPtr != NULL )
			{
				if (workPtr->IsParticipateSchWork())	//参与排程工作
				{
					AppendWork* pAppWork = Get(workPtr->GetId());
					if(pAppWork == NULL)
					{
						AppendWork appendwork;
						appendwork.id = workPtr->GetId();
						appendwork.SetWorkPtr(workPtr);
						appendwork.failFlag = 1;
						appendwork.earlieststarttime = max(stTm,AMAPI::GetWorkLimit(appendwork.id,STARTAFTER));

						appendwork.latestendtime = min(endTm,AMAPI::GetWorkLimit(appendwork.id,ENDBEFORE));

						appendwork.initEarlyStartTime = appendwork.earlieststarttime;
						appendwork.initEarlyEndTime = appendwork.earliestendtime;
						appendwork.initLatestStartTime = appendwork.lateststarttime;
						appendwork.initLatestEndTime = appendwork.latestendtime;
						appendwork.SetRuleEarlyStartTm(appendwork.initEarlyStartTime);
						appendwork.SetRuleLatestEndTm(appendwork.initLatestEndTime);
						appendwork.SetOrigSchFlg(workPtr->GetSchedulerFlg());
						Add(appendwork);
					}

				}
			}
		}
	}
}

#include "DisobeyRestrict.h"

void AppendWorkDao::CheckDisobeyRestrict()
{
	CDisobeyRestrict disobeyRestrict;

	for(std::map<int, AppendWork>::iterator iter = m_map.begin();
		iter != m_map.end();iter++)
	{
		Work& work = iter->second.GetWorkPtr();
		if(work.ValidPointer() && work.IsParticipateSchWork())
		{
			SBH_INT refWorkId = 0;
			if(disobeyRestrict.CheckMaxIntval(work,refWorkId))
			{
				//违反最大间隔
				AMAPI::InsertWorkOffendRest(7,work.GetId(),refWorkId);
			}

		}
	}
}

void AppendWorkDao::ClearWorkPoint()
{
	for(std::map<int, AppendWork>::iterator iter = m_map.begin();
		iter != m_map.end();iter++)
	{
		iter->second.ClearWorkPoint();
	}
}