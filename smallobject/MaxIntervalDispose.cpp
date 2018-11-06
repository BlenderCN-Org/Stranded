#include "MaxIntervalDispose.h"
#include "SingleObvCalcV3.h"
#include "SingleConvCalcV3.h"
#include "GlobalVarSBH.h"
#include "ResourceMgr.h"
#include "AM_API.h"

long GetResId(WorkResInfo& workResInfo)
{
	if(workResInfo.resRelaID != NULL)
		return workResInfo.resRelaID->GetResId();
	else 
		return 0;
}



CMaxIntervalDispose::CMaxIntervalDispose(void)
{
}

CMaxIntervalDispose::~CMaxIntervalDispose(void)
{
}

//�����������������
SBH_INT CMaxIntervalDispose::CalcMaxInterval( SBH_INT preWorkId,SBH_INT postWorkId,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo,DirType dir ,SBH_BOOL bFirst )
{
	WorkPtr& preWorkPtr = AMAPI::GetWorkPtrMap()->Get(preWorkId);
	WorkPtr& postWorkPtr = AMAPI::GetWorkPtrMap()->Get(postWorkId);
	if(preWorkPtr == NULL || postWorkPtr == NULL)
		return CANNOTRESOLVE;

	//�������������
	Init();

	if(bFirst)
		m_changeRes = true;

	//ȡ��ԭ�ƻ����ݵĸ���
	GetWorkPlan(preWorkId,m_preWorkPlan);
	GetWorkPlan(postWorkId,m_postWorkPlan);

	//�����Դ�ƻ�
	AMAPI::ClearWorkPlan(preWorkId);
	AMAPI::ClearWorkPlan(postWorkId);
	//pData->RetractWorkPlan(preWorkId);
	//pData->RetractWorkPlan(postWorkId);

	if(CalcSingleLoop(preWorkPtr,postWorkPtr,preWorkResInfo,postWorkResInfo,dir))
	{	
		return m_checkflg;
	}
	else
	{
		return CANNOTRESOLVE;
	}

}

void CMaxIntervalDispose::GetWorkPlan( SBH_INT workId,TestSchedulerInfo& workSchInfo )
{
	WorkPtr& workPtr = AMAPI::GetWorkPtrMap()->Get(workId);
	if(workPtr == NULL)
		return;

	//�����ƻ��б�
	list<WorkSchPlanPtr> schPlanlanPtrList = workPtr->GetSchPlanList();

	map<SBH_INT, map<SBH_ULONG,TimeRangeList> > 	planList;

	//�����ƻ���ȡ��Ӧ��Ϣ
	for(list<WorkSchPlanPtr>::iterator iter = schPlanlanPtrList.begin();
		iter != schPlanlanPtrList.end();iter++)
	{
		WorkSchPlanPtr& schPlan = *iter;
		planList[schPlan->GetType()][schPlan->GetPlanSelResId()].push_back(TimeRange(schPlan->GetStartTime(),schPlan->GetEndTime(),(SBH_DOUBLE)schPlan->GetUsedQuantity()));
	}

	//����ɾ���ƻ���������Դ����
	for(map<SBH_INT, map<SBH_ULONG,TimeRangeList> >::iterator iter = planList.begin();
		iter != planList.end();iter++)
	{
		workSchInfo.InsertAddPlan(workPtr->GetId(),iter->first,iter->second);
	}
}

//����Դ���������ݹ�
SBH_BOOL CMaxIntervalDispose::CalcSingleLoop( WorkPtr& preWorkPtr ,WorkPtr& postWorkPtr,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo ,DirType dir )
{
	SBH_BOOL trFlg = false;
	if(dir == Dir_Obverse)//����
	{
		trFlg = CalcSingleLoopObv(preWorkPtr,postWorkPtr,preWorkResInfo,postWorkResInfo);
	}
	else	//����
	{
		trFlg = CalcSingleLoopConv(preWorkPtr,postWorkPtr,preWorkResInfo,postWorkResInfo);
	}

	//ȷ�ϼƻ�
	m_preWorkPlan.SurePlan();
	m_postWorkPlan.SurePlan();

	return trFlg;
}

