#include "MaxIntervalDisposeManager.h"

#include "GlobalVarSBH.h"
#include "Scheduler.h"

CMaxIntervalDisposeManager* CMaxIntervalDisposeManager::m_pMaxDisManager = NULL;

static int iterativeCount = 0;

WorkResInfo CMaxIntervalDisposeManager::m_defWorkResInfo;
CMaxIntervalDisposeManager::CMaxIntervalDisposeManager(void)
{
	m_pCombSeq = NULL;
}

CMaxIntervalDisposeManager::~CMaxIntervalDisposeManager(void)
{
}

void CMaxIntervalDisposeManager::Init()
{
	//m_workPairList.clear();
	m_work2Seq.clear();
	m_origWorkPlan.Init();
	//m_singleValDispose

	pData->parame.m_MaxValCheck = true;

	////�������й������������������
	//for(map<SBH_INT,vector<WorkResInfo> >::iterator iter1 =	m_workResInfoList.begin();
	//	iter1 != m_workResInfoList.end();iter1++)
	//{
	//	for(map<SBH_INT,vector<WorkResInfo> >::iterator iter2 =	iter1;
	//		iter2 != m_workResInfoList.end();iter2++)
	//	{
	//		if(iter1->first != iter2->first)
	//		{
	//			//������������
	//			CheckIfMaxInterval(iter1->first,iter2->first);
	//		}

	//	}
	//}

	SBH_INT i = 0;
	m_combCount = 1;
	for(map<SBH_INT,vector<WorkResInfo> >::iterator iter =	m_workResInfoList.begin();
		iter != m_workResInfoList.end();iter++)
	{
		m_work2Seq[iter->first] = i++;
		m_combCount *= (SBH_INT)iter->second.size();
	}

	//������������100��
	//add by yp 2010.12.22
	if(m_combCount > 100)
	{
		m_combCount = 100;
	}

	NewCombSeq();
}

void CMaxIntervalDisposeManager::Clear()
{
	m_workResInfoList.clear();
	m_workPairList.clear();
	m_work2Seq.clear();

	m_workPlanList.clear();
}

SBH_BOOL CMaxIntervalDisposeManager::MaxIntervalCheck()
{
	Init();

	//�ж��Ƿ���������������
	if(!CheckMaxval())
	{
		pData->parame.m_MaxValCheck = false;
		return true;
	}

	BackUpWorkPlan();
	SBH_BOOL rtFlg = ResolveMaxvalPro();

	//ʧ��
	if( !rtFlg)
	{
		//Υ���������ǿ��
		if(AMAPI::GetParameter()->GetDsbMaxInterVal() == 1)
		{
			//�����ų�Ϊ��������
			//��¼ԭ״̬
			ScheduleMethod tempState = pData->parame.m_curSchMethod;
			pData->parame.m_curSchMethod = SM_FORCE;//(ǿ��)

			rtFlg = ResolveMaxvalPro();
			pData->parame.m_curSchMethod = tempState;
		}
	}

	//���±߽�
	if( !rtFlg)
	{
		//�ָ��ƻ�
		ResumeWorkPlan();
		WorkOffendRestOutput();
	}

	UpdataWorkBound();

	m_vecCombine.clear();
	pData->parame.m_MaxValCheck = false;

	return rtFlg;
}

//�½�����
void CMaxIntervalDisposeManager::NewCombSeq()
{
// 	m_pCombSeq = new SBH_INT[m_combCount * m_workResInfoList.size()];
// 
// 	unsigned int curWorkBlock = m_combCount;
// 	int elem = 0;
// 	for(map<SBH_INT,vector<WorkResInfo> >::iterator iter =	m_workResInfoList.begin();
// 		iter != m_workResInfoList.end();iter++)
// 	{
// 		curWorkBlock = curWorkBlock / iter->second.size();
// 		unsigned int j =0 , k = 0;
// 		for(unsigned int i = 0;i< m_combCount ;i++)
// 		{
// 			if(j >= curWorkBlock)
// 			{
// 				j = 0;
// 				k++;
// 				if(k >= iter->second.size())
// 					k = 0;
// 			}
// 			m_pCombSeq[ i * m_workResInfoList.size() + elem] = k;
// 			j++;
// 		}
// 
// 		elem++;
// 	}

	m_vecCombine.clear();
	YK_ITER iterFirst = m_workResInfoList.begin();
	YK_ITER iterSecond = m_workResInfoList.end();
	if (iterFirst != m_workResInfoList.end())
	{
		iterSecond = iterFirst;
		++iterSecond;
	}

	if (iterSecond != m_workResInfoList.end())
	{
		m_vecTemp.push_back(0);
		CombineResSeq(iterFirst, 0, iterSecond, 0);
		m_vecTemp.clear();
	}

	//list<int> aaa;
	//for(int i=0;i<m_combCount * m_workResInfoList.size();i++)
	//{
	//	aaa.push_back(m_pCombSeq[i]);
	//}
}

