#include "WebHttp.h"
#include <iostream>
#include <fstream>

void StringTrim(std::string& strOri)
{
	if (strOri.empty())
		return;

	int i = 0;
	while (strOri[i] == ' ' || strOri[i] == '\t' || strOri[i] == '\r' || strOri[i] == '\n')
		i++;

	strOri = strOri.substr(i);

	if (strOri.empty())
		return;

	i = strOri.size() - 1;
	while (strOri[i] == ' ' || strOri[i] == '\t' || strOri[i] == '\r' || strOri[i] == '\n')
		i--;
	strOri = strOri.substr(0, i + 1);
}

std::string SearchOneUrl(const char* pUrl)
{
	CWebHttp web;

	if (web.SetCrackUrl(pUrl))
	{
		if (web.ConnectWeb())
		{
			if (web.OpenRequest("GET"))
			{
				return web.ReadGETData();
			}
		}
	}

	return "";
}

bool HandleRetData(const char* pFileName, std::string strData)
{
	FILE* fp = _tfopen(pFileName, _T("a+"));
	if (fp)
	{
		bool bIsFind = false;

		std::string::size_type nStartPos = strData.find("summary");
		if (nStartPos != std::string::npos)
		{
			fseek(fp, 0, SEEK_END);
			int iFileLen = ftell(fp);
			if (iFileLen <= 0)
			{
				_ftprintf(fp, "��������,��ַ,����,�绰");
				_ftprintf(fp, "\n");
			}

			//std::string strTotalData;
			std::string strOneName;
			std::string strTempName;
			std::string strSlash;
			
			
			// Ѱ�� /shop
			std::string::size_type nShopPos = strData.find("/shop", nStartPos + 7);
			while (nShopPos != std::string::npos)
			{
				if (!bIsFind)
					bIsFind = true;

				strOneName.clear();

				// Ѱ�� alt="���ܼҾ������"
				std::string::size_type nAltPos = strData.find("alt", nShopPos + 5);

				// �ҵ���������
				std::string::size_type nAltEndPos = strData.find('\"', nAltPos + 5);
				strTempName = strData.substr(nAltPos + 5, nAltEndPos - (nAltPos + 5));

				strOneName = strTempName;
				strOneName += ",";

				// �ҵ���ַ
				std::string::size_type nAddressPos = strData.find("address", nAltEndPos+1);
				std::string::size_type nAddressEndPos = strData.find("<", nAddressPos + 9);
				strTempName = strData.substr(nAddressPos + 9, nAddressEndPos - (nAddressPos + 9));

				StringTrim(strTempName);

				strOneName += strTempName;
				strOneName += ",";

				// Ѱ�����ֺ͵绰�Ĵ�
				std::string::size_type nNamePos = strData.find("<", nAddressEndPos + 4);
				std::string::size_type nNameEndPos = strData.find("<", nNamePos + 3);
				strTempName = strData.substr(nNamePos + 3, nNameEndPos - (nNamePos + 3));

				StringTrim(strTempName);

				std::string::size_type nSlashPos = strTempName.find("/");
				strSlash = strTempName.substr(0, nSlashPos);
				StringTrim(strSlash);

				strOneName += strSlash;
				strOneName += ",";

				strSlash = strTempName.substr(nSlashPos+1);

				strOneName += strSlash;

				_ftprintf(fp, strOneName.c_str());
				_ftprintf(fp, "\n");
			
				nShopPos = strData.find("/shop", nNameEndPos);

			}

		}
		
		fclose(fp);

		return bIsFind;
	}

	return false;
}

int main(int argc, char* argv[])
{
	printf("\r\n ��ʼ����������ȴ���...... \r\n");
	
	// ��ȡ��ǰ����Ŀ¼
	char lpFileName[MAX_PATH];
	if (SUCCEEDED(::GetModuleFileName(NULL, lpFileName, MAX_PATH)))
	{
		BOOL bRet = ::PathRemoveFileSpec(lpFileName);
		if (bRet == FALSE)
		{
			printf("\r\n [CWebHttp::main()] PathRemoveFileSpec error! code: [%d] \r\n", ::GetLastError());
			return false;
		}

		char lpHtmlFileName[MAX_PATH];
		char lpNameFilename[MAX_PATH];
#pragma warning(push)
#pragma warning(disable:4996)
		strcpy(lpHtmlFileName, lpFileName);
		strcat(lpHtmlFileName, "\\");
		strcat(lpHtmlFileName, "www.txt");
#pragma warning(pop)

		std::ifstream fin(lpHtmlFileName, std::ios::in);
		if(fin)
		{
			char szTempBuf[256] = { 0 };
			std::string strBuf;
			std::string strCity;
			int iUrlIdx = 1;
			while (getline(fin, strBuf))
			{
				// ��ȡ����
				std::string::size_type nCityPos = strBuf.find("//");
				std::string::size_type nCityEndPos = strBuf.find(".", nCityPos + 2);
				strCity = strBuf.substr(nCityPos + 2, nCityEndPos - (nCityPos + 2));

				memset(lpNameFilename, 0, MAX_PATH);

				strcpy(lpNameFilename, lpFileName);
				strcat(lpNameFilename, "\\");
				strcat(lpNameFilename, strCity.c_str());
				strcat(lpNameFilename, "_name.csv");

				iUrlIdx = 1;
				// ����ÿһ��վ��
				for (;;)
				{
					sprintf(szTempBuf, "%s/category/all/page/%d", strBuf.c_str(), iUrlIdx);
					
					printf("\r\n ������ַ: [%s] \r\n", szTempBuf);
					
					std::string strRet = SearchOneUrl(szTempBuf);
					if (strRet.empty())
					{
						printf("\r\n [SearchOneUrl()] ��ַ������Ϊ��! \r\n");
						break;
					}
					else
					{
						bool bIsfind = HandleRetData(lpNameFilename, strRet);
						if (!bIsfind)
						{
							printf("\r\n [SearchOneUrl()] ��ַ���������� [%s]! \r\n", strBuf.c_str());
							break;
						}
					}
					
					iUrlIdx++;

					Sleep(2000);
				}

				Sleep(1000);
			}
		}
	}

	printf("\r\n ������ɣ����������ļ��� \r\n");

	return 0;
}