//������Դ�����������
SBH_BOOL CMaxIntervalDispose::CalcSingleLoopObv( WorkPtr& preWorkPtr ,WorkPtr& postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo )
{
	//��ȡ�����С���
	int minInterval = AMAPI::GetMinMaxInterval(preWorkPtr,postWorkPtr,MININTELVAL,preWorkResInfo.resRelaID,postWorkResInfo.resRelaID);
	int maxInterval = AMAPI::GetMinMaxInterval(preWorkPtr,postWorkPtr,MAXINTELVAL,preWorkResInfo.resRelaID,postWorkResInfo.resRelaID);

	AppendWork* pPreWork = pData->GetAppendWork(preWorkPtr->GetId());

	AppendWork* pPostWork = pData->GetAppendWork(postWorkPtr->GetId());

	if(pPreWork == NULL || pPostWork == NULL)
		return false;

	SBH_BOOL rtFlg = ObvMaxInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
		, minInterval, maxInterval, pPreWork, pPostWork);

	if(rtFlg && m_checkflg == NOTDISOBEY)
	{
		return ObvMinInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
			, minInterval, maxInterval, pPreWork, pPostWork);
	}
	else
	{
		return rtFlg;
	}

}

//������Դ�����������
SBH_BOOL CMaxIntervalDispose::CalcSingleLoopConv( WorkPtr& preWorkPtr ,WorkPtr& postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo)
{
	//��ȡ�����С���
	int minInterval = AMAPI::GetMinMaxInterval(preWorkPtr,postWorkPtr,MININTELVAL,preWorkResInfo.resRelaID,postWorkResInfo.resRelaID);
	int maxInterval = AMAPI::GetMinMaxInterval(preWorkPtr,postWorkPtr,MAXINTELVAL,preWorkResInfo.resRelaID,postWorkResInfo.resRelaID);

	AppendWork* pPreWork = pData->GetAppendWork(preWorkPtr->GetId());

	AppendWork* pPostWork = pData->GetAppendWork(postWorkPtr->GetId());

	if(pPreWork == NULL || pPostWork == NULL)
		return false;

	SBH_BOOL rtFlg = ConvMaxInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
		, minInterval, maxInterval, pPreWork, pPostWork);

	if(rtFlg && m_checkflg == NOTDISOBEY)
	{
		return ConvMinInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
			, minInterval, maxInterval, pPreWork, pPostWork);
	}
	else
	{
		return rtFlg;
	}
}

//��ȡǰ��������ʱ���
time_t CMaxIntervalDispose::GetPreWorkPdEndTm(SBH_LONG workId,SBH_LONG resId)
{
	if(resId != 0) 
		return m_preWorkPlan.GetPdEndTime(workId,resId);
	else 
		return m_preWorkPlan.GetPdEndTime(workId);
}

//��ȡ��������ʱ���
time_t CMaxIntervalDispose::GetPreWorkPdStTm(SBH_LONG workId,SBH_LONG resId)
{
	if(resId != 0) 
		return m_preWorkPlan.GetPdStTime(workId,resId);
	else
		return m_preWorkPlan.GetPdStTime(workId);
}

//��ȡǰ��������ʱ���
time_t CMaxIntervalDispose::GetPostWorkPdEndTm(SBH_LONG workId,SBH_LONG resId)
{
	if(resId != 0) 
		return m_postWorkPlan.GetPdEndTime(workId,resId);
	else 
		return m_postWorkPlan.GetPdEndTime(workId);
}

//��ȡ��������ʱ���
time_t CMaxIntervalDispose::GetPostWorkPdStTm(SBH_LONG workId,SBH_LONG resId)
{
	if(resId != 0) 
		return m_postWorkPlan.GetPdStTime(workId,resId);
	else
		return m_postWorkPlan.GetPdStTime(workId);
}



