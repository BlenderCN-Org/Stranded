#include "stdAfx.h"
#include "ApsCal.h"
#include "Biz_Global.h"
#include <math.h>
#include "NewRule.h"
#include  "InforTransfor.h"
#include "CodeBringRule.h"
#include "DataApi.h"
#include <algorithm>
#include "Tools.h"

#ifdef _CLOCK_DEBUG
#include "clock.h"

static Timer T_NewCreateRela1(L"NewCreateRela1");
static Timer T_NewCreateRela2(L"NewCreateRela2");
static Timer T_NewCreateRela3(L"NewCreateRela3");
static Timer T_UpdateRelation(L"UpdateRelation");
static Timer T_SaveWorkOutput(L"SaveWorkOutput");
#define  CLOCK_PASE(tm)  tm.Pause();
#else
#define  CLOCK_PASE(tm) 
#endif

template<class Container1,class Container2>
Container1& RemoveList(Container1& dst,Container2& src)
{
	for(Container2::iterator iter = src.begin();
		iter != src.end();iter++)
	{
		dst.remove(*iter);
	}
	return dst;
}


ApsCal::ApsCal(void)
{
	 m_unused        =0;
	 m_supplyAmout   =0;
	 m_totalAmout    =0;          //���漯��0��1��3�еĶ�������֮��
	 m_assignAmout   =0;          //�����������
}

ApsCal::~ApsCal(void)
{
}

//����������ϵ
void ApsCal::CreateRelation( YKItemPtr& itemPtr )
{
	//��Ʒ���Ϸ�ʽ��
	if(itemPtr->GetStockMode() == 1) //������
	{
		return;
	}

	//���������Ʒ����
	//������Ʒ
	list<YKWorkInputItemPtr>  workInputItemListOrg = itemPtr->GetWorkInList();

	//���ɷ���������������Ʒ����
	list<YKWorkInputItemPtr>  workInputItemList;
	CreatWorkInputList(itemPtr,workInputItemList,workInputItemListOrg);
	//ѡ�������Ʒ
	list<YKWorkOutputItemPtr> workOutputItemListOrg = itemPtr->GetWorkOutList();

	list<YKWorkOutputItemPtr> workOutputItemList1;
	list<YKWorkOutputItemPtr> workOutputItemList2;
	CreatWorkOutputList(itemPtr,workOutputItemList1,workOutputItemList2,workOutputItemListOrg);

	//������
	YK_FLOAT minStockCount  = itemPtr->GetMinStock();

		//����������Ʒ�б�
		for(list<YKWorkInputItemPtr>::iterator iterIItem =workInputItemList.begin()
			;iterIItem!=workInputItemList.end();)
		{
			if((*iterIItem)->GetScarcityAmount() < 0.000001)
			{
				iterIItem = workInputItemList.erase(iterIItem);
				continue;
			}
			//ֻ��������
			{
				YKOrderPtr orderPtr = g_pBizModel->GetYKOrderPtrMap()->Get((*iterIItem)->GetOrderId());
				if(orderPtr != NULL)
				{
					YKOrderPtr peakOrder = orderPtr->GetPeakOrder();

					if(peakOrder == NULL)	peakOrder = orderPtr;

					if(peakOrder != NULL
						&& (peakOrder->GetDesignation() != L""
						|| peakOrder->GetType() == OrderType_WillOrder) ) //�������췬�Ż����򶩵�
					{
						//�����򶩵������������췬��
						if(peakOrder->GetType() != OrderType_WillOrder)
						{
							for(list<YKWorkOutputItemPtr>::iterator iterOut =workOutputItemList1.begin()
								;iterOut!=workOutputItemList1.end();)
							{
								if((*iterOut)->GetSpilthAmount() < 0.00001)
								{
									iterOut = workOutputItemList1.erase(iterOut);
									continue;
								}

								Input2OutputByWillOrderAndDesignation(peakOrder,*iterIItem,workOutputItemList1,workOutputItemList2);


								YKOrderPtr orderOutPtr = g_pBizModel->GetYKOrderPtrMap()->Get((*iterOut)->GetOrderId()); 
								if(orderOutPtr != NULL
									&& orderOutPtr->GetDesignation() == orderPtr->GetDesignation())
								{
									//�����������
									Input2Output(*iterIItem,*iterOut,(*iterOut)->GetSpilthAmount());

									if(BZERO((*iterOut)->GetSpilthAmount())) //��������Ϊ0
									{
										iterOut = workOutputItemList1.erase(iterOut); //ɾ�������
										if(BZERO((*iterIItem)->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
										{
											break;
										}
										else
											continue;
									}
									if(BZERO((*iterIItem)->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
									{
										break;
									}
								}
								if(iterOut == workOutputItemList1.end())
									break;

								iterOut++;
							}
						}
						//���Ʒ��ŵ����򶩵�
						else //if(!peakOrder->GetDesignation().empty())
						{
							//�������
							Input2OutputByWillOrderAndDesignation(peakOrder,*iterIItem,workOutputItemList1,workOutputItemList2);
							//�������충��
							Input2OutputByWillOrder(peakOrder,*iterIItem,workOutputItemList1,workOutputItemList2);
						}

						//���򶩵�
						if(itemPtr->GetAutoSupply() == 1) //���Զ�����,�˳�
						{
							iterIItem++;
							continue;
						}
						else
						{					
							//���򶩵�
							if (!BatchManageByDemand(*iterIItem))
								CreateSpecialOrder(*iterIItem);
							 iterIItem++;
							 continue;
						}


					}
				}
			}



			if(itemPtr->GetAutoSupply() != 3) //��1��1����
			{
				//����1ȡһ������������¼
				for(list<YKWorkOutputItemPtr>::iterator iterOut =workOutputItemList1.begin()
					;iterOut!=workOutputItemList1.end();)
				{
					YKWorkOutputItemPtr& workOutItem = *iterOut;

					if((*iterOut)->GetSpilthAmount() < 0.00001)
					{
						iterOut = workOutputItemList1.erase(iterOut);
						continue;
					}

					YKOrderPtr orderInnPtr = g_pBizModel->GetYKOrderPtrMap()->Get((*iterIItem)->GetOrderId());
					if(orderInnPtr != NULL
						&& orderInnPtr->GetDesignation() != L"")
					{
						iterOut++;
						continue;
					}

					//������С�ڿ������
					if(workOutItem->GetSpilthAmount() <= minStockCount)
					{
						minStockCount -= workOutItem->GetSpilthAmount();

						workOutItem->SetStockAmount(workOutItem->GetSpilthAmount());
						workOutItem->SetSpilthAmount(0);

						iterOut = workOutputItemList1.erase(iterOut);
						continue;
					}
					else
					{
						//������棨��������
						workOutItem->SetSpilthAmount(workOutItem->GetSpilthAmount()-minStockCount);
						workOutItem->SetStockAmount(minStockCount);
//						YK_FLOAT usedQuantity = workOutItem->GetSpilthAmount() - minStockCount;

						minStockCount = 0;

						//�����������
						Input2Output(*iterIItem,*iterOut,workOutItem->GetSpilthAmount());

						if(BZERO((*iterOut)->GetSpilthAmount())) //��������Ϊ0
						{
							iterOut = workOutputItemList1.erase(iterOut); //ɾ�������

							if(BZERO((*iterIItem)->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
								break;
							else
								continue;
						}
						else  //�����Ʒ����������Ϊ0��������Ʒ����������Ϊ0
							break;
					}
				}



			}

			if(BZERO((*iterIItem)->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
			{
				iterIItem = workInputItemList.erase(iterIItem);
				continue;
			}


			//���м�Ʒ��������
			if(itemPtr->GetAutoSupply() == 1) //���Զ�����,�˳�
			{
				break;
			}

			{
				//��2ȡһ������������¼
				for(list<YKWorkOutputItemPtr>::iterator iterOut =workOutputItemList2.begin()
					;iterOut!=workOutputItemList2.end();)
				{
					if((*iterOut)->GetSpilthAmount() < 0.00001)
					{
						iterOut = workOutputItemList2.erase(iterOut);
						continue;
					}
					//���¶�������
					CreateSingleDocument(*iterIItem,workOutputItemList2,1);

					if(BZERO((*iterOut)->GetSpilthAmount())) //��������Ϊ0
					{
						iterOut = workOutputItemList2.erase(iterOut); //ɾ�������

						if(BZERO((*iterIItem)->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
							break;
						else
							continue;
					}
					else  //�����Ʒ����������Ϊ0��������Ʒ����������Ϊ0
						break;
				}

				//������Ʒ����Ϊ0
				if(BZERO((*iterIItem)->GetScarcityAmount()))
				{
					iterIItem = workInputItemList.erase(iterIItem);
					continue;
				}
				else
				{
					CreateSingleDocument(*iterIItem,workOutputItemList2);

				}

			}

			iterIItem++;
		}

		//����
		if(minStockCount > 0.000001
			&&(itemPtr->GetAutoSupply() == 2
			|| itemPtr->GetAutoSupply() == 5))
		{
			for(list<YKWorkOutputItemPtr>::iterator iterOut =workOutputItemList1.begin()
				;iterOut!=workOutputItemList1.end();)
			{
				YKWorkOutputItemPtr& workOutItem = *iterOut;

				if((*iterOut)->GetSpilthAmount() < 0.00001)
				{
					iterOut = workOutputItemList1.erase(iterOut);
					continue;
				}

				//������С�ڿ������
				if(workOutItem->GetSpilthAmount() <= minStockCount)
				{
					minStockCount -= workOutItem->GetSpilthAmount();

					workOutItem->SetStockAmount(workOutItem->GetSpilthAmount());
					workOutItem->SetSpilthAmount(0);

					iterOut = workOutputItemList1.erase(iterOut);
					continue;
				}
				else
				{
					//������棨��������
					workOutItem->SetStockAmount(minStockCount);
					workOutItem->SetSpilthAmount(workOutItem->GetSpilthAmount()-minStockCount);

					minStockCount = 0;
					break;
				}
			}

			if(minStockCount > 0.000001)
			{
				YKOrderPtr orderPtr = YKOrder::CreatNew(minStockCount,itemPtr->GetId());
				if(orderPtr != NULL)
				{
					// ������Ϣ����order_transfor add 2010-1-28
					//OrderInfoTransfor(nextOrder,orderPtr);
					orderPtr->UpdateIOItem();
					YKWorkOutputItemPtr workOutputItem = YKWorkOutputItem::FindOrderOutItem(orderPtr);
					if(workOutputItem != NULL)
					{
						workOutputItem->SetStockAmount(minStockCount);
						workOutputItem->SetSpilthAmount(0);
						minStockCount = 0;
					}
					InforTransfor Trans;
					// ������Ϣ���� add 2010-2-1 by ll
					Trans.WorkInforTransfor(orderPtr);

					orderPtr->CreateOrderCode();
				}	
			}
		

		}

		//ɾ������Ĳ�����Ʒ
		DelSpilthOut(workOutputItemList2);
}

void	ApsCal::DelSpilthOut(list<YKWorkOutputItemPtr>& workOutputItemList)
{
	for(list<YKWorkOutputItemPtr>::iterator iter = workOutputItemList.begin();
		iter != workOutputItemList.end();iter++)
	{

		YKWorkOutputItemPtr&outPtr=*iter;
		if (outPtr->GetStockAmount()>PRECISION)
			continue; 

		YKOrderPtr order = outPtr->GetOrder();
		if(order == NULL)	continue;
		if (order->GetType() == OrderType_MTOrder ) //�ɹ�����
			continue;
		if (order->DelCondition())
		{
			if (order->GetType() == OrderType_StockOrder ) //�ɹ�����
			{
				AddDeAutoStockOrderList(order->GetId());
			}
			else if (order->GetType() == OrderType_PudOrder )  //���춨��
			{
				AddDeAutoPudOrderList(order->GetId());
			}
			order->Delete();
		}
	}
}

//����������Ʒ��
void ApsCal::CreatWorkInputList( YKItemPtr& itemPtr,list<YKWorkInputItemPtr>& workInputItemList ,list<YKWorkInputItemPtr>& orgList)
{

//	YKWorkInputItemPtrMap* inputPtrMap = g_pBizModel->GetYKWorkInputItemPtrMap();
//	for(inputPtrMap->Begin();inputPtrMap->NotEnd();inputPtrMap->Step())
	for(list<YKWorkInputItemPtr>::iterator iter = orgList.begin();
		iter != orgList.end();iter++)
	{
		//YKWorkInputItemPtr& workInputItemPtr = inputPtrMap->Get();
		YKWorkInputItemPtr& workInputItemPtr = *iter;
		if(workInputItemPtr != NULL
			&& workInputItemPtr->GetInputItemId() == itemPtr->GetId())
		{
			YKWorkPtr workPtr = workInputItemPtr->GetWork();
			if(workPtr != NULL)
			{
				if( workPtr->GetDivisionType() != 1 )
				{
					continue;
				}

				if(workInputItemPtr->GetScarcityAmount() > PRECISION)
				{
					workInputItemList.push_back(workInputItemPtr);
				}
			}
		}

	}

	//����
	SortWorkInputList(workInputItemList);
}

//�Թ���������Ʒ����
void ApsCal::SortWorkInputList( list<YKWorkInputItemPtr>& workInputItemList )
{
	//����Ϊ�գ��˳�
	if (workInputItemList.empty())
	{
		return;
	}

	list<YKWorkInputItemPtr> tempWorkInputItemList;

	//���չ���ʵ�ʿ�������
	list<YKWorkInputItemPtr>::iterator iter = workInputItemList.begin();
	while(iter!=workInputItemList.end())
	{
		YKWorkPtr work = (*iter)->GetWork();
		if(work == NULL)	break;;

		if( work->GetPlantStartTm() > 0)
		{
			list<YKWorkInputItemPtr>::iterator iterPS = tempWorkInputItemList.begin();
			for(;iterPS != tempWorkInputItemList.end();)
			{
				YKWorkPtr workPS = (*iterPS)->GetWork();
				if(workPS == NULL)	break;;

				if(work->GetPlantStartTm() < workPS->GetPlantStartTm())
				{
					tempWorkInputItemList.insert(iterPS,*iter);
					break;
				}
				iterPS++;
			}
			if(iterPS == tempWorkInputItemList.end())
				tempWorkInputItemList.push_back(*iter);

			iter = workInputItemList.erase(iter);
			continue;
		}
		iter++;
	}

	//���ն����ƻ�����ʱ������
	iter = workInputItemList.begin();
	while(iter!=workInputItemList.end())
	{
		YKOrderPtr order = (*iter)->GetOrder();
		if(order == NULL)	break;

		if(  order->GetPlanStartTime() > 0)
		{
			list<YKWorkInputItemPtr>::iterator iterOS = tempWorkInputItemList.begin();
			for(;iterOS != tempWorkInputItemList.end();)
			{
				YKOrderPtr orderOS = (*iterOS)->GetOrder(); 
				if(orderOS == NULL)	break;

				if(orderOS->GetPlanStartTime() > order->GetPlanStartTime())
				{
					tempWorkInputItemList.insert(iterOS,*iter);
					break;
				}
				iterOS++;
			}
			if(iterOS == tempWorkInputItemList.end())
				tempWorkInputItemList.push_back(*iter);

			iter = workInputItemList.erase(iter);
			continue;
		}

		iter++;
	}

	//���ն��㶨���ƻ�����ʱ������
	iter = workInputItemList.begin();
	while(iter!=workInputItemList.end())
	{
		YKOrderPtr order = (*iter)->GetOrder();
		if(order == NULL)	break;

		{
			list<YKWorkInputItemPtr>::iterator iterTC = tempWorkInputItemList.begin();
			for(;iterTC != tempWorkInputItemList.end();)
			{
				YKOrderPtr orderTC = (*iterTC)->GetOrder(); 
				if(orderTC == NULL)	break;

				YKOrderPtr& tcPeakOrderPtr = orderTC->GetPeakOrder();
				if (tcPeakOrderPtr == NULL) tcPeakOrderPtr = orderTC;
				YKOrderPtr& peakOrderPtr = order->GetPeakOrder();
				if (peakOrderPtr == NULL) peakOrderPtr = order;

				if(tcPeakOrderPtr->GetLastCompleteTime()
					> peakOrderPtr->GetLastCompleteTime())
				{
					tempWorkInputItemList.insert(iterTC,*iter);
					//iter = workInputItemList.erase(iter);
					break;
				}				
				else if(tcPeakOrderPtr->GetLastCompleteTime()
					== peakOrderPtr->GetLastCompleteTime())
				{
					if(order->GetId() < orderTC->GetId() )
					{
						tempWorkInputItemList.insert(iterTC,*iter);
						break;
					}
				}
				iterTC++;
			}
			if(iterTC==tempWorkInputItemList.end())
				tempWorkInputItemList.push_back(*iter);

			iter = workInputItemList.erase(iter);
			continue;
		}

		iter++;
	}

	//����������
	workInputItemList = tempWorkInputItemList;
}



void ApsCal::CreatWorkOutputList( YKItemPtr& itemPtr, 
								 list<YKWorkOutputItemPtr>& workOutputItemList1, 
								 list<YKWorkOutputItemPtr>& workOutputItemList2,
								 list<YKWorkOutputItemPtr>& workOutputItemListOrg)
{
//	YKWorkOutputItemPtrMap* outputPtrMap = g_pBizModel->GetYKWorkOutputItemPtrMap();
//	for(outputPtrMap->Begin();outputPtrMap->NotEnd();outputPtrMap->Step())
	for(list<YKWorkOutputItemPtr>::iterator iter = workOutputItemListOrg.begin();
		iter != workOutputItemListOrg.end();iter++)
	{
		//YKWorkOutputItemPtr& workOutputItemPtr = outputPtrMap->Get();
		YKWorkOutputItemPtr& workOutputItemPtr = *iter;

		if(!BZERO(workOutputItemPtr->GetStockAmount()))
		{
			workOutputItemPtr->SetSpilthAmount(workOutputItemPtr->GetShowSpilthAmount());
			workOutputItemPtr->SetStockAmount(0);
		}

		if( workOutputItemPtr != NULL
			&& workOutputItemPtr->GetOutputItemId() == itemPtr->GetId() 
			&& workOutputItemPtr->GetSpilthAmount() > PRECISION)
		{
			YKWorkPtr workPtr = workOutputItemPtr->GetWork();

			//��Ӧ�����ǲ��ǰ�������˳�
			if(workPtr == NULL) continue;
				
			if(workPtr->GetDivisionType() != DivType_NoDivWork )
			{
				continue;
			}

			{
				//����1
				YKOrderPtr orderPtr = workOutputItemPtr->GetOrder();
				if(orderPtr != NULL)
				{
					//�����Ƿ���
					if(orderPtr->GetNotDispatch())
						continue;

					//������֮ǰ�����̱�����Ϊ��Ч
					if(!orderPtr->GetValid())
						continue;

					if(orderPtr->GetDiffer() == OrderDiffer_Hand		//¼�붨��
						|| orderPtr->GetSchState() == OrderSchState_Start
						|| orderPtr->GetSchState() == OrderSchState_Done) //��ʼ�������
					{
						if((itemPtr->GetStockMode() == StockMode_Buy //���Ϸ�ʽ�ɹ�
							&& (orderPtr->GetType() == OrderType_StockOrder
							||orderPtr->GetType() == OrderType_RelOrder
							||orderPtr->GetType() == OrderType_AbsOrder)) //�ɹ�����
							||(itemPtr->GetStockMode() == StockMode_Inside //���Ϸ�ʽ����
							&& (orderPtr->GetType() == OrderType_PudOrder
							||orderPtr->GetType() == OrderType_RelOrder
							||orderPtr->GetType() == OrderType_AbsOrder))	//���춨��
							||(itemPtr->GetStockMode()!= StockMode_Buy
							&& itemPtr->GetStockMode()!= StockMode_Inside))
						{
							workOutputItemList1.push_back(workOutputItemPtr);
							continue;
						}
					}
					
					if((itemPtr->GetStockMode() == StockMode_BuyPre || itemPtr->GetStockMode() == StockMode_Buy)//���Ϸ�ʽ�ɹ�����
						&& orderPtr->GetType() == OrderType_StockOrder) //�ɹ�����
					{
						workOutputItemList2.push_back(workOutputItemPtr);
					}

					if((itemPtr->GetStockMode() == StockMode_InsidePre || itemPtr->GetStockMode() == StockMode_Inside) //���Ϸ�ʽ��������
						&& orderPtr->GetType() == OrderType_PudOrder)	//���춨��
					{
						workOutputItemList2.push_back(workOutputItemPtr);
					}
				}	
			}

		}
	}

	//����
	SortWorkOutputList(workOutputItemList2);
	SortWorkOutputList(workOutputItemList1);
	//workOutputItemList1.sort(SortWorkOutputList);
	//workOutputItemList2.sort(SortWorkOutputList);
}

bool ApsCal::SortWorkOutputList( YKWorkOutputItemPtr& first,YKWorkOutputItemPtr& second )
{

	return first->GetId() < second->GetId();
}
//�Թ��������Ʒ����
void ApsCal::SortWorkOutputList( list<YKWorkOutputItemPtr>& workOutputItemList )
{
	//����Ϊ�գ��˳�
	if (workOutputItemList.empty())
	{
		return;
	}

	list<YKWorkOutputItemPtr> tempWorkOutputItemList;

	//���չ���ʵ�ʿ�������
	list<YKWorkOutputItemPtr>::iterator iter = workOutputItemList.begin();
	while(iter!=workOutputItemList.end())
	{
		YKWorkPtr work = (*iter)->GetWork();
		if(work == NULL)	break;;

		if( work->GetPlantStartTm() > 0)
		{
			list<YKWorkOutputItemPtr>::iterator iterPS = tempWorkOutputItemList.begin();
			for(;iterPS != tempWorkOutputItemList.end();)
			{
				YKWorkPtr workPS = (*iterPS)->GetWork();
				if(workPS == NULL)	break;;

				if(work->GetPlantStartTm() < workPS->GetPlantStartTm())
				{
					tempWorkOutputItemList.insert(iterPS,*iter);
					break;
				}
				iterPS++;
			}
			if(iterPS == tempWorkOutputItemList.end())
				tempWorkOutputItemList.push_back(*iter);

			iter = workOutputItemList.erase(iter);
			continue;
		}
		iter++;
	}

	//���ն����ƻ�����ʱ������
	iter = workOutputItemList.begin();
	while(iter!=workOutputItemList.end())
	{
		YKOrderPtr order = (*iter)->GetOrder();
		if(order == NULL)	break;

		if(  order->GetPlanStartTime() > 0)
		{
			list<YKWorkOutputItemPtr>::iterator iterOS = tempWorkOutputItemList.begin();
			for(;iterOS != tempWorkOutputItemList.end();)
			{
				YKOrderPtr orderOS = (*iterOS)->GetOrder(); 
				if(orderOS == NULL)	break;

				if(orderOS->GetPlanStartTime() > order->GetPlanStartTime())
				{
					tempWorkOutputItemList.insert(iterOS,*iter);
					break;
				}
				iterOS++;
			}
			if(iterOS == tempWorkOutputItemList.end())
				tempWorkOutputItemList.push_back(*iter);

			iter = workOutputItemList.erase(iter);
			continue;
		}

		iter++;
	}

	//���ն��㶨���ƻ�����ʱ������
	iter = workOutputItemList.begin();
	while(iter!=workOutputItemList.end())
	{
		YKOrderPtr order = (*iter)->GetOrder();
		if(order == NULL)	break;

		{
			list<YKWorkOutputItemPtr>::iterator iterTC = tempWorkOutputItemList.begin();
			for(;iterTC != tempWorkOutputItemList.end();)
			{
				YKOrderPtr orderTC = (*iterTC)->GetOrder(); 
				if(orderTC == NULL)	break;

				YKOrderPtr& tcPeakOrderPtr = orderTC->GetPeakOrder();
				if (tcPeakOrderPtr == NULL) tcPeakOrderPtr = orderTC;
				YKOrderPtr& peakOrderPtr = order->GetPeakOrder();
				if (peakOrderPtr == NULL) peakOrderPtr = order;

				if(tcPeakOrderPtr->GetLastCompleteTime()
					> peakOrderPtr->GetLastCompleteTime())
				{
					tempWorkOutputItemList.insert(iterTC,*iter);
					//iter = workInputItemList.erase(iter);
					break;
				}				
				else if(tcPeakOrderPtr->GetLastCompleteTime()
					== peakOrderPtr->GetLastCompleteTime())
				{
					if(order->GetId() < orderTC->GetId() )
					{
						tempWorkOutputItemList.insert(iterTC,*iter);
						break;
					}
				}
				iterTC++;
			}
			if(iterTC==tempWorkOutputItemList.end())
				tempWorkOutputItemList.push_back(*iter);

			iter = workOutputItemList.erase(iter);
			continue;
		}

		iter++;
	}

	//����������
	workOutputItemList = tempWorkOutputItemList;
}

//���ɲ��䶨�����󼰹���
void ApsCal::CreateSingleDocument( YKWorkInputItemPtr& inputItem, 
								  list<YKWorkOutputItemPtr>& workOutputItemList, 
								  YK_INT type )
{
	//��������
	YK_FLOAT count = inputItem->GetScarcityAmount();
	if(BZERO(count))
		return;

	UINT itemId = inputItem->GetInputItemId();

	//ȡ��Ʒ
	YKItemPtr itemPtr = g_pBizModel->GetYKItemPtrMap()->Get(itemId);
	if(itemPtr == NULL)
		return;

	vector<YK_FLOAT> counts;

	//ȡ�󶩵�
	YKOrderPtr nextOrder = inputItem->GetOrder();
	if(nextOrder == NULL)	return;

	if(nextOrder->GetDesignation() == L""
		&& itemPtr->CalBatchManage(count,counts) ) //�������������
	{
		for(vector<YK_FLOAT>::iterator countIt = counts.begin();
			countIt != counts.end();countIt++)
		{
			//���ɶ���
			YKOrderPtr orderPtr;

			YK_ULONG orderId = 0;
			YKWorkOutputItemPtr workOutputItem;

			if(type == 0)
			{
				orderPtr = YKOrder::CreatNew(/*count*/*countIt,itemId);
				if(orderPtr == NULL)	return;

				orderId = orderPtr->GetId();

				//if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4)
				//{
				//	orderPtr->SetType(OrderType_StockOrder);
				//}
				orderPtr->UpdateIOItem();
				workOutputItem = YKWorkOutputItem::FindOrderOutItem(orderPtr);
			}
			else
			{
				if(!workOutputItemList.empty())					
				{
					orderPtr = g_pBizModel->GetYKOrderPtrMap()->Get(workOutputItemList.front()->GetOrderId());
					if( orderPtr != NULL)
					{
						orderId = orderPtr->GetId();
						//orderPtr->SetTopOrder(nextOrder);
						//YKOrderPtr peakOrder =  nextOrder->GetPeakOrder();
						//orderPtr->SetPeakOrder(peakOrder);
						workOutputItem = workOutputItemList.front();
					}
				}
			}

			//���������Ʒ��
	//		YKWorkOutputItemPtr workOutputItem=YKWorkOutputItem::FindOrderOutItem(orderPtr);
			if (workOutputItem == NULL) 
			{
				return ;
			}

			UINT workOutputItemId = workOutputItem->GetId();

			YK_FLOAT relationCount = min(inputItem->GetScarcityAmount(),workOutputItem->GetSpilthAmount());

			//����������
			YKWorkRelationPtr relationPtr=YKWorkRelation::CreatNew(inputItem,workOutputItem,2,relationCount);

			if(workOutputItem->GetSpilthAmount() > 0.000001 && type == 0)
				workOutputItemList.push_back(workOutputItem);

			InforTransfor Trans;
			if(type == 0)
			{
				orderPtr->CreateOrderCode();
				// ������Ϣ����order_transfor add 2010-1-28
				Trans.OrderInfoTransfor(nextOrder,orderPtr);
				// ������Ϣ���� add 2010-2-1 by ll
				Trans.WorkInforTransfor(orderPtr);
			}
			else
			{
				// �ж���ϲ㶩������ add 2010-2-3 by ll
				YKOrderPtr nextOrder = relationPtr->GetNextOrderPtr();
				YKOrderPtr preOrder = relationPtr->GetPreOrderPtr();
				if(NULL != nextOrder&&NULL != preOrder)
				{
					list<YKOrderPtr> orderList=preOrder->GetNexOrderList();
					YKOrderPtr optr=NULL;
					for (list<YKOrderPtr>::iterator oitor=orderList.begin();
						oitor!=orderList.end();oitor++)
					{
						if (NULL != optr)
						{
							// ��������ʱ������Ķ���
							if(optr->GetLastCompleteTime()>(*oitor)->GetLastCompleteTime())
								optr=*oitor;
						}
						else optr=*oitor;
					}
					// ����û�д���
					if(NULL!=optr&&(optr->GetId()==nextOrder->GetId()))
					{
						YKWorkOutputItemPtr workOutPtr=relationPtr->GetWorkOutputItemPtr();
						if(NULL != workOutPtr&&preOrder->GetDiffer()==OrderDiffer_Auto
							&&workOutPtr->GetIsMainProduction())	// �Զ����䶩��  ���������Ʒʱ������
						{
							Trans.OrderInfoTransfor(nextOrder,preOrder);
							//�Ƿ���ڹ�����״̬��ָʾ�������
							list<YKWorkPtr> workList;
							preOrder->GetWorkPtrList(workList);
							YK_BOOL b_=false;
							for (list<YKWorkPtr>::iterator itor=workList.begin();
								itor!=workList.end();itor++)
							{
								YKWorkPtr& workPtr=*itor;
								// ����״̬��ָʾ�������
								if(NULL != workPtr&&workPtr->GetState()>WorkState_IndicationDone)
								{	b_=true; break;}
							}
							if (!b_)	// �����ڹ���״̬��ָʾ�������
							{
								// ǰ�����Ƿ��л���Ʒ����·��
								if(false)
								{
									// ��Ʒ����·�߸ı� ����չ��  ������Ϣ����
								}
								else Trans.WorkInforTransfor(preOrder);	// ������Ϣ����
							}
							else Trans.WorkInforTransfor(preOrder);	// ������Ϣ����
						}
					}
					else { } //�����Ѿ�����
				}
			}
		}

	}
	else
	{
		//���ɶ���
		YKOrderPtr orderPtr;

		YK_ULONG orderId = 0;
		YKWorkOutputItemPtr workOutputItem;

		if(counts.empty())
			counts.push_back(count);

		if(type == 0)
		{
			orderPtr = YKOrder::CreatNew(counts[0],itemId);
			if(orderPtr == NULL) return;
			orderId = orderPtr->GetId();

			//if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4)
			//{
			//	orderPtr->SetType(OrderType_StockOrder);
			//}
			orderPtr->UpdateIOItem();
			workOutputItem = YKWorkOutputItem::FindOrderOutItem(orderPtr);
		}
		else
		{
			if(!workOutputItemList.empty())					
			{
				orderPtr = g_pBizModel->GetYKOrderPtrMap()->Get(workOutputItemList.front()->GetOrderId());
				if( orderPtr != NULL)
				{
					orderId = orderPtr->GetId();
					//orderPtr->SetTopOrder(nextOrder);
					//YKOrderPtr peakOrder =  nextOrder->GetPeakOrder();
					//orderPtr->SetPeakOrder(peakOrder);
					workOutputItem = workOutputItemList.front();
				}
			}
		}
		

		//���������Ʒ��

		if (workOutputItem == NULL) 
		{
			return ;
		}

		UINT workOutputItemId = workOutputItem->GetId();

		YK_FLOAT relationCount = min(inputItem->GetScarcityAmount(),workOutputItem->GetSpilthAmount());

		//����������
		YKWorkRelationPtr relationPtr = YKWorkRelation::CreatNew(inputItem,workOutputItem,2,relationCount);

		if(workOutputItem->GetSpilthAmount() > 0.000001 && type == 0)
			workOutputItemList.push_back(workOutputItem);

		InforTransfor Trans;
		if(type == 0)
		{
			orderPtr->CreateOrderCode();
			// ������Ϣ����order_transfor add 2010-1-28
			Trans.OrderInfoTransfor(nextOrder,orderPtr);
			// ������Ϣ���� add 2010-2-1 by ll
			Trans.WorkInforTransfor(orderPtr);
		}
		else
		{
			// �ж���ϲ㶩������ add 2010-2-3 by ll
			YKOrderPtr nextOrder = relationPtr->GetNextOrderPtr();
			YKOrderPtr preOrder = relationPtr->GetPreOrderPtr();
			if(NULL != nextOrder&&NULL != preOrder)
			{
				list<YKOrderPtr> orderList=preOrder->GetNexOrderList();
				YKOrderPtr optr=NULL;
				for (list<YKOrderPtr>::iterator oitor=orderList.begin();
					oitor!=orderList.end();oitor++)
				{
					if (NULL != optr)
					{
						// ��������ʱ������Ķ���
						if(optr->GetLastCompleteTime()>(*oitor)->GetLastCompleteTime())
							optr=*oitor;
					}
					else optr=*oitor;
				}
				// ����û�д���
				if(NULL!=optr&&(optr->GetId()==nextOrder->GetId()))
				{
					YKWorkOutputItemPtr workOutPtr=relationPtr->GetWorkOutputItemPtr();
					if(NULL != workOutPtr&&preOrder->GetDiffer()==OrderDiffer_Auto
						&&workOutPtr->GetIsMainProduction())	// �Զ����䶩��  ���������Ʒʱ������
					{
						Trans.OrderInfoTransfor(nextOrder,preOrder);
						//�Ƿ���ڹ�����״̬��ָʾ�������
						list<YKWorkPtr> workList;
						preOrder->GetWorkPtrList(workList);
						YK_BOOL b_=false;
						for (list<YKWorkPtr>::iterator itor=workList.begin();
							itor!=workList.end();itor++)
						{
							YKWorkPtr& workPtr=*itor;
							// ����״̬��ָʾ�������
							if(NULL != workPtr&&workPtr->GetState()>WorkState_IndicationDone)
							{	b_=true; break;}
						}
						if (!b_)	// �����ڹ���״̬��ָʾ�������
						{
							// ǰ�����Ƿ��л���Ʒ����·��
							if(false)
							{
								// ��Ʒ����·�߸ı� ����չ��  ������Ϣ����
							}
							else Trans.WorkInforTransfor(preOrder);	// ������Ϣ����
						}
						else Trans.WorkInforTransfor(preOrder);	// ������Ϣ����
					}
				}
				else { } //�����Ѿ�����
			}
		}

//		inputItem->SetScarcityAmount(inputItem->GetScarcityAmount() - relationCount);
	}
}

void ApsCal::Input2Output( YKWorkInputItemPtr& workInputItem
						  ,YKWorkOutputItemPtr& workOutputItem
						  ,YK_FLOAT usedQuantity )
{
	//�������������¿������������
	YK_FLOAT relationCount = 0;

	//����
	if(BZERO(workInputItem->GetScarcityAmount()-usedQuantity))//��������������������
	{
		relationCount = workInputItem->GetScarcityAmount();
//		workInputItem->SetScarcityAmount(0);
//		workOutputItem->SetSpilthAmount(workOutputItem->GetSpilthAmount()-usedQuantity);
	}
	else if(workInputItem->GetScarcityAmount() > usedQuantity) //������ڹ�Ӧ
	{
		//��Ӧ����
		relationCount = usedQuantity;
		//workInputItem->SetScarcityAmount(workInputItem->GetScarcityAmount() 
		//	- usedQuantity);
		//��������Ϊ0
//		workOutputItem->SetSpilthAmount(workOutputItem->GetSpilthAmount()-usedQuantity);
	}
	else //����С�ڹ�Ӧ
	{
		//��������
		relationCount = workInputItem->GetScarcityAmount();
//		workInputItem->SetScarcityAmount(0);
		//��Ӧ����
//		workOutputItem->SetSpilthAmount(workOutputItem->GetSpilthAmount() - workInputItem->GetScarcityAmount());
	}
	//�����ֶ�
    m_unused=m_unused-relationCount;
	YKWorkRelation::CreatNew(workInputItem ,workOutputItem,RelationType_Differ,relationCount);

}


void ApsCal::CreateSpecialOrder(YKWorkInputItemPtr& inputItem)
{
	//��������
	YK_FLOAT count = inputItem->GetScarcityAmount();
	if(BZERO(count))
		return;

	UINT itemId = inputItem->GetInputItemId();

	//ȡ��Ʒ
	YKItemPtr itemPtr = g_pBizModel->GetYKItemPtrMap()->Get(itemId);
	if(itemPtr == NULL)
		return;


	//ȡ�󶩵�
	YKOrderPtr nextOrder = inputItem->GetOrder();
	if(nextOrder == NULL)	return;

	{
		//���ɶ���
		YKOrderPtr orderPtr;

		YK_ULONG orderId = 0;
		YKWorkOutputItemPtr workOutputItem;


		orderPtr = YKOrder::CreatNew(count,itemId);
		if(orderPtr == NULL)	return;

		orderId = orderPtr->GetId();

		// ������Ϣ����order_transfor add 2010-1-28
		InforTransfor Trans;
		Trans.OrderInfoTransfor(nextOrder,orderPtr);
		
		orderPtr->UpdateIOItem();

		//orderPtr->SetTopOrder(nextOrder);
		//YKOrderPtr peakOrder =  nextOrder->GetPeakOrder();
		//orderPtr->SetPeakOrder(peakOrder);

		workOutputItem = YKWorkOutputItem::FindOrderOutItem(orderPtr);


		//���������Ʒ��
		if (workOutputItem == NULL) return ;

		UINT workOutputItemId = workOutputItem->GetId();

		YK_FLOAT relationCount = min(inputItem->GetScarcityAmount(),workOutputItem->GetSpilthAmount());

		//����������
		YKWorkRelationPtr relationPtr = YKWorkRelation::CreatNew(inputItem,workOutputItem,2,relationCount,RelationFixFlg_UnFix);

		orderPtr->CreateOrderCode();

		// ������Ϣ���� add 2010-2-1 by ll
		Trans.WorkInforTransfor(orderPtr);
	}

}

void ApsCal::OrderTransfor()
{
	int remarkFlg = g_pBizModel->GetApsParam()->GetOrderRemarkTransferWay();
	int specFlg = g_pBizModel->GetApsParam()->GetSpecTransferWay();
	if (remarkFlg != 1 || specFlg != 1)
	{
		YKOrderPtrMap* pOrderMap = g_pBizModel->GetYKOrderPtrMap();
		for (pOrderMap->Begin();pOrderMap->NotEnd();pOrderMap->Step())
		{
			YKOrderPtr& orderPtr = pOrderMap->Get();
			if (orderPtr != NULL && orderPtr->GetNxtRelationList().empty())
			{
				TransforLoop(orderPtr,remarkFlg,specFlg);
			}
		}
	}
}

BOOL ApsCal::TransforLoop( YKOrderPtr& ptr,UINT reFlg,UINT spFlg )
{
	if (ptr != NULL)
	{
		list<YKWorkRelationPtr> lst = ptr->GetPreRelationList();
		list<YKWorkRelationPtr>::const_iterator i_lst = lst.begin();
		for (; i_lst != lst.end(); i_lst++)
		{
			YKWorkRelationPtr rela = *i_lst;
			if(rela->GetRelaTye() != 2)
				continue;
			YKOrderPtr orderPtr = rela->GetPreOrderPtr();
			if (orderPtr != NULL)
			{
				//�ж��Ƿ�Ϊ�Զ�����
				if(orderPtr->GetDiffer() == OrderDiffer_Auto)
				{
					//��ն���ԭ����񣬱�ע
					orderPtr->DeleteSpec();
					orderPtr->SetRemark(L"");
					// һ�����������������ϲ㶩��  ȡ���ֵ
					orderPtr->SetPriority(max(ptr->GetPriority(),orderPtr->GetPriority()));

					if (orderPtr->GetType() == OrderType_PudOrder)
					{
						if (reFlg == 2 || reFlg == 4)
							orderPtr->SetRemark(ptr->GetRemark());
						if (spFlg == 2 || spFlg == 4)
							orderPtr->SpecTransfor(ptr);
					}
					else if (orderPtr->GetType() == OrderType_StockOrder)
					{
						if (reFlg == 3|| reFlg == 4)
							orderPtr->SetRemark(ptr->GetRemark());
						if (spFlg == 3|| spFlg == 4)
							orderPtr->SpecTransfor(ptr);
					}
				}

				TransforLoop(orderPtr,reFlg,spFlg);
			}
		}
		return TRUE;
	}
	return FALSE;
}

void ApsCal::RemarkTransfor()
{
	YKOrderPtrMap* pOrderMap = g_pBizModel->GetYKOrderPtrMap();
	for (pOrderMap->Begin();pOrderMap->NotEnd();pOrderMap->Step())
	{
		YKOrderPtr& orderPtr = pOrderMap->Get();
		if (orderPtr != NULL)
		{
			list<YKWorkPtr> workList;
			orderPtr->GetWorkPtrList(workList);
			for (list<YKWorkPtr>::iterator i_work = workList.begin();
				i_work != workList.end(); i_work++)
			{
				YKWorkPtr& workPtr = *i_work;
				if (workPtr != NULL && !(workPtr->GetOutterChangeFlg() == ChangeFlg_Add 
					|| workPtr->GetOutterChangeFlg() == ChangeFlg_Change))
				{
					//��չ�����ע
					//add by lq 09-10-27
					//YK_WSTRING strTmp;
					workPtr->SetRemark(L"");
					//��������	
					YKOrderPtr orderPtr = workPtr->GetOrder();
					if (orderPtr != NULL&& !orderPtr->GetRemark().empty())
					{
						workPtr->SetRemark(orderPtr->GetRemark());
						continue;
					}
					//������Ʒ����
					//add by lq 09-10-27
					if(orderPtr != NULL)
					{
						YKItemPtr& orderItemPtr = g_pBizModel->GetYKItemPtrMap()->Get(orderPtr->GetPudId());
						if (orderItemPtr != NULL && !orderItemPtr->GetRemark().empty())
						{
							workPtr->SetRemark(orderItemPtr->GetRemark());
							continue;

						}
					}

					//��Ʒ����
					YKItemPtr& itemPtr = g_pBizModel->GetYKItemPtrMap()->Get(workPtr->GetItemId());
					if (itemPtr != NULL&& !itemPtr->GetRemark().empty()) 
					{
						workPtr->SetRemark(itemPtr->GetRemark());
						continue;
					
					}
					//���򴫵�
					YKOperationPtr operPtr = workPtr->GetOperation();
					if (operPtr != NULL && !operPtr->GetRemark().empty())
					{
						workPtr->SetRemark(operPtr->GetRemark());
						continue;
						
					}
				}
			}
		}
	}
}

//  �Զ�����������Ʒ���м���Ʒ
void ApsCal::AutoAddAllMiddleItem()
{
	if(NULL == g_pBizModel) return;
	YKItemPtrMap* itemPtrMap=g_pBizModel->GetYKItemPtrMap();
	for (itemPtrMap->Begin();itemPtrMap->NotEnd();itemPtrMap->Step())
	{
		YKItemPtr itemPtr=itemPtrMap->Get();
		AutoMiddleItemAdd(itemPtr);
	}
}
//  �Զ����䵥����Ʒ���м���Ʒ
void ApsCal::AutoMiddleItemAdd( YKItemPtr& ptr )
{
	if(NULL == ptr) return;
	// �м���Ʒ��ԭ�ϲ�����
	if(ItemType_MiddleProduct==ptr->GetType()||ItemType_Materials==ptr->GetType())  return;

    list<YKProductRoutingPtr>PdRPtrList=ptr->GetProductRoutingList();
	for (list<YKProductRoutingPtr>::iterator iter=PdRPtrList.begin();
		iter!=PdRPtrList.end();iter++)
	{
          YKProductRoutingPtr&productRoutingPtr=*iter;
		list<YKOperationPtr> operationPtrList=productRoutingPtr->GetOperPtrList();
		for (list<YKOperationPtr>::iterator itor=operationPtrList.begin();
			itor!=operationPtrList.end();itor++)
		{
			YKOperationPtr& operPtr=*itor;
			if(NULL == operPtr) continue;
			//	���乤������
			operPtr->ReturnNewFrontOperId(operPtr->GetPreOperByLayer());
			//  ���乤�����
			operPtr->ReturnNewBackOperId(operPtr->GetNexOperByLayer());

		}
	}
}
// �����Ѿ��еĹ���������� �������Ƴ����Ĺ����������
void ApsCal::BaseRelByOperation()
{
	YKOperInputItemPtrMap*  ykOperInputItemPtrMap = g_pBizModel->GetYKOperInputItemPtrMap();
	for(ykOperInputItemPtrMap->Begin();ykOperInputItemPtrMap->NotEnd();ykOperInputItemPtrMap->Step())
	{
		YKOperInputItemPtr& operInputItemPtr = ykOperInputItemPtrMap->Get();
		if(operInputItemPtr != NULL)
		{
			YKOperationPtr operationPtr = operInputItemPtr->GetOperation();
			YKOperationPtr operationPrePtr = operInputItemPtr->GetLogicPreOperPtr();   // operInputItemPtr->GetPreOperPtr();   modify 2011-3-4
			if(operationPtr	!= NULL&&operationPrePtr != NULL)
			{
				// 
				YK_ULONG itemId=0;
				// �Ҷ�Ӧ�������  
				YK_ULONG inItemId=0;
				if (operInputItemPtr->GetItem()!=NULL)
				{
					inItemId=operInputItemPtr->GetItem()->GetId();
					itemId=inItemId;  //��������ƷΪ��
				}
				// ���빤�� ���������Ӧ��Ʒ  �����Ƿ��ж�Ӧ��� itemId�б������������ͬ��Ʒ  Ϊ0����Ҫ�½���Ʒ�����м���Ʒ
				YK_BOOL b=operationPrePtr->CheckOperOut(operationPtr->GetId(),inItemId,itemId);
				if (itemId<=0)	// û����Ʒ
				{
					// �½���Ʒ
					YK_WSTRING code=L"";
					/*code+=operationPrePtr->GetCode();
					code+=L"_";
					code+=operationPtr->GetCode();*/
// 					YKItemPtrMap* itemMap=g_pBizModel->GetYKItemPtrMap();
// 					if (NULL != itemMap)
// 					{
// 						YKItemPtr& itemPtr=itemMap->Get(operationPtr->GetItemId2());
// 						if (NULL != itemPtr)
// 						{
// 							code+=itemPtr->GetCode().c_str();
// 						}
// 					}
// 					code+=L"_";
// 					code+=BIZMODELTOOLS::IToWString(operationPrePtr->GetProcessNumber(),10);
					

					YKItem ykitem; ykitem->NewTemp();
					ykitem->SetType(ItemType_MiddleProduct);
					ykitem->SetCalcType(3);  //����
					ykitem->SetStockMode(StockMode_No);  //����
					ykitem->SetAutoSupply(1);  //  ��

					// change 2010-11-4 by ll
					CodeBringRule codeBringRule;
					code = codeBringRule.GetCode(RULE_DEF_AUTOMIDDLEITEM,operationPrePtr->GetId(),CNewRuleCell::AddSerialNumber(RULE_DEF_AUTOMIDDLEITEM));

					map<YK_WSTRING,YK_ULONG>::iterator itemitor=g_pBizModel->m_ItemInfo.find(code);
					if (itemitor!=g_pBizModel->m_ItemInfo.end())
					{
						itemId = itemitor->second;
						ykitem->FreeTemp();
					}
					else
					{
						itemId = ykitem->Insert();//g_pBizModel->InsertBiz(ykitem);
						ykitem->SetCode(code);
						ykitem->SetItemName(code);
						g_pBizModel->m_ItemInfo.insert(pair<YK_WSTRING,YK_ULONG>(code,itemId));
					}
				}
				if (!b)
				{
					// �½��������
					YKOperOutputItem operItemOut; operItemOut.New();
					operItemOut.SetOperationId(operationPrePtr->GetId());
					operItemOut.SetNextOper(operationPtr->GetId());
					operItemOut.SetLogicNextOper(operationPtr->GetId());   //add 2011--3-4
					operItemOut.SetOutputItemId(itemId);
					YK_ULONG Id = operItemOut.GetId();
					YKOperOutputItemPtr opOtPtr=g_pBizModel->GetYKOperOutputItemPtrMap()->Get(Id);
					operationPrePtr->SetOperInfoBack(operationPtr->GetId(),itemId,Id);
					operationPrePtr->AddOperOutPtr(opOtPtr);

					YKBOM bom2; bom2->New();
					bom2->SetCmdType(BOM_Output);
					bom2->SetCmdCode(opOtPtr->GetCmdCode());
					bom2->SetBomPtrAll(operationPrePtr->GetLastItemId(),operationPrePtr->GetId(),operationPrePtr->GetPdRoutingId(),0);
					bom2->SetOperOutputItemPtr(opOtPtr);
					YK_ULONG bom2Id= bom2.GetId();//g_pBizModel->InsertBiz(bom2);
					bom2->ParapareOut();
					opOtPtr->SetBomId(bom2Id);
				}
				//if (operInputItemPtr->IfMidItem())	   //�м�Ʒʱ���������� 
				if (operInputItemPtr->GetItem() == NULL)	
					operInputItemPtr->SetInputItemId(itemId);
				operationPtr->SetOperInfoFront(operationPrePtr->GetId(),itemId,operInputItemPtr->GetId());
			}
			else if (operationPtr != NULL)
			{
				operationPtr->SetOperInfoFront(0,0,0);
			}
		}
	}

	//�������������Ʒ
	YKOperOutputItemPtrMap*  ykOperOutputItemPtrMap = g_pBizModel->GetYKOperOutputItemPtrMap();
	for(ykOperOutputItemPtrMap->Begin();ykOperOutputItemPtrMap->NotEnd();ykOperOutputItemPtrMap->Step())
	{
		YKOperOutputItemPtr& operOutputItemPtr = ykOperOutputItemPtrMap->Get();
		if(operOutputItemPtr != NULL)
		{
			YKOperationPtr operationPtr = operOutputItemPtr->GetOperation();
			YKOperationPtr operationNextPtr = operOutputItemPtr->GetLogicNextOperPtr();   //operOutputItemPtr->GetNextOperPtr(); mofify 2011-3-4
			if(operationPtr	!= NULL&&operationNextPtr != NULL)
			{
				if (operOutputItemPtr->IfReplace()/*!operOutputItemPtr->IfReItem()*/)  //����������ڱ���������ʱ����ʹ�ø�������й���
				{
					continue;
				}
				// 
				YK_ULONG itemId=0;
				// �Ҷ�Ӧ�������  
				YK_ULONG outItemId=0;
				if (operOutputItemPtr->GetItem()!=NULL)
				{
					outItemId=operOutputItemPtr->GetItem()->GetId();
					itemId=outItemId;    //���������Ʒ�������������Ʒ�������벻����
				}
				// ���빤�� ���������Ӧ��Ʒ  �����Ƿ��ж�Ӧ���� itemId�б������������ͬ��Ʒ  Ϊ0����Ҫ�½���Ʒ�����м���Ʒ
				YK_BOOL b=operationNextPtr->CheckOperIn(operationPtr->GetId(),outItemId,itemId);
				if (itemId<=0)	// û����Ʒ
				{
					// �½���Ʒ
					YK_WSTRING code=L"Auto_";
					/*code+=operationPtr->GetCode();
					code+=L"_";
					code+=operationNextPtr->GetCode();*/
// 					YKItemPtrMap* itemMap=g_pBizModel->GetYKItemPtrMap();
// 					if (NULL != itemMap)
// 					{
// 						YKItemPtr& itemPtr=itemMap->Get(operationPtr->GetItemId2());
// 						if (NULL != itemPtr)
// 						{
// 							code+=itemPtr->GetCode().c_str();
// 						}
// 					}
// 					code+=L"_";
/*					code+=BIZMODELTOOLS::IToWString(operationPtr->GetProcessNumber(),10);*/
					
					YKItem ykitem; ykitem.NewTemp();
					//ykitem->SetId(0);
					ykitem->SetType(ItemType_MiddleProduct);
					ykitem->SetCalcType(3);  //����
					ykitem->SetStockMode(StockMode_No);  //����
					ykitem->SetAutoSupply(1);  //  ��

					// change 2010-11-4 by ll
					CodeBringRule codeBringRule;
					code = codeBringRule.GetCode(RULE_DEF_AUTOMIDDLEITEM,operationNextPtr->GetId(),CNewRuleCell::AddSerialNumber(RULE_DEF_AUTOMIDDLEITEM));

					map<YK_WSTRING,YK_ULONG>::iterator itemitor=g_pBizModel->m_ItemInfo.find(code);
					if (itemitor!=g_pBizModel->m_ItemInfo.end())
					{
						itemId=itemitor->second;
						ykitem.FreeTemp();
					}
					else
					{
						ykitem->SetCode(code);
						ykitem->SetItemName(code);
						itemId = ykitem.Insert();//g_pBizModel->InsertBiz(ykitem);
						g_pBizModel->m_ItemInfo.insert(pair<YK_WSTRING,YK_ULONG>(code,itemId));
					}
				}
				if (!b)
				{
					// �½��������
					YKOperInputItem operItemIn; operItemIn.New();
					operItemIn.SetOperationId(operationNextPtr->GetId());
					operItemIn.SetPreOper(operationPtr->GetId());
					operItemIn.SetLogicPreOper(operationPtr->GetId());   //add 2011--3-4
					operItemIn.SetInputItemId(itemId);
					YK_ULONG Id = operItemIn.GetId();
					YKOperInputItemPtr opOtPtr=g_pBizModel->GetYKOperInputItemPtrMap()->Get(Id);
					operationNextPtr->SetOperInfoFront(operationPtr->GetId(),itemId,Id);
					operationNextPtr->AddOperInPtr(opOtPtr);

					YKBOM bom; bom->New();
					bom->SetCmdType(BOM_Input);
					bom->SetCmdCode(opOtPtr->GetCmdCode());
					bom->SetBomPtrAll(operationPtr->GetLastItemId(),operationNextPtr->GetId(),operationPtr->GetPdRoutingId(),0);
					bom->SetOperInputItemPtr(opOtPtr);
					YK_ULONG bomId=bom.GetId();//g_pBizModel->InsertBiz(bom);
					bom->ParapareOut();
					opOtPtr->SetBomId(bomId);
				}
				//if (operOutputItemPtr->IfMidItem())	   //�м�Ʒʱ����������       
				if (operOutputItemPtr->GetItem() == NULL)
					operOutputItemPtr->SetOutputItemId(itemId);

				operationPtr->SetOperInfoBack(operationNextPtr->GetId(),itemId,operOutputItemPtr->GetId());
			}
			else if (operationPtr != NULL)
			{
				operationPtr->SetOperInfoBack(0,0,0);
			}
		}
	}
}
//���򶩵�����
void ApsCal::Input2OutputByWillOrderAndDesignation(YKOrderPtr&peakOrder,YKWorkInputItemPtr& inputItem, list<YKWorkOutputItemPtr>& workOutputItemList1, 
												   list<YKWorkOutputItemPtr>& workOutputItemList2 )
{
	if(BZERO(inputItem->GetScarcityAmount()))
		return;

	YK_FLOAT relationCount ;   //��������
	list<YKWorkOutputItemPtr>::iterator iter_output1;
	iter_output1=workOutputItemList1.begin();
	for (;iter_output1!=workOutputItemList1.end();iter_output1++)
	{
         YKWorkOutputItemPtr&outputPtr=*iter_output1;
         YKOrderPtr orderOutPtr =outputPtr->GetOrder();        
          
		 if (orderOutPtr!=NULL)
		 {
			 if (orderOutPtr->GetType()==	OrderType_RelOrder	
				 ||	orderOutPtr->GetType() == OrderType_AbsOrder)
			 {
				 if (orderOutPtr->GetDesignation()==peakOrder->GetDesignation())//�ö������Ʒ����붥�㶩�����Ʒ�����ͬ
				 {
					 relationCount = min(inputItem->GetScarcityAmount(),outputPtr->GetSpilthAmount());

					 if (relationCount < 0.00001)
						 continue;
					 //����������
					 YKWorkRelation::CreatNew(inputItem,outputPtr,2,relationCount,RelationFixFlg_UnFix);
					 if(BZERO(inputItem->GetScarcityAmount()))
						 return;
				 }
			 }
		 }
	}

//	list<YKWorkOutputItemPtr>::iterator iter_output1;
	iter_output1=workOutputItemList1.begin();
	for (;iter_output1!=workOutputItemList1.end();iter_output1++)
	{
		YKWorkOutputItemPtr&outputPtr=*iter_output1;
		YKOrderPtr orderOutPtr =outputPtr->GetOrder();


		if (orderOutPtr!=NULL)
		{
			if (orderOutPtr->GetType()==	OrderType_RelOrder	
				||	orderOutPtr->GetType() == OrderType_AbsOrder)
			{
				if (orderOutPtr->GetDesignation().empty())//�ö������Ʒ����붥�㶩�����Ʒ�����ͬ
				{
					relationCount = min(inputItem->GetScarcityAmount(),outputPtr->GetSpilthAmount());

					if (relationCount < 0.00001)
						continue;
					//����������
					YKWorkRelation::CreatNew(inputItem,outputPtr,2,relationCount,RelationFixFlg_UnFix);
					if(BZERO(inputItem->GetScarcityAmount()))
						return;
				}
			}
		}
	}


		//if(BZERO(inputItem->GetScarcityAmount()))
		//	return;
		 //   list<YKWorkOutputItemPtr>::iterator iter_output2;
		 //   iter_output2=workOutputItemList2.begin();
			//for (;iter_output2!=workOutputItemList2.end();iter_output2++)
			//{
			//	YKWorkOutputItemPtr&outputPtr=*iter_output2;
			//	YKOrderPtr orderOutPtr =outputPtr->GetOrder();
			//	
			//	if (orderOutPtr!=NULL)
			//    {
			//		if (orderOutPtr->GetType()==OrderType_PudOrder
			//			&& orderOutPtr->GetDiffer() != 2)	//�ֶ�¼��
			//			continue;
			//	 if (orderOutPtr->GetDesignation()==peakOrder->GetDesignation())//�ö������Ʒ����붥�㶩�����Ʒ�����ͬ
			//	 {
			//		  relationCount = min(inputItem->GetScarcityAmount(),outputPtr->GetSpilthAmount());
			//		  if (relationCount < 0.0001)
			//			  continue;
			//		 //����������
			//		 YKWorkRelation::CreatNew(inputItem,outputPtr,2,relationCount,RelationFixFlg_UnFix);
			//		 if(BZERO(inputItem->GetScarcityAmount()))
			//			 return;

			//	 }
			//   } 
			//}
}


//���򶩵�����
void ApsCal::Input2OutputByWillOrder(YKOrderPtr&peakOrder,YKWorkInputItemPtr& inputItem, list<YKWorkOutputItemPtr>& workOutputItemList1, 
												   list<YKWorkOutputItemPtr>& workOutputItemList2 )
{
	if(BZERO(inputItem->GetScarcityAmount()))
		return;
	YK_FLOAT relationCount ;   //��������
	list<YKWorkOutputItemPtr>::iterator iter_output1;
	iter_output1=workOutputItemList1.begin();
	for (;iter_output1!=workOutputItemList1.end();iter_output1++)
	{
		YKWorkOutputItemPtr&outputPtr=*iter_output1;
		YKOrderPtr orderOutPtr =outputPtr->GetOrder();


		if (orderOutPtr!=NULL)
		{
			if (orderOutPtr->GetType()==OrderType_PudOrder
				&& orderOutPtr->GetDiffer() != 2)
				continue;

			if (orderOutPtr->GetDesignation()==peakOrder->GetDesignation())//�ö������Ʒ����붥�㶩�����Ʒ�����ͬ
			{
				relationCount = min(inputItem->GetScarcityAmount(),outputPtr->GetSpilthAmount());

				if (relationCount < 0.0001)
					continue;
				//����������
				YKWorkRelation::CreatNew(inputItem,outputPtr,2,relationCount,RelationFixFlg_UnFix);
				if(BZERO(inputItem->GetScarcityAmount()))
					return;

			}
		}
	}

	if(BZERO(inputItem->GetScarcityAmount()))
		return;
	list<YKWorkOutputItemPtr>::iterator iter_output2;
	iter_output2=workOutputItemList2.begin();
	for (;iter_output2!=workOutputItemList2.end();iter_output2++)
	{
		YKWorkOutputItemPtr&outputPtr=*iter_output2;
		YKOrderPtr orderOutPtr =outputPtr->GetOrder();

		if (orderOutPtr!=NULL)
		{
			if (orderOutPtr->GetType()==OrderType_PudOrder
				&& orderOutPtr->GetDiffer() != 2)	//�ֶ�¼��
				continue;
			if (orderOutPtr->GetDesignation()==peakOrder->GetDesignation())//�ö������Ʒ����붥�㶩�����Ʒ�����ͬ
			{
				relationCount = min(inputItem->GetScarcityAmount(),outputPtr->GetSpilthAmount());
				if (relationCount < 0.0001)
					continue;
				//����������
				YKWorkRelation::CreatNew(inputItem,outputPtr,2,relationCount,RelationFixFlg_UnFix);
				if(BZERO(inputItem->GetScarcityAmount()))
					return;

			}
		} 
	}
}


//�ɹ���������Ʒ��¼����Ʒ����չ������������
bool ApsCal::BatchManageByDemand(YKWorkInputItemPtr& inputItem)
{
	//��������
	YK_FLOAT count = inputItem->GetScarcityAmount();
	if(BZERO(count))
		return false;

	UINT itemId = inputItem->GetInputItemId();

	//ȡ��Ʒ
	YKItemPtr itemPtr = g_pBizModel->GetYKItemPtrMap()->Get(itemId);
	if(itemPtr == NULL)
		return false;

	vector<YK_FLOAT> counts;

	//ȡ�󶩵�
	YKOrderPtr nextOrder = inputItem->GetOrder();
	if(nextOrder == NULL)	return false;

	if( itemPtr->CalBatchManage(count,counts) ) //�������������
	{
		for(vector<YK_FLOAT>::iterator countIt = counts.begin();
			countIt != counts.end();countIt++)
		{
			//���ɶ���
			YKOrderPtr orderPtr;

			YK_ULONG orderId = 0;
			YKWorkOutputItemPtr workOutputItem;


			orderPtr = YKOrder::CreatNew(*countIt,itemId);
			if(orderPtr == NULL)	return false;

			orderId = orderPtr->GetId();

			orderPtr->UpdateIOItem();
	
			workOutputItem = YKWorkOutputItem::FindOrderOutItem(orderPtr);
			
			orderId = orderPtr->GetId();
			//orderPtr->SetTopOrder(nextOrder);
			//YKOrderPtr peakOrder =  nextOrder->GetPeakOrder();
			//if(peakOrder == NULL) peakOrder = nextOrder;
			//orderPtr->SetPeakOrder(peakOrder);			
			
			if (workOutputItem == NULL) break;

			UINT workOutputItemId = workOutputItem->GetId();

			YK_FLOAT relationCount = min(inputItem->GetScarcityAmount(),workOutputItem->GetSpilthAmount());

			//����������
			YKWorkRelationPtr relationPtr=YKWorkRelation::CreatNew(inputItem,workOutputItem,2,relationCount);
		
			orderPtr->CreateOrderCode();
		}
		return true;
	}
	return false;

}


////����������ϵ --������
void ApsCal::NewCreateRelation( list<YKItemPtr>& itemPtrList )
{
	ClearAmount();
	UpdateBackOrderRelation(itemPtrList);
	list<YKWorkInputItemPtr>  workInputItemListOrg;// = itemPtr->GetWorkInList();

	//���ɷ���������������Ʒ����
	list<YKWorkInputItemPtr>  workInputItemList;
//	list<YKWorkRelationPtr> reationList;   //��ÿ����Ʒ����Ӧ�Ĺ���
	for (list<YKItemPtr>::iterator iter=itemPtrList.begin();iter!=itemPtrList.end();iter++)
		CreatWorkInPutList(*iter,workInputItemList,workInputItemListOrg);
	
	workInputItemList.sort(SortWorkInputItemList);

	////���������
	//#ifdef _CLOCK_DEBUG
	//	char buf[200];
	//	buf[0]='\0';
	//	for (list<YKWorkInputItemPtr>::iterator iter=workInputItemList.begin();iter!=workInputItemList.end();iter++)
	//	{ 
	//		sprintf(buf+strlen(buf), "%d - > ",(*iter)->GetId());
	//		TimeLog::WriteLog(buf);
	//		buf[0]='\0';
	//	}
	//#endif

	//ѡ�������Ʒ
	list<YKWorkOutputItemPtr> workOutputItemListOrg;// = itemPtr->GetWorkOutList();

	list<YKWorkOutputItemPtr> workOutputItemList0;
	list<YKWorkOutputItemPtr> workOutputItemList1;
	list<YKWorkOutputItemPtr> workOutputItemList2;
	list<YKWorkOutputItemPtr> workOutputItemList21;
	list<YKWorkOutputItemPtr> workOutputItemList3;
	
	for (list<YKItemPtr>::iterator iter=itemPtrList.begin();iter!=itemPtrList.end();iter++)
		CreatWorkOutputList(*iter,workOutputItemList0,workOutputItemList1,workOutputItemList2,workOutputItemList21,workOutputItemList3,workOutputItemListOrg);
	workOutputItemList0.sort(SortWorkOutPutList);
	workOutputItemList1.sort(SortWorkOutPutList);
	workOutputItemList2.sort(SortWorkOutPutList);
	workOutputItemList21.sort(SortWorkOutPutList);

		//����������Ʒ�б�
	CreatReationByDesignation(/*itemPtr,*/workInputItemList,workOutputItemList0);  //�������Ʒ���֮��Ĺ���

	AllotStock(workInputItemList,workOutputItemList1,workOutputItemList2); //������

	ReationUpdate(workInputItemList,workOutputItemList2,workOutputItemList21);            //���¹�������
	
	//1��1����
	list<YKItemPtr>::iterator iter=itemPtrList.begin();
	for (;iter!=itemPtrList.end();iter++)
	{
		YKItemPtr&itemPtr=*iter;
		//list<YKWorkInputItemPtr>inList=GetWorkInList(itemPtr,workInputItemList);
		EquitySupply(itemPtr,workInputItemList);
	}
	//��1��1����
	/*list<YKItemPtr>::iterator*/ iter=itemPtrList.begin();
	for (;iter!=itemPtrList.end();iter++)
	{
		YKItemPtr&itemPtr=*iter;
		if(itemPtr->GetAutoSupply() != AutoFlag_No) //���Զ�����,�˳�
		{
			list<YKWorkInputItemPtr>inList=GetWorkInList(itemPtr,workInputItemList);
			list<YKWorkOutputItemPtr>outList1=GetWorkOutList(itemPtr,workOutputItemList2);
			list<YKWorkOutputItemPtr>outList2=GetWorkOutList(itemPtr,workOutputItemList21);
			//����
			list<YKWorkOutputItemPtr> tempOutList1 = outList1;
			list<YKWorkOutputItemPtr> tempOutList2 = outList2;
			NoEquitySupply( itemPtr,inList,outList1,outList2);
			//��ƥ��õĲ���ɾ��
			RemoveList(workOutputItemList2,RemoveList(tempOutList1,outList1));
			RemoveList(workOutputItemList21,RemoveList(tempOutList2,outList2));
		}
	}

	//ɾ������Ĳ�����Ʒ
	DelSpilthOut(workOutputItemList1);     //����ȥ���������ֵ���
	SubtractStockAmout( workOutputItemList21);          //��ȥ������������
	DelSpilthOut(workOutputItemList0);
	DelSpilthOut(workOutputItemList2);
	DelSpilthOut(workOutputItemList3);
}
//����������Ʒ��
void ApsCal::CreatWorkInPutList( YKItemPtr& itemPtr,
								list<YKWorkInputItemPtr>& workInputItemList ,
								list<YKWorkInputItemPtr>& orgList)
{
	const set<YK_ULONG>& workInSet = itemPtr->GetWorkInSet();
	bool ifCyc=false;
	set<YK_ULONG>mainInSet;
	for(set<YK_ULONG>::const_iterator iter = workInSet.begin();
		iter != workInSet.end();iter++)
	{
		YKWorkInputItem workInputItemPtr = g_pBizModel->GetYKWorkInputItemPtrMap()->Get(*iter);
		if(workInputItemPtr != NULL
			&& workInputItemPtr->GetInputItemId() == itemPtr->GetId())
		{
			YKOrderPtr orderPtr = workInputItemPtr->GetOrder();
			if (NULL==orderPtr)  continue;
			////ά���Ķ���
			//if (orderPtr->GetType() == OrderType_MTOrder)
			//	continue;

			if(orderPtr->GetExcptFlg()!=1) continue; //��������ȷչ��

			//�Ƿ��ɱ�־
			if(orderPtr->GetNotDispatch() == 1) continue;

			if (!workInputItemPtr->GetActualWorkRecursionFlg()||!workInputItemPtr->IfReationForResultWork())
			{
				continue;
			}
			if (orderPtr->GetType()==OrderType_PudOrder&&orderPtr->GetPudId()==itemPtr->GetId())   //������ƷΪ����
				continue;
		
			CNewRuleManager rulManager;
			rulManager.SetInfo(RULE_DEF_SCH_FILTER_Item,g_pBizModel->GetApsParam()->GetItemFiterRule(),TblItem);
			if(!rulManager.GetBoolValue(workInputItemPtr->GetInputItemId()))  //��Ʒɸѡ
				continue;

			rulManager.SetInfo(RULE_DEF_FILTER_Order,g_pBizModel->GetApsParam()->GetOrderFiter(),TblOrder);
			if(!rulManager.GetBoolValue(workInputItemPtr->GetOrderId()))  //����ɸѡ
				continue;

			ifCyc=false;
			/*YK_ULONG orderId=orderPtr->GetId();
			orderPtr->IfHaveReationCyc(orderId,ifCyc);
			if (ifCyc)
				continue;*/

			YKWorkPtr workPtr = workInputItemPtr->GetWork();
			if(workPtr != NULL)
			{
				if( workPtr->GetDivisionType() != 1 )
				{
					continue;
				}
				list<YKWorkRelationPtr>inReationList;
				workInputItemPtr->GetRelationPtrList(inReationList);  //�������й���
				//reationList.insert(reationList.end(),inReationList.begin(),inReationList.end()); //����������й���������,������������

				if (!workInputItemPtr->GetMainMaterialFlg())  //
				{
					if (workInputItemPtr->IfDemandAmount())  //������������
					{
						workInputItemList.push_back(workInputItemPtr);
					}
					//mainInSet.insert(workInputItemPtr->GetId());
				}
				else
				{
					if(workInputItemPtr->GetScarcityAmount() > PRECISION)
					{
						workInputItemList.push_back(workInputItemPtr);
						/*list<YKWorkInputItemPtr>inList=workPtr->GetCmdCodeWorkInList(workInputItemPtr->GetCmdCode());
						if (inList.empty())
						{
							if (mainInSet.find(workInputItemPtr->GetId())==mainInSet.end())
							{
								workInputItemList.push_back(workInputItemPtr);
								mainInSet.insert(workInputItemPtr->GetId());
							}
						}
						else
						{
							list<YKWorkInputItemPtr>::iterator iIter=inList.begin();
							for (;iIter!=inList.end();iIter++)
							{ 
								YKWorkInputItemPtr&inPtr=*iIter;
								if (mainInSet.find(inPtr->GetId())==mainInSet.end())
								{
									workInputItemList.push_back(inPtr);
									mainInSet.insert(inPtr->GetId());
								}		
							} 
						}*/
					}
					else if(BZERO(workInputItemPtr->GetScarcityAmount()))
					{
						if (ReationAutoSupply(inReationList))
						{
							list<YKWorkInputItemPtr>inList=workPtr->GetCmdCodeWorkInList(workInputItemPtr->GetCmdCode());
							if (inList.empty())
							{
								workInputItemList.push_back(workInputItemPtr);
							}
							else
							{
								list<YKWorkInputItemPtr>::iterator iIter=inList.begin();
								for (;iIter!=inList.end();iIter++)
								{ 
									YKWorkInputItemPtr&inPtr=*iIter;
									workInputItemList.push_back(inPtr);
								} 
							}
							//workInputItemList.push_back(workInputItemPtr);
						}
					}
				}
			}
		}

	}

	

	//����
	//workInputItemList.sort(SortWorkInputItemList);
//	SortWorkInPutList(workInputItemList);
}

//�Թ���������Ʒ����
void ApsCal::SortWorkInPutList( list<YKWorkInputItemPtr>& workInputItemList )
{
	//����Ϊ�գ��˳�
	if (workInputItemList.empty())
	{
		return;
	}
	list<YKWorkInputItemPtr> tempWorkInputItemList;
	list<YKWorkInputItemPtr> willOrderWorkInputItemList;
	//���չ���ʵ�ʿ�������
	bool Equation=false;
	list<YKWorkInputItemPtr>::iterator iter = workInputItemList.begin();
	while(iter!=workInputItemList.end())
	{
		YKOrderPtr order= (*iter)->GetOrder();
		YKOrderPtr peakOrder = order->GetPeakOrder();

		if(peakOrder == NULL)	peakOrder = order;
		if (!peakOrder->WillOrderByPriorityBorderLine()) //С�ڸ������ȼ������򶩵�
		{
			willOrderWorkInputItemList.push_back(*iter);
			iter = workInputItemList.erase(iter);
			continue;
		}

		YKWorkPtr work = (*iter)->GetWork();
		if(work == NULL)	break;;

		if (work->GetSchFlag()==SchFlg_WaitSch)   //���Ų����ƻ�
		{
			iter++;
			continue;
		}
		if( work->GetActualStartTm() > 0)
		{
			list<YKWorkInputItemPtr>::iterator iterPS = tempWorkInputItemList.begin();
			for(;iterPS != tempWorkInputItemList.end();)
			{
				YKWorkPtr workPS = (*iterPS)->GetWork();
				if(workPS == NULL)	break;;

				if(workPS->GetActualStartTm()>work->GetActualStartTm())
				{
					tempWorkInputItemList.insert(iterPS,*iter);
					break;
				}else if(workPS->GetActualStartTm()==work->GetActualStartTm())
				{
					Equation=true;
					break;
				}
				iterPS++;
			}
			if (Equation)  //���ʵ�����
			{
				iter++;
				Equation=false;
				continue;
			}
			if(iterPS == tempWorkInputItemList.end())
				tempWorkInputItemList.push_back(*iter);

			iter = workInputItemList.erase(iter);
			continue;
		}
		iter++;
	}

	//���չ����ƻ�����ʱ������
	Equation=false;
	iter = workInputItemList.begin();
	while(iter!=workInputItemList.end())
	{
		YKOrderPtr order = (*iter)->GetOrder();
		if(order == NULL)	break;
		YKOrderPtr peakOrder = order->GetPeakOrder();

		if(peakOrder == NULL)	peakOrder = order;
		if (!peakOrder->WillOrderByPriorityBorderLine()) //С�ڸ������ȼ������򶩵�
		{
			willOrderWorkInputItemList.push_back(*iter);
			iter = workInputItemList.erase(iter);
			continue;
		}
		YKWorkPtr work = (*iter)->GetWork();
		if(work == NULL)	break;;
		if (work->GetSchFlag()==SchFlg_WaitSch)   //���Ų����ƻ�
		{
			iter++;
			continue;
		}
		if( work->GetActualStartTm() > 0)  //�������ʵ��˵���й�����ʵ����ʼʱ���������
		{
            iter++;
			continue;
		}
		if(  work->GetPlantStartTm() > 0)
		{
			list<YKWorkInputItemPtr>::iterator iterOS = tempWorkInputItemList.begin();
			for(;iterOS != tempWorkInputItemList.end();)
			{
				YKWorkPtr workOS = (*iterOS)->GetWork();
				if(workOS == NULL)	break;;
				if( workOS->GetPlantStartTm()>work->GetPlantStartTm())
				{
					tempWorkInputItemList.insert(iterOS,*iter);
					break;
				}else if (workOS->GetPlantStartTm()==work->GetPlantStartTm())
				{
					Equation=true;
					break;
				}
				iterOS++;
			}
			if (Equation)  ////����ƻ����
			{
				iter++;
				Equation=false;
				continue;
			}
			if(iterOS == tempWorkInputItemList.end())
				tempWorkInputItemList.push_back(*iter);
             
			iter = workInputItemList.erase(iter);
			continue;
		}

		iter++;
	}
	//���ն��������깤ʱ�̼���ֵ����
	iter = workInputItemList.begin();
	while(iter!=workInputItemList.end())
	{
		YKOrderPtr order = (*iter)->GetOrder();
		if(order == NULL)	break;
		YKOrderPtr peakOrder = order->GetPeakOrder();
		if(peakOrder == NULL)	peakOrder = order;
		if (!peakOrder->WillOrderByPriorityBorderLine()) //С�ڸ������ȼ������򶩵�
		{
			willOrderWorkInputItemList.push_back(*iter);
			iter = workInputItemList.erase(iter);
			continue;
		}
		list<YKWorkInputItemPtr>::iterator iterOC = tempWorkInputItemList.begin();
		for(;iterOC != tempWorkInputItemList.end();)
		{
			YKOrderPtr orderOS = (*iterOC)->GetOrder(); 
			if(orderOS == NULL)	break;

			if(orderOS->GetCalLastEndTime() > order->GetCalLastEndTime())
			{
				tempWorkInputItemList.insert(iterOC,*iter);
				break;
			}

			iterOC++;
		}
		if(iterOC == tempWorkInputItemList.end())
			tempWorkInputItemList.push_back(*iter);

		iter = workInputItemList.erase(iter);
	}
	willOrderWorkInputItemList.sort(SortWillOrderWorkIn);
	tempWorkInputItemList.insert(tempWorkInputItemList.end(),willOrderWorkInputItemList.begin(),willOrderWorkInputItemList.end());
	workInputItemList = tempWorkInputItemList;
}

bool ApsCal::SortWillOrderWorkIn(YKWorkInputItemPtr& first,YKWorkInputItemPtr& second)
{
	YKOrderPtr firstOrder=first->GetOrder();
	YKOrderPtr secondOrder=second->GetOrder();
	YKOrderPtr firstpeakOrder = firstOrder->GetPeakOrder();
	if(firstpeakOrder == NULL)	firstpeakOrder = firstOrder;
	YKOrderPtr secondpeakOrder = secondOrder->GetPeakOrder();
	if(secondpeakOrder == NULL)	secondpeakOrder = secondOrder;
	if (NULL!=firstpeakOrder&&NULL!=secondpeakOrder)
	{ 
		if(firstpeakOrder->GetCalLastEndTime() < secondpeakOrder->GetCalLastEndTime())
			return true;
		else 
			return false;
	}
	return false;
}

void ApsCal::CreatWorkOutputList( YKItemPtr& itemPtr, 
								 list<YKWorkOutputItemPtr>& workOutputItemList0, 
								 list<YKWorkOutputItemPtr>& workOutputItemList1, 
								 list<YKWorkOutputItemPtr>& workOutputItemList2,
								 list<YKWorkOutputItemPtr>& workOutputItemList21,
								 list<YKWorkOutputItemPtr>& workOutputItemList3,
								 list<YKWorkOutputItemPtr>& workOutputItemListOrg)
{
	workOutputItemListOrg=itemPtr->GetWorkOutList();
	YK_FLOAT outAmout0=0,outAmout1=0,outAmout3=0;
	for(list<YKWorkOutputItemPtr>::iterator iter = workOutputItemListOrg.begin();
		iter != workOutputItemListOrg.end();iter++)
	{
		YKWorkOutputItemPtr& workOutputItemPtr = *iter;

		if(!BZERO(workOutputItemPtr->GetStockAmount()))
		{
			workOutputItemPtr->SetSpilthAmount(workOutputItemPtr->GetShowSpilthAmount());
			workOutputItemPtr->SetStockAmount(0);	
		}

		if( workOutputItemPtr != NULL
			&& workOutputItemPtr->GetOutputItemId() == itemPtr->GetId() )
		{
			if (workOutputItemPtr->GetSpilthAmount() < PRECISION) continue;

			YKOrderPtr orderPtr = workOutputItemPtr->GetOrder();
			if (NULL==orderPtr/*||!orderPtr->GetValid()*/||orderPtr->GetExcptFlg()!=1)//δ��ɸѡ�Ķ��� //��������ȷչ��
				continue;

			if (orderPtr->GetType() == OrderType_MTOrder)
				continue;

			if(orderPtr->GetNotDispatch())
				continue;

			/*if( ( orderPtr->GetType()==OrderType_SellOrder && !BIZAPI::CheckLimit( LimitOfAuthor_Sell ))
				||( orderPtr->GetType()==OrderType_StockOrder && !BIZAPI::CheckLimit( LimitOfAuthor_Stock ) ) )
				continue;*/

			//���ݶ�����������Ϣ
			if (orderPtr->GetDiffer()==OrderDiffer_Auto)
			{
				if (workOutputItemPtr->GetIsMainProduction())
				{
					list<YKWorkRelationPtr>inforransferReationList;
					workOutputItemPtr->GetRelationPtrList(inforransferReationList);  //�������й���
					//������������չ������������˳������������������ϵ���������ݶ�����������Ϣ 
					SortReationByTimeInWorkInput(inforransferReationList);
				}
			}
			 
			YKWorkPtr workPtr = workOutputItemPtr->GetWork();

			//��Ӧ�����ǲ��ǰ�������˳�
			if(workPtr == NULL) continue;

			if(workPtr->GetDivisionType() != DivType_NoDivWork )
			{
				continue;
			}

			{
				YKOrderPtr orderPtr = workOutputItemPtr->GetOrder();
				if(orderPtr != NULL)
				{

					YK_SHORT nStockMode(0);
					nStockMode = itemPtr->GetStockMode();
					YK_SHORT nType(0);
					nType = orderPtr->GetType();

					//����0
					if (orderPtr->GetDesignation()!=L"")
					{
						outAmout0+=workOutputItemPtr->GetSpilthAmount();
						workOutputItemList0.push_back(workOutputItemPtr);
						if (orderPtr->GetDiffer() == OrderDiffer_Hand||orderPtr->ReationFilterByState()) 
							m_minStckAmoutList.push_back(workOutputItemPtr);          //�������С������
						else
							m_minTarStckAmoutList.push_back(workOutputItemPtr);          //�������СĿ�������
						continue;
					}

					if(orderPtr->GetDiffer() == OrderDiffer_Hand		//¼�붨��
						||orderPtr->ReationFilterByState()
						/*|| orderPtr->GetSchState() == OrderSchState_Start
						|| orderPtr->GetSchState() == OrderSchState_Done*/
						) //��ʼ�������
					{		
						if(	(nStockMode == StockMode_Buy //���Ϸ�ʽ�ɹ�
							&& (nType == OrderType_StockOrder
							||nType == OrderType_RelOrder
							||nType == OrderType_AbsOrder)
							) //�ɹ�����
							||
							(nStockMode == StockMode_Inside //���Ϸ�ʽ����								
							&& (nType == OrderType_PudOrder
							||nType == OrderType_RelOrder
							||nType == OrderType_AbsOrder)
							)	//���춨��
							||
							(nStockMode!= StockMode_Buy && nStockMode!= StockMode_Inside)
							)
						{
							outAmout1+=workOutputItemPtr->GetSpilthAmount();
							workOutputItemList1.push_back(workOutputItemPtr);
							continue;
						}				
					}

					if((nStockMode == StockMode_BuyPre || nStockMode == StockMode_Buy)//���Ϸ�ʽ�ɹ�����
						&& nType == OrderType_StockOrder) //�ɹ�����
					{
						if (IfHaveRelation(workOutputItemPtr))
						{
							workOutputItemList21.push_back(workOutputItemPtr);
							continue;
						}
   
                            workOutputItemList2.push_back(workOutputItemPtr);
							continue;

					}

					if((nStockMode == StockMode_InsidePre ||nStockMode == StockMode_Inside) //���Ϸ�ʽ��������
						&& nType == OrderType_PudOrder)	//���춨��
					{
						if (IfHaveRelation(workOutputItemPtr))
						{
							workOutputItemList21.push_back(workOutputItemPtr);
							continue;
						}
							workOutputItemList2.push_back(workOutputItemPtr);
							continue;
					}
					//����3
					workOutputItemList3.push_back(workOutputItemPtr);
					if (orderPtr->GetDiffer() == OrderDiffer_Hand||orderPtr->ReationFilterByState()) 
						m_minStckAmoutList.push_back(workOutputItemPtr);          //�������С������
					else
						m_minTarStckAmoutList.push_back(workOutputItemPtr);          //�������СĿ�������
				}	
			}

		}
	}
	m_totalAmout=outAmout0+outAmout1+outAmout3;   //������Ŀ���桢��С�������������
	YK_FLOAT minStockCount  = itemPtr->GetMinStock();
	YK_FLOAT minTarStockCount  = itemPtr->GetMinTargetStock();
	if (itemPtr->GetAutoSupply()==AutoFlag_Yes
		||itemPtr->GetAutoSupply()==AutoFlag_YesChangeOne)
	{
		m_unused=max(outAmout1-max(minStockCount/*-outAmout0-outAmout3*/,0),0);  
		if (minStockCount>0)
		{
			m_minStckAmoutMap[itemPtr->GetId()]=minStockCount;
			m_minTarStckAmoutMap[itemPtr->GetId()]=max(minTarStockCount,minStockCount);
		}
		else if (minTarStockCount>0)
		{
			m_minTarStckAmoutMap[itemPtr->GetId()]=minTarStockCount;
		}
	}
	else
	{
		m_unused=outAmout1;
	}
	//m_supplyAmout=max(max(itemPtr->GetMinTargetStock(),minStockCount)-outAmout0-outAmout1-outAmout3,0);

	//����

	//workOutputItemList0.sort(SortWorkOutPutList);
	//workOutputItemList1.sort(SortWorkOutPutList);
	//workOutputItemList2.sort(SortWorkOutPutList);
	//workOutputItemList21.sort(SortWorkOutPutList);
}

bool ApsCal::SortWorkOutPutList( YKWorkOutputItemPtr& first,YKWorkOutputItemPtr& second )
{
	if ( first==NULL || second==NULL )
	{
		return false;
	}			

	//Add 2011-12-22 ��涩���������충��ǰ��
	YKOrderPtr fOrder = first->GetOrder();
	YKOrderPtr sOrder = second->GetOrder();
	if ( fOrder != NULL && sOrder != NULL )
	{
		if (fOrder->GetItem() == sOrder->GetItem())
		{
			//1�����ۣ�2�����춨����3���ɹ�������4��ά��������5����涨�������������6����涨��������������7�����򶩵���8�����󶩵�
			const YK_SHORT fOrderType = fOrder->GetType();
			const YK_SHORT sOrderType = sOrder->GetType();
			
			return fOrderType > sOrderType;
		}			
	}
	

	YK_TIME_T fTm=first->GetCmpTime() ;
	YK_TIME_T sTm=second->GetCmpTime() ;
	if (fTm<sTm)
	{
		return true;
	}
	else if (fTm==sTm)
	{
		YKOrderPtr order =first->GetOrder();
		YKOrderPtr orderPS =second->GetOrder();
		if (NULL!=order&&NULL!=orderPS)  //���ն������ȼ���
		{
			//���ȼ�����
			if(order->GetPriority() > orderPS->GetPriority())
				return true;
			else if(order->GetPriority() == orderPS->GetPriority())
			{
				//ID������
				if(order->GetId() < orderPS->GetId())
					return true;
				else if (order->GetId() == orderPS->GetId())
				{
					return first->GetId() < second->GetId();
				}
				
				return false;
			}		
		}
	}

	//if(first->GetOutPutItemCompTime() < second->GetOutPutItemCompTime())
	//{
	//	return true;
	//}else if(first->GetOutPutItemCompTime() == second->GetOutPutItemCompTime())
	//{
	//	YKOrderPtr order =first->GetOrder();
	//	YKOrderPtr orderPS =second->GetOrder();
	//	if (NULL!=order&&NULL!=orderPS)  //���ն������ȼ���
	//	{
	//		//���ȼ�����
	//		if(order->GetPriority() > orderPS->GetPriority())
	//			return true;
	//		else if(order->GetPriority() == orderPS->GetPriority())
	//		{
	//			//ID������
	//			if(first->GetId() < second->GetId())
	//				return true;
	//			else
	//				return false;
	//		}

	//	}
	//}
	return false;
}
//����ֵtrueΪ��ƥ��
bool ApsCal::InAndOutIfMatch(/*YKItemPtr& ptr,*/YKWorkInputItemPtr& workInItem,YKWorkOutputItemPtr& workOutItem)
{
	YKOrderPtr preOrder=workOutItem->GetOrder();
	YKOrderPtr nexOrder=workInItem->GetOrder();
	if (NULL!=preOrder&&NULL!=nexOrder)
	{
		if (preOrder->CmpLastCompleteTime(nexOrder))
		{
			return true;
		}
		//if (ptr->GetAutoSupply()==AutoFlag_YesStOne
		//	&&preOrder->GetDiffer()==OrderDiffer_Auto)
		//{
		//	return true;
		//}
		//YKOrderPtr peakOrder = nexOrder->GetPeakOrder();

		//if(peakOrder == NULL)	peakOrder = nexOrder;
		//if ((peakOrder->GetDesignation()!=L""||peakOrder->GetType()==OrderType_WillOrder)
		//	&&preOrder->GetDiffer()==OrderDiffer_Auto)
		//{
		//	return true;
		//}
		//if (ptr->GetAutoSupply()>AutoFlag_Yes
		//	&&preOrder->GetDiffer()==OrderDiffer_Auto&&nexOrder->IfHaveReation(preOrder->GetId()))
		//{
		//	return true;
		//}
		////���򶩵���������
		//if (peakOrder->GetType()==OrderType_WillOrder)
		//{
		//	if (!IfCreatReationByApsParam(preOrder,2))
		//	{
		//		return true;
		//	}

		//}
		////���Ʒ��ŵĶ���
		//if (peakOrder->GetDesignation()!=L"")
		//{
		//	if (!IfCreatReationByApsParam(preOrder,1))
		//	{
		//		return true;
		//	} 
		//}
		//�󶩵���Լ
	}
	return false;
}
//�����ų̲����ж��Ƿ񴴽�����true:Ψһ����
bool ApsCal::IfCreatReationByApsParam(YKOrderPtr&preOrder,int type)
{
	if (type==1)    //���Ʒ��ŵĶ���
	{ 
		if (preOrder->GetType()==OrderType_AbsOrder
			&&g_pBizModel->GetApsParam()->ParseHaveDesignOrderRelCondition(0))
		{
			return true;
		}else if (preOrder->GetType()==OrderType_RelOrder
			&&g_pBizModel->GetApsParam()->ParseHaveDesignOrderRelCondition(1))
		{
			return true;
		}else if (preOrder->GetType()==OrderType_StockOrder
			&&g_pBizModel->GetApsParam()->ParseHaveDesignOrderRelCondition(2))
		{
			return true;
		}
		else if (preOrder->GetType()==OrderType_PudOrder
			&&g_pBizModel->GetApsParam()->ParseHaveDesignOrderRelCondition(3))
		{
			return true;
		}

	}else
	{
		if (preOrder->GetType()==OrderType_AbsOrder
			&&g_pBizModel->GetApsParam()->ParseWillOrderRelCondition(0))
		{
			return true;
		}else if (preOrder->GetType()==OrderType_RelOrder
			&&g_pBizModel->GetApsParam()->ParseWillOrderRelCondition(1))
		{
			return true;
		}else if (preOrder->GetType()==OrderType_StockOrder
			&&g_pBizModel->GetApsParam()->ParseWillOrderRelCondition(2))
		{
			return true;
		}
		else if (preOrder->GetType()==OrderType_PudOrder
			&&g_pBizModel->GetApsParam()->ParseWillOrderRelCondition(3))
		{
			return true;
		}
	}
	return false;
}

//�������Ʒ���֮��Ĺ���
void ApsCal::CreatReationByDesignation(/*YKItemPtr& itemPtr,*/list<YKWorkInputItemPtr>&workInputItemList,list<YKWorkOutputItemPtr>& workOutputItemList0)
{
	for(list<YKWorkInputItemPtr>::iterator iterIItem =workInputItemList.begin()
		;iterIItem!=workInputItemList.end();iterIItem++)
	{
          YKWorkInputItemPtr&inPtr=*iterIItem;
		  if (NULL==inPtr)
		  continue;
		  InOutByDesignation(inPtr,workOutputItemList0);
	}

}
void ApsCal::Input2OutputByWillOrderAndDesignation(YKOrderPtr&peakOrder,YKWorkInputItemPtr& inputItem,
												   list<YKWorkOutputItemPtr>& workOutputItemList0, 
												   int type,list<YKWorkRelationPtr>&reationList )
{
	if(BZERO(inputItem->GetScarcityAmount()))
		return;

	YK_FLOAT relationCount ;   //��������
	list<YKWorkOutputItemPtr>::iterator iter_output1;
	iter_output1=workOutputItemList0.begin();
	for (;iter_output1!=workOutputItemList0.end();iter_output1++)
	{
		YKWorkOutputItemPtr&outputPtr=*iter_output1;
		YKOrderPtr orderOutPtr =outputPtr->GetOrder();        

		if (orderOutPtr!=NULL)
		{
			if ((orderOutPtr->GetType()==	OrderType_RelOrder
				||	orderOutPtr->GetType() == OrderType_AbsOrder)
				&&IfCreatReationByApsParam(orderOutPtr,type))
			{
				if (orderOutPtr->GetDesignation()==peakOrder->GetDesignation())//�ö������Ʒ����붥�㶩�����Ʒ�����ͬ
				{
					relationCount = min(inputItem->GetScarcityAmount(),outputPtr->GetSpilthAmount());

					if (relationCount < 0.00001)
						continue;


					YKOrderPtr nexOrder= inputItem->GetOrder();
					//if (nexOrder->IfHaveReationCyc(outputPtr->GetOutputItemId())) //���ڹ���ѭ��
					//	continue;
					if (NULL!=orderOutPtr&&NULL!=nexOrder)    //���ݹ�������깤ʱ�̼���ֵ
					{
						orderOutPtr->PreOrderLateFinishTimeCalValue(nexOrder);   
					}
					//����������
					YKWorkRelationPtr&workReation =YKWorkRelation::CreatNew(inputItem,outputPtr,RelationType_Differ,relationCount,RelationFixFlg_UnFix);
					if (NULL!=workReation)
					{
                       reationList.push_back(workReation);
					}
					
					if(BZERO(inputItem->GetScarcityAmount()))
						return;
				}
			}
		}
	}

}

void ApsCal::Input2OutputByWillOrder(YKOrderPtr&peakOrder,YKWorkInputItemPtr& inputItem,
									 list<YKWorkOutputItemPtr>& workOutputItemList0, 
									 int type,list<YKWorkRelationPtr>&reationList )
{
	if(BZERO(inputItem->GetScarcityAmount()))
		return;
	YK_FLOAT relationCount ;   //��������
	list<YKWorkOutputItemPtr>::iterator iter_output1;
	iter_output1=workOutputItemList0.begin();
	for (;iter_output1!=workOutputItemList0.end();iter_output1++)
	{
		YKWorkOutputItemPtr&outputPtr=*iter_output1;
		YKOrderPtr orderOutPtr =outputPtr->GetOrder();


		if (orderOutPtr!=NULL)
		{
			if ((orderOutPtr->GetType()==OrderType_PudOrder
				||orderOutPtr->GetType()==OrderType_StockOrder)
				&&!IfCreatReationByApsParam(orderOutPtr,type)
				&& orderOutPtr->GetDiffer() != 2)
				continue;

			if (orderOutPtr->GetDesignation()==peakOrder->GetDesignation())//�ö������Ʒ����붥�㶩�����Ʒ�����ͬ
			{
				relationCount = min(inputItem->GetScarcityAmount(),outputPtr->GetSpilthAmount());

				if (relationCount < 0.0001)
					continue;
				YKOrderPtr nexOrder= inputItem->GetOrder();
				//if (nexOrder->IfHaveReationCyc(outputPtr->GetOutputItemId())) //���ڹ���ѭ��
				//	continue;

				if (NULL!=orderOutPtr&&NULL!=nexOrder)    //���ݹ�������깤ʱ�̼���ֵ
				{
					orderOutPtr->PreOrderLateFinishTimeCalValue(nexOrder);   
				}
				//����������
				YKWorkRelationPtr&workReation=YKWorkRelation::CreatNew(inputItem,outputPtr,RelationType_Differ,relationCount,RelationFixFlg_UnFix);
				if (NULL!=workReation)
				{
					 reationList.push_back(workReation);
				}
				if(BZERO(inputItem->GetScarcityAmount()))
					return;

			}
		}
	}
}
void ApsCal::ReationUpdate(list<YKWorkInputItemPtr>& workInputItemList
						   ,list<YKWorkOutputItemPtr>& workOutputItemList2,list<YKWorkOutputItemPtr>& workOutputItemList21)
{
	list<YKWorkRelation> reationList;
	for(list<YKWorkInputItemPtr>::iterator iter = workInputItemList.begin();iter != workInputItemList.end();++iter)
	{
		list<YKWorkRelationPtr> inReationList;
		iter->GetRelationPtrList(inReationList);  //�������й���
		reationList.insert(reationList.end(),inReationList.begin(),inReationList.end()); //����������й���������,������������
	}

	for (list<YKWorkRelationPtr>::iterator itor=reationList.begin()
		;itor!=reationList.end();itor++)
	{
		YKWorkRelationPtr&workReation=*itor;
		if (NULL!=workReation)
		{
			if(workReation->GetRelaTye()==RelationType_Same)
				continue;
			YKOrderPtr           preOrder   =workReation->GetPreOrderPtr();
			YKOrderPtr           nxtOrder   =workReation->GetNextOrderPtr();
			YKItemPtr            itemPtr    =workReation->GetItemPtr();
			YKWorkOutputItemPtr  workOut    = workReation->GetWorkOutputItemPtr();
			YKWorkInputItemPtr   workIn     = workReation->GetWorkInputItemPtr();
			YKWorkPtr            preWorkPtr = workReation->GetPreWorkPtr();
			YKWorkPtr            nxtWorkPtr = workReation->GetNextWorkPtr();

			if (preOrder==NULL||workOut==NULL||workIn==NULL||itemPtr==NULL||nxtOrder==NULL || preWorkPtr == NULL || nxtWorkPtr == NULL )
				continue;
            if ( preWorkPtr->GetDivisionType() == DivType_DivBackWork 
				|| nxtWorkPtr->GetDivisionType() == DivType_DivBackWork  )
            {
				continue;
            }
             

			//if( ( preOrder->GetType()==OrderType_SellOrder && !BIZAPI::CheckLimit( LimitOfAuthor_Sell ))   //�޲������۶�����Ȩ��
			//	||( preOrder->GetType()==OrderType_StockOrder && !BIZAPI::CheckLimit( LimitOfAuthor_Stock ) ) )//�޲����ɹ�������Ȩ��
			//	continue;

			//if( ( nxtOrder->GetType()==OrderType_SellOrder && !BIZAPI::CheckLimit( LimitOfAuthor_Sell ))   //�޲������۶�����Ȩ��
			//	||( nxtOrder->GetType()==OrderType_StockOrder && !BIZAPI::CheckLimit( LimitOfAuthor_Stock ) ) )//�޲����ɹ�������Ȩ��
			//	continue;

			if (IfUpdateForPreOrder(itemPtr,workIn ,workOut))
			{
				if(!preOrder->IfAllowUpdate())  //�Ƿ��������
					continue;

				//bool update=false;

				//list<YKWorkPtr>worklist;
				//preOrder->GetWorkPtrList(worklist);
				//if (worklist.empty())  //�޹�����ɾ���ö���
				//{
				//	update=false;
				//}
				////�й���
				//list<YKWorkPtr>::iterator iter=worklist.begin();
				//for (;iter!=worklist.end();iter++)
				//{ 
				//	YKWorkPtr&work=*iter;
				//	if (NULL!=work)
				//	{
				//		if (work->GetState()>WorkState_PlantDone) //δ�ƻ�,�ƻ����
				//		{
				//			update=true;
				//			break;
				//		}
				//	}
				//}
				//if (update)
				//	continue;
				//if( itemPtr->GetAutoSupply()>AutoFlag_Yes)
				//{
				//                      update=true;
				//}else
				//{
				//	YKOrderPtr nexOrder =workReation->GetNextOrderPtr();

				//	YKOrderPtr peakOrder = nexOrder->GetPeakOrder();

				//	if(peakOrder == NULL)	peakOrder = nexOrder;
				//	if ((nexOrder->GetDesignation()!=L""||nexOrder->GetType()==OrderType_WillOrder)
				//		||(peakOrder->GetDesignation()!=L""||peakOrder->GetType()==OrderType_WillOrder))
				//	{
				//		update=true;
				//	}
				//}
				//if (!update)
				//	continue;

				YK_FLOAT rAmout=0;
				YKOrderPtr nextOrderPtr =workReation->GetNextOrderPtr();
				if (g_pBizModel->GetApsParam()->GetInPOdBatch())
				{
					YK_FLOAT maxBatchAmout = itemPtr->GetSplitMaxBatch();
					if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4 )
					{
						maxBatchAmout = itemPtr->GetPurMaxBatch();
					}
					if (maxBatchAmout==-1||maxBatchAmout==0)
					{
						rAmout=workIn->GetScarcityAmount()+workReation->GetAmount()+nextOrderPtr->GetAlreadyAssignQuantity();
					}else
						rAmout=min(workIn->GetScarcityAmount()+workReation->GetAmount()+nextOrderPtr->GetAlreadyAssignQuantity(),maxBatchAmout);
				}else
					rAmout=workIn->GetScarcityAmount()+workReation->GetAmount()+nextOrderPtr->GetAlreadyAssignQuantity();
				if (rAmout<0.000001)
				{
					workReation->Delete();  //ɾ����������
					YKOrderPtr peakOrder = preOrder->GetPeakOrder();
					if(peakOrder == NULL)	peakOrder = preOrder;
					if (preOrder->GetDesignation()==L""||peakOrder->GetDesignation()==L"")     //�����Ʒ��ŵĶ������ٴ���
						SaveWorkOutGather(workOut,workOutputItemList2,workOutputItemList21);
					continue;
				}

				//add by yp 2012.6.5 һ��һ������������
				//////////////////////////////////////////////////////////////////////////
				YK_FLOAT batchUnitAmout = itemPtr->GetSplitBatchUnit();
				if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4 )
				{
					batchUnitAmout = itemPtr->GetPurBatchUnit();
				}
				if(batchUnitAmout > PRECISION)
					rAmout = ceil(rAmout/batchUnitAmout)*batchUnitAmout;
				//////////////////////////////////////////////////////////////////////////

				YK_FLOAT oldAmout=preOrder->GetAmount();
				preOrder->SetAmount(rAmout);   //����ǰ����������
				preOrder->SetAboveOrderId( nxtOrder->GetId());
				preOrder->UpdateIOItem();  
				preOrder->SetAboveOrderId( 0);
				//���������Ϣ
				if ( preOrder->GetExcptFlg()!=1)
				{
					if (preOrder->GetType() == OrderType_PudOrder )  //���춨��
						AddFialPudOrderList(preOrder->GetId());
					if (preOrder->GetExcptFlg()==Spread_No_Bom)
						AddFailItemBomList(  preOrder->GetPudId());    //���һ��BOM��Ч��Ϣ
					if (preOrder->DelCondition())
					{
						preOrder->Delete();
					}
				}else
				{
					ReUpdateReation(workIn,preOrder);

					YKWorkOutputItemPtr outPtr= YKWorkOutputItem::FindOrderOutItem(preOrder);

					if (NULL!=outPtr&&outPtr->GetSpilthAmount()>0)
					{
						YKOrderPtr peakOrder = preOrder->GetPeakOrder();
						if(peakOrder == NULL)	peakOrder = preOrder;
						if (preOrder->GetDesignation()==L""||peakOrder->GetDesignation()==L"")     //�����Ʒ��ŵĶ������ٴ���
						{
							DeleteWorkOutPtr(workOut, workOutputItemList2,workOutputItemList21);
							SaveWorkOutGather(outPtr,workOutputItemList2,workOutputItemList21);
						}
					}
					if (oldAmout!=preOrder->GetAmount())
					{
						if (preOrder->GetType() == OrderType_StockOrder 
							&&preOrder->GetDiffer()==OrderDiffer_Auto) //�ɹ�����
						{
							AddUpdataAutoStockOrderList(preOrder->GetId());
						}else if (preOrder->GetType() == OrderType_PudOrder 
							&&preOrder->GetDiffer()==OrderDiffer_Auto)  //���춨��
						{
							AddUpdataAutoPudOrderList(preOrder->GetId());
						}
					}
				}
			} //end in or out
		}// end reation 
	}//end for
}

//��¼�붩������״̬�Ķ�����������
void ApsCal::CreatReationHandOrState(YKItemPtr& itemPtr,YKWorkInputItemPtr& workInItem,list<YKWorkOutputItemPtr>& workOutputItemList1)
{
	//����1ȡһ������������¼
	for(list<YKWorkOutputItemPtr>::iterator iterOut =workOutputItemList1.begin()
		;iterOut!=workOutputItemList1.end();)
	{
		YKWorkOutputItemPtr& workOutItem = *iterOut;

		if (m_unused<=0)
			break;

		if((*iterOut)->GetSpilthAmount() < 0.00001)
		{
			iterOut = workOutputItemList1.erase(iterOut);
			continue;
		}

		//�Ƿ��������Ʒƥ��
		if (InAndOutIfMatch(/* itemPtr,*/ workInItem, workOutItem))
		{
			iterOut++;
			continue;
		}
		YKOrderPtr nexOrder= workInItem->GetOrder();
        YKOrderPtr orderOutPtr= (*iterOut)->GetOrder();
        
		//if (!orderOutPtr->JustIfHaveTopOrder())
		//	orderOutPtr->OrderLateFinishTimeCalValue();
		//if (NULL!=orderOutPtr&&NULL!=nexOrder)    //���ݹ�������깤ʱ�̼���ֵ
		//{
		//	orderOutPtr->PreOrderLateFinishTimeCalValue(nexOrder);   
		//}
		////���ݶ��� ��������Ϣ
		//JustIfInforTransfer(workInItem,workOutItem);

		InforransferAndOrderRenovate(workInItem,workOutItem);   //��Ϣ�����Լ���������
		if(orderOutPtr->GetExcptFlg()!=1)
		{
			if (orderOutPtr->GetExcptFlg()==Spread_No_Bom)
				AddFailItemBomList( itemPtr->GetId());    //���һ��BOM��Ч��Ϣ
			//���������Ϣ
			AddFialPudOrderList(orderOutPtr->GetId());
			if (orderOutPtr->DelCondition())
				orderOutPtr->Delete();
			iterOut=workOutputItemList1.erase(iterOut);
			continue;
		}
		YKWorkOutputItemPtr workOutputItem = YKWorkOutputItem::FindOrderOutItem(orderOutPtr);
		if (NULL==workOutputItem)
		{
			iterOut=workOutputItemList1.erase(iterOut);
			continue;
		}
		*iterOut=workOutputItem;         //������Ʒ����·���л�����������ı䣬ͬʱ�����������
		workOutItem=workOutputItem;
		
		 YK_FLOAT reationAmout=0;   
		 //����Թ���������֮��
		 CalReationAmoutForHandOrStateOrder(itemPtr,workInItem,reationAmout);
         reationAmout=min(m_unused,min(workInItem->GetScarcityAmount()+reationAmout,workOutItem->GetSpilthAmount()));
		 if (reationAmout<0.000001)
		    if (iterOut!=workOutputItemList1.end())
		    {
				iterOut++;
				continue;
		    }else
				return;

		if (!FindTheSameReationByWorkInAndWorkOut(reationAmout, workInItem,workOutItem)) 
		  YKWorkRelation::CreatNew(workInItem ,workOutItem,RelationType_Differ,reationAmout);

		m_unused=m_unused-reationAmout;
		m_totalAmout-=reationAmout;
			//�����������
	//	Input2Output(workInItem,*iterOut,workOutItem->GetSpilthAmount());
		 //�����ֶ�

		if(BZERO((*iterOut)->GetSpilthAmount())) //��������Ϊ0
		{
			iterOut = workOutputItemList1.erase(iterOut); //ɾ�������

 			if(BZERO((workInItem)->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
 				break;
 			else
				continue;
		}
		else  //�����Ʒ����������Ϊ0��������Ʒ����������Ϊ0
			break;
	}
}
//1��1��ʽ����
void ApsCal::EquitySupply(YKItemPtr& itemPtr,YKWorkInputItemPtr& workInItem)
{
	//������Ʒ����Ϊ0
	if(BZERO(workInItem->GetScarcityAmount()))
	{
		return;
	}
	else
	{ 
		CNewRuleManager rulManager;
		rulManager.SetInfo(RULE_DEF_SCH_FILTER_Item,g_pBizModel->GetApsParam()->GetItemFiterRule(),TblItem);
		if(!rulManager.GetBoolValue(workInItem->GetInputItemId()))  //��Ʒɸѡ
			return;

		rulManager.SetInfo(RULE_DEF_FILTER_Order,g_pBizModel->GetApsParam()->GetOrderFiter(),TblOrder);
		if(!rulManager.GetBoolValue(workInItem->GetOrderId()))  //����ɸѡ
			return;

		YKOrderPtr nextOrder =workInItem->GetOrder();

		if(nextOrder->CheckItemCyc(itemPtr->GetId()))     //�й���Ʒ��ѭ��
			return ;

		//if( ( itemPtr->GetStockMode() == StockMode_Buy || itemPtr->GetStockMode() == StockMode_BuyPre )
		//	&&  !BIZAPI::CheckLimit( LimitOfAuthor_Stock ) )//�޲����ɹ�������Ȩ��
		//	return;


		list<YK_FLOAT>couts;
		if (!g_pBizModel->GetApsParam()->GetInPOdBatch())
		{
           couts.push_back(workInItem->GetScarcityAmount()); 
		}
		else
			EquitySupplyByMaxBatchSplit(itemPtr,workInItem,couts);

		YK_BOOL desWillFlg =   (nextOrder->PeakDesignationOrders()||nextOrder->PeakWillOrders())?true:false;
		for (list<YK_FLOAT>::iterator iter =couts.begin();iter!=couts.end();iter++)
        {
			YK_FLOAT rAmout = *iter;
			//add by yp 2012.6.5 һ��һ������������
			//////////////////////////////////////////////////////////////////////////
			YK_FLOAT batchUnitAmout = itemPtr->GetSplitBatchUnit();
			if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4 )
			{
				batchUnitAmout = itemPtr->GetPurBatchUnit();
			}
			if(batchUnitAmout > PRECISION)
				rAmout = ceil(rAmout/batchUnitAmout)*batchUnitAmout;
			//////////////////////////////////////////////////////////////////////////

			YKOrderPtr orderPtr = YKOrder::CreatNew(rAmout,itemPtr->GetId());
			if(orderPtr != NULL)
			{
				//���㶩��������깤(����ֵ)
				orderPtr->OrderLateFinishTimeCalValue();
				orderPtr->OrderEarlyStartTimeCalValue();

				//������Ϣ����
				//    //��ʱ time: 0.185000 
				InforTransfor Trans;
				Trans.OrderInfoTransfor(nextOrder,orderPtr);

			

				if (NULL!=orderPtr&&NULL!=nextOrder)    //���ݹ�������깤ʱ�̼���ֵ
				{
					orderPtr->PreOrderLateFinishTimeCalValue(nextOrder);   
				}

				orderPtr->SetAboveOrderId( nextOrder->GetId());
				
				orderPtr->UpdateIOItem();
			
				orderPtr->SetAboveOrderId( 0);


				orderPtr->FlashOrderState();
				if(orderPtr->GetExcptFlg()!=1)
				{
					if (orderPtr->GetExcptFlg()==Spread_No_Bom)
						AddFailItemBomList( itemPtr->GetId());    //���һ��BOM��Ч��Ϣ
					//���������Ϣ
					AddFialPudOrderList(orderPtr->GetId());
					orderPtr->Delete();
					return;
				}

				//������Ϣ����
				Trans.WorkInforTransfor(orderPtr);


				YKWorkOutputItemPtr workOutputItem = YKWorkOutputItem::FindOrderOutItem(orderPtr);
				if(workOutputItem != NULL)
				{
					YKWorkRelationPtr relationPtr = YKWorkRelation::CreatNew(workInItem,workOutputItem,2,workOutputItem->GetPlantAmount());
				}

				if (desWillFlg)
				{
					CodeBringRule codeBringRule;
					orderPtr->SetCode(codeBringRule.GetCode(RULE_DEF_AUTOONEORDER,orderPtr->GetId(),CNewRuleCell::AddSerialNumber(RULE_DEF_AUTOONEORDER)));
					orderPtr->FlashWorkCode();
				}
				else
				{
						orderPtr->CreateOrderCode();
				}
				//YKOrderPtr peakOrder = nextOrder->GetPeakOrder();

				//if(peakOrder == NULL)	peakOrder = nextOrder;

	
				//if(/*peakOrder != NULL
				//   &&(peakOrder->GetDesignation() != L""
				//   ||peakOrder->GetType() == OrderType_WillOrder)*/
				//   nextOrder->PeakDesignationOrders()
				//   ||nextOrder->PeakWillOrders()) //�������췬�Ż����򶩵�
				//{

				//		// change 2010-11-4 by ll

				//	CodeBringRule codeBringRule;
				//	orderPtr->SetCode(codeBringRule.GetCode(RULE_DEF_AUTOONEORDER,orderPtr->GetId(),CNewRuleCell::AddSerialNumber(RULE_DEF_AUTOONEORDER)));
				//	orderPtr->FlashWorkCode();

				//}else
				//{		
				//		orderPtr->CreateOrderCode();
				//}

			}	
        }

	}
}
//��1��1��ʽ����
void ApsCal::NoEquitySupply(YKItemPtr& itemPtr,list<YKWorkInputItemPtr>&workInputItemList,list<YKWorkOutputItemPtr>& workOutputItemList2,list<YKWorkOutputItemPtr>&workOutputItemList21)
{
	if (!itemPtr->IfValid()) return ;
	if (itemPtr->GetAutoSupply()!=AutoFlag_Yes)
	{
		if (itemPtr->GetAutoSupply()!=AutoFlag_YesChangeOne)
		{
			return;
		}
	}

	//�Ƚ�����21�빤��������Ʒ���й���
    PriorityReationGather21(workInputItemList,workOutputItemList21);

	list<YK_FLOAT>couts;
	CollectGatherByMaxMinBatchAndOrderUniteDurTime(itemPtr,workInputItemList,couts);
	//MatchInAndOut(itemPtr,workInputItemList,workOutputItemList2,couts);

	YK_SHORT  nIsStSpMd =  g_pBizModel->GetApsParam()->GetApsIsSDataSpecMt();
	if ( nIsStSpMd != 1 )	//�����й��ƥ��
	{
		MatchInAndOut(itemPtr,workInputItemList,workOutputItemList2,couts);
	}
	else	//���ƥ�� �߼�����
		MatchInAndOutRel(itemPtr,workInputItemList,workOutputItemList2,couts);	//Add 2012-06-07 

}

//�ж��Ƿ���Ϣ����
void ApsCal::JustIfInforTransfer(YKWorkInputItemPtr& workInItem,YKWorkOutputItemPtr& workOutItem)
{
	YKOrderPtr preOrder=workOutItem->GetOrder();
	if (NULL!=preOrder)
	{
		if (!preOrder->JustIfHaveTopOrder())
		{ 
			if (preOrder->GetDiffer()==OrderDiffer_Auto)
			{
				if (workOutItem->GetIsMainProduction())
				{
					//������Ϣ���� ��������Ϣ����
					YKOrderPtr  nexOrder=workInItem->GetOrder();
					InforTransfor Trans;
					Trans.OrderInfoTransfor(nexOrder,preOrder);
					Trans.WorkInforTransfor(preOrder);
				}

			}

		}
	}
}
//������Ϣ����
void ApsCal::Inforransfer(YKOrderPtr& orderPtr)
{
    if (orderPtr->JustIfHaveTopOrder()) //������
    {
		return ;
    }
	// ����������Ϣ����
	InforTransfor Trans;
	Trans.WorkInforTransfor(orderPtr);
}
void ApsCal::CreatNewPreOrder(YKItemPtr& itemPtr,YKOrderPtr&newPreOrder,YKWorkInputItemPtr&workin ,YKWorkOutputItemPtr& workOutputItem,YK_FLOAT reationAmoutValue )
{
	YK_FLOAT newPreOrderAmout=workin->GetScarcityAmount();
	if (reationAmoutValue>0)
	{
		newPreOrderAmout =reationAmoutValue;
	}else 	if (newPreOrderAmout<0.000001)
	{
		return;
	}
	vector<YK_FLOAT> counts;
	itemPtr->CalBatchManage(newPreOrderAmout,counts); //�������������
		for(vector<YK_FLOAT>::iterator countIt = counts.begin();
			countIt != counts.end();countIt++)
		{
			newPreOrder = YKOrder::CreatNew(newPreOrderAmout,itemPtr->GetId());
			if(newPreOrder != NULL)
			{
				newPreOrder->UpdateIOItem();
				workOutputItem = YKWorkOutputItem::FindOrderOutItem(newPreOrder);
				if(workOutputItem != NULL)
				{
					YKOrderPtr nexOrder= workin->GetOrder();
					//if (nexOrder->IfHaveReationCyc(itemPtr->GetId())) //���ڹ���ѭ��
					//{
					//	newPreOrder->Delete();
					//	return;
					//}
					if (NULL!=newPreOrder&&NULL!=nexOrder)    //���ݹ�������깤ʱ�̼���ֵ
					{
						newPreOrder->PreOrderLateFinishTimeCalValue(nexOrder);   
					}
					//������Ϣ����
					Inforransfer(newPreOrder);
					//����������
					YKWorkRelation::CreatNew(workin,workOutputItem,RelationType_Differ,workin->GetScarcityAmount(),RelationFixFlg_UnFix);

				}
			}

			newPreOrder->CreateOrderCode();
		}
}
//����������
//void ApsCal::CreatOrderReationByScarcityAmount(YKItemPtr&itemPtr,list<YKWorkInputItemPtr>&workInputItemList)
//{
//	//���չ������봴������
//	YKOrderPtr newPreOrder=NULL;
//	YKWorkOutputItemPtr workOutputItem=NULL;
//	YK_FLOAT  scarcityTotalAmount=0; 
//	YK_FLOAT BatchAmout = itemPtr->GetSplitMaxBatch();
//	if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4 )
//	{
//		BatchAmout = itemPtr->GetPurMaxBatch();
//	}
//		list<YKWorkInputItemPtr>::iterator i_initer=workInputItemList.begin();
//		for (;i_initer!=workInputItemList.end();)
//		{
//			YKWorkInputItemPtr&workin=*i_initer;
//			if (NULL!=workin)
//			{
//				YKOrderPtr nexOrder=workin->GetOrder();
//				if (NULL!=nexOrder)
//				{
//					if(BZERO(workin->GetScarcityAmount())) //��������Ϊ0
//					{
//						i_initer = workInputItemList.erase(i_initer); //ɾ�������
//						continue;
//					}
//					scarcityTotalAmount+=workin->GetScarcityAmount(); 
//					if (workOutputItem==NULL)
//						CreatNewPreOrder(itemPtr,newPreOrder,workin ,workOutputItem);
//					else
//						//����������
//						YKWorkRelation::CreatNew(workin,workOutputItem,RelationType_Differ,workin->GetScarcityAmount(),RelationFixFlg_UnFix);
//
//					if (i_initer==workInputItemList.end())
//					{
//						break;
//					}else
//					{
//						i_initer++;
//					}
//				}
//			}
//		}//end workin
//
//	if(newPreOrder != NULL)
//	{
//		newPreOrder->SetAmount(scarcityTotalAmount+m_supplyAmout);
//		if (workOutputItem!=NULL)
//		{
//			workOutputItem->SetSpilthAmount(0);
//			workOutputItem->SetPlantAmount(scarcityTotalAmount+m_supplyAmout);
//		}
//		newPreOrder->UpdateIOItem();
//	}
//}
//void ApsCal::CreatReationForOutAndMuchIn(YKItemPtr&itemPtr,list<YKWorkInputItemPtr>&workInputItemList,YKWorkOutputItemPtr&workOutPtr,int type)
//{
//	//out-in : 1-��
//	bool Exist=false;
//	YK_FLOAT reationSpilthAmout=workOutPtr->GetPlantAmount()-workOutPtr->GetSpilthAmount();
//	list<YKWorkInputItemPtr>::iterator i_initer=workInputItemList.begin();
//	for (;i_initer!=workInputItemList.end();)
//	{
//		YKWorkInputItemPtr&workin=*i_initer;
//		YKOrderPtr nexOrder=workin->GetOrder();
//		if (NULL!=nexOrder)
//		{
//			if(BZERO(workin->GetScarcityAmount())) //��������Ϊ0
//			{
//				i_initer = workInputItemList.erase(i_initer); //ɾ�������
//				continue;
//			}
//			/*if (nexOrder->IfHaveReationCyc(workOutPtr->GetOutputItemId()))
//				break;*/
//			YKOrderPtr preOrder=workOutPtr->GetOrder();
//			if (NULL!=preOrder&&NULL!=nexOrder)    //���ݹ�������깤ʱ�̼���ֵ
//			{
//				preOrder->PreOrderLateFinishTimeCalValue(nexOrder);   
//			}
//			JustIfInforTransfer(workin,workOutPtr);
//			reationSpilthAmout+=workin->GetScarcityAmount();
//            Exist=true;		
//			//����������
//			YKWorkRelationPtr relationPtr = YKWorkRelation::CreatNew(workin,workOutPtr,2,workin->GetScarcityAmount());
//			if (i_initer==workInputItemList.end())
//			{
//				break;
//			}else if(BZERO(workin->GetScarcityAmount())) //��������Ϊ0
//			{
//				i_initer = workInputItemList.erase(i_initer); //ɾ�������
//				continue;
//			}else
//			{
//				i_initer++;
//			}
//		}
//	}//end workin
//	//����ǰ����������
//    YK_FLOAT ReationAmout=reationSpilthAmout+m_supplyAmout;
//    YKOrderPtr preOrder=workOutPtr->GetOrder(); 
//	if (NULL!=preOrder)   //�����������½���չ��
//	{
//        if (type==0)  //���������뽨������������������������
//        {
//			YK_FLOAT minBatch =itemPtr->GetSplitMinBatch();
//			if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4 )
//			{
//				minBatch = itemPtr->GetPurMinBatch();
//			}
//			if (Exist)//�ѽ�����
//			{
//               YK_FLOAT alReationAmout=reationSpilthAmout+m_supplyAmout;
//			   if (alReationAmout<minBatch)  //������С��������
//			   {
//				   preOrder->SetAmount(minBatch);
//				   workOutPtr->SetPlantAmount(minBatch);
//				   workOutPtr->SetSpilthAmount(minBatch-alReationAmout);
//				   preOrder->UpdateIOItem();
//			   }else
//			   {
//				   preOrder->SetAmount(minBatch);
//				   workOutPtr->SetPlantAmount(minBatch);
//				   workOutPtr->SetSpilthAmount(0);
//				   preOrder->UpdateIOItem();
//			   }
//			}else  //����������������
//			{
//               YK_FLOAT alReationAmout=reationSpilthAmout+m_supplyAmout;
//			   if (alReationAmout<minBatch)  //������С��������
//			   {
//				   preOrder->SetAmount(minBatch);
//				   workOutPtr->SetPlantAmount(minBatch);
//				   workOutPtr->SetSpilthAmount(minBatch-alReationAmout);
//				   preOrder->UpdateIOItem();
//			   }else
//			   {
//				   if (alReationAmout>0.0000001)  //�������еļ���
//				   {
//					   preOrder->SetAmount(alReationAmout);
//					   workOutPtr->SetPlantAmount(alReationAmout);
//					   workOutPtr->SetSpilthAmount(0);
//					   preOrder->UpdateIOItem();
//				   }
//			   }
//			}
//        }else //������һ������Ĺ���
//		{
//			preOrder->SetAmount(reationSpilthAmout+m_supplyAmout);
//			workOutPtr->SetPlantAmount(reationSpilthAmout+m_supplyAmout);
//			workOutPtr->SetSpilthAmount(0);
//			preOrder->UpdateIOItem();
//		}
//	}
//
//}//end workout
//
//
//
//
//
//

//����ʱ���ڼ�������С�������������ռ�����
void ApsCal::CollectGatherByMaxMinBatchAndOrderUniteDurTime(YKItemPtr&itemPtr,list<YKWorkInputItemPtr>&workInputItemList,list<YK_FLOAT>&couts)
{
	YK_FLOAT maxBatchAmout = itemPtr->GetSplitMaxBatch();
	YK_FLOAT minBatchAmout = itemPtr->GetSplitMinBatch();
	if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4 )
	{
		maxBatchAmout = itemPtr->GetPurMaxBatch();
		minBatchAmout = itemPtr->GetPurMinBatch();
	}

	YK_TIME_T startTime=0,durTime=itemPtr->GetOrderUniteDurTime().GetTime();
	if (durTime<=0) //��ʱ���ڼ����� 
	{
		SplitScarcityAmountByMaxMinBatch(itemPtr,workInputItemList,couts);
	}
	else
	{
		bool start=false;
        YK_TIME_T startTime=0/*,durTime=itemPtr->GetOrderUniteDurTime().GetTime()*/;
        YK_FLOAT difDur=0;
        list<YKWorkInputItemPtr>tempWorkInputItemList; 
		list<YKWorkInputItemPtr>calWorkInputItemList; 
        tempWorkInputItemList.insert(tempWorkInputItemList.end(),workInputItemList.begin(),workInputItemList.end());
		for (list<YKWorkInputItemPtr>::iterator i_iter=tempWorkInputItemList.begin();
			i_iter!=tempWorkInputItemList.end();)
		{
			YKWorkInputItemPtr&workin=*i_iter;
			if(BZERO(workin->GetScarcityAmount())) //��������Ϊ0
			{
				i_iter = tempWorkInputItemList.erase(i_iter); //ɾ�������
				continue;
			}
             YKOrderPtr nexOrder=(*i_iter)->GetOrder();
			 if (NULL!=nexOrder)
			 {
				 if (!start) //��õ�һ����ٽ���ʱ�̼���ֵ
				 {
					 start=true;
					 startTime=nexOrder->GetCalLastEndTime();
					 if (startTime<=0)
					 {
						 nexOrder->GetCalLastEndTimeOfNexOrder(startTime);  
					 }
				 }
				 YK_TIME_T curTime=nexOrder->GetCalLastEndTime();
				 if (curTime<=0)
				 {
					 nexOrder->GetCalLastEndTimeOfNexOrder(curTime);   
				 }
				 if (curTime-startTime<=durTime)
				 {
					 calWorkInputItemList.push_back(*i_iter);
					 i_iter = tempWorkInputItemList.erase(i_iter); //ɾ�������con
					 continue;
				 }else
				 {

					 SplitScarcityAmountByMaxMinBatchAndOrderUniteDurTime(itemPtr,calWorkInputItemList,couts,1,difDur);
					 calWorkInputItemList.clear();
					 startTime=nexOrder->GetCalLastEndTime();
					 if (startTime<=0)
					 {
						 nexOrder->GetCalLastEndTimeOfNexOrder(startTime);  
					 }
					 calWorkInputItemList.push_back(*i_iter);
					 i_iter = tempWorkInputItemList.erase(i_iter); //ɾ�������con
					 continue;
				 }
			 }
		}
		//if (!calWorkInputItemList.empty())
		 SplitScarcityAmountByMaxMinBatchAndOrderUniteDurTime(itemPtr,calWorkInputItemList,couts,0,difDur);
	}
}
//���������С�����ָ�
void ApsCal::SplitScarcityAmountByMaxMinBatch(YKItemPtr&itemPtr,list<YKWorkInputItemPtr>&workInputItemList,list<YK_FLOAT>&couts,int type)
{
	//YK_FLOAT maxBatchAmout = itemPtr->GetSplitMaxBatch();
	//YK_FLOAT minBatchAmout = itemPtr->GetSplitMinBatch();
	//YK_FLOAT batchUnitAmout = itemPtr->GetSplitBatchUnit();
	//if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4 )
	//{
	//	maxBatchAmout = itemPtr->GetPurMaxBatch();
	//	minBatchAmout = itemPtr->GetPurMinBatch();
	//	batchUnitAmout = itemPtr->GetPurBatchUnit();
	//}

	//YK_FLOAT scarcityTotalAmount=0;   //��������
	//YK_FLOAT scarcitylastAmount=0;
	//list<YKWorkInputItemPtr>::iterator i_initer=workInputItemList.begin();
	//for (;i_initer!=workInputItemList.end();)
	//{
	//	YKWorkInputItemPtr&workin=*i_initer;
	//	if(BZERO(workin->GetScarcityAmount())) //��������Ϊ0
	//	{
	//		i_initer = workInputItemList.erase(i_initer); //ɾ�������
	//		continue;
	//	}
	//	scarcityTotalAmount+=workin->GetScarcityAmount(); 
 //       if (i_initer==workInputItemList.end())
 //       {
	//		break;
 //       }else
	//		i_initer++;
	//	
	//}//end workin
	//if (type==0)  //������ʱ�仮��
	//{		
	//	//scarcityTotalAmount+=m_supplyAmout;   //modify 2011-2-16
	//	map<YK_ULONG,YK_DOUBLE>::iterator iter=m_minTarStckAmoutMap.find(itemPtr->GetId());
	//	if (iter!=m_minTarStckAmoutMap.end())
	//	{
	//		scarcityTotalAmount+=iter->second;
	//		//m_minTarStckAmoutMap.erase(iter);
	//	}
	//	//scarcityTotalAmount+=m_supplyAmoutMap[itemPtr->GetId()];
	//}
	//if (scarcityTotalAmount<PRECISION)
	//	return;
	//if (maxBatchAmout>0.000001)
	//{
	//	//����
	//	YK_FLOAT averAmout= maxBatchAmout ;  //floor(maxBatchAmout/batchUnitAmout)*batchUnitAmout;
	//	if (batchUnitAmout>0)
	//		averAmout= floor(maxBatchAmout/batchUnitAmout)*batchUnitAmout;

	//	if (averAmout<PRECISION)
	//	   averAmout=maxBatchAmout;
	//	YK_FLOAT tailAmout=0;
	//	if ( averAmout > scarcityTotalAmount  ) //scarcityTotalAmount < maxBatchAmout)
	//	{
	//		if (batchUnitAmout>0)
	//			tailAmout=ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
	//		else
	//			tailAmout=max(scarcityTotalAmount,minBatchAmout);

	//		couts.push_back(tailAmout);          
	//	} 
	//	else
	//	{
	//		while(scarcityTotalAmount - averAmout > PRECISION)
	//		{
	//			couts.push_back(averAmout);
	//			scarcityTotalAmount -= averAmout;
	//		}
	//		if (scarcityTotalAmount>PRECISION)
	//		{
	//			if (batchUnitAmout>0)
	//				tailAmout=ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
	//			else
	//				tailAmout=max(scarcityTotalAmount,minBatchAmout);
	//			//tailAmout=ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
	//			couts.push_back(tailAmout);
	//		}
	//	}
	//}
	//else   //�����������ֵ������
	//{
	//	YK_FLOAT tailAmout=0;
	//	if (batchUnitAmout>0)
	//		tailAmout=ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
	//	else
	//		tailAmout=max(scarcityTotalAmount,minBatchAmout);
	//	//tailAmout=ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
	//	couts.push_back(tailAmout);  
	//}

	//Add 2012-05-21 ���ݹ��ƥ�䴦��
	YK_FLOAT maxBatchAmout = itemPtr->GetSplitMaxBatch();
	YK_FLOAT minBatchAmout = itemPtr->GetSplitMinBatch();
	YK_FLOAT batchUnitAmout = itemPtr->GetSplitBatchUnit();
	if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4 )
	{
		maxBatchAmout = itemPtr->GetPurMaxBatch();
		minBatchAmout = itemPtr->GetPurMinBatch();
		batchUnitAmout = itemPtr->GetPurBatchUnit();
	}
	YK_FLOAT scarcityTotalAmount=0;   //��������
	YK_FLOAT scarcitylastAmount=0;
	YK_DOUBLE fAmount(0.0);	//�洢���ƥ��ֵ�� ����

	YK_SHORT  nIsStSpMd =  g_pBizModel->GetApsParam()->GetApsIsSDataSpecMt();
	if ( nIsStSpMd == 1 )
	{
		map<YK_WSTRING,YK_DOUBLE> specOrDataSpMap;	//�洢 ������ֵ�����ͬ��������¼
		list<YKWorkInputItemPtr>::iterator i_initer=workInputItemList.begin();
		for (;i_initer!=workInputItemList.end();)
		{
			//Add �洢 ������ֵ���ƥ�� ����ֵ
			YKWorkInputItemPtr& workin=*i_initer;
			if ( workin != NULL )
			{
				if(BZERO(workin->GetScarcityAmount())) //��������Ϊ0
				{
					i_initer = workInputItemList.erase(i_initer); //ɾ�������
					continue;
				}

				YKOrderPtr orderPtr = workin->GetOrder();
				if ( orderPtr != NULL )
				{
					YK_WSTRING& dataSpecWstr = orderPtr->GetSpecDataStr();
					YK_WSTRING& specWstr = orderPtr->GetSpecStr();				
					specOrDataSpMap[specWstr+dataSpecWstr] += workin->GetScarcityAmount();
				} 
				if (i_initer==workInputItemList.end())
				{
					break;
				}else
					i_initer++;
			}
		}
		if (type==0)  //������ʱ�仮��
		{	
			//Add ��ȡ ������
			for ( map<YK_WSTRING,YK_DOUBLE>::iterator fItor = specOrDataSpMap.begin(); fItor != specOrDataSpMap.end();fItor++ )
			{
				fAmount += fItor->second;
			}

			scarcityTotalAmount = fAmount;	//����������
			map<YK_ULONG,YK_DOUBLE>::iterator iter=m_minTarStckAmoutMap.find(itemPtr->GetId());
			if (iter!=m_minTarStckAmoutMap.end())
			{
				scarcityTotalAmount= fAmount +iter->second;
			}
		}
		if (scarcityTotalAmount<PRECISION)
			return;
		if (maxBatchAmout>0.000001)
		{
			//����
			YK_FLOAT averAmout= maxBatchAmout ;  //floor(maxBatchAmout/batchUnitAmout)*batchUnitAmout;
			if (batchUnitAmout>0)
				averAmout= floor(maxBatchAmout/batchUnitAmout)*batchUnitAmout;

			if (averAmout<PRECISION)
				averAmout=maxBatchAmout;
			YK_FLOAT tailAmout=0;
			if ( averAmout > scarcityTotalAmount  ) //scarcityTotalAmount < maxBatchAmout)
			{
				for ( map<YK_WSTRING,YK_DOUBLE>::iterator fItor = specOrDataSpMap.begin(); fItor != specOrDataSpMap.end();fItor++  )
				{
					YK_DOUBLE aCount = fItor->second;
					if ( batchUnitAmout>0 )
					{
						tailAmout=ceil(max(aCount,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
					}else
					{
						tailAmout=max(aCount,minBatchAmout);
					}
					couts.push_back(tailAmout);
				}

				couts.push_back(scarcityTotalAmount-fAmount);		//	�洢Ŀ����       
			} 
			else
			{
				while(scarcityTotalAmount - averAmout > PRECISION)
				{
					couts.push_back(averAmout);
					scarcityTotalAmount -= averAmout;
				}
				if (scarcityTotalAmount>PRECISION)
				{
					if (batchUnitAmout>0)
						tailAmout=ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
					else
						tailAmout=max(scarcityTotalAmount,minBatchAmout);
					couts.push_back(tailAmout);
				}
			}
		}
		else   //�����������ֵ������
		{
			for ( map<YK_WSTRING,YK_DOUBLE>::iterator fItor = specOrDataSpMap.begin(); fItor != specOrDataSpMap.end();fItor++  )
			{
				YK_FLOAT tailAmout=0;
				YK_DOUBLE dDemand = fItor->second;
				if (batchUnitAmout>0)
				{
					tailAmout = ceil(max(dDemand,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
				}
				else
					tailAmout=max(dDemand,minBatchAmout);	
				couts.push_back(tailAmout); 			
			}	
			if ( scarcityTotalAmount-fAmount >  PRECISION )
			{
				couts.push_back( scarcityTotalAmount-fAmount );
			}
		}
	}else
	{
		list<YKWorkInputItemPtr>::iterator i_initer=workInputItemList.begin();
		for (;i_initer!=workInputItemList.end();)
		{
			YKWorkInputItemPtr&workin=*i_initer;
			if(BZERO(workin->GetScarcityAmount())) //��������Ϊ0
			{
				i_initer = workInputItemList.erase(i_initer); //ɾ�������
				continue;
			}
			scarcityTotalAmount+=workin->GetScarcityAmount(); 
		       if (i_initer==workInputItemList.end())
		       {
				break;
		       }else
				i_initer++;
			
		}//end workin
		if (type==0)  //������ʱ�仮��
		{	
			m_minStckAmoutMap;
			map<YK_ULONG,YK_DOUBLE>::iterator iter=m_minTarStckAmoutMap.find(itemPtr->GetId());
			if (iter!=m_minTarStckAmoutMap.end())
			{
				scarcityTotalAmount+=iter->second;
			}
		}
		if (scarcityTotalAmount<PRECISION)
			return;
		if (maxBatchAmout>0.000001)
		{
			//����
			YK_FLOAT averAmout= maxBatchAmout ;  //floor(maxBatchAmout/batchUnitAmout)*batchUnitAmout;
			if (batchUnitAmout>0)
				averAmout= floor(maxBatchAmout/batchUnitAmout)*batchUnitAmout;

			if (averAmout<PRECISION)
			   averAmout=maxBatchAmout;
			YK_FLOAT tailAmout=0;
			if ( averAmout > scarcityTotalAmount  ) //scarcityTotalAmount < maxBatchAmout)
			{
				if (batchUnitAmout>0)
					tailAmout=ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
				else
					tailAmout=max(scarcityTotalAmount,minBatchAmout);

				couts.push_back(tailAmout);          
			} 
			else
			{
				while(scarcityTotalAmount - averAmout > PRECISION)
				{
					couts.push_back(averAmout);
					scarcityTotalAmount -= averAmout;
				}
				if (scarcityTotalAmount>PRECISION)
				{
					if (batchUnitAmout>0)
						tailAmout=ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
					else
						tailAmout=max(scarcityTotalAmount,minBatchAmout);
					couts.push_back(tailAmout);
				}
			}
		}
		else   //�����������ֵ������
		{
			YK_FLOAT tailAmout=0;
			if (batchUnitAmout>0)
				tailAmout=ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
			else
				tailAmout=max(scarcityTotalAmount,minBatchAmout);
			couts.push_back(tailAmout);  
		}
	}
}
//����ʱ�䡢�����С�����ָ�
void ApsCal::SplitScarcityAmountByMaxMinBatchAndOrderUniteDurTime(YKItemPtr&itemPtr,list<YKWorkInputItemPtr>&workInputItemList,list<YK_FLOAT>&couts,int type,YK_FLOAT&difDur)
{
	YK_FLOAT maxBatchAmout = itemPtr->GetSplitMaxBatch();
	YK_FLOAT minBatchAmout = itemPtr->GetSplitMinBatch();
	YK_FLOAT batchUnitAmout = itemPtr->GetSplitBatchUnit();
	if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4 )
	{
		maxBatchAmout = itemPtr->GetPurMaxBatch();
		minBatchAmout = itemPtr->GetPurMinBatch();
		batchUnitAmout = itemPtr->GetPurBatchUnit();
	}

	YK_FLOAT scarcityTotalAmount=0-difDur;
	YK_FLOAT scarcitylastAmount=0;
	list<YKWorkInputItemPtr>::iterator i_initer=workInputItemList.begin();
	for (;i_initer!=workInputItemList.end();)
	{
		YKWorkInputItemPtr&workin=*i_initer;
		if(BZERO(workin->GetScarcityAmount())) //��������Ϊ0
		{
			i_initer = workInputItemList.erase(i_initer); //ɾ�������
			continue;
		}
		scarcityTotalAmount+=workin->GetScarcityAmount(); 
		if (i_initer==workInputItemList.end())
		{
			break;
		}else
			i_initer++;

	}//end workin
	if (type==0)  //������ʱ�仮��
	{
		map<YK_ULONG,YK_DOUBLE>::iterator iter=m_minTarStckAmoutMap.find(itemPtr->GetId());
		if (iter!=m_minTarStckAmoutMap.end())
		{
			scarcityTotalAmount+=iter->second;
			//m_minTarStckAmoutMap.erase(iter);
		}
	}
	if (scarcityTotalAmount<PRECISION)  //��������С��0ʱ�����������Ѿ���������һ���Ĳ�������������
	{
		difDur = 0 - scarcityTotalAmount;

		return;
	}
	if (maxBatchAmout>0.000001)  //��Ҫ���������С����
	{
		//����
		YK_FLOAT averAmout =  maxBatchAmout;//floor(maxBatchAmout/batchUnitAmout)*batchUnitAmout;
		if (batchUnitAmout > 0)
			averAmout =max(maxBatchAmout ,(floor(maxBatchAmout/batchUnitAmout)*batchUnitAmout));

		while(scarcityTotalAmount - averAmout > PRECISION)
		{
			couts.push_back(averAmout);
			scarcityTotalAmount -= averAmout;
		}
		if (scarcityTotalAmount>PRECISION)
		{
			//if(type == 0)   //���һ����Ҫ������С����
			{
				if (batchUnitAmout > 0)
					averAmout=ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
				else
					averAmout=max(scarcityTotalAmount,minBatchAmout);
			}
			couts.push_back(averAmout);
			difDur=averAmout-scarcityTotalAmount;
		}

	}
	else
	{
		YK_FLOAT tailAmout=0;
		if (batchUnitAmout > 0)
			tailAmout=ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout;
		else
			tailAmout=max(scarcityTotalAmount,minBatchAmout);

		couts.push_back(tailAmout);  
		difDur=tailAmout-scarcityTotalAmount;
	}
}
void ApsCal::MatchInAndOut(YKItemPtr& itemPtr,list<YKWorkInputItemPtr>&workInputItemList,list<YKWorkOutputItemPtr>& workOutputItemList2
						   ,list<YK_FLOAT>&couts)
{
	list<YKWorkOutputItemPtr>newWorkOutList;
	bool exist=false;
	for (list<YK_FLOAT>::iterator iter=couts.begin();
		iter!=couts.end();)
	{
		for(list<YKWorkOutputItemPtr>::iterator iterOut =workOutputItemList2.begin()
			;iterOut!=workOutputItemList2.end();)
		{
			YKWorkOutputItemPtr& workOutItem = *iterOut;
			if (workOutItem->GetSpilthAmount() < 0.00001)
			{
				iterOut = workOutputItemList2.erase(iterOut);
				continue;
			}
			//����ǰ����������
			YKOrderPtr preOrder=workOutItem->GetOrder(); 
			if (NULL!=preOrder)   //�����������½���չ��
			{
				YK_FLOAT oldAmout=preOrder->GetAmount();
				preOrder->SetAmount(*iter);
				workOutItem->SetPlantAmount(*iter);
				workOutItem->SetSpilthAmount(*iter);
				preOrder->OrderLateFinishTimeCalValue();
				if(oldAmout!=preOrder->GetAmount())
				{
					if (preOrder->GetType() == OrderType_StockOrder 
						&&preOrder->GetDiffer()==OrderDiffer_Auto) //�ɹ�����
					{
						AddUpdataAutoStockOrderList(preOrder->GetId());
					}else if (preOrder->GetType() == OrderType_PudOrder 
						&&preOrder->GetDiffer()==OrderDiffer_Auto)  //���춨��
					{
						AddUpdataAutoPudOrderList(preOrder->GetId());
					}
				}

				newWorkOutList.push_back(workOutItem);
				iterOut = workOutputItemList2.erase(iterOut);
				exist=true;
				break;
			}
		}
		if (exist)
		{
			exist=false;
			iter=couts.erase(iter);
			if (iter!=couts.end())
			{
				continue; 
			}else
				break;
		}else
			break;
	}

	CLOCK_PASE(T_NewCreateRela2)
	list<YKWorkOutputItemPtr>::iterator o_iter =newWorkOutList.begin();
	for (;o_iter!=newWorkOutList.end();o_iter++)
	{
		YKWorkOutputItemPtr&workOut =*o_iter;
		YKOrderPtr preOrder=workOut->GetOrder();
		YK_FLOAT totalAmout=preOrder->GetAmount();
		YK_FLOAT scarcityAmount=0;
		YK_FLOAT lastscarcityAmount=0;
		preOrder->OrderLateFinishTimeCalValue();
		YKWorkOutputItemPtr curWorkOutPtr=NULL;
		list<YKWorkInputItemPtr>::iterator i_iter =workInputItemList.begin();
		for (int i=0;i_iter!=workInputItemList.end();i++)
		{
			YKWorkInputItemPtr&workin=*i_iter;
			if(BZERO(workin->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
			{
				i_iter = workInputItemList.erase(i_iter); //ɾ�������
				i--;
			    continue;
			}
			 YKOrderPtr nexOrder=workin->GetOrder();
			 if (NULL!=preOrder&&NULL!=nexOrder)    //���ݹ�������깤ʱ�̼���ֵ
			 {
				 preOrder->PreOrderLateFinishTimeCalValue(nexOrder);   
			 }
			 if (i==0)     //�����յ�һ���ϲ㶨����������չ��
			 {
				 //������Ϣ����
				 if (!preOrder->JustIfHaveTopOrder())
				 {
					 InforTransfor Trans;
					 Trans.OrderInfoTransfor(nexOrder,preOrder);
				 }
				 preOrder->SetAboveOrderId( nexOrder->GetId());
				 preOrder->UpdateIOItem();
				 preOrder->SetAboveOrderId( 0);
				 preOrder->FlashOrderState();
                 curWorkOutPtr=YKWorkOutputItem::FindOrderOutItem(preOrder);
				 if(preOrder->GetExcptFlg()!=1)
				 {
					 //���������Ϣ
					 if (preOrder->GetExcptFlg()!=1)
					 {
					 	if (preOrder->GetExcptFlg()==Spread_No_Bom)
					 		AddFailItemBomList( itemPtr->GetId());    //���һ��BOM��Ч��Ϣ
					 	if (preOrder->GetType() == OrderType_PudOrder )  //���춨��
					 	{
					 		AddFialPudOrderList(preOrder->GetId());
					 	}
					 }
					 preOrder->Delete();
					 break;
				 }
				 //������Ϣ����
				 if (!preOrder->JustIfHaveTopOrder())
				 {
					 //������Ϣ����
					 InforTransfor Trans;
					 Trans.WorkInforTransfor(preOrder);
				 }
			 }

			 lastscarcityAmount=scarcityAmount;
			 scarcityAmount+=workin->GetScarcityAmount();
			 if (scarcityAmount<totalAmout)    //һ�������Ӧ�������
			 {
				 if (NULL!=workin&&NULL!=curWorkOutPtr&&workin->GetScarcityAmount()>0)
				    YKWorkRelation::CreatNew(workin ,curWorkOutPtr,RelationType_Differ,workin->GetScarcityAmount());
                if (NULL!=preOrder)
                {
					if(preOrder->GetCode().empty())
                      preOrder->CreateOrderCode();
                }
			 }else
			 {
                 if (scarcityAmount-lastscarcityAmount>PRECISION)   //һ�������Ӧ������ 
                 {
					  if (NULL!=workin&&NULL!=curWorkOutPtr&&((totalAmout-lastscarcityAmount)>0))
					 YKWorkRelation::CreatNew(workin ,curWorkOutPtr,RelationType_Differ,totalAmout-lastscarcityAmount);
					 if (NULL!=preOrder)
					 {
						 if(preOrder->GetCode().empty())
							 preOrder->CreateOrderCode();
					 }
                 }
				 break;
			 }

			 if(BZERO(workin->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
			 {
				 i_iter = workInputItemList.erase(i_iter); //ɾ�������
				 i--;
				 continue;
			 }else
			 {
				 if (BZERO(curWorkOutPtr->GetSpilthAmount()))
				 {
					 break;
				 }

			 }
		}//end for workin
		if (NULL!=preOrder)
		{
			if(preOrder->GetCode().empty())
				preOrder->CreateOrderCode();
		}
	}
	CLOCK_PASE(T_NewCreateRela2)
	CreatePreOrder(itemPtr,couts,workInputItemList);
	SpreadMuster(newWorkOutList);
	AllotMinTargetStock(itemPtr,newWorkOutList);
	//AllotMinStock(workOutputItemList2);// ������2��ʣ��������Ʒ
}

//Add 2012-06-07 ���������������ϵ
void ApsCal::MatchInAndOutRel(YKItemPtr& itemPtr,list<YKWorkInputItemPtr>&workInputItemList,list<YKWorkOutputItemPtr>& workOutputItemList2,list<YK_FLOAT>&couts)
{
	list<YKWorkOutputItemPtr>newWorkOutList;
	
	//������
	map<YK_WSTRING,list<YKWorkInputItemPtr>> workInputObjMap;
	for ( list<YKWorkInputItemPtr>::iterator wiItor =workInputItemList.begin(); wiItor != workInputItemList.end();wiItor++ )
	{
		YKWorkInputItemPtr& winPtr = *wiItor;
		if ( winPtr != NULL )
		{
			YKOrderPtr& orderPtr = winPtr->GetOrder();
			if ( orderPtr != NULL )
			{
				YK_WSTRING& specValue = orderPtr->GetSpecStr();
				YK_WSTRING& dataSpecValue = orderPtr->GetSpecDataStr();
				workInputObjMap[specValue+dataSpecValue].push_back(winPtr);
			}
		}
	}
	list<YKWorkOutputItemPtr>::iterator o_iter =newWorkOutList.begin();
	for (;o_iter!=newWorkOutList.end();o_iter++)
	{
		YKWorkOutputItemPtr&workOut =*o_iter;
		YKOrderPtr preOrder=workOut->GetOrder();
		YK_FLOAT totalAmout=preOrder->GetAmount();
		YK_FLOAT scarcityAmount=0;
		YK_FLOAT lastscarcityAmount=0;
		preOrder->OrderLateFinishTimeCalValue();
		YKWorkOutputItemPtr curWorkOutPtr=NULL;

		if ( preOrder != NULL )
		{
			YK_WSTRING& orderSpec = preOrder->GetSpecStr();
			YK_WSTRING& orderDataSpec = preOrder->GetSpecDataStr();

			//�������
			map<YK_WSTRING,list<YKWorkInputItemPtr>>::iterator fItor = workInputObjMap.find( orderSpec+orderDataSpec );
			if ( fItor != workInputObjMap.end() )
			{
				list<YKWorkInputItemPtr>& workInputPtrList = fItor->second;
			
				list<YKWorkInputItemPtr>::iterator i_iter =workInputPtrList.begin();
				for (int i=0;i_iter!=workInputPtrList.end();i++)
				{
					YKWorkInputItemPtr&workin=*i_iter;
					if(BZERO(workin->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
					{
						i_iter = workInputPtrList.erase(i_iter); //ɾ�������
						i--;
						continue;
					}
					YKOrderPtr nexOrder=workin->GetOrder();
					if (NULL!=preOrder&&NULL!=nexOrder)    //���ݹ�������깤ʱ�̼���ֵ
					{
						preOrder->PreOrderLateFinishTimeCalValue(nexOrder);   
					}
					if (i==0)     //�����յ�һ���ϲ㶨����������չ��
					{
						//������Ϣ����
						if (!preOrder->JustIfHaveTopOrder())
						{
							InforTransfor Trans;
							Trans.OrderInfoTransfor(nexOrder,preOrder);
						}
						preOrder->SetAboveOrderId( nexOrder->GetId());
						preOrder->UpdateIOItem();
						preOrder->SetAboveOrderId( 0);
						preOrder->FlashOrderState();
						curWorkOutPtr=YKWorkOutputItem::FindOrderOutItem(preOrder);
						if(preOrder->GetExcptFlg()!=1)
						{
							//���������Ϣ
							if (preOrder->GetExcptFlg()!=1)
							{
								if (preOrder->GetExcptFlg()==Spread_No_Bom)
									AddFailItemBomList( itemPtr->GetId());    //���һ��BOM��Ч��Ϣ
								if (preOrder->GetType() == OrderType_PudOrder )  //���춨��
								{
									AddFialPudOrderList(preOrder->GetId());
								}
							}
							preOrder->Delete();
							break;
						}
						//������Ϣ����
						if (!preOrder->JustIfHaveTopOrder())
						{
							//������Ϣ����
							InforTransfor Trans;
							Trans.WorkInforTransfor(preOrder);
						}
					}

					lastscarcityAmount=scarcityAmount;
					scarcityAmount+=workin->GetScarcityAmount();
					if (scarcityAmount<totalAmout)    //һ�������Ӧ�������
					{
						if (NULL!=workin&&NULL!=curWorkOutPtr&&workin->GetScarcityAmount()>0)
							YKWorkRelation::CreatNew(workin ,curWorkOutPtr,RelationType_Differ,workin->GetScarcityAmount());
						if (NULL!=preOrder)
						{
							if(preOrder->GetCode().empty())
								preOrder->CreateOrderCode();
						}
					}else
					{
						if (scarcityAmount-lastscarcityAmount>PRECISION)   //һ�������Ӧ������ 
						{
							if (NULL!=workin&&NULL!=curWorkOutPtr&&((totalAmout-lastscarcityAmount)>0))
								YKWorkRelation::CreatNew(workin ,curWorkOutPtr,RelationType_Differ,totalAmout-lastscarcityAmount);
							if (NULL!=preOrder)
							{
								if(preOrder->GetCode().empty())
									preOrder->CreateOrderCode();
							}
						}
						break;
					}

					if(BZERO(workin->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
					{
						i_iter = workInputPtrList.erase(i_iter); //ɾ�������
						i--;
						continue;
					}else
					{
						if (BZERO(curWorkOutPtr->GetSpilthAmount()))
						{
							break;
						}

					}
				}//end for workin
				if (NULL!=preOrder)
				{
					if(preOrder->GetCode().empty())
						preOrder->CreateOrderCode();
				}
			}
		}
	}	

	CreatePreOrderBySpec(itemPtr,couts,workInputItemList);	// ���ݹ��ƥ����� ���鴦��
	SpreadMuster(newWorkOutList);
	AllotMinTargetStock(itemPtr,newWorkOutList);
}
//�ȹ�������21
void ApsCal::PriorityReationGather21(list<YKWorkInputItemPtr>&workInputItemList,list<YKWorkOutputItemPtr>& workOutputItemList21)
{
	list<YKWorkInputItemPtr>::iterator i_iter =workInputItemList.begin();
	for (;i_iter!=workInputItemList.end();)
	{
		YKWorkInputItemPtr&workin=*i_iter;
		if(BZERO(workin->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
		{
			i_iter = workInputItemList.erase(i_iter); //ɾ�������
			continue;
		}

		//����1ȡһ������������¼
		for(list<YKWorkOutputItemPtr>::iterator iterOut =workOutputItemList21.begin()
			;iterOut!=workOutputItemList21.end();)
		{
			YKWorkOutputItemPtr& workOutItem = *iterOut;

			if(workOutItem->GetSpilthAmount() < 0.00001)
			{
				iterOut = workOutputItemList21.erase(iterOut);
				continue;
			}
            if (workin->GetInputItemId()!=workOutItem->GetOutputItemId())
            {
				iterOut++;
				continue;
            }
			YKOrderPtr nexOrder=workin->GetOrder();

            YKOrderPtr preOrder=workOutItem->GetOrder();

			if (NULL!=preOrder&&NULL!=nexOrder)    //���ݹ�������깤ʱ�̼���ֵ
			{
				preOrder->PreOrderLateFinishTimeCalValue(nexOrder);   
				preOrder->UpdateIOItem();
			}
			YK_FLOAT reationAmout=min(workOutItem->GetSpilthAmount(),workin->GetScarcityAmount());
			if (reationAmout>0)
			{
				if (!FindTheSameReationByWorkInAndWorkOut(reationAmout, workin,workOutItem)) 
					YKWorkRelation::CreatNew(workin ,workOutItem,RelationType_Differ,reationAmout);
			}
			if(BZERO(workOutItem->GetSpilthAmount())) //��������Ϊ0
			{
				iterOut = workOutputItemList21.erase(iterOut); //ɾ�������

				if(BZERO(workin->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
					break;
				else
					continue;
			}
			else  //�����Ʒ����������Ϊ0��������Ʒ����������Ϊ0
				break;
		}
		if(BZERO(workin->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
		{
			i_iter = workInputItemList.erase(i_iter); //ɾ�������
			continue;
		}else
			break;
	}
	if (workInputItemList.empty())
	{
		//����1ȡһ������������¼
		for(list<YKWorkOutputItemPtr>::iterator iterOut =workOutputItemList21.begin()
			;iterOut!=workOutputItemList21.end();)
		{
			YKWorkOutputItemPtr& workOutItem = *iterOut;

			if(workOutItem->GetSpilthAmount() < 0.00001)
			{
				iterOut = workOutputItemList21.erase(iterOut);
				continue;
			}
			m_supplyAmout=max(0,m_supplyAmout-max(0,min(m_supplyAmout,workOutItem->GetSpilthAmount()))); 
			//m_supplyAmoutMap[workOutItem->GetOutputItemId()]=max(0,m_supplyAmoutMap[workOutItem->GetOutputItemId()]-max(0,min(m_supplyAmoutMap[workOutItem->GetOutputItemId()],workOutItem->GetSpilthAmount()))); ;
           if (iterOut==workOutputItemList21.end())
           {
			   break;
           }
		   else
               iterOut++;
		}
	}
}
bool ApsCal::ReationAutoSupply(list<YKWorkRelationPtr>&inReationList)
{
	list<YKWorkRelationPtr>::iterator iter=inReationList.begin();
	for (;iter!=inReationList.end();iter++)
	{
        YKWorkRelationPtr&workRelationPtr =*iter;
		if (NULL!=workRelationPtr)
		{
           YKOrderPtr preOrder= workRelationPtr->GetPreOrderPtr();
		   if (workRelationPtr->GetRelaTye()!=RelationType_Same&&NULL!=preOrder)
		   { 
              if (preOrder->GetDiffer()==OrderDiffer_Auto)
              {
                 list<YKWorkPtr>workList;
					 preOrder->GetWorkPtrList(workList);
					 list<YKWorkPtr>::iterator i_iter=workList.begin();
					 for (;i_iter!=workList.end();i_iter++)
					 {
                          YKWorkPtr&work=*i_iter;
						  if (NULL!=work&&work->GetState()>WorkState_PlantDone)
						  {
                           return false;
						  }
					 }
                         return true;

              }
		   }
		}

	}
	      return false;
}
//��1��1��ʽ��ʽ�°�����������ָ�󲹳�
void ApsCal::EquitySupplyByMaxBatchSplit(YKItemPtr&itemPtr,YKWorkInputItemPtr&workInItemPtr,list<YK_FLOAT>&couts)
{
	YK_FLOAT maxBatchAmout = itemPtr->GetSplitMaxBatch();
	YK_FLOAT minBatchAmout = itemPtr->GetSplitMinBatch();
	YK_FLOAT batchUnitAmout = itemPtr->GetSplitBatchUnit();
	if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4 )
	{
		maxBatchAmout = itemPtr->GetPurMaxBatch();
		minBatchAmout = itemPtr->GetPurMinBatch();
		batchUnitAmout = itemPtr->GetPurBatchUnit();
	}
	if (batchUnitAmout<=PRECISION)
	{
		batchUnitAmout=1;
	}
	YK_FLOAT scarcityTotalAmount=workInItemPtr->GetScarcityAmount();   //��������
	if (scarcityTotalAmount<PRECISION)
		return;
	if (maxBatchAmout>PRECISION)
	{
		//����
		YK_FLOAT averAmout=maxBatchAmout/*floor(maxBatchAmout/batchUnitAmout)*batchUnitAmout*/;
		if (averAmout<PRECISION)
			averAmout=maxBatchAmout;
		YK_FLOAT tailAmout=0;
		if (scarcityTotalAmount < maxBatchAmout)
		{
			tailAmout=scarcityTotalAmount/*ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout*/;
			couts.push_back(tailAmout);          
		} 
		else
		{
			while(scarcityTotalAmount - averAmout > PRECISION)
			{
				couts.push_back(averAmout);
				scarcityTotalAmount -= averAmout;
			}
			if (scarcityTotalAmount>PRECISION)
			{
				tailAmout=scarcityTotalAmount/*ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout*/;
				couts.push_back(tailAmout);
			}
		}
	}else   //�����������ֵ������
	{
		YK_FLOAT tailAmout=0;
		tailAmout=scarcityTotalAmount/*ceil(max(scarcityTotalAmount,minBatchAmout)/batchUnitAmout)*batchUnitAmout*/;
		couts.push_back(tailAmout);  
	}
}
//������Զ����䶩���Ĺ�������֮��
void ApsCal::CalReationAmoutForHandOrStateOrder(YKItemPtr&itemPtr,YKWorkInputItemPtr&workInItemPtr,YK_FLOAT& reationAmout)
{
   if (itemPtr->GetAutoSupply()<AutoFlag_YesStOne) //���ǣ����+1��1���죩����ǣ��������1��1���죩
   {
      return;
   }
   list<YKWorkRelationPtr>reationlist;
	   workInItemPtr->GetRelationPtrList(reationlist);
	   for (list<YKWorkRelationPtr>::iterator iter=reationlist.begin();
		   iter!=reationlist.end();iter++)
	   {
             YKWorkRelationPtr&workReation=*iter;
			 if (NULL!=workReation)
			 {
                 if (workReation->GetRelaTye()!=RelationType_Same)
                 {
                      YKOrderPtr preOrder=workReation->GetPreOrderPtr();
					  if (NULL!=preOrder&&preOrder->GetDiffer()==OrderDiffer_Auto)
					  {
                          if (!preOrder->ReationFilterByState())
                          {
                              reationAmout+=workReation->GetAmount();
                          }
					  }
                 }
			 }
	   }
}
//�������º���ж��������Ĺ�������ٷŽ������Ʒ����2��21
void ApsCal::SaveWorkOutGather(YKWorkOutputItemPtr&workOutPtr,list<YKWorkOutputItemPtr>& workOutputItemList2,list<YKWorkOutputItemPtr>& workOutputItemList21)
{ 
	if (workOutPtr->GetPlantAmount()!=workOutPtr->GetSpilthAmount())  //�����ѷ�����
	{
		bool Exsit=false;
		list<YKWorkOutputItemPtr>::iterator o_iter=workOutputItemList2.begin();
		for (;o_iter!=workOutputItemList2.end();o_iter++)
		{
			YKWorkOutputItemPtr&workOut=*o_iter;
			if (NULL!=workOut&&workOut->GetId()==workOutPtr->GetId())
			{
				Exsit=true;
				break;
			}
		}
		list<YKWorkOutputItemPtr>::iterator o_iter2=workOutputItemList21.begin();
		for (;o_iter2!=workOutputItemList21.end();o_iter2++)
		{
             YKWorkOutputItemPtr&workOut=*o_iter2;
			 if (NULL!=workOut&&workOut->GetId()==workOutPtr->GetId())
			 {
                 Exsit=true;
				 break;
			 }
		}
		if (!Exsit)
		{
			workOutputItemList21.push_back(workOutPtr);
			workOutputItemList21.sort(SortWorkOutPutList);
		}
	}else
	{
		bool Exsit=false;
		list<YKWorkOutputItemPtr>::iterator o_iter=workOutputItemList2.begin();
		for (;o_iter!=workOutputItemList2.end();o_iter++)
		{
			YKWorkOutputItemPtr&workOut=*o_iter;
			if (NULL!=workOut&&workOut->GetId()==workOutPtr->GetId())
			{
				//workOutputItemList2.erase(o_iter);
				Exsit=true;
				break;
			}
		}
		list<YKWorkOutputItemPtr>::iterator o_iter2=workOutputItemList21.begin();
		for (;o_iter2!=workOutputItemList21.end();o_iter2++)
		{
			YKWorkOutputItemPtr&workOut=*o_iter2;
			if (NULL!=workOut&&workOut->GetId()==workOutPtr->GetId())
			{
				workOutputItemList21.erase(o_iter2);
				//Exsit=true;
				break;
			}
		}
		if (!Exsit)
		{
			workOutputItemList2.push_back(workOutPtr);
			workOutputItemList2.sort(SortWorkOutPutList);
		}
	}
}
//���Ҷ�Ӧ����������ö�Ӧ�����Ѿ����ڣ���ֻ������������ԭ����������
//����ֵtrue:����
bool ApsCal::FindTheSameReationByWorkInAndWorkOut(YK_FLOAT&newAmout, YKWorkInputItemPtr&workInPtr,YKWorkOutputItemPtr&workOutPtr)
{
	list<YKWorkRelationPtr>relationList;
    workInPtr->GetRelationPtrList(relationList);
	for (list<YKWorkRelationPtr>::iterator itor=relationList.begin()
		;itor!=relationList.end();itor++)
	{
		YKWorkRelationPtr&workReation=*itor;
		if (NULL!=workReation)
		{ 
			YKWorkInputItemPtr&curWorkInPtr=workReation->GetWorkInputItemPtr();
			YKWorkOutputItemPtr&curWorkOutPtr=workReation->GetWorkOutputItemPtr();
			if (NULL!=curWorkInPtr&&NULL!=curWorkOutPtr)
			{
				if (curWorkInPtr->GetId()==workInPtr->GetId()
					&&curWorkOutPtr->GetId()==workOutPtr->GetId())
				{
					workReation->SetAmount(workReation->GetAmount()+newAmout);
					curWorkInPtr->SetScarcityAmount(curWorkInPtr->GetScarcityAmount()-newAmout);
					curWorkOutPtr->SetSpilthAmount(curWorkOutPtr->GetSpilthAmount()-newAmout);
					return true;
				}
			}
		}
	}
	return false;
}
//��Ϣ�����Լ���������
//����������Ϣ����
//void ApsCal::InforransferAndOrderRenovate(YKWorkInputItemPtr&workInPtr,YKWorkOutputItemPtr&workOutPtr)
//{
//	YKOrderPtr preOrder=workOutPtr->GetOrder();
//	YKOrderPtr nexOrder=workInPtr->GetOrder();
//	if (NULL==preOrder||NULL==nexOrder)
//        return;
//	if (preOrder->GetDiffer()==OrderDiffer_Auto)
//	{
//		if (NULL!=workOutPtr&&workOutPtr->GetIsMainProduction())
//		{
//			//������Ϣ����     
//			InforTransfor Trans;
//            Trans.OrderInfoTransfor(nexOrder,preOrder);
//			if (preOrder->ReationFilterByState()) //������Ϣ����
//			{
//				Trans.WorkInforTransfor(preOrder);
//			}else
//                Trans.WorkInforTransfor(preOrder);
//		}
//	}
//}
//������������չ������������˳������������������ϵ���������ݶ�����������Ϣ 
void ApsCal::SortReationByTimeInWorkInput(list<YKWorkRelationPtr>&workRelationList)
{
	list<YKWorkRelationPtr>tempWorkRelationList;
	list<YKWorkRelationPtr>willOrderWorkRelationList;
	//���չ���ʵ�ʿ�������
	list<YKWorkRelationPtr>::iterator iter = workRelationList.begin();
	while(iter!=workRelationList.end())
	{
		YKWorkRelationPtr&workReationPtr=*iter;
		if (NULL==workReationPtr)
		{
			iter = workRelationList.erase(iter);
			continue;
		}
		if (workReationPtr->GetRelaTye()==RelationType_Same)
		{
			//iter = workRelationList.erase(iter);
			return ;
		}
		YKWorkInputItemPtr&curinPutPtr=workReationPtr->GetWorkInputItemPtr();
		if (NULL==curinPutPtr)
		{
			iter = workRelationList.erase(iter);
			continue;
		}
		YKOrderPtr order= workReationPtr->GetNextOrderPtr();
		if (NULL==order)
		{
			iter = workRelationList.erase(iter);
			continue;
		}
		YKWorkPtr nexWork = workReationPtr->GetNextWorkPtr();
		if(nexWork == NULL)	
		{
			iter = workRelationList.erase(iter);
			continue;
		}
		YKOrderPtr peakOrder = order->GetPeakOrder();

		if(peakOrder == NULL)	peakOrder = order;
		if (!peakOrder->WillOrderByPriorityBorderLine()) //С�ڸ������ȼ������򶩵�
		{
			willOrderWorkRelationList.push_back(*iter);
			iter = workRelationList.erase(iter);
			continue;
		}

		if (nexWork->GetSchFlag()==SchFlg_WaitSch)
		{
			iter++;
			continue;
		}
		if( nexWork->GetPlantStartTm() > 0)
		{
			list<YKWorkRelationPtr>::iterator iterPS = tempWorkRelationList.begin();
			for(;iterPS != tempWorkRelationList.end();)
			{
				YKWorkPtr workPS = (*iterPS)->GetNextWorkPtr();
				//if(workPS == NULL)	break;;

				if(nexWork->GetPlantStartTm() < workPS->GetPlantStartTm())
				{
					tempWorkRelationList.insert(iterPS,*iter);
					break;
				}
				iterPS++;
			}
			if(iterPS == tempWorkRelationList.end())
				tempWorkRelationList.push_back(*iter);

			iter = workRelationList.erase(iter);
			continue;
		}
		iter++;
	}

	//���ն����ƻ�����ʱ������
	iter = workRelationList.begin();
	while(iter!=workRelationList.end())
	{
		YKWorkRelationPtr&workReationPtr=*iter;
		if (NULL==workReationPtr)
		{
			iter = workRelationList.erase(iter);
			continue;
		}
		YKOrderPtr order= workReationPtr->GetNextOrderPtr();
		if (NULL==order)
		{
			iter = workRelationList.erase(iter);
			continue;
		}
		YKOrderPtr peakOrder = order->GetPeakOrder();

		if(peakOrder == NULL)	peakOrder = order;
		if (!peakOrder->WillOrderByPriorityBorderLine()) //С�ڸ������ȼ������򶩵�
		{
			willOrderWorkRelationList.push_back(*iter);
			iter = workRelationList.erase(iter);
			continue;
		}
		if(  order->GetPlanStartTime() > 0)
		{
			list<YKWorkRelationPtr>::iterator iterOS = tempWorkRelationList.begin();
			for(;iterOS != tempWorkRelationList.end();)
			{
				YKOrderPtr orderOS = (*iterOS)->GetNextOrderPtr(); 
				/*if(orderOS == NULL)	break;*/

				if(orderOS->GetPlanStartTime() > order->GetPlanStartTime())
				{
					tempWorkRelationList.insert(iterOS,*iter);
					break;
				}
				iterOS++;
			}
			if(iterOS == tempWorkRelationList.end())
				tempWorkRelationList.push_back(*iter);

			iter = workRelationList.erase(iter);
			continue;
		}

		iter++;
	}
	//���ն��������깤ʱ�̼���ֵ����
	iter = workRelationList.begin();
	while(iter!=workRelationList.end())
	{
		YKWorkRelationPtr&workReationPtr=*iter;
		if (NULL==workReationPtr)
		{
			iter = workRelationList.erase(iter);
			continue;
		}
		YKOrderPtr order= workReationPtr->GetNextOrderPtr();
		if (NULL==order)
		{
			iter = workRelationList.erase(iter);
			continue;
		}
		YKOrderPtr peakOrder = order->GetPeakOrder();
		if(peakOrder == NULL)	peakOrder = order;
		if (!peakOrder->WillOrderByPriorityBorderLine()) //С�ڸ������ȼ������򶩵�
		{
			willOrderWorkRelationList.push_back(*iter);
			iter = workRelationList.erase(iter);
			continue;
		}
		list<YKWorkRelationPtr>::iterator iterOC = tempWorkRelationList.begin();
		for(;iterOC != tempWorkRelationList.end();)
		{
			YKOrderPtr orderOS = (*iterOC)->GetNextOrderPtr(); 
			/*	if(orderOS == NULL)	break;*/

			if(orderOS->GetCalLastEndTime() > order->GetCalLastEndTime())
			{
				tempWorkRelationList.insert(iterOC,*iter);
				break;
			}
			iterOC++;
		}
		if(iterOC == tempWorkRelationList.end())
			tempWorkRelationList.push_back(*iter);

		iter = workRelationList.erase(iter);
	}
	willOrderWorkRelationList.sort(SortWillOrderReationByTimeInWorkIn);
	tempWorkRelationList.insert(tempWorkRelationList.end(),willOrderWorkRelationList.begin(),willOrderWorkRelationList.end());
	workRelationList = tempWorkRelationList;
 	for (list<YKWorkRelationPtr>::iterator r_iter=workRelationList.begin();
 		r_iter!=workRelationList.end();r_iter++)
     {
 		YKWorkRelationPtr&workReationPtr=*r_iter;
 		YKWorkInputItemPtr&curinPutPtr=workReationPtr->GetWorkInputItemPtr();
 		YKWorkOutputItemPtr&curoutPutPtr=workReationPtr->GetWorkOutputItemPtr();
         //���Ѵ��ڵĹ��������ж���Ϣ����
 		InforransferAndOrderRenovate(curinPutPtr,curoutPutPtr);
 		return;
     }
}
//�������ͨ����������
bool ApsCal::SortWillOrderReationByTimeInWorkIn(YKWorkRelationPtr&first,YKWorkRelationPtr&second)
{
	YKOrderPtr firstOrder=first->GetNextOrderPtr();
	YKOrderPtr secondOrder=second->GetNextOrderPtr();
	YKOrderPtr firstpeakOrder = firstOrder->GetPeakOrder();
	if(firstpeakOrder == NULL)	firstpeakOrder = firstOrder;
	YKOrderPtr secondpeakOrder = secondOrder->GetPeakOrder();
	if(secondpeakOrder == NULL)	secondpeakOrder = secondOrder;
	if (NULL!=firstpeakOrder&&NULL!=secondpeakOrder)
	{ 
		if(firstpeakOrder->GetCalLastEndTime() < secondpeakOrder->GetCalLastEndTime())
			return true;
		else 
			return false;
	}
	return false;
}
//�ж�ǰ���������Ƿ���Ҫ����
bool ApsCal::IfUpdateForPreOrder(YKItemPtr& itemPtr,YKWorkInputItemPtr&workIn ,YKWorkOutputItemPtr&workOut)
{
	if (!BZERO(workOut->GetSpilthAmount()))
	{
		return true;
	}
	if (workIn->GetScarcityAmount()<0)
	{
		return true;
	}
	if (workIn->GetScarcityAmount()>0)
	{
		if (itemPtr->GetAutoSupply()>AutoFlag_Yes)
		{
			return true; 
		}
		YKOrderPtr nexOrder =workIn->GetOrder();
		/*YKOrderPtr peakOrder = nexOrder->GetPeakOrder();

		if(peakOrder == NULL)	peakOrder = nexOrder;*/

		if(/*peakOrder != NULL
			&&(peakOrder->GetDesignation() != L""
			||peakOrder->GetType() == OrderType_WillOrder)*/
			nexOrder->PeakDesignationOrders()
			||nexOrder->PeakWillOrders()) //�������췬�Ż����򶩵�
		{
			return true;
		}
	}

		YK_FLOAT maxBatchAmout = itemPtr->GetSplitMaxBatch();
		if(itemPtr->GetStockMode() == 3 || itemPtr->GetStockMode() == 4 )
		{
			maxBatchAmout = itemPtr->GetPurMaxBatch();
		}
		if (maxBatchAmout>0)   //0��-1��ʾ�����
		{
			YKOrderPtr preOrderPtr=workOut->GetOrder(); 
			if (preOrderPtr->GetAmount()>maxBatchAmout)
			{
				if (g_pBizModel->GetApsParam()->GetInPOdBatch())
				{
					if (itemPtr->GetAutoSupply()>AutoFlag_Yes)
					{
						return true; 
					}
					YKOrderPtr nexOrder =workIn->GetOrder();
					/*YKOrderPtr peakOrder = nexOrder->GetPeakOrder();

					if(peakOrder == NULL)	peakOrder = nexOrder;*/

					if(/*peakOrder != NULL
						&&(peakOrder->GetDesignation() != L""
						||peakOrder->GetType() == OrderType_WillOrder)*/
						nexOrder->PeakDesignationOrders()
						||nexOrder->PeakWillOrders()) //�������췬�Ż����򶩵�
					{
						return true;
					}
				}
				else 	if (itemPtr->GetAutoSupply()==AutoFlag_Yes)
				{
					YKOrderPtr nexOrder =workIn->GetOrder();
				/*	YKOrderPtr peakOrder = nexOrder->GetPeakOrder();

					if(peakOrder == NULL)	peakOrder = nexOrder;*/

					if(/*peakOrder != NULL
						&&(!(peakOrder->GetDesignation() != L""
							||peakOrder->GetType() == OrderType_WillOrder))*/
							nexOrder->PeakDesignationOrders()
							||nexOrder->PeakWillOrders()) //�� �������췬�Ż����򶩵�
					{
						return true;
					}
				}
			}
		}
	return false;
}
//�����Ϣ
void ApsCal::SetMsgInfo(MSG_LEVEL level,EOR_TYPE msgCodeId,YK_ULONG ObjTbl,list<YK_ULONG>& objIdlist)
{
	if (objIdlist.empty())
		return;
	CMessageData mesData; mesData.New();
	// ��Ϣ����
	mesData->SetLevel(level);
	// ��ʾ��Ϣ����Ϣ���ݣ�
	CError  	errorInfo;
	errorInfo.clear();
	errorInfo.m_errorType=msgCodeId;			// ��Ϣ����

	// ���/���ࣨ��Ϣ������λ�ã�
	mesData.SetType(MSG_TYPE_LOGIC);		// �߼�
	mesData.SetMessageData( level,MSG_TYPE_APS , L"", errorInfo);
	// ���/���ࣨ��Ϣ������λ�ã�
	mesData.SetType(MSG_TYPE_LOGIC);		// �߼�
	// ʱ��
	mesData.SetTime(BIZMODELTOOLS::FormatTime(CDealTime::GetLocalTime()));
	// ����
	mesData.SetTbl(TblOrder);
	mesData.SetObjTbl(0);
	mesData.SetIdlist(objIdlist);
	//g_pBizModel->InsertBiz(mesData);
}
//�����Ϣ
void ApsCal::SetMsgInfo(MSG_LEVEL level,EOR_TYPE msgCodeId,YK_ULONG ObjTbl,set<YK_ULONG>& objIdlist)
{
	if (objIdlist.empty())
		return;
	CMessageData mesData; mesData.New();
	// ��Ϣ����
	mesData.SetLevel(level);
	// ��ʾ��Ϣ����Ϣ���ݣ�
	CError  	errorInfo;
	errorInfo.clear();
	errorInfo.m_errorType=msgCodeId;			// ��Ϣ����

	// ���/���ࣨ��Ϣ������λ�ã�
	mesData.SetType(MSG_TYPE_LOGIC);		// �߼�
	mesData.SetMessageData( level,MSG_TYPE_APS , L"", errorInfo);
	// ���/���ࣨ��Ϣ������λ�ã�
	mesData.SetType(MSG_TYPE_LOGIC);		// �߼�
	// ʱ��
	mesData.SetTime(BIZMODELTOOLS::FormatTime(CDealTime::GetLocalTime()));
	// ����
	mesData.SetTbl(ObjTbl);
	mesData.SetObjTbl(0);
	mesData.SetIdlist(objIdlist);
	//g_pBizModel->InsertBiz(mesData);
}
void ApsCal::OutPutMessage()
{
	//����BOM��Ч�ľ�����Ϣ
	SetMsgInfo();
// 	list<YK_ULONG>fialPudOrderList;
//     fialPudOrderList.insert(fialPudOrderList.begin(),m_fialPudOrderList.begin(),m_fialPudOrderList.end());
	// ���충��չ��ʧ��:����*��
	SetMsgInfo( MSG_LEVEL_WARNING, EOR_TYPE_APS_ORDER_FILEOPEN, TblOrder, m_fialPudOrderList);   
	// �Զ��������충����ɾ��:����*��
	SetMsgInfo( MSG_LEVEL_WARNING, EOR_TYPE_APS_AUTOORDER_DELCOUNT, TblOrder, m_deAutoPudOrderList);   
	// �Զ��������충����������:����*��
	SetMsgInfo( MSG_LEVEL_WARNING, EOR_TYPE_APS_AUTOORDER_OUNTFLESH, TblOrder, m_updataAutoPudOrderList);   
	// �Զ�����ɹ�������ɾ��:����*��
	SetMsgInfo( MSG_LEVEL_WARNING, EOR_TYPE_APS_AUTOORDER_BUYDELCOUNT, TblOrder, m_deAutoStockOrderList);   
// 	list<YK_ULONG>updataAutoStockOrderList;
// 	updataAutoStockOrderList.insert(updataAutoStockOrderList.begin(),m_updataAutoStockOrderList.begin(),m_updataAutoStockOrderList.end()); 
	// �Զ�����ɹ�������������:����*��
	SetMsgInfo( MSG_LEVEL_WARNING, EOR_TYPE_APS_AUTOORDER_BUYCOUNTFLESH, TblOrder, m_updataAutoStockOrderList);   

	set<YK_ULONG>m_inValidWorkSet =g_pBizModel->GetInValidWorkSet();  //���������벻����

	SetMsgInfo( MSG_LEVEL_WARNING, EOR_TYPE_APS_InValid_WorkIn, TblWork, m_inValidWorkSet);   

	ClearList();
	g_pBizModel->ClearList();

}

void ApsCal::SubtractStockAmout(list<YKWorkOutputItemPtr>& workOutputItemList21)
{
	for(list<YKWorkOutputItemPtr>::iterator iter = workOutputItemList21.begin();
		iter != workOutputItemList21.end();iter++)
	{
	/*	m_assignAmoutMap[(*iter)->GetOutputItemId()]-=(*iter)->GetSpilthAmount()  ;
		if (m_assignAmoutMap[(*iter)->GetOutputItemId()]>0)
		{
			continue; 
		}else
			return;*/

		/*	m_assignAmout-= (*iter)->GetSpilthAmount()  ;
		if (m_assignAmout>PRECISION)
		{
		continue; 
		}else
		return;*/

	}
}
	//����BOM��Ч�ľ�����Ϣ
void ApsCal::SetMsgInfo()
{
	if (m_failItemBomList.empty())
		return;
	CMessageData mesData;
	mesData->New();
	// ��Ϣ����
	mesData.SetLevel(MSG_LEVEL_WARNING);
	// ��ʾ��Ϣ����Ϣ���ݣ�

	mesData.SetMessageCode(GetRemarkFiled(ID_ProductRouting_Select_Warn_InExist_SetUp_ByUser)+L":"+BIZMODELTOOLS::IToWString(m_failItemBomList.size(),10)+L"��");
	// ���/���ࣨ��Ϣ������λ�ã�
	mesData.SetType(MSG_TYPE_LOGIC);		// �߼�

	// ʱ��
	mesData.SetTime(BIZMODELTOOLS::FormatTime(CDealTime::GetLocalTime()));
	// ����
	mesData.SetTbl(TblItem);
	mesData.SetObjTbl(0);
	list<YK_ULONG>failItemBomList;
	failItemBomList.insert(failItemBomList.begin(),m_failItemBomList.begin(),m_failItemBomList.end());
	mesData.SetIdlist(failItemBomList);
	mesData.SetCount(failItemBomList.size());
	//g_pBizModel->InsertBiz(mesData);
}
void	ApsCal::GetMessage()
{
	ClearList();
	set<YK_ULONG>failPudOrderList =g_pBizModel->GetFailPudOrderList();  //����չ��ʧ�ܵ����춨��
	AppFialPudOrderList(failPudOrderList);

	set<YK_ULONG>failItemBomList =g_pBizModel->GetFailItemBomList();  //��Ʒ����·����Ч��
	AppFialItemBomList(failItemBomList);

	m_inValidWorkSet =g_pBizModel->GetInValidWorkSet();  //�����Ĺ������벻����
	

}
void ApsCal::ReUpdateReation(YKWorkInputItemPtr&inPtr,YKOrderPtr&preOrder)
{
	YKWorkPtr&tailWorkPtr= preOrder->GetTailOperationWork();
	if (NULL!=tailWorkPtr)
	{
		list<YKWorkOutputItemPtr>workOutList;
		tailWorkPtr->GetWorkOutList(workOutList);
		for(list<YKWorkOutputItemPtr>::iterator oIter = workOutList.begin();
			oIter != workOutList.end();oIter++)
		{
			YKWorkOutputItemPtr& workOut = *oIter;
			if(preOrder->GetPudId() == workOut->GetOutputItemId())
			{
				list<YKWorkRelationPtr>reationList;
				workOut->GetRelationPtrList(reationList);
				if (reationList.empty())
				{
					YK_FLOAT workCount = min(inPtr->GetScarcityAmount(),workOut->GetSpilthAmount());
					if(workCount>0.000001)	
					{
						YKWorkRelation::CreatNew(inPtr,workOut,2,workCount);
						return;
					}
				}
			}
		}
	}	
}
void ApsCal::CreatePreOrder(YKItemPtr& itemPtr,list<YK_FLOAT>&couts,list<YKWorkInputItemPtr>&workInputItemList)
{
	list<YKWorkOutputItemPtr>newWorkOutList;
	YKWorkOutputItemPtr workOutPtr=NULL;
	bool Exist=true;
	list<YK_FLOAT>::iterator iter=couts.begin();
	list<YKWorkInputItemPtr>::iterator i_iter =workInputItemList.begin();
	for (;i_iter!=workInputItemList.end();)
	{
		YKWorkInputItemPtr&workin=*i_iter;
		if(BZERO(workin->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
		{
			i_iter = workInputItemList.erase(i_iter); //ɾ�������
			continue;
		}
		YKOrderPtr nexOrderPtr=workin->GetOrder(); 
		if (NULL!=workOutPtr)
		{
			YKOrderPtr preOrderPtr=workOutPtr->GetOrder(); 
			YK_FLOAT reationAmout=min(workOutPtr->GetSpilthAmount() ,workin->GetScarcityAmount());
			if (reationAmout>PRECISION)
				YKWorkRelation::CreatNew(workin ,workOutPtr,RelationType_Differ,reationAmout);

			if (NULL!=preOrderPtr&&NULL!=nexOrderPtr)    //���ݹ�������깤ʱ�̼���ֵ
				preOrderPtr->PreOrderLateFinishTimeCalValue(nexOrderPtr);   
		}
		if(BZERO(workin->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
		{
			i_iter = workInputItemList.erase(i_iter); //ɾ�������
			continue;
		}
		if(NULL!=workOutPtr&&BZERO(workOutPtr->GetSpilthAmount()))		//�ж������������Ƿ�Ϊ0
		{    
			workOutPtr=NULL;	
		}
		if (NULL==workOutPtr)
		{
			if (iter==couts.end())
				return ;	
			for (;iter!=couts.end();iter++)
			{
				YKOrderPtr orderPtr = YKOrder::CreatNew(*iter,itemPtr->GetId());
				if(orderPtr != NULL)
				{
					orderPtr->OrderLateFinishTimeCalValue();
					orderPtr->OrderEarlyStartTimeCalValue();
					if (NULL!=orderPtr&&NULL!=nexOrderPtr)    //���ݹ�������깤ʱ�̼���ֵ
						orderPtr->PreOrderLateFinishTimeCalValue(nexOrderPtr);   

					InforTransfor Trans;
					Trans.OrderInfoTransfor(nexOrderPtr,orderPtr);

					orderPtr->UpdateIOItem();
					orderPtr->FlashOrderState();
					if (NULL!=orderPtr)
					{
						if(orderPtr->GetCode().empty())
							orderPtr->CreateOrderCode();
					}
					//���������Ϣ
					if (orderPtr->GetExcptFlg()!=1)
					{
						if (orderPtr->GetExcptFlg()==Spread_No_Bom)
							AddFailItemBomList( itemPtr->GetId());    //���һ��BOM��Ч��Ϣ
						if (orderPtr->GetType() == OrderType_PudOrder )  //���춨��
						{
							AddFialPudOrderList(orderPtr->GetId());
						}
						orderPtr->Delete();
						continue;
					}
					//������Ϣ����

					Trans.WorkInforTransfor(orderPtr);
					
					YKWorkOutputItemPtr workOutputItem = YKWorkOutputItem::FindOrderOutItem(orderPtr);
					if(workOutputItem != NULL)
					{
						newWorkOutList.push_back(workOutputItem);
						YK_FLOAT amount=min(workOutputItem->GetSpilthAmount() ,workin->GetScarcityAmount());
						if (amount>0)
							YKWorkRelation::CreatNew(workin ,workOutputItem,RelationType_Differ,amount);
						if (NULL!=orderPtr)
						{
							if(orderPtr->GetCode().empty())
								orderPtr->CreateOrderCode();
						}
						if(BZERO(workOutputItem->GetSpilthAmount()))		//�ж������������Ƿ�Ϊ0
						{    
							workOutPtr=NULL;	
							if(BZERO(workin->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
							{
								  if (iter!=couts.end())
								  {
									  iter++;
								  }
									  break;
							}
						}else
						{
                          workOutPtr=workOutputItem;	
						  if (iter!=couts.end())
						  {
							  iter++;
						  }
						  break;
						}
					}
			    }
			}
		}
		if(BZERO(workin->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
		{
			i_iter = workInputItemList.erase(i_iter); //ɾ�������
			continue;
		}	
	}

	//////////////////////////////////////////////////////////////////////////
	//�������Ĵ���
	for (;iter!=couts.end();iter++)
	{
		YKOrderPtr orderPtr = YKOrder::CreatNew(*iter,itemPtr->GetId());
		if(orderPtr != NULL)
		{
			orderPtr->OrderLateFinishTimeCalValue();
			orderPtr->OrderEarlyStartTimeCalValue();
			orderPtr->UpdateIOItem();
			orderPtr->FlashOrderState();
			//���������Ϣ
			if (orderPtr->GetExcptFlg()!=1)
			{
				if (orderPtr->GetExcptFlg()==Spread_No_Bom)
					AddFailItemBomList( itemPtr->GetId());    //���һ��BOM��Ч��Ϣ
				if (orderPtr->GetType() == OrderType_PudOrder )  //���춨��
				{
					AddFialPudOrderList(orderPtr->GetId());
				}
				orderPtr->Delete();
				break;
			}
			YKWorkOutputItemPtr workOutputItem = YKWorkOutputItem::FindOrderOutItem(orderPtr);
			if(workOutputItem != NULL)
				newWorkOutList.push_back(workOutputItem);
			if (NULL!=orderPtr)
			{
				if(orderPtr->GetCode().empty())
					orderPtr->CreateOrderCode();
			}
			//������Ϣ����
			InforTransfor Trans;
			Trans.WorkInforTransfor(orderPtr);
		}
	}
	AllotMinTargetStock(itemPtr,newWorkOutList); // ������2��ʣ��������Ʒ
}

//Add 2012-06-06 ���ݹ��ƥ��CreatePreOrder
void ApsCal::CreatePreOrderBySpec(YKItemPtr& itemPtr,list<YK_FLOAT>&couts,list<YKWorkInputItemPtr>&workInputItemList)
{
	list<YKWorkOutputItemPtr>newWorkOutList;
	YKWorkOutputItemPtr workOutPtr=NULL;
	list<YK_FLOAT>::iterator iter=couts.begin();

	map<YK_WSTRING,list<YKWorkInputItemPtr>> mapWkInputSpecId;
	for ( list<YKWorkInputItemPtr>::iterator fItor = workInputItemList.begin();fItor != workInputItemList.end(); fItor++ )
	{
		YKWorkInputItemPtr& workInputItemPtr = *fItor;
		if ( workInputItemPtr != NULL )
		{
			YKOrderPtr& winOrderPtr =workInputItemPtr->GetOrder(); 
			if ( winOrderPtr != NULL )
			{
				YK_WSTRING& specValue = winOrderPtr->GetSpecStr(); //���
				YK_WSTRING& specDataValue = winOrderPtr->GetSpecDataStr();	//��ֵ���
				
				mapWkInputSpecId[specValue+specDataValue].push_back(workInputItemPtr);
			}
		}
	}

	for( map<YK_WSTRING,list<YKWorkInputItemPtr>>::iterator vItor = mapWkInputSpecId.begin(); vItor != mapWkInputSpecId.end()&&iter!=couts.end();vItor++,iter++ )
	{
		list<YKWorkInputItemPtr>& workInputList = vItor->second;
		for ( list<YKWorkInputItemPtr>::iterator lItor = workInputList.begin(); lItor != workInputList.end();  )
		{
			YKWorkInputItemPtr& workInPtr = *lItor;
			if ( workInPtr != NULL )
			{
				if(BZERO(workInPtr->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
				{
					lItor = workInputList.erase(lItor); //ɾ�������
					continue;
				}
				YKOrderPtr nexOrderPtr=workInPtr->GetOrder(); 
				if (NULL!=workOutPtr)
				{
					YKOrderPtr preOrderPtr=workOutPtr->GetOrder(); 
					YK_FLOAT reationAmout=min(workOutPtr->GetSpilthAmount() ,workInPtr->GetScarcityAmount());
					if (reationAmout>PRECISION)
						YKWorkRelation::CreatNew(workInPtr ,workOutPtr,RelationType_Differ,reationAmout);

					if (NULL!=preOrderPtr&&NULL!=nexOrderPtr)    //���ݹ�������깤ʱ�̼���ֵ
						preOrderPtr->PreOrderLateFinishTimeCalValue(nexOrderPtr);   
				}
				if(BZERO(workInPtr->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
				{
					lItor = workInputList.erase(lItor); //ɾ�������
					continue;
				}
				if(NULL!=workOutPtr&&BZERO(workOutPtr->GetSpilthAmount()))		//�ж������������Ƿ�Ϊ0
				{    
					workOutPtr=NULL;	
				}
				if (NULL==workOutPtr)
				{
					if (iter==couts.end())
						return ;
					YKOrderPtr orderPtr = YKOrder::CreatNew(*iter,itemPtr->GetId());
					if(orderPtr != NULL)
					{
						orderPtr->OrderLateFinishTimeCalValue();
						orderPtr->OrderEarlyStartTimeCalValue();
						if (NULL!=orderPtr&&NULL!=nexOrderPtr)    //���ݹ�������깤ʱ�̼���ֵ
							orderPtr->PreOrderLateFinishTimeCalValue(nexOrderPtr);   

						InforTransfor Trans;
						Trans.OrderInfoTransfor(nexOrderPtr,orderPtr);

						orderPtr->UpdateIOItem();
						orderPtr->FlashOrderState();
						if (NULL!=orderPtr)
						{
							if(orderPtr->GetCode().empty())
								orderPtr->CreateOrderCode();
						}
						//���������Ϣ
						if (orderPtr->GetExcptFlg()!=1)
						{
							if (orderPtr->GetExcptFlg()==Spread_No_Bom)
								AddFailItemBomList( itemPtr->GetId());    //���һ��BOM��Ч��Ϣ
							if (orderPtr->GetType() == OrderType_PudOrder )  //���춨��
							{
								AddFialPudOrderList(orderPtr->GetId());
							}
							orderPtr->Delete();
							continue;
						}
						//������Ϣ����

						Trans.WorkInforTransfor(orderPtr);

						YKWorkOutputItemPtr workOutputItem = YKWorkOutputItem::FindOrderOutItem(orderPtr);
						if(workOutputItem != NULL)
						{
							newWorkOutList.push_back(workOutputItem);
							YK_FLOAT amount=min(workOutputItem->GetSpilthAmount() ,workInPtr->GetScarcityAmount());
							if (amount>0)
								YKWorkRelation::CreatNew(workInPtr ,workOutputItem,RelationType_Differ,amount);
							if (NULL!=orderPtr)
							{
								if(orderPtr->GetCode().empty())
									orderPtr->CreateOrderCode();
							}
							if(BZERO(workOutputItem->GetSpilthAmount()))		//�ж������������Ƿ�Ϊ0
							{    
								workOutPtr=NULL;	
							}else
							{
								workOutPtr=workOutputItem;	
							}
						}
					}
				}
			}
			++lItor;
		}		
	}

	//�������Ĵ���
	for (;iter!=couts.end();iter++)
	{
		YKOrderPtr orderPtr = YKOrder::CreatNew(*iter,itemPtr->GetId());
		if(orderPtr != NULL)
		{
			orderPtr->OrderLateFinishTimeCalValue();
			orderPtr->OrderEarlyStartTimeCalValue();
			orderPtr->UpdateIOItem();
			orderPtr->FlashOrderState();
			//���������Ϣ
			if (orderPtr->GetExcptFlg()!=1)
			{
				if (orderPtr->GetExcptFlg()==Spread_No_Bom)
					AddFailItemBomList( itemPtr->GetId());    //���һ��BOM��Ч��Ϣ
				if (orderPtr->GetType() == OrderType_PudOrder )  //���춨��
				{
					AddFialPudOrderList(orderPtr->GetId());
				}
				orderPtr->Delete();
				break;
			}
			YKWorkOutputItemPtr workOutputItem = YKWorkOutputItem::FindOrderOutItem(orderPtr);
			if(workOutputItem != NULL)
				newWorkOutList.push_back(workOutputItem);
			if (NULL!=orderPtr)
			{
				if(orderPtr->GetCode().empty())
					orderPtr->CreateOrderCode();
			}
			//������Ϣ����
			InforTransfor Trans;
			Trans.WorkInforTransfor(orderPtr);
		}
	}
	AllotMinTargetStock(itemPtr,newWorkOutList); // ������2��ʣ��������Ʒ
}
bool ApsCal::IfHaveRelation(YKWorkOutputItemPtr&workOutPtr)
{
	list<YKWorkRelationPtr> relationList;
	workOutPtr->GetRelationPtrList(relationList);
	if (!relationList.empty())
	{
		return true;
	}
	YKOrderPtr orderPtr=workOutPtr->GetOrder();
	if (NULL!=orderPtr&&orderPtr->GetAlreadyAssignQuantity()>PRECISION)
	{
		return true;
	}
	return false;

}
void ApsCal::ClearAmount()
{
	m_unused        =0;
	m_supplyAmout   =0;
	m_totalAmout    =0;          //���漯��0��1��3�еĶ�������֮��
	m_assignAmout   =0;          //�����������
	m_minStckAmoutMap.clear();          //�������С������
	m_minTarStckAmoutMap.clear();          //�������СĿ�������
	m_minStckAmoutList.clear();          //�������С������
	m_minTarStckAmoutList.clear();          //�������СĿ�������
	//m_backOrderMap.clear();
}

void ApsCal::InforransferAndOrderRenovate(YKWorkInputItemPtr&workInPtr,YKWorkOutputItemPtr&workOutPtr)
{
	YKOrderPtr preOrder=workOutPtr->GetOrder();
	YKOrderPtr nexOrder=workInPtr->GetOrder();
	if (NULL==preOrder||NULL==nexOrder)
		return;

	if (preOrder->PreNoSchOrder())    //���ŵĹ���������Ϣ����
	{
		if (!preOrder->JustIfHaveTopOrder()) //�Ƿ������ϲ㶩��
		{
			if (preOrder->GetDiffer()==OrderDiffer_Auto)
			{
				if (NULL!=workOutPtr&&workOutPtr->GetIsMainProduction())
				{
					//������Ϣ����     
					InforTransfor Trans;
					Trans.OrderInfoTransfor(nexOrder,preOrder);
					if (preOrder->ReationFilterByState()) //������Ϣ����
					{
						Trans.WorkInforTransfor(preOrder);
					}else
						Trans.WorkInforTransfor(preOrder);
				}
			}
		}
		//preOrder->PreOrderLateFinishTimeCalValue(nexOrder);  	//���ݹ�������깤ʱ�̼���ֵ
		if (!preOrder->ReationFilterByState())  //�Ƿ���ڹ���״̬��ָʾ�������
		{
			YKItemPtr itemPtr=preOrder->GetItem();
			if (NULL!=itemPtr)
			{
				if (!itemPtr->IfSwitchProductRoutin()) //ǰ�����Ƿ��л���Ʒ����·��
				{
					preOrder->SetAboveOrderId( nexOrder->GetId());
					preOrder->UpdateIOItem();
					preOrder->SetAboveOrderId( 0);
				}
			}
		}
	}
	if (preOrder->GetExcptFlg()==1)
	{
		InforTransfor Trans;      		//������Ϣ����     
		Trans.WorkInforTransfor(preOrder);
	}
}

void ApsCal::DeleteWorkOutPtr(YKWorkOutputItemPtr&workOutPtr,list<YKWorkOutputItemPtr>& workOutputItemList2,list<YKWorkOutputItemPtr>& workOutputItemList21)
{
	if (NULL==workOutPtr) return;
	for (list<YKWorkOutputItemPtr>::iterator iter=workOutputItemList2.begin();
		iter!=workOutputItemList2.end();iter++)
    {
             YKWorkOutputItemPtr outPtr=*iter;
			 if (NULL!=outPtr&&outPtr->GetId()==workOutPtr->GetId())
			 {
                    workOutputItemList2.erase(iter);
					return;
			 }
    }
	for (list<YKWorkOutputItemPtr>::iterator iter=workOutputItemList21.begin();
		iter!=workOutputItemList21.end();iter++)
	{
		YKWorkOutputItemPtr outPtr=*iter;
		if (NULL!=outPtr&&outPtr->GetId()==workOutPtr->GetId())
		{
			workOutputItemList21.erase(iter);
			return; 
		}
	}
}

void ApsCal::OrderSpread()
{
	g_bizProcess = 0;
	g_pBizModel->SetTableFlashFlg(TRUE,TblResource);

	g_bizProcess = 2;
	g_pBizModel->InitSignLocation();  //��ʼ����־λ

	g_bizProcess = 4;
	// �Զ������м���Ʒ
	YKApsParamPtr ApsParam=	g_pBizModel->GetApsParam();
	if(NULL!=ApsParam&&ApsParam->GetAutoMiddleItem())		//�Ƿ��Զ������м���Ʒ
	{
		YKItemPtrMap* itemPtrMap=g_pBizModel->GetYKItemPtrMap();
		if(NULL != itemPtrMap)
			itemPtrMap->InitItemCode();
		g_pBizModel->ClearOperInfo();
		BaseRelByOperation();
		AutoAddAllMiddleItem();
		g_pBizModel->SetTableFlashFlg(TRUE,TblBOM);
		g_pBizModel->SetTableFlashFlg(TRUE,TblItem);
		g_pBizModel->SetTableFlashFlg(TRUE,ID_BOM_VIEW_DLG);
		g_pBizModel->SetTableFlashFlg(TRUE,BOMViewFlash_Only);		
	}

	g_bizProcess = 5;
	//����������PSI����
//	CalcForecastOrder();
	
	CNewRuleManager m_rulManager;
	m_rulManager.SetInfo(RULE_DEF_SCH_FILTER_Item,ApsParam->GetItemFiterRule(),TblItem);		//��Ʒɸѡ

	//����ͽ���
	if(NULL!=ApsParam&&ApsParam->GetItemRank())		//�Ƿ���Ҫ�ͽ������
	{
		g_pBizModel->CalRankCode();
	} 
	g_bizProcess = 7;

	//����󶨵�������ϵ
	g_pBizModel->YKBackOrderDelete();

	g_bizProcess = 10;
	//����չ��
	g_pBizModel->UpdateIOItem();

	g_bizProcess = 20;

	g_pBizModel->YKRelation();	// ����ά��

	g_bizProcess = 22;
	//��ʼ����Ʒ�б�
	CItemList itemList;
	//��ʼ����Ʒ�б�
	itemList.InitItem(3);

	g_bizProcess = 25;

	GetMessage();

	//������Ʒ�б�
	YK_ULONG rankCode=0;
	list<YKItemPtr> itemPtrList;
	InitBackOrderRelation();	//	�󶩵��Ĵ���

	g_bizProcess = 30;

	int count = 0;
	int maxItem = itemList.GetCount();
	for(itemList.Begin();itemList.NotEnd();itemList.Step())
	{
		YKItemPtr&	itemPtr = itemList.GetItem();
		if (rankCode!=itemPtr->GetRankCode())	//�ͽ���
		{
			rankCode=itemPtr->GetRankCode();
			NewCreateRelation( itemPtrList );	//	����������ϵ --������
			itemPtrList.clear();
			itemPtrList.push_back(itemPtr);
		}
		else
			itemPtrList.push_back(itemPtr);		

		g_bizProcess = 30 + (double)((++count)*50)/maxItem;
	}
	NewCreateRelation( itemPtrList );		//	����������ϵ --������

	g_bizProcess = 80;

	//�Զ����䶩������code
	g_pBizModel->CreateOrderCode();

	g_bizProcess = 85;

	g_pBizModel->GetYKWorkPtrMap()->CreatAvbResource();		//����������Դ

	g_bizProcess = 90;

	if (NULL!=ApsParam&&ApsParam->GetObTransfer())
		g_pBizModel->GetYKOrderPtrMap()->RestrictionTransfer();			// ����Լ������

	g_bizProcess = 95;

	g_pBizModel->GetYKOrderPtrMap()->CalculateIntendBegin_EndTime();	//	���㶩���ļƻ������깤ʱ��

	OutPutMessage();	//���չ�����

	g_pBizModel->ResumeSignLocation();  //�ָ���־λ
	g_bizProcess = 100;
}

bool ApsCal::SortWorkInputItemList( YKWorkInputItem& first,YKWorkInputItem& second )
{
	YKOrderPtr fOrder = first->GetOrder();
	YKOrderPtr fPeakOrder;
	if(fOrder != NULL)
	{
		fOrder->GetPeakOrder(fPeakOrder);
	}

	//ȡ�����붩��
	YKOrderPtr sOrder = second->GetOrder();
	YKOrderPtr sPeakOrder;
	if(sOrder != NULL)
	{
		sOrder->GetPeakOrder(sPeakOrder);
	}

	//�ж϶�����Ч��
	if(fPeakOrder == NULL)
	{
		if(sPeakOrder == NULL)
			return first->GetId() < second.GetId();
		else
			return false;
	}
	if(sPeakOrder == NULL)	//ǰ��������Ч
		return true;

	if(fPeakOrder->WillOrderByPriorityBorderLine() && sPeakOrder->WillOrderByPriorityBorderLine())	//ȫ��Ϊ���򶩵�
	{
		YK_TIME_T fTm=first->GetCmpTime() ;  //modify 2011-2-17
		YK_TIME_T sTm=second->GetCmpTime() ;
		if (fTm<sTm)
		{
			return true;
		}
		else if (fTm==sTm)
		{
			YKOrderPtr order =first->GetOrder();
			YKOrderPtr orderPS =second->GetOrder();
			if (NULL!=order&&NULL!=orderPS)  //���ն������ȼ���
			{
				//���ȼ�����
				if(order->GetPriority() > orderPS->GetPriority())
					return true;
				else if(order->GetPriority() < orderPS->GetPriority())
					return false;
				else if(order->GetPriority() == orderPS->GetPriority())
				{
					//ID������
					if(order->GetId() < orderPS->GetId())
						return true;
					else if (order->GetId() == orderPS->GetId())
					{
                         return first->GetId()<second->GetId();
					}
						return false;
				}

			}
		}
		else
			return false;

		YKWork fWorkPS = first->GetWork();
		YKWork sWorkPS = second->GetWork();
		//0.�ж��Ƿ����
		if (fWorkPS->GetSchFlag() == SchFlg_WaitSch)
		{
			if(sWorkPS->GetSchFlag() == SchFlg_WaitSch)
			{
				
				//3.���ն��������깤ʱ�̼���ֵ����
				if(fOrder->GetCalLastEndTime() <= 0)
				{
					if(sOrder->GetCalLastEndTime() > 0)
						return false;
				}
				else 
				{
					if(sOrder->GetCalLastEndTime() <= 0)
						return true;
					else if(fOrder->GetCalLastEndTime() > sOrder->GetCalLastEndTime())
					{
						return false;
					}
					else if(fOrder->GetCalLastEndTime() < sOrder->GetCalLastEndTime())
					{
						return true;
					}
				}
				return fOrder->GetId() < sOrder.GetId();
			}
			else
			{
				return false;
			}
		}
		else
		{
			if(sWorkPS->GetSchFlag() == SchFlg_WaitSch)
			{
				return true;
			}
		}

		//1.���չ���ʵ�ʿ�������
		if(fWorkPS->GetActualStartTm() <= 0)
		{
			if(sWorkPS->GetActualStartTm() > 0)
				return false;
		}
		else 
		{
			if(sWorkPS->GetActualStartTm() <= 0)
				return true;
			else if(fWorkPS->GetActualStartTm() > sWorkPS->GetActualStartTm())
			{
				return false;
			}
			else if(fWorkPS->GetActualStartTm() < sWorkPS->GetActualStartTm())
			{
				return true;
			}
		}
		//2.���չ����ƻ�����ʱ������
		if(fWorkPS->GetPlantStartTm() <= 0)
		{
			if(sWorkPS->GetPlantStartTm() > 0)
				return false;
		}
		else 
		{
			if(sWorkPS->GetPlantStartTm() <= 0)
				return true;
			else if(fWorkPS->GetPlantStartTm() > sWorkPS->GetPlantStartTm())
			{
				return false;
			}
			else if(fWorkPS->GetPlantStartTm() < sWorkPS->GetPlantStartTm())
			{
				return true;
			}
		}
		//3.���ն��������깤ʱ�̼���ֵ����
		if(fOrder->GetCalLastEndTime() <= 0)
		{
			if(sOrder->GetCalLastEndTime() > 0)
				return false;
		}
		else 
		{
			if(sOrder->GetCalLastEndTime() <= 0)
				return true;
			else if(fOrder->GetCalLastEndTime() > sOrder->GetCalLastEndTime())
			{
				return false;
			}
			else if(fOrder->GetCalLastEndTime() < sOrder->GetCalLastEndTime())
			{
				return true;
			}
		}
		//4.��id����
		return first->GetId() < second.GetId();
	}
	else if (fPeakOrder->WillOrderByPriorityBorderLine() && (!sPeakOrder->WillOrderByPriorityBorderLine())) //
	{
		return true;
	}
	else if((!fPeakOrder->WillOrderByPriorityBorderLine()) && sPeakOrder->WillOrderByPriorityBorderLine())
	{
		return false;
	}
	else	//ȫΪ���򶩵�
	{
		if(fPeakOrder->GetCalLastEndTime() < sPeakOrder->GetCalLastEndTime())
			return true;
		else 
			return false;
	}
}

void ApsCal::ReationStock(list<YKWorkInputItemPtr>&workInputItemList,list<YKWorkOutputItemPtr>&workOutputItemList1)
{
	//Add 2011-12-13 �������
	list<YKWorkInputItemPtr>::iterator iter=workInputItemList.begin();
	for (;iter!=workInputItemList.end();)
	{
		YKWorkInputItemPtr inPtr=*iter;

		if (inPtr != NULL)
		{
			YKItemPtr itemPtr=inPtr->GetItem();
			if (NULL==itemPtr)
			{
				iter = workInputItemList.erase(iter);
				continue;
			}
			if(itemPtr->GetStockMode() == StockMode_No)   //��Ʒ���Ϸ�ʽ������
			{
				iter++;
				continue;
			}
			if(itemPtr->GetAutoSupply() == AutoFlag_YesOne) //��1��1����
			{
				iter++;
				continue;
			}

			if (inPtr->GetCmdCode().empty())
			{
				RelationStock(inPtr,workOutputItemList1);
			}
			else
			{
				//Add 2011-12-08 �ж��Ƿ�Ϊ����
				if (!inPtr->/*GetMainMaterialFlg*/GetItemReplaceSign())
				{
					iter++;
					continue;
				}

				YK_ULONG operId = inPtr->GetOperationId();	//��ȡ������Ʒ ����ID
				YKOrderPtr orderPtr = inPtr->GetOrder();	//��ȡ������Ʒ ����

				//�ɹ���ID ��ù������
				YKOperationPtr operPtr = g_pBizModel->GetYKOperationPtrMap()->Get(operId);

				if (operPtr != NULL && orderPtr != NULL)
				{	
					list<YKWorkInputItemPtr> singleItemWorkInputItemList = workInputItemList;

					//��������ƥ�乤�����벢��ù�������
					GetWorkInItemListByOperInItem(operPtr,orderPtr,inPtr,singleItemWorkInputItemList);

					//ִ���������
					ItemReplaceByWorkInItemList( workOutputItemList1,singleItemWorkInputItemList );
				}
			}

			iter++;

		}	
	}



	//list<YKWorkInputItemPtr>::iterator iter=workInputItemList.begin();
	//for (;iter!=workInputItemList.end();)
	//{
	//	YKWorkInputItemPtr inPtr=*iter;
	//	if (NULL!=inPtr)  //�������롢���������
	//	{
	//		YKItemPtr itemPtr=inPtr->GetItem();
	//		if (NULL==itemPtr)
	//		{
	//			iter = workInputItemList.erase(iter);
	//			continue;
	//		}
	//		if(itemPtr->GetStockMode() == StockMode_No)   //��Ʒ���Ϸ�ʽ������
	//		{
	//			iter++;
	//			continue;
	//		}
	//		if(itemPtr->GetAutoSupply() == AutoFlag_YesOne) //��1��1����
	//		{
	//			iter++;
	//			continue;
	//		}
	//		//Add 2011-12-08 �ж��Ƿ�Ϊ����
	//		if (!inPtr->/*GetMainMaterialFlg*/GetItemReplaceSign())
	//		{
	//			iter++;
	//			continue;
	//		}
	//		if (inPtr->GetCmdCode().empty())
	//		{
	//			RelationStock(inPtr,workOutputItemList1);
	//		}
	//		else
	//		{	
			//	//��ù��������б�
			//	list<YKWorkInputItemPtr>cmdComList=inPtr->GetCmdCodeCombList();

			//	if (cmdComList.size()==1)
			//	{
			//		RelationStock(inPtr,workOutputItemList1);
			//	}
			//	else
			//	{
			//		//if (inPtr->GetItemReplaceSign()==L""||inPtr->GetItemReplaceSign()==L"M")
			//		{
			//			//�����ȫ������ڹ���������
			//			//����	����������
			//			YKWorkInputItemPtr aInPtr=inPtr->GetReationAllReplace(cmdComList);
			//			if (NULL!=aInPtr)
			//			{
			//				if (aInPtr->GetId()==inPtr->GetId())
			//					RelationStock(aInPtr,workOutputItemList1);
			//				else
			//				{
			//					aInPtr->ItemRepalce();
			//					RelationStock(aInPtr,workOutputItemList1);
			//					inPtr->ItemRepalce();  //��᲻������������ 
			//				}	
			//			}
			//			else
			//			{
			//				cmdComList.sort(SortCombWorkInList);
			//				list<YKWorkInputItemPtr>::iterator iiter;
			//				for ( iiter=cmdComList.begin(); iiter!=cmdComList.end();iiter++)
			//				{
			//					YKWorkInputItemPtr cInPtr=*iiter;
			//					if (NULL!=cInPtr)
			//					{
			//						cInPtr->ItemRepalce();
			//						RelationStock(cInPtr,workOutputItemList1);
			//						if (cInPtr->GetScarcityAmount()>0)  //��ȫ���
			//						{
			//							if (cInPtr->GetItemReplaceType()==ItemReplaceType_All)
			//							{
			//								cInPtr->DeleteAllReplaceReation(m_nWkRePtrList);
			//							}
			//						}
			//					}
			//				}
			//				inPtr->ItemRepalce();  //��᲻������������ 
			//				if (inPtr->GetScarcityAmount()>0&&inPtr->GetItemReplaceType()==ItemReplaceType_All)  //��ȫ���
			//				{		
			//					RelationStock(inPtr,workOutputItemList1);
			//				}
			//			}
			//		}					
			//	}
		//	}
		//}
		//iter++;
	//}
}
 bool ApsCal::SortCombWorkInList(YKWorkInputItemPtr&first,YKWorkInputItemPtr&second)
 {
       if (first==NULL||second==NULL)
       {
 		  return true;
       }
       if (first->GetItemReplaceType()==1&&second->GetItemReplaceType()==1)
       {
 		  bool fRelFlg=first->MatchAllItemRepalceReation();
 		  bool sRelFlg=second->MatchAllItemRepalceReation();
 		  if (fRelFlg||sRelFlg)
 		  {
              return fRelFlg;
 		  }
       }
 	  if (first->GetPriority()==second->GetPriority())
 	  {
 		  if (first->GetCmdCode()==L""||first->GetCmdCode()==L"M")
 		  {
 			  return true;
 		  }
 		  else if (second->GetCmdCode()==L""||second->GetCmdCode()==L"M")
 		  {
 			    return true;
 		  }
 		  else if (first->GetId()<second->GetId())
 		  {
 			   return true;
 		  }
 		  else 
 			  return false;
 	  }
 	  return first->GetPriority()>second->GetPriority();
 }
void ApsCal::RelationStock(YKWorkInputItemPtr&inPtr,list<YKWorkOutputItemPtr>&workOutputItemList0,YK_BOOL reType)
{
	CNewRuleManager rulManager;
	rulManager.SetInfo(RULE_DEF_SCH_FILTER_Item,g_pBizModel->GetApsParam()->GetItemFiterRule(),TblItem);
	if(!rulManager.GetBoolValue(inPtr->GetInputItemId()))  //��Ʒɸѡ
		return;

	rulManager.SetInfo(RULE_DEF_FILTER_Order,g_pBizModel->GetApsParam()->GetOrderFiter(),TblOrder);
	if(!rulManager.GetBoolValue(inPtr->GetOrderId()))  //����ɸѡ
		return;

	YKOrderPtr nexOrder=inPtr->GetOrder();
	if (NULL == nexOrder)
	{
		return;
	}
	if(nexOrder->CheckItemCyc(inPtr->GetInputItemId()))  //������Ʒ�Ĺ���
		return ;

	m_nWkRePtrList.clear();
	list<YKWorkOutputItemPtr>::iterator iter=workOutputItemList0.begin();
	for (;iter!=workOutputItemList0.end();)
	{
		YKWorkOutputItemPtr &outPtr=*iter;
		if (NULL!=outPtr)
		{
			//if (BZERO(inPtr->GetScarcityAmount()))
			//{
			//	return;
			//}
			if (inPtr->GetInputItemId()!=outPtr->GetOutputItemId())
			{
				iter++;
				continue;
			}
			if (BZERO(outPtr->GetSpilthAmount()))
			{
				iter++;
				continue;
			}
			YKOrderPtr preOrderPtr=outPtr->GetOrder();
			if (NULL==preOrderPtr)
			{
				iter=workOutputItemList0.erase(iter);
				continue;
			}

			int i=0;

			if (reType&&(!(preOrderPtr->IfMatchDesignation(nexOrder)/*preOrderPtr->GetDesignation()==nexOrder->GetDesignation()&&(nexOrder->GetDesignation()!=L"")*/)))  //�Ʒ��Ų�ƥ��
			{
				iter++;
				continue;
			}

			//�Ƿ��������Ʒƥ��
			if (!reType&&InAndOutIfMatch(/* itemPtr,*/ inPtr, outPtr))
			{
				iter++;
				continue;
			}

			//Add 2012-04-16 ����Ƿ�ƥ�� false Ϊ����ƥ��
			if ( !IsRelWorkInputAndOutputBySpecs( inPtr ,outPtr ) )
			{
				iter++;
				continue;
			}

			if (!g_pBizModel->IsRelafixInVal(inPtr,outPtr)/*IsRelafixInVal(inPtr,outPtr)*/)
			{
				InforransferAndOrderRenovate(inPtr,outPtr);   //��Ϣ�����Լ���������
				if (preOrderPtr->GetExcptFlg()==1)
				{
					//��������Ϊ���ṩ��������������Сֵ modify by yp 2011.11.23
					YK_DOUBLE reationAmout=min(inPtr.GetScarcityAmount()/*inPtr.GetPlantAmount()*/,outPtr->GetSpilthAmount());
					if (reationAmout>0)
					{
						YKWorkRelationPtr rePtr=YKWorkRelation::CreatNew(inPtr ,outPtr,RelationType_Differ,reationAmout);
						if (NULL!=rePtr)
						{
							m_nWkRePtrList.push_back(rePtr);
							if (NULL!=nexOrder)
								preOrderPtr->PreOrderLateFinishTimeCalValue(nexOrder);  	//���ݹ�������깤ʱ�̼���ֵ
						}
					}
					if (BZERO(inPtr->GetScarcityAmount()) || inPtr->GetScarcityAmount() < 0.0001)
					{
						return;
					}
				}
				else
				{
					iter++;
					continue;
				}
			}
		}
		iter++;
	}
}
//����ֵtrueΪ�����̶���Ч
//bool ApsCal::IsRelafixInVal(YKWorkInputItemPtr&inPtr,YKWorkOutputItemPtr&outPtr)
//{
//	YKOrderPtr preOrder =outPtr->GetOrder();// GetPreOrderPtr();
//	if (preOrder->GetDesignation()!= L"")
//	{
//		YKOrderPtr nextOrder =inPtr->GetOrder();// GetNextOrderPtr();
//		YKOrderPtr peakOrder = nextOrder->GetPeakOrder();
//
//		if(!(peakOrder!= NULL))	peakOrder = nextOrder;
//
//		if(peakOrder->GetDesignation() == L""
//			&& nextOrder->GetDesignation() == L"") 	
//			return true;
//
//		if(peakOrder->GetDesignation() != preOrder->GetDesignation()
//			|| nextOrder->GetDesignation() != preOrder->GetDesignation()) 	
//			return true;
//	}else    //ǰ�������Ʒ���
//	{
//		YKOrderPtr nextOrder = inPtr->GetOrder();//GetNextOrderPtr();
//		YKOrderPtr peakOrder = nextOrder->GetPeakOrder();
//
//		if(!(peakOrder!= NULL))	peakOrder = nextOrder;
//
//		if(peakOrder->GetDesignation() != L""
//			|| nextOrder->GetDesignation() != L"") 	 //�󶩵��򶥲㶩�����Ʒ���
//		{
//			if (preOrder->GetType()==OrderType_AbsOrder
//				&&!g_pBizModel->GetApsParam()->ParseHaveDesignOrderRelCondition(0))
//			{
//				return true;
//			}
//			if (preOrder->GetType()==OrderType_RelOrder
//				&&!g_pBizModel->GetApsParam()->ParseHaveDesignOrderRelCondition(1))
//			{
//				return true;
//			}
//			if (preOrder->GetDiffer()==OrderDiffer_Hand&&preOrder->GetType()==OrderType_PudOrder
//				&&!g_pBizModel->GetApsParam()->ParseHaveDesignOrderRelCondition(3)) //¼�����충��
//			{
//				return true;
//			}
//			if (preOrder->GetDiffer()==OrderDiffer_Hand&&preOrder->GetType()==OrderType_StockOrder
//				&&!g_pBizModel->GetApsParam()->ParseHaveDesignOrderRelCondition(0)) //¼��ɹ�����
//			{
//				return true;
//			}
//		}
//	}
//	YKOrderPtr nextOrder =inPtr->GetOrder();// GetNextOrderPtr();
//	YKOrderPtr peakOrder = nextOrder->GetPeakOrder();
//
//	if(!(peakOrder!= NULL))	peakOrder = nextOrder;
//
//	if(peakOrder->GetType() == OrderType_WillOrder
//		|| nextOrder->GetType() == OrderType_WillOrder) 	
//	{
//		if (preOrder->GetType()==OrderType_AbsOrder
//			&&!g_pBizModel->GetApsParam()->ParseWillOrderRelCondition(0))
//		{
//			return true;
//		}
//		if (preOrder->GetType()==OrderType_RelOrder
//			&&!g_pBizModel->GetApsParam()->ParseWillOrderRelCondition(1))
//		{
//			return true;
//		}
//		if (preOrder->GetDiffer()==OrderDiffer_Hand&&preOrder->GetType()==OrderType_PudOrder
//			&&!g_pBizModel->GetApsParam()->ParseWillOrderRelCondition(3)) //¼�����충��4
//		{
//			return true;
//		}
//		if (preOrder->GetDiffer()==OrderDiffer_Hand&&preOrder->GetType()==OrderType_StockOrder
//			&&!g_pBizModel->GetApsParam()->ParseWillOrderRelCondition(2)) //¼��ɹ�����
//		{
//			return true;
//		}   	
//	}
//	if (preOrder->GetExcptFlg()!=1
//		||nextOrder->GetExcptFlg()!=1)
//	{
//		return true;
//	}
//	YKItemPtr itemPtr=inPtr->GetItem();//GetItemPtr();
//	if (itemPtr!=NULL)
//	{
//		YK_SHORT autoSupply =itemPtr->GetAutoSupply();  //�Զ������־
//		if (autoSupply==AutoFlag_YesOne        /*��1��1���죩*/
//			// 			||autoSupply==AutoFlag_YesStOne        /* �ǣ����+1��1���죩    */
//			// 			||autoSupply==AutoFlag_YesChangeOne
//			)  /* �ǣ��������1��1���죩 */
//		{
//			YKOrderPtr preOrder = outPtr->GetOrder();//GetPreOrderPtr();
//			if (preOrder->GetDiffer()==OrderDiffer_Hand)
//			{
//				return true;
//			}
//		}
//		//get/set	���Ϸ�ʽ
//		YK_SHORT  stockMode =itemPtr->GetStockMode();  //���Ϸ�ʽ
//		if (stockMode==	StockMode_No)          /* ����     */
//		{
//			return true;
//		}
//		if (stockMode==	StockMode_BuyPre)          /* �ɹ�����     */
//		{
//			if (preOrder->GetDiffer()==OrderDiffer_Auto&&preOrder->GetType()==OrderType_PudOrder) //�Զ��������충��
//			{
//				return true;
//			}
//		}
//		if (stockMode==	StockMode_InsidePre)          /* ��������     */
//		{
//			if (preOrder->GetDiffer()==OrderDiffer_Auto&&preOrder->GetType()==OrderType_StockOrder) //�Զ�����ɹ�����
//			{
//				return true;
//			}
//		}
//		if (stockMode==	StockMode_Buy)          /* �ɹ�     */
//		{
//			if (preOrder->GetType()==OrderType_PudOrder) //���충��
//			{
//				return true;
//			}
//		}
//		if (stockMode==	StockMode_Inside)          /* ����     */
//		{
//			if (preOrder->GetType()==OrderType_StockOrder) //�ɹ�����
//			{
//				return true;
//			}
//		}
//	}
//	YK_BOOL ifCyc=false;
//	YK_ULONG orderId=preOrder->GetId();
//	YK_ULONG itemId=preOrder->GetPudId();
//	preOrder->IfHaveReationCyc(orderId,ifCyc);
//	//preOrder->IfHaveReationCyc(preOrder->GetPudId(),ifCyc);
//	if (!ifCyc)		
//	return inPtr->MatchItemRepalceReation();
//	//return MatchItemRepalceReation();
//	return false;
//}
////�����������
//YK_DOUBLE ApsCal::Stock(YKItemPtr& itemPtr,list<YKWorkOutputItemPtr>& workOutputItemList1)
//{
//	if (itemPtr->GetAutoSupply()==AutoFlag_Yes
//		||itemPtr->GetAutoSupply()==AutoFlag_YesChangeOne)
//	{
//		YK_FLOAT minStockCount  = itemPtr->GetMinStock();
//		list<YKWorkOutputItemPtr>::iterator iter=workOutputItemList1.begin();
//		for (;iter!=workOutputItemList1.end();)
//		{
//			YKWorkOutputItemPtr&outPtr=*iter;
//			if (NULL!=outPtr)
//			{
//				m_totalAmout-=outPtr->GetSpilthAmount();
//				m_totalAmoutMap[itemPtr->GetId()]-=outPtr->GetSpilthAmount();
//				if (minStockCount>=outPtr->GetSpilthAmount())
//				{
//					minStockCount=minStockCount-outPtr->GetSpilthAmount();
//					iter=workOutputItemList1.erase(iter);
//					if (minStockCount==0)
//						return 0;
//				}
//				else
//					return outPtr->GetSpilthAmount()-minStockCount;
//
//			}
//		}
//	}
//	return 0;
//}
//1��1����
void ApsCal::EquitySupply(YKItemPtr&itemPtr,list<YKWorkInputItemPtr>&workInputItemList)
{
	//���м�Ʒ��������
	if(itemPtr->GetAutoSupply() == AutoFlag_No) //���Զ�����,�˳�
		return;
	if (!itemPtr->IfValid())
		return ;  //��ǰ��Ʒ���Ϸ�

	for(list<YKWorkInputItemPtr>::iterator iterIItem =workInputItemList.begin()
		;iterIItem!=workInputItemList.end();iterIItem++)
	{
		YKWorkInputItemPtr&inPtr=*iterIItem;
		if (NULL==inPtr||BZERO(inPtr->GetScarcityAmount()))
			continue;
		if (itemPtr->GetId()!=inPtr->GetInputItemId())
			continue;

		if (itemPtr->GetAutoSupply() > AutoFlag_Yes)  //1��1��������
		{
			EquitySupply(itemPtr, inPtr);
		}else
		{
			YKOrderPtr nexOrder =inPtr->GetOrder();
			YKOrderPtr peakOrder = nexOrder->GetPeakOrder();

			if(peakOrder == NULL)	peakOrder = nexOrder;

			if(nexOrder->PeakDesignationOrders()
			   ||nexOrder->PeakWillOrders()) //�������췬�Ż����򶩵�
			{
				EquitySupply(itemPtr, inPtr);

			}
		}
	}
	//for(list<YKWorkInputItemPtr>::iterator iterIItem =workInputItemList.begin()
	//	;iterIItem!=workInputItemList.end();)
	//{
	//	YKWorkInputItemPtr&inPtr=*iterIItem;
	//	if (NULL==inPtr)
	//	{
	//		iterIItem = workInputItemList.erase(iterIItem);
	//		continue;
	//	}
	//	if (BZERO(inPtr->GetScarcityAmount()))
	//	{
	//		iterIItem++;
	//		continue;
	//	}
	//	/*if (NULL==inPtr||BZERO(inPtr->GetScarcityAmount()))
	//	{
	//		iterIItem = workInputItemList.erase(iterIItem);
	//		continue;
	//	}
	//	YKItemPtr itemPtr=inPtr->GetItem();
	//	if (NULL==itemPtr)
	//	{
	//		iterIItem = workInputItemList.erase(iterIItem);
	//		continue;
	//	}*/
	//	if (itemPtr->GetId()!=inPtr->GetInputItemId())
	//	{
	//		iterIItem++;
	//		continue;
	//	}
	//	if (!itemPtr->IfValid())
	//	{
	//		iterIItem++;
	//		continue ;  //��ǰ��Ʒ���Ϸ�
	//	}
	//	//���м�Ʒ��������
	//	if(itemPtr->GetAutoSupply() == AutoFlag_No) //���Զ�����,�˳�
	//	{
	//		iterIItem = workInputItemList.erase(iterIItem);
	//		continue;
	//	}
	//	if (itemPtr->GetAutoSupply() > AutoFlag_Yes)  //1��1��������
	//	{
	//		EquitySupply(itemPtr, inPtr);
	//	}else
	//	{
	//		YKOrderPtr nexOrder =inPtr->GetOrder();
	//		YKOrderPtr peakOrder = nexOrder->GetPeakOrder();

	//		if(peakOrder == NULL)	peakOrder = nexOrder;

	//		if(/*peakOrder != NULL
	//			&&(peakOrder->GetDesignation() != L""
	//			||peakOrder->GetType() == OrderType_WillOrder)*/
	//			nexOrder->PeakDesignationOrders()
	//			||nexOrder->PeakWillOrders()) //�������췬�Ż����򶩵�
	//		{
	//			EquitySupply(itemPtr, inPtr);

	//		}
	//	}
	//	if(BZERO(inPtr->GetScarcityAmount()))		//�ж������������Ƿ�Ϊ0
	//	{
	//		iterIItem = workInputItemList.erase(iterIItem);
	//		continue;
	//	}else if (iterIItem==workInputItemList.end())
	//	{
	//		break;
	//	}
	//	iterIItem++;
	//}
}
list<YKWorkInputItemPtr> ApsCal::GetWorkInList(YKItemPtr&itemPtr,list<YKWorkInputItemPtr>&workInputItemList)
{

	list<YKWorkInputItemPtr>inList;


	YK_ULONG itemId=itemPtr->GetId();
	CNewRuleManager rulManager;
	rulManager.SetInfo(RULE_DEF_SCH_FILTER_Item,g_pBizModel->GetApsParam()->GetItemFiterRule(),TblItem);
	if(!rulManager.GetBoolValue(itemId))  //��Ʒɸѡ
		return inList;

	list<YKWorkInputItemPtr>::iterator iter=workInputItemList.begin();
	for (;iter!=workInputItemList.end();iter++)
	{
		YKWorkInputItemPtr&inPtr=*iter;
		if (NULL!=inPtr&&inPtr->GetInputItemId()==itemId)
		{
			YKOrder nxtOrder=inPtr->GetOrder();
			if (NULL != nxtOrder /*&& !nxtOrder->CheckItemCyc(itemPtr->GetId())*/) //û�й���ѭ��
			{
				inList.push_back(inPtr);
			}
		}
	}
	return inList;
}
list<YKWorkOutputItemPtr> ApsCal::GetWorkOutList(YKItemPtr&itemPtr,list<YKWorkOutputItemPtr>&workOutputItemList)
{
	list<YKWorkOutputItemPtr>outList;
	YK_ULONG itemId=itemPtr->GetId();
	list<YKWorkOutputItemPtr>::iterator iter=workOutputItemList.begin();
	for (;iter!=workOutputItemList.end();iter++)
	{
		YKWorkOutputItemPtr&outPtr=*iter;
		if (NULL!=outPtr&&outPtr->GetOutputItemId()==itemId)
		{
			outList.push_back(outPtr);
			//iter=workOutputItemList.erase(iter);
		}
		/*else
			iter++;*/
	}
	return outList;
}

//����
bool ApsCal::SortWorkOutItem( YKWorkOutputItemPtr& workOutPtrF,YKWorkOutputItemPtr& workOutPtrS )
{
	return workOutPtrF.GetTime() < workOutPtrS.GetTime();
}

////һ��һ��棬�������̶����Զ�������Ϊ�ɹ����ϵĿ��
void ApsCal::AllotAutoToStock(list<YKWorkInputItemPtr>& workInputItemList,list<YKWorkOutputItemPtr>& workOutputItemList1,list<YKWorkOutputItemPtr>& workOutputItemList2)
{
	//1���ҳ��й�����ϵ����ǰ��������Ϊ�Զ����䶩���ĵ�������Ʒ
	//2��ɸѡ����Ʒ���ڣ�����Ʒ���Ϸ�ʽΪ���һ��һ��������Ʒ
	list<YKWorkInputItemPtr> workInputItemPtr;
	for ( list<YKWorkInputItemPtr>::iterator fItor = workInputItemList.begin();fItor != workInputItemList.end(); fItor++ )
	{
		YKWorkInputItemPtr& workInputPtr = *fItor;
		if ( workInputPtr != NULL )
		{
			list<YKWorkRelationPtr> workRelaPtrList;
			workInputPtr->GetRelationPtrList(workRelaPtrList);		//��������Ʒ - ��ȡ������ϵ
			for ( list<YKWorkRelationPtr>::iterator wrItor = workRelaPtrList.begin(); wrItor != workRelaPtrList.end();wrItor++ )
			{
				YKWorkRelationPtr& workRelaPtr = *wrItor;
				if ( workRelaPtr != NULL )
				{
					YKOrderPtr preRelOrderPtr = workRelaPtr->GetPreOrderPtr();
					if ( preRelOrderPtr != NULL )
					{
						if ( preRelOrderPtr->GetDiffer() == OrderDiffer_Auto )	//��������  �Զ�����
						{
							YKItemPtr  preRelItemPtr = preRelOrderPtr->GetItem();
							if ( preRelOrderPtr != NULL )
							{
								if ( preRelItemPtr->GetAutoSupply() == AutoFlag_YesStOne )	//���+1��1����
								{
									workInputItemPtr.push_back(workInputPtr);
								}
							}
						}						
					}
				}
			}
		}
	}




	for ( list<YKWorkInputItemPtr>::iterator relItor =   workInputItemPtr.begin(); relItor != workInputItemPtr.end();relItor++ )
	{
		YK_DOUBLE fSpiltAmount(0);
		YK_DOUBLE fDemandAmount(0);

		YKWorkInputItemPtr& relInputItemPtr= *relItor;
		if ( relInputItemPtr != NULL )
		{
			//3������������Ʒ��ѡ������һ�����������Ʒ���ҵ���Ӧ����Ʒ��ͬ����������Ϊ��棬�Ҷ�����������0�������Ʒ�б�
			//4����3����ɸѡ���������Ʒ����Ŀǰ��ʱ������
			list<YKWorkOutputItemPtr> workOutItemList;

			for ( list<YKWorkOutputItemPtr>::iterator workOutItor = workOutputItemList1.begin();workOutItor != workOutputItemList1.end(); workOutItor++ )
			{
				YKWorkOutputItemPtr& workOutItemPtr = *workOutItor;
				if ( workOutItemPtr != NULL )
				{
					if ( relInputItemPtr.GetInputItemId() == workOutItemPtr->GetOutputItemId() )	//���� ��� ��Ʒ��ͬ
					{
						YKOrderPtr workOutOrderPtr = workOutItemPtr->GetOrder();
						if ( workOutOrderPtr != NULL )
						{
							YK_SHORT nType = workOutOrderPtr->GetType();
							if ( nType == OrderType_RelOrder || nType == OrderType_AbsOrder )
							{
								if ( workOutItemPtr->GetSpilthAmount() > PRECISION )
								{
									workOutItemList.push_back(workOutItemPtr);									
								}
							}
						}
					}					
				}
			}

			if(workOutItemList.empty())
				continue;

			workOutItemList.sort(SortWorkOutItem);

			//5�����������Ʒ�б��������������֮�͡�	
			for ( list<YKWorkOutputItemPtr>::iterator workOutitor = workOutItemList.begin(); workOutitor != workOutItemList.end(); ++workOutitor )
			{
				YKWorkOutputItemPtr& workOutputItemPtr = *workOutitor;
				fSpiltAmount += workOutputItemPtr->GetSpilthAmount();		//��������
			}

			fDemandAmount = relInputItemPtr->GetScarcityAmount();	//��������

			//6�������������ʹ�����������δ������ʱʱ���������ԭ����Ӧ�Ĺ�����ϵ,
			if ( (fSpiltAmount - fDemandAmount) > PRECISION )
			{
				list<YKWorkRelationPtr> relationList;
				relInputItemPtr->GetRelationPtrList(relationList);

				for(list<YKWorkRelationPtr>::iterator iter = relationList.begin();iter != relationList.end();++iter)
				{
					YKWorkOutputItem workOut = iter->GetWorkOutputItemPtr();
					iter->DeleteWorkRelationEntire();
					if(find(workOutputItemList2.begin(),workOutputItemList2.end(),workOut) == workOutputItemList2.end())
						workOutputItemList2.push_back(workOut);
				}
			}

			fDemandAmount = relInputItemPtr->GetScarcityAmount();	//��������
			if(fDemandAmount < PRECISION)	continue;

			//7������������Ʒ�������������б�֮��Ĺ�����ϵ��
			RelationStock(relInputItemPtr,workOutItemList);

		}
	}



}

void ApsCal::AllotStock(list<YKWorkInputItemPtr>&workInputItemList,list<YKWorkOutputItemPtr>&workOutputItemList1,list<YKWorkOutputItemPtr>& workOutputItemList2)
{
	m_minStckAmoutList.insert(m_minStckAmoutList.end(),workOutputItemList1.begin(),workOutputItemList1.end());
	m_minStckAmoutList.sort(SortWorkOutPutList);         //�������С������
	AllotMinStock();  //��С���Ĵ���

	//һ��һ��棬�������̶����Զ�������Ϊ�ɹ����ϵĿ��
	AllotAutoToStock(workInputItemList,workOutputItemList1,workOutputItemList2);

	//�������
	ReationStock(workInputItemList,workOutputItemList1);

	AllotMinTargetStock(m_minStckAmoutList); //�Ƚ��ֶ�¼����״̬�Ķ���������С��ȫ���
	
	AllotMinTargetStock(m_minTarStckAmoutList); //��С��ȫ��� - ��ȥ��СĿ����
}
//��ȥ��С���ͬʱ����ȥĿ����
void ApsCal::AllotMinStock()
{
	list<YKWorkOutputItemPtr>::iterator iter=m_minStckAmoutList.begin();  //������
	for (;iter!=m_minStckAmoutList.end();iter++)
	{
		YKWorkOutputItemPtr&outPtr=*iter;
		if (NULL!=outPtr&&outPtr->GetSpilthAmount()>0)
		{
			YK_ULONG nItemId = outPtr->GetOutputItemId();
			map<YK_ULONG,YK_DOUBLE>::iterator iIter=m_minStckAmoutMap.find(nItemId);
			if (iIter!=m_minStckAmoutMap.end())
			{
				YK_DOUBLE& amount=iIter->second;
				YK_FLOAT fSplitAmount = outPtr->GetSpilthAmount();
				YK_FLOAT fStockAmount = outPtr->GetStockAmount();
				YK_DOUBLE sAmount=min(amount,fSplitAmount);

				if (fSplitAmount>=amount)
				{
					outPtr->SetStockAmount(fStockAmount+amount);
					outPtr->SetSpilthAmount(fSplitAmount-amount);
					YKWorkPtr workPtr = outPtr->GetWork();
					if ( workPtr != NULL )
						workPtr->AllotStockAmount();   //���ÿ�����ֽ⵽�ӹ�����				
					m_minStckAmoutMap.erase(iIter);	
				}
				else
				{
					amount -= fSplitAmount;
					outPtr->SetStockAmount(fStockAmount+fSplitAmount);
					outPtr->SetSpilthAmount(0);
					YKWorkPtr workPtr = outPtr->GetWork();
					if ( workPtr != NULL )
						workPtr->AllotStockAmount();   //���ÿ�����ֽ⵽�ӹ�����		
				}
				map<YK_ULONG,YK_DOUBLE>::iterator niIter=m_minTarStckAmoutMap.find(nItemId);
				if (niIter!=m_minTarStckAmoutMap.end())
				{
					/*YK_DOUBLE&amount=niIter->second;
					amount-=sAmount;
					if (amount<=0)
						m_minTarStckAmoutMap.erase(niIter);*/

					//Add 2012-05-14 
					YK_DOUBLE amount=niIter->second;
					amount-=sAmount;
					if (amount<0)
					{
						m_minTarStckAmoutMap.erase(niIter);						
					}else if ( amount > 0 )
					{
						m_minTarStckAmoutMap[nItemId] = amount;
					}
				}
			}
		}
	}
}
//��ȥ��С���ͬʱ����ȥĿ����
void ApsCal::AllotMinTargetStock(list<YKWorkOutputItemPtr>&minTarStkList)
{
	list<YKWorkOutputItemPtr>::iterator iter=minTarStkList.begin();  //������
	for (;iter!=minTarStkList.end();iter++)
	{
		YKWorkOutputItemPtr&outPtr=*iter;
		if (NULL!=outPtr&&outPtr->GetSpilthAmount()>0)
		{
			map<YK_ULONG,YK_DOUBLE>::iterator iIter=m_minTarStckAmoutMap.find(outPtr->GetOutputItemId());
			if (iIter!=m_minTarStckAmoutMap.end())
			{
				YK_DOUBLE& amount=iIter->second;
				YK_FLOAT fSpiltAmount = outPtr->GetSpilthAmount();
				YK_FLOAT fStockAmount = outPtr->GetStockAmount();
				YK_DOUBLE sAmount=min(amount,outPtr->GetSpilthAmount());

				if (fSpiltAmount>=amount)
				{
					outPtr->SetStockAmount(fStockAmount+amount);
					outPtr->SetSpilthAmount(fSpiltAmount-amount);
					YKWorkPtr workPtr = outPtr->GetWork();
					if ( workPtr != NULL )
						workPtr->AllotStockAmount();   //���ÿ�����ֽ⵽�ӹ�����		
					m_minTarStckAmoutMap.erase(iIter);
				}
				else
				{
					amount -= outPtr->GetSpilthAmount();
					outPtr->SetStockAmount( fStockAmount+fSpiltAmount );
					outPtr->SetSpilthAmount(0);
					YKWorkPtr workPtr = outPtr->GetWork();
					if ( workPtr != NULL )
						workPtr->AllotStockAmount();   //���ÿ�����ֽ⵽�ӹ�����		
				}
			}
		}
	}
}
//������СĿ����
void ApsCal::AllotMinTargetStock(YKItemPtr& itemPtr,list<YKWorkOutputItemPtr>&outStockList)
{
	list<YKWorkOutputItemPtr>::iterator iter=outStockList.begin();  //������
	for (;iter!=outStockList.end();iter++)
	{
		YKWorkOutputItemPtr&outPtr=*iter;
		if (NULL!=outPtr&&outPtr->GetSpilthAmount()>0&&itemPtr->GetId()==outPtr->GetOutputItemId())
		{
			YKOrderPtr orderPtr=outPtr->GetOrder();
			if (NULL!=orderPtr&&orderPtr->GetExcptFlg()==1)
			{
				map<YK_ULONG,YK_DOUBLE>::iterator iIter=m_minTarStckAmoutMap.find(outPtr->GetOutputItemId());
				if (iIter!=m_minTarStckAmoutMap.end())
				{
					YK_DOUBLE& amount=iIter->second;
					YK_FLOAT fSpiltAmount = outPtr->GetSpilthAmount();
					YK_FLOAT fStockAmount = outPtr->GetStockAmount();
					YK_DOUBLE sAmount=min(amount,outPtr->GetSpilthAmount());

					if (outPtr->GetSpilthAmount()>=amount)
					{
						outPtr->SetStockAmount( fStockAmount+amount );
						outPtr->SetSpilthAmount( fSpiltAmount-amount );
						YKWorkPtr workPtr = outPtr->GetWork();
						if ( workPtr != NULL )
							workPtr->AllotStockAmount();   //���ÿ�����ֽ⵽�ӹ�����		

						m_minTarStckAmoutMap.erase(iIter);
					}
					else
					{
						amount -= outPtr->GetSpilthAmount();
						outPtr->SetStockAmount( fStockAmount+fSpiltAmount );
						outPtr->SetSpilthAmount(0);
						YKWorkPtr workPtr = outPtr->GetWork();
						if ( workPtr != NULL )
							workPtr->AllotStockAmount();   //���ÿ�����ֽ⵽�ӹ�����		
					}
				}
			}
		}
	}
}
//�󶩵��Ĵ���
void ApsCal::InitBackOrderRelation()
{
	m_backOrderMap.clear();
	g_pBizModel->BackOrderRelation(m_backOrderMap);
}
void ApsCal::UpdateBackOrderRelation(list<YKItemPtr>&itemPtrList)
{
	if (m_backOrderMap.empty())
	{
		return;
	}
	list<YKOrderPtr>deOrderList;  //���ֶ�¼�����ڹ���״̬�Ķ������ᱻɾ��
	for (list<YKItemPtr>::iterator iter=itemPtrList.begin();iter!=itemPtrList.end();iter++)
	{
		YKItemPtr&itemPtr=*iter;
		map<YK_ULONG,list<YKOrderPtr>>::iterator iiter=m_backOrderMap.find(itemPtr->GetId());
		if (iiter!=m_backOrderMap.end())
		{
			list<YKOrderPtr>&orderPtrList=iiter->second;
			list<YKOrderPtr>::iterator iIter=orderPtrList.begin();
			for (;iIter!=orderPtrList.end();iIter++)
			{
				/*if( ( (*iIter)->GetType()==OrderType_SellOrder && !BIZAPI::CheckLimit( LimitOfAuthor_Sell ))
					||( (*iIter)->GetType()==OrderType_StockOrder && !BIZAPI::CheckLimit( LimitOfAuthor_Stock ) ) )
					continue;*/
				UpdateBackOrderRelation(*iIter,deOrderList);
			}
			m_backOrderMap.erase(itemPtr->GetId());
		}
	}
	list<YKOrderPtr>::iterator iIter=deOrderList.begin();
	for (;iIter!=deOrderList.end();iIter++)
	{
		YKOrderPtr&orderPtr=*iIter;
		if (orderPtr!=NULL&&orderPtr->DelCondition())
		{
			if (orderPtr->GetType() == OrderType_StockOrder ) //�ɹ�����
			{
				AddDeAutoStockOrderList(orderPtr->GetId());
			}
			else if (orderPtr->GetType() == OrderType_PudOrder )  //���춨��
			{
				AddDeAutoPudOrderList(orderPtr->GetId());
			}
			orderPtr->Delete();
		}
	}
}
void ApsCal::UpdateBackOrderRelation(YKOrderPtr&orderPtr,list<YKOrderPtr>&deOrderList)
{
	list<NextOrderOper>& nextOrderList = orderPtr->GetNextOrderList();
	for(list<NextOrderOper>::iterator iter = nextOrderList.begin();
		iter != nextOrderList.end();iter++)
	{

		YKOrderPtr& nextOrder = g_pBizModel->GetYKOrderPtrMap()->Get(iter->nextOrderId);

		if(nextOrder == NULL||(nextOrder->GetDiffer()==OrderType_MTOrder))//ά���������乤�����ɺ��ٽ����󶩵��Ĺ���
			continue;

		if(nextOrder->GetExcptFlg()!=1) continue;

		/*if( ( nextOrder->GetType()==OrderType_SellOrder && !BIZAPI::CheckLimit( LimitOfAuthor_Sell ))
			||( nextOrder->GetType()==OrderType_StockOrder && !BIZAPI::CheckLimit( LimitOfAuthor_Stock ) ) )
			continue;*/


		// ������ĩ����
		YKWorkPtr preWorkPtr = orderPtr->GetTailOperationWork();
		if(preWorkPtr == NULL)	return;
		YKWorkPtr nextWorkPtr = nextOrder->GetWork(iter->operCode);
		if(nextWorkPtr == NULL)	
		{
			// �������׹���
			nextWorkPtr = nextOrder->GetHeadOperationWork();
			if(nextWorkPtr == NULL)
				continue;
		}					

		YK_ULONG inId=0;
		YK_ULONG outId=0;
		YKWorkRelation pWorkRelation; pWorkRelation.NewTemp();

		//����
		pWorkRelation->SetAmount(0);	
		pWorkRelation->SetRelaTye(RelationType_Back);					//����

		//�����ֶ�
		pWorkRelation->SetPrevOrderId(orderPtr->GetId());			//ǰ����ID
		pWorkRelation->SetNextOrderId(nextOrder->GetId());		//�󶨵�ID
		pWorkRelation->SetPrevWorkId(preWorkPtr->GetId());
		pWorkRelation->SetNextWorkId(nextWorkPtr->GetId());

		bool hasRealRela = false;
		//������Ʒ������ϵ
		list<YKWorkInputItemPtr> workInList;
		nextWorkPtr->GetWorkInList(workInList);
		list<YKWorkOutputItemPtr> workOutList; 
		preWorkPtr->GetWorkOutList(workOutList);
		//����������������Ҷ�Ӧ��Ʒ
		for(list<YKWorkInputItemPtr>::iterator iIter = workInList.begin();
			iIter != workInList.end();iIter++)
		{
			for(list<YKWorkOutputItemPtr>::iterator oIter = workOutList.begin();
				oIter != workOutList.end();oIter++)
			{
				YKWorkInputItemPtr& workIn = *iIter;
				YKWorkOutputItemPtr& workOut = *oIter;
				if(workIn->GetInputItemId() > 0
					&&workIn->GetInputItemId() == workOut->GetOutputItemId())
				{
					CNewRuleManager rulManager;
					rulManager.SetInfo(RULE_DEF_SCH_FILTER_Item,g_pBizModel->GetApsParam()->GetItemFiterRule(),TblItem);
					if(!rulManager.GetBoolValue(workIn->GetInputItemId()))  //��Ʒɸѡ
						break;

					rulManager.SetInfo(RULE_DEF_FILTER_Order,g_pBizModel->GetApsParam()->GetOrderFiter(),TblOrder);
					if(!rulManager.GetBoolValue(workIn->GetOrderId()))  //����ɸѡ
						continue;

					if (!workIn->GetActualWorkRecursionFlg()||!workIn->IfReationForResultWork())
						break;
					if(g_pBizModel->IsRelafixInVal(workIn,workOut)/*orderPtr->IsRelafixInVal(nextOrder,workIn)*/)  //�����̶���Ч
						break;
					InforransferAndOrderRenovate(workIn,workOut);
					if (orderPtr->GetExcptFlg()!=1)
					{
						deOrderList.push_back(orderPtr);
						break;
					}
					workIn->ItemRepalce();   //����ϴ���
					if (BZERO(workIn->GetScarcityAmount()))
					{

					}
					YK_FLOAT workCount = min(workIn->GetScarcityAmount(),workOut->GetSpilthAmount());
					if(workCount>0.000001)	// change by ll 2010-1-20  ���������¼�Ĳ�������>0 ���������¼�Ķ�������>0
					{
						orderPtr->PreOrderLateFinishTimeCalValue(nextOrder);  	//���ݹ�������깤ʱ�̼���ֵ
						pWorkRelation->SetAmount(workCount);
						pWorkRelation->SetInputItemId(workIn->GetId());
						pWorkRelation->SetOutputItemId(workOut->GetId());
						pWorkRelation->SetItemId(workIn->GetInputItemId());
						workIn->SetScarcityAmount(workIn->GetScarcityAmount() - workCount);
						workOut->SetSpilthAmount(workOut->GetSpilthAmount() - workCount);

						inId=workIn->GetId();
						outId=workOut->GetId();
						workIn->SumUpRepalceAmount();   //�������
						hasRealRela = true;
						break;
					}
					else
						workIn->SumUpRepalceAmount();   //�������
				}
			}
		}

		if (orderPtr->GetExcptFlg()!=1)
		{
			pWorkRelation.FreeTemp();
			return;
		}

		if(!hasRealRela)
		{
			if (NULL==preWorkPtr)
			{
				pWorkRelation.FreeTemp();
				continue;
			}
			YK_ULONG visualId = g_pBizModel->GetMaxId();  //������ƷID
			//�½���������������
			YKWorkInputItem workIn; workIn.New();
			workIn.SetScarcityAmount(0);
			workIn.SetInputItemId(visualId);
			workIn.SetPlantAmount(0);
			workIn.SetWorkId(nextWorkPtr->GetId());
			workIn.SetOrderId(nextOrder->GetId());		// add 2010-1-20
			YK_ULONG workInid = workIn.GetId();
			nextWorkPtr->AddWorkInputItemPtr(g_pBizModel->GetYKWorkInputItemPtrMap()->Get(workInid));
			workIn.CreateSonIn();

			//�½��������������
			YKWorkOutputItem workOut; workOut.New();
			workOut.SetSpilthAmount(0);
			workOut.SetOutputItemId(visualId);
			workOut.SetPlantAmount(0);
			workOut.SetWorkId(preWorkPtr->GetId());
			workOut.SetOrderId(orderPtr->GetId());	// add 2010-1-20
			const YK_ULONG workOutid = workOut.GetId();
			preWorkPtr->AddWorkOutputItemPtr(g_pBizModel->GetYKWorkOutputItemPtrMap()->Get(workOutid));
			workOut.CreateSonOut();

			pWorkRelation->SetAmount(0);
			pWorkRelation->SetInputItemId(workIn.GetId());
			pWorkRelation->SetOutputItemId(workOut.GetId());
			pWorkRelation->SetItemId(visualId);
			workIn.SetScarcityAmount(0);
			workOut.SetSpilthAmount(0);
			inId=workIn.GetId();
			outId=workOut.GetId();

		}

		//�����С
		pWorkRelation->SetMaxVal(iter->maxInterVal);
		pWorkRelation->SetMinVal(iter->minInterVal);

		//	SetPdRoutingId(pdRouting);			//��Ʒ����·��Id
		pWorkRelation->SetFixFlg(RelationFixFlg_Fix);						//���ù̶�

		//���������ϵ��
		const YK_ULONG id = pWorkRelation->Insert();//g_pBizModel->InsertBiz(pWorkRelation);
		if(id > 0)
		{
			YKWorkRelationPtr& wPtr = g_pBizModel->GetYKWorkRelationPtrMap()->Get(id);
			//���¶�����������Ӧ������
			//ǰ����
			orderPtr->AddNxtRelationList(wPtr);
			//�󶨵�
			nextOrder->AddPreRelationList(wPtr);

			preWorkPtr->AddNextRelationPtr(wPtr);
			nextWorkPtr->AddPreRelationPtr(wPtr);

			YKWorkInputItemPtr nInPtr= g_pBizModel->GetYKWorkInputItemPtrMap()->Get(inId);
			YKWorkOutputItemPtr nOutPtr= g_pBizModel->GetYKWorkOutputItemPtrMap()->Get(outId);
			if (NULL!=nInPtr)
			{
				nInPtr->AddRelationPtr(id);
			}
			if (NULL!=nOutPtr)
			{
				nOutPtr->AddRelationPtr(id);
			}
		}
	}
}

//�������Ʒ��Ŷ����Ĺ���
void ApsCal::InOutByDesignation(YKWorkInputItemPtr&inPtr,list<YKWorkOutputItemPtr>&workOutputItemList0)
{
	if (NULL==inPtr)
		return ;
	inPtr->ItemRepalce();   //��������������������롣
	if (inPtr->GetCmdCode().empty())
	{
		RelationStock(inPtr,workOutputItemList0,true);
	}
	else
	{	
		list<YKWorkInputItemPtr>cmdComList=inPtr->GetCmdCodeCombList();
		if (cmdComList.size()==1)
		{
			RelationStock(inPtr,workOutputItemList0,true);
		}
		else
		{
			YKWorkInputItemPtr aInPtr=inPtr->GetReationAllReplace(cmdComList);//����	����������
			if (NULL!=aInPtr)
			{
				if (aInPtr->GetId()==inPtr->GetId())
					RelationStock(aInPtr,workOutputItemList0,true);
				else  if (aInPtr->GetItemReplaceType()==ItemReplaceType_Party)
				{
					RelationStock(inPtr,workOutputItemList0,true);
				}
			}	
			else
			{
				RelationStock(inPtr,workOutputItemList0,true);
				if (inPtr->GetScarcityAmount()>0)  //������������0 ��ȫ���
				{
					if (inPtr->GetItemReplaceType()==ItemReplaceType_All)
					{
						inPtr->DeleteAllReplaceReation(m_nWkRePtrList);
					}
				}
			}			
		}
	}
	inPtr->SumUpRepalceAmount();//��᲻������������ 
}

//δչ���ļ�������չ��
void ApsCal::SpreadMuster(list<YKWorkOutputItemPtr>&newWorkOutList)
{
	list<YKWorkOutputItemPtr>::iterator iter = newWorkOutList.begin();
	for ( ; iter != newWorkOutList.end() ; iter++)
	{
		YKWorkOutputItemPtr & outPtr = *iter;
		if ( outPtr != NULL )
		{
			YKOrderPtr orderPtr = outPtr->GetOrder() ;
			if ( orderPtr != NULL &&  orderPtr->IfPeakOrder() ) //�޺��������Ҫ�Ըö�����������չ��
			{
				orderPtr->UpdateIOItem();
				orderPtr->FlashOrderState();				 
				if(orderPtr->GetExcptFlg()!=1)
				{
					//���������Ϣ
					if (orderPtr->GetExcptFlg()==Spread_No_Bom)
						AddFailItemBomList( orderPtr->GetPudId());    //���һ��BOM��Ч��Ϣ
					if (orderPtr->GetType() == OrderType_PudOrder )  //���춨��
					{
						AddFialPudOrderList(orderPtr->GetId());
					}
					orderPtr->Delete();
					break;
				}
			}
		}
	}
}

//Add 2011-12-09 ��������ƥ�乤�����벢��ù�������
void ApsCal::GetWorkInItemListByOperInItem(YKOperationPtr& operPtr,YKOrderPtr& order,YKWorkInputItemPtr& workInputItemPtr,list<YKWorkInputItemPtr>& workInputItemList)
{
	//�ɹ�����ù���������Ʒ�б�
	list<YKWorkInputItemPtr>inList;

	//��ʼ��-����б�
	workInputItemList.clear();

	map<YK_WSTRING,list<YKOperInputItemPtr>> inUseTypeMap;	//��������
	map<YK_WSTRING,list<YKOperInputItemPtr>> inMidItemMap;	//�м�Ʒ

	operPtr->FilterOperInputItemPtrList( inUseTypeMap,inMidItemMap,order->GetCalLastEndTime(), order->GetId());
	
	//��ȡ��ͬʹ�����͵�List
	list<YKOperInputItemPtr> tempOperInputList;
	map<YK_WSTRING,list<YKOperInputItemPtr>>::iterator inUseTypeMapItor = inUseTypeMap.find(workInputItemPtr.GetCmdCode());
	if (inUseTypeMapItor != inUseTypeMap.end())
	{
		tempOperInputList = inUseTypeMapItor->second;
	}	

	//���tempOperInputListList�е�һ��Ԫ��
	YKOperInputItemPtr firstOperInputItem;
	if (tempOperInputList.size() > 0)
	{
		firstOperInputItem = tempOperInputList.front();
	}	

	if (workInputItemPtr != NULL)
	{		
		YKWorkPtr work = workInputItemPtr->GetWork();	//�ɹ��������ù���

		work->GetWorkInList(inList);	//�ɹ�����ù��������б�
	}

	if (inList.size() > 0)
	{		
		for (list<YKOperInputItemPtr>::iterator tempItor = tempOperInputList.begin();tempItor != tempOperInputList.end();tempItor++)
		{
			YKOperInputItemPtr tempOperInput = *tempItor;

			if (tempOperInput != NULL)
			{
				for (list<YKWorkInputItemPtr>::iterator workInputItemItor = inList.begin(); workInputItemItor != inList.end();workInputItemItor++)
				{
					YKWorkInputItemPtr tempWorkInputItemPtr = *workInputItemItor;

					if (tempWorkInputItemPtr != NULL )
					{
						if (tempWorkInputItemPtr->GetInputItemId() == tempOperInput->GetInputItemId())
						{
							//�ж��б����Ƿ���ڣ�������ִ����Ӳ���
							if (workInputItemList.size() > 0)
							{

								//��ѯworkInputItemList ���Ƿ������ֵͬ, ���ǣ���ִ������
								if ( find( workInputItemList.begin(),workInputItemList.end(),tempWorkInputItemPtr) != workInputItemList.end() )
								{
									continue;
								}
							}

							workInputItemList.push_back(tempWorkInputItemPtr);
						}
					}
				}				
			}					
		}			
			
	}
}

//Add 2011-12-09 ִ���������
void ApsCal::ItemReplaceByWorkInItemList( list<YKWorkOutputItemPtr>& workOutputItemList, list<YKWorkInputItemPtr>& workInputItemList )
{
	if (workInputItemList.size() > 0)
	{	
		YKWorkInputItemPtr wInItemPtr =  workInputItemList.front();
		if (wInItemPtr != NULL)
		{			
			//�������
			YK_SHORT repType(ItemReplaceType_No);
			repType= wInItemPtr->GetItemReplaceType(); 

			//��ȫ���
			if( ItemReplaceType_All == repType )
			{
				//���������ϵ
				DelWrokInRelation( workInputItemList,workOutputItemList,true );

				//�������ϵĹ���
				RelationStock(wInItemPtr,workOutputItemList);	

				//����ִ�С�����ȫ���
				ExecuteAllReplace( workOutputItemList,workInputItemList );
			}
			else if ( ItemReplaceType_Party == repType ) //�������
			{
				//���������ϵ
				DelWrokInRelation( workInputItemList,workOutputItemList,true );

				//����ִ�С����������
				ExecutePartReplace( workOutputItemList,workInputItemList );
			}
			else if (ItemReplaceType_No == repType)	//�����
			{
				//���������ϵ
				DelWrokInRelation( workInputItemList,workOutputItemList,false );

				//�������ϵĹ���
				RelationStock(wInItemPtr,workOutputItemList);	
			}
		}	
	}
}

//Add 2011-12-12 ����������������ϵ
void ApsCal::DelWrokInRelation( list<YKWorkInputItemPtr>& workInputItemList,list<YKWorkOutputItemPtr>& workOutputItemList,bool mMeIsCanDelete )
{
	//Add 2011-12-12 ���������ϵ
	for ( list<YKWorkInputItemPtr>::iterator workInputPtrItor = workInputItemList.begin(); workInputPtrItor != workInputItemList.end();workInputPtrItor++ )
	{
		YKWorkInputItemPtr wkInPtr = *workInputPtrItor;

		if (wkInPtr != NULL)
		{		
			//����������Ʒ��ù��������б�
			list<YKWorkRelationPtr> relationList; 
			wkInPtr->GetRelationPtrList(relationList);

			if ( relationList.size() > 0 )
			{
				for ( list<YKWorkRelationPtr>::iterator realItor = relationList.begin(); realItor != relationList.end();realItor++ )
				{
					YKWorkRelationPtr workRealPtr = *realItor;

					if ( workRealPtr != NULL )
					{
						//�ɹ�����ϵ��������Ʒ
						YKWorkOutputItemPtr workOutPutPtr = workRealPtr->GetWorkOutputItemPtr();
						YKWorkInputItemPtr workInPutPtr = workRealPtr->GetWorkInputItemPtr();

						if ( workOutPutPtr != NULL || workInPutPtr != NULL )
						{
							//��ȡ����
							YKWorkInputItemPtr mainMatelPtr = workInputItemList.front();

							if (mainMatelPtr != NULL)
							{
								//����
								if (wkInPtr == mainMatelPtr)
								{
									//��mMeIsCanDelete Ϊflase ʱ��ֻ�帨��
									if (!mMeIsCanDelete)
									{										
										continue;
									}	
									else
									{
										if (workOutPutPtr->GetId() == wkInPtr->GetId())
										{
											//������Ʒ�Ĳ�������
											wkInPtr->SetScarcityAmount(wkInPtr->GetScarcityAmount()+workRealPtr->GetAmount());	

											//�����Ʒ��������
											workOutPutPtr->SetSpilthAmount(workOutPutPtr->GetSpilthAmount() + workRealPtr->GetAmount());
											//��������б�����ֵ��ԭֵ����ͬ����ִ�в���
											UpdateWorkOutPutItemList(workOutPutPtr,workOutputItemList);

											//ɾ��������ϵ
											workRealPtr->DeleteWorkRelationEntire();											
										}
									}
								}
								else  //����
								{	
									//�ɸ�������ת��Ϊ��������
									YK_DOUBLE  mainMetalNum = wkInPtr->GetPlantAmount()/wkInPtr->GetItemReplaceRate();

									mainMatelPtr->SetPlantAmount( mainMatelPtr->GetPlantAmount() + mainMetalNum );	

									//������Ʒ�������
									mainMatelPtr->SetItemReplaceAmount( mainMatelPtr->GetItemReplaceAmount() - mainMetalNum );

									//����������������Ϊ0
									wkInPtr->SetPlantAmount(0);

									//�����Ʒ��������
									workOutPutPtr->SetSpilthAmount( workOutPutPtr->GetSpilthAmount() + workRealPtr->GetAmount() );
									//��������б�����ֵ��ԭֵ����ͬ����ִ�в���
									UpdateWorkOutPutItemList(workOutPutPtr,workOutputItemList);

									//ɾ��������ϵ
									workRealPtr->DeleteWorkRelationEntire();
								}
							}								
						}								
					}						
				}					
			}		
		}
	}
}

//Add 2011-12-12 ���¹�������б�,����ֵ��ԭֵ����ͬ����ִ�в���
void ApsCal::UpdateWorkOutPutItemList( YKWorkOutputItemPtr& wOItemPtr,list<YKWorkOutputItemPtr>& workOutputItemList )
{
	if (wOItemPtr != NULL)
	{
		bool isCanInsert = true;

		for ( list<YKWorkOutputItemPtr>::iterator wOItemItor = workOutputItemList.begin(); wOItemItor != workOutputItemList.end();wOItemItor++ )
		{
			YKWorkOutputItemPtr tempWOI = *wOItemItor;

			if (tempWOI != NULL)
			{
				if ( tempWOI == wOItemPtr )
				{
					isCanInsert = false;
				}
			}
		}

		if (isCanInsert)
		{
			workOutputItemList.push_back(wOItemPtr);
		}
	}
}

//Add 2011-12-12 ����ִ�С�����ȫ���
void ApsCal::ExecuteAllReplace( list<YKWorkOutputItemPtr>& workOutputItemList, list<YKWorkInputItemPtr>& workInputItemList )
{
	if ( workInputItemList.size() >0 && workOutputItemList.size() > 0 )
	{
		//���ϵ���������
		YK_DOUBLE  mItemAllCount(0.0);

		for (list<YKWorkInputItemPtr>::iterator wInPutItor = workInputItemList.begin(); wInPutItor != workInputItemList.end(); wInPutItor++)
		{
			//��ʱ�洢������ȫ������������Ʒ��¼
			list<YKWorkOutputItemPtr> tempWOutItemList;

			YKWorkInputItemPtr workInputItemPtr = *wInPutItor;
			
			if (workInputItemPtr != NULL)
			{
				//�������ͬ��Ʒ�� ���������
				YK_DOUBLE  nACount(0.0);

				//��ʼ��
				tempWOutItemList.clear();

				for( list<YKWorkOutputItemPtr>::iterator wOIItor = workOutputItemList.begin(); wOIItor != workOutputItemList.end();wOIItor++ )
				{
					YKWorkOutputItemPtr wOutputItemPtr = *wOIItor;
					if (wOutputItemPtr != NULL)
					{
						if (workInputItemPtr->GetItem() == wOutputItemPtr->GetItem())
						{
							nACount += wOutputItemPtr->GetSpilthAmount();

							tempWOutItemList.push_back(wOutputItemPtr);
						}
					}
				}	

				//
				YKWorkInputItemPtr mInPutItemPtr = workInputItemList.front();
				if (mInPutItemPtr != NULL)
				{
					//������������
					mItemAllCount = mInPutItemPtr->GetScarcityAmount();	

					//������������
					if (workInputItemPtr->GetInputItemId() != mInPutItemPtr->GetInputItemId())
					{
						nACount = nACount/workInputItemPtr->GetItemReplaceRate();		//���Ͽ��������������
					}
					if ( mItemAllCount - nACount > PRECISION)	//��������ȫ���
					{
						continue;
					}
					else	//ִ����ȫ�������
					{
						//ִ����ȫ�������
						AllReplaceMethod( mInPutItemPtr,workInputItemPtr,tempWOutItemList,mItemAllCount );	
					}
				}				
			}
		}
	}
}
//Add 2011-12-12 ����ִ�С����������
void ApsCal::ExecutePartReplace( list<YKWorkOutputItemPtr>& workOutputItemList, list<YKWorkInputItemPtr>& workInputItemList )
{
	if ( workInputItemList.size() >0 && workOutputItemList.size() > 0 )
	{
		for (list<YKWorkInputItemPtr>::iterator wInputItemItor = workInputItemList.begin(); wInputItemItor != workInputItemList.end(); wInputItemItor++)
		{
			//��ʱ�洢���в�������������Ʒ��¼
			list<YKWorkOutputItemPtr> tempWOutItemList;

			YKWorkInputItemPtr workInPutPtr = *wInputItemItor;
			
			if (workInPutPtr != NULL)
			{
				//�������ͬ��Ʒ�� ���������
				YK_DOUBLE  nACount(0.0);

				//��ʼ��
				tempWOutItemList.clear();

				//�������Ʒ��Ϣ ����
				for( list<YKWorkOutputItemPtr>::iterator wOIItor = workOutputItemList.begin(); wOIItor != workOutputItemList.end();wOIItor++ )
				{
					YKWorkOutputItemPtr wOutputItemPtr = *wOIItor;

					if ( wOutputItemPtr != NULL )
					{
						if ( wOutputItemPtr->GetItem() == workInPutPtr->GetItem() )
						{
							nACount += wOutputItemPtr->GetSpilthAmount();

							tempWOutItemList.push_back(wOutputItemPtr);
						}
					}
				}	

				//������������
				YK_DOUBLE mItemAllCount(0.0);

				YKWorkInputItemPtr mInputPtr = workInputItemList.front();
				if ( mInputPtr != NULL )
				{
					//������������
					mItemAllCount = mInputPtr->GetScarcityAmount();

					if (workInPutPtr->GetInputItemId() != mInputPtr->GetInputItemId())
					{
						//�����������
						nACount = nACount/workInPutPtr->GetItemReplaceRate();
					}

					//�ж�ִ�� �������
					if ( !BZERO(nACount) && !BZERO(mItemAllCount))
					{
						//������������ С�� �����Ʒ����
						if ( nACount-mItemAllCount > PRECISION || BZERO(nACount-mItemAllCount) )
						{
							//����ȫ������д���
							YK_DOUBLE tempAcount = min(mItemAllCount,nACount);
							AllReplaceMethod(mInputPtr,workInPutPtr,tempWOutItemList,tempAcount);
						}
						else	//ִ�в������
						{
							//�ж��Ƿ������ϣ����ǣ������������
							if (mInputPtr->GetInputItemId() == workInPutPtr->GetInputItemId())
							{
								//���������������
								mInputPtr->SetItemReplaceAmount(0);
							}
							else
							{
								//�������ϵĲ����������
								YK_DOUBLE nItemReAcount = mInputPtr->GetItemReplaceAmount();
								YK_DOUBLE nRepAmount = min( mInputPtr->GetScarcityAmount(),nACount );
								mInputPtr->SetItemReplaceAmount( nItemReAcount+nRepAmount );	
								mInputPtr->SetScarcityAmount(mInputPtr->GetScarcityAmount() - nRepAmount);

								//���ϵ���������		
								YK_DOUBLE fItemReplaceAmount = nACount*workInPutPtr->GetItemReplaceRate();
								workInPutPtr->SetPlantAmount(fItemReplaceAmount);		//���ϼƻ���������
								workInPutPtr->SetScarcityAmount(fItemReplaceAmount);	//���ϵ���������


								// �ڸ����� ���ñ����������
								workInPutPtr->SetReplaceMateriel(BIZMODELTOOLS::IToWString(mInputPtr->GetInputItemId(),10));
							}					

							for (list<YKWorkOutputItemPtr>::iterator  wOItemIor = tempWOutItemList.begin(); wOItemIor != tempWOutItemList.end();wOItemIor++)
							{
								YKWorkOutputItemPtr tempWOPtr = *wOItemIor;
								if (tempWOPtr != NULL)
								{
									//�����������ж��Ƿ�ִ���´ν�������
									if ( BZERO(workInPutPtr->GetScarcityAmount()) )	//����������Ϊ0ʱ��ֹͣ��������
									{
										return;
									}

									YK_DOUBLE tempAmount = min(workInPutPtr->GetScarcityAmount(),tempWOPtr->GetSpilthAmount());							

									//����������ϵ
									Input2Output( workInPutPtr,tempWOPtr, tempAmount);	
								}
							}
						}
					}	
				}						
			}
		}
	}
}

//Add 2011-12-14 ��������ʱ����ȫ�������
void ApsCal::AllReplaceMethod( YKWorkInputItemPtr mInPutItemPtr,YKWorkInputItemPtr workInputItemPtr,list<YKWorkOutputItemPtr>& tempWOutItemList,YK_DOUBLE mItemAllCount )
{
	//�ж��Ƿ�Ϊ���ϣ����ǣ����ø���ֵ
	if( mInPutItemPtr->GetInputItemId() != workInputItemPtr->GetInputItemId() )
	{
		//��������
		mInPutItemPtr->SetScarcityAmount(0);	//��������(��������)����Ϊ0
		mInPutItemPtr->SetItemReplaceAmount( mItemAllCount + mInPutItemPtr->GetItemReplaceAmount());	

		//���ø���
		YK_DOUBLE fItemReplaceAmount = mItemAllCount * workInputItemPtr->GetItemReplaceRate();
		workInputItemPtr->SetPlantAmount( fItemReplaceAmount );		//���ϼƻ���������
		workInputItemPtr->SetScarcityAmount(fItemReplaceAmount);	//������������

		//�ڸ����� ���ñ����������
		workInputItemPtr->SetReplaceMateriel(BIZMODELTOOLS::IToWString(mInPutItemPtr->GetInputItemId(),10));
	}

	for (list<YKWorkOutputItemPtr>::iterator tempWOItor = tempWOutItemList.begin(); tempWOItor != tempWOutItemList.end(); tempWOItor++)
	{
		YKWorkOutputItemPtr tempWorkOutPutPtr = *tempWOItor;		  

		if (tempWorkOutPutPtr != NULL)
		{	
			if (workInputItemPtr->GetInputItemId() == tempWorkOutPutPtr->GetOutputItemId())
			{
				//�����������ж��Ƿ�ִ���´ν�������
				if ( BZERO(workInputItemPtr->GetScarcityAmount()) )	//����������Ϊ0ʱ��ֹͣ��������
				{
					return;
				}

				YK_DOUBLE tempAmount = min(workInputItemPtr->GetScarcityAmount(),tempWorkOutPutPtr->GetSpilthAmount());							

				//����������ϵ
				Input2Output( workInputItemPtr,tempWorkOutPutPtr, tempAmount);	
			}						
		}

	}
}


static bool SortOrderByLastCompleteTime(YKOrder& first,YKOrder& second)
{
	if(first == NULL && second == NULL)
		return false;
	else if(first != NULL && second == NULL)
		return true;
	else if(first == NULL && second != NULL)
		return false;
	else
	{
		YK_TIME_T firTm = first.GetLastCompleteTime();
		YK_TIME_T secTm = second.GetLastCompleteTime();
		if(firTm == 0 && secTm != 0)
			return false;
		else if(firTm != 0 && secTm == 0)
			return true;
		else
			return firTm < secTm;

	}
}

struct PSIunit
{
public:
	float m_demand;			//����
	float m_accommodate;		//��Ӧ
	float m_balance;			//ƽ��

	PSIunit():m_demand(0),m_accommodate(0),m_balance(0){}
	PSIunit(float demand,float accommodate,float balance)
	{m_demand = demand;m_accommodate = accommodate;m_balance = balance;}
};

map<YK_ULONG , map<time_t,PSIunit> >	g_PSIMap;

map<YK_ULONG,map<YK_ULONG,map<time_t,PSIunit>>> 	g_PSIClientMap;		//Add 2012-05-07 ���PSI�ͻ�

//��ȡʱ��鿪ʼʱ��
time_t GetTimeBoundStTm(time_t tme)
{
	if(tme == 0 || tme >= MAXTIME)
		tme = g_pBizModel->GetShowEndTime();
	return CDealTime(tme).GetDate().GetTime();
}

long GetTimeBoundLength()
{
	return 86400;
}

wstring GetTimeText( YK_TIME_T tme )
{

	if ( 0 == tme ) return _T("");

	const INT maxTimeBufferSize = 128;

	static LPCTSTR pFormat = L"%m/%d";

	TCHAR szBuffer[maxTimeBufferSize];

	struct tm ptmTemp;
	errno_t err = _localtime64_s(&ptmTemp, &tme);

	if (err != 0 || !_tcsftime(szBuffer, maxTimeBufferSize, pFormat, &ptmTemp))
	{
		szBuffer[0] = '\0';
	}

	return szBuffer;
}

void OutPutPSIData()
{
	time_t stTm = GetTimeBoundStTm(g_pBizModel->GetShowStartTime());
	time_t endTm = GetTimeBoundStTm(g_pBizModel->GetShowEndTime());

	long timeBoundLength = GetTimeBoundLength();
	string filename = "C:\\Program Files\\YUKON\\PSI.txt";
	std::ofstream writefile(filename.c_str(), std::ofstream::out/*|std::fstream::app*/);
	if(writefile)
	{
		//��ӡ��ͷ
		writefile<<"��Ʒ|PSI|���";
		for(time_t tme = stTm;tme<=endTm;tme+=timeBoundLength)
		{
			writefile  <<"|" << TOOLS::W2A_S(GetTimeText(tme)).c_str();
		}
		writefile<<endl;

		//Add 2012-05-03
		YKPSIPtrMap* psiPtrMap = g_pBizModel->GetYKPSIPtrMap();
		if ( psiPtrMap != NULL )
		{
			YK_UINT nCount(0);
			writefile << "\n\t\tPSI��ʼ\n";
			for ( psiPtrMap->Begin();psiPtrMap->NotEnd();psiPtrMap->Step() )
			{
				++nCount;
				YKPSI psiPtr = psiPtrMap->Get();
				if ( psiPtr != NULL )
				{
					YK_WSTRING itemCode;
					YK_WSTRING itemId = psiPtr->GetPSIItem();
					YK_ULONG nItemId = _wtol(itemId.c_str());
					YKItemPtr itemPtr = g_pBizModel->GetYKItemPtrMap()->Get(nItemId);
					if ( itemPtr != NULL )
					{
						itemCode = itemPtr->GetCode();
					}
					writefile<<"��"<<nCount<<"�У�"<<"  PSI��Ʒ:" <<TOOLS::W2A_S(itemCode)<<"\tPSI�ͻ�:" <<TOOLS::W2A_S(psiPtr->GetPSIClient())<<"\tPSI����:" <<TOOLS::W2A_S(psiPtr->GetPSIDate())<<"\tPSI����:" <<psiPtr->GetPSIAcount()<<"\tPSI����:" <<psiPtr->GetPSINumber()<<endl;
				}
			}
			writefile << "\t\tPSI����\n\n";
		}

		//��ӡ��Ʒ
		for(map<YK_ULONG , map<time_t,PSIunit> >::iterator iter = g_PSIMap.begin();
			iter != g_PSIMap.end();iter++)
		{
			YKItem item = g_pBizModel->GetYKItemPtrMap()->Get(iter->first);
			if(item != NULL)
			{
				for(int i = 0;i<3;i++)
				{
					if(i == 0)
					{
						writefile<<TOOLS::W2A_S(item->GetCode()).c_str()<<"|������| ";
						for(time_t tme = stTm;tme<=endTm;tme+=timeBoundLength)
						{
							writefile << "|" << g_PSIMap[iter->first][tme].m_demand;
						}
						writefile<<endl;
					}
					else if(i == 1)
					{
						writefile<<TOOLS::W2A_S(item->GetCode()).c_str()<<"|��Ӧ��|";
						writefile<< g_PSIMap[iter->first][0].m_accommodate;
						for(time_t tme = stTm;tme<=endTm;tme+=timeBoundLength)
						{
							writefile << "|" << g_PSIMap[iter->first][tme].m_accommodate;
						}
						writefile<<endl;
					}
					else if(i == 2)
					{
						writefile<<TOOLS::W2A_S(item->GetCode()).c_str()<<"|ƽ����| ";
						for(time_t tme = stTm;tme<=endTm;tme+=timeBoundLength)
						{
							writefile << "|" << g_PSIMap[iter->first][tme].m_balance;
						}
						writefile<<endl;
					}
				}
			}
		}
	}

	writefile.close();
	writefile.clear();
}

//����Ԥ�ƶ���������
void ApsCal::CalcForecastOrder()
{
	//����������ȡ�����۶�����Ԥ�ƶ���������Ʒ���࣬������������
	map<YK_ULONG,list<YKOrder>>	saleOrders,forecastOrders,stockOrders;
	g_PSIMap.clear();
	
	YKOrderPtrMap* pOrderMap = g_pBizModel->GetYKOrderPtrMap();
	for(pOrderMap->Begin();pOrderMap->NotEnd();pOrderMap->Step())
	{
		YKOrder order = pOrderMap->Get();

		if(order.GetType() == OrderType_SellOrder)
			saleOrders[order.GetPudId()].push_back(order);
		else if(order.GetType() == OrderType_Forecast)
			forecastOrders[order.GetPudId()].push_back(order);
		else if(order.GetType() == OrderType_AbsOrder)
			stockOrders[order.GetPudId()].push_back(order);
	}

	//����
	for(map<YK_ULONG,list<YKOrder>>::iterator iter = saleOrders.begin();
		iter != saleOrders.end();iter++)
	{
		list<YKOrder>& saleList = iter->second;
		saleList.sort(SortOrderByLastCompleteTime);

		for(list<YKOrder>::iterator iterSale = saleList.begin();iterSale != saleList.end();iterSale++)
		{
			g_PSIMap[iter->first][GetTimeBoundStTm(iterSale->GetLastCompleteTime())].m_demand = iterSale->GetAmount();
		}

	}

	//����Ԥ�ƶ�������
	for(map<YK_ULONG,list<YKOrder>>::iterator iter = forecastOrders.begin();
		iter != forecastOrders.end();iter++)
	{
		//����
		list<YKOrder>& forecastList = iter->second;
		forecastList.sort(SortOrderByLastCompleteTime);
		for(list<YKOrder>::iterator iterFore = forecastList.begin();iterFore != forecastList.end();iterFore++)
		{
			if(BZERO(iterFore->GetAlreadyAssignQuantity()))
				iterFore->SetAlreadyAssignQuantity(iterFore->GetAmount());
			//��ԭ����
			iterFore->SetAmount(iterFore->GetAlreadyAssignQuantity());
			if(!BZERO(iterFore->GetAmount()))
			{
				iterFore->SetValid(true);
				g_PSIMap[iter->first][GetTimeBoundStTm(iterFore->GetLastCompleteTime())].m_accommodate = iterFore->GetAlreadyAssignQuantity();
			}
		}

		float stockCount = 0;
		//�����Ӧ�����
		for(list<YKOrder>::iterator iterStock = stockOrders[iter->first].begin();
			iterStock != stockOrders[iter->first].end();iterStock++)
		{
			stockCount += iterStock->GetAmount();
		}

		g_PSIMap[iter->first][0].m_accommodate = stockCount;
		g_PSIMap[iter->first][0].m_balance = stockCount;

	}

	//����ǰ��Ĳ��֣�����ƽ��
	for(map<YK_ULONG , map<time_t,PSIunit> >::iterator iterPSI = g_PSIMap.begin();
		iterPSI != g_PSIMap.end();iterPSI++)
	{
		map<time_t,PSIunit>& psiList = iterPSI->second;

		//map<time_t,PSIunit>::iterator iter1 = psiList.begin();
		//map<time_t,PSIunit>::iterator iter2 = iter1;
		//if(iter1 != psiList.end())
		//	iter2++;

		//for(;iter2 != psiList.end();iter1++,iter2++)
		//{
		//	iter2->second.m_balance = iter1->second.m_balance + iter2->second.m_accommodate - iter2->second.m_demand;
		//}
		time_t stTm = GetTimeBoundStTm(g_pBizModel->GetShowStartTime());
		time_t endTm = GetTimeBoundStTm(g_pBizModel->GetShowEndTime());

		long timeBoundLength = GetTimeBoundLength();

		time_t tm1 = stTm;
		psiList[tm1].m_balance = psiList[0].m_accommodate + psiList[tm1].m_accommodate - psiList[tm1].m_demand;
		time_t tm2 = tm1 + timeBoundLength;
		for(;tm2<=endTm;tm2+=timeBoundLength,tm1 += timeBoundLength)
		{			
			psiList[tm2].m_balance = psiList[tm1].m_balance + psiList[tm2].m_accommodate - psiList[tm2].m_demand;
		}
	}

	//����ƽ������ÿ��Ԥ�ƶ���ֵ
	//����Ԥ�ƶ������� ����
	for(map<YK_ULONG,list<YKOrder>>::iterator iter = saleOrders.begin();
		iter != saleOrders.end();iter++)
	{
		//ȡ���
		float stockCount = g_PSIMap[iter->first][0].m_accommodate;

		float demandCount = 0;

		list<YKOrder>& saleList = iter->second;

		list<YKOrder>& forecastList = forecastOrders[iter->first];

		for(list<YKOrder>::iterator iterSale = saleList.begin();iterSale != saleList.end();iterSale++)
		{
			if(stockCount - iterSale->GetAmount() > PRECISION)
				stockCount = stockCount - iterSale->GetAmount();
			else
			{
				demandCount += iterSale->GetAmount() - stockCount;
				stockCount = 0;
			}

			if(!BZERO(demandCount))
			{
				for(list<YKOrder>::iterator iterFore = forecastList.begin();iterFore != forecastList.end();iterFore++)
				{
					//if(GetTimeBoundStTm(iterFore->GetLastCompleteTime()) > GetTimeBoundStTm(iterSale->GetLastCompleteTime())) 
					//	break;

					if((iterFore->GetAmount() - demandCount) > PRECISION)
					{
						iterFore->SetAmount(iterFore->GetAmount() - demandCount);
						demandCount = 0;
					}
					else
					{
						demandCount = demandCount - iterFore->GetAmount();
						iterFore->SetAmount(0);
						//iterFore->SetValid(false);
					}

					if(BZERO(demandCount))
						break;
				}
			}
		}
	}
}

void ApsCal::CalcPSItable()
{
	g_PSIMap.clear();

	YKOrderPtrMap* pOrderMap = g_pBizModel->GetYKOrderPtrMap();
	for(pOrderMap->Begin();pOrderMap->NotEnd();pOrderMap->Step())
	{
		YKOrder order = pOrderMap->Get();

		if(order.GetType() == OrderType_AbsOrder)
		{
			g_PSIMap[order.GetPudId()][0].m_accommodate = order.GetAmount();
		}
	}

	//�����������ӹ�����ȡ�������
	YKWorkPtrMap* pWorkMap = g_pBizModel->GetYKWorkPtrMap();
	for(pWorkMap->Begin();pWorkMap->NotEnd();pWorkMap->Step())
	{
		YKWork work = pWorkMap->Get();
		//�ӹ���ȡ��������
		list<YKWorkInputItem>	workInList;
		list<YKWorkOutputItem>	workOutList;
		work->GetWorkInList(workInList);
		work->GetWorkOutList(workOutList);

		//�������룬����
		for(list<YKWorkInputItem>::iterator iterIn = workInList.begin();
			iterIn != workInList.end();iterIn++)
		{
			YKWorkInputItem& workIn = *iterIn;

			if(workIn != NULL && workIn.GetInputItemId() > 0)
			{
				time_t tm1 = workIn->GetTime();
				if(tm1 == 0)
					tm1 = work->GetPlantStartTm();
				
				g_PSIMap[workIn.GetInputItemId()][GetTimeBoundStTm(tm1)].m_demand += workIn.GetPlantAmount();				
			}
		}

		//�������
		for(list<YKWorkOutputItem>::iterator iterOut = workOutList.begin();
			iterOut != workOutList.end();iterOut++)
		{
			YKWorkOutputItem& workOut = *iterOut;
			if(workOut != NULL && workOut.GetOutputItemId() > 0)
			{
				time_t tm1 = workOut->GetTime();
				if(tm1 == 0)
					tm1 = work->GetPlantCompleteTm();

				g_PSIMap[workOut.GetOutputItemId()][GetTimeBoundStTm(tm1)].m_accommodate += workOut.GetPlantAmount();
			}
		}

	}

	//����ǰ��Ĳ��֣�����ƽ��
	for(map<YK_ULONG , map<time_t,PSIunit> >::iterator iterPSI = g_PSIMap.begin();
		iterPSI != g_PSIMap.end();iterPSI++)
	{
		map<time_t,PSIunit>& psiList = iterPSI->second;
		time_t stTm = GetTimeBoundStTm(g_pBizModel->GetShowStartTime());
		time_t endTm = GetTimeBoundStTm(g_pBizModel->GetShowEndTime());

		long timeBoundLength = GetTimeBoundLength();

		time_t tm1 = stTm;
		psiList[tm1].m_balance = psiList[0].m_accommodate + psiList[tm1].m_accommodate - psiList[tm1].m_demand;
		time_t tm2 = tm1 + timeBoundLength;
		for(;tm2<=endTm;tm2+=timeBoundLength,tm1 += timeBoundLength)
		{			
			psiList[tm2].m_balance = psiList[tm1].m_balance + psiList[tm2].m_accommodate - psiList[tm2].m_demand;				
		}
	}

	//��ӡPSI����
	OutPutPSIData();
}

//Add 2012-05-03 ����PSI ����YKPSIPtrMap ��
void ApsCal::CalcPSIInPSIPtrMap()
{
	g_PSIMap.clear();
	list<YK_ULONG> objIdForDel;
	YKPSIPtrMap* ptrMap = g_pBizModel->GetYKPSIPtrMap();
	if ( ptrMap != NULL )
	{
		ptrMap->Clear();
	}

	YKOrderPtrMap* pOrderMap = g_pBizModel->GetYKOrderPtrMap();
	for(pOrderMap->Begin();pOrderMap->NotEnd();pOrderMap->Step())
	{
		YKOrder order = pOrderMap->Get();
		if(order.GetType() == OrderType_AbsOrder)
		{
			YK_ULONG nPudId = order.GetPudId(); 
			g_PSIMap[nPudId][0].m_accommodate = order.GetAmount();
		}
	}

	//�����������ӹ�����ȡ�������
	YKWorkPtrMap* pWorkMap = g_pBizModel->GetYKWorkPtrMap();
	for(pWorkMap->Begin();pWorkMap->NotEnd();pWorkMap->Step())
	{
		YKWork work = pWorkMap->Get();
		//�ӹ���ȡ��������
		list<YKWorkInputItem>	workInList;
		list<YKWorkOutputItem>	workOutList;
		work->GetWorkInList(workInList);
		work->GetWorkOutList(workOutList);

		//�������룬����
		for(list<YKWorkInputItem>::iterator iterIn = workInList.begin();
			iterIn != workInList.end();iterIn++)
		{
			YKWorkInputItem& workIn = *iterIn;

			if(workIn != NULL && workIn.GetInputItemId() > 0)
			{
				time_t tm1 = workIn->GetTime();
				if(tm1 == 0)
					tm1 = work->GetPlantStartTm();
				YK_ULONG inputPudId = workIn.GetInputItemId();
				time_t inputTimeSt = GetTimeBoundStTm(tm1);
				g_PSIMap[inputPudId][inputTimeSt].m_demand += workIn.GetPlantAmount();	

				//���ݹ��� ��ȡ����
				YKOrderPtr orderPtrByWork = work->GetOrder();
				if ( orderPtrByWork != NULL )
				{
					YKPSI pPsiPtr;
					pPsiPtr->New();
					if ( pPsiPtr != NULL )
					{
						pPsiPtr->SetPSIItem( TOOLS::IToWString(inputPudId,10) );	//��ƷID
						pPsiPtr->SetPSIClient( TOOLS::IToWString(orderPtrByWork->GetClientId(),10) );	//�ͻ�ID
						pPsiPtr->SetPSIDate( TOOLS::IToWString(inputTimeSt,10) );	//����
						objIdForDel.push_back(pPsiPtr->GetId() );
					}
				}			
			}
		}

		//�������
		for(list<YKWorkOutputItem>::iterator iterOut = workOutList.begin();
			iterOut != workOutList.end();iterOut++)
		{
			YKWorkOutputItem& workOut = *iterOut;
			if(workOut != NULL && workOut.GetOutputItemId() > 0)
			{
				YK_ULONG outPudId = workOut.GetOutputItemId();
				time_t tm1 = workOut->GetTime();
				if(tm1 == 0)
					tm1 = work->GetPlantCompleteTm();
				time_t outputTimeSt = GetTimeBoundStTm(tm1);
				g_PSIMap[outPudId][outputTimeSt].m_accommodate += workOut.GetPlantAmount();

				//���ݹ��� ��ȡ����
				YKOrderPtr outPutOrderPtrByWork = work->GetOrder();
				if ( outPutOrderPtrByWork != NULL )
				{
					YKPSI pPsiPtr;
					pPsiPtr->New();
					if ( pPsiPtr != NULL )
					{
						pPsiPtr->SetPSIItem( TOOLS::IToWString(outPudId,10));		//��ƷID
						pPsiPtr->SetPSIClient( TOOLS::IToWString(outPutOrderPtrByWork->GetClientId(),10) );		//�ͻ�ID
						pPsiPtr->SetPSIDate( TOOLS::IToWString(outputTimeSt,10) );	//����
						objIdForDel.push_back( pPsiPtr->GetId() );
					}
				}		
			}
		}
	}

	//����ǰ��Ĳ��֣�����ƽ��
	for(map<YK_ULONG , map<time_t,PSIunit> >::iterator iterPSI = g_PSIMap.begin();
		iterPSI != g_PSIMap.end();iterPSI++)
	{
		YKPSI* pPSIPtr(NULL);
		YK_ULONG nPudItemId = iterPSI->first;
		YKPSIPtrMap* pPSIPtrMap = g_pBizModel->GetYKPSIPtrMap();
		if ( pPSIPtrMap != NULL )
		{
			for ( pPSIPtrMap->Begin();pPSIPtrMap->NotEnd();pPSIPtrMap->Step() )
			{
				YKPSI tpPSIPtr = pPSIPtrMap->Get();
				if ( tpPSIPtr != NULL )
				{
					YK_WSTRING itemIdstr = tpPSIPtr->GetPSIItem();
					if ( nPudItemId == _wtol( itemIdstr.c_str()) )
					{
						pPSIPtr = &(*tpPSIPtr);
						break;
					}
				}
			}
		}	

		map<time_t,PSIunit>& psiList = iterPSI->second;
		time_t stTm = GetTimeBoundStTm(g_pBizModel->GetShowStartTime());
		time_t endTm = GetTimeBoundStTm(g_pBizModel->GetShowEndTime());
		long timeBoundLength = GetTimeBoundLength();
		time_t tm1 = stTm;
		psiList[tm1].m_balance = psiList[0].m_accommodate + psiList[tm1].m_accommodate - psiList[tm1].m_demand;
		time_t tm2 = tm1 + timeBoundLength;

		if ( pPSIPtr != NULL )
		{
			YKPSI tpPsiPtr;
			tpPsiPtr->New();
			if ( tpPsiPtr != NULL )
			{
				tpPsiPtr->SetPSIItem( pPSIPtr->GetPSIItem() );			//��Ʒ
				tpPsiPtr->SetPSIClient( pPSIPtr->GetPSIClient() );		//�ͻ�
				tpPsiPtr->SetPSIDate( TOOLS::IToWString(tm1,10) );		//����	
				tpPsiPtr->SetPSIAcount( pPSIPtr->GetPSIAcount() +  psiList[tm1].m_balance );		//����
				tpPsiPtr->SetPSINumber( pPSIPtr->GetPSINumber() + psiList[tm1].m_balance-psiList[0].m_balance );		//����
			}	
		}

		for(;tm2<=endTm;tm2+=timeBoundLength,tm1 += timeBoundLength)
		{			
			psiList[tm2].m_balance = psiList[tm1].m_balance + psiList[tm2].m_accommodate - psiList[tm2].m_demand;

			if ( pPSIPtr != NULL )
			{
				YKPSI tpPsiPtr;
				tpPsiPtr->New();
				if ( tpPsiPtr != NULL )
				{
					tpPsiPtr->SetPSIItem( pPSIPtr->GetPSIItem() );			//��Ʒ
					tpPsiPtr->SetPSIClient( pPSIPtr->GetPSIClient() );		//�ͻ�
					tpPsiPtr->SetPSIDate( TOOLS::IToWString(tm2,10) );		//����
					tpPsiPtr->SetPSIAcount( tpPsiPtr->GetPSIAcount() + psiList[tm2].m_balance  );		//����
					tpPsiPtr->SetPSINumber( tpPsiPtr->GetPSINumber() + psiList[tm2].m_balance-psiList[tm1].m_balance );		//����
				}							
			}				
		}
	}

	//ɾ������������PSI����
	for ( list<YK_ULONG>::iterator fItor = objIdForDel.begin(); fItor != objIdForDel.end(); fItor++ )
	{
		YK_ULONG objId = *fItor;
		YKPSI ptrObj = g_pBizModel->GetYKPSIPtrMap()->Get(objId);
		if ( ptrObj != NULL )
		{
			ptrObj.Delete();
		}
	}

	//��ӡPSI����
	OutPutPSIData();
}

//Add 2012-05-04 ���ݶ����� ��Ӧ��=�����ͻ� ����PSI
void ApsCal::CalcPSIInPSIPtrMapByClient()
{
	g_PSIMap.clear();
	g_PSIClientMap.clear();
	list<YK_ULONG> objIdForDel;
	YKPSIPtrMap* ptrMap = g_pBizModel->GetYKPSIPtrMap();
	if ( ptrMap != NULL )
	{
		ptrMap->Clear();
	}
	
	

	/*YKOrderPtrMap* pOrderMap = g_pBizModel->GetYKOrderPtrMap();
	for(pOrderMap->Begin();pOrderMap->NotEnd();pOrderMap->Step())
	{
		YKOrder order = pOrderMap->Get();
		if(order.GetType() == OrderType_AbsOrder)
		{
			YK_ULONG nPudId = order.GetPudId(); 
			YK_ULONG nClientId = order.GetClientId();
			g_PSIClientMap[nPudId][nClientId][0].m_accommodate = order.GetAmount();
		}
	}*/

	//�����������ӹ�����ȡ�������
	map<YK_ULONG,map<time_t,PSIunit>> objNTimeMap; 		//����-ʱ�� ����

	map<YK_ULONG,map<time_t,PSIunit>> workInputNTimeMap; 		//����-ʱ�� ����

	YKWorkPtrMap* pWorkMap = g_pBizModel->GetYKWorkPtrMap();
	for(pWorkMap->Begin();pWorkMap->NotEnd();pWorkMap->Step())
	{
		YKWork work = pWorkMap->Get();
		//�ӹ���ȡ��������
		list<YKWorkInputItem>	workInList;
		list<YKWorkOutputItem>	workOutList;
		work->GetWorkInList(workInList);
		work->GetWorkOutList(workOutList);

		//�������룬����
		for(list<YKWorkInputItem>::iterator iterIn = workInList.begin();
			iterIn != workInList.end();iterIn++)
		{
			YKWorkInputItem& workIn = *iterIn;			

			if(workIn != NULL && workIn.GetInputItemId() > 0)
			{
				time_t tm1 = workIn->GetTime();
				if(tm1 == 0)
					tm1 = work->GetPlantStartTm();					
				time_t inputTimeSt = GetTimeBoundStTm(tm1);
				YK_ULONG inputPudId = workIn.GetInputItemId();
				//�ͻ�����			
				YKOrderPtr workInputOrderPtr = workIn->GetOrder();
				if ( workInputOrderPtr != NULL )
				{
					YK_ULONG nClientId = workInputOrderPtr->GetClientId();
					g_PSIClientMap[inputPudId][nClientId][inputTimeSt].m_demand += workIn.GetPlantAmount();

					workInputNTimeMap[inputPudId][inputTimeSt].m_demand += workIn.GetPlantAmount();
				}

				//���ݹ��� ��ȡ����
				YKOrderPtr orderPtrByWork = work->GetOrder();
				if ( orderPtrByWork != NULL )
				{
					YKPSI pPsiPtr;
					pPsiPtr->New();
					if ( pPsiPtr != NULL )
					{
						pPsiPtr->SetPSIItem( TOOLS::IToWString(inputPudId,10) );	//��ƷID
						pPsiPtr->SetPSIClient( TOOLS::IToWString(orderPtrByWork->GetClientId(),10) );	//�ͻ�ID
						pPsiPtr->SetPSIDate( TOOLS::IToWString(inputTimeSt,10) );	//����
						objIdForDel.push_back(pPsiPtr->GetId() );
					}
				}			
			}
		}

		//�������
		for(list<YKWorkOutputItem>::iterator iterOut = workOutList.begin();
			iterOut != workOutList.end();iterOut++)
		{
			YKWorkOutputItem& workOut = *iterOut;
			if(workOut != NULL && workOut.GetOutputItemId() > 0)
			{
				YK_ULONG outPudId = workOut.GetOutputItemId();
				time_t tm1 = workOut->GetTime();
				if(tm1 == 0)
					tm1 = work->GetPlantCompleteTm();
				time_t outputTimeSt = GetTimeBoundStTm(tm1);

				//���ݿͻ����з���
				YKOrderPtr workOutPutPtr = workOut->GetOrder();
				if ( workOutPutPtr != NULL )
				{
					YK_ULONG nClientId = workOutPutPtr.GetClientId();
					g_PSIClientMap[outPudId][nClientId][outputTimeSt].m_accommodate += workOut.GetPlantAmount();

					objNTimeMap[outPudId][outputTimeSt].m_accommodate += workOut.GetPlantAmount();
				}

				//���ݹ��� ��ȡ����
				YKOrderPtr outPutOrderPtrByWork = work->GetOrder();
				if ( outPutOrderPtrByWork != NULL )
				{
					YKPSI pPsiPtr;
					pPsiPtr->New();
					if ( pPsiPtr != NULL )
					{
						pPsiPtr->SetPSIItem( TOOLS::IToWString(outPudId,10));		//��ƷID
						pPsiPtr->SetPSIClient( TOOLS::IToWString(outPutOrderPtrByWork->GetClientId(),10) );		//�ͻ�ID
						pPsiPtr->SetPSIDate( TOOLS::IToWString(outputTimeSt,10) );	//����
						objIdForDel.push_back( pPsiPtr->GetId() );
					}
				}		
			}
		}
	}

	//����ǰ��Ĳ��֣�����ƽ��
	for(map<YK_ULONG,map<YK_ULONG,map<time_t,PSIunit>>> ::iterator iterPSI = g_PSIClientMap.begin();
		iterPSI != g_PSIClientMap.end();iterPSI++)
	{
		list<YKPSI> psiObjList;
		YK_ULONG nPudItemId = iterPSI->first;
		YKPSIPtrMap* pPSIPtrMap = g_pBizModel->GetYKPSIPtrMap();
		if ( pPSIPtrMap != NULL )
		{
			for ( pPSIPtrMap->Begin();pPSIPtrMap->NotEnd();pPSIPtrMap->Step() )
			{
				YKPSI tpPSIPtr = pPSIPtrMap->Get();
				if ( tpPSIPtr != NULL )
				{
					YK_WSTRING itemIdstr = tpPSIPtr->GetPSIItem();
					if ( nPudItemId == _wtol( itemIdstr.c_str()) )
					{
						psiObjList.push_back(tpPSIPtr);
					}
				}
			}
		}			

		//����			
		for ( list<YKPSI>::iterator fItor = psiObjList.begin();fItor != psiObjList.end();fItor++ )
		{			
			map<YK_ULONG,map<time_t,PSIunit>>& psiList = iterPSI->second;
			time_t stTm = GetTimeBoundStTm(g_pBizModel->GetShowStartTime());
			time_t endTm = GetTimeBoundStTm(g_pBizModel->GetShowEndTime());
			long timeBoundLength = GetTimeBoundLength();
			time_t tm1 = stTm;

			YKPSI psiObj = *fItor;
			YK_ULONG clientId;	
			if ( psiObj != NULL)
			{
				clientId = _wtol(psiObj->GetPSIClient().c_str());

				psiList[clientId][tm1].m_balance = psiList[clientId][0].m_accommodate + psiList[clientId][tm1].m_accommodate - psiList[clientId][tm1].m_demand;
				time_t tm2 = tm1 + timeBoundLength;

				YKPSI tempPsiPtr;
				tempPsiPtr->New();
				if ( tempPsiPtr != NULL )
				{
					YK_DOUBLE dBalance = psiList[clientId][tm1].m_balance;
					YK_DOUBLE dCreAcount = psiList[clientId][tm1].m_balance-psiList[clientId][0].m_balance;
					YK_DOUBLE dataAcount  = psiObj->GetPSIAcount();
					YK_DOUBLE dataNumber = psiObj->GetPSINumber();
					YK_ULONG itemId = _wtol( psiObj->GetPSIItem().c_str());					
					
					tempPsiPtr->SetPSIItem( psiObj->GetPSIItem() );			//��Ʒ
					tempPsiPtr->SetPSIClient( psiObj->GetPSIClient());		//�ͻ�
					tempPsiPtr->SetPSIDate( TOOLS::IToWString(tm1,10) );		//����	
					//����
					YK_DOUBLE dAcount(0);
					if ( workInputNTimeMap[itemId][tm1].m_demand != 0  )
					{
						workInputNTimeMap[itemId][tm1].m_demand = workInputNTimeMap[itemId][tm1].m_demand-psiList[clientId][tm1].m_balance;
					}else
					{
						dAcount = psiList[clientId][tm1].m_balance;
					}

					tempPsiPtr->SetPSIAcount(  dAcount );		//����
					tempPsiPtr->SetPSINumber(  psiList[clientId][tm1].m_balance-psiList[clientId][0].m_balance );		//����
				}	

				for(;tm2<=endTm;tm2+=timeBoundLength,tm1 += timeBoundLength)
				{			
					psiList[clientId][tm2].m_balance = psiList[clientId][tm2-timeBoundLength].m_balance + psiList[clientId][tm2].m_accommodate - psiList[clientId][tm2].m_demand;

					YKPSI tpPsiPtr;
					tpPsiPtr->New();
					if ( tpPsiPtr != NULL )
					{
						YK_DOUBLE dBalance = psiList[clientId][tm2].m_balance;
						YK_DOUBLE dCreAcount = psiList[clientId][tm2].m_balance-psiList[clientId][tm1].m_balance;
						YK_ULONG itemId = _wtol(psiObj->GetPSIItem().c_str());

						tpPsiPtr->SetPSIItem( psiObj->GetPSIItem() );			//��Ʒ
						tpPsiPtr->SetPSIClient( psiObj->GetPSIClient());		//�ͻ�
						tpPsiPtr->SetPSIDate( TOOLS::IToWString(tm2,10) );		//����						
						//����
						YK_DOUBLE d2Acount(0);
						if ( workInputNTimeMap[itemId][tm2].m_demand != 0  )
						{
							d2Acount =  psiList[clientId][tm2].m_balance - workInputNTimeMap[itemId][tm2].m_demand;
							if ( psiList[clientId][tm2].m_balance < workInputNTimeMap[itemId][tm2].m_demand )
							{
								d2Acount = 0;
								workInputNTimeMap[itemId][tm2].m_demand = workInputNTimeMap[itemId][tm2].m_demand - psiList[clientId][tm2].m_balance;
								psiList[0][tm2].m_balance  = psiList[0][tm2].m_demand-psiList[clientId][tm2].m_balance;		//�ͻ�Ϊ��
								psiList[clientId][tm2].m_balance  = 0;								
							}else
							{
								psiList[clientId][tm2].m_balance  = psiList[clientId][tm2].m_balance -  workInputNTimeMap[itemId][tm2].m_demand;
								workInputNTimeMap[itemId][tm2].m_demand = 0;
								psiList[0][tm2].m_demand = 0;		//�ͻ�Ϊ��
							}						
						}else
						{
							d2Acount = psiList[clientId][tm2].m_accommodate-psiList[clientId][tm2].m_demand+psiList[clientId][tm2-timeBoundLength].m_balance;
						}

						tpPsiPtr->SetPSIAcount( d2Acount );		//����
						tpPsiPtr->SetPSINumber( psiList[clientId][tm2].m_balance-psiList[clientId][tm2-timeBoundLength].m_balance );		//����
					}				
				}
			}
		}	
	}

	//ɾ������������PSI����
	for ( list<YK_ULONG>::iterator fItor = objIdForDel.begin(); fItor != objIdForDel.end(); fItor++ )
	{
		YK_ULONG objId = *fItor;
		YKPSI ptrObj = g_pBizModel->GetYKPSIPtrMap()->Get(objId);
		if ( ptrObj != NULL )
		{
			ptrObj.Delete();
		}
	}

	//��ӡPSI����
	OutPutPSIData();
}

//Add 2012-04-16 �жϹ��������빤����� �й���Ƿ�һ��
YK_BOOL ApsCal::IsRelWorkInputAndOutputBySpecs( YKWorkInputItemPtr& workInputPtr, YKWorkOutputItemPtr& workOutputPtr )
{
	//����ֵ2��ʾ�����ù��ƥ��
	if(g_pBizModel->GetApsParam()->GetApsIsSDataSpecMt() == 2)
		return true;

	YK_BOOL isSpecBool(false),isDataSpecBool(false);
	//��������
	YKWorkPtr inputWork = NULL;
	list<YKSpecPtr> inputWorkSpecList ,inputWorkDataSpecList;
	//�������
	YKWorkPtr outputWork = NULL;
	list<YKSpecPtr> outputWorkSpecList ,outputWorkDataSpecList;

	//�ɹ������� ��ȡ������ �����ֵ���
	if ( workInputPtr.ValidPointer() )		
	{
		inputWork = workInputPtr->GetWork();		
	}
	if ( inputWork.ValidPointer() )
	{
		inputWorkSpecList = inputWork->GetSpecPtrList();
		inputWorkDataSpecList = inputWork->GetSpecDataPtrList();
	}

	//�ɹ������ ��ȡ������ �����ֵ���
	if ( workOutputPtr.ValidPointer()  )
	{
		outputWork = workOutputPtr->GetWork();
	}	
	if ( outputWork.ValidPointer() )
	{
		outputWorkSpecList = outputWork->GetSpecPtrList();
		outputWorkDataSpecList = outputWork->GetSpecDataPtrList();
	}

	//�жϹ�������͹�������Ƿ�ƥ��
	YK_INT nInWSpecSize = inputWorkSpecList.size();
	YK_INT nInWDataSpecSize = inputWorkDataSpecList.size();
	YK_INT nOutWSpecSize = outputWorkSpecList.size();
	YK_INT nOutWDataSpecSize = outputWorkDataSpecList.size();

	if ( nInWSpecSize ==0 && nOutWSpecSize == 0  )
	{
		isSpecBool = true;
	}
	if ( nInWDataSpecSize ==0 && nOutWDataSpecSize == 0 )
	{
		isDataSpecBool = true;
	}
	if ( nInWSpecSize == 0 && nOutWSpecSize >0 )		//��ָ������ʹ��ָ��������Ʒ
	{
		isSpecBool = true;
	}
	if ( nInWDataSpecSize == 0 && nOutWDataSpecSize>0 )
	{
		isDataSpecBool = true;
	}

	//�ȽϹ��
	for ( list<YKSpecPtr>::iterator fItor = inputWorkSpecList.begin(); fItor != inputWorkSpecList.end();fItor++ )
	{
		YKSpecPtr inWorkSpcePtr = *fItor;
		if ( inWorkSpcePtr.ValidPointer() )
		{
			YKSpecItemPtr inSpecItemPtr = inWorkSpcePtr->GetSpecItemPtr();
			if ( inSpecItemPtr.ValidPointer() )
			{
				for ( list<YKSpecPtr>::iterator oFItor = outputWorkSpecList.begin(); oFItor != outputWorkSpecList.end();oFItor++ )
				{
					YKSpecPtr outWorkSpcePtr = *oFItor;
					if ( outWorkSpcePtr.ValidPointer() )
					{
						YKSpecItemPtr outSpecItemPtr = outWorkSpcePtr->GetSpecItemPtr();
						if ( outSpecItemPtr.ValidPointer() )
						{
							if ( inSpecItemPtr->GetType() == outSpecItemPtr->GetType() )		//������ͬ
							{
								if ((inSpecItemPtr->GetCode() == outSpecItemPtr->GetCode()))
								{
									if (( inWorkSpcePtr->GetSpecValue() == outWorkSpcePtr->GetSpecValue() ))
									{
										isSpecBool = true;
									}else
									{
										isSpecBool = false;
									}
									
									break;
								}
							}
						}						
					}
				}
			}			
		}
	}

	//�Ƚ���ֵ���
	for ( list<YKSpecPtr>::iterator fDItor = inputWorkDataSpecList.begin(); fDItor != inputWorkDataSpecList.end();fDItor++ )
	{
		YKSpecPtr inWorkDataSpcePtr = *fDItor;
		if ( inWorkDataSpcePtr.ValidPointer() )
		{
			YKSpecItemPtr inWDSpItemPtr = inWorkDataSpcePtr->GetSpecItemPtr();
			if ( inWDSpItemPtr.ValidPointer() )
			{
				for ( list<YKSpecPtr>::iterator oFDItor = outputWorkDataSpecList.begin(); oFDItor != outputWorkDataSpecList.end();oFDItor++ )
				{
					YKSpecPtr outWorkDataSpcePtr = *oFDItor;
					if ( outWorkDataSpcePtr.ValidPointer() )
					{
						YKSpecItemPtr outWDSpItemPtr = outWorkDataSpcePtr->GetSpecItemPtr();
						if ( outWDSpItemPtr.ValidPointer() )
						{
							if ( inWDSpItemPtr->GetType() == outWDSpItemPtr->GetType() )
							{
								if ( (inWDSpItemPtr->GetCode() == outWDSpItemPtr->GetCode()) )
								{
									if (( inWorkDataSpcePtr->GetSpecValue() == outWorkDataSpcePtr->GetSpecValue()))
									{
										isDataSpecBool = true;
									}else
									{
										isDataSpecBool = false;
									}
									break;
								}
							}
						}						
					}
				}
			}			
		}
	}

	return isSpecBool&&isDataSpecBool;
}