//��ȡ��ǰ��ϺŶ�Ӧ�����Ĺ���������Դ��
WorkResInfo& CMaxIntervalDisposeManager::GetWorkResInfo( SBH_UINT workId,SBH_UINT uncombSeqNum )
{
	if (uncombSeqNum >= m_vecCombine.size())
		return m_defWorkResInfo;

	const int nIndex = m_vecCombine[uncombSeqNum][m_work2Seq[workId]];
	return m_workResInfoList[workId][nIndex];
	//return m_workResInfoList[workId][m_pCombSeq[m_workResInfoList.size() * uncombSeqNum + m_work2Seq[workId]]];
}

//���������������
SBH_BOOL CMaxIntervalDisposeManager::ResolveAllComb(SBH_UINT combSeq,SBH_BOOL bFirst)
{

	iterativeCount++;

	//���õ����������100��
	if(iterativeCount > 100)
		return false;

	//�������й�����
	for(list< pair<SBH_INT,SBH_INT> >::iterator combIter = m_workPairList.begin();
		combIter != m_workPairList.end();combIter++)
	{
		//��ȷǰ����
		const SBH_INT& preWorkId = combIter->first;
		const SBH_INT& postWorkId = combIter->second;
		SBH_INT singleFlg = m_singleValDispose.CalcMaxInterval(preWorkId,postWorkId,GetWorkResInfo(preWorkId,combSeq),GetWorkResInfo(postWorkId,combSeq),GetDirType(),bFirst);
		if(singleFlg == RESOLVEPROB)	//���Լ���ģ������˹����ƻ�����Ҫ���¼���
		{
			return ResolveAllComb(combSeq,false);
		}
		else if(singleFlg == CANNOTRESOLVE) //�޷���������������
		{
			return false;
		}

		//δ����Υ��������������һ��
	}

	return true;
}

void CMaxIntervalDisposeManager::CheckIfMaxInterval( SBH_INT firWorkId,SBH_INT secWorkId )
{
	//��ȡ����
	WorkPtr&  firWorkPtr = AMAPI::GetWorkPtrMap()->Get(firWorkId);
	WorkPtr&  secWorkPtr = AMAPI::GetWorkPtrMap()->Get(secWorkId);
	if(firWorkPtr != NULL && secWorkPtr != NULL)
	{
		if(AMAPI::GetMinMaxInterval(firWorkPtr,secWorkPtr,MAXINTELVAL,NULL,NULL) > 0)
		{
			m_workPairList.push_back(make_pair(firWorkId,secWorkId));
		}
		else if(AMAPI::GetMinMaxInterval(secWorkPtr,firWorkPtr,MAXINTELVAL,NULL,NULL) > 0)
		{
			m_workPairList.push_back(make_pair(secWorkId,firWorkId));
		}
	}
}

//���빤����������Ӧ��Դ�����Ϣ
void CMaxIntervalDisposeManager::InsertWorkInfo( SBH_INT workId,list<WorkResInfo>& workResInfoList )
{
	for(list<WorkResInfo>::iterator iter = workResInfoList.begin();
		iter != workResInfoList.end();iter++)
	{
		m_workResInfoList[workId].push_back(*iter);
	}
}

//������Դ��϶�Ӧ���ų̼ƻ�
void CMaxIntervalDisposeManager::InsertWorkPlan( SBH_INT workId,vector<TestSchedulerInfo>& workResInfoList )
{
	m_workPlanList[workId] = workResInfoList;
}

//����ԭ�ƻ�
void CMaxIntervalDisposeManager::BackUpWorkPlan()
{
	for(map<SBH_INT,vector<WorkResInfo> >::iterator iter =	m_workResInfoList.begin();
		iter != m_workResInfoList.end();iter++)
	{
		m_singleValDispose.GetWorkPlan(iter->first,m_origWorkPlan);
		//pData->RetractWorkPlan(iter->first);
	}
}

