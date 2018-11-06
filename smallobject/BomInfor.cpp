#include "stdafx.h"
#include "YKScheduler.h"
#include "MainFrm.h"

#include "BOMInfo.h"

#include  "Biz_API.h"
#include "EnumDef.h"
#include "Resource.h"
#include "Tools.h"

#include "CCreatGridMenu.h"
#include "ColorVector.h"
#include "BOMModeDlg.h"
#include "BOMItemEditDlg.h"
#include <afxctl.h >
#include "PopMenuMgr.h"
#include "NewModelDlg.h"
#include "TmEditDlg.h"
#include "NewRuleDlg.h"
#include "RgbDef.h"
#include "FlexUtil.h"

#define  NEW_BOM_IN		1
#define  NEW_BOM_DO		2
#define  NEW_BOM_OUT	3

#define FIRSTROWHEIGHT		400			//��һ���и�
#define ROWHEIGHT			345			//��ʼ��ÿ���и�
#define	INITROWCOUNT		60			//��ʼ��ʱ����������

#ifdef _CLOCK_DEBUG
#include "clock.h"
#define  CLOCK_PASE(tm)  tm.Pause();
#else
#define  CLOCK_PASE(tm)
#endif


#define  WORD_ASTERISK				L"*"
#define  WORD_VERLINE				L"!"
#define  WORD_EQUAL					L"="

IMPLEMENT_DYNCREATE (CBOMInfo, CVsflexgridn1);

//////////////////////////////////////////////////////////////////////////

BEGIN_EVENTSINK_MAP(CBOMInfo, CWnd)
	ON_EVENT(CBOMInfo, 912, 21, CBOMInfo::BeforeEditVsflexgrid, VTS_I4 VTS_I4 VTS_PBOOL)
	ON_EVENT(CBOMInfo, 912, 24, CBOMInfo::AfterEditVsflexgrid, VTS_I4 VTS_I4)
	ON_EVENT(CBOMInfo, 912, DISPID_MOUSEMOVE, CBOMInfo::MouseMoveVsflexgrid, VTS_I2 VTS_I2 VTS_R4 VTS_R4)
	ON_EVENT(CBOMInfo, 912, DISPID_MOUSEUP, CBOMInfo::MouseUpVsflexgrid, VTS_I2 VTS_I2 VTS_R4 VTS_R4)
	ON_EVENT(CBOMInfo, 912, DISPID_DBLCLICK, CBOMInfo::DblClickVsflexgrid, VTS_NONE)
	ON_EVENT(CBOMInfo, 912, 5, CBOMInfo::BeforeMouseDownVsflexgrid, VTS_I2 VTS_I2 VTS_R4 VTS_R4 VTS_PBOOL)
	ON_EVENT(CBOMInfo, 912, 10, CBOMInfo::BeforeScrollVsflexgridu1, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL)
	ON_EVENT(CBOMInfo, 912, 18, CBOMInfo::AfterUserResizeVsflexgridu1, VTS_I4 VTS_I4)
	ON_EVENT(CBOMInfo, 912, 11, CBOMInfo::AfterScrollVsflexgrid, VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CBOMInfo, 912, 35, CBOMInfo::BeforeScrollTipVsflexgrid, VTS_I4)
	ON_EVENT(CBOMInfo, 912, 34, CBOMInfo::CellButtonClickVsflexgridu1, VTS_I4 VTS_I4)
	ON_EVENT(CBOMInfo, 912, DISPID_MOUSEDOWN, CBOMInfo::MouseDownVsflexgrid, VTS_I2 VTS_I2 VTS_R4 VTS_R4)
	ON_EVENT(CBOMInfo, 912, 16, CBOMInfo::AfterMoveColumnVsflexgrid, VTS_I4 VTS_PI4)
	ON_EVENT(CBOMInfo, 912, 23, CBOMInfo::ValidateEditVsflexgrid, VTS_I4 VTS_I4 VTS_PBOOL)
	ON_EVENT(CBOMInfo, 912, 15, CBOMInfo::BeforeMoveColumnVsflexgrid, VTS_I4 VTS_PI4)
END_EVENTSINK_MAP()

CBOMInfo::CBOMInfo(void)
	: m_singleShow(false)
	, m_itemId(0)
	, m_nClientRowCount(0)
{
	InitMemberData();
}

CBOMInfo::~CBOMInfo(void)
{

}

void CBOMInfo::InitMemberData()
{
	m_uTabNameIndex = GetStTable().GetNameIndex();
	m_bErrorFlg= false;
	m_NewRow = 0;
	m_lOldBottomRow = 0;
	m_bClickedFCol = false;
	m_editCode = _T("");
	m_listHideCol.clear();
	m_codeList.clear();
}

void CBOMInfo::InitFlexGrid()
{
	// �л���ǩ�����ó�Ա����
	if (m_uTabNameIndex != GetStTable().GetNameIndex())
		InitMemberData();

	long lRDProperty = get_Redraw();
	put_Redraw(flexRDNone);

	Clear(COleVariant((long)flexClearEverywhere), COleVariant((long)flexClearEverything));

	// ���ñ������������
	put_Cols(m_BOMAxisManager.GetFieldName(1).size() + 1);
	put_Rows(m_BOMAxisManager.GetRowCount() + 2);

	put_AllowSelection(TRUE);
	put_EditText(_T(""));   // ����Ϊ��ʹ�ù����ʧ

	//��ɫ
	put_BackColorBkg(get_BackColor());
	put_ForeColorSel(get_BackColor());

	// �̶���Ԫ�������͡���ɫ������ɫ����ǹ̶�������ɫ
	put_GridLinesFixed(flexGridFlat);
	put_GridColorFixed(COLOR_Bom_FixGid/*RGB(170,177,193)*/);//��ͷ���߿���ɫ
	put_BackColorFixed(GRID_COLOR_TOP1);//��ͷ��ɫ
	put_SheetBorder(COLOR_Bom_Gid_SelBkg/*RGB(0,0,0)*/);

	// ����������������ͣ������
	put_Ellipsis(flexEllipsisEnd);
	put_GridLines(flexGridFlat);
	put_HighLight(flexHighlightWithFocus);
	put_GridColor(COLOR_Bom_Gid/*RGB(170,177,193)*/);	//�����ߵ���ɫ
	put_BackColorBkg(COLOR_Bom_Gid_Bkg/*RGB (255, 255, 255)*/);
	put_RowHeight(0,400);
	put_RowHeight(-1, 380);
	put_FontName(L"����");
	put_FontSize((float)10);
	put_ColAlignment(-1, flexAlignLeftCenter);
	put_FocusRect(flexFocusSolid);//ȥ��ϵͳ�Զ������߿�
	put_BackColor(COLOR_Bom_Gid_Bkg/*RGB (255, 255, 255)*/);

	// ����������
	put_AutoSearch(flexSearchNone);
	put_Editable(flexEDKbdMouse);
	put_AllowUserResizing(flexResizeBoth);
	put_SelectionMode(flexSelectionFree);
	put_ScrollTrack(FALSE);
	put_ScrollTips(TRUE);
	put_ExplorerBar(flexExMove);
	put_BackColorSel(COLOR_Bom_Gid_SelBkg/*RGB(0, 0, 0)*/);
	put_EditSelLength(0);
	put_OwnerDraw(flexODOver);

	// ���õ�Ԫ�ںϵ�
	put_MergeCol(-1, false);
	put_MergeCells(flexMergeRestrictAll);
	put_MergeCompare(flexMCIncludeNulls);
	list<UINT> comLines = m_BOMAxisManager.GetComLines();
	for (list<UINT>::iterator iter = comLines.begin(); iter != comLines.end(); iter++)
	{
		put_MergeCol(*iter, true); // columns can be merged
	}

	SetColHide();

	// ���ù̶���
	put_FrozenCols(GetStTable().fixCol);

	UpdateClientRowCount();
	put_Redraw(lRDProperty);
}

BOOL CBOMInfo::UpdateClientRowCount()
{
	BOOL bOK = FALSE;

	const int nRowCount = get_BottomRow() - get_TopRow() + 1;
	if (nRowCount != GetClientRowCount())
	{
		SetClientRowCount(nRowCount);
		bOK = TRUE;
	}

	return bOK;
}

void CBOMInfo::FixCol(long col)
{
	long lastFCol = get_FrozenCols()+1;
	if (col < lastFCol)
		return;

	// �ƶ��̶���
	put_ColPosition(col, lastFCol);
	stTable& curTbl = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());
	DATATABLAPI::GoTostTblPos(curTbl, col - 1, lastFCol-1);
	SetStTable(curTbl);

	// �̶��̶���
	put_FrozenCols(lastFCol);
	curTbl.fixCol = lastFCol;
	GetStTable().fixCol = lastFCol;


	ReSetDataSource();
	RedrawWindow();

}

void CBOMInfo::UnAllFixCol(long col)
{
	const int nFrozenCols = 1;

	put_FrozenCols(nFrozenCols);
	GetStTable().fixCol = nFrozenCols;
	stTable &tbl = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());
	tbl.fixCol = nFrozenCols;

	ReSetDataSource();
	RedrawWindow();
}
BOOL CBOMInfo::OnCreate( CWnd* pParentWnd )
{
	BOOL rt =  Create (L"BOMInfo",WS_CHILD | WS_VISIBLE, 
		CRect (0, 0, 200, 300), pParentWnd, 912);
	if(!rt)
		return rt;
	
	if (m_singleShow)
		BIZAPI::SetTableFlashFlg(TRUE, ID_BOM_VIEW_DLG);
	else 
		BIZAPI::SetTableFlashFlg(TRUE, TblBOM);

	m_toolTip.Create((HWND)GetSafeHwnd());
	return rt;
}

