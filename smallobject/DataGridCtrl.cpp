#include "stdafx.h"

#include "MainFrm.h"
#include "DataGridCtrl.h"

#include "ComFunc.h"

#include  "Biz_API.h"
#include "EnumDef.h"
#include "Resource.h"
#include "Tools.h"
//#include "DataSave.h"
#include "CCreatGridMenu.h"
#include "clock.h"
#include "WatchBoardEndTmDlg.h"
#include "WavbResChangeDlg.h"
#include "SetTmDlg.h"
#include "ModifyCtrl.h"
#include "LoadFileDlg.h"
#include "MarksSetDlg.h"
#include "OrderReationConditionDlg.h"
#include "OrderInfoTranDlg.h"
#include "CalendarMgrDlg.h"
#include "MTOrderContionSetDlg.h"
#include <atldbcli.h>
#include "TableName.h"
#include "MainKeySetDlg.h"
#include "NewModelDlg.h"
#include "TmEditDlg.h"
#include "FlexUtil.h"
#include "NewRuleDlg.h"
#include "UtilUI.h"
#include "ThreadMsgDef.h"
#include "DataSaveWstr.h"
//////////////////////////////////////////////////////////////////////////
#ifdef _CLOCK_DEBUG
#define  CLOCK_PASE(tm)  tm.Pause();
#else
#define  CLOCK_PASE(tm)
#endif
//////////////////////////////////////////////////////////////////////////

#define FIRSTROWHEIGHT		400			//��һ���и�
#define ROWHEIGHT			345			//��ʼ��ÿ���и�
#define	INITROWCOUNT		60			//��ʼ��ʱ����������

#define NORMALGRID			1			//��ͨ���
#define OUTLINEGRID			2			//��outline("+")�ı�� 

#define  WORD_ASTERISK				L"*"
#define  WORD_VERLINE				L"!"
#define  WORD_EQUAL					L"="
///////////////

IMPLEMENT_DYNCREATE (CDataGridCtrl, CVsflexgridn1);

BEGIN_EVENTSINK_MAP(CDataGridCtrl, CWnd)
	ON_EVENT(CDataGridCtrl, 124, 21, CDataGridCtrl::BeforeEditVsflexgrid, VTS_I4 VTS_I4 VTS_PBOOL)
	ON_EVENT(CDataGridCtrl, 124, 24, CDataGridCtrl::AfterEditVsflexgrid, VTS_I4 VTS_I4)
	ON_EVENT(CDataGridCtrl, 124, 11, CDataGridCtrl::AfterScrollVsflexgrid, VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	//ON_EVENT(CDataGridCtrl, 124, 13, CDataGridCtrl::BeforeSortVsflexgrid, VTS_I4 VTS_PI2)
	ON_EVENT(CDataGridCtrl, 124, DISPID_KEYDOWN, CDataGridCtrl::KeyDownVsflexgrid, VTS_PI2 VTS_I2)
	ON_EVENT(CDataGridCtrl, 124, DISPID_MOUSEDOWN, CDataGridCtrl::MouseDownVsflexgrid, VTS_I2 VTS_I2 VTS_R4 VTS_R4)
	ON_EVENT(CDataGridCtrl, 124, DISPID_MOUSEUP, CDataGridCtrl::MouseUpVsflexgrid, VTS_I2 VTS_I2 VTS_R4 VTS_R4)
	ON_EVENT(CDataGridCtrl, 124, DISPID_MOUSEMOVE, CDataGridCtrl::MouseMoveVsflexgrid, VTS_I2 VTS_I2 VTS_R4 VTS_R4)
	ON_EVENT(CDataGridCtrl, 124, DISPID_DBLCLICK, CDataGridCtrl::DblClickVsflexgrid, VTS_NONE)
	ON_EVENT(CDataGridCtrl, 124, 16, CDataGridCtrl::AfterMoveColumnVsflexgrid, VTS_I4 VTS_PI4)
	//ON_EVENT(CDataGridCtrl, 124, 8, CDataGridCtrl::BeforeSelChangeVsflexgridu1, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL)
	ON_EVENT(CDataGridCtrl, 124, 23, CDataGridCtrl::ValidateEditVsflexgrid, VTS_I4 VTS_I4 VTS_PBOOL)
	//ON_EVENT(CDataGridCtrl, 124, 5, CDataGridCtrl::BeforeMouseDownVsflexgrid, VTS_I2 VTS_I2 VTS_R4 VTS_R4 VTS_PBOOL)
	ON_EVENT(CDataGridCtrl, 124, 17, CDataGridCtrl::BeforeUserResizeVsflexgridu1, VTS_I4 VTS_I4 VTS_PBOOL)
	ON_EVENT(CDataGridCtrl, 124, 18, CDataGridCtrl::AfterUserResizeVsflexgridu1, VTS_I4 VTS_I4)
	ON_EVENT(CDataGridCtrl, 124, 10, CDataGridCtrl::BeforeScrollVsflexgridu1, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL)
	//ON_EVENT(CDataGridCtrl, 124, 3, CDataGridCtrl::EnterCellVsflexgridu1, VTS_NONE)
	ON_EVENT(CDataGridCtrl, 124, 34, CDataGridCtrl::CellButtonClickVsflexgridu1, VTS_I4 VTS_I4)
	//ON_EVENT(CDataGridCtrl, 124, 82, CDataGridCtrl::AfterDataRefreshVsflexgrid, VTS_NONE)

	ON_EVENT(CDataGridCtrl, 124, 19, CDataGridCtrl::BeforeCollapseVsflexgrid, VTS_I4 VTS_I2 VTS_PBOOL)
	ON_EVENT(CDataGridCtrl, 124, 20, CDataGridCtrl::AfterCollapseVsflexgrid, VTS_I4 VTS_I2)

	ON_EVENT(CDataGridCtrl, 124, 15, CDataGridCtrl::BeforeMoveColumnVsflexgrid, VTS_I4 VTS_PI4)

END_EVENTSINK_MAP()

//BOOL CDataGridCtrl::m_bCanPaste = FALSE;
BOOL CDataGridCtrl::m_bClipBoardEmpty = TRUE;

CDataGridCtrl::CDataGridCtrl(void)
{
	m_hCursor = AfxGetApp()->LoadCursor(IDC_CURSOR1);

	m_bCanPaste = FALSE;
	InitMemberData();
}

CDataGridCtrl::~CDataGridCtrl(void)
{
	ClearCopyRowBasePtr();
}

POINT MouseToScreen(HWND hWnd, float X, float Y) 
{
	// get screen resolution (once)
	static POINT logPix = {0,0};
	if (logPix.x == 0)
	{
		HDC hdc = GetDC(NULL);
		logPix.x = GetDeviceCaps(hdc, LOGPIXELSX); 
		logPix.y = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
	}

	// convert client twips to client pixels
	POINT pt;
	pt.x = MulDiv(logPix.x, (int)X, 1440);
	pt.y = MulDiv(logPix.y, (int)Y, 1440);

	// convert client pixels to screen pixels
	ClientToScreen(hWnd, &pt);

	// done
	return pt;
}

POINT MouseToScreen2(HWND hWnd, float X, float Y) 
{
	// get screen resolution (once)
	static POINT logPix = {0,0};
	if (logPix.x == 0)
	{
		HDC hdc = GetDC(NULL);
		logPix.x = GetDeviceCaps(hdc, LOGPIXELSX); 
		logPix.y = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
	}

	// convert client twips to client pixels
	POINT pt;
	pt.x = MulDiv(logPix.x, (int)X, 1440);
	pt.y = MulDiv(logPix.y, (int)Y, 1440);

	// convert client pixels to screen pixels
	//ClientToScreen(hWnd, &pt);
	// done
	return pt;
}

//ת���õ����е�CString�ַ���������sizeת���������
POINT ScrentToClient(HWND hWnd, POINT &pt) 
{
	pt.x = (pt.x + 12) * 15;
	pt.y = (pt.y + 12) * 15;
	return pt;
}
BOOL CDataGridCtrl::OnCreate( CWnd* pParentWnd ,LPRECT lpRect )
{
	BOOL rt  = FALSE;
	if(lpRect == NULL)
	{
		rt =  Create (L"DataGridCtrl",WS_CHILD | WS_VISIBLE, 
			CRect (0, 0, 0, 0), pParentWnd, 124);
	}
	else
	{
		rt =  Create (L"DataGridCtrl",WS_CHILD | WS_VISIBLE, 
			*lpRect, pParentWnd, 124);
	}

	if(!rt)
		return rt;

	BIZAPI::SetTableFlashFlg(TRUE, GetTblType());

	m_toolTip.Create((HWND)GetSafeHwnd());
	if (GetTblType() == TblMessageData)
		InitIcons();

	put_ExplorerBar(flexExMove);

	put_FlexDataSource((LPDISPATCH)& m_FlexDataSource);//��������
	return rt;
}

void CDataGridCtrl::InitIcons()
{
	CImageList images;
	images.Create (IDB_BITMAP_INFO, 20, 0, COLOR_Image1/*RGB (192,192, 192)*/);
	for (int i = 0; i < images.GetImageCount(); i++)
	{
		const HICON hIconFolder = images.ExtractIcon(i);
		CPictureHolder picFolder;
		picFolder.CreateFromIcon(hIconFolder, TRUE);

		COleVariant vPicFolder;
		V_VT(&vPicFolder) = VT_DISPATCH;
		V_DISPATCH(&vPicFolder) = picFolder.GetPictureDispatch();
		m_vecIcons.push_back(vPicFolder);
	}

	images.DeleteImageList();
}


void CDataGridCtrl::InitMemberData()
{
	m_uTabNameIndex = GetStTable().GetNameIndex();
	m_bCrossCursor = FALSE;

	m_rtDlgMenu.SetRectEmpty();
	m_lClickRow = m_lclickCol = 0;
	m_lDlgMenuSpace = 0;
	m_bCanNewData = FALSE;
	m_bInAntoResize = FALSE;
	m_bReDraw = FALSE;
	m_nClientRowCount = 0;
	m_lCurTopRow = 0;
	m_lCurLeftCol = 0;

	m_bEditing = FALSE;
}
void CDataGridCtrl::InitFlexGrid()
{
	// �л���ǩ�����ó�Ա����
	if (m_uTabNameIndex != GetStTable().GetNameIndex())
		InitMemberData();

	const long lRDProperty = get_Redraw();
	put_Redraw(flexRDNone);
	ClearFlexGrid();	// ��ձ������

	ResertDataSource();
	put_AllowSelection(TRUE);

	// �̶���Ԫ�������͡���ɫ������ɫ����ǹ̶�������ɫ
	put_GridLinesFixed(flexGridFlat);
	put_GridColorFixed(COLOR_Data_FixGid/*RGB(170,177,193)*/);
	put_BackColorFixed(GRID_COLOR_TOP1);
	if (TblMessageData != GetTblType())
		put_SheetBorder(COLOR_Data_Gid_SelBkg/*RGB(0,0,0)*/);
	else
		put_SheetBorder(COLOR_Data_FixGid/*RGB(170,177,193)*/);
	
	// ����������������ͣ������
	put_GridLines(flexGridFlat);
	put_GridColor(COLOR_Data_Gid/*RGB(170,177,193)*/); // �����ߵ���ɫ
	put_BackColorBkg(COLOR_Data_Gid_Bkg/*RGB (255, 255, 255)*/);
	put_Ellipsis(flexEllipsisEnd); // �ڸ�������ʡ��
	put_FontName(L"����");
	put_FontSize((float)10); // ��������
	put_ColAlignment(-1, flexAlignLeftCenter);
	put_FocusRect(flexFocusSolid); // ȥ��ϵͳ�Զ������߿�
	put_RowHeight(-1, ROWHEIGHT); // ���������߶�
	put_RowHeight(0, FIRSTROWHEIGHT); // ��ͷ�ĸ߶�

	// ����������
	put_AutoSearch(flexSearchNone);
	put_Editable(flexEDKbdMouse);
	put_AllowUserResizing(flexResizeBoth);
	put_SelectionMode(flexSelectionFree);
	put_ScrollTrack(FALSE);
	put_ScrollTips(TRUE);
	put_OwnerDraw(flexODContent);

	// ���ѡ��ʱ�����
	put_HighLight(flexHighlightWithFocus);
	put_ForeColorSel(get_BackColor());
	put_BackColorSel(COLOR_Data_Gid_SelBkg/*RGB(0, 0, 0)*/);

	// ��������������������������
	if(GetTblType() == TblOrder || GetTblType() == TblWork)
	{
		put_OutlineCol(1);
		put_FrozenCols(1);
		put_OutlineBar(flexOutlineBarCompleteLeaf);
		put_SubtotalPosition(flexSTAbove);
		put_IsSubtotal(-1, FALSE);
		put_IsSubtotal(-1, TRUE);
	}

	// ���ù̶���
	put_FrozenCols(GetStTable().fixCol);
	if (GetTblType() == TblMessageData)
	{
		put_ExplorerBar(flexExSort);
		put_ExtendLastCol(TRUE);
	}

	StartHideCol();//�����еĳ�ʼ����ʾ
	StartSetWidth();//��ʼ�������п�

	UpdateClientRowCount();
	put_Redraw(flexRDBuffered);
}

// �̶߳�ȡ����ʱ���ã��Ը���FlexGrid�Ľ������ݼ�������Ϣ
void CDataGridCtrl::UpdateFlexGrid(BOOL bFlashed)
{
	if (FALSE == bFlashed)
		Flash();
	else
		ResertDataSource();
}

void CDataGridCtrl::ClearFlexGrid()
{
	Clear(COleVariant((long)flexClearEverywhere), COleVariant((long)flexClearEverything));
	// ��ѡ���ڲ��Ϸ�����
	Select(0, 0, COleVariant((long)0), COleVariant((long)0));

	ClearDlgMenu();
}

void CDataGridCtrl::ClearDlgMenu()
{
	if (m_dlgMenu.GetSafeHwnd())
		m_dlgMenu.DestroyWindow();

	// ���dlgMenuѡ��ı߿�
	if (!m_rtDlgMenu.IsRectNull())
	{
		CellBorderRange(min(m_rtDlgMenu.top, m_rtDlgMenu.bottom), m_rtDlgMenu.left, 
			max(m_rtDlgMenu.top, m_rtDlgMenu.bottom), m_rtDlgMenu.right, 
			COLOR_Bom_Cancel_SelBkg/*RGB(255, 255, 255)*/, 0, 0, 0, 0, 0, 0);
		m_rtDlgMenu.SetRectEmpty();
		put_SelectionMode(flexSelectionFree);
	}
}

BOOL CDataGridCtrl::UpdateClientRowCount()
{
	BOOL bOK = FALSE;

	int nRowCount = GetClientRowCount();
	const UINT uTblType = GetTblType();
	if (TblOrder == uTblType || TblWork == uTblType)
	{
		const long lAllRows = get_Rows();
		nRowCount = lAllRows;
		if (lAllRows > 1)
			nRowCount = (get_ClientHeight()-get_RowHeight(0)) / get_RowHeight(1) + 2;
	}
	else
	{
		nRowCount = get_BottomRow() - get_TopRow() + 1;
	}

	if (nRowCount != GetClientRowCount())
	{
		SetClientRowCount(nRowCount);
		bOK = TRUE;
	}

	return bOK;
}
// ��ȡ�������
long CDataGridCtrl::GetFlexGridType()
{
	long lValue = NORMALGRID;

	if(TblWork == GetTblType() || TblOrder == GetTblType())
		lValue = OUTLINEGRID;

	return lValue;
}

void CDataGridCtrl::SortTable()
{
	m_FlexDataSource.WaitForThreadQuit();

	BeginWaitCursor();
	put_Rows(1);
	const UINT uTblType = GetTblType();
	switch (uTblType)
	{
	case TblOrder:
		m_FlexDataSource.SortOrderTbl();
		break;
	case TblWork:
		m_FlexDataSource.SortWorkTbl();
		break;
	default:
		m_FlexDataSource.SortGridRowList();
	}

	Flash();
	EndWaitCursor();
}


void CDataGridCtrl::InitSource()
{
	m_FlexDataSource.InitAxis(this);
}

// ��ʼ����Դ��ʽ��:		�����µ������б�(ֱ������  ����bizDataManager��ȡ)
void CDataGridCtrl::InitSource(vector<GridRowInfo>& idList)
{
	put_Rows(1);
	m_FlexDataSource.InitAxis(idList);
}

// ��������Դ
void CDataGridCtrl::ResertDataSource()
{
	put_FlexDataSource((LPDISPATCH)&m_FlexDataSource);
	put_VirtualData(TRUE);
}



void CDataGridCtrl::ResetSource()
{
	m_FlexDataSource.ResetBizDateManager();
}



// ˢ����ͼ  tblFlash=TRUE������ҳǩ���ˢ��(��Ҫ����ȡ���� ����ˢ������) 
// ���򣺲����¶�ȡ����  �������ݵ�stble�ͺ���  add 2010-5-17 by ll
// tblFlash:�Ƿ�ȫ������ˢ�±�־
// Ĭ��: tblFlash=TRUE
void CDataGridCtrl::Flash( BOOL bReLoadData)
{
	if(bReLoadData && BIZAPI::GetTableFlashFlg(GetTblType()))
	{
		put_Rows(1);
		m_FlexDataSource.InitAxis(this);
		BIZAPI::SetTableFlashFlg(FALSE, GetTblType());
		return;		// �̶߳�ȡ���ݣ��տ�ʼ��ȡ����ʱ������ƽ��棬��Ϊ���ݻ�û��ȡ
	}

	InitFlexGrid();

	SetBackState();
	ReDrawScreen(get_TopRow());
#ifdef LIMITFUNCTION
	BIZAPI::LimitCheck();
#endif
}

// ����������
void CDataGridCtrl::StartHideCol()
{	
	//ͨ��mode�е�isHide��ʼ���������ص���
	YKFieldPtrMap* pFieldMap =  BIZAPI::GetBizMap()->GetYKFieldPtrMap();
	int i = 0;
	bool bHide = false;
	for (vector<stVector>::iterator iter = GetStTable().mode.begin(); 
		iter != GetStTable().mode.end(); iter++, i++)
	{
		stVector& stv = *iter;
		put_ColHidden(i + 1, stv.IsHide());

		YKFieldPtr ptr = pFieldMap->Get(GetTblType(), stv.mode);
		if (ptr != NULL && !ptr->CanUse())
		{
			put_ColHidden(i + 1,TRUE);
		}
	}
}

// ���õ�Ԫ���п�
void CDataGridCtrl::StartSetWidth()
{
	//ͨ���õ�mode�е�width���ó�ʼ����п�
	int i = 0;
	for (vector<stVector>::iterator iter = GetStTable().mode.begin(); 
		iter != GetStTable().mode.end(); iter++, i++)
	{
		if (1200 == iter->width)
		{
			iter->width = GetColWidth(i+1);

			//����������ĵ�һ��
			if(GetFlexGridType() == OUTLINEGRID && i == 0)
			{
				iter->width += 800;
			}
		}

		put_ColWidth(i + 1, iter->width);
		//if((*iter).width == 0)
		//{
		//	(*iter).width = GetColWidth(i+1);

		//	//����������ĵ�һ��
		//	if(GetFlexGridType() == OUTLINEGRID && i == 0)
		//	{
		//		(*iter).width += 800;
		//	}
		//}

		//put_ColWidth(i + 1, (*iter).width);
	}
}

long CDataGridCtrl::GetColWidth( long col )
{
	int top = get_TopRow();//�õ��������
	int bottom = get_BottomRow();//�õ���Ͷ�����
	int left = get_LeftCol();//�õ����������
	int right = get_RightCol();//�õ����ұ�����
	int width = 0;//��width��ʱ����

	CDC *hDc = GetDC();//�õ�DC

	{//�õ���ͷ�����ݣ���Ϊ���ұ߹�����������ʱ����Ҫ�ͱ�ͷ���ݵĿ�����Ƚ�			
		CString str = get_TextMatrix(0,col);
		CSize s;
		s = hDc->GetTextExtent(str);//�õ��ƶ��ַ�����size
		POINT pTemp1;
		pTemp1.x = s.cx;
		pTemp1.y = s.cy;
		ScrentToClient(GetSafeHwnd(), pTemp1);//��sizeת��������еĴ�С

		width = pTemp1.x > width ? pTemp1.x : width;//���ͷ���ݳ������Ƚ�

	}

	ReleaseDC(hDc);
	return width;
}

void CDataGridCtrl::ReDrawScreen(long lBegRow, BOOL bDown /* = TRUE */)
{
	const UINT uTblType = GetTblType();
	if (0 == uTblType)
		return;

	m_bReDraw = TRUE;
	// ����RedrawģʽΪflexRDNone���ɼӿ������ٶ�
	// ���п��ܵ���topRow��bottomRow�����ݲ�׼ȷ
	// Ҫ���׼ȷ��topRow��bottomRow�Ȱ�Redraw��ΪflexRDBuffered���ٻ�ȡ
	const long lRDProperty = get_Redraw();
	put_Redraw(flexRDNone);

	put_BackColor(RGB(255, 255, 255));

	// �������������ػ洦��
	if (TblOrder == uTblType || TblWork == uTblType)
	{
		UpdateScreenData(lBegRow, bDown);
		m_FlexDataSource.UpdateFixedColNum(Row_GridToData(get_TopRow()), GetClientRowCount());
	}

	SetTblAlternateClr();

	SetCanNotEditCols();
	SetBackColors();
	FillErrorColor();

	put_Redraw(flexRDBuffered);
	m_bReDraw = FALSE;
}

void CDataGridCtrl::UpdateScreenData(long begRow, BOOL bDown /* = TRUE */, BOOL bModeFlag /* = FALSE */)
{
	long lRootIndex = 0, lCurIndex = 0, lOldRIndex = 0;
	long lLimitRow = 0;
	if (bDown)
	{
		lCurIndex = m_FlexDataSource.GetRoot(Row_GridToData(begRow));
		lRootIndex = lCurIndex;
		lOldRIndex = lRootIndex;
		do 
		{
			if (m_FlexDataSource.GetGridRowId(lCurIndex) > -1)
			{
				do 
				{
					put_RowOutlineLevel(Row_DataToGrid(lCurIndex), m_FlexDataSource.GetGridRowLevel(lCurIndex));
				} while (m_FlexDataSource.GetGridRowLevel(++lCurIndex) > 0);

				for (long index = lRootIndex; index < lCurIndex; index++)
				{
					if (m_FlexDataSource.GetGridRowShowFlag(index))
						put_IsCollapsed(Row_DataToGrid(index), m_FlexDataSource.GetGridRowState(index));
				}

				lRootIndex = lCurIndex;
			}
			else
			{
				lCurIndex++;
			}

			// �˴�RedrawģʽΪflexRDNone��δ�����к�Ӱ��
			// ����Ȳ��ð�Redrawģʽ��ΪflexRDBuffered
			//put_Redraw(flexRDBuffered);
			lLimitRow = get_BottomRow();
			//put_Redraw(flexRDNone);
		} while (Row_DataToGrid(lCurIndex) <= lLimitRow);

		if (-1 == m_FlexDataSource.GetGridRowId(lCurIndex))
		{
			// �ﵽ���ݱ��ĩβ�����ϲ��ң���ʱ��õ�topRow�᲻׼ȷ��������bModeFlagΪTRUE
			UpdateScreenData(Row_DataToGrid(lOldRIndex), FALSE, TRUE);
		}
	}
	else
	{
		lRootIndex = m_FlexDataSource.FindNextRootIndex(Row_GridToData(begRow));
		lCurIndex = lRootIndex;
		do 
		{
			lCurIndex--;
			if (m_FlexDataSource.GetGridRowId(lCurIndex) > -1)
			{
				do 
				{
					put_RowOutlineLevel(Row_DataToGrid(lCurIndex), m_FlexDataSource.GetGridRowLevel(lCurIndex));
				} while (m_FlexDataSource.GetGridRowLevel(lCurIndex--) > 0);

				for (long index = (++lCurIndex); index < lRootIndex; index++)
				{
					if (m_FlexDataSource.GetGridRowShowFlag(index))
						put_IsCollapsed(Row_DataToGrid(index), m_FlexDataSource.GetGridRowState(index));
				}

				lRootIndex = lCurIndex;
			}

			if (bModeFlag)
			{
				// topRow�᲻׼ȷ��������RedrawΪflexRDBuffered��������topRow
				put_Redraw(flexRDBuffered);
				lLimitRow = get_TopRow();
				put_Redraw(flexRDNone);
			}
			else
			{
				lLimitRow = get_TopRow();
			}
		} while (Row_DataToGrid(lCurIndex) >= lLimitRow);
	}
}
void CDataGridCtrl::SetTblAlternateClr()
{
	_variant_t var;
	var.vt = VT_I4;

	// ���ڶ����͹������еļ��ɫ�ɸ���FixedCol�������ż�Խ�������
	if (GetTblType() == TblOrder || GetTblType() == TblWork)
	{
		for (map<long, long>::iterator iter = m_FlexDataSource.GetFixedColNumBeg();
			iter != m_FlexDataSource.GetFixedColNumEnd(); iter++)
		{
			if (iter->second % 2 == 0)
				var.lVal = GRID_COLOR_LINE1;
			else
				var.lVal = GRID_COLOR_LINE2;

			put_Cell(flexcpBackColor, COleVariant(Row_DataToGrid(iter->first)), COleVariant((long)1), 
				COleVariant(Row_DataToGrid(iter->first)), COleVariant(get_Cols()-1), var);
		}
	}
	else
	{
		const long dataRows = get_Rows()-1;
		const long topRow = get_TopRow();
		long bottomRow = get_BottomRow();
		if (bottomRow >= dataRows)
		{
			if (m_FlexDataSource.CanNew())
				bottomRow--;
		}

		if (dataRows > 0)
		{
			for (long row = topRow; row <= bottomRow; row++)
			{
				if (row % 2 == 0)
					var.lVal = GRID_COLOR_LINE1;
				else
					var.lVal = GRID_COLOR_LINE2;

				put_Cell(flexcpBackColor, COleVariant(row), COleVariant((long)1), COleVariant(row), COleVariant(get_Cols()-1), var);
			}
		}
	}

	const long lDataLastRow = Row_DataToGrid(m_FlexDataSource.GetGridRowListSize()-1);
	const long lGridLastRow = get_Rows()-1;
	if (lDataLastRow < lGridLastRow)
	{
		var.lVal = COLOR_Data_Gid_Bkg;
		put_Cell(flexcpBackColor, COleVariant(lDataLastRow+1), COleVariant((long)1), COleVariant(lGridLastRow), COleVariant(get_Cols()-1), var);
	}

//	var.Detach();
}
void CDataGridCtrl::SetTblLoadFileInfoClr()
{
	_variant_t var;
	var.vt = VT_I4;

	const long topRow = get_TopRow();
	const long bottomRow = get_BottomRow();
	for (long row = topRow; row <= bottomRow; row++)
	{
		if (row > 0 && row <= 8)
			var.lVal = COLOR_Data_Gid_LoadFileInfo1;
		else if (row > 8 && row <= 22)
			var.lVal = COLOR_Data_Gid_LoadFileInfo2;
		else
			var.lVal = COLOR_Data_Gid_LoadFileInfo3;

		put_Cell(flexcpBackColor, COleVariant(row), COleVariant((long)1), COleVariant(row), COleVariant((long)1), var);
	}
}
void CDataGridCtrl::SetTblMessageClr()
{
	const long ind = m_FlexDataSource.EnumToIndex(CMessageData::MessageData_level);
	if(ind >= 0 && get_Rows() > 1) 
	{
		const long topRow = get_TopRow();
		const long bottomRow = get_BottomRow();
		for (long row = topRow; row <= bottomRow; row++)
		{
			//UINT tbl_= TblMessageData;
			YK_ULONG itemId = YK_ULONG(m_FlexDataSource.GetGridRowId(Row_GridToData(row)));
			//HICON hIconFolder;
			int i_= 0;
			//
			YKMessageDataMap* msgMap=BIZAPI::GetBizMap()->GetYKMessageDataMap();
			if (NULL != msgMap)
			{
				CMessageDataPtr& ptr_=msgMap->Get(itemId);
				if (NULL != ptr_)
				{
					i_=ptr_->GetLevel();
				}
			}
			short indexp=-1;
			switch(i_)
			{
			case 0:
				{
					indexp=0;
					break;
				}
			case 1:
				{
					indexp=1;
					break;
				}
			case 2:
				{
					indexp=2;
					break;
				}
			case 3:
				{
					indexp=-1;
					break;
				}
			}
			if (indexp>=0)
			{
				put_Cell(flexcpPictureAlignment, COleVariant(row), COleVariant(ind+1),
					COleVariant(row), COleVariant(ind+1), COleVariant((short)flexPicAlignLeftCenter));

				/*static COleVariant vPicFolder;
				if (vPicFolder.vt == VT_EMPTY)
				{
					hIconFolder = m_FileViewImages.ExtractIcon(indexp);
					CPictureHolder picFolder;
					picFolder.CreateFromIcon(hIconFolder, TRUE);
					V_VT(&vPicFolder) = VT_DISPATCH;
					V_DISPATCH(&vPicFolder) = picFolder.GetPictureDispatch();
					DestroyIcon(hIconFolder);
				}*/
				put_Cell(flexcpPicture,COleVariant(row), COleVariant((long)1), COleVariant(row), COleVariant((long)1), m_vecIcons[indexp]);
			}
		}
	}
}
void CDataGridCtrl::SetCanNotEditCols()
{
	const long lDataRowSize = m_FlexDataSource.GetGridRowListSize();
	if(lDataRowSize <= 0)
		return;

	long topRow = get_TopRow();
	long bottomRow = get_BottomRow();
	if (bottomRow > lDataRowSize)
		bottomRow = lDataRowSize;

	list<long>	canNotEditCols;
	m_FlexDataSource.GetCanNotEditCols(canNotEditCols);
	_variant_t var2; 
	var2.vt = VT_I4;
	var2.lVal = GRID_COLOR_CANNOTEDIT;
	for (list<long>::iterator iter = canNotEditCols.begin();
		iter!=canNotEditCols.end();iter++)
	{
		put_Cell(flexcpBackColor, COleVariant(topRow), COleVariant(*iter),
			COleVariant(bottomRow), COleVariant((long)(*iter)), var2);
	}
//	var2.Detach();
}

void CDataGridCtrl::FillErrorColor()
{
	for(map<long,map<long,CString>>::iterator finder1 = m_FlexDataSource.m_errorInfo.begin();
		finder1!=m_FlexDataSource.m_errorInfo.end();finder1++)
	{
		for(map<long,CString>::iterator finder2=finder1->second.begin();
			finder2!=finder1->second.end();finder2++)
		{
			long col = m_FlexDataSource.EnumToIndex(finder2->first);
			if(col <0) continue;
			_variant_t var2; 
			var2.vt = VT_I4;
			var2.lVal = GRID_COLOR_EDITERROR;
			put_Cell(flexcpBackColor, COleVariant((long)finder1->first), COleVariant(col+1),
				COleVariant((long)(finder1->first)), COleVariant((long)(col+1)), var2);
			//var2.Detach();
		}
	}
}
void CDataGridCtrl::SetBackColors()
{
	const YK_UINT uTblType = GetTblType();
#ifdef PMCMODE
	if (TblLoadFileInfo == uTblType)
		return;
#endif

	if (uTblType == TblLoadFileInfo)
	{
		SetTblLoadFileInfoClr();
	}
	else if (uTblType == TblMessageData)
	{
		SetTblMessageClr();
	}
	else
	{
		const long tRow = get_TopRow();//�õ��������
		const long bRow = get_BottomRow();//�õ���Ͷ�����
		const long lCol = get_LeftCol();
		const long rCol = get_RightCol();

		_variant_t var;
		var.vt = VT_I4;
		var.lVal = 0;
		long col = 0;
		ULONG uBkClr = 0;
		for (long row = tRow; row <= bRow; row++)
		{
			uBkClr = m_FlexDataSource.GetBackColor(Row_GridToData(row), col);
			if (uBkClr > 0 && col > 0)
			{
				var.lVal = uBkClr;
				put_Cell(flexcpBackColor, COleVariant(row), COleVariant(col),
					COleVariant(row), COleVariant(col), var);

				if( HLS_L(RGB2HLS (uBkClr)) < GRID_COLOR_TEXT_REVERSE)
				{
					var.lVal = RGB(255, 255, 255);
					put_Cell(flexcpForeColor, COleVariant(row), COleVariant(col),
						COleVariant(row), COleVariant(col), var);
				}
			}
		}
	}
}

void CDataGridCtrl::SetBackState()
{
	if (m_lCurTopRow > 0 && m_lCurTopRow < get_Rows())
	{
		put_TopRow(m_lCurTopRow);
	}
	if (m_lCurLeftCol > 0 && m_lCurLeftCol < get_Cols())
	{
		put_LeftCol(m_lCurLeftCol);
	}
}
void CDataGridCtrl::AdaptView(long col/* = -1*/)
{
	return;
	if(col<0)
		OptimizeTable();
	if(col > get_Cols()) col = -1;

	CRect rect;
	GetClientRect(&rect);

	long with=0;
	long clientwith=rect.Width()*15-40;

	int lastCol = 0;
	int lastWith = 0;
	stTable &stTbl = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());

	long bigColWith = 0;
	map<long, long>	bigColInfo;
	const int len = stTbl.mode.size();
	for (int i = 0; i < len; i++)
	{
		stVector& stV = stTbl.mode[i];
		if (!stV.IsHide())
		{
			if(i >= col)
			{
				bigColWith += stV.width;
				bigColInfo[i+1] = stV.width;
			}
			with += stV.width;
			lastCol = i+1;
			lastWith = stV.width;
		}
	}
	if(col>0 && col == lastCol)		// ���һ��
	{
		AdaptView(-1);
		return;
	}
	with += get_ColWidth(0);			// ��һ��Ϊ�̶��У�����У�
	if (with < clientwith-1)
	{
		if(col >= 0)		// 
		{
			long canuseW = bigColWith + clientwith - with;
			for (map<long,long>::iterator itor = bigColInfo.begin();		//����������
				itor != bigColInfo.end(); itor++)
			{
				put_ColWidth(itor->first, long(canuseW*(itor->second/(bigColWith*1.0))));
			}
		}
		else{
			lastWith = clientwith - with + lastWith;
			if(lastWith > 10) lastWith -= 2;
			put_ColWidth(lastCol, lastWith);
		}
	}
	else if (with > clientwith+1)
	{
		if(col>=0)		// 
		{
			long notMoveW = with - bigColWith;
			long canuseW = clientwith-notMoveW;
			if (canuseW<=0)
			{

			}
			else
			{
				for (map<long,long>::iterator itor = bigColInfo.begin();		//����������
					itor!=bigColInfo.end();itor++)
				{
					put_ColWidth(itor->first,long(canuseW*(itor->second/(bigColWith*1.0))));
				}
			}
		}
		else		// ȫ������
		{
			long bigW = with - clientwith;
			while(bigW >= 5)
			{
				long big = 0;
				long bigCol = 0;
				for (size_t i =0;i<stTbl.mode.size();i++)
				{
					stVector& stV = stTbl.mode[i];
					if (!stV.IsHide())
					{
						long w = get_ColWidth(i+1);
						if(w > big)
						{
							big = w;
							bigCol = i+1;
						}
					}
				}
				if(big<=0) break;
				if(bigW > big)		//  �����ȴ�������п�ʱ  ����п��ȥ�����1/5 �ٱȽ�
				{
					long delw = big/5;
					bigW -= delw;
					big -= delw;
					put_ColWidth(bigCol,big);
				}
				else				//  ������С��������п�ʱ  ����п��ȥ�����п��1/5 �ٱȽ�
				{
					long delw = bigW/5;
					bigW -= delw;
					big -= delw;
					put_ColWidth(bigCol,big);
				}
			}
		}

	}

	for (int i = 1; i < get_Cols(); i++)
	{	//�޸Ĵ�ʱ���ȫ���е�width
		stTbl.mode[i-1].width = get_ColWidth(i);
		GetStTable().mode[i-1].width = get_ColWidth(i);
	}
	//SaveData();//��������
}