//�ָ���ԭ�ƻ�
void CMaxIntervalDisposeManager::ResumeWorkPlan()
{
	for(map<SBH_INT,vector<WorkResInfo> >::iterator iter =	m_workResInfoList.begin();
		iter != m_workResInfoList.end();iter++)
	{
		pData->RetractWorkPlan(iter->first);
	}

	m_origWorkPlan.FinallySurePlan();
}

SBH_BOOL CMaxIntervalDisposeManager::SetWorkPlan( SBH_INT combSeq )
{
	for(map<SBH_INT,vector<WorkResInfo> >::iterator iter =	m_workResInfoList.begin();
		iter != m_workResInfoList.end();iter++)
	{
		pData->RetractWorkPlan(iter->first);

		if (combSeq >= m_vecCombine.size())
			return false;

		const int nResIndex = m_vecCombine[combSeq][m_work2Seq[iter->first]];
		TestSchedulerInfo& tempPlan = m_workPlanList[iter->first][nResIndex];//m_workPlanList[iter->first][m_pCombSeq[m_workResInfoList.size() * combSeq + m_work2Seq[iter->first]]];

		if(tempPlan.Empty())
			return false;

		tempPlan.FinallySurePlan();
	}

	return true;
}

void CMaxIntervalDisposeManager::UpdataWorkBound()
{
	CScheduler scheduling;

	AppendWork* pAppWork = NULL;

	for(map<SBH_INT,vector<WorkResInfo> >::iterator iter =	m_workResInfoList.begin();
		iter != m_workResInfoList.end();iter++)
	{		
		pAppWork = pData->GetAppendWork(iter->first);
		if(pAppWork != NULL)
		{
			pAppWork->ResetTm();

			pAppWork->SetISSucessedFlg(true);
			pAppWork->SetISSchedFlg(true);

		}
		
	}

}

//��������Υ��Լ����Ϣ
void CMaxIntervalDisposeManager::WorkOffendRestOutput()
{
	return;
	//�������й�����
	for(list< pair<SBH_INT,SBH_INT> >::iterator combIter = m_workPairList.begin();
		combIter != m_workPairList.end();combIter++)
	{
		//��ȷǰ����
		const SBH_INT& preWorkId = combIter->first;
		const SBH_INT& postWorkId = combIter->second;

		//���������
		if(m_singleValDispose.MaxInterValCheck(preWorkId,postWorkId))
		{
			//Υ�������
			AMAPI::InsertWorkOffendRest(7,preWorkId,postWorkId);
		}

	}
}

SBH_BOOL CMaxIntervalDisposeManager::CheckMaxval()
{
	//�������й�����
	for(list< pair<SBH_INT,SBH_INT> >::iterator combIter = m_workPairList.begin();
		combIter != m_workPairList.end();combIter++)
	{
		//��ȷǰ����
		const SBH_INT& preWorkId = combIter->first;
		const SBH_INT& postWorkId = combIter->second;

		//���������
		if(m_singleValDispose.MaxInterValCheck(preWorkId,postWorkId))
		{
			return true;
		}

	}

	return false;
}

//������������⡣
SBH_BOOL CMaxIntervalDisposeManager::ResolveMaxvalPro()
{
	//����ʹ��ԭ��Դ��������2012.5.9
	//��ȡ��ǰ��Դ�����
	SBH_INT curResI = GetCurResCombId();
	//��ɹ���������
	if(curResI > 0)
	{
		if(ResolveAllComb(curResI))
		{
			return true;
		}	
	}

	//�������й�����Դ���
	for(SBH_UINT i=0;i<m_combCount;i++)
	{
		//���������������ϣ���ÿ��������
		//���ù����ƻ�
		if(!SetWorkPlan(i))
			continue;

		//��ʼ����������
		iterativeCount = 0;
		//��ɹ���������
		if(ResolveAllComb(i))
		{
			return true;
		}	
	}

	return false;
}

void CMaxIntervalDisposeManager::UpdateWorkSchFlg( SBH_BOOL bSucceed )
{

}