void CBOMInfo::ReSetGridHead()
{
	vector<wstring> headcodeList = m_BOMAxisManager.GetFieldName(1);
	const int listLen = headcodeList.size();
	/*put_Cols(listLen + 1);
	put_Rows(1);*/
	for (int i = 0; i < listLen; i++)
	{
		put_TextMatrix(0, i+1, headcodeList[i].c_str());
	}
}

void CBOMInfo::ReSetDataSource()
{
	put_EditText(_T(""));   // ����Ϊ��ʹ�ù����ʧ
	// ������һ��Ϊ�½���
	put_Rows(m_BOMAxisManager.GetRowCount() + 2);
}

// ˢ����ͼ  bLoadData=TRUE������ҳǩ���ˢ��(��Ҫ����ȡ����) 
// ���򣺲����¶�ȡ����  �������ݵ�stble�ͺ���  add 2010-5-17 by ll
// bLoadData:�Ƿ��ȡ����
// Ĭ�� tblFlash=TRUE
void CBOMInfo::Flash(BOOL bLoadData)
{
	m_BOMAxisManager.SetTblType(TblBOM);
	m_BOMAxisManager.SetSingleItemId(m_itemId);

	if (BIZAPI::EnterDataBaseCriSel())
	{
		try
		{
			if(bLoadData)
			{
				bool bInitData = false;
				// ����Ӧˢ�±�־Ϊ ˢ��
				if (m_singleShow)	// BOM��ͼ
				{
					if (BIZAPI::GetTableFlashFlg(ID_BOM_VIEW_DLG))		// BOM���
					{
						bInitData = true;
						BIZAPI::SetTableFlashFlg(FALSE, ID_BOM_VIEW_DLG);
					}
				}
				else if (BIZAPI::GetTableFlashFlg(GetTblType()))		// BOM���
				{
					bInitData = true;
					BIZAPI::SetTableFlashFlg(FALSE, GetTblType());
				}
				if (bInitData)
				{
					put_Rows(1);
					m_BOMAxisManager.Init(m_singleShow);
				}
			}



			// �Ա���ʼ��ǰ�����ʼ������Դ
			InitFlexGrid();
			ReSetGridHead();
			//ReSetDataSource();

			UpdateScreenData(get_TopRow(), get_BottomRow());
			SetColWidth();
		}
		catch (...)
		{
		}

		BIZAPI::LeaveDataBaseCriSel();
	}
}

////������Դ
//void CBOMInfo::ResetSource()
//{
//	::SendMessage(theApp.m_pMainWnd->GetSafeHwnd(),_ACTIVITE_VIWE_,123,0);
//}

void CBOMInfo::BeforeEditVsflexgrid( long Row, long Col, BOOL* Cancel )
{
	m_editCode = get_TextMatrix(Row,Col);
#ifdef LIMITFUNCTION
	if (BIZAPI::GetIsLimited())
	{
		*Cancel = TRUE;
		return;
	}
#endif
	if( !BIZAPI::CheckLimit( TblBOM,0/*LimitOfAuthor_Bom*/ ) )  //Ȩ������
	{
		*Cancel = TRUE;
		return;
	}
	wstring comboText;

	bool b=false;
	UINT editType = GetCombCode(  Row,  Col,  comboText ,b);
	if(!b /*&& editType != FILEDPARA_EDITTYPE_POPUP*/)
	{
		*Cancel = TRUE;
		return;
	}
	else if(editType == FILEDPARA_EDITTYPE_EDIT)
	{
		put_ComboList(L"");
	}
	else if (editType == FILEDPARA_EDITTYPE_POPUP)
	{
		put_ComboList(L"...");//����Ϊ��...����ť
	}
	else if (editType == FILEDPARA_EDITTYPE_POPUP_EDIT)
	{
		put_ComboList(L"|...");//�ɱ�д������
	}
	else
	{
		put_ComboList(comboText.c_str());
		if(comboText.empty())		// û��ѡ��ܱ༭
			*Cancel = TRUE;
	}
}
UINT CBOMInfo::GetCombCode( int nRow, int nCol, wstring& comboText ,bool& b)
{
	//��һ��Ϊ��ţ�ȥ��
	int nColumn = nCol - 1;
	b = false;
	m_codeList.clear();

	if (nColumn < GetStTable().mode.size() && nColumn >=0)
	{
		UINT enumCol = GetStTable().mode[nColumn].mode;

		YKFieldPtr temppara = BIZAPI::GetBizMap()->GetFieldPara(GetTblType(), enumCol);
		if(NULL == temppara) return FILEDPARA_EDITTYPE_EDIT;

		b = (temppara->m_modifyFly == FILEDPARA_ISMODIFY_TRUE) && !temppara->HaveVirRule();
		if(nRow > m_BOMAxisManager.GetRowCount())		// ���һ��
		{
			b = true;
			return FILEDPARA_EDITTYPE_EDIT;
		}

		short bomType = m_BOMAxisManager.GetRowInfo_type(nRow-1);
		UINT havePud = 1;
		if (bomType == 0 && enumCol == YKBOM::BOM_cmdType)
		{
			YK_ULONG bomId = 0;
			m_BOMAxisManager.GetRowInfo(nRow-1,bomId);
		}
		if(b)
		{
			b = (temppara->GetModifyFly() == FILEDPARA_ISMODIFY_TRUE
				&&!temppara->HaveVirRule()
				&&YKBOM::CanbeEdit(bomType,enumCol,(havePud == 1)));
		}
		if(!b) 	return temppara->m_editType;

		int tempEdit = -1;
		UINT refTabType = temppara->m_relTbl;
		if (temppara->m_editType == FILEDPARA_EDITTYPE_COMBO_STORE)
		{
			YK_ULONG key=0;
			m_BOMAxisManager.GetRowInfo(nRow-1,key);
			if (refTabType == 8644)		// ����
			{
				refTabType += m_BOMAxisManager.GetRowInfo_type(nRow-1);
			}
			if(refTabType == 8645||refTabType == 8647)
				tempEdit = 2;
			else if (refTabType == 8646)
				tempEdit = 1;
		}

		if (temppara->m_editType == FILEDPARA_EDITTYPE_COMBO||tempEdit == 1)
		{
			UINT enumStart=temppara->m_enumSt;
			UINT enumEnd=temppara->m_enumEd;
			// �������ʹ������
			if (enumCol==YKBOM::BOM_cmdCode)
			{
				switch(bomType)
				{
				case 1:		// ����
					{
						enumStart=BOM_CODETYPE_IN_1;
						enumEnd=BOM_CODETYPE_IN_1;
						break;
					}
				case 2:		// ����
					{
						YKFieldPtr temppara2 = BIZAPI::GetBizMap()->GetFieldPara(TblOperAvbRes,YKOperAvbRes::OperAvbRes_UseType);
						enumStart=temppara2->m_enumSt;
						enumEnd=temppara2->m_enumEd;
						break;
					}
				case 3:		// ���
					{
						enumStart=BOM_CODETYPE_OUT_1;
						enumEnd=BOM_CODETYPE_OUT_2;
						break;
					}
				}
			}
			comboText = temppara->GetCombCode(m_codeList);
			//����Code + �ַ���
			//DoCodeAndWord(comboText,nColumn);
		}
		else if (temppara->m_editType == FILEDPARA_EDITTYPE_COMBO_CODE
			||tempEdit == 2)
		{
			if (refTabType > 0)
			{
				BIZAPI::GetCodeList(refTabType,m_codeList);
				for (map<wstring,long>::iterator i_code = m_codeList.begin();
					i_code != m_codeList.end();i_code++)
				{
					comboText.push_back(_T('|'));
					comboText += i_code->first;
				}
			}
		}
		return temppara->m_editType;
	}
	return 0;
}

