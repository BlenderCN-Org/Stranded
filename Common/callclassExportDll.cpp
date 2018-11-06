// callclassExportDll.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
using namespace std;
#include "../../classExportDll/classExportDll/IKPerson.h"
int _tmain(int argc, _TCHAR* argv[])
{
	HMODULE hDll = ::LoadLibrary(_T("classExportDll.dll"));
	if (NULL != hDll)
	{
		PFNGetIKPerson pFun = (PFNGetIKPerson)::GetProcAddress(hDll, "GetIKPerson");
		if (NULL != pFun)
		{
			IKPerson* pIKPerson = (*pFun)();
			if (NULL != pIKPerson)
			{
				pIKPerson->SetOld(103);
				pIKPerson->SetName("liyong");
				cout << pIKPerson->GetOld() << endl;
				cout << pIKPerson->GetName() << endl;
				delete pIKPerson;
			}
		}
		::FreeLibrary(hDll);
	}
	return 0;
}