void CDataGridCtrl::KeyDownVsflexgrid( short* KeyCode, short Shift )
{
	// �ж��Ƿ�ɶԱ����и��ơ�ճ�������С�ɾ������
	if (2 == Shift) // Ctrl��
	{
		switch (*KeyCode)
		{
		case 67:		// Ctrl+C����
			CTRL_C();
			break;
		case 86:		// Ctrl+Vճ��
			CTRL_V();
			break;
		case 88:		// Ctrl+X����
			CTRL_X();
			break;
		}
	}
	else if (0 == Shift)	// û�и��Ӽ�
	{
		if (*KeyCode == 46)	// delete�����������
			DeleteSelect();
	}
	else if (1 == Shift)	// Shift��
	{
		if ((*KeyCode == 46) && CanBeNew(GetTblType())) // shift+delete����ɾ��������
		{
			long tRow(0), lCol(0), bRow(0), rCol(0);
			GetSelection(&tRow, &lCol, &bRow, &rCol);
			DeleteRows(tRow, bRow);
		}
	}
}


bool CDataGridCtrl::GetTopLeftMenuShaleShow()
{
	bool bOK = false;

	CWnd* pWnd = GetParent();
	if (pWnd->IsKindOf(RUNTIME_CLASS(CBCGPDockingControlBar)))
	{
		COutputBar* bar = (COutputBar*)pWnd;
		if(NULL != bar)
			bOK = (!bar->m_singlShale);
	}
	return bOK;
}
void CDataGridCtrl::ReplaceOnePrivate( int Row, int Col, CString strReplace, CString strFind )
{
	CString strText;
	strText = get_TextMatrix(Row, Col);
	strText.Replace(strFind, strReplace);

	//Add 2011-11-21 ���OnItemChanged ����
	unsigned long tblName = m_FlexDataSource.m_table.tblType;
	unsigned long id = m_FlexDataSource.GetGridRowId(Row-1);
	if(Col <= m_FlexDataSource.m_table.mode.size())
	{
		unsigned long col = m_FlexDataSource.m_table.mode[Col-1].mode;

		wstring temp = strText.GetString();
		m_FlexDataSource.DataModify(tblName,id,col,temp, true);

		put_FlexDataSource((LPDISPATCH)&  m_FlexDataSource);
	}

}

void CDataGridCtrl::Edit(long mouseUpRow)
{
	//���к�
	UINT id = m_FlexDataSource.GetIdByIndex(Row_GridToData(mouseUpRow));//*m_FlexDataSource.m_pRowList)[row-1];

	if (id >0)
	{
		MSG ms;
		ms.message = GetTblType();
		ms.wParam = id;
		ms.lParam = -1;
		::SendMessage(theApp.g_hMainFrm,IDU_UPDATE_MODIFY_DLG,(int)&ms,0);
	}
}


// void CDataGridCtrl::EditRecord( long Row1, long Row2 )
// {
// 	list<long> selectRows;
// 	for(long i = Row1; i <= Row2;)
// 	{
// 		selectRows.push_back(i);
// 		i++;
// 		if(i >= get_Rows())//�ж��Ƿ񳬹�����
// 			break;//�˳�����ѭ��
// 	}
// 
// 	EditRecord(selectRows);
// }
// 
// void CDataGridCtrl::EditRecord(const list<long>& rowList )
// {
// 	list<ULONG> ids;
// 	m_FlexDataSource.InsertOpertion(1, rowList, ids);
// }

void CDataGridCtrl::RealNew(unsigned long newId,int type)
{

	if (newId > 0)
	{
		if(type == 1)
		{
			m_FlexDataSource.RealNew(newId);
			if (GetTblType() == TblItem)
			{
				BIZAPI::SetTableFlashFlg(TRUE, Item_Change);
			}

			if (GetTblType() == TblOrder)
				BIZAPI::SetTableFlashFlg(TRUE, TblOrder);

			Flash(TRUE);
		}
		else	//ȡ���½�
		{
			m_FlexDataSource.DeleteOpt(GetTblType(), newId);
		}

	}
}
// BOOL CDataGridCtrl::NewRecord(long row, long copyBegin, long copyEnd)
// {
// 	BOOL bOK = FALSE;
// 
// 	if (copyEnd > (get_Rows()-1))
// 		copyEnd = (get_Rows()-1);
// 	
// 	vector<YK_ULONG> copyRows;
// 	copyRows.reserve(copyEnd-copyBegin+1);
// 	for(long i = copyBegin; i <= copyEnd; ++i)
// 	{
// 		copyRows.push_back(i);
// 	}
// 	bOK = m_FlexDataSource.NewRecord(row, copyRows);
// 
// 	return bOK;
// }

BOOL CDataGridCtrl::NewRecord(long row)
{
	BOOL bOK = FALSE;

	if (!m_vecCopyed.empty())
	{
		bOK = m_FlexDataSource.NewRecord(row, m_vecCopyed);
	}
	/*if (!m_listCopyed.empty())
		bOK = m_FlexDataSource.NewRecord(row, m_listCopyed);*/

	return bOK;
}
void CDataGridCtrl::OnItemChanged( int nRow,int nColumn,BSTR newVal,bool b /*= true*/ )
{
	list<CellMsg> Rowlist;
	CellMsg cellmsg;
	cellmsg.row = nColumn;
	cellmsg.bstr = newVal;
	Rowlist.push_back(cellmsg);
	OnItemChanged(nRow,Rowlist,b);
	SetBackColors();
}

void CDataGridCtrl::OnItemChanged( long nRow, const list<CellMsg>& Rowlist,bool b /*= true*/ )
{
	//Add 2011-11-3
	if (Rowlist.size() > 0)
	{
		//BIZAPI::BeginRebackOper(OPERTYPEMODIFY);

		// work avb res
		if (GetTblType() == TblWorkAvbResource)
		{
			YKAvbResTempPtr& avbTempPtr = BIZAPI::GetBizMap()->GetYKAvbResTempMap()->Get(m_FlexDataSource.GetGridRowId(Row_GridToData(nRow)));
			if(avbTempPtr != NULL)
			{
				list<YKWorkAvbResourcePtr>& workAvbList = avbTempPtr->GetAvbResPtrList();
				for (list<YKWorkAvbResourcePtr>::iterator itor = workAvbList.begin();
					itor != workAvbList.end();itor++)
				{
					// ���볷����Ϣ
					RecordInfo reinfo;
					reinfo.id = (*itor)->GetId();
					reinfo.operType = OPERTYPEMODIFY;
					BIZAPI::InsertRelDataStruct(GetTblType(), reinfo);
				}
			}
		}
		else
		{
			// ���볷����Ϣ
			RecordInfo reinfo;
			reinfo.id = m_FlexDataSource.GetGridRowId(Row_GridToData(nRow));
			reinfo.operType = OPERTYPEMODIFY;
			BIZAPI::InsertRelDataStruct(GetTblType(), reinfo);
		}

		for (list<CellMsg>::const_iterator itor = Rowlist.begin();itor!=Rowlist.end();itor++)
		{
			m_FlexDataSource.OnItemChanged(nRow-1,itor->row,itor->bstr,b);
		}
		//BIZAPI::EndRebackOper();
	}
}

