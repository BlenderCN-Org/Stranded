// YKSchedulerDoc.cpp : implementation of the CYKSchedulerDoc class
//

#include "stdafx.h"
#include "YKScheduler.h"

#include "YKSchedulerDoc.h"
//#include "TblInfo.h"
#include  "Biz_API.h"
#include "LanguagePack.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CYKSchedulerDoc

IMPLEMENT_DYNCREATE(CYKSchedulerDoc, CDocument)

BEGIN_MESSAGE_MAP(CYKSchedulerDoc, CDocument)
END_MESSAGE_MAP()


// CYKSchedulerDoc construction/destruction

CYKSchedulerDoc::CYKSchedulerDoc()
{
	// TODO: add one-time construction code here

}

CYKSchedulerDoc::~CYKSchedulerDoc()
{

	//theApp.g_viewTips.Delete(1);
}

BOOL CYKSchedulerDoc::OnNewDocument()
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




// CYKSchedulerDoc serialization

void CYKSchedulerDoc::Serialize(CArchive& ar)
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


// CYKSchedulerDoc diagnostics

#ifdef _DEBUG
void CYKSchedulerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CYKSchedulerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CYKSchedulerDoc commands

void CYKSchedulerDoc::OnChangedViewList()
{
	// TODO: Add your specialized code here and/or call the base class

	CDocument::OnChangedViewList();
}