SBH_BOOL CMaxIntervalDispose::ObvMaxInterValCheck(WorkPtr&  preWorkPtr ,WorkPtr&  postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo
												  ,SBH_INT minInterval,SBH_INT maxInterval,AppendWork* pPreWork,AppendWork* pPostWork)
{
	//��ȡǰ�����������ʱ��
//	time_t preWorkTmPoint = GetPreWorkTmPoint(preWorkPtr->GetId(),GetResId(preWorkResInfo));
//	time_t postWorkTmPoint = GetPostWorkTmPoint(postWorkPtr->GetId(),GetResId(postWorkResInfo));
	InitWorkTm( preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo);

	//Υ�������ʱ��
	SBH_INT maxValTm(0);

	//�����������
	CSingleObvCalcV3* pCalc = CSingleObvCalcV3::Get();

	//�ж��Ƿ���������,��Լ������
	if(m_workBoundForMaxVal.CheckMinMaxIntval(preWorkPtr,postWorkPtr,false,maxValTm))//Υ��
	{
		//����Υ��
		m_checkflg = RESOLVEPROB;

		//������������л���Դ���һ�ν���������飬ֱ�ӽ���С���
		if(m_changeRes && pPostWork->GetSchLevel() >= SL_Level3)
		{
			return ObvMinInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
				, minInterval, maxInterval, pPreWork, pPostWork);
		}

		//�ж�ǰ����״̬
		if( pPreWork->GetSchLevel() < SL_Level3)
		{
			return false;
		}

		m_changeRes = false;

		//��ʼ������ʱ��
		//pData->schTools.m_lastSucessTestSchInfo.Init();
		pData->schTools.Init();
		pCalc->GetPrdTm(preWorkPtr,preWorkResInfo,pData->schTools.m_prdTimeLen);

		//����ǰ������߽磬�ų�
		//�߽���ʼ��Ϊ������ʼʱ��-�����
		//ǰ������߽�
//		SBH_TIME newLeftLimit = CalcLimitStTm(GetWorkPdStTm(preWorkPtr,GetResId(preWorkResInfo)) + maxValTm,preWorkResInfo,Dir_Right);
		//ǰ������߽�
//		SBH_TIME newLeftLimit = pCalc->GetNewLimit(limitStTm,preWorkResInfo,pData->schTools.m_prdTimeLen,Dir_Left);

		SBH_TIME newLeftLimit = GetPreWorkPdStTm(preWorkPtr->GetId(),GetResId(preWorkResInfo)) + maxValTm;
		//ȡ��ǰ�����ƻ�
		m_preWorkPlan.ClearPlan();

		if(pCalc->Schedule(preWorkPtr,newLeftLimit,preWorkResInfo,TYPE_PRODUCETM))
		{
			pData->schTools.MeragePlan();
			//�ų̳ɹ�
			m_preWorkPlan = pData->schTools.m_lastSucessTestSchInfo;
			//pData->schTools.m_lastSucessTestSchInfo.Init();
			pData->schTools.Init();

			return ObvMinInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
				, minInterval, maxInterval, pPreWork, pPostWork);
		}
		//�ų�ʧ�ܣ�����
		return false;

	}

	m_changeRes = false;
	return true;

}

SBH_BOOL CMaxIntervalDispose::ObvMinInterValCheck(WorkPtr&  preWorkPtr ,WorkPtr&  postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo
												  ,SBH_INT minInterval,SBH_INT maxInterval,AppendWork* pPreWork,AppendWork* pPostWork)
{
	//��ȡǰ�����������ʱ��
//	time_t preWorkTmPoint = GetPreWorkTmPoint(preWorkPtr->GetId(),GetResId(preWorkResInfo));
//	time_t postWorkTmPoint = GetPostWorkTmPoint(postWorkPtr->GetId(),GetResId(postWorkResInfo));
	InitWorkTm( preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo);

	//�����������
	CSingleObvCalcV3* pCalc = CSingleObvCalcV3::Get();

	SBH_INT minValTm(0);

	//�ж��Ƿ������С���,��Լ������
	if(m_workBoundForMaxVal.CheckMinMaxIntval(preWorkPtr,postWorkPtr,true,minValTm) || m_changeRes)//Υ��
	{
		m_checkflg = RESOLVEPROB;

		//�ж�ǰ����״̬
		if(pPostWork->GetSchLevel() < SL_Level3)
		{
			return false;
		}

		//���º�����߽磬�ų�
		SBH_TIME newLeftLimit = GetPostWorkPdStTm(postWorkPtr->GetId(),GetResId(postWorkResInfo)) + minValTm;

		//������Դ���һ����С�������
		if(	m_changeRes )
		{
			m_changeRes = false;
			//newLeftLimit = GetPostWorkPdStTm(postWorkPtr,GetResId(postWorkResInfo)) + minValTm;
		}

		//ȡ�������ƻ�
		m_postWorkPlan.ClearPlan();

		//��ʼ������ʱ��
		//pData->schTools.m_lastSucessTestSchInfo.Init();
		pData->schTools.Init();
		pCalc->GetPrdTm(postWorkPtr,postWorkResInfo,pData->schTools.m_prdTimeLen);
		if(pCalc->Schedule(postWorkPtr,newLeftLimit,postWorkResInfo,TYPE_PRODUCETM))
		{
			pData->schTools.MeragePlan();
			//�ų̳ɹ�
			m_postWorkPlan = pData->schTools.m_lastSucessTestSchInfo;
			//pData->schTools.m_lastSucessTestSchInfo.Init();
			pData->schTools.Init();

			return ObvMaxInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
				, minInterval, maxInterval, pPreWork, pPostWork);
		}
		//�ų�ʧ�ܣ�����
		return false;

	}

	return true;
}