void CDataGridCtrl::AnalyseData(int &iTemp, CString &strLeft, CString &strRight
								, CString strTemp, variant_t var)
{
	BSTR bstrTemp;
	bstrTemp = strTemp.AllocSysString();
	CString str;
	int length;
	_bstr_t bstr;
	switch (var.vt)
	{//����
	case VT_INT:
	case VT_I8:
	case VT_I4:
	case VT_I2:
	case VT_UINT:
	case VT_UI8:
	case VT_UI4:
	case VT_UI2:
	case VT_UI1:
		bstrTemp = strTemp.AllocSysString();
		iTemp = _wtoi(bstrTemp);
		strLeft = _T("");
		strRight = _T("");
		break;
		//������
	case VT_R4:
	case VT_R8:
		length = strTemp.GetLength();
		str = strTemp.Left(strTemp.Find(_T(".")));
		bstrTemp = str.AllocSysString();
		iTemp = _wtoi(bstrTemp);
		strLeft = _T("");
		strRight = strTemp.Right(length - strTemp.Find(_T(".")) - 1);
		break;
	case VT_BSTR:
		AnalyseCString(strTemp, strLeft, strRight, iTemp);
		break;
	default:
		break;
	}
	::SysFreeString(bstrTemp);
}

void CDataGridCtrl::AnalyseCString(CString strText, CString &strLeft, CString &strRight, int &iTemp)
{
	int length = strText.GetLength();
	CString strTemp = _T("");
	CString strFirstString = strText.Left(1);
	bool bAllNumber = true;
	if (strFirstString != _T("-"))//�ж��ַ�����һ���ַ��Ƿ�Ϊ��-�����ţ�������
	{
		int countDot = 0;//��.���ĸ���
		for (int i = length - 1; i >= 0; i--)
		{
			if (i > 0 && i < length - 1 && strText[i] == L'.')
			{
				countDot++;
				if (countDot > 1)//��.�� 1��ʱ���ַ����Ͳ���һ������ֵ�ˡ�
				{
					bAllNumber = false;
					break;
				}
				continue;
			}
			bAllNumber = AnalyseOneCString(strText[i]);
			if (!bAllNumber)//bAllNumber Ϊfalseʱ���أ�˵����ȫ�����ֵ��ַ���
			{
				break;
			}
		}
		if (bAllNumber)//ȫ�����ֵ��ַ���
		{
			int dot = strText.Find(_T("."));
			if (dot == -1)//˵��ȫ�����ֵ��ַ�����û�С�."
			{
				iTemp = _wtoi(strText.GetString());
				strLeft = _T("");
				strRight = _T("");
			}
			else//�ַ�����һ��������
			{
				int iDotPos = strText.Find(_T("."));
				CString strNumber = strText.Left(iDotPos);
				iTemp = _wtoi(strNumber.GetString());
				strRight = strText.Right(length - iDotPos);
				strLeft = _T("");
			}
		}
		else//��ȫ�����ֵ��ַ�������
		{
			bool bIsAllNotNumber = true;
			wstring wstrTemp = L"";
			wstring wstrRight = L"";
			wstring wstrLeft = L"";
			int iNumberCount = 0;
			bool bNext = false;
			bool bMidNumber = true;
			for (int i = length - 1; i >= 0; i--)//�Ӻ���ǰ�������ж��ַ��Ƿ�Ϊ�����ַ�
			{
				if (AnalyseOneCString(strText[i]))
				{
					if (bMidNumber)
					{
						iNumberCount = 2;//���ڶ��ε��ַ������������ַ���
					}
					else
					{
						iNumberCount = 3;//�������ε��ַ���
					}
					bNext = true;//����������ַ���
				}
				else
				{
					iNumberCount = 1;//����һ�ε��ַ���
					if (bNext)
					{
						iNumberCount = 3;//�������ε��ַ���
						bMidNumber = false;
					}
				}
				switch (iNumberCount)
				{
				case 1:
					wstrRight += strText[i];//�ұ��ַ���
					break;
				case 2:
					wstrTemp += strText[i];//�м������ַ���
					break;
				case 3:
					wstrLeft += strText[i];//����ַ���
					break;
				default:
					break;
				}
			}
			if (wstrTemp != L"")
			{
				iTemp = _wtoi(wstrTemp.c_str());//ת������
			}

			if (wstrLeft != L"")
			{
				strLeft = wstrLeft.c_str();//����ַ���
				strLeft = strLeft.MakeReverse();
			}
			else
			{
				strLeft = _T("");
			}

			if (wstrRight != L"")
			{
				strRight = wstrRight.c_str();//�ұ��ַ���
				strRight = strRight.MakeReverse();
			}
			else
			{
				strRight = _T("");
			}
		}
	}
	else
	{
		int countDot = 0;//��.���ĸ���
		for (int i = length - 1; i > 0; i--)
		{
			if (i > 0 && i < length - 1 && strText[i] == L'.')
			{
				countDot++;
				if (countDot > 1)//��.�� 1��ʱ���ַ����Ͳ���һ������ֵ�ˡ�
				{
					bAllNumber = false;
					break;
				}
				continue;
			}
			bAllNumber = AnalyseOneCString(strText[i]);
			if (!bAllNumber)//bAllNumber Ϊfalseʱ���أ�˵����ȫ�����ֵ��ַ���
			{
				break;
			}
		}
		if (bAllNumber)//ȫ�����ֵ��ַ���
		{
			int dot = strText.Find(_T("."));
			if (dot == -1)//˵��ȫ�����ֵ��ַ�����û�С�."
			{
				iTemp = _wtoi(strText.GetString());
				strLeft = _T("");
				strRight = _T("");
			}
			else//�ַ�����һ��������
			{
				int iDotPos = strText.Find(_T("."));
				CString strNumber = strText.Left(iDotPos);
				iTemp = _wtoi(strNumber.GetString());
				strRight = strText.Right(length - iDotPos);
				strLeft = _T("");
			}
		}
	}
}
//����һ���ַ���
bool CDataGridCtrl::AnalyseOneCString(wchar_t strOne)
{
	bool bReturn = false;
	switch (strOne)
	{
	case L'0':
	case L'1':
	case L'2':
	case L'3':
	case L'4':
	case L'5':
	case L'6':
	case L'7':
	case L'8':
	case L'9':
		bReturn = true;
		break;
	default:
		bReturn = false;
		break;
	}
	return bReturn;
}

//��"+"���ͱ��ĵ�һ��˫��������
void CDataGridCtrl::CreatEditBoxForOutlineGrid(long row, long col)
{
	//ǰ������Ч�����кŶ�Ӧ������Ч
	const long lDataRow = Row_GridToData(row);
	const long lRowId = m_FlexDataSource.GetGridRowId(lDataRow);
	if (col < 2 && lRowId > -1)
	{
		MSG ms;
		const UINT uTblType = m_FlexDataSource.GetGridRowTblType(lDataRow);
		if (TblUseResource == uTblType)
		{
			GridRowInfo uperGridInfo;
			if (m_FlexDataSource.GetUpperRowInfo(lDataRow, uperGridInfo))
			{
				if (TblWork == uperGridInfo.tblType)
				{
					ms.message = uTblType;
					ms.wParam = BIZAPI::CreateUseResource(uperGridInfo.id, lRowId);
					ms.lParam = -1;
					::SendMessage(theApp.g_hMainFrm, IDU_UPDATE_MODIFY_DLG, (int)&ms, 0);
					return;
				}
			}
		}
		ms.message = uTblType;
		ms.wParam = lRowId;
		ms.lParam = -1;
		::SendMessage(theApp.g_hMainFrm, IDU_UPDATE_MODIFY_DLG, (int)&ms, 0);
	}
}

// ���Ʋ���
void CDataGridCtrl::CTRL_C()
{
	if (IsCanCopy())
	{
		// ��ռ��е����ݻ���
		m_rtCopy.SetRectEmpty();
		ClearCopyRowBasePtr();

		// ��ռ��а�
		if (OpenClipboard())
		{
			EmptyClipboard();
			CloseClipboard();
		}

		m_bClipBoardEmpty = TRUE;

		GetSelection(&m_rtCopy.top, &m_rtCopy.left, &m_rtCopy.bottom, &m_rtCopy.right);
		if (m_bCanNewData)
		{
			GetCopyRowBasePtr(m_rtCopy.top, m_rtCopy.bottom);
		}
		else
		{
// 			Copy();
			// create a copy of the data
			CString sClip = get_Clip();
			if(m_rtCopy.left==m_rtCopy.right && get_TextMatrix(m_rtCopy.bottom, m_rtCopy.right)==CString(""))
				sClip += L"\r\n";
			HGLOBAL hGlobal = GlobalAlloc(GMEM_SHARE, sClip.GetLength()*2+2);
			if (!hGlobal) return;
			void* pData = GlobalLock(hGlobal);
			memcpy(pData, sClip.GetBuffer(0), sClip.GetLength()*2+2);
			GlobalUnlock(hGlobal);

			// copy data to clipboard
			OpenClipboard();
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, hGlobal);
			CloseClipboard();
			m_bClipBoardEmpty = FALSE;
		}


		m_bCanPaste = TRUE;
	}
}

// ճ������
void CDataGridCtrl::CTRL_V()
{
	if (IsCanPaste())	// ����ճ��
	{
		m_FlexDataSource.SetDataEditType(2);

		BOOL bOK = FALSE;
		const long row = get_Row();
		const long col = get_Col();
		m_FlexDataSource.SetEidtModel(false);	// ����Ϊctrl + v �༭ģʽ

		if (FALSE == m_bCanNewData) // ��Ϊ�½�����
		{
			//Add 2012-01-11 ���ơ�����/�������á��������ճ������
			const long tblType = GetTblType();		//��ID
			if (  TblLoadFileInfo != tblType )
			{
				if (IsRightSelection())
				{
					m_FlexDataSource.EditRecord(row, col, row+m_rtCopy.bottom-m_rtCopy.top, col+m_rtCopy.right-m_rtCopy.left);
					Paste();
				}
			}
		}
		else
		{
			bOK = NewRecord(row);

			if (bOK) // �½��ɹ���ˢ�½���
			{
				// ��ճ���������зŵ����㡣
				const long newTRow = get_Rows()-1;
				Flash();
				put_TopRow(newTRow);

				// ѡ�е����ݱ�ĩβ������һ��
				const long newBRow = Row_DataToGrid(m_FlexDataSource.GetGridRowListSize()-1);
				Select(newBRow, 0, COleVariant(newBRow), COleVariant((long)0));

				if (GetTblType() == TblResource)
					::PostMessage(theApp.g_hMainFrm, ID_INIT_RESOURCE, 0, 0);
			}
		}
		Invalidate();
		UpdateWindow();
	}
}

// ���в���
void CDataGridCtrl::CTRL_X()
{

	if (IsCanCut())
	{
		CTRL_C();

		if (m_bCanNewData) // �ж�ѡ�е��Ƿ��ǵ�һ��
		{
			long tRow(0), lCol(0), bRow(0), rCol(0);
			GetSelection(&tRow, &lCol, &bRow, &rCol);
			DeleteRows(tRow, bRow);
		}
		else
		{
			DeleteSelect();
		}
	}
}

// ���������������б��浽������
void CDataGridCtrl::GetCopyRowBasePtr(long begRow, long endRow)
{
	YK_ULONG id = 0;
	m_vecCopyed.reserve(endRow-begRow+1);
	for (long row = begRow; row <= endRow; row++)
	{
		id = m_FlexDataSource.GetIdByIndex(Row_GridToData(row));
		if (id > 0)
			//m_listCopyed.push_back(BIZAPI::Copy(GetTblType(), id));
			m_vecCopyed.push_back(BIZAPI::Copy(GetTblType(), id));
	}
}
// ��ռ��������еĻ���
void CDataGridCtrl::ClearCopyRowBasePtr()
{
	YKMapBase* pMap = BIZAPI::GetBizMap()->GetTblPoint(GetTblType());

	if (pMap)
	{
		for (vector<YK_ULONG>::iterator iter = m_vecCopyed.begin();
			iter != m_vecCopyed.end(); ++iter)
		{
			pMap->Free(*iter);
		}
	}
	m_vecCopyed.clear();
// 	YKBase* ptr = NULL;
// 	for (list<YKBase*>::iterator iter = m_listCopyed.begin();
// 		iter != m_listCopyed.end(); iter++)
// 	{
// 		ptr = *iter;
// 		if (ptr != NULL)
// 		{
// 			ptr->FreeTemp();
// 			delete ptr;
// 		}
// 	}
// 	m_listCopyed.clear();
}


// ɾ��������
void CDataGridCtrl::DeleteRows(long begRow, long endRow)
{
#ifdef LIMITFUNCTION
	if(BIZAPI::GetIsLimited())
		return;
#endif

	vector<long> listDelRows;
	listDelRows.push_back(Row_GridToData(begRow));
	listDelRows.push_back(Row_GridToData(endRow));
	vector<vector<long> > vecRows;
	vecRows.push_back(listDelRows);
	if (false == m_FlexDataSource.DeleteRows(vecRows)) return;
	/*listDelRows.reserve(endRow-begRow+1);
	for (long row = begRow; row <= endRow; ++row)
	{
		listDelRows.push_back(row);
	}
	m_FlexDataSource.Del(listDelRows);*/
#ifdef LIMITFUNCTION
	BIZAPI::LimitCheck();
#endif
	switch (GetTblType())
	{
	case TblItem:
		BIZAPI::SetTableFlashFlg(TRUE, Item_Change);
		break;
	case TblResource:
		::PostMessage(theApp.g_hMainFrm, ID_INIT_RESOURCE, 0, 0);
		break;
	}

	// ɾ���������г���һҳ����ԭ����Ķ����кż�¼��Ч
	if (listDelRows.size() > (size_t)GetClientRowCount())
		m_FlexDataSource.ReSetFixedNumRecord();

	// ������ԭɾ�����С�����һ�е�ѡ��, ˢ�½���
	Select(begRow, 0, COleVariant(begRow), COleVariant((long)0));
	Flash();
}

// �����ѡ��������
void CDataGridCtrl::DeleteSelect()
{
#ifdef LIMITFUNCTION
	if(BIZAPI::GetIsLimited())
		return;
#endif

	long rowSel1, colSel1, rowSel2, colSel2;
	//ɾ��ѡ������
	GetSelection(&rowSel1, &colSel1, &rowSel2, &colSel2);

	// ���ٵ����ͷ Ȼ��DEL�����ܻ����
	if(rowSel1 <= 0 || rowSel2 <= 0)
		return;

	if(!BIZAPI::CheckLimit(GetTblType(), m_FlexDataSource.GetIdByIndex(Row_GridToData(rowSel1))))
		return;

	m_FlexDataSource.SetDataEditType(2);
	//////////////////////////////////////////////////////////////////////////
	//��¼ɾ������
	//EditRecord(rowSel1, rowSel2);
	m_FlexDataSource.EditRecord(rowSel1, colSel1, rowSel2, colSel2);

	//////////////////////////////////////////////////////////////////////////
	for( int i = rowSel1; i <= rowSel2 ; i++)
	{
		for(int j = colSel1; j <= colSel2 ; j++ )
		{
			if (j < get_Cols())//�ж��Ƿ񳬹�����
			{
				//���뵼��������ɾ��
				if(m_FlexDataSource.GetTblType() == TblLoadFileInfo
					&&(m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_DateType||	// ���ڸ�ʽ
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_UpdateDate||	// ��������
//					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_InsertCondition||	// ����ɸѡ����
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_InputType||		// ���뷽ʽ
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_InputEnterCount||	// ����������Ŀ
//					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_OutputFilterContion||	// ����ɸѡ����
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_OutputType||	// ������ʽ
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_IsoutputHeader||	// �Ƿ񵼳���ͷ
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_ClientTblType||
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_ClientTblName||
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_ClientField||
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_FieldEnum||
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_Id||
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_InsideTblType||
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_OutputFileFormat||
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_FileType||
//					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_Connection||
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_MainKey||		// ����
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_OutterChangeFlg ||
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_InnerChangeFlg ||
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_ServerId||
					m_FlexDataSource.GetEnumIdByIndex(j-1) == YKLoadFileInfo::YKLoadFileInfo_EditionNum					
					))
					continue;
				put_TextMatrix(i, j, _T("")); //�޸�ָ�����������		
			}
			else
			{
				break;//�˳�����ѭ��
			}
		}
		if( i > get_Rows() )//�ж��Ƿ񳬹�����
			break;//�˳�����ѭ��
	}
	Invalidate();
	UpdateWindow();
}


void CDataGridCtrl::ClearData()
{
	if(m_FlexDataSource.GetGridRowListSize() <= 0) 
		return;

	DeleteRows(1, get_Rows()-1);
	/*vector<long> delRows;
	for(int i = 1; i < get_Rows(); i++)
	{
		delRows.push_back(i);
	}
	m_FlexDataSource.Del(delRows);
#ifdef LIMITFUNCTION
	BIZAPI::LimitCheck();
#endif
	switch (GetTblType())
	{
	case TblItem:
		BIZAPI::SetTableFlashFlg(TRUE, Item_Change);
		break;
	case TblResource:
		::PostMessage(theApp.g_hMainFrm, ID_INIT_RESOURCE, 0, 0);
		break;
	}

	Flash(TRUE);*/
}

BOOL CDataGridCtrl::IsCanCopy()
{
	BOOL bOK = FALSE;

	if(BIZAPI::CheckLimit(GetTblType(), m_FlexDataSource.GetIdByIndex(Row_GridToData(get_Row()))))
	{
		long tRow(0), lCol(0), bRow(0), rCol(0);
		const long lastDataRow = Row_DataToGrid(m_FlexDataSource.GetGridRowListSize()-1);
		GetSelection(&tRow, &lCol, &bRow, &rCol);
		if ((tRow > 0 && lCol > 0) && tRow <= lastDataRow)
		{
// 			// �Ȱ���������ѡ���ƵĹ��ܽ���
// 			if (!m_bCanNewData && (tRow != bRow || lCol != rCol))
// 				bOK = FALSE;
// 			else
				bOK = TRUE;
		}
	}

	return bOK;
}

BOOL CDataGridCtrl::IsCanCut()
{
	BOOL bOK = FALSE;

	
	bOK = (IsCanCopy() && GetEditContentFlg());

	return bOK;
}

BOOL CDataGridCtrl::IsCanPaste()
{
	BOOL bOK = FALSE;

	// ���ǰ���Ƿ�����ճ��
	if(BIZAPI::CheckLimit(GetTblType(), m_FlexDataSource.GetIdByIndex(Row_GridToData(get_Row()))))
	{
		if ((get_Col() == 0) || GetEditContentFlg())
			bOK = m_bCanPaste || !m_bClipBoardEmpty;
	}

	return bOK;
}

BOOL CDataGridCtrl::GetEditContentFlg()
{
	BOOL bOK = FALSE;
	if(!BIZAPI::CheckLimit(GetTblType(), m_FlexDataSource.GetIdByIndex(Row_GridToData(get_Row()))))
		return bOK;

	// ���Ҹ�����ѡ����ı���ɫ���ж��Ƿ���Ա༭
	long tRow(0), lCol(0), bRow(0), rCol(0);
	GetSelection(&tRow, &lCol, &bRow, &rCol);
	const long lastDataRow = Row_DataToGrid(m_FlexDataSource.GetGridRowListSize()-1);
	if (tRow <= lastDataRow)
	{
		wstring wstrText;
		for (long col = lCol; col <= rCol; col++)
		{
			YKField& filedPara = GetEditType(Row_GridToData(tRow), col, wstrText);
			bOK = ((FILEDPARA_ISMODIFY_TRUE == filedPara.m_modifyFly)/* && !(filedPara.HaveVirRule())*/);

			if (bOK)
				break;
		}
	}

	return bOK;
}

bool CDataGridCtrl::IsCanUnAllFixCol()
{
	bool bOK = FALSE;

	const long fixCols = get_FrozenCols();
	if (TblOrder == GetTblType() || TblWork == GetTblType())
	{
		if (fixCols > 1)
			bOK = TRUE;
	}
	else
	{
		if (fixCols > 0)
			bOK = TRUE;
	}

	return bOK;
}
bool CDataGridCtrl::IsWillOrder( long mouseUpRow )
{
	bool bOK = false;

	if(GetTblType() == TblOrder)
	{
		bOK = m_FlexDataSource.IsWillOrder(Row_GridToData(mouseUpRow));
	}

	return bOK;
}
bool CDataGridCtrl::IsItemTabl()
{
	bool bOK = false;

	if(GetTblType() == TblItem)
		bOK = true;

	return bOK;
}

bool CDataGridCtrl::CanBeNew(UINT tableType)
{
	bool bOK = true;

	if( tableType == TblWork||tableType == TblWorkInputItem|| tableType == TblWorkOutputItem
		||tableType == TblWorkAvbResource|| tableType == TblWorkRelation
		/*||tableType == TblGanttPara*/||tableType ==TblMpsParam
		||tableType == TblMrpParam|| tableType == TblRestrictParam||tableType == TblSchSysParam
		||tableType == TblOrdPriCalcParam|| tableType ==TblOptLogicParam
		/*||tableType == TblCreateMTOrderParam*/ ||tableType ==TblOptObjParam
		||tableType == TblRestrictInfo
		||tableType == TblReducePeriod
		||tableType == TblUseResource
		||tableType == TblProduceIndication
		||tableType == TblCategorySequence
		||tableType == TblResourceSequence
		||tableType == TblLoadFileInfo
		||tableType == TblWorkChange
		||tableType == TblMessageData
		||tableType == TblManufactureDemand)
	{
		bOK = false;
	}

	return bOK;
}