void CBOMInfo::ValidateEditVsflexgrid( long Row, long Col, BOOL* Cancel )
{
	// �л��˱�ǩ��ȡ���༭ģʽ
	if (m_uTabNameIndex != GetStTable().GetNameIndex())
		return;

	if (Row < 1) 
		return;

	CString cstr = get_EditText();
	wstring wstr = cstr.GetString();
	TOOLS::RemoveBlank(wstr);
	cstr = wstr.c_str();
	
	//Add 2011-10-20 ϵͳд�����
	unsigned long  tableName = m_BOMAxisManager.m_table.tblType;	//��
	YKFieldPtr temppara = BIZAPI::GetBizMap()->GetFieldPara(tableName, Col-1);

	if (Row > m_BOMAxisManager.GetRowCount()) return;
	int tablCol = m_BOMAxisManager.m_table.mode[Col-1].mode;

	if (tablCol > 0)
	{
		//Add 2011-10-20 -zk
		YK_ULONG key=0;
		m_BOMAxisManager.GetRowInfo(Row-1,key);			//��ȡ����ID
		bool judageValue = BIZAPI::TableColsValueJudage(m_BOMAxisManager.m_table.tblType,key,tablCol,wstr);

		//ִ��д�����
		if (judageValue)
		{
			BomModify(m_BOMAxisManager.m_table.tblType,key,tablCol,wstr);
			m_bErrorFlg=FALSE;

		}
		else
		{
			m_bErrorFlg = TRUE;
		}
	}

	FlashBomViewByRow(Row);

	if (m_bErrorFlg)  //�༭����
	{
		vector<long> sBomId =  GetSameField( Row,  Col);
		if ( sBomId.empty() )
		{
			FillErrorColor( Row,  Col);
			put_EditText(cstr);
			put_TextMatrix(Row,Col,cstr);
		}
		else
		{
			vector<long>::iterator iter = sBomId.begin() ;
			for ( ;iter != sBomId.end() ; iter++ )
			{
				FillErrorColor( *iter,  Col);
				put_EditText(cstr);
				put_TextMatrix(*iter,Col,cstr);
			}
		}
		*Cancel = TRUE;		// ����ΪTRUE �ù�걣�����û�¼���������λ��
	}
	else
	{
		if (tablCol >=0)
		{
			if (tablCol == YKBOM::BOM_processId||	// ���ձ��
				tablCol == YKBOM::BOM_processCode||	//���մ���
				tablCol == YKBOM::BOM_useItemId||	//  ��Ʒ/��Դ)
				tablCol == YKBOM::BOM_preProcessId||	// ���й��ձ��
				tablCol == YKBOM::BOM_cmdType)		
			{
				// bom����������Ҫˢ��
				if (m_singleShow)
				{
					BIZAPI::SetTableFlashFlg(TRUE,BOMViewFlash_Only);
					if(tablCol == YKBOM::BOM_useItemId)
					{
						UINT type=m_BOMAxisManager.GetRowInfo_type(Row-1);
						if (type == 1||type == 3)		// ��Ʒ
						{
							// ��Ʒ���Ŀ�����Ҫ������Ʒ��
							BIZAPI::SetTableFlashFlg(TRUE,BOM_VIEW_ITEMTREE);
						}
					}
					FlashBomViewByRow(Row);
					return;
				}
			}
		}
	}


}
void CBOMInfo::AfterEditVsflexgrid( long Row, long Col )
{
	long r1,r2,c1,c2;
	GetSelection(&r1,&c1,&r2,&c2);
	//get_Cell();
	CString cstr=get_TextMatrix(Row,Col);

	//Add 2012-03-23 
	//if(cstr == m_editCode)		// ����û�иı�
	//	return;
	wstring wcstr = cstr.GetString();
	TOOLS::RemoveBlank(wcstr);
	cstr = wcstr.c_str();

	if(Row == (get_Rows()-1))
	{
		if (m_BOMAxisManager.IsKeyCol(Col-1))
		{
			if(cstr.IsEmpty())
			{
				put_TextMatrix(Row,Col,L"");
				Select(r1,c1,COleVariant((long)r2),COleVariant((long)c2));
				return;
			}
			if (m_singleShow)
			{
				YKItemPtrMap* itemMap = BIZAPI::GetBizMap()->GetYKItemPtrMap();
				YKItemPtr& itemPtr = itemMap->Get(m_itemId);
				if (NULL != itemPtr)
				{
					// ��Ʒ�������һ��
					if(itemPtr->GetCode()!=cstr.GetString())
					{
						put_TextMatrix(Row,Col,L"");
						Select(r1,c1,COleVariant((long)r2),COleVariant((long)c2));
						return;
					}
				}
				else
				{
					put_TextMatrix(Row,Col,L"");
					Select(r1,c1,COleVariant((long)r2),COleVariant((long)c2));
					return;
				}
			}

			//BIZAPI::BeginRebackOper(OPERTYPENEW);
			unsigned long newId = BIZAPI::NewItemClassByTbl(m_BOMAxisManager.m_table.tblType,cstr.GetString());
			if (newId > 0)
			{
				// ����reback ��Ϣ		only bom info  
// 				RecordInfo recodeinfo;
// 				recodeinfo.id = newId;
// 				recodeinfo.operType = OPERTYPENEW;
// 				BIZAPI::InsertRelDataStruct(m_BOMAxisManager.m_table.tblType,recodeinfo);
				vector<YK_ULONG> listIds;
				listIds.push_back(newId);
				BIZAPI::BeginUndoRedo();
				BIZAPI::InsertOpertion(2, m_BOMAxisManager.m_table.tblType, listIds);
				BIZAPI::EndUndoRedo();
				sBOMInfo bominfo;
				bominfo.bomId = newId;
				bominfo.color = 0;
				m_BOMAxisManager.InsertBomInfo(bominfo);
				if (m_singleShow)		// 
				{
					BIZAPI::SetTableFlashFlg(TRUE,TblBOM);
				}
				else BIZAPI::SetTableFlashFlg(TRUE,ID_BOM_VIEW_DLG);

				Flash(FALSE);
			}
			//BIZAPI::EndRebackOper();

			Select(r1,c1,COleVariant((long)r2),COleVariant((long)c2));
			return;
		}
		put_TextMatrix(Row,Col,L"");
		return;
	}
	else
	{
		int tablCol = m_BOMAxisManager.m_table.mode[Col-1].mode;
		if (tablCol >=0)
		{
			Flash(FALSE);
		}
	}
}

void CBOMInfo::OnItemChanged(long row, long col, BSTR newVal )
{

	if (row > m_BOMAxisManager.GetRowCount()) 
		return;

	const int tablCol = GetStTable().mode[col-1].mode;
	YKFieldPtr temppara = BIZAPI::GetBizMap()->GetFieldPara(GetTblType(), tablCol);	
	
	if (tablCol >= 0)
	{
		YK_ULONG key = 0;
		m_BOMAxisManager.GetRowInfo(row-1, key);
		
		YKBOMPtr& ptr = BIZAPI::GetBizMap()->GetBOMPtrMap()->Get(key);
		if (ptr == NULL)
			return;

		
		//BIZAPI::BeginRebackOper(OPERTYPEMODIFY);			// ����һ��������Ϣ��ջ

		set<unsigned long> idList;
		idList.insert(key);
		BIZAPI::InsertBOMReBackInfo(idList, OPERTYPEMODIFY, TblBOM, false);

		int cmdType = -1;
		if (tablCol == YKBOM::BOM_cmdType)
		{
			cmdType = ptr->GetCmdType();
		}
		//CDataSave ds;
		if (temppara->m_editType == FILEDPARA_EDITTYPE_COMBO_CODE)
		{
			_variant_t var;
			wstring temp(newVal);
			if (temp == WORD_ASTERISK)
			{
				var.vt = VT_BSTR;
				CString strWord(temp.c_str());
				var.bstrVal = strWord.AllocSysString();
			}
			else
			{
				var.vt = VT_UINT;
				map<wstring, long>::iterator i_find = m_codeList.find(temp);
				if (i_find != m_codeList.end())
				{
					var.uintVal = i_find->second;
				}
				else
				{
					//BIZAPI::DelRebackOper();
					return;
				}
			}

// 			YK_SHORT errorType = ds.JudgeTableName(GetTblType(), key, tablCol, var);
// 
// 			if(errorType != EOR_TYPE_NONE && errorType != EOR_TYPE_NONE_REFLSH)
// 			{
// 				m_bErrorFlg=TRUE;
// 				theApp.ShowDataErrorView(GetTblType(), errorType, row, tablCol, GetStTable().GetName(), key);
// 				BIZAPI::DelRebackOper();
// 				//var.Clear();
// 				return;
// 			}
			m_bErrorFlg=FALSE;
			//var.Clear();			
		}
		else 
		{
// 			if(temppara->m_editType ==FILEDPARA_EDITTYPE_COMBO)
// 				ds.SetFiledType(1);
// 			if(tablCol == YKBOM::BOM_itemId)		// ǿ��
// 				ds.SetFiledType(1);
// 			YK_SHORT errorType=ds.JudgeTableName(GetTblType(), key, tablCol, newVal);
// 
// 			if(errorType != EOR_TYPE_NONE && errorType != EOR_TYPE_NONE_REFLSH)
// 			{
// 				theApp.ShowDataErrorView(GetTblType(), errorType, row, tablCol, GetStTable().GetName(), key);
// 				m_bErrorFlg = TRUE;
// 				BIZAPI::DelRebackOper();
// 				return;
// 			}
			m_bErrorFlg = FALSE;
			if(tablCol == YKBOM::BOM_cmdType)
			{
				if(ptr->CanChangeCmdType())
				{
					m_BOMAxisManager.ResetBomId(row-1, key, ptr->GetCmdType());

					BIZAPI::SetTableFlashFlg(TRUE,BOMViewFlash_Only);
					//BIZAPI::SetTableFlashFlg(TRUE,ID_BOM_VIEW_DLG);
				}
			}
// 			if (errorType == EOR_TYPE_NONE_REFLSH)		// ˢ��
// 			{
// 				UINT havePud = 1;
// 				wstring wsCode = m_BOMAxisManager.GetCode(row-1, col-1, havePud);
// 				put_TextMatrix(row, col, wsCode.c_str());
// 			}
		}
	
		//BIZAPI::EndRebackOper();
	}
}


void CBOMInfo::MouseDownVsflexgrid( short Button, short Shift, float X, float Y )
{
	if (1 != Button && 2 != Button)	// ֻ�������������Ҽ��¼�
		return;

	const long row = get_MouseRow();
	const long col = get_MouseCol();

	YKGridFindAndReplaceMgr::Get()->ClearFindSel();

	if (row >= 0 && col >= 0)	// �������
	{
		if (2 == Button)	// �һ�Ҳ��ѡ��
		{
			if (col == 0)	// ��һ��ȫѡ��
			{
				long tRow(0), lCol(0), bRow(0), rCol(0);
				GetSelection(&tRow, &lCol, &bRow, &rCol);
				if (!(m_bClickedFCol && (row >= tRow && row <= bRow)))
				{
					Select(row, 1, COleVariant(row), COleVariant(get_Cols()-1));
				}
			}
			else
			{
				Select(row, col, COleVariant(row), COleVariant(col));
			}
		}

		if (0 == col)	// ��һ��
			m_bClickedFCol = true;
		else
			m_bClickedFCol = false;

		if (row > 0 && row <= Row_DataToGrid(m_BOMAxisManager.GetRowCount()-1))
		{
			// ������  �������Ա���ʾ
			YK_ULONG lParam = 0;
			m_BOMAxisManager.GetRowInfo(row -1, lParam);
			::PostMessage(theApp.g_hMainFrm, IDU_UPDATE_MODIFY_VIEW, GetTblType(), (UINT)lParam);
		}
	}

}