SBH_BOOL CMaxIntervalDispose::ConvMaxInterValCheck( WorkPtr&  preWorkPtr ,WorkPtr&  postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo 
												   ,SBH_INT minInterval,SBH_INT maxInterval,AppendWork* pPreWork,AppendWork* pPostWork )
{
	//��ȡǰ�����������ʱ��
//	time_t preWorkTmPoint = GetPreWorkTmPoint(preWorkPtr->GetId(),GetResId(preWorkResInfo));
//	time_t postWorkTmPoint = GetPostWorkTmPoint(postWorkPtr->GetId(),GetResId(postWorkResInfo));
	InitWorkTm( preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo);

	//�����������
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();

	SBH_INT maxValTm(0);
	//�ж��Ƿ���������,��Լ������
	if(m_workBoundForMaxVal.CheckMinMaxIntval(preWorkPtr,postWorkPtr,false,maxValTm))//Υ��
	{
		//����Υ��
		m_checkflg = RESOLVEPROB;

		//������������л���Դ���һ�ν���������飬ֱ�ӽ���С���
		if(m_changeRes && pPreWork->GetSchLevel() >= SL_Level3)
		{
			return ConvMinInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
				, minInterval, maxInterval, pPreWork, pPostWork);
		}

		//�жϺ���״̬
		if(pPostWork->GetSchLevel() < SL_Level3)
		{
			return false;
		}

		m_changeRes = false;

		SBH_TIME newRightLimit = GetPostWorkPdEndTm(postWorkPtr->GetId(),GetResId(postWorkResInfo)) - maxValTm;

		//ȡ�������ƻ�
		m_postWorkPlan.ClearPlan();

		//��ʼ������ʱ��
		//pData->schTools.m_lastSucessTestSchInfo.Init();
		pData->schTools.Init();
		pCalc->GetPrdTm(postWorkPtr,postWorkResInfo,pData->schTools.m_prdTimeLen);

		//���º����ұ߽磬�ų�
//		SBH_TIME limitStTm = CalcLimitStTm(preWorkTmPoint + maxInterval,postWorkResInfo,Dir_Left);
//		SBH_TIME newRightLimit = pCalc->GetNewLimit(limitStTm,postWorkResInfo,pData->schTools.m_prdTimeLen,Dir_Right);

		if(pCalc->Schedule(postWorkPtr,newRightLimit,postWorkResInfo,TYPE_PRODUCETM))
		{
			pData->schTools.MeragePlan();
			//�ų̳ɹ�
			m_postWorkPlan = pData->schTools.m_lastSucessTestSchInfo;
			//pData->schTools.m_lastSucessTestSchInfo.Init();
			pData->schTools.Init();

			return ConvMinInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
				, minInterval, maxInterval, pPreWork, pPostWork);
		}
		//�ų�ʧ�ܣ�����
		return false;

	}

	m_changeRes = false;
	return true;
}

