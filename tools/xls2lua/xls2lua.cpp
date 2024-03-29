// xls2lua.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CSpreadSheet.h"
#include <fstream>
#include <Shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using namespace std;

void convertXls2Lua();

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			convertXls2Lua();
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}

void convertXls2Lua()
{
	printf("Excel file(xls) convert lua tool, Version 0.1.0\n\n");
	printf("********************************************************************\n");
	printf("本工具使用注意事项：\n");
	printf("1、请将需要转成lua配置的excel文件放到工具所在目录下的excel目录。\n");
	printf("2、每个xls文件只支持转第一个sheet，并且命名为Sheet1。\n");
	printf("3、Excel配置表第一行为字段名，第二行为字段解析，从第三行开始为数据。\n");
	printf("4、字段解析不导出，如果第一行字段名为空，则该字段不导出到lua。\n");
	printf("5、导出的lua文件在本目录下的lua文件夹里。\n");
	printf("6、表头字段名不能以大写字母开头后跟数字，如F1、F2，这是Excel的关键字。\n");
	printf("7、数据格子读到空值则中断该表的导出，因此第一列不能配置空值。\n");
	printf("8、如果提示缺少驱动程序，请完整安装Office任意版本。\n\n\n");
	printf("********************************************************************\n");

	printf("开始导出文件，请稍等！\n");

	CFileFind finder; 
	CString strFile, strTitle, strOutFile, strTemp; 

	// 获取当前程序目录
	TCHAR lpFileName[MAX_PATH];
	::GetModuleFileName(NULL, lpFileName, MAX_PATH);
	::PathRemoveFileSpec(lpFileName);
	
	// 是否有xls目录
	TCHAR lpExcelFileName[MAX_PATH];
#pragma warning(push)
#pragma warning(disable:4996)
	strcpy(lpExcelFileName, lpFileName);
	strcat(lpExcelFileName, "\\excel\\");
#pragma warning(pop)
	if ((::GetFileAttributes(lpExcelFileName)) == INVALID_FILE_ATTRIBUTES)
		::CreateDirectory(lpExcelFileName, NULL);

	// 是否有lua目录
	TCHAR lpLuaFileName[MAX_PATH];
#pragma warning(push)
#pragma warning(disable:4996)
	strcpy(lpLuaFileName, lpFileName);
	strcat(lpLuaFileName, "\\lua\\");
#pragma warning(pop)
	if ((::GetFileAttributes(lpLuaFileName)) == INVALID_FILE_ATTRIBUTES)
		::CreateDirectory(lpLuaFileName, NULL);

	BOOL bWorking = finder.FindFile("excel\\*.xls*"); 
	while(bWorking) 
	{ 
		bWorking = finder.FindNextFile(); 
		strFile = finder.GetFilePath();
		strTitle = finder.GetFileTitle();		

		CSpreadSheet sheet(strFile, "Sheet1", false);
		sheet.BeginTransaction();

		short tCol = sheet.GetTotalColumns();
		long tRow = sheet.GetTotalRows();
		if (tCol <= 0 || tRow <= 3)
		{
			continue;
		}
		
		ofstream outputFile;
		strOutFile.Format("lua\\%s.lua", strTitle);
		outputFile.open(strOutFile, std::ios::out | std::ios::trunc);
		
		outputFile << "--[[" << strTitle << ".lua\n";
		outputFile << "--]]" << "\n\n";

		outputFile << "local " << strTitle << "= \n{\n";
		CStringArray headerArray, dataArray;
		sheet.ReadRow(headerArray, 1);
		for (int i = 3; i <= tRow; ++i)
		{
			sheet.ReadRow(dataArray, i);
			if(dataArray[0] == "") 
			{
				//如果首列读到空值，则停止导出
				break;
			}
			outputFile << "\t{ ";
			for(int j = 0; j < tCol; ++j)
			{
				//未配置表头的字段以及数据为空的字段不导出
				strTemp.Format("F%d", j+1);
				if(headerArray[j] == strTemp || dataArray[j] == "") 
				{
					continue;
				}				
				//判断是字符串还是数字，字符串则加上引号
				int pos = dataArray[j].FindOneOf(".");
				outputFile << headerArray[j] << " = ";
				if (pos <= 0) outputFile << "'";
				outputFile << dataArray[j];
				if (pos <= 0) outputFile << "'";
				outputFile << ", ";
			}
			outputFile << "},\n";
		}	
		outputFile << "}\n";
		outputFile << "return " << strTitle << ";\n";
		outputFile.flush();
		outputFile.close();

		printf("%s导出成功！\n", (LPCTSTR)strFile);
	}
	printf("\n导出所有xls到lua成功!\n\n");
	printf("请按任意键退出程序!\n");
	getchar();
}