void CBOMInfo::MouseUpVsflexgrid( short Button, short Shift, float X, float Y )
{
	if (1 != Button && 2 != Button)	// ֻ�������������Ҽ��¼�
		return;

	if (m_singleShow)		// BOM��ͼ���ܻ�ı���ͼ����
	{
		CPoint pointw;
		GetCursorPos(&pointw);
		MSG msg;
		msg.pt = pointw;
		msg.lParam = Shift;
		::SendMessage(GetParent()->GetSafeHwnd(), 77778, (int)&msg, 1);
	}
	
	const long row = get_MouseRow();
	const long col = get_MouseCol();

	m_NewRow = row;

	if (2 == Button)	// ����һ�
	{
		const long lastDataRow = Row_DataToGrid(m_BOMAxisManager.GetRowCount()-1);
		if (0 == row && 0 == col)	// ���Ͻ�
		{
			TrackRLeftUpMenu(X, Y);
		}
		else if (row > 0 && 0 == col) // ��������һ��
		{
			TrackRFirstColMenu(X, Y);
		}
		else if (0 == row && col > 0) // ��ͷ
		{
			TrackRHeadMenu(X, Y);
		}
		else if ((row > 0 && row <= lastDataRow) && col > 0)	// ������
		{
			TrackRDataMenu(X, Y);
		}
		else
		{
			TrackROutGridMenu(X, Y);
		}
	}


}

void CBOMInfo::TrackRLeftUpMenu(float X, float Y)
{
	const POINT ptScreen = MouseToScreen(GetSafeHwnd(), X, Y);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_GRIDMENU_ALLDISPLAY, /*L"��ʾȫ����"*/GetRemarkFiled(ID_UI_MENU_ALLDISPLAY).c_str());
#ifndef PMCMODE
	menu.AppendMenu(MF_STRING, ID_GRIDMENU_DEFAULTDISPLAY,GetRemarkFiled(ID_UI_MENU_DEFAULTDISPLAY).c_str() );
#endif
	menu.AppendMenu(MF_STRING, ID_GRIDMENU_RESIZE, /*L"�Ż��п�"*/GetRemarkFiled(ID_UI_MENU_OPTIMIZECOLWIDTH).c_str());
	menu.AppendMenu(MF_STRING, ID_GRIDMENU_OUTPUTHTML,GetRemarkFiled(ID_GRIDMENU_OUTPUTHTML).c_str());
	//�����´˱��Ƿ�������ص���
	bool bHideCol = false;
	for (vector<stVector>::iterator itr = m_BOMAxisManager.m_table.mode.begin(); itr != m_BOMAxisManager.m_table.mode.end(); itr++)
	{
		if ((*itr).GetIsHide())
		{	//�������ص���
			bHideCol = true;
		}
	}
	menu.EnableMenuItem(ID_GRIDMENU_ALLDISPLAY, bHideCol ? MF_ENABLED : MF_GRAYED | MF_DISABLED);	
	const int nOption = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD 
		, ptScreen.x, ptScreen.y, this);

	if (nOption == ID_GRIDMENU_ALLDISPLAY)//��ʾȫ����
	{
		put_ColHidden(-1, false);
		/*for (int i = 0; i < get_Cols(); i++)
		{
			put_ColHidden(i, false);
		}*/
		YKFieldPtrMap* pFieldMap =  BIZAPI::GetBizMap()->GetYKFieldPtrMap();
		//��ֵ���˱��ж�Ӧ��isHideȫ����ֵΪ1����Ϊ��ȫ����ʾ��
		stTable &tbl = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());
		for (size_t j = 0; j < GetStTable().mode.size(); j++)
		{
			YKFieldPtr ptr = pFieldMap->Get(GetTblType(), GetStTable().mode[j].mode);
			if (ptr != NULL && !ptr->CanUse())
			{
				put_ColHidden(j,TRUE);
				GetStTable().mode[j].SetIsHide(TRUE);// = 1;
				tbl.mode[j].SetIsHide(TRUE);// = 1;//��Ӧ��isHideΪ1
			}
			else
			{
				GetStTable().mode[j].SetIsHide(FALSE);// = 1;
				tbl.mode[j].SetIsHide(FALSE);// = 1;//��Ӧ��isHideΪ1
			}

		}
	}
	else if (nOption == ID_GRIDMENU_RESIZE)//�Ż��п�
	{
		OptimizeTable();//�Ż��п�
	}
	else if ( ID_GRIDMENU_DEFAULTDISPLAY == nOption)
	{
		UINT uTblType = GetTblType();
		UINT nameIndex  = GetStTable().GetNameIndex();
		stTable aTable = DATATABLAPI::GetResumeTbl(uTblType, nameIndex);
		DATATABLAPI::SetstTble(uTblType, nameIndex, aTable);
		SetStTable(aTable);

		Flash(FALSE);
	}
	else if (ID_GRIDMENU_OUTPUTHTML == nOption)
	{
		CString tableName(GetRemarkFiled(GetTblType()).c_str());
		CString strPath = theApp.m_curPath;
		strPath += L"\\";
		strPath += tableName;
		strPath += L".html";
		CFlexUtil flex;
		if (flex.SaveGridToHTML(this,strPath,tableName))
		{
			ShellExecute(this->GetSafeHwnd(), NULL, strPath, NULL, NULL, SW_SHOWNORMAL);
		}
	}
}

void CBOMInfo::TrackRHeadMenu(float X, float Y)
{
	const long col = get_MouseCol();
	const POINT ptScreen = MouseToScreen(GetSafeHwnd(), X, Y);

	CGridMenu * pGridMenu = CGridMenu::GetCGridMenu();
	long tRow(0), lCol(0), bRow(0), rCol(0);
	GetSelection(&tRow, &lCol, &bRow, &rCol);
	//m_listHideCol��HideCol�еõ�����������ص��к�
	if (!m_listHideCol.empty())
	{	//������ѡ��������Ƿ�������ص��кţ��еģ������á�������ء�Ϊ����	
		for (list<long>::iterator itr = m_listHideCol.begin(); itr != m_listHideCol.end(); itr++)
		{
			pGridMenu->m_bIsHideCol = false;//��������ء�������
			if ((*itr) > lCol && (*itr) < rCol)
			{
				pGridMenu->m_bIsHideCol = true;//��������ء�����
				break;
			}
		}
	}
	else
	{
		pGridMenu->m_bIsHideCol = false;
	}

	const int nOption = pGridMenu->CreatMenu(GetStTable(), TOPROW, ptScreen, this);
	
	switch (nOption)
	{
	case ID_GRIDMENU_HIDECOL:	// ������
		HideCol(col);
		break;
	case ID_GRIDMENU_DISPHIDECOL:	// ���������
		break;
	case ID_GRIDMENU_FIXCOL:	// �̶���
		FixCol(col);
		break;
	case ID_GRIDMENU_DISPFIXCOL:	// ������й̶���
		UnAllFixCol(col);
		break;
	case ID_GRIDMENU_SETCOLS: // ���������
		SetCols();
		break;
	case ID_GRIDMENU_FIELD_SET:
		{
			// ���Զ���
			UINT hu = YKField::FuseTbl_field(GetTblType(),m_BOMAxisManager.GetEnumIdByIndex(col-1));
			MSG ms;
			ms.message = TblFieldItem;
			ms.wParam = hu;
			ms.lParam = -1;
			::SendMessage(theApp.g_hMainFrm,IDU_UPDATE_MODIFY_DLG,(int)&ms,0);
		}
		break;
	}

}
void CBOMInfo::TrackRFirstColMenu(float X, float Y)
{
	const long row = get_MouseRow();
	// �½��в�������Щ����
	if(row == (get_Rows()-1))
		return;
	if( !BIZAPI::CheckLimit( TblBOM,0/*LimitOfAuthor_Bom*/ ) )  //Ȩ������
		return;
	
	CPopMenuMgr ab;
	ab.SetMemId(IDR_MENU_BOM);
	ab.GetMenu();

	CMenu* pSubMenu = ab.GetMenu().GetSubMenu(0);
	const POINT pt = MouseToScreen(GetSafeHwnd(),X, Y);

	// BOM VIEW �����½�
	if(m_singleShow)
	{
		pSubMenu->EnableMenuItem(ID_BOM_NEW,MF_DISABLED|MF_GRAYED);
	}
	
	// check ���е������Ƿ��Ѿ���ȷ�½�  û�в�������Щ����
	YK_ULONG key = 0;
	m_BOMAxisManager.GetRowInfo(row - 1, key);
	if (key > 0)
	{
		YKBOMMap* bomMap=BIZAPI::GetBizMap()->GetBOMPtrMap();
		YKBOMPtr& bomptr = bomMap->Get(key);
		if(!bomptr->IsIntegrity())
		{
			pSubMenu->EnableMenuItem(ID_GRIDMENU_ADDBOM_IN,MF_DISABLED|MF_GRAYED);
			pSubMenu->EnableMenuItem(ID_GRIDMENU_ADDBOM_DO,MF_DISABLED|MF_GRAYED);
			pSubMenu->EnableMenuItem(ID_GRIDMENU_ADDBOM_OUT,MF_DISABLED|MF_GRAYED);
			pSubMenu->EnableMenuItem(ID_BOM_NEW_PUDLINE,MF_DISABLED|MF_GRAYED);
		}
	}

	const UINT flags = TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD ;
	const int nOption = (int)pSubMenu->TrackPopupMenu(flags, pt.x, pt.y, this);

	if (nOption == ID_GRIDMENU_ADDBOM_IN)
	{
		CreatBOM(BOM_Input, row);
	}
	else if (nOption == ID_GRIDMENU_ADDBOM_DO)
	{
		CreatBOM(BOM_AvbRes, row);
	}
	else if (nOption == ID_GRIDMENU_ADDBOM_OUT)
	{
		CreatBOM(BOM_Output, row);
	}
	else if (nOption == ID_GRIDMENU_ADDBOM_DEL)
	{
		//////////////////////////////////////////////////////////////////////////
		//ѡ�����б�
		long tRow(0), lCol(0), bRow(0), rCol(0);
		GetSelection(&tRow, &lCol, &bRow, &rCol);
		set<unsigned long> bomIdlist;
		for(long row = tRow; row <= bRow; row++)
		{
			YK_ULONG bomId;
			m_BOMAxisManager.GetRowInfo(Row_GridToData(row), bomId);
			bomIdlist.insert(bomId);
		}
		UnDoNew(bomIdlist);

		// ������ԭɾ�����С�����һ�е�ѡ��
		Select(tRow, 0, COleVariant(tRow), COleVariant((long)0));
	}
	else if (nOption == ID_BOM_NEW)
	{
		CBOMModeDlg bomModeDlg;
		bomModeDlg.DoModal();
	}
	else if (nOption == ID_BOM_NEW_PUDLINE)
	{
		CreatBOM(BOM_Output, row, true);
	}
}

