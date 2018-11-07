// SchedulerDoc.cpp : implementation of the CSchedulerDoc class
//

#include "stdafx.h"
#include "Scheduler.h"

#include "SchedulerDoc.h"
//#include "TblInfo.h"
#include  "Biz_API.h"
#include "LanguagePack.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSchedulerDoc

IMPLEMENT_DYNCREATE(CSchedulerDoc, CDocument)

BEGIN_MESSAGE_MAP(CSchedulerDoc, CDocument)
END_MESSAGE_MAP()


// CSchedulerDoc construction/destruction

CSchedulerDoc::CSchedulerDoc()
{
	// TODO: add one-time construction code here

}

CSchedulerDoc::~CSchedulerDoc()
{

	//theApp.g_viewTips.Delete(1);
}

BOOL CSchedulerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	
	ViewTip vt;
	if (theApp.g_viewTips.Find(theApp.g_tblType,vt))
	    //CDocument::SetTitle(vt.name.c_str());
		CDocument::SetTitle(GetRemarkFiled(theApp.g_tblType).c_str());
	else
		CDocument::SetTitle(L"");

	
	return TRUE;
}




// CSchedulerDoc serialization

void CSchedulerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CSchedulerDoc diagnostics

#ifdef _DEBUG
void CSchedulerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSchedulerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CSchedulerDoc commands

void CSchedulerDoc::OnChangedViewList()
{
	// TODO: Add your specialized code here and/or call the base class

	CDocument::OnChangedViewList();
}
