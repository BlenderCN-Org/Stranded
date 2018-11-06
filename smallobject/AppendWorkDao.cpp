/*
* Copyright (c) 2008,�����������(�Ϻ�)���޹�˾
* All rights reserved.
*/

/**********************************************************************
 *                         �������ݷ�����                             *
 *                                                                    *
 * ��������  :   ������������                           			  *
 * �������  :   ��������,������ʶ                                    *
 * ���ز���  :	  ���������									      *
 * ��   ��  :                                               		  *
 * ��   ��  :                                  		                  *
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


// ��ʼ����������(Ҫôȫ�Ǹ��ǹ�����Ҫô����ͨ������ʼ��)
void AppendWorkDao::Create(list<WorkPtr>& works)
/*******************************************************************************
//���룺1��Work						��������
//�����1��AppendWork				����������							
*******************************************************************************/
{
	//��1��	���룺WORK
	//��2��	������
	//�����¹��̽���������������ÿ����¼��
	//��	ID���������ǹ���ID (�������ͣ�4)��
	//��	�������翪��ʱ�� �� �գ�
	//��	����������ʱ�� �� �գ�
	//��	���������깤ʱ�� �� �գ�
	//��	���������깤ʱ�� �� �գ�
	//��	��ʼ�������翪��ʱ�� �� �գ�
	//��	��ʼ����������ʱ�� �� �գ�
	//��	��ʼ���������깤ʱ�� �� �գ�
	//��	��ʼ���������깤ʱ�� �� �գ�
	//��	�ų̱�ǣ����ݡ������������������ǹ���ID�ġ��ų̱�ǣ�1�������ų̱�ǣ�1�������ų̱�ǣ�2�� 
	//?	�ų�ʧ�ܱ�ǣ�1��1.�ɹ���2.ʧ�ܣ���
	//��3��	�������������ADD_WORK��

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
//���룺1��const Work& object				����Ҫ��ӵ������еĶ���
//�����1����							
*******************************************************************************/
{
	//�����������в������ID�������ID�����ڣ�����ɹ�
	map<int, AppendWork>::iterator iter = m_map.find(object.id);
	if (iter == m_map.end())
	{
		m_map.insert(std::map<int, AppendWork>::value_type(object.id,object));
	}
// 	std::pair<std::map<int,AppendWork>::iterator,bool > ret =
// 		m_map.insert(std::map<int, AppendWork>::value_type(object.id,object));
// 	//�����ID���ڣ�ʹ���±귽������
// 	if(!ret.second)
// 		m_map[object.id] = object;
	return object.id;
}


bool AppendWorkDao::Get(const int objectid, AppendWork& object) 
/*******************************************************************************
//���룺1��const unsigned int objectId			��ȡֵ�����ID��
//�����1��Work& object						��ȡֵ����
*******************************************************************************/
{
	//�������в�������ID
	std::map<int, AppendWork>::iterator iter = m_map.find(objectid);
	if(iter == m_map.end())
	{
		//û�ҵ�������false
		return false;
	}
	else 
	{
		//�ҵ�������Ҫ����ֵ��object��
		object = iter->second;
		return true;
	}	
}

AppendWork * AppendWorkDao::Get(const int objectid) 
/*******************************************************************************
//���룺1��const unsigned int objectId			��ȡֵ�����ID��
//�����1��Work* object						��ȡֵ�����ָ�룩
*******************************************************************************/
{
	//�������в�������ID
	AppendWork * p_object = NULL;
	std::map<int, AppendWork>::iterator iter = m_map.find(objectid);
	if(iter != m_map.end())
	{
		//�ҵ�������Ҫ����ֵ��object��
		p_object = &iter->second;
	}	
	return p_object;
}


bool AppendWorkDao::Update(const AppendWork& object) 
/*******************************************************************************
//���룺1��const unsigned int objectId			�������¶���
//�������
*******************************************************************************/
{
	//�����в��Ҷ�ӦID
	if(m_map.count(object.id))
	{
		//������ֵ������true
		m_map[object.id] = object;
		return true;
	}
	else
		//�����ڷ���fasle
		return false;
}


void AppendWorkDao::GetAll(std::map<int,AppendWork>& allobject) 
/*******************************************************************************
//���룺��
//�����1��allobject			�����������أ�
*******************************************************************************/
{
	allobject = m_map;
}

std::map<int,AppendWork>* AppendWorkDao::GetAll() 
/*******************************************************************************
//���룺��
//�����1��p_allobject			��������ָ�뷵�أ�
*******************************************************************************/
{
	return &m_map;
}


void AppendWorkDao::StartLoop(void)
{
	//����ѭ����������ֵ
	work_It = m_map.begin();
}

bool AppendWorkDao::Get(AppendWork & object)
/*******************************************************************************
//���룺��
//�����1��const Work& object				��ȡֵ����
*******************************************************************************/
{
	//������δָ������β��ȡֵ�����������true
	if(work_It != m_map.end())
	{
		object = work_It->second;
		work_It++;
		return true;
	}
	//ѭ����������������false
	else
		return false;
}

bool AppendWorkDao::Get(AppendWork** p_object)
/*******************************************************************************
//���룺��
//�����1��const Work** p_object				��ȡֵ����ָ�룩
*******************************************************************************/
{
	//������δָ������β��ȡֵ�����������true
	if(work_It != m_map.end())
	{
		*p_object = &work_It->second;
		work_It++;
		return true;
	}
	//ѭ����������������false
	else
		return false;
}

void AppendWorkDao::Clear(void)
{
	//�������
	m_map.clear();
	maxId = 0;
}


int AppendWorkDao::Create(AppendWork & object)
/*******************************************************************************
//���룺1��object									����Ҫ����id����ӵĹ�������
//�����1����������ֵ								��������id�ţ�
*******************************************************************************/
{
	//object.id = maxId + 1;
	return Add(object);
}

void AppendWorkDao::ClearAppendWork(const list<WorkPtr>& worklist)
/*******************************************************************************
//���룺1��list<int> worklist						������չ����б�
//�����1����
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

//���ɲ����ų̵Ĺ���
void AppendWorkDao::Init()
{
	Clear();
	m_bCaculateWorkBound = false;
	//�����ų̼���߽�
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
				if (workPtr->IsParticipateSchWork())	//�����ų̹���
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
				else if (workPtr->GetType() == WORKTYPE_ORDER)		//����
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
	//	//����߽�(����ʽ������)
	//	CaculateWorkBound();
	//}
}

void AppendWorkDao::InitWorkBound()//��ʼ�������߽�
{
	if (!m_bCaculateWorkBound)
	{
		m_bCaculateWorkBound = true;
		if(pData->newRegulation.IsNeedInitWorkBound())
		{
			//����߽�(����ʽ������)
			CaculateWorkBound();
		}
	}
}
//���㹤���ĳ�ʼ�߽�
void AppendWorkDao::CaculateWorkBound()
{
	//�������й���,ȡ�׹������Ϻ�ĩ��������
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
				//�жϴ˹����Ƿ�Ϊ�׹���
				list<Work*> workList;
				workPtr->GetPreworks(workList);
				if(workList.empty())
				{
					firstWorkList.insert(&workPtr);
					//list<int> workIdRecoder;
					//CalcEarlyBound(workPtr,pAppWork,workIdRecoder);
				}

				//�жϴ˹����Ƿ�Ϊĩ����
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

	//���׹�����ʼ��������,�������翪ʼ
	while(!firstWorkList.empty())
	{
		CalcEarlyBound(firstWorkList);
	}

	//��ĩ���������������
	while(!lastWorkList.empty())
	{
		CalcLatestBound(lastWorkList);
	}
}

//����߽����
void AppendWorkDao::CalcLatestBound( WorkPtr& workPtr,set<Work*>& preWorkList )
{
	////���Լ��Ƿ������б���,��ֹѭ��
	AppendWork* pAppWork = Get(workPtr->GetId());

	//ȷ���Լ��ı߽�
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

	//ȡ����
	list<Work*> workList;
	workPtr->GetPreworks(workList);
	//��������������ı߽�
	for(list<Work*>::iterator iter = workList.begin();
		iter != workList.end();iter++)
	{
		WorkPtr&  preWork = **iter;

		AppendWork* preAppWork = Get(preWork->GetId());

		if(preAppWork != NULL)
		{
			preAppWork->SetRuleLatestEndTm(min(preAppWork->GetRuleLatestEndTm(),pAppWork->initLatestStartTime)) ;

			////�ݹ����
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

//����߽����
void AppendWorkDao::CalcEarlyBound( WorkPtr& workPtr,set<Work*>& postWorkList )
{
	//���Լ��Ƿ������б���,��ֹѭ��

	AppendWork* pAppWork = Get(workPtr->GetId());
	//ȷ���Լ��ı߽�
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

	//ȡ����
	list<Work*> workList;
	workPtr->GetPostworks(workList);

	//��������������ı߽�
	for(list<Work*>::iterator iter = workList.begin();
		iter != workList.end();iter++)
	{
		WorkPtr&  postWork = **iter;

		AppendWork* postAppWork = Get(postWork->GetId());

		if(postAppWork != NULL)
		{
			postAppWork->SetRuleEarlyStartTm(max(postAppWork->GetRuleEarlyStartTm(),pAppWork->initEarlyEndTime)) ;

			//�ݹ����
			//CalcEarlyBound(postWork,postAppWork,workIdRecoder);
			postWorkList.insert(&postWork);
		}
	}

	//���Լ����б�ȥ��
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
	//����,����������ݼ��ϵ�����        
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
				if (workPtr->IsParticipateSchWork())	//�����ų̹���
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
				else if (workPtr->GetType() == WORKTYPE_ORDER)		//����
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
				if (workPtr->IsParticipateSchWork())	//�����ų̹���
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
				//Υ�������
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