void CBOMInfo::TrackRDataMenu(float X, float Y)
{
	// BOM VIEW �����½�
	if(m_singleShow)
		return;

	const long row = get_MouseRow();
	const POINT pt = MouseToScreen(GetSafeHwnd(), X, Y);
	// ShowNewMenu(pt, lMouseRow);
	CPopMenuMgr popMgr;
	popMgr.SetMemId(IDR_MENU_BOMGRID_TO_VIEW);
	CMenu* pSubMenu = popMgr.GetMenu().GetSubMenu(0);

	const UINT flags = TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD ;
	const int nOption = (int)pSubMenu->TrackPopupMenu(flags, pt.x, pt.y, this);
	if (nOption == ID_BOM_BOMVIEW_EDIT)
	{
		YK_ULONG bomId;
		m_BOMAxisManager.GetRowInfo(row-1,bomId);
		if(bomId<=0) return;

		YKBOMMap* bomMap=BIZAPI::GetBizMap();
		if(NULL == bomMap) return;

		YKBOMPtr& bomPtr=bomMap->Get(bomId);
		if(NULL == bomPtr) return;

		YKItemPtr itemptr=bomPtr->GetItemPtr();
		if (NULL != itemptr)
		{
			theApp.m_BOMViewItemId=itemptr->GetId();
			theApp.OpenTblPage(ID_BOM_VIEW_DLG);
		}
	}
}
void CBOMInfo::TrackROutGridMenu(float X, float Y)
{
	// BOM VIEW �����½�
	if(m_singleShow || (!BIZAPI::CheckLimit(TblBOM, 0)))
		return;
	
	const POINT ptScreen = MouseToScreen(GetSafeHwnd(), X, Y);
	CGridMenu * pGridMenu = CGridMenu::GetCGridMenu();
	const int nOption = pGridMenu->CreatMenu(GetStTable(), GRIDNEW, ptScreen, this);
	if(nOption == ID_GRIDMENU_NEW)
	{
		CBOMModeDlg bomModeDlg;
		bomModeDlg.DoModal();
	}
}
// ����type ����BOM
void CBOMInfo::CreatBOM(short type, long row, bool newpud /* = false */)
{
	YK_ULONG bomId;
	m_BOMAxisManager.GetRowInfo(row - 1, bomId);
	if(bomId <= 0) return;

	YKBOMMap* bomMap = BIZAPI::GetBizMap();
	if(NULL == bomMap) return;

	YKBOMPtr& bomPtr = bomMap->Get(bomId);
	if(NULL == bomPtr) return;

	YKItemPtr itemptr = bomPtr->GetItemPtr();
	YKOperationPtr operptr;
	YKProductRoutingPtr proroutingptr;
	YK_ULONG newId=0;
	if (newpud)
	{
		// new oper
		YKBOM pBom; pBom.NewTemp();
		pBom->SetCmdType(BOM_Output);
		pBom->SetItemPtr(itemptr);
		if(NULL != itemptr)
			pBom->SetItemId(itemptr->GetId());
		newId = pBom.Insert();//BIZAPI::GetBizMap()->InsertBiz(pBom);
	}
	else
	{
		operptr = bomPtr->GetOperPtr();
		proroutingptr = bomPtr->GetProductRoutingPtr();
		newId = bomMap->NewBom(itemptr, operptr, proroutingptr, type);
	}

	if (newId > 0)
	{
		MSG ms;
		ms.message = TblBOM;
		ms.wParam = newId;
		ms.lParam = -1;
		::SendMessage(theApp.g_hMainFrm, IDU_UPDATE_MODIFY_DLG, (int)&ms, 1);
	}
}

//
void CBOMInfo::DblClickVsflexgrid()
{
	const long lMouseRow = get_MouseRow();
	const long lMouseCol = get_MouseCol();

	if (lMouseRow > 0 && lMouseCol == 0)
	{
		Edit_BOM(lMouseRow);
	}
	/*long firstRow = get_RowPos(0) + get_RowHeight(0);
	long firstCol = get_ColPos(0) + get_ColWidth(0);
	if(m_Y > firstRow)
	{
		if (m_X < firstCol)
		{
			Edit_BOM(m_Y);
			return ;
		}
	}*/
}
//


void CBOMInfo::BeforeMouseDownVsflexgrid( short Button, short Shift, float X, float Y, BOOL* Cancel )
{
	//CDataGridCtrl::BeforeMouseDownVsflexgrid(Button,Shift,X,Y,Cancel);
}

void CBOMInfo::Edit_BOM(long row)
{
	if(row > Row_DataToGrid(m_BOMAxisManager.GetRowCount()-1))
		return;

	YK_ULONG id ;
	m_BOMAxisManager.GetRowInfo(row - 1, id);

	if(m_BOMAxisManager.GetColor(row - 1) == RGB_COLOR_NONE)
		return;	// δָ�����ܵ����༭

	if (id >= 0)
	{
		MSG ms;
		ms.message = TblBOM;
		ms.wParam = id;
		ms.lParam = -1;
		::SendMessage(theApp.g_hMainFrm, IDU_UPDATE_MODIFY_DLG, (int)&ms, 0);
	}
}


void CBOMInfo::UnDoNew(const set<unsigned long>& idList)
{
	BIZAPI::InsertBOMReBackInfo(idList, OPERTYPEDELETE, TblBOM);
	set<unsigned long> flashItemlist;
	for (set<unsigned long>::const_iterator iter = idList.begin();
		iter != idList.end(); iter++)
	{
		m_BOMAxisManager.UnDoNew(*iter);
		flashItemlist.insert(GetBomItemidByBOMId(*iter));
	}
	// temp use bom ���ʱ���ܲ���ˢ��bom��ͼ  ��Ϊ��Ʒ����Ӧ
	BIZAPI::SetTableFlashFlg(TRUE,BOMViewFlash_Only);
	if(m_singleShow)
	{
		BIZAPI::SetTableFlashFlg(TRUE, TblBOM);
		// ˢ��BOM ��ͼ
		for (set<unsigned long>::iterator iter = flashItemlist.begin();
			iter != flashItemlist.end(); iter++)
		{
			if((*iter) > 0)
				FlashBomViewByItemId(*iter);
		}
	}
	else 
	{
		BIZAPI::SetTableFlashFlg(TRUE, ID_BOM_VIEW_DLG);
	}

	Flash();
}
void CBOMInfo::NewBOM(YK_ULONG classid, int type)
{
	if (type == 1)		// 
	{
		m_BOMAxisManager.NewBOM(m_NewRow-1, classid);
		// temp use bom ���ʱ���ܲ���ˢ��bom��ͼ  ��Ϊ��Ʒ����Ӧ
		BIZAPI::SetTableFlashFlg(TRUE, BOMViewFlash_Only);
		if(m_singleShow)
		{
			BIZAPI::SetTableFlashFlg(TRUE, TblBOM);
			// ˢ��BOM ��ͼ
			FlashBomViewByRow(m_NewRow);
		}
		else 
		{
			BIZAPI::SetTableFlashFlg(TRUE, ID_BOM_VIEW_DLG);
		}

		Flash();
	}
	else
	{
		m_BOMAxisManager.DelItem(classid);
	}
}

//void CBOMInfo::EditRecord(list<long>& rowList )
//{
//	list<unsigned long> ids;
//	for (list<long>::const_iterator iter = rowList.begin();
//		iter != rowList.end(); iter++)
//	{
//		long row = *iter;
//
//		if(row < 1)
//			continue;
//
//		unsigned long bomId = 0;
//		m_BOMAxisManager.GetRowInfo(row, bomId);
//		if (bomId > 0)
//			ids.push_back(bomId);
//	}
//
//	if(ids.size() > 0)
//		BIZAPI::InsertOpertion(1, GetTblType(), ids);
//}
void CBOMInfo::BeforeScrollVsflexgridu1(long OldTopRow, long OldLeftCol, long NewTopRow, long NewLeftCol, BOOL* Cancel)
{
	m_lOldBottomRow = get_BottomRow();
}
void CBOMInfo::AfterScrollVsflexgrid(long OldTopRow, long OldLeftCol, long NewTopRow, long NewLeftCol )
{
	if (NewTopRow == OldTopRow)
		return;

	if (NewTopRow > OldTopRow)
	{
		if (NewTopRow > m_lOldBottomRow)
			UpdateScreenData(NewTopRow, get_BottomRow());
		else
			UpdateScreenData(m_lOldBottomRow+1, get_BottomRow());
	}
	else
	{
		if (get_BottomRow() < OldTopRow)
			UpdateScreenData(NewTopRow, get_BottomRow());
		else
			UpdateScreenData(NewTopRow, OldTopRow-1);
	}
}