SBH_BOOL CMaxIntervalDispose::ConvMinInterValCheck( WorkPtr&  preWorkPtr ,WorkPtr&  postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo 
												   ,SBH_INT minInterval,SBH_INT maxInterval,AppendWork* pPreWork,AppendWork* pPostWork )
{
	//��ȡǰ�����������ʱ��
//	time_t preWorkTmPoint = GetPreWorkTmPoint(preWorkPtr->GetId(),GetResId(preWorkResInfo));
//	time_t postWorkTmPoint = GetPostWorkTmPoint(postWorkPtr->GetId(),GetResId(postWorkResInfo));
	InitWorkTm( preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo);

	//�����������
	CSingleConvCalcV3* pCalc = CSingleConvCalcV3::Get();

	SBH_INT minValTm(0);
	//�ж��Ƿ������С���,��Լ������
	if(m_workBoundForMaxVal.CheckMinMaxIntval(preWorkPtr,postWorkPtr,true,minValTm)|| m_changeRes)//Υ��
	{
		//����Υ��
		m_checkflg = RESOLVEPROB;

		//�ж�ǰ����״̬
		if(pPreWork->GetSchLevel() < SL_Level3)
		{
			return false;
		}

		//����ǰ�����ұ߽磬�ų�
		//SBH_TIME newRightLimit = m_postWorkPlan.GetPdStTime(postWorkPtr->GetId(),postWorkResInfo.resRelaID->GetResId()) - minInterval ;
		SBH_TIME newRightLimit = GetPreWorkPdEndTm(preWorkPtr->GetId(),GetResId(preWorkResInfo)) - minValTm;

		//������Դ���һ����С�������
		if(	m_changeRes )
		{
			m_changeRes = false;
			//newRightLimit = postWorkTmPoint - minInterval;
		}

		//ȡ��ǰ�����ƻ�
		m_preWorkPlan.ClearPlan();

		//��ʼ������ʱ��
		//pData->schTools.m_lastSucessTestSchInfo.Init();
		pData->schTools.Init();
		pCalc->GetPrdTm(preWorkPtr,preWorkResInfo,pData->schTools.m_prdTimeLen);
		if(pCalc->Schedule(preWorkPtr,newRightLimit,preWorkResInfo,TYPE_PRODUCETM))
		{
			pData->schTools.MeragePlan();
			//�ų̳ɹ�
			m_preWorkPlan = pData->schTools.m_lastSucessTestSchInfo;
			//pData->schTools.m_lastSucessTestSchInfo.Init();
			pData->schTools.Init();

			return ConvMaxInterValCheck(preWorkPtr , postWorkPtr , preWorkResInfo , postWorkResInfo
				, minInterval, maxInterval, pPreWork, pPostWork);
		}
		//�ų�ʧ�ܣ�����
		return false;

	}

	return true;
}

//SBH_TIME CMaxIntervalDispose::GetNewLimit(const SBH_TIME& startTm,WorkResInfo& workResInfo,const TimeLenInfo& pdLen, SBH_INT dir)
//{
//	UnionFindMode findInfo;
//
//	//д��Դ��(��������Դ)
//	findInfo.m_resCaps[workResInfo.resRelaID->GetResId()] = 0;
//	//����Դ
//	for(list<WorkResRelaPtr>::iterator iter = workResInfo.assistResRelaList.begin();
//		iter != workResInfo.assistResRelaList.end();iter++)
//	{
//		findInfo.m_resCaps[(*iter)->GetResId()] = 0;
//	}
//
//	findInfo.m_time = startTm;
//
//	findInfo.m_len = pdLen.Max();//��׼�ӹ�ʱ��
//	findInfo.m_interrupt = 0;//����жϣ�-1�����жϣ�0�������ж�
//	findInfo.m_bLock = false;//�Ƿ�Ϊ����
//	findInfo.m_bStrideWork = false;//�Ƿ���Կ繤��
//	findInfo.m_bOverTime = false;//�Ƿ���ԼӰ�
//	findInfo.m_bInfinity = true;//�Ƿ����������ų�
//	findInfo.m_bIgnoreFurnaceRes = true;//�Ƿ������¯��Դ
//	findInfo.m_nType = TYPE_PRODUCETM;//���� TYPE_PRODUCETM ....
//
//	if(dir == Dir_Left)	//����������ʱ�������
//	{
//		if(pData->mgRemainCapacityDao.UnionFindIdleF2(findInfo,Dir_Converse))
//			return max(findInfo.m_outStartTime,AMAPI::GetSchStartTime());
//	}
//	else
//	{
//		if(pData->mgRemainCapacityDao.UnionFindIdleF2(findInfo,Dir_Obverse))
//			return min(findInfo.m_outEndTime,AMAPI::GetSchEndTime());
//	}
//
//	return startTm;
//}

void CMaxIntervalDispose::Init()
{
	m_preWorkPlan.Init();
	m_postWorkPlan.Init();
	m_workBoundForMaxVal.Clear();

	m_checkflg = NOTDISOBEY;
	m_changeRes = false;
}