void CDataGridCtrl::Print()
{
	PrintGrid( COleVariant(L""),            // no doc name (we'll handle the page footer)
		COleVariant(1L),            // show print dialog
		COleVariant(0L),            // orientation (portrait)
		COleVariant(1440L),         // left, right margins: 1 inch
		COleVariant(1440L));        // top, bottom margins: 1 inch);
}

//�½�����  ���½������ݷ���m_pRowListExtra��  ������������Դ��ˢ�����ݣ�  add  2010-7-9
void CDataGridCtrl::OnNewItemClass(CString newVal)
{
	wstring wstr = newVal.GetString();
	//BIZAPI::BeginRebackOper(OPERTYPENEW);
	//Add 2012-03-19
	UINT tblType = GetTblType();
	unsigned long newId = BIZAPI::NewItemClassByTbl(GetTblType(), wstr);
	if (newId > 0)
	{
		m_FlexDataSource.CreateLimitRecord(newId);
		//Add 2012-03-19 
		m_FlexDataSource.CreateNewObjByTblType(newId,m_tblTypes);

		GridRowInfo rowInfo;
		rowInfo.id = newId;
		rowInfo.tblType = GetTblType();
		rowInfo.rowLevel = 0;
		rowInfo.beCollapsed = flexOutlineCollapsed;
		rowInfo.bShow = TRUE;
		m_FlexDataSource.PushBGridRowList(rowInfo);

		//// ����reback ��Ϣ
		//RecordInfo recodeinfo;
		//recodeinfo.id = newId;
		//recodeinfo.operType = OPERTYPENEW;
		//BIZAPI::InsertRelDataStruct(m_FlexDataSource.GetTblType(), recodeinfo);

		BIZAPI::BeginUndoRedo();
		vector<YK_ULONG> listIds;
		listIds.push_back(newId);
		BIZAPI::InsertOpertion(2, GetTblType(), listIds);
		BIZAPI::EndUndoRedo();

		// ˢ�º�ָ�ԭ�ȵ�ѡ��
		long tRow(0), lCol(0), bRow(0), rCol(0);
		GetSelection(&tRow, &lCol, &bRow, &rCol);
		Flash();
		Select(tRow, lCol, COleVariant(bRow), COleVariant(rCol));
	}
	//BIZAPI::EndRebackOper();
}

void CDataGridCtrl::TrackRLeftUpMenu(float X, float Y)
{
	const POINT ptScreen = MouseToScreen(GetSafeHwnd(), X, Y);
	CGridMenu * pGridMenu = CGridMenu::GetCGridMenu();
	pGridMenu->m_bIsShowShale = GetTopLeftMenuShaleShow();
	int option = pGridMenu->CreatMenu(GetStTable(), LEFTTOP, ptScreen, this);
	if (option == ID_GRIDMENU_ALLDISPLAY)//��ʾȫ����
	{
		/*const long cols = get_Cols();
		for (int i = 0; i < get_Cols(); i++)
		{
			put_ColHidden(i, false);
		}*/
		put_ColHidden(-1, false);
		YKFieldPtrMap* pFieldMap =  BIZAPI::GetBizMap()->GetYKFieldPtrMap();
		//��ֵ���˱��ж�Ӧ��isHideȫ����ֵΪ1����Ϊ��ȫ����ʾ��
		stTable &tbl = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());
		const size_t len = GetStTable().mode.size();
		for (size_t j = 0; j < len; j++)
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

// 		if(GetTblType() == TblMessageData)
// 			AdaptView();
	}
	else if ( ID_GRIDMENU_DEFAULTDISPLAY == option)
	{
		//Ӧ�ñ����Զ����ֶ�
		stTable newSt = GetStTable();
		//��¼�Զ����ֶ�
		vector<stVector> customSt;
		for(vector<stVector>::iterator iter = newSt.mode.begin();
			iter != newSt.mode.end();iter++)
		{
			if(iter->mode > BIGGEST_FILED_ENUM)
				customSt.push_back(*iter);
		}
		//ȡĬ��״̬
		UINT tbl = GetTblType();
		UINT nameIndex  = GetStTable().GetNameIndex();
		stTable aTable = DATATABLAPI::GetResumeTbl(tbl, nameIndex);

		//�����Զ����ֶ�
		aTable.mode.insert(aTable.mode.end(),customSt.begin(),customSt.end());

		DATATABLAPI::SetstTble(tbl, nameIndex, aTable);
		SetStTable(aTable);
// 		if(GetTblType() == TblMessageData)
// 			AdaptView();

		Flash();
	}
	else if (option == ID_GRIDMENU_RESIZE)//�Ż��п�
	{
// 		if(GetTblType() == TblMessageData)
// 			AdaptView();
// 		else 
			OptimizeTable();//�Ż��п�
	}
	else if (ID_GRIDMENU_OUTPUTHTML == option)
	{
		OutputHtml();
	}
	else if (ID_GRIDMENU_SIGNELSHALE == option)		// ҳǩ��ʾ
	{
		::SendMessage(GetParent()->GetSafeHwnd(),ID_GRIDMENU_SIGNELSHALE,0,0);
	}
}

void CDataGridCtrl::TrackRFirstColMenu(float X, float Y)
{
	const long row = get_MouseRow();
	// �����½����е����һ�в�������Щ����
	if (CanBeNew(GetTblType()) && row == (get_Rows()-1))
		return;

	const POINT ptScreen = MouseToScreen(GetSafeHwnd(), X, Y);
	CGridMenu * pGridMenu = CGridMenu::GetCGridMenu();
	pGridMenu->m_bIsWillOrder = IsWillOrder(row);
	pGridMenu->m_bIsItemTbl = IsItemTabl();
	pGridMenu->m_bIsCreateMenuTurn = IsWorkReturn(row);
	pGridMenu->m_bIsLimit = BIZAPI::CheckLimit(GetTblType(), m_FlexDataSource.GetIdByIndex(Row_GridToData(row)));
	pGridMenu->m_bIsDisPatch = IsWorkOrOrderRow(row) && BIZAPI::CheckInstructionLimit(InsLimitOfAuthor_DisPatchWork);
	pGridMenu->m_bIsLeftColCopy = IsCanPaste() ? true : false;
	pGridMenu->m_bIsCopy = IsCanCopy() ? true : false;
	pGridMenu->m_bIsCut = IsCanCopy() ? true : false;
	const int nCmd = pGridMenu->CreatMenu(GetStTable(), LEFTCOL, ptScreen, this);
	pGridMenu->m_bIsLimit = true;
	pGridMenu->m_bIsDisPatch = true;

	switch (nCmd)
	{
	case ID_GRIDMENU_EDIT:			// �༭
		Edit(row);
		break;
	case ID_GRIDMENU_COPY:			// ����
		CTRL_C();
		break;
	case ID_GRIDMENU_CUT:			// ����
		CTRL_X();
		break;
	case ID_GRIDMENU_PASTE:			// ճ��
		CTRL_V();
		break;
	case ID_GRIDMENU_DELETE:		// ɾ����ѡ��
		{
			long tRow(0), lCol(0), bRow(0), rCol(0);
			GetSelection(&tRow, &lCol, &bRow, &rCol);
			DeleteRows(tRow, bRow);
		}
		break;
	case ID_GRIDMENU_CLEAR:			// ��ձ������
		ClearData();
		break;
	case ID_GRIDMENU_WILLORDERSURE:
		SureWillOrder(row);
		break;
	case ID_GRIDMENU_WILLORDERDIS:
		CancelWillOrder(row);
		break;
	case ID_BOM_BOMVIEW_EDITITEMTABL:
		OperItemBomView(row);
		break;
	case ID_GRIDMENU_WORK_RELATION:
		JumpToWorkRelation(row);
		break;
	case ID_GRIDMENU_WORK_CAN_USE_RESOURCE:
		JumpToWorkAvbResource(row);
		break;
	case ID_GRIDMENU_DELETE_WORK:
		DeleteWork();
		break;
	case ID_UI_GRIDMENU_DISPATH:
		Dispath(row);
		break;
	}
}

void CDataGridCtrl::TrackRHeadMenu(float X, float Y)
{
	const long mouseCol = get_MouseCol();
	const POINT ptScreen = MouseToScreen(GetSafeHwnd(), X, Y);
	CGridMenu * pGridMenu = CGridMenu::GetCGridMenu();
	long col1(0), col2(0), row1(0), row2(0);

	//�õ���ʱ��ѡ�����
	GetSelection(&row1, &col1, &row2, &col2);
	//m_listHideCol��HideCol�еõ�����������ص��к�
	if (!m_listHideCol.empty())
	{	//������ѡ��������Ƿ�������ص��кţ��еģ������á�������ء�Ϊ����	
		for (list<long>::iterator itr = m_listHideCol.begin(); itr != m_listHideCol.end(); itr++)
		{
			pGridMenu->m_bIsHideCol = false;//��������ء�������
			if ((*itr) > col1 && (*itr) < col2)
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
	case ID_GRIDMENU_HIDECOL:		// ������
		HideCol(mouseCol);
		break;
	case ID_GRIDMENU_DISPHIDECOL:	// ���������
		DispHideCol(mouseCol);
		break;
	case ID_GRIDMENU_FIXCOL:		// �̶���
		FixCol(mouseCol);
		break;
	case ID_GRIDMENU_DISPFIXCOL:	// ����̶���
		UnAllFixCol(mouseCol);
		break;
	case ID_GRIDMENU_SETCOLS:		// ���������
		SetCols();
		break;
	case ID_GRIDMENU_FIELD_SET:		// ���Զ���
		{
			const UINT hu = YKField::FuseTbl_field(GetTblType(), m_FlexDataSource.GetEnumIdByIndex(mouseCol-1));
			MSG ms;
			ms.message = TblFieldItem;
			ms.wParam = hu;
			ms.lParam = -1;
			::SendMessage(theApp.g_hMainFrm, IDU_UPDATE_MODIFY_DLG, (int)&ms, 0);
		}
		break;
	}
}

void CDataGridCtrl::TrackRDataMenu(float X, float Y)
{
	// ��Ϣ��û���Ҽ�����
	if(GetTblType() == TblMessageData)
		return;

	const long row = get_MouseRow();
	
	const POINT ptScreen = MouseToScreen(GetSafeHwnd(), X, Y);
	CGridMenu * pGridMenu = CGridMenu::GetCGridMenu();
	pGridMenu->m_bIsLimit = BIZAPI::CheckLimit(GetTblType(), m_FlexDataSource.GetIdByIndex(Row_GridToData(row)));
	pGridMenu->m_bIsCopy = IsCanCopy() ? true : false;
	pGridMenu->m_bEditContent = GetEditContentFlg() ? true : false;
	pGridMenu->m_bPaste = IsCanPaste() ? true : false;
	const int cmd = pGridMenu->CreatMenu(GetStTable(), GRIDCONTENT, ptScreen, this);
	pGridMenu->m_bIsLimit = true;

	switch (cmd)
	{
	case ID_GRIDMENU_PASTE:
		CTRL_V();
		break;
	case ID_GRIDMENU_COPY:
		CTRL_C();
		break;
	case ID_GRIDMENU_CUT:
		CTRL_X();
		break;
	case ID_GRIDMENU_DELETE:
		DeleteSelect();
		break;
	case ID_GRIDMENU_NEW:
		m_FlexDataSource.New();
		break;
	case ID_GRIDMENU_FIND:
		::PostMessage(theApp.g_hMainFrm, IDU_FIND_CODE_MFG, 0, 0);
		break;
	case ID_GRIDMENU_REPLACE:
		::PostMessage(theApp.g_hMainFrm, IDU_FIND_CODE_REPLACE, 0, 0);
		break;
	}
}

void CDataGridCtrl::TrackDlgMenu(long row)
{
	CBitmap bkBmp;
	if (!bkBmp.LoadBitmap(IDB_BITMAP_RIGHTCH))
		return;

	BITMAP temp;
	bkBmp.GetBitmap(&temp);
	CRect rect(0, 0, temp.bmWidth, temp.bmHeight);

	if (m_dlgMenu.GetSafeHwnd())
		m_dlgMenu.DestroyWindow();

	m_dlgMenu.Create(IDD_DLG_MENU, this);
	m_dlgMenu.m_rect = rect;

	// �����Ƿ���ʾSort�˵�
	const CString strText = get_TextMatrix(m_lClickRow, m_lclickCol);
	const int length = strText.GetLength();
	bool bAllChar = false;
	for (int i = length-1; i >= 0; i--)//�Ӻ���ǰ�������ж��ַ��Ƿ�Ϊ�����ַ�
	{
		if(AnalyseOneCString(strText[i]))
		{
			bAllChar = true;
			break;
		}
	}
	m_dlgMenu.m_bIsShowSort = bAllChar;	//�Ƿ���ʾsort�˵�

	// ���ø����ڼ���ʽ
	m_dlgMenu.SetParentHWnd(GetParent()->GetSafeHwnd());
	m_dlgMenu.ModifyStyle(0, SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);

	MoveDlgMenu(row);
}

void CDataGridCtrl::MoveDlgMenu(long row)
{
	m_lDlgMenuSpace = row - get_TopRow();
	float X = float(get_ColPos(m_rtDlgMenu.right) + get_ColWidth(m_rtDlgMenu.right));
	float Y = float(get_RowPos(row) + get_RowHeight(row));

	POINT ptMenuPos = MouseToScreen(GetSafeHwnd(), X, Y);
	POINT ptDlgPos = MouseToScreen2(GetSafeHwnd(), X, Y);
	m_dlgMenu.m_X = ptDlgPos.x;
	m_dlgMenu.m_Y = ptDlgPos.y - m_dlgMenu.m_rect.Height();
	m_dlgMenu.m_iX = ptMenuPos.x;
	m_dlgMenu.m_iY = ptMenuPos.y;

	m_dlgMenu.MoveWindow(m_dlgMenu.m_X, m_dlgMenu.m_Y, m_dlgMenu.m_rect.Width(), m_dlgMenu.m_rect.Height());
	m_dlgMenu.ShowWindow(SW_SHOW);
}
void CDataGridCtrl::HideCol(long col)
{
	/*long col = CalCol(X);*/
	if(col > 0)
		put_ColHidden(col, TRUE);

	m_listHideCol.push_back(col);//�洢�����е��к�

	//�޸�mode�еĶ�Ӧ��isHide Ϊ0
	stTable& curTable = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());
	curTable.mode[col - 1].SetIsHide(TRUE);// = 0;
	GetStTable().mode[col - 1].SetIsHide(TRUE);// = 0;

	//SaveData();///��������
}

//���������
void CDataGridCtrl::DispHideCol(long col)
{
	//	get_ColHidden()
	/*int col = CalCol(X);*/
	long col1 = 0;
	long col2 = 0;
	long row1 = 0;
	long row2 = 0;
	GetSelection(&row1,&col1,&row2,&col2);

	for(;col1<col2;col1++)
	{
		put_ColHidden(col1, FALSE);
		if (!m_listHideCol.empty())
		{
			for (list<long>::iterator itr = m_listHideCol.begin(); itr != m_listHideCol.end(); itr++)
			{
				if (col1 == (*itr))
				{
					m_listHideCol.erase(itr);
					/*int& iTemp = m_FlexDataSource.m_table.isHide.at(col1 - 1);
					iTemp = 1;*/
					GetStTable().mode[col1 - 1].SetIsHide(FALSE);// = 1;
					break;
				}
			}
		}
	}
	//m_bIsHideCol = false;
}

void CDataGridCtrl::FixCol(long col)
{
	const long lForzenCol = get_FrozenCols() + 1;
	if (col < lForzenCol)
		return;

	// �ƶ��̶���
	put_ColPosition(col, lForzenCol);
	stTable &tbl = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());
	DATATABLAPI::GoTostTblPos(tbl, col-1, lForzenCol-1);
	SetStTable(tbl);

	// �̶��̶���
	put_FrozenCols(lForzenCol);
	tbl.fixCol = lForzenCol;
	GetStTable().fixCol = lForzenCol;

	ResertDataSource();
	RedrawWindow();
}

void CDataGridCtrl::UnAllFixCol(long col)
{
	int nFrozenCols = 1;
	/*if (TblOrder == GetTblType() || TblWork == GetTblType())
		nFrozenCols = 1;*/

	put_FrozenCols(nFrozenCols);
	GetStTable().fixCol = nFrozenCols;
	stTable &tbl = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());
	tbl.fixCol = nFrozenCols;
}

void CDataGridCtrl::SetCols()
{    
	CDlgSetCols dlgSetCols;
	dlgSetCols.SetsTbl(GetStTable());
	if (dlgSetCols.DoModal() == IDOK)
	{
		stTable stTbl = dlgSetCols.GetsTbl();
		m_FlexDataSource.SetTable(stTbl);
// 		if(GetTblType() == TblMessageData)
// 			AdaptView();
		Flash(FALSE);
	}
}

void CDataGridCtrl::OptimizeTable()
{
	int rows = get_Rows();
	if(rows <= 0) return;
	int top = get_TopRow();//�õ��������
	int bottom = get_BottomRow();//�õ���Ͷ�����
	if(rows == 1)
	{
		top = 0;
		bottom = 0;
	}
	int left = get_LeftCol();//�õ����������
	int right = get_RightCol();//�õ����ұ�����
	int width = 0;//��width��ʱ����

	CDC *hDc = GetDC();//�õ�DC
	int j = 1;
	if(GetTblType() == TblMessageData) j = 0;
	bool bSTable = false;
	if (GetTblType() == TblOrder || 
		GetTblType() == TblWork)
	{
		bSTable = true;
	}
	for (; j <= right; j++)//�е�ѭ��
	{//�õ���ͷ�����ݣ���Ϊ���ұ߹�����������ʱ����Ҫ�ͱ�ͷ���ݵĿ�����Ƚ�			
		//CString str = get_TextMatrix(0,j);
		CSize s;
		s = hDc->GetTextExtent(get_TextMatrix(0,j));//�õ��ƶ��ַ�����size
		POINT pTemp1;
		pTemp1.x = s.cx;
		pTemp1.y = s.cy;
		ScrentToClient(GetSafeHwnd(), pTemp1);//��sizeת��������еĴ�С
		if (bSTable && j == 1 )
		{
			pTemp1.x += 300;
		}
		for (int i = top; i <= bottom; i++)
		{
			//CString strTemp = get_TextMatrix(i, j);//�õ���top������
			int length;					

			CSize sz = hDc->GetTextExtent(get_TextMatrix(i, j));//�õ���Ӧ���ݵ�size
			POINT pTemp;
			pTemp.x = sz.cx;
			pTemp.y = sz.cy;
			ScrentToClient(GetSafeHwnd(), pTemp);//ת��size�����
			length = pTemp.x;
			if (bSTable && j == 1 )
			{
				length += 300;
			}
			width = width < length ? length : width;//�Ƚϴ�ʱ�ַ����Ŀ�ȣ��õ����һ���ַ�������	
		}
		int w = pTemp1.x > width ? pTemp1.x : width;//���ͷ���ݳ������Ƚ�
		put_ColWidth(j, w);//����j�еĿ��
		width = 0;//��Ϊ0�������Ժ�Ƚ�
	}
	//�õ���ʱ��stTable�ṹ
	stTable &stTbl = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());
	const long cols = get_Cols();
	for (int i = 1; i < get_Cols(); i++)
	{	//�޸Ĵ�ʱ���ȫ���е�width
		stTbl.mode[i-1].width = get_ColWidth(i);
		GetStTable().mode[i-1].width = get_ColWidth(i);
	}
	//SaveData();//��������
}

void CDataGridCtrl::SureWillOrder(long mouseUpRow)
{
	m_FlexDataSource.SureWillOrder(Row_GridToData(mouseUpRow));

	RedrawWindow();
}

//ȡ�����򶩵�
void CDataGridCtrl::CancelWillOrder(long mouseUpRow)
{
	if(m_FlexDataSource.CancelWillOrder(Row_GridToData(mouseUpRow)))
	{
		// ���򶩵�ȡ����ɾ�����������͹��� ��Ҫˢ������
		BIZAPI::SetTableFlashFlg(TRUE,TblWork);
		BIZAPI::SetTableFlashFlg(TRUE,TblWorkAvbResource);
		BIZAPI::SetTableFlashFlg(TRUE,TblWorkRelation);

		BIZAPI::SetTableFlashFlg(TRUE, GetTblType());
		Flash(TRUE);
	}
}

void CDataGridCtrl::OperItemBomView(long mouseUpRow)
{
	/*long row = CalRow(Y);*/
	if(mouseUpRow < 1)
		return ;
	unsigned long itemId = m_FlexDataSource.GetItemIdByRow(mouseUpRow);
	if (itemId <= 0)
	{
		MessageBox(GetRemarkFiled(ID_UI_MB_PLEASE_SELECT_P_HP).c_str(),GetRemarkFiled(TIP).c_str());
		//MessageBox(L"��ѡ���Ʒ����Ʒ��");
		return;
	}
	theApp.m_BOMViewItemId=itemId;
	theApp.OpenTblPage(ID_BOM_VIEW_DLG);
}

bool CDataGridCtrl::IsWorkReturn( long  mouseUpRow )
{
	bool bOK = false;

	if(((GetTblType() == TblOrder || GetTblType() == TblWork) )&&
		m_FlexDataSource.GetGridRowTblType(Row_GridToData(mouseUpRow)) == TblWork)
	{
		bOK = true;
	}

	return bOK;
}