void CBOMInfo::BeforeScrollTipVsflexgrid( long Row )
{
	CString strTips;
	strTips.Format(_T("Row:%d"),Row);
	put_ScrollTipText(strTips);
}

void CBOMInfo::FlashBomViewByRow(long row)
{
	unsigned long itemId = GetBomItemidByRow(row);
	FlashBomViewByItemId(itemId);
}
void CBOMInfo::FlashBomViewByItemId(unsigned long itemId)
{
	if(itemId>0)
	{
		// ˢ��BOM��ͼ ��Ϣ
		::SendMessage(GetParent()->GetSafeHwnd(),ID_MESSAGE_FLASH_BOMVIEW,itemId,0);
	}
}
// ��ʾ��Ϣ
CString CBOMInfo::GetShowCode()
{
	CString cstr;
	int row = get_MouseRow();
	int col = get_MouseCol();
	if (row>=0&&col>=0)
	{
		if (row == 0&&col>0)
		{
			UINT enumIndex=m_BOMAxisManager.GetEnumIdByIndex(col-1);
			TBL_FIELD_TYPE* m_pFiledType = LANGUAGEPACKAPI::GetTblFieldTypeByType(TblType(m_BOMAxisManager.m_table.tblType));
			if (enumIndex == DATESAVE_SPEC_DATA_COL )
			{
				cstr = GetRemarkFiled(Spec_Date_Def_RMK).c_str();
			}
			else if (enumIndex == DATESAVE_SPEC_COL)
			{
				cstr = GetRemarkFiled(Spec_Spec_Def_RMK).c_str();
			}
			else if (enumIndex== DATESAVE_SPEC_RANGE_COL)
			{
				cstr = GetRemarkFiled(Spec_Range_Def_RMK).c_str();
			}
			else if(enumIndex < (*m_pFiledType).size())
			{
				UINT type = (*m_pFiledType)[enumIndex];
				cstr = GetRemarkFiled(type).c_str();
			}
			else if (IS_USER_DEF_ENM(enumIndex))
			{
				YKFieldPtr ptr = BIZAPI::GetBizMap()->GetYKFieldPtrMap()->Get(GetTblType(),enumIndex);
				if (NULL != ptr)
				{
					cstr = ptr->GetTipCode().c_str();
				}
			}
		}
		else cstr = get_TextMatrix(get_MouseRow(),get_MouseCol());
	}
	return cstr;
}
void CBOMInfo::MouseMoveVsflexgrid( short Button, short Shift, float X, float Y )
{
	CString szTip = GetShowCode();
	m_toolTip.Update(szTip);
}

//����Ԫ��..."ʱ�����Ի�����Ϣ��Ӧ
void CBOMInfo::CellButtonClickVsflexgridu1(long Row, long Col)
{
	InfoDlgBom(Row, Col, 1);
}

// �����Ի����޸�
void CBOMInfo::InfoDlgBom(long Row, long Col, int button)
{
	const long lastDataRow = Row_DataToGrid(m_BOMAxisManager.GetRowCount()-1);
	if (button == 1 && Col > 0 && Row > 0 && Col < get_Cols() && Row <= lastDataRow)
	{
		UINT tableType = TblBOM;
		CString code_s = get_TextMatrix(Row,Col);
		YK_ULONG mId = 0;
		m_BOMAxisManager.GetRowInfo(Row-1,mId);
		UINT enumIndex = m_BOMAxisManager.GetEnumIdByIndex(Col-1);

		YKFieldPtrMap* fMap = BIZAPI::GetBizMap()->GetYKFieldPtrMap();
		if(NULL == fMap) return;
		YKFieldPtr ptr = fMap->GetFieldPara(tableType,enumIndex);
		if(NULL == ptr) return;
		// Tm
		if (ptr->GetChekType() == CHECKTYPE_TIME1)
		{
			CTmEditDlg tmEditDlg;
			tmEditDlg.Set(tableType,enumIndex);
			tmEditDlg.SetTmType(BIZAPI::GetBizMap()->GetTimeShowType_UI());
			tmEditDlg.SetTm(code_s.GetString());
			if (tmEditDlg.DoModal() == IDOK)
			{
				CString cstr=tmEditDlg.GettmCode();
				if(cstr != code_s)	// change
				{
					//Add 2011-10-20
					YK_ULONG key=0;
					m_BOMAxisManager.GetRowInfo(Row-1,key);			//��ȡ����ID
					wstring  wstr = cstr.GetString();

					//Add 2011-12-16 ���ȷ������к�����֤
					if(BIZAPI::TableColsValueJudage(m_BOMAxisManager.m_table.tblType,key,m_BOMAxisManager.m_table.mode[Col-1].mode,wstr))
					{
						BomModify(m_BOMAxisManager.m_table.tblType,key,m_BOMAxisManager.m_table.mode[Col-1].mode,wstr);

						Flash(FALSE);
					}					
				}
			}
			return;
		}
		switch(tableType)
		{
		case TblBOM:
			{
				YKBOMMap* bomMap=BIZAPI::GetBizMap()->GetBOMPtrMap();
				if (NULL != bomMap)
				{
					YKBOMPtr& bomPtr=bomMap->Get(mId);
					if(NULL == bomPtr) return;
					if (enumIndex == YKBOM::BOM_useItemId)
					{
						UINT type=m_BOMAxisManager.GetRowInfo_type(Row-1);
						if (type == 1||type == 3)		// ��Ʒ
						{
							CBOMItemEditDlg itemEditDlg;
							itemEditDlg.SetItemId(bomPtr->GetUseItemId2());
							itemEditDlg.SetLastItemId(GetBomItemidByRow(Row));

							itemEditDlg.SetRelBomId(bomPtr->GetId(),0);
							YKProductRoutingPtr pudPtr=bomPtr->GetProductRoutingPtr();

							if (NULL != pudPtr)
							{
								itemEditDlg.SetPudlineId(pudPtr->GetId());
							}
							itemEditDlg.SetBomId(mId);

							////and by lq  2011-2-16
							//short type=ItemType_MiddleProduct;				// �м���Ʒ
							//if (type == 3 && bomPtr->GetPreProcessId()<=0)		// ���
							//	type = ItemType_Materials;
							//else if (type == 1 && bomPtr->GetPreProcessId()<=0)	// ����
							//	type =ItemType_HalfProduct;
							//itemEditDlg.SetItemType(type);

							long lProId = bomPtr->GetPreProcessId();
							if ( type == 3 )		//���
							{
								if ( lProId <0 )	//����ID С��0							
									type = ItemType_Materials;
								else				//����ID ���ڵ���0
									type = ItemType_HalfProduct;
							}
							else if ( type== 1 )	// ����
							{
								if( lProId<=0 )		//	ǰ����ID С�ڵ���0
									type =ItemType_Materials;
								else
									type = ItemType_MiddleProduct;	
							}
							itemEditDlg.SetItemType(type);

							if (itemEditDlg.DoModal()==IDOK)
							{
								CString cstr=itemEditDlg.GetItemCode2();
								if(cstr != code_s)	// change
								{
									//Add 2011-10-20
									wstring wstr = cstr.GetString();
									YK_ULONG key = 0;
									m_BOMAxisManager.GetRowInfo(Row-1,key);
									BomModify(TblBOM,key,YKBOM::BOM_useItemId,wstr);

									put_TextMatrix(Row,Col,cstr);
									BIZAPI::SetTableFlashFlg(TRUE,BOMViewFlash_Only);
									// ��Ʒ���Ŀ�����Ҫ������Ʒ��
									BIZAPI::SetTableFlashFlg(TRUE,BOM_VIEW_ITEMTREE);
									FlashBomViewByRow(Row);
								}
							}
						}
						else if (type == 2)		//  ��Դ
						{
							CNewModelDlg  dlg;
							dlg.SetCodeUseType(TRUE,code_s.GetString());
							dlg.TransInfo(TblBOM,YKBOM::BOM_useItemId,mId);
							if (dlg.DoModal() == IDOK)
							{
								CString cstr=dlg.GetCode_Cstr();
								if(cstr != code_s)	// change
								{
									put_TextMatrix(Row,Col,cstr);
			
									//Add 2011-10-20
									wstring wstr = cstr.GetString();
									YK_ULONG key = 0;
									m_BOMAxisManager.GetRowInfo(Row-1,key);

									BomModify(m_BOMAxisManager.m_table.tblType,key,m_BOMAxisManager.m_table.mode[Col-1].mode,wstr);
								}
							}
						}
					}
					else if (enumIndex == YKBOM::BOM_nexResId&&bomPtr->GetCmdType() == BOM_AvbRes)
					{
						CNewModelDlg  dlg;
						dlg.TransInfo(TblBOM,YKBOM::BOM_nexResId,mId);
						if (dlg.DoModal() == IDOK)
						{
							CString cstr=dlg.GetCode_Cstr();
							if(cstr != code_s)	// change
							{
								//Add 2011-10-20
								wstring wstr = cstr.GetString();
								YK_ULONG key = 0;
								m_BOMAxisManager.GetRowInfo(Row-1,key);
								BomModify(m_BOMAxisManager.m_table.tblType,key,m_BOMAxisManager.m_table.mode[Col-1].mode,wstr);

								put_TextMatrix(Row,Col,cstr);
							}
							
						}
					}
					else if (enumIndex == YKBOM::BOM_processCondition)
					{
						/*CNewModelDlg  dlg;
						dlg.SetDivisionStr(L",");
						dlg.TransInfo(TblBOM,YKBOM::BOM_processCondition,mId);*/

						CNewRuleDlg dlg;
						dlg.InitRule(RULE_DEF_FILTER_PudLine,code_s.GetString(),TblOrder);
						if (dlg.DoModal() == IDOK)
						{
							CString cstr=dlg.GetCode().c_str();
							if(code_s == L" ") code_s = L"";
							if(cstr != code_s)	// change
							{
								//Add 2011-10-20
								wstring wstr = cstr.GetString();
								YK_ULONG key = 0;
								m_BOMAxisManager.GetRowInfo(Row-1,key);
								BomModify(m_BOMAxisManager.m_table.tblType,key,m_BOMAxisManager.m_table.mode[Col-1].mode,wstr);

								Flash(FALSE);
							}
						}
					}
					else if (enumIndex == YKBOM::BOM_selectCondition)
					{
						if (bomPtr->GetCmdType() == BOM_Input
							||bomPtr->GetCmdType() == BOM_Output)
						{
							/*CNewModelDlg  dlg;
							dlg.SetDivisionStr(L",");
							dlg.TransInfo(TblBOM,YKBOM::BOM_selectCondition,mId);*/

							CNewRuleDlg dlg;
							dlg.InitRule(RULE_DEF_FILTER_PudInput,code_s.GetString(),TblOrder);
							if (dlg.DoModal() == IDOK)
							{
								CString cstr=dlg.GetCode().c_str();
								if(cstr != code_s)	// change
								{

									//Add 2011-10-20
									wstring wstr = cstr.GetString();
									YK_ULONG key = 0;
									m_BOMAxisManager.GetRowInfo(Row-1,key);
									BomModify(m_BOMAxisManager.m_table.tblType,key,m_BOMAxisManager.m_table.mode[Col-1].mode,wstr);

									Flash(FALSE);
								}
							}
						}
					}
					else if (enumIndex == DATESAVE_SPEC_COL)
					{
						YKBOMMap* bomMap=BIZAPI::GetBizMap()->GetBOMPtrMap();
						if (NULL != bomMap)
						{
							YKBOMPtr& bomPtr=bomMap->Get(mId);
							if(NULL == bomPtr) return;
							YKOperationPtr operPtr = bomPtr->GetOperPtr();
							if(NULL == operPtr) return;
							CNewModelDlg  dlg;
							dlg.SetDivisionStr(L"/");
							dlg.SetCodeUseType(TRUE,code_s.GetString());
							dlg.TransInfo(1000,TblOperation,operPtr->GetId());
							if (dlg.DoModal() == IDOK)
							{
								CString cstr=dlg.GetCode_Cstr();
								if(code_s == L" ") code_s = L"";
								if(cstr != code_s)	// change
								{
									//Add 2011-10-20
									wstring wstr = cstr.GetString();
									YK_ULONG key = 0;
									m_BOMAxisManager.GetRowInfo(Row-1,key);
									BomModify(m_BOMAxisManager.m_table.tblType,key,m_BOMAxisManager.m_table.mode[Col-1].mode,wstr);

									Flash(FALSE);
								}
							}
						}
					}
					else if(enumIndex == DATESAVE_SPEC_DATA_COL)
					{
						YKBOMMap* bomMap=BIZAPI::GetBizMap()->GetBOMPtrMap();
						if (NULL != bomMap)
						{
							YKBOMPtr& bomPtr=bomMap->Get(mId);
							if(NULL == bomPtr) return;
							YKOperationPtr operPtr = bomPtr->GetOperPtr();
							if(NULL == operPtr) return;
							CNewModelDlg  dlg;
							dlg.SetDivisionStr(L"/");
							dlg.SetCodeUseType(TRUE,code_s.GetString());
							dlg.TransInfo(1001,TblOperation,operPtr->GetId());
							if (dlg.DoModal() == IDOK)
							{
								CString cstr=dlg.GetCode_Cstr();
								if(code_s == L" ") code_s = L"";
								if(cstr != code_s)	// change
								{
									//Add 2011-10-20
									wstring wstr = cstr.GetString();
									YK_ULONG key = 0;
									m_BOMAxisManager.GetRowInfo(Row-1,key);
									BomModify(m_BOMAxisManager.m_table.tblType,key,m_BOMAxisManager.m_table.mode[Col-1].mode,wstr);

									Flash(FALSE);
								}
							}
						}
					}
					else if(enumIndex == YKBOM::BOM_DePart)
					{
						CNewModelDlg  dlg;
						dlg.TransInfo(TblBOM,YKBOM::BOM_DePart,mId);
						if (dlg.DoModal() == IDOK)
						{
							CString cstr=dlg.GetCode_Cstr();
							if(cstr != code_s)	// change
							{
								//Add 2011-10-20
								wstring wstr = cstr.GetString();
								YK_ULONG key = 0;
								m_BOMAxisManager.GetRowInfo(Row-1,key);
								BomModify(m_BOMAxisManager.m_table.tblType,key,m_BOMAxisManager.m_table.mode[Col-1].mode,wstr);

								Flash(FALSE);
							}
						}
						break;
					}
				}
				break;
			}
		}
	}
}