void CMaxIntervalDisposeManager::CombineResSeq( YK_ITER iterLeft, size_t nLResIndex, YK_ITER iterRigtht, size_t nRResIndex )
{
	if (FitCombineCondition(iterLeft->second[nLResIndex], iterRigtht->second[nRResIndex]))
	{
		m_vecTemp.push_back(nRResIndex);
		YK_ITER iterTemp = iterRigtht;
		if ((++iterTemp) != m_workResInfoList.end())
		{
			CombineResSeq(++iterLeft, nRResIndex, ++iterRigtht, 0);
		}
		else // �ҵ���һ�����
		{
			m_vecCombine.push_back(m_vecTemp);
			m_vecTemp.pop_back();
			if (nRResIndex < (iterRigtht->second.size() - 1))
			{
				CombineResSeq(iterLeft, nLResIndex, iterRigtht, ++nRResIndex);
			}
			else if (nLResIndex < (iterLeft->second.size() - 1))
			{
				m_vecTemp.pop_back();
				m_vecTemp.push_back(++nLResIndex);
				CombineResSeq(iterLeft, nLResIndex, iterRigtht, 0);
			}
		}
	}
	else
	{
		if (nRResIndex < (iterRigtht->second.size() - 1))
		{
			CombineResSeq(iterLeft, nLResIndex, iterRigtht, ++nRResIndex);
		}
	}
}

bool CMaxIntervalDisposeManager::FitCombineCondition( WorkResInfo& lWorkRes, WorkResInfo& rWorkRes )
{
	static SBH_BOOL bIterator = false;
	pair<SBH_INT, SBH_INT> pairGp(lWorkRes.resRelaID.GetWorkId(), rWorkRes.resRelaID.GetWorkId());
	
	bool bOK = false;
	list< pair<SBH_INT,SBH_INT> >::iterator iterWKPair = find(m_workPairList.begin(), m_workPairList.end(), pairGp);
	if (iterWKPair != m_workPairList.end())
	{
		if (lWorkRes.resRelaID.GetBNxtResRtc())
		{
			set<unsigned long> setBackRes = lWorkRes.resRelaID.GetNxtResList();
			if (setBackRes.find(rWorkRes.resRelaID.GetResId()) != setBackRes.end())
			{
				bOK = true;
			}
		}
		else
		{
			bOK = true;
		}
	}
	else
	{
		if (false == bIterator)
		{
			bIterator = true;
			bOK = FitCombineCondition(rWorkRes, lWorkRes);
			bIterator = false;
		}

	}

	return bOK;
}

void CMaxIntervalDisposeManager::ProMaxValAbort( vector<SBH_INT>& vecWK )
{
	if (vecWK.size() < 2) return;

	set<vector<SBH_INT>>& setMaxIntervalGp = pData->workFilter.GetMaxIntervalGp();
	if (find(setMaxIntervalGp.begin(), setMaxIntervalGp.end(), vecWK) == setMaxIntervalGp.end())
	{
		const SBH_INT nTemp = vecWK[0];
		vecWK[0] = vecWK[1];
		vecWK[1] = nTemp;

		if (find(setMaxIntervalGp.begin(), setMaxIntervalGp.end(), vecWK) == setMaxIntervalGp.end())
		{
			MaxIntervalCheck();
			Clear();
			return;
		}
	}

	m_workPairList.push_back(make_pair(vecWK[0], vecWK[1]));
}

SBH_INT CMaxIntervalDisposeManager::GetCurResCombId()
{
	map<SBH_INT,WorkResInfo> workIdMap;
	//�������й�����
	for(list< pair<SBH_INT,SBH_INT> >::iterator combIter = m_workPairList.begin();
		combIter != m_workPairList.end();combIter++)
	{
		//��ȷǰ����
		workIdMap[combIter->first];
		workIdMap[combIter->second];
	}

	for(map<SBH_INT,WorkResInfo>::iterator iter = workIdMap.begin();iter != workIdMap.end();++iter)
	{
		Work work = AMAPI::GetWorkPtrMap()->Get(iter->first);

		work.GetPlanResRelaCom(iter->second.resRelaID, iter->second.assistResRelaList, true);

	}

	for(SBH_INT i = 0; i< m_vecCombine.size();i++)
	{
		map<SBH_INT,WorkResInfo>::iterator iter = workIdMap.begin();
		for(;iter != workIdMap.end();++iter)
		{
			if(!(GetWorkResInfo(iter->first,i) == iter->second))
				break;
		}

		if(iter == workIdMap.end())
			return i;
	}
	
	return 0;
}