//��ת������������ϵ
void CDataGridCtrl::JumpToWorkRelation(long mouseUpRow)
{
	const long lRowInfoId = m_FlexDataSource.GetGridRowId(Row_GridToData(mouseUpRow));
	const UINT uTblType = m_FlexDataSource.GetGridRowTblType(Row_GridToData(mouseUpRow));
	if (TblWork == uTblType)
	{
		MSG msg;
		msg.message = (UINT)TblWorkRelation;
		msg.wParam = lRowInfoId;
		msg.lParam = uTblType;
		::SendMessage(GetParent()->GetSafeHwnd(), IDU_WORK_RELATION, (int)&msg, 0);
	}
}

//��ת������ʹ����Դ
void CDataGridCtrl::JumpToWorkAvbResource(long mouseUpRow)
{
	const long lRowInfoId = m_FlexDataSource.GetGridRowId(Row_GridToData(mouseUpRow));
	const UINT uTblType = m_FlexDataSource.GetGridRowTblType(Row_GridToData(mouseUpRow));
	if (TblWork == uTblType)
	{
		MSG msg;
		msg.message = (UINT)TblWorkAvbResource;
		msg.wParam = lRowInfoId;
		msg.lParam = uTblType;
		::SendMessage(GetParent()->GetSafeHwnd(), IDU_WORK_RELATION, (int)&msg, 0);
	}
}

BOOL CDataGridCtrl::OutputHtml()
{
	BOOL bOK = FALSE;

	CString tableName(GetRemarkFiled(GetTblType()).c_str());
	CString strPath = theApp.m_curPath;
	strPath += L"\\";
	strPath += tableName;
	strPath += L".html";
	CFlexUtil flex;
	if (flex.SaveGridToHTML(this,strPath,tableName))
	{
		ShellExecute(this->GetSafeHwnd(), NULL, strPath, NULL, NULL, SW_SHOWNORMAL);
		bOK = TRUE;
	}
	return bOK;
}

void CDataGridCtrl::DeleteWork()
{
	long rowSel1, colSel1, rowSel2, colSel2;
	//ɾ��ѡ������
	GetSelection(&rowSel1, &colSel1, &rowSel2, &colSel2);

	if(TblOrder == GetTblType())
	{
		bool flg = false;
		YKOrderPtrMap* pOrderMap = BIZAPI::GetBizMap()->GetYKOrderPtrMap();
		long row = rowSel1;
		for (; row <= rowSel2; row++)
		{
			const long id = m_FlexDataSource.GetGridRowId(row-1);
			if (id > -1)
			{
				if (m_FlexDataSource.GetGridRowTblType(row-1) == TblOrder)
				{
					YKOrderPtr& orderPtr = pOrderMap->Get(id);
					if (orderPtr != NULL)
					{
						orderPtr->DeleteAllWork();
						flg = true;
					}
				}
			}	
		}
		if (flg)
		{
#ifdef LIMITFUNCTION
			BIZAPI::LimitCheck();
#endif
			BIZAPI::SetTableFlashFlg(TRUE,TblWork);
			BIZAPI::SetTableFlashFlg(TRUE,TblWorkAvbResource);
			BIZAPI::SetTableFlashFlg(TRUE,TblWorkRelation);

			BIZAPI::SetTableFlashFlg(TRUE, GetTblType());
			Flash(TRUE);
			BIZAPI::GetBizMap()->RedrawAllGantt();
		}

	}
}
void CDataGridCtrl::Dispath(long mouseRow)
{
	stTable& stalbe = GetStTable();
	if (TblOrder == stalbe.tblType || TblWork == stalbe.tblType)
	{
		UINT uTblType = TblNone;
		if (m_FlexDataSource.GetGridRowId(mouseRow-1) > -1)
		{
			bool bDispathOrderFlg(false);
			uTblType = m_FlexDataSource.GetGridRowTblType(mouseRow-1);
			if (TblOrder == uTblType)
				bDispathOrderFlg = true;

			long rowSel1(0), colSel1(0), rowSel2(0), colSel2(0);
			//ɾ��ѡ������
			list<YKWorkPtr> dispathWorkPtrList;
			GetSelection(&rowSel1, &colSel1, &rowSel2, &colSel2);
			bool flg = false;
			YKOrderPtrMap* pOrderMap = BIZAPI::GetBizMap()->GetYKOrderPtrMap();
			YKWorkPtrMap* workPtrMap = BIZAPI::GetBizMap()->GetYKWorkPtrMap();

			long row = rowSel1;
			long rowId = -1;
			for (; row <= rowSel2;row++)
			{
				rowId = m_FlexDataSource.GetGridRowId(row-1);
				if (rowId > -1)
				{
					uTblType = m_FlexDataSource.GetGridRowTblType(row-1);
					if (TblOrder == uTblType)
					{
						YKOrderPtr& orderPtr = pOrderMap->Get(rowId);
						if (NULL != orderPtr)
						{
							list<YKWorkPtr> sonWorkList;
							orderPtr->GetWorkPtrList(sonWorkList);
							for (list<YKWorkPtr>::iterator iSon = sonWorkList.begin();
								iSon != sonWorkList.end();iSon++)
							{
								YKWorkPtr& ptr = *iSon;
								if (NULL != ptr)
								{
									if (DivType_NoDivWork == bDispathOrderFlg&& ptr->GetDivisionType())
										dispathWorkPtrList.push_back(ptr);
									if (!bDispathOrderFlg)
										dispathWorkPtrList.push_back(ptr);
								}
							}
						}
					}
					else if (TblWork == uTblType)
					{
						YKWorkPtr& workPtr = workPtrMap->Get(rowId);
						if (NULL != workPtr)
						{
							if (DivType_NoDivWork == bDispathOrderFlg&& workPtr->GetDivisionType())
								dispathWorkPtrList.push_back(workPtr);
							if (!bDispathOrderFlg)
								dispathWorkPtrList.push_back(workPtr);
						}
					}
				}
			}

			if (dispathWorkPtrList.empty())
				return;

			if (bDispathOrderFlg)
			{
				if (MessageBox(GetRemarkFiled(ID_UI_MB_PLEASE_RDISPATH_ORDER).c_str(),GetRemarkFiled(TIP).c_str()
					,MB_YESNO|MB_ICONQUESTION)== IDNO) return;
			}
			else
			{
				if (MessageBox(GetRemarkFiled(ID_UI_MB_PLEASE_RDISPATH_WORK).c_str(),GetRemarkFiled(TIP).c_str()
					,MB_YESNO|MB_ICONQUESTION)== IDNO) return;
			}

			//�����������
			BIZAPI::DisPatchWork(dispathWorkPtrList);
			//ˢ�½���
#ifdef LIMITFUNCTION
			BIZAPI::LimitCheck();
#endif
			if (bDispathOrderFlg)
				BIZAPI::SetTableFlashFlg(TRUE, TblOrder);

			BIZAPI::SetTableFlashFlg(TRUE, TblWork);
			BIZAPI::SetTableFlashFlg(TRUE, TblWorkAvbResource);
			BIZAPI::SetTableFlashFlg(TRUE, TblWorkRelation);

			BIZAPI::SetTableFlashFlg(TRUE, GetTblType());
			Flash(TRUE);
			BIZAPI::GetBizMap()->RedrawAllGantt();
		}
	}
}

void CDataGridCtrl::CopyCell()
{
	CString strText;
	variant_t var;

	const long begRow = m_rtDlgMenu.top;
	const long col = m_rtDlgMenu.left;
	const long endRow = m_rtDlgMenu.bottom;

	var = get_Cell(flexcpText, COleVariant(begRow), 
		COleVariant(col), COleVariant(begRow), COleVariant(col));
	
	//BSTR bstr = var.bstrVal;

	wstring temp = var.bstrVal;


	BIZAPI::BeginUndoRedo();
	const long minRow = min(begRow, endRow);
	const long maxRow = max(begRow, endRow);
	for (long row = minRow; row <= maxRow; row++)
	{
		if (m_FlexDataSource.GetGridRowShowFlag(Row_GridToData(row)))
		{
			//m_FlexDataSource.OnItemChanged(Row_GridToData(row), col-1, bstr);

			////Add 2011-11-21 
			//const long id = m_FlexDataSource.GetIdByIndex(Row_GridToData(row));					//����ID
			//DataModify(GetTblType(), id, m_FlexDataSource.m_table.mode[col-1].mode, temp);

			//Add ������/�������á����Ƶ�Ԫ�����
			const long id = m_FlexDataSource.GetIdByIndex(Row_GridToData(row));					//����ID
			const long tblType = GetTblType();	//���ID 
			const long ncol  = m_FlexDataSource.m_table.mode[col-1].mode;
			//Add 2012-01-12 �ԡ�����/�������á����п��ƣ����������ɸѡ����/����ɸѡ����/�ͻ��ֶ��б�/�ڲ���Ӧ���б�(�ڲ�ʹ��)/�����ⲿ����Դʱʹ�õļ� ��5��� ���Ƶ�Ԫ�����
			if (  TblLoadFileInfo == tblType  && (
				YKLoadFileInfo::YKLoadFileInfo_InsertCondition == ncol || YKLoadFileInfo::YKLoadFileInfo_OutputFilterContion == ncol 
				|| YKLoadFileInfo::YKLoadFileInfo_FieldEnum == ncol || YKLoadFileInfo::YKLoadFileInfo_MainKey == ncol 
				|| YKLoadFileInfo::YKLoadFileInfo_ClientField == ncol
				) 
				)
			{
				ClearDlgMenu();
				return;
			}
			else
			{
				DataModify(GetTblType(), id, ncol, temp);
			}
		}
	}
	BIZAPI::EndUndoRedo();

	ClearDlgMenu();
	ReDrawScreen(get_TopRow());
}

void CDataGridCtrl::SortCell()
{
	const long begRow = m_rtDlgMenu.top;
	const long col = m_rtDlgMenu.left;
	const long endRow = m_rtDlgMenu.bottom;

	CString strOld = get_TextMatrix(begRow, col);
	if (!strOld.IsEmpty())
	{
		variant_t var;
		CString strLeft = _T(""), strRight = _T(""), strText = _T("");
		int iTemp;

		var = get_Cell(flexcpText, COleVariant(begRow), 
			COleVariant(col), COleVariant(begRow), COleVariant(col));
		AnalyseData(iTemp, strLeft, strRight, strOld, var);

		//Add 2012-01-11
		const long  tblType = GetTblType();		//��ID
		const long  ncol = m_FlexDataSource.m_table.mode[col-1].mode;		//��

		if (begRow <= endRow)	// ��ͷ���µ���
		{
			BIZAPI::BeginUndoRedo();
			for (long row = begRow; row <= endRow; row++)
			{
				if (m_FlexDataSource.GetGridRowShowFlag(Row_GridToData(row)))
				{
					strText.Format(L"%d", iTemp);
					strText = strLeft + strText + strRight;
					//m_FlexDataSource.OnItemChanged(row - 1, col - 1, strText.AllocSysString());

					//Add 2011-11-21 
					unsigned long id = m_FlexDataSource.GetIdByIndex(Row_GridToData(row));				//����ID
					wstring temp = strText.GetString();

					//Add 2012-01-11 ���ơ�����/�������á������з�ʽ���					
					if ( TblLoadFileInfo == tblType &&( 
						YKLoadFileInfo::YKLoadFileInfo_MainKey == ncol 
					    || YKLoadFileInfo::YKLoadFileInfo_FieldEnum == ncol)
					   )
					{
						ClearDlgMenu();
						return;
					}
					else
						DataModify(GetTblType(),id, m_FlexDataSource.m_table.mode[col-1].mode,temp);

					iTemp++;
				}
			}
			BIZAPI::EndUndoRedo();
		}
		else	// ��β���ϵݼ�
		{
			BIZAPI::BeginUndoRedo();
			for (long row = begRow; row >= endRow; row--)
			{
				if (m_FlexDataSource.GetGridRowShowFlag(Row_GridToData(row)))
				{
					strText.Format(L"%d", iTemp);
					strText = strLeft + strText + strRight;
					//m_FlexDataSource.OnItemChanged(row - 1, col - 1, strText.AllocSysString());

					//Add 2011-11-21 
					unsigned long id = m_FlexDataSource.GetIdByIndex(Row_GridToData(row));				//����ID

					wstring temp = strText.GetString();

					//Add 2012-01-11 ���ơ�����/�������á������з�ʽ���	
					if (
						TblLoadFileInfo == tblType &&( 
					    YKLoadFileInfo::YKLoadFileInfo_MainKey == ncol 
						|| YKLoadFileInfo::YKLoadFileInfo_FieldEnum == ncol)
					   )
					{
						ClearDlgMenu();
						return;
					}
					DataModify(GetTblType(),id,m_FlexDataSource.m_table.mode[col-1].mode,temp);

					iTemp--;
				}
			}
			BIZAPI::EndUndoRedo();
		}

		ClearDlgMenu();
		ReDrawScreen(get_TopRow());
	}
}


// ��ʾ��Ϣ
CString CDataGridCtrl::GetShowCode()
{
	CString cstr;
	const long row = get_MouseRow();
	const long col = get_MouseCol();
	if (row >= 0 && col >= 0)
	{
		if (row == 0 && col > 0)
		{
			UINT enumIndex=m_FlexDataSource.GetEnumIdByIndex(col-1);
			TBL_FIELD_TYPE* pFiledType = LANGUAGEPACKAPI::GetTblFieldTypeByType(TblType(GetTblType()));
			if(NULL == pFiledType) return cstr;

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
			else if(enumIndex < (*pFiledType).size())
			{
				UINT type = (*pFiledType)[enumIndex];
				cstr = GetRemarkFiled(type).c_str();
				// û���ҵ���Ӧ�Ľ���  ѡ���ֶ���Ϣ
				if(cstr == GetRemarkFiled(-1).c_str())
				{
					TBL_FIELD_TYPE* tblFilekd = LANGUAGEPACKAPI::GetTblCsvFieldByType(TblType(GetTblType()));
					if (NULL != tblFilekd)
					{
						if(enumIndex < (*tblFilekd).size())
							cstr = GetRemarkFiled((*tblFilekd)[enumIndex]).c_str();
					}
				}
			}
			else if (IS_USER_DEF_ENM(enumIndex))
			{
				YKFieldPtr ptr = BIZAPI::GetBizMap()->GetYKFieldPtrMap()->Get(GetTblType(), enumIndex);
				if (NULL != ptr)
				{
					cstr = ptr->GetTipCode().c_str();
				}
			}
		}
		else cstr = get_TextMatrix(get_MouseRow(),get_MouseCol());	
		//����Ϣ������ʾ��λͼת��Ϊ�ַ�
		if (row > 0)
			m_FlexDataSource.TranMessageImageToStr(row ,col,cstr);
	}
	return cstr;
}
void CDataGridCtrl::ShowLast()
{
	const long row = m_FlexDataSource.GetRecordCount();
	if (row > 3)
		put_TopRow(row - 2);
	else 
		put_TopRow(0);
}


BOOL CDataGridCtrl::MouseInSelection()
{
	BOOL bOK = FALSE;

	const long mouseRow = get_MouseRow();
	const long mouseCol = get_MouseCol();
	long tRow(0), lCol(0), bRow(0), rCol(0);
	GetSelection(&tRow, &lCol, &bRow, &rCol);
	if (mouseRow >= tRow && mouseRow <= bRow &&
		mouseCol >= lCol && mouseCol <= rCol)
	{
		bOK = TRUE;
	}

	return bOK;
}

BOOL CDataGridCtrl::IsRightSelection()
{
	BOOL bOK = TRUE;

	long tRow(0), lCol(0), bRow(0), rCol(0);
	GetSelection(&tRow, &lCol, &bRow, &rCol);
	if (0 == tRow && 0 == lCol && 0 == bRow && 0 == rCol)
	{
		bOK = FALSE;
	}

	return bOK;
}