// ͨ���еõ�BOM����ƷID
unsigned long CBOMInfo::GetBomItemidByRow(long row )
{
	YK_ULONG bomId;
	m_BOMAxisManager.GetRowInfo(row-1, bomId);
	if(bomId <= 0) return 0;

	YKBOMMap* bomMap = BIZAPI::GetBizMap();
	if(NULL == bomMap) return 0;

	YKBOMPtr& bomPtr = bomMap->Get(bomId);
	if(NULL == bomPtr) return 0;

	YKItemPtr itemptr = bomPtr->GetItemPtr();
	if (NULL != itemptr)
		return itemptr->GetId();
	return 0;
}
unsigned long CBOMInfo::GetBomItemidByBOMId(unsigned long bomId)
{
	if(bomId<=0) return 0;

	YKBOMMap* bomMap=BIZAPI::GetBizMap();
	if(NULL == bomMap) return 0;

	YKBOMPtr& bomPtr=bomMap->Get(bomId);
	if(NULL == bomPtr) return 0;

	YKItemPtr itemptr=bomPtr->GetItemPtr();
	if (NULL != itemptr)
		return itemptr->GetId();
	return 0;
}
// �Ż��п�
void CBOMInfo::OptimizeTable()
{
	BeginWaitCursor();
	const long lMaxCols = get_Cols();
	const long lMaxRows = get_Rows();

	CDC *hDc = GetDC();		//�õ�DC
	wstring wstr;
	wstring wstr2;
	for (long col = 1; col < lMaxCols; ++col)//�е�ѭ��
	{
		//�õ���ͷ�����ݣ���Ϊ���ұ߹�����������ʱ����Ҫ�ͱ�ͷ���ݵĿ�����Ƚ�			
		int width = hDc->GetTextExtent(get_TextMatrix(0, col)).cx;

		for (long row = 0; row < lMaxRows; ++row)
		{
			//CString strTemp = get_TextMatrix(row, col);//�õ���top������
			//wstr2 = strTemp.GetString();
			//wstr= (wstr.size() >= wstr2.size()) ? wstr : wstr2;
			width = max(hDc->GetTextExtent(get_TextMatrix(row, col)).cx,width);
		}

		put_ColWidth(col, (width+12)*15);//����j�еĿ��
	}

	//�õ���ʱ��stTable�ṹ
	stTable &stTbl = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetName());
	for (long i = 1; i < get_Cols(); i++)
	{	//�޸Ĵ�ʱ���ȫ���е�width
		stTbl.mode[i-1].width = get_ColWidth(i);
		GetStTable().mode[i-1].width = get_ColWidth(i);
	}

	EndWaitCursor();
}

void CBOMInfo::HideCol(long col)
{
	if(col > 0)
		put_ColHidden(col, TRUE);
	m_listHideCol.push_back(col);//�洢�����е��к�

	//�޸�mode�еĶ�Ӧ��isHide Ϊ0
	stTable& curTable = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());
	curTable.mode[col - 1].SetIsHide(TRUE);// = 0;
	GetStTable().mode[col - 1].SetIsHide(TRUE);// = 0;

	//SaveData();///��������
}

void CBOMInfo::SetCols()
{
	CDlgSetCols dlgSetCols;
	dlgSetCols.SetsTbl(GetStTable());
	if (dlgSetCols.DoModal() == IDOK)
	{
		SetStTable(dlgSetCols.GetsTbl());
		Flash(FALSE);
	}
}

//ת���õ����е�CString�ַ���������sizeת���������
POINT ScrentToClient( POINT &pt) 
{
	pt.x = (pt.x + 12) * 15;
	pt.y = (pt.y + 12) * 15;
	return pt;
}

//��ȡ�е��п�
long CBOMInfo::GetColWidth( long col )
{
	int top = get_TopRow();//�õ��������
	int bottom = get_BottomRow();//�õ���Ͷ�����
	int left = get_LeftCol();//�õ����������
	int right = get_RightCol();//�õ����ұ�����
	int width = 0;//��width��ʱ����

	CDC *hDc = GetDC();//�õ�DC
	//	for (int j = left; j <= right; j++)//�е�ѭ��
	{//�õ���ͷ�����ݣ���Ϊ���ұ߹�����������ʱ����Ҫ�ͱ�ͷ���ݵĿ�����Ƚ�			
		CString str = get_TextMatrix(0,col);
		CSize s;
		s = hDc->GetTextExtent(str);//�õ��ƶ��ַ�����size
		POINT pTemp1;
		pTemp1.x = s.cx;
		pTemp1.y = s.cy;
		ScrentToClient( pTemp1);//��sizeת��������еĴ�С

		width = pTemp1.x > width ? pTemp1.x : width;//���ͷ���ݳ������Ƚ�
	}

	return width;
}