SBH_BOOL CMaxIntervalDispose::MaxInterValCheck( SBH_INT preWorkId,SBH_INT postWorkId )
{
	//�������������
	//Init();

	//��ȡǰ��������ָ��
	WorkPtr&  preWorkPtr = AMAPI::GetWorkPtrMap()->Get(preWorkId);
	WorkPtr&  postWorkPtr = AMAPI::GetWorkPtrMap()->Get(postWorkId);

	//�й��������ڡ����������Υ����Ϣ
	if(!(preWorkPtr != NULL && postWorkPtr != NULL))
		return true;

	//WorkResRelaPtr preMainRes = NULL,postMainRes = NULL;
	//list<WorkResRelaPtr> preAssRes,postAssRes;

	////ȡ��ԭ�ƻ�����Դ
	//AMAPI::GetPlanResRelaCom(preWorkPtr,preMainRes,preAssRes);
	//AMAPI::GetPlanResRelaCom(postWorkPtr,postMainRes,postAssRes);

	////��ȡ�����С���
	////	int minInterval = AMAPI::GetMinMaxInterval(preWorkPtr,postWorkPtr,MININTELVAL,preWorkResInfo.resRelaID,postWorkResInfo.resRelaID);
	//int maxInterval = AMAPI::GetMinMaxInterval(preWorkPtr,postWorkPtr,MAXINTELVAL,preMainRes,postMainRes);

	//time_t postWorkTmPoint(0),preWorkTmPoint(0);

	CWorkBoundV3 workBoundV3;
	SBH_INT balance;

	//�ж��Ƿ���������,��Լ������
	//if(maxInterval > 0 && postWorkTmPoint - preWorkTmPoint > maxInterval)//Υ��
	if(workBoundV3.CheckMinMaxIntval(preWorkPtr,postWorkPtr,false,balance))
	{
		return true;
	}

	return false;
}

//����߽翪ʼʱ��
SBH_TIME CMaxIntervalDispose::CalcLimitStTm( const SBH_TIME& startTm ,WorkResInfo& workResInfo, SBH_INT dir)
{
	SBH_TIME rtTm(0);
	if(dir == Dir_Left)
	{
		SBH_TIME stTm = pData->mgRemainCapacityDao.GetOnDutyTime(workResInfo,startTm);
		if(stTm > startTm)
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

			rtTm = pData->mgRemainCapacityDao.GetOnPreDutyTime(findInfo,startTm) - 1;
		}
		else
		{
			//�ж��Ƿ�Ϊ�°࿪ʼ
			if(pData->mgRemainCapacityDao.IfOffDutyTimePoint(workResInfo.resRelaID->GetResId(),startTm+1) /*> (startTm+1)*/)
			{
				rtTm = startTm - 1;
			}
			else
				rtTm = stTm;//��ʼʱ���
		}
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

		SBH_TIME stTm = pData->mgRemainCapacityDao.GetOnPreDutyTime(findInfo,startTm);
		if(stTm < startTm)
		{
			rtTm = pData->mgRemainCapacityDao.GetOnDutyTime(workResInfo,startTm) + 1;

		}
		else
		{
			//�ж��Ƿ�Ϊ�ϰ࿪ʼ
			if(pData->mgRemainCapacityDao.GetOnPreDutyTime(findInfo,startTm-1) < (startTm-1))
			{
				rtTm = startTm + 1;//��ʼʱ���
			}
			else
				rtTm = startTm ;//��ʼʱ���
		}
	}

	return rtTm;
}

void CMaxIntervalDispose::InitWorkTm(WorkPtr&  preWorkPtr ,WorkPtr&  postWorkPtr ,WorkResInfo& preWorkResInfo ,WorkResInfo& postWorkResInfo)
{
	m_workBoundForMaxVal.InsertWorkTm(preWorkPtr->GetId(),STTMFORMAXVAL,GetPreWorkPdStTm(preWorkPtr->GetId(),GetResId(preWorkResInfo)));
	m_workBoundForMaxVal.InsertWorkTm(preWorkPtr->GetId(),ENDTMFORMAXVAL,GetPreWorkPdEndTm(preWorkPtr->GetId(),GetResId(preWorkResInfo)));
	m_workBoundForMaxVal.InsertWorkTm(postWorkPtr->GetId(),STTMFORMAXVAL,GetPostWorkPdStTm(postWorkPtr->GetId(),GetResId(postWorkResInfo)));
	m_workBoundForMaxVal.InsertWorkTm(postWorkPtr->GetId(),ENDTMFORMAXVAL,GetPostWorkPdEndTm(postWorkPtr->GetId(),GetResId(postWorkResInfo)));
}