void CDataGridCtrl::GridDlg(int rowId, int Row, UINT m_tblType
		, UINT m_uId, stTable m_tableInfo/*, map<UINT,_variant_t>* m_varModidyList*/,int checkType)
{
	//�� Col
	int index = m_tableInfo.mode[rowId].mode;
	//����ID
	UINT id = m_FlexDataSource.GetIdByIndex(Row_GridToData(Row));
	//����
	UINT nTblType = m_FlexDataSource.m_table.tblType;
	
	//���ݱ���������id���� ȡֵ
	wstring wstr = BIZAPI::GetValue(nTblType,id,index);

	BIZAPI::BeginUndoRedo();
	//  ʱ��༭ -- �磺��������ʵ�ʿ�ʼ�ͽ���ʱ��
	if (checkType == CHECKTYPE_TIME1&&GetTblType()!=TblSchSysParam
		&&(GetTblType()!=TblWatchBoard))
	{
		CTmEditDlg tmDlg;
		tmDlg.Set(m_tblType,index);
		tmDlg.SetTmType(BIZAPI::GetBizMap()->GetTimeShowType_UI());
		//tmDlg.SetTm((*m_varModidyList)[index].bstrVal);
		tmDlg.SetTm(wstr);
		if (tmDlg.DoModal() == IDOK)
		{
			//CString str;
			//str = tmDlg.GettmCode();				
			////OnItemChanged(Row,rowId,str.AllocSysString());


			// Add 2011-11-14
			wstr = tmDlg.GettmCode().GetString();
			bool checkValue = BIZAPI::TableColsValueJudage(nTblType,id,index,wstr);
			if (checkValue)
			{
				DataModify(nTblType,id,index,wstr);
			}
		}
		return;
	}

	if (m_tblType == TblWatchBoard)
	{
		if ( index == YKWatchBoard::WatchBoard_WatchEndTm)//ȷ���ڼ�
		{
			wstring wstr ; 

			if (id >0)
			{
				BIZAPI::SetGetIDMode(TRUE);

				//Add 2011-10-24 
				wstr = BIZAPI::GetValue(nTblType,id,index);

				BIZAPI::SetGetIDMode(FALSE);
			}			
			CNewRuleDlg dlg;
			dlg.SetSetTimeFlg();
			dlg.InitRule(RULE_DEF_ALL,wstr,TblWatchBoard);
			dlg.SetFieldIndex(index);
			if (dlg.DoModal()==IDOK)
			{
				//Add 2011-10-24
				wstr = dlg.GetCode();
				DataModify(nTblType,id,index,wstr);
			}
		}

	}
	else if (index == DATESAVE_SPEC_COL)
	{
		CNewModelDlg  dlg;
		dlg.SetDivisionStr(L"/");
		//dlg.SetCodeUseType(TRUE,(*m_varModidyList)[index].bstrVal);
		dlg.SetCodeUseType(TRUE,wstr);
		dlg.TransInfo(1000,m_tblType,m_uId);
		if (dlg.DoModal() == IDOK)
		{			
			//OnItemChanged(Row,rowId,dlg.GetCode_Cstr().AllocSysString());

			//Add 2011-10-24
			wstr = dlg.GetCode_Wstr();
			DataModify(nTblType,id,index,wstr);
		}
	}
	else if(index == DATESAVE_SPEC_DATA_COL)
	{
		CNewModelDlg  dlg;
		dlg.SetDivisionStr(L"/");
		//dlg.SetCodeUseType(TRUE,(*m_varModidyList)[index].bstrVal);
		dlg.SetCodeUseType(TRUE,wstr);
		dlg.TransInfo(1001,m_tblType,m_uId);
		if (dlg.DoModal() == IDOK)
		{
			//OnItemChanged(Row,rowId,dlg.GetCode_Cstr().AllocSysString());

			//Add 2011-10-24
			wstr = dlg.GetCode_Wstr();
			DataModify(nTblType,id,index,wstr);
		}
	}
	else if (index == DATESAVE_SPEC_RANGE_COL)
	{
		CNewModelDlg  dlg;
		dlg.SetDivisionStr(L"/");
		//dlg.SetCodeUseType(TRUE,(*m_varModidyList)[index].bstrVal);
		dlg.SetCodeUseType(TRUE,wstr);
		dlg.TransInfo(1002,m_tblType,m_uId);
		if (dlg.DoModal() == IDOK)
		{
			//OnItemChanged(Row,rowId,dlg.GetCode_Cstr().AllocSysString());

			// Add 2011-11-10
			wstr = dlg.GetCode_Wstr();
			DataModify(nTblType,id,index,wstr);
		}
	}
	else if (m_tblType == TblWorkAvbResource)
	{
		if (index == YKWorkAvbResource::WorkAvbResource_ResId
			||index == YKWorkAvbResource::WorkAvbResource_PosstiveResId)
		{
			WavbResChangeDlg  dlg;
			dlg.SetId(m_uId);
			dlg.SetTblType(m_tblType);
			/*	
			_variant_t var_;
			_variant_t var_2;
			UINT type_ = 0;
			var_=(*m_varModidyList)[YKWorkAvbResource::WorkAvbResource_ResId];
			var_2=(*m_varModidyList)[YKWorkAvbResource::WorkAvbResource_PosstiveResId];
			dlg.SetActiveRes(var_.bstrVal);
			dlg.SetPosRes(var_2.bstrVal);
			*/

			//Add 2011-10-24 
			wstr = BIZAPI::GetValue(m_tblType,id,YKWorkAvbResource::WorkAvbResource_ResId);
			dlg.SetActiveRes(wstr);
			dlg.SetPosRes(BIZAPI::GetValue(nTblType,id,YKWorkAvbResource::WorkAvbResource_PosstiveResId));

			if (IDOK == dlg.DoModal())
			{		
				CString cstring1=dlg.GetActiveRes().c_str();
				CString cstring2=dlg.GetPosRes().c_str();

				list<CellMsg> Collist;
				CellMsg cellmsg;
				cellmsg.bstr = cstring1.AllocSysString();
				cellmsg.row = YKWorkAvbResource::WorkAvbResource_ResId;
				Collist.push_back(cellmsg);

				cellmsg.bstr = cstring2.AllocSysString();
				cellmsg.row = YKWorkAvbResource::WorkAvbResource_PosstiveResId;
				Collist.push_back(cellmsg);

				//OnItemChanged(Row,Collist,false);
				if (Collist.size() > 0)
				{
					//BIZAPI::BeginRebackOper(OPERTYPEMODIFY);
					//BIZAPI::BeginUndoRedo();

					// work avb res
					if (m_FlexDataSource.m_table.tblType == TblWorkAvbResource)
					{
						YKAvbResTempPtr& avbTempPtr=	BIZAPI::GetBizMap()->GetYKAvbResTempMap()->Get(m_FlexDataSource.GetGridRowId(Row_GridToData(Row)));
						if(avbTempPtr != NULL)
						{
							list<YKWorkAvbResourcePtr>& workAvbList = avbTempPtr->GetAvbResPtrList();
							for (list<YKWorkAvbResourcePtr>::iterator itor = workAvbList.begin();
								itor != workAvbList.end();itor++)
							{
								// ���볷����Ϣ
								RecordInfo reinfo;
								reinfo.id = (*itor)->GetId();
								reinfo.operType = OPERTYPEMODIFY;
								BIZAPI::InsertRelDataStruct(m_FlexDataSource.m_table.tblType,reinfo);
							}
						}
					}
					else
					{
						// ���볷����Ϣ
						RecordInfo reinfo;
						reinfo.id = m_FlexDataSource.GetGridRowId(Row_GridToData(Row));
						reinfo.operType = OPERTYPEMODIFY;
						BIZAPI::InsertRelDataStruct(m_FlexDataSource.m_table.tblType,reinfo);
					}

					for (list<CellMsg>::iterator itor = Collist.begin();itor!=Collist.end();itor++)
					{
						//m_FlexDataSource.OnItemChanged(nRow-1,itor->row,itor->bstr,b);

						//Add 2011-11-10
						wstr = itor->bstr;
						DataModify(nTblType,id,itor->row,wstr);
					}
					//BIZAPI::EndRebackOper();
					//BIZAPI::EndUndoRedo();
				}
				
			}
			//var_.Detach();
			//var_2.Detach();
		}
	}
	else if (m_tblType == TblApsParam)
	{
		switch(index)
		{
		case YKApsParam::SchRule:
			{
				CNewModelDlg  dlg;
				dlg.SetDivisionStr(L"");
				dlg.TransInfo(m_tblType,YKApsParam::SchRule,m_uId);
				if (dlg.DoModal() == IDOK)
				{
					//OnItemChanged(Row,rowId,dlg.GetCode_Cstr().AllocSysString());

					// Add 2011-11-10
					wstr = dlg.GetCode_Wstr();
					DataModify(nTblType,id,index,wstr);
				}
				break;
			}
		case YKApsParam::resRule:
			{
				CNewModelDlg  dlg;
				//_variant_t var_=(*m_varModidyList)[YKApsParam::resRule];
				dlg.SetDivisionStr(L",");
				//dlg.SetCodeUseType(TRUE,var_.bstrVal);
				dlg.SetCodeUseType(TRUE,wstr);
				dlg.TransInfo(m_tblType,YKApsParam::resRule,m_uId);
				if (dlg.DoModal() == IDOK)
				{
					//OnItemChanged(Row,rowId,dlg.GetCode_Cstr().AllocSysString());

					// Add 2011-11-10
					wstr = dlg.GetCode_Wstr();
					DataModify(nTblType,id,index,wstr);
				}
				break;
			}
		case YKApsParam::togetherDo:
			{
				CNewModelDlg  dlg;
				dlg.SetDivisionStr(L"#");
				//dlg.SetCodeUseType(TRUE,(*m_varModidyList)[YKApsParam::togetherDo].bstrVal);
				dlg.SetCodeUseType(TRUE,wstr);
				dlg.TransInfo(TblApsParam,YKApsParam::togetherDo,m_uId);
				if (dlg.DoModal() == IDOK)
				{
					//OnItemChanged(Row,rowId,dlg.GetCode_Cstr().AllocSysString());

					// Add 2011-11-10
					wstr = dlg.GetCode_Wstr();
					DataModify(nTblType,id,index,wstr);
				}
				break;
			}
		case YKApsParam::OrderFiterStr: //����ɸѡ���ʽ add 2009/11/11 
			{
				// change 2010-11-4 by ll
				//wstring wstr = (*m_varModidyList)[index].bstrVal;
				CNewRuleDlg dlg;
				dlg.InitRule(RULE_DEF_FILTER_Order,wstr,TblOrder);
				if (dlg.DoModal() == IDOK)
				{
					//OnItemChanged(Row,rowId,dlg.GetCode_cstr().AllocSysString());

					// Add 2011-11-10
					wstr = dlg.GetCode();
					DataModify(nTblType,id,index,wstr);
				}
			}
			break;
		case YKApsParam::WillOrderRelCondition: //���򶩵��������� 
			{
				COrderReationConditionDlg dlg;
				/*
				_variant_t vars=(*m_varModidyList)[YKApsParam::WillOrderRelCondition];
				vars.vt = VT_BSTR;
				UINT type = 0;
				wstring str=vars.bstrVal;
				vars.Detach();
				dlg.SetOrderReationCondition(str);
				dlg.SetOrderReationCondition(str,1);
				*/

				//Add 2011-10-24
				dlg.SetOrderReationCondition(wstr);
				dlg.SetOrderReationCondition(wstr,1);
				dlg.SetType(1);
				if (dlg.DoModal() == IDOK)
				{				
					/*CString cstring=dlg.GetOrderReationCondition().c_str();					
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetOrderReationCondition();
					DataModify(nTblType,id,index,wstr);
				}
			}
			break;
		case YKApsParam::HaveDesignOrderRelCondition://���Ʒ��ŵĶ�����������
			{
				COrderReationConditionDlg dlg;
				/*
				_variant_t vars=(*m_varModidyList)[YKApsParam::HaveDesignOrderRelCondition];
				vars.vt = VT_BSTR;
				UINT type = 0;
				wstring str=vars.bstrVal;
				vars.Detach();
				dlg.SetOrderReationCondition(str);
				dlg.SetOrderReationCondition(str,2);
				*/

				//Add 2011-10-24 
				dlg.SetOrderReationCondition(wstr);
				dlg.SetOrderReationCondition(wstr,2);
				dlg.SetType(2);
				if (dlg.DoModal() == IDOK)
				{
					/*CString cstring=dlg.GetOrderReationCondition().c_str();					
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetOrderReationCondition();
					DataModify(nTblType,id,index,wstr);

				}
			}
			break;
		case YKApsParam::ReationOrderTypeReferConsignmentTime://���ս����ڽ��������Ķ�������
			{
				COrderReationConditionDlg dlg;
				/*	
				_variant_t vars=(*m_varModidyList)[YKApsParam::ReationOrderTypeReferConsignmentTime];
				vars.vt = VT_BSTR;
				UINT type = 0;
				wstring str=vars.bstrVal;
				vars.Detach();
				dlg.SetOrderReationCondition(str);
				dlg.SetOrderReationCondition(str,3);
				*/
				dlg.SetOrderReationCondition(wstr);
				dlg.SetOrderReationCondition(wstr,3);
				dlg.SetType(3);
				if (dlg.DoModal() == IDOK)
				{
					/*CString cstring=dlg.GetOrderReationCondition().c_str();					
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetOrderReationCondition();
					DataModify(nTblType,id,index,wstr);
				}
			}
			break;
		case YKApsParam::OneOrderTranInfo:		// 1��1���䶩����Ϣ������Ϣ	add 2010-1-28 by ll
			{
				COrderInfoTranDlg dlg;
				/*
				_variant_t vars=(*m_varModidyList)[YKApsParam::OneOrderTranInfo];
				vars.vt = VT_BSTR;
				UINT type = 0;
				wstring str=vars.bstrVal;
				vars.Detach();
				dlg.SetDlgType(1);
				dlg.SetValueStr(str);
				*/
				dlg.SetDlgType(1);
				dlg.SetValueStr(wstr);
				if (dlg.DoModal() == IDOK)
				{
					/*CString cstring=dlg.GetValueStr().c_str();						
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetValueStr();
					DataModify(nTblType,id,index,wstr);
				}
				break;
			}
		case YKApsParam::AutoPudOrderTranInfo:	// �Զ��������충����Ϣ������Ϣ	add 2010-1-28 by ll
			{
				COrderInfoTranDlg dlg;
				/*
				_variant_t vars=(*m_varModidyList)[YKApsParam::AutoPudOrderTranInfo];
				vars.vt = VT_BSTR;
				UINT type = 0;
				wstring str=vars.bstrVal;
				vars.Detach();
				dlg.SetDlgType(2);
				dlg.SetValueStr(str);
				*/
				dlg.SetDlgType(2);
				dlg.SetValueStr(wstr);
				if (dlg.DoModal() == IDOK)
				{

					/*CString cstring=dlg.GetValueStr().c_str();					
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/
					// Add 2011-11-10
					wstr = dlg.GetValueStr();
					DataModify(nTblType,id,index,wstr);

				}
				break;
			}
		case YKApsParam::AutoBuyOrderTranInfo:	// �Զ�����ɹ�������Ϣ������Ϣ	add 2010-1-28 by ll
			{
				COrderInfoTranDlg dlg;
				/*
				_variant_t vars=(*m_varModidyList)[YKApsParam::AutoBuyOrderTranInfo];
				vars.vt = VT_BSTR;
				UINT type = 0;
				wstring str=vars.bstrVal;
				vars.Detach();
				dlg.SetDlgType(3);
				dlg.SetValueStr(str);
				*/
				dlg.SetDlgType(3);
				dlg.SetValueStr(wstr);
				if (dlg.DoModal() == IDOK)
				{

					/*CString cstring=dlg.GetValueStr().c_str();					
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetValueStr();
					DataModify(nTblType,id,index,wstr);

				}
				break;
			}
		case YKApsParam::WorkInfoTransfor:		// ������Ϣ����	add 2010-1-28 by ll
			{
				COrderInfoTranDlg dlg;
				/*
				_variant_t vars=(*m_varModidyList)[YKApsParam::WorkInfoTransfor];
				vars.vt = VT_BSTR;
				UINT type = 0;
				wstring str=vars.bstrVal;
				vars.Detach();
				dlg.SetDlgType(4);
				dlg.SetValueStr(str);
				*/
				dlg.SetDlgType(4);
				dlg.SetValueStr(wstr);
				if (dlg.DoModal() == IDOK)
				{
					/*
					CString cstring=dlg.GetValueStr().c_str();
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetValueStr();
					DataModify(nTblType,id,index,wstr);
				}
				break;
			}
		default :break;
		}
	}
	else if ( m_tblType == TblSchSysParam)
	{
		switch(index)
		{
		case YKSchSysParam::SchSysParam_ActualGetTm:
			{
				WatchBoardEndTmDlg watchboardEndTm;
				watchboardEndTm.SetDlgType(1);
				watchboardEndTm.SetWatchboardId(m_uId);
				if (watchboardEndTm.DoModal()==IDOK)
				{
					
					/*
					CString cstr=watchboardEndTm.GetWatchTm().c_str();					
					OnItemChanged(Row,rowId,cstr.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = watchboardEndTm.GetWatchTm();
					DataModify(nTblType,id,index,wstr);
				}
				break;
			}
		case YKSchSysParam::SchSysParam_StrFilePath:
			{
				TCHAR szFilters[]= _T("YukonSys data (*.yks)|*.yks|All Files (*.*)|*.*||");

				CFileDialog  openFile(TRUE,_T("yks"),_T("Data.yks"),OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFilters);
				if ( IDOK == openFile.DoModal() )
				{
					//OnItemChanged(Row,rowId,openFile.GetPathName().AllocSysString());

					// Add 2011-11-10
					wstr = openFile.GetPathName().GetString();
					DataModify(nTblType,id,index,wstr);
				}
				break;
			}
		case YKSchSysParam::SchSysParam_WorkCodeFormat:
			{
				//wstring wstr = (*m_varModidyList)[YKSchSysParam::SchSysParam_WorkCodeFormat].bstrVal;
				CNewRuleDlg dlg;
				dlg.InitRule(RULE_DEF_WORKCODE,wstr,TblWork);
				if (dlg.DoModal() == IDOK)
				{					
					/*
					CString cstring=dlg.GetCode().c_str();
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetCode();
					DataModify(nTblType,id,index,wstr);
				}
			}
			break;
		case YKSchSysParam::SchSysParam_ManOrderCodeFormat:
			{
				//wstring wstr = (*m_varModidyList)[YKSchSysParam::SchSysParam_ManOrderCodeFormat].bstrVal;
				CNewRuleDlg dlg;
				dlg.InitRule(RULE_DEF_AUTOORDER,wstr,TblOrder);
				if (dlg.DoModal()==IDOK)
				{
					
					/*
					CString cstring=dlg.GetCode().c_str();					
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetCode();
					DataModify(nTblType,id,index,wstr);
				}
			}
			break;
		case YKSchSysParam::SchSysParam_BuyOrderCodeFormat:
			{
				//wstring wstr = (*m_varModidyList)[YKSchSysParam::SchSysParam_BuyOrderCodeFormat].bstrVal;
				CNewRuleDlg dlg;
				dlg.InitRule(RULE_DEF_AUTOBUYORDER,wstr,TblOrder);
				if (dlg.DoModal()==IDOK)
				{					
					/*
					CString cstring=dlg.GetCode().c_str();					
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetCode();
					DataModify(nTblType,id,index,wstr);
				}
			}
			break;
		case YKSchSysParam::SchSysParam_OneToOneOrderCodeFormat:
			{
				//wstring wstr = (*m_varModidyList)[YKSchSysParam::SchSysParam_OneToOneOrderCodeFormat].bstrVal;
				CNewRuleDlg dlg;
				dlg.InitRule(RULE_DEF_AUTOONEORDER,wstr,TblOrder);
				if (dlg.DoModal()==IDOK)
				{					
					/*
					CString cstring=dlg.GetCode().c_str();					
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetCode();
					DataModify(nTblType,id,index,wstr);
				}
			}
			break;
		case YKSchSysParam::SchSysParam_MaintainOrderCodeFormat:
			{
				//wstring wstr = (*m_varModidyList)[YKSchSysParam::SchSysParam_MaintainOrderCodeFormat].bstrVal;
				CNewRuleDlg dlg;
				dlg.InitRule(RULE_DEF_AUTOEORDER,wstr,TblOrder);
				if (dlg.DoModal()==IDOK)
				{					
					/*
					CString cstring=dlg.GetCode().c_str();					
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetCode();
					DataModify(nTblType,id,index,wstr);
				}
			}
			break;
		case YKSchSysParam::SchSysParam_AntoItemCodeFormat:
			{
				//wstring wstr = (*m_varModidyList)[YKSchSysParam::SchSysParam_AntoItemCodeFormat].bstrVal;
				CNewRuleDlg dlg;
				dlg.InitRule(RULE_DEF_AUTOMIDDLEITEM,wstr,TblOperation);
				if (dlg.DoModal()==IDOK)
				{
					
					/*
					CString cstring=dlg.GetCode().c_str();						
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetCode();
					DataModify(nTblType,id,index,wstr);
				}
			}
			break;
		case YKSchSysParam::SchSysParam_BaseTime:   // add 
		case YKSchSysParam::SchSysParam_SchStartTime:   // add 
		case YKSchSysParam::SchSysParam_SchEndTime:   // add 
		case YKSchSysParam::SchSysParam_ShowStartTime:   // add 
		case YKSchSysParam::SchSysParam_ShowEndTime:   // add 
			{
				//wstring wstr = (*m_varModidyList)[YKSchSysParam::SchSysParam_BaseTime].bstrVal;
				CNewRuleDlg dlg;
				dlg.InitRule(RULE_DEF_ALL,wstr,TblSchSysParam);
				if (dlg.DoModal()==IDOK)
				{					
					/*
					CString cstring=dlg.GetCode().c_str();					
					OnItemChanged(Row,rowId,cstring.AllocSysString());
					*/

					// Add 2011-11-10
					wstr = dlg.GetCode();
					DataModify(nTblType,id,index,wstr);
				}
				break;			

			}
			break;
		}

	}
	else if (m_tblType == TblResource)
	{
		switch(index)
		{
		case YKResource::Resource_Restrict:
			{
				CNewModelDlg  dlg;
				dlg.SetDivisionStr(L" <= ");
				//dlg.SetCodeUseType(TRUE,(*m_varModidyList)[index].bstrVal);
				dlg.SetCodeUseType(TRUE,wstr);
				dlg.TransInfo(TblResource,YKResource::Resource_Restrict,m_uId);
				if (dlg.DoModal() == IDOK)
				{
					//OnItemChanged(Row,rowId,dlg.GetCode_Cstr().AllocSysString());

					// Add 2011-11-10
					wstr = dlg.GetCode_Wstr();
					DataModify(nTblType,id,index,wstr);
				}
				break;
			}
		case YKResource::Resource_StoveResAvbExp:
			{
				CNewModelDlg  dlg;
				dlg.SetStoveResAvbExpType(1);
				dlg.TransInfo(TblResource,YKResource::Resource_StoveResAvbExp,m_uId);
				if (dlg.DoModal() == IDOK)
				{
					////Add 2011-10-24  ϵͳд��
					//wstr = dlg.GetCode_Cstr().AllocSysString();
					//short rValue = BIZAPI::InsertValue(nTblType,id,index,wstr);

					// Add 2011-11-10
					wstr = dlg.GetCode_Wstr();
					DataModify(nTblType,id,index,wstr);
				}
				break;
			}
		case YKResource::Resource_BackResId:
			{
				CNewModelDlg  dlg;
				dlg.TransInfo(TblResource,YKResource::Resource_BackResId,m_uId);
				if (dlg.DoModal() == IDOK)
				{
					////Add 2011-10-24  ϵͳд��
					//wstr = dlg.GetCode_Cstr().AllocSysString();
					//short rValue = BIZAPI::InsertValue(nTblType,id,index,wstr);

					// Add 2011-11-10
					wstr = dlg.GetCode_Wstr();
					DataModify(nTblType,id,index,wstr);

				}
				break;
			}
		case YKResource::Resource_Department:
			{
				CNewModelDlg  dlg;
				dlg.TransInfo(TblResource,YKResource::Resource_Department,m_uId);
				if (dlg.DoModal() == IDOK)
				{
					////Add 2011-10-24  ϵͳд��
					//wstr = dlg.GetCode_Cstr().AllocSysString();
					//short rValue = BIZAPI::InsertValue(nTblType,id,index,wstr);
					// Add 2011-11-10
					wstr = dlg.GetCode_Wstr();
					DataModify(nTblType,id,index,wstr);
				}
				break;
			}
		default:
			break;
		}						
	}
	else if (m_tblType == TblItem)
	{


			CNewModelDlg  dlg;
			dlg.TransInfo(TblItem,YKItem::Item_Depart,m_uId);
			if (dlg.DoModal() == IDOK)
			{
				//OnItemChanged(Row,rowId,dlg.GetCode_Cstr().AllocSysString());

				// Add 2011-11-10
				wstr = dlg.GetCode_Wstr();
				DataModify(nTblType,id,index,wstr);
			}

	}
	else if (m_tblType == TblCalendar)
	{
		if (index == YKCalendar::Calendar_DayShiftId)
		{
			CCalendarMgrDlg dlg;
			dlg.SetId(m_uId);
			//YK_WSTRING tempStr = (*m_varModidyList)[index].bstrVal;
			dlg.SetDayShiftStr(wstr);
			if (dlg.DoModal() == IDOK)
			{
				vector<YK_WSTRING>& lst = dlg.GetDayShiftStrList();
				CString str(TOOLS::JoinStr(lst,_T(";")).c_str());

				//OnItemChanged(Row,rowId,str.AllocSysString());

				// Add 2011-11-10
				wstr = str.GetString();
				DataModify(nTblType,id,index,wstr);

			}
		}
	}
	else if (m_tblType == TblSettingTm)
	{
		if (index == YKSettingTm::SettingTm_PrevSpc)
		{
			CSetTmDlg dlg;
			dlg.SetDlgType(1);
			dlg.SetuId(m_uId);

			if (dlg.DoModal() == IDOK)
			{
				////Add 2011-10-24  ϵͳд��
				//wstr = dlg.GetName().AllocSysString();
				//short rValue = BIZAPI::InsertValue(nTblType,id,index,wstr);

				// Add 2011-11-10
				wstr = dlg.GetName().GetString();
				DataModify(nTblType,id,index,wstr);
			}
		}
		else if (index == YKSettingTm::SettingTm_NextSpc)
		{
			CSetTmDlg dlg;
			dlg.SetDlgType(2);
			dlg.SetuId(m_uId);

			if (dlg.DoModal() == IDOK)
			{
				////Add 2011-10-24  ϵͳд��
				//wstr = dlg.GetName().AllocSysString();
				//short rValue = BIZAPI::InsertValue(nTblType,id,index,wstr);
				// Add 2011-11-10
				wstr = dlg.GetName().GetString();
				DataModify(nTblType,id,index,wstr);
			}
		}
	}
	else if (m_tblType == TblOrder)
	{
		switch(index)
		{
		case YKOrder::Order_NextOrderIds:
		case YKOrder::Order_ToNextOrderMaxSapceTm:
		case YKOrder::Order_ToNextOrderMinSapceTm:
			{
				CNewModelDlg  Dlg;
				Dlg.TransInfo(TblOrder,YKOrder::Order_NextOrderIds,m_uId);
				if (Dlg.DoModal() == IDOK)
				{
					list<CellMsg> Rowlist;
					CellMsg cellmsg;
					cellmsg.bstr = Dlg.GetNexOrderStr().AllocSysString();
					cellmsg.row = YKOrder::Order_NextOrderIds;
					Rowlist.push_back(cellmsg);

					cellmsg.bstr = Dlg.GetMinTmStr().AllocSysString();
					cellmsg.row = YKOrder::Order_ToNextOrderMinSapceTm;
					Rowlist.push_back(cellmsg);

					cellmsg.bstr = Dlg.GetMaxTmStr().AllocSysString();
					cellmsg.row = YKOrder::Order_ToNextOrderMaxSapceTm;
					Rowlist.push_back(cellmsg);

					//OnItemChanged(Row,Rowlist,false);

					if (Rowlist.size() > 0)
					{
						//BIZAPI::BeginRebackOper(OPERTYPEMODIFY);
						BIZAPI::BeginUndoRedo();

						// work avb res
						if (m_FlexDataSource.m_table.tblType == TblWorkAvbResource)
						{
							YKAvbResTempPtr& avbTempPtr=	BIZAPI::GetBizMap()->GetYKAvbResTempMap()->Get(m_FlexDataSource.GetGridRowId(Row_GridToData(Row)));
							if(avbTempPtr != NULL)
							{
								list<YKWorkAvbResourcePtr>& workAvbList = avbTempPtr->GetAvbResPtrList();
								for (list<YKWorkAvbResourcePtr>::iterator itor = workAvbList.begin();
									itor != workAvbList.end();itor++)
								{
									// ���볷����Ϣ
									RecordInfo reinfo;
									reinfo.id = (*itor)->GetId();
									reinfo.operType = OPERTYPEMODIFY;
									BIZAPI::InsertRelDataStruct(m_FlexDataSource.m_table.tblType,reinfo);
								}
							}
						}
						else
						{
							// ���볷����Ϣ
							RecordInfo reinfo;
							reinfo.id = m_FlexDataSource.GetGridRowId(Row_GridToData(Row));
							reinfo.operType = OPERTYPEMODIFY;
							BIZAPI::InsertRelDataStruct(m_FlexDataSource.m_table.tblType,reinfo);
						}

						for (list<CellMsg>::iterator itor = Rowlist.begin();itor!=Rowlist.end();itor++)
						{
							//m_FlexDataSource.OnItemChanged(nRow-1,itor->row,itor->bstr,b);

							// Add 2011-11-10
							wstr = itor->bstr;

							bool checkValue(false) ;
							if ( nTblType == TblOrder && index == 19 )		//Add 2012-04-24 ������<�󶩵��ֶ����⴦��<"|">>
							{
								checkValue = true;
							}else
							{
								checkValue = BIZAPI::TableColsValueJudage(nTblType,id,index,wstr);
							}

							if (checkValue)
							{
								DataModify(nTblType,id,itor->row,wstr);
							}
						}
					//BIZAPI::EndRebackOper();
						BIZAPI::EndUndoRedo();
					}
					
				}
				break;
			}
		case YKOrder::Order_InterOperation :      //	�м乤��/* 
			{
				////if ((*m_varModidyList)[YKOrder::Order_Type].bstrVal==GetRemarkFiled(Order_Produce))
				//if(wstr == GetRemarkFiled(Order_Produce))
				//Add 2011-10-27
				//if(index == YKOrder::Order_InterOperation )
				if (GetRemarkFiled(Order_Produce) == BIZAPI::GetValue(TblOrder,m_uId,YKOrder::Order_Type))
				{
					CNewModelDlg  Dlg;
					Dlg.TransInfo(TblOrder,YKOrder::Order_InterOperation,m_uId);
					if (Dlg.DoModal() == IDOK)
					{
						
						//CString str=Dlg.GetInterOperationStr().c_str();
						////Add 2011-10-24  ϵͳд��
						//wstr = str.AllocSysString();
						//short rValue = BIZAPI::InsertValue(nTblType,id,index,wstr);

						// Add 2011-11-10
						wstr = Dlg.GetInterOperationStr();
						DataModify(nTblType,id,index,wstr);
					}
					break;
				}
				break;
			}
		case YKOrder::Order_workActual :      //	����ʵ������/* 
			{
				////if ((*m_varModidyList)[YKOrder::Order_Type].bstrVal==GetRemarkFiled(Order_Produce))
				//if(wstr == GetRemarkFiled(Order_Produce))
				//Add 2011-10-27 
				if (GetRemarkFiled(Order_Produce) == BIZAPI::GetValue(TblOrder,m_uId,YKOrder::Order_Type))
				{
					CNewModelDlg  Dlg;
					Dlg.TransInfo(TblOrder,YKOrder::Order_workActual,m_uId);
					if (Dlg.DoModal() == IDOK)
					{
						/*
						CString str=Dlg.GetActualStr().c_str();							
						OnItemChanged(Row,rowId,str.AllocSysString());
						*/

						// Add 2011-11-10
						wstr = Dlg.GetActualStr();
						DataModify(nTblType,id,index,wstr);
					}
					break;
				}
				break;
			}
		case YKOrder::Order_DePart:
			{
				CNewModelDlg  dlg;
				dlg.TransInfo(TblOrder,YKOrder::Order_DePart,m_uId);
				if (dlg.DoModal() == IDOK)
				{
					//OnItemChanged(Row,rowId,dlg.GetCode_Cstr().AllocSysString());

					// Add 2011-11-10
					wstr = dlg.GetCode_Wstr();
					DataModify(nTblType,id,index,wstr);
				}
				break;
			}
		default:
			break;
		}
	}
	else if (m_tblType == TblLoadFileInfo)
	{
		if (index == YKLoadFileInfo::YKLoadFileInfo_ClientField
			|| index == YKLoadFileInfo::YKLoadFileInfo_FieldEnum)
		{
			CLoadFileDlg lfDlg;
			lfDlg.SetUId(m_uId);
			if (lfDlg.DoModal() == IDOK)
			{
				YKLoadFileInfoPtr ptr = lfDlg.GetlfPtr();
				if (ptr != NULL){

					list<FieldInfo>& fieldList = lfDlg.GetFieldInfoList();
					//BIZAPI::BeginRebackOper(OPERTYPEMODIFY);
					BIZAPI::BeginUndoRedo();
					// ���볷����Ϣ
					RecordInfo reinfo;
					reinfo.id = m_uId;
					reinfo.operType = OPERTYPEMODIFY;
					BIZAPI::InsertRelDataStruct(m_tblType,reinfo);
					//BIZAPI::EndRebackOper();
					BIZAPI::EndUndoRedo();

					ptr->SetField(fieldList);			
				}
			}
		}
		else if (index == YKLoadFileInfo::YKLoadFileInfo_ClientTblName)
		{
			CTableNameDlg dlg;
			//YK_WSTRING strFileType = (*m_varModidyList)[YKLoadFileInfo::YKLoadFileInfo_FileType].bstrVal ;
			YK_WSTRING strFileType = BIZAPI::GetValue(TblLoadFileInfo,m_uId,YKLoadFileInfo::YKLoadFileInfo_FileType) ;
			if (strFileType == _T("OLE DB"))	
				dlg.m_Type = 0;
			else	
				dlg.m_Type = 1;			
			YKLoadFileInfoPtr ptr = BIZAPI::GetBizMap()->GetYKLoadFileInfoPtrMap()->Get(m_uId);
			if (ptr != NULL)
			{
				wstring& str = ptr->GetStrConnection();
				dlg.SetLinkString(str);

				if (dlg.DoModal() == IDOK)
				{
					CString strTablName(dlg.GetSelectTableName().c_str());
					if (!strTablName.IsEmpty())
					{
						//OnItemChanged(Row,rowId,strTablName.AllocSysString());

						// Add 2011-11-10
						wstr = strTablName.GetString();
						DataModify(nTblType,id,index,wstr);
					}					
				}
			}
		}
		else if (index == YKLoadFileInfo::YKLoadFileInfo_Connection)
		{
			YKLoadFileInfoPtr ptr = BIZAPI::GetBizMap()->GetYKLoadFileInfoPtrMap()->Get(m_uId);
			if (ptr != NULL)
			{
				if (ptr->GetFileType() == FILE_OLE_DB)
				{
					CDataSource ds;
					if (ds.Open()==S_OK)
					{
						BSTR pStr;
						ds.GetInitializationString(&pStr,true);
						YKLoadFileInfo::EncryptConnection(pStr,true);
						//OnItemChanged(Row,rowId,pStr);

						// Add 2011-11-10
						wstr = (pStr);
						DataModify(nTblType,id,index,wstr);
						SysFreeString(pStr);
					}
				}
				else
				{

					CWnd * pWnd = theApp.GetMainWnd();

					if ( !pWnd ) return;

					BROWSEINFO bi;
					ITEMIDLIST* pidl;
					TCHAR buf[MAX_PATH];
					memset(buf,0,MAX_PATH);
					bi.hwndOwner		= pWnd->GetSafeHwnd();
					bi.pidlRoot			= NULL;
					bi.pszDisplayName	= buf;
					bi.lpszTitle		= GetRemarkFiled(ID_UI_TEXT_CHOOSEOPENFILE).c_str();   // _T("��ѡ��򿪵��ļ���")
					bi.ulFlags			= BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN ;
					bi.lpfn				= NULL;
					bi.lParam			= 0;
					bi.iImage			= IDR_MAINFRAME;

					pidl = SHBrowseForFolder(&bi);
					if (pidl != NULL)
					{
						SHGetPathFromIDList(pidl,buf);
						CString strDir = buf;
						LPMALLOC lpMalloc;
						HRESULT hr = SHGetMalloc(&lpMalloc);
						//OnItemChanged(Row,rowId,strDir.AllocSysString());

						// Add 2011-11-10
						wstr = strDir.GetString();
						DataModify(nTblType,id,index,wstr);

						lpMalloc->Free(pidl);
						lpMalloc->Release();
					}
				}
			}

		}
		else if (index == YKLoadFileInfo::YKLoadFileInfo_InsertCondition 
			|| index == YKLoadFileInfo::YKLoadFileInfo_OutputFilterContion)
		{
			YKLoadFileInfoPtr& ptr = BIZAPI::GetBizMap()->GetYKLoadFileInfoPtrMap()->Get(m_uId);
			if (ptr != NULL)
			{
				//wstring wstr = (*m_varModidyList)[index].bstrVal;
				CNewRuleDlg dlg;
				dlg.InitRule(RULE_DEF_FILTER,wstr,ptr->GetClientTblType());
				if (dlg.DoModal() == IDOK)
				{
					wstr = dlg.GetCode();
					//CString conCstr(conditionStr.c_str());
					////OnItemChanged(Row,rowId,conCstr.AllocSysString());

					//// Add 2011-11-10
					//wstr = conCstr.GetString();
					DataModify(nTblType,id,index,wstr);
				}
			}
		}
		else if (index == YKLoadFileInfo::YKLoadFileInfo_MainKey)
		{
			YKLoadFileInfoPtr& ptr = BIZAPI::GetBizMap()->GetYKLoadFileInfoPtrMap()->Get(m_uId);
			if (ptr != NULL)
			{
				UINT clientTbl = ptr->GetClientTblType();
				if (clientTbl == TblWorkInputItem || clientTbl == TblWorkOutputItem
					||clientTbl == TblWorkRelation || clientTbl == TblRestrictInfo
					||clientTbl == TblCategorySequence || clientTbl == TblUseResource
					||clientTbl == TblResourceSequence || clientTbl == TblWorkChange
					||clientTbl == TblMessageData || clientTbl == TblResultWork
					|| clientTbl == TblCalendar
					||clientTbl == TblSettingTm || clientTbl == TblTransportTm
					|| clientTbl == TblItemGather
					|| clientTbl == TblManufactureCapitalInfor)
				{
					return;
				}
				YK_WSTRING mainKey = ptr->GetMainKey();
				UINT Tbltype = ptr->GetClientTblType();
				CMainKeySetDlg maiKeyDlg;
				maiKeyDlg.SetblType(Tbltype);
				maiKeyDlg.SetMainKeyCode(mainKey);
				if (maiKeyDlg.DoModal() == IDOK)
				{
					wstr =maiKeyDlg.GetMainKeyCoede();
					DataModify(nTblType,id,index,wstr);
				}
			}
		}
	}
	else if (m_tblType == TblGanttPara)
	{
		CMarksSetDlg dlg;
		if (index == YKGanttPara::GanttPara_MarksContext)
		{
			dlg.SetDlgType(WorkMark);
			if (dlg.DoModal() == IDOK)
			{
				wstr = dlg.GetInfoStr().GetString();
				DataModify(nTblType,id,index,wstr);
			}
		}
		else if (index == YKGanttPara::GanttPara_TipsContext)		
		{
			dlg.SetDlgType(WorkTip);
			if (dlg.DoModal() == IDOK)
			{
				wstr = dlg.GetInfoStr().GetString();
				DataModify(nTblType,id,index,wstr);
			}
		}
		else if ( index == YKGanttPara::GanttPara_ResMarksContext  ||index == YKGanttPara::GanttPara_ItemMarksContext 
				  || index == YKGanttPara::GanttPara_OrderMarksContext 
				)		//��Դ����ͼ ��Դ����Ʒ������	��ʾ��ǩ��������
		{
			if ( dlg.DoModal() == IDOK )
			{
				wstr = dlg.GetInfoStr().GetString();
				DataModify(nTblType,id,index,wstr);
			}
		}
		else if (index == YKGanttPara::GanttPara_FontFormat)
		{
			YKBizMap* pMap = BIZAPI::GetBizMap();
			/*YK_WSTRING strName;
			long ulColor;
			int size;
			pMap->GetFontFormat(strName,ulColor,size);*/

			COLORREF dwColor;
			YK_WSTRING wstrFacename;
			int nSize;
			YK_WSTRING wstrStylename;
			BOOL bUnderline;
			int nWeight;
			BYTE nStyle;
			BOOL bItalic;
			BOOL bStrikeout;

			pMap->GetFontFormat( dwColor,wstrFacename,nSize,wstrStylename,bUnderline,nWeight,nStyle,bItalic,bStrikeout);
			
			LOGFONT		logFont;
			memset(&logFont, 0, sizeof(LOGFONT));
			//_tcscpy_s(logFont.lfFaceName,strName.size()+1,strName.c_str());
			_tcscpy_s(logFont.lfFaceName,wstrFacename.length()+1,wstrFacename.c_str());

			//// Add Focus altered values if required
			//// Init LOGFONT
			//logFont.lfWeight=FW_NORMAL;
			//logFont.lfItalic=FALSE;
			//logFont.lfUnderline=FALSE;
			//logFont.lfStrikeOut=FALSE;
			//logFont.lfHeight= strSize/*size*/;
			////-----------------------------------
			//// Create font 
			////-----------------------------------
			//logFont.lfCharSet=DEFAULT_CHARSET;
			//logFont.lfOutPrecision=OUT_DEFAULT_PRECIS;
			//logFont.lfClipPrecision=CLIP_DEFAULT_PRECIS;
			//logFont.lfQuality=ANTIALIASED_QUALITY;
			//logFont.lfPitchAndFamily=DEFAULT_PITCH;

			//Add 2012-03-08
			logFont.lfWeight=nWeight;
			logFont.lfItalic=bItalic;
			logFont.lfUnderline=bUnderline;
			logFont.lfStrikeOut=bStrikeout;
			logFont.lfHeight= nSize;
			//-----------------------------------
			// Create font 
			//-----------------------------------
			logFont.lfCharSet=nStyle;
			logFont.lfOutPrecision=OUT_DEFAULT_PRECIS;
			logFont.lfClipPrecision=CLIP_DEFAULT_PRECIS;
			logFont.lfQuality=ANTIALIASED_QUALITY;
			logFont.lfPitchAndFamily=DEFAULT_PITCH;

			CFontDialog fontDlg(&logFont);
			fontDlg.m_cf.rgbColors = dwColor;

			if (IDOK == fontDlg.DoModal())
			{
				//// Add 2011-11-10
				//wstr = newFont.GetString();
				//DataModify(nTblType,id,index,wstr);

				dwColor/*ulColor*/ = fontDlg.GetColor();
				wstrFacename = fontDlg.GetFaceName().GetString();
				nSize = logFont.lfHeight;
				wstrStylename = fontDlg.GetStyleName().GetString();
				bUnderline = fontDlg.IsUnderline();
				nWeight = fontDlg.GetWeight();
				nStyle = logFont.lfCharSet;
				bItalic = fontDlg.IsItalic();
				bStrikeout = fontDlg.IsStrikeOut();		

				fontDlg.GetCurrentFont(&logFont);
				CString newFont;
				//newFont.Format(_T("%s;%d;%d"),logFont.lfFaceName,ulColor,logFont.lfHeight);

				newFont.Format(_T("%d;%s;%d;%s;%d;%d;%d;%d;%d"),dwColor,wstrFacename.c_str(),nSize,wstrStylename.c_str(),bUnderline,nWeight,nStyle,bItalic,bStrikeout);

				// Add 2011-11-10
				wstr = newFont.GetString();
				DataModify(nTblType,id,index,wstr);

				BIZAPI::GetBizMap()->SetFontFormat(wstr);
			}
		}
	}else if (m_tblType == TblCreateMTOrderParam)
	{
		if (index == YKCreateMTOrderParam::CreateMTOrderParam_Condition)
		{
			//CMTOrderContionSetDlg dlg;
			//dlg.SetContionStr(wstr.c_str());
			//if (dlg.DoModal() == IDOK)
			//{
			//	////Add 2011-10-24  ϵͳд��
			//	//wstr = dlg.GetContionStr().AllocSysString();
			//	//short rValue = BIZAPI::InsertValue(nTblType,id,index,wstr);

			//	// Add 2011-11-10
			//	wstr = dlg.GetContionStr().GetString();
			//	DataModify(nTblType,id,index,wstr);
			//}

			MTCIPDLG cipDlg;
		
			cipDlg.SetContextText(wstr);

			//
			cipDlg.SetShowRectWindow(false);
			if ( cipDlg.DoModal() == IDOK )
			{
				wstr = cipDlg.GetCodeWstr();

				DataModify(nTblType,id,index,wstr);
			}


		}
	}

	BIZAPI::EndUndoRedo();
}


void CDataGridCtrl::MouseDownVsflexgrid( short Button, short Shift, float X, float Y )
{
	if (1 != Button && 2 != Button)	// ֻ�������������һ��¼�
		return;

	const long row = get_MouseRow();
	const long col = get_MouseCol();

	YKGridFindAndReplaceMgr::Get()->ClearFindSel();
	if (m_bCrossCursor && 1 == Button) // ��ʮ�ֹ�괦������
	{
		::SetCursor(m_hCursor); // ����ʮ�ֹ��
		Select(m_lClickRow, m_lclickCol, COleVariant(m_lClickRow), COleVariant(m_lclickCol));

		put_SelectionMode(flexSelectionListBox);
		m_rtDlgMenu.SetRect(m_lclickCol, m_lClickRow, m_lclickCol, row);
		return;
	}
	else
	{
		ClearDlgMenu();
	}

	if ((col == 0 && row >= 0) ||	// ����һ�У�������Ч
		(row > 0  && col > 0))		// ��������������Ч
	{
		if (0 == col)	// ��һ�У���ʱ���ơ�ճ��Ϊ�½�����
		{
			if (1 == Button)	// ������
			{
				if (0 == row)	// ���Ͻǣ�ȫѡ������
				{
					const long lastDataRow = Row_DataToGrid(m_FlexDataSource.GetGridRowListSize()-1);
					Select(1, 1, COleVariant(lastDataRow), COleVariant(get_Cols()-1));
				}
				else
				{
					Select(row, 1, COleVariant(row), COleVariant(get_Cols()-1));
				}

				if (1 == Shift) // ��Shift��ѡ������
				{
					Select(m_lClickRow, 1, COleVariant(row), COleVariant(get_Cols()-1));
				}
			}
			else if (row > 0)
			{
				long tRow(0), lCol(0), bRow(0), rCol(0);
				GetSelection(&tRow, &lCol, &bRow, &rCol);
				if (!(row >= tRow && row <= bRow && lCol == 1 && rCol == get_Cols() - 1))
				{
					Select(row, 1, COleVariant(row), COleVariant(get_Cols()-1));
				}
			}

			if (CanBeNew(GetTblType()))
				m_bCanNewData = TRUE;
		}
		else
		{
			// ��Ϊ��������Ҳ���ѡ�������һ���ѡ�е�Ԫ��
			if (0 == m_lclickCol || !(2 == Button && MouseInSelection()))
			{
				Select(row, col, COleVariant(row), COleVariant(col));
			}

			if (CanBeNew(GetTblType()))
				m_bCanNewData = FALSE;
		}

		// �������Ա���ʾ
		if (row > 0 && row <= Row_DataToGrid(m_FlexDataSource.GetGridRowListSize()-1))
		{
			const long lParam = m_FlexDataSource.GetIdByIndex(Row_GridToData(row));
			if (lParam > 0)
				::SendMessage(theApp.g_hMainFrm, IDU_UPDATE_MODIFY_VIEW, GetTblType(), lParam);	
		}

		m_lClickRow = row;
		m_lclickCol = col;
	}
}

void CDataGridCtrl::MouseUpVsflexgrid( short Button, short Shift, float X, float Y )
{
	if (1 != Button && 2 != Button) // ֻ�������������Ҽ��¼�
		return;

	if (m_bEditing)	// ���ڱ༭״̬������Ӧ����¼�
		return;

	const long row = get_MouseRow();
	const long col = get_MouseCol();
	if (!m_rtDlgMenu.IsRectNull())
	{
		if(BIZAPI::CheckLimit(GetTblType(), m_FlexDataSource.GetIdByIndex(Row_GridToData(m_rtDlgMenu.bottom))))
		{
			CellBorderRange(min(m_rtDlgMenu.top, m_rtDlgMenu.bottom), m_rtDlgMenu.left, 
				max(m_rtDlgMenu.top, m_rtDlgMenu.bottom), m_rtDlgMenu.right, 
				COLOR_Bom_Cancel_SelBkg1/*RGB(0, 0, 0)*/, 2, 2, 2, 2, 0, 0);
			TrackDlgMenu(m_rtDlgMenu.bottom);
			put_SelectionMode(flexSelectionFree);
			Select(0, 0, COleVariant((long)0), COleVariant((long)0));
		}
	}

	if (2 == Button)	// ����һ�
	{
		if (0 == row && 0 == col)	// ���Ͻ�
		{
			TrackRLeftUpMenu(X, Y);
		}
		else if (row > 0 && 0 == col) // ��һ��
		{
			TrackRFirstColMenu(X, Y);
		}
		else if (0 == row && col > 0) // ��ͷ
		{
			TrackRHeadMenu(X, Y);
		}
		else if (row > 0 && col > 0)	// ������
		{
			TrackRDataMenu(X, Y);
		}
	}
}

void CDataGridCtrl::MouseMoveVsflexgrid( short Button, short Shift, float X, float Y )
{
	if (0 != Button && 1 != Button && 2 != Button) // ֻ��������ƶ���ס������Ҽ��ƶ��¼�
		return;

	CString szTip = GetShowCode();
	m_toolTip.Update(szTip);
	m_bInAntoResize = FALSE;

	const long row = get_MouseRow();
	
	if (m_lclickCol > 0)	// ����Ĳ�Ϊ��һ��
	{
		// �����갴��ʱ��Ԫ������½�����
		const long lCPosY = get_RowPos(m_lClickRow) + get_RowHeight(m_lClickRow);
		const long lCPosX = get_ColPos(m_lclickCol) + get_ColWidth(m_lclickCol);
		if (0 == Button && GetEditContentFlg() && 
			(X > lCPosX && X < lCPosX+60 && Y > lCPosY-100 && Y < lCPosY))
		{
			::SetCursor(m_hCursor); // ����ʮ�ֹ��
			m_bCrossCursor = TRUE;
		}
		else
		{
			m_bCrossCursor = FALSE;
		}
	}

	if (1 == Button /*&& GetEditContentFlg() */&& !m_rtDlgMenu.IsRectNull() && !m_dlgMenu.GetSafeHwnd())
	{
		::SetCursor(m_hCursor); // ����ʮ�ֹ��

		// ���֮ǰ�ı߿�
		CellBorderRange(min(m_rtDlgMenu.top, m_rtDlgMenu.bottom), m_rtDlgMenu.left, 
			max(m_rtDlgMenu.top, m_rtDlgMenu.bottom), m_rtDlgMenu.right, 
			COLOR_Bom_Cancel_SelBkg/*RGB(255, 255, 255)*/, 0, 0, 0, 0, 0, 0);

		if (row < 0)
			m_rtDlgMenu.bottom = get_BottomRow();
		else if (row == 0)
			m_rtDlgMenu.bottom = get_TopRow();
		else
			m_rtDlgMenu.bottom = row;
		//m_rtDlgMenu.SetRect(m_lclickCol, m_lClickRow, m_lclickCol, row);
		CellBorderRange(min(m_rtDlgMenu.top, m_rtDlgMenu.bottom), m_rtDlgMenu.left, 
			max(m_rtDlgMenu.top, m_rtDlgMenu.bottom), m_rtDlgMenu.right, 
			COLOR_Bom_Cancel_SelBkg2/*RGB(130, 130, 130)*/, 2, 2, 2, 2, 0, 0);
	}
}


void CDataGridCtrl::DblClickVsflexgrid()
{
	const long row = get_MouseRow();
	const long col = get_MouseCol();

	if (row < 0 || col < 0)	// ˫�����ٱ������
		return;

	if (row == 0) // ��һ�У���ͷ
	{
		if (FALSE == m_bInAntoResize && col >= get_FixedCols())	// �Ƿ�����Ӧ���Ҳ��ڹ̶���
		{
			if(GetTblType() != TblMessageData && GetTblType() != TblLoadFileInfo)
			{
				m_FlexDataSource.CreateSortCondition(col);
				SortTable();
			}
		}
	}
	else if(row > 0) // �ڶ��п�ʼ�����ڵ�һ��
	{
		if(GetFlexGridType() == OUTLINEGRID)
		{
			CreatEditBoxForOutlineGrid(row, col);
		}
		else
		{
			if(col < 1)
			{
				Edit(row);
			}
			else
			{
				if (GetTblType() == TblMessageData &&
					m_FlexDataSource.GetEnumIdByIndex(col-1) == CMessageData::MessageData_messageCode)
				{
					::PostMessage(theApp.g_hMainFrm,ID_MESSAGE_MSGCONDLG,(YK_ULONG)m_FlexDataSource.GetIdByIndex(Row_GridToData(row)),0);
				}
			}
		}
	}

	m_bInAntoResize = FALSE;

}


void CDataGridCtrl::BeforeEditVsflexgrid(long Row, long Col, BOOL* Cancel)
{
	if (Col <= 0)	// ��һ�в��ɱ༭
	{
		*Cancel = TRUE;
		return;
	}
#ifdef LIMITFUNCTION
	if (BIZAPI::GetIsLimited() || !m_FlexDataSource.CheckLimit(Row_GridToData(Row), Col))	// Ȩ������
	{
		*Cancel = TRUE;
		return;
	}
#endif
	//m_befEditCode = get_TextMatrix(Row, Col);
	wstring comboText;

	//ȥ����һ�У������У�
	YKField& filedPara = GetEditType(Row_GridToData(Row), Col, comboText);
	bool b_ = ((FILEDPARA_ISMODIFY_TRUE == filedPara.m_modifyFly) && !(filedPara.HaveVirRule()));

	if(OUTLINEGRID == GetFlexGridType())		//��OutLine�ı��
	{
		const UINT uRowTblType = m_FlexDataSource.GetGridRowTblType(Row_GridToData(Row));
		if (TblNone != uRowTblType && GetTblType() != uRowTblType)
		{
			*Cancel = TRUE;
			return;
		}
	}

	if(Col > GetStTable().mode.size())
		return;

	if (TblOrder == GetTblType())
	{
		const long id = m_FlexDataSource.GetGridRowId(Row_GridToData(Row));
		if (id > -1)
		{

			{
				int index = GetStTable().mode[Col-1].mode;
				wstring odertype = m_FlexDataSource.GetItemValue(id, YKOrder::Order_Type,TblOrder).GetString();
				if(GetRemarkFiled(Order_Mainten) == odertype)		// ά������
				{
					if (index == YKOrder::Order_PudId ||
						index == YKOrder::Order_Amount ||
						index == YKOrder::Order_UserSpecifyAmount || 
						index == YKOrder::Order_ClientId)
					{
						b_ = false;
					}
				}
				else if (odertype == GetRemarkFiled(Order_Buy) ||		// �ɹ�����
					odertype == GetRemarkFiled(Order_Mainten) ||
					odertype == GetRemarkFiled(Order_Save_comp) ||
					odertype == GetRemarkFiled(Order_Save_Abs))
				{
					if (index == YKOrder::Order_OperSelector)	         //	ָ������ѡ����
					{
						b_ = false;
					}
				}
			}

		}
	}
	else if (TblManufactureCapitalInfor == GetTblType() && 
		YKManufactureCapitalInfor::ManufactureCapitalInfor_Depart == GetStTable().mode[Col-1].mode)
	{
		if(comboText.empty())		// û��ѡ��ܱ༭
		{
			put_ComboList(L"");
			return;
		}
	}
	if(!b_)
	{
		*Cancel = TRUE;
		return;
	}
	else if(filedPara.m_editType == FILEDPARA_EDITTYPE_EDIT)
	{
		put_ComboList(L"");
	}
	else if (filedPara.m_editType == FILEDPARA_EDITTYPE_POPUP)
	{
		put_ComboList(L"...");//����Ϊ��...����ť
		//put_DataType( flexDTDate);
	}
	else if (filedPara.m_editType == FILEDPARA_EDITTYPE_POPUP_EDIT)
	{
		put_ComboList(L"|...");//�ɱ�д������
	}
	else if (filedPara.m_editType == FILEDPARA_EDITTYPE_COMBO_TEXTIN)
	{
		put_ComboList(comboText.c_str());		// ���Ա༭�������б�
	}
	else
	{
		put_ComboList(comboText.c_str());
		if(comboText.empty())		// û��ѡ��ܱ༭
			*Cancel = TRUE;
	}
}

void CDataGridCtrl::AfterEditVsflexgrid( long Row, long Col )
{
	if (m_FlexDataSource.IsKeyCol(Col-1))
	{
		if (Row == get_Rows()-1)
		{
			CString cstrCode = m_FlexDataSource.GetNewCode();
			wstring wstrcode = cstrCode.GetString();
			TOOLS::RemoveBlank(wstrcode);
			cstrCode = wstrcode.c_str();
			if (!cstrCode.IsEmpty())
			{
				YKFieldPtr temppara = BIZAPI::GetBizMap()->GetFieldPara(GetTblType(), GetStTable().mode[Col-1].mode);
				if (temppara->m_editType == FILEDPARA_EDITTYPE_COMBO_CODE)
				{
					if (cstrCode == WORD_ASTERISK)
					{
					}
					else
					{
						unsigned long tempId = 0;
						map<wstring,long>::iterator i_find = m_FlexDataSource.m_codeList.find(cstrCode.GetString());
						if (i_find != m_FlexDataSource.m_codeList.end())
						{
							tempId = i_find->second;
						}
						else
						{
							tempId = 0;
							return;
						}
						cstrCode.Format(L"%ld",tempId);
					}
				}
				else if ( temppara->m_editType == FILEDPARA_EDITTYPE_COMBO_TEXTIN )
				{
					if (GetTblType() == TblManufactureCapitalInfor )
					{
						unsigned long tempId = 0;
						map<wstring,long>::iterator i_find = m_FlexDataSource.m_codeList.find(cstrCode.GetString());
						if (i_find != m_FlexDataSource.m_codeList.end())
						{
							tempId = i_find->second;
						}
						else
						{
							tempId = BIZAPI::NewItemClassByTbl(TblDepart,cstrCode.GetString());
						}
						cstrCode.Format(L"%ld",tempId);
					}
				}
				m_FlexDataSource.SetNewCode(L"");

// 				if(CDataSave::CheckKeyCode(GetTblType(), wstrcode) == EOR_TYPE_REPEAT)
// 				{
// 					theApp.m_errorVar.vt = VT_BSTR;
// 					theApp.m_errorVar.bstrVal = cstrCode.AllocSysString();
// 					theApp.ShowDataErrorView(GetTblType(),EOR_TYPE_REPEAT,Row,GetStTable().mode[Col-1].mode,GetStTable().GetName());
// 					return;
// 				}
				OnNewItemClass(cstrCode);
			}
		}
	}
	if (GetTblType() == TblSchSysParam)
	{
		int index = GetStTable().mode[Col-1].mode;
		if (index == YKSchSysParam::SchSysParam_TimeShowType)
		{
			::SendMessage(theApp.g_hMainFrm, IDU_UPDATE_MODIFY_WACTH, GetTblType(), 0);
		}
	}
}
void CDataGridCtrl::ValidateEditVsflexgrid( long Row, long Col, BOOL* Cancel )
{
	//// �л��˱�ǩ��ȡ���༭ģʽ
	//if (m_uTabNameIndex != GetStTable().GetNameIndex())
	//	return;

	//CString cstr = get_EditText();

	//m_FlexDataSource.SetDataEditType(1);

	//BIZAPI::BeginRebackOper();		// ����һ��������Ϣ��ջ
	//RecordInfo reinfo;
	//reinfo.id = m_FlexDataSource.GetGridRowId(Row_GridToData(Row));
	//reinfo.operType = OPERTYPEMODIFY;
	//BIZAPI::InsertRelDataStruct(GetTblType(),reinfo);

	//m_FlexDataSource.SetEditModeFlg(true);  //�༭���޸�����ģʽ

	//m_FlexDataSource.ResetEditErrorFlg();
	//m_FlexDataSource.OnItemChanged(Row-1,Col-1,cstr.AllocSysString());

	//m_FlexDataSource.SetEditModeFlg(false);  //�༭���޸�����ģʽ
	//// У�����
	//if(m_FlexDataSource.IsEditErrorFlg())
	//{
	//	m_FlexDataSource.InsertError(Row,Col,cstr);
	//	*Cancel = TRUE;							// ����ΪTRUE �ù�걣�����û�¼���������λ�� 
	//	BIZAPI::DelRebackOper();				// û�б��� ������ǰ�ĳ�����Ϣ��ջ  �ͷŵ�ǰ�ĳ�����Ϣ 
	//}
	//else
	//{
	//	m_FlexDataSource.EraseError(Row,Col);
	//	BIZAPI::EndRebackOper();				// �رտ����ĳ�����Ϣ
	//}

	//ReDrawScreen(Row);

	if(Col > m_FlexDataSource.m_table.mode.size())
		return;

	unsigned long  tableName = m_FlexDataSource.m_table.tblType;							//��
	unsigned long id = m_FlexDataSource.GetIdByIndex(Row_GridToData(Row));					//����ID
	unsigned long col = m_FlexDataSource.m_table.mode[Col-1].mode;							//��
	//Add 2011-11-1
	// �л��˱�ǩ��ȡ���༭ģʽ
	if (m_uTabNameIndex != GetStTable().GetNameIndex())
		return;

	if (m_FlexDataSource.GetGridRowId(Row_GridToData(Row)) > -1 || 0 == col)	// ����½��еڶ��п��½�����
	{
		CString cstr = get_EditText();
		wstring wstr = cstr.GetString();

		m_FlexDataSource.SetDataEditType(1);

		m_FlexDataSource.SetEditModeFlg(true);  //�༭���޸�����ģʽ

		bool checkValue = BIZAPI::TableColsValueJudage(tableName,id,col,wstr);
		if (checkValue && cstr.Find(L"\n")==-1)
		{
			m_FlexDataSource.EraseError(Row, Col);

			if (id > 0)
			{
				BIZAPI::BeginUndoRedo();
				DataModify(tableName,id,col,wstr);
				BIZAPI::EndUndoRedo();
			}
		}
		else
		{
			m_FlexDataSource.InsertError(Row,Col,cstr);
			*Cancel = TRUE;							// ����ΪTRUE �ù�걣�����û�¼���������λ�� 
		}
		m_bEditing = (!checkValue);
		m_FlexDataSource.SetEditModeFlg(false);  //�༭���޸�����ģʽ
	}

	ReDrawScreen(Row);
}

//����Ԫ��..."ʱ�����Ի�����Ϣ��Ӧ
void CDataGridCtrl::CellButtonClickVsflexgridu1(long Row, long Col)
{
	if (Col > 0 && Row > 0 && Col < get_Cols() && Row < get_Rows())
	{
		wstring comboText;
		m_FlexDataSource.SetEditModeFlg(false);  //�������޸�����ģʽ
		const YKField& filedPara = GetEditType(Row_GridToData(Row), Col, comboText);
		if(filedPara.m_editType == FILEDPARA_EDITTYPE_POPUP || 
		   filedPara.m_editType == FILEDPARA_EDITTYPE_POPUP_EDIT)
		{
			const UINT id = m_FlexDataSource.GetIdByIndex(Row_GridToData(Row));
			if (id > 0)
			{
				GridDlg(Col - 1, Row,GetTblType(), id, GetStTable(),filedPara.m_chekType);
			}	
		}
		m_FlexDataSource.SetEditModeFlg(true) ; //�������޸�����ģʽ
		put_FlexDataSource((LPDISPATCH)&m_FlexDataSource);
	}
}
void CDataGridCtrl::BeforeCollapseVsflexgrid( long Row, short State, BOOL* Cancel )
{
	//��ʱ����Ӧһ�ε���򿪶���ڵ㹦��
	if(Row == -1)
		*Cancel = TRUE;
}

void CDataGridCtrl::AfterCollapseVsflexgrid( long Row, short State )
{
	if (m_bReDraw)
		return;

	long index = Row_GridToData(Row);
	m_FlexDataSource.SetGridRowState(index, State);

	BOOL bShow = FALSE;
	const int nOldRLevel = m_FlexDataSource.GetGridRowLevel(index);
	int nCurRLevel = m_FlexDataSource.GetGridRowLevel(++index);
	while (nCurRLevel > -1 && nCurRLevel > nOldRLevel)
	{
		if ((nOldRLevel+1) == nCurRLevel)
		{
			switch (State)
			{
			case flexOutlineExpanded:
				bShow = TRUE;
				break;
			case flexOutlineCollapsed:
				bShow = FALSE;
				break;
			}
			m_FlexDataSource.SetGridRowShowFlag(index, bShow);
		}

		nCurRLevel = m_FlexDataSource.GetGridRowLevel(++index);
	}

	ReDrawScreen(Row);
}

void CDataGridCtrl::BeforeScrollVsflexgridu1(long OldTopRow, long OldLeftCol, long NewTopRow, long NewLeftCol, BOOL* Cancel)
{
	m_lOldTopRow = get_TopRow();
	m_lOldBottomRow = get_BottomRow();
}


void CDataGridCtrl::AfterScrollVsflexgrid( long OldTopRow, long OldLeftCol, long NewTopRow, long NewLeftCol )
{
	if (m_bReDraw)
		return;

	m_lCurLeftCol = NewLeftCol;
	m_lCurTopRow = NewTopRow;
	const long bottomRow = get_BottomRow();

	if (NewTopRow > OldTopRow)
	{
		if (m_lOldBottomRow >= NewTopRow)
			ReDrawScreen(m_lOldBottomRow);
		else
			ReDrawScreen(NewTopRow);
	}
	else
	{
		if (bottomRow > m_lOldTopRow)
			ReDrawScreen(m_lOldTopRow, FALSE);
		else
			ReDrawScreen(NewTopRow);
	}

	// �Ƿ�Ҫ�ƶ�dlgMenu
	if (m_dlgMenu.GetSafeHwnd())
	{
		if (OldTopRow != NewTopRow)	// ��ֱ����
		{
			const long tRow = min(m_rtDlgMenu.top, m_rtDlgMenu.bottom);
			const long bRow = max(m_rtDlgMenu.top, m_rtDlgMenu.bottom);

			if (tRow > bottomRow || bRow < NewTopRow) // ������ʾ��������m_dlgMenu
			{
				if (m_dlgMenu.IsWindowVisible())
					m_dlgMenu.ShowWindow(SW_HIDE);
			}
			else // ����ʾ�����ж��Ƿ�Ҫ�ƶ��˵���
			{
				if (!m_dlgMenu.IsWindowVisible())
					m_dlgMenu.ShowWindow(SW_SHOW);

				if (tRow > NewTopRow + m_lDlgMenuSpace)
				{
					MoveDlgMenu(tRow);
				}
				else if (bRow < NewTopRow + m_lDlgMenuSpace)
				{
					MoveDlgMenu(bRow);
				}
			}
		}
		else	// ˮƽ����
		{
			MoveDlgMenu(NewTopRow + m_lDlgMenuSpace);
		}
	}
}

void CDataGridCtrl::BeforeMoveColumnVsflexgrid( long Col, long* Position )
{
	const long lForzenCol = get_FrozenCols();
	if (Col <= lForzenCol || (*Position) <= lForzenCol)
	{
		*Position = Col;
	}
}

void CDataGridCtrl::AfterMoveColumnVsflexgrid( long Col, long* Position )
{
	if(Col == (*Position))
		return;
	
	stTable& curTable = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());
	DATATABLAPI::GoTostTblPos(curTable,Col-1,(*Position)-1);
	SetStTable(curTable);

	ResertDataSource();
	RedrawWindow();
}

void CDataGridCtrl::BeforeUserResizeVsflexgridu1(long Row, long Col, BOOL* Cancel)
{
	// TODO: Add your message handler code here
	m_bInAntoResize = TRUE;
}


void CDataGridCtrl::AfterUserResizeVsflexgridu1(long Row, long Col)
{
	// TODO: Add your message handler code here
	if(Col >= 1 && Col <= GetStTable().mode.size())
	{
		const int width = get_ColWidth(Col);//�õ����еĿ��
		stTable &stTbl = DATATABLAPI::GetstTble(GetTblType(), GetStTable().GetNameIndex());//�õ��˱��stTable�ṹ
		stTbl.mode[Col-1].width = width;//�޸ı��п�
		GetStTable().mode[Col-1].width = width;//�޸ı��п�
	}


	UpdateClientRowCount();
	ReDrawScreen(get_TopRow());

// 	if(GetTblType() == TblMessageData)
// 		AdaptView(Col);
}

void CDataGridCtrl::DataModify(unsigned int tblType,int id,int col,wstring& var)
{
	m_FlexDataSource.DataModify(tblType,id,col,var, true);	
}

bool CDataGridCtrl::IsWorkOrOrderRow( long mouseUpRow )
{
	bool bOK = false;

	UINT tbl = m_FlexDataSource.GetGridRowTblType(Row_GridToData(mouseUpRow));
	if(tbl == TblWork || tbl == TblOrder)
	{
		bOK = true;
	}

	return bOK;
}