void CBOMInfo::AfterUserResizeVsflexgridu1( long Row, long Col )
{
	if(Col < 1 )	return;

	int width = get_ColWidth(Col);//�õ����еĿ��
	stTable &stTbl = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());//�õ��˱��stTable�ṹ
	stTbl.mode[Col - 1].width = width;//�޸ı��п�
	m_BOMAxisManager.m_table.mode[Col - 1].width = width;//�޸ı��п�

	//SaveData();///��������
}

void CBOMInfo::BeforeMoveColumnVsflexgrid( long Col, long* Position )
{
	const long lForzenCol = get_FrozenCols();
	if (Col <= lForzenCol || (*Position) <= lForzenCol)
	{
		*Position = Col;
	}
}

void CBOMInfo::AfterMoveColumnVsflexgrid( long Col, long* Position )
{
	if(Col == (*Position))
		return;

	stTable& curTable = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());
	DATATABLAPI::GoTostTblPos(curTable, Col - 1, (*Position)-1);
	SetStTable(curTable);
	
	ReSetDataSource();
	RedrawWindow();
}
//������Ԫ��ı���ɫ
void CBOMInfo::FillErrorColor(long Row, long Col)
{
	_variant_t var2; 
	var2.vt = VT_I4;
	var2.lVal = GRID_COLOR_EDITERROR;
	put_Cell(flexcpBackColor, COleVariant((long)Row), COleVariant(Col),
		COleVariant((long)(Row)), COleVariant((long)(Col)), var2);
//	var2.Detach();
}

vector<long> CBOMInfo::GetSameField(long Row, long Col)
{
	vector<long>  sBomIds;
	YK_ULONG bomId;
	m_BOMAxisManager.GetRowInfo(Row-1,bomId);
	if(bomId<=0) return sBomIds ;

	YKBOMMap* bomMap=BIZAPI::GetBizMap();
	if(NULL == bomMap) return sBomIds;

	YKBOMPtr bomPtr=bomMap->Get(bomId);
	if(NULL == bomPtr) return sBomIds;

	UINT  enumId = m_BOMAxisManager.GetEnumIdByIndex(Col-1); 
	if ( enumId == YKBOM::BOM_processPriority )
	{
		YKProductRoutingPtr proroutingptr = bomPtr->GetProductRoutingPtr();  
		if ( proroutingptr !=  NULL )
		{
			vector<unsigned long>bomIds = m_BOMAxisManager.GetBOMId();
			int i = 1;
			for ( vector<unsigned long>::iterator iter = bomIds.begin() ;
				iter != bomIds.end() ; iter++ , i++ )
			{
				YKBOMPtr pBom = bomMap->Get(*iter);
				if(NULL != pBom) 
				{
					YKProductRoutingPtr pPro =  pBom->GetProductRoutingPtr();  
					if ( pPro !=  NULL&& pPro->GetId() == proroutingptr->GetId() )
					{
						sBomIds.push_back( i );
					}
				}
			}
		}
	}
	else if ( enumId == YKBOM::BOM_processId 
		|| enumId == YKBOM::BOM_SplitRate				//�ָ����/* */)
		|| enumId == YKBOM::BOM_SplitAmount			//�ָ���/* */)
		|| enumId == YKBOM::BOM_SplitBatch				//�ָ�����/* */)
		|| enumId == YKBOM::BOM_MaxBatch				//���ָ�����/* */)
		|| enumId == YKBOM::BOM_MinBatch				//��С�ָ�����/* */)
		|| enumId == YKBOM::BOM_BatchUnit				//�ָ�������λ/* */)
		|| enumId == YKBOM::BOM_WorkParallelAmount)		//������������/* */) )
	{
		YKOperationPtr operPtr = bomPtr->GetOperPtr();  
		if ( operPtr !=  NULL )
		{
			vector<unsigned long>bomIds = m_BOMAxisManager.GetBOMId();
			int i = 1;
			for ( vector<unsigned long>::iterator iter = bomIds.begin() ;
				iter != bomIds.end() ; iter++ , i++ )
			{
				YKBOMPtr pBom = bomMap->Get(*iter);
				if(NULL != pBom) 
				{
					YKOperationPtr pOper =  pBom->GetOperPtr();  
					if ( pOper !=  NULL && pOper->GetId() == operPtr->GetId() )
					{
						sBomIds.push_back( i );
					}
				}
			}
		}
	}	
	return sBomIds;
}

void CBOMInfo::UpdateScreenData(long topRow, long bottomRow)
{
	put_Redraw(flexRDNone);

	const long lClrLen = m_BOMAxisManager.GetColorLine();
	const long lFieldCount = m_BOMAxisManager.GetFieldCout();
	const long lMaxRow = get_Rows();
	for (long row = topRow; row <= bottomRow; row++)
	{
		if (row < lMaxRow)
		{
			if(row == lMaxRow - 1)
			{
				put_TextMatrix(row, 0, L"*");	
				continue;
			}

			put_TextMatrix(row, 0, TOOLS::IToWString(row, 10).c_str());

			_variant_t var;
			var.vt = VT_I4;
			var.lVal = m_BOMAxisManager.GetColor(row - 1);
			if (lFieldCount > lClrLen)
				put_Cell(flexcpBackColor, COleVariant(row), COleVariant(lClrLen), COleVariant(row), COleVariant(lFieldCount), var);

			UINT havePud = 1;
			for (long col = 0; col < lFieldCount; col++)		// col
			{
				put_TextMatrix(row, col+1, m_BOMAxisManager.GetCode(row-1, col, havePud).c_str());

				short cmdType = 0;
				if (var.lVal == RGB_COLOR_IN)
					cmdType = BOM_Input;
				else if (var.lVal == RGB_COLOR_OUT)
					cmdType = BOM_Output;
				else if (var.lVal == RGB_COLOR_AVB)
					cmdType = BOM_AvbRes;

				// ���ò��ܱ༭�ĵ�Ԫ��������ɫ add 2010-9-25
				UINT enumCol = m_BOMAxisManager.m_table.mode[col].mode;
				if (var.lVal == RGB_COLOR_NONE &&
					(enumCol == YKBOM::BOM_cmdType || enumCol == YKBOM::BOM_processCode))		// δָ����BOM
				{
					// 
				}
				else havePud = 1;

				YKFieldPtr temppara = BIZAPI::GetBizMap()->GetFieldPara(m_BOMAxisManager.m_table.tblType, enumCol);

				if ((NULL == temppara) ||
					(temppara->GetModifyFly() == FILEDPARA_ISMODIFY_FALSE ||
					temppara->HaveVirRule() || !YKBOM::CanbeEdit(cmdType, enumCol, (havePud == 1))))
				{
					_variant_t var2;
					var2.vt = VT_I4;
					var2.lVal = GRID_COLOR_CANNOTEDIT;
					put_Cell(flexcpBackColor, COleVariant(row), COleVariant(col+1), COleVariant(row), COleVariant(col+1), var2);
				}
			}
		}
	}

	put_Redraw(flexRDBuffered);
}

//��ʼ��map��list�е�isHide
void CBOMInfo::SetColHide()
{
	//ͨ��mode�е�isHide��ʼ���������ص���
	YKFieldPtrMap* pFieldMap =  BIZAPI::GetBizMap()->GetYKFieldPtrMap();

	YKFieldPtr ptr;
	vector<stVector>::iterator iter = GetStTable().mode.begin();
	for (int i = 1; iter != GetStTable().mode.end(); iter++, i++)
	{
		put_ColHidden(i, iter->GetIsHide());

		ptr = pFieldMap->Get(GetTblType(), iter->mode);
		if (NULL != ptr && !ptr->CanUse())
			put_ColHidden(i, TRUE);
	}
}

//��ʼ��map ��list��width
void CBOMInfo::SetColWidth()
{
	//ͨ���õ�mode�е�width���ó�ʼ����п�
	vector<stVector>::iterator iter = GetStTable().mode.begin();
	for (int i = 1; iter != GetStTable().mode.end(); iter++, i++)
	{
		if(iter->width == 1200)
			iter->width = GetColWidth(i);

		put_ColWidth(i, iter->width);
	}
}

void CBOMInfo::BomModify(unsigned int tblType,int id,int col,wstring& var)
{
	//��������
	//BIZAPI::BeginRebackOper(OPERTYPEMODIFY);
	set<unsigned long> idList;
	idList.insert(id);
	BIZAPI::InsertBOMReBackInfo(idList,OPERTYPEMODIFY,tblType,false);
	//BIZAPI::EndRebackOper();

	BIZAPI::InsertValue(tblType,id,col,var);				
}

void CBOMInfo::ReplaceOnePrivate( int Row, int Col, CString strReplace, CString strFind )
{
	CString strText;
	strText = get_TextMatrix(Row, Col);
	strText.Replace(strFind, strReplace);

	//Add 2011-11-21 ���OnItemChanged ����
	int tablCol = m_BOMAxisManager.m_table.mode[Col-1].mode;
	if (tablCol > 0)
	{
		YK_ULONG key = 0;
		m_BOMAxisManager.GetRowInfo(Row-1, key);	//��ȡ����ID
		YK_WSTRING wstr = strText.GetString();
		BomModify(m_BOMAxisManager.m_table.tblType, key, tablCol, wstr);
	}
}