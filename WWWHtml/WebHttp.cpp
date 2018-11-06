#include "WebHttp.h"

#define USE_WINHTTP
#ifdef USE_WINHTTP
#pragma comment(lib, "Winhttp.lib")
#else
#pragma comment(lib, "Wininet.lib")
#endif


#pragma comment(lib, "../lib/Common.lib")

#define NL					_T("\n")	// new line

static const char* S_AGENT_NAME = "Browser 1.0";
static const int BUF_SIZE = 4096;

CWebHttp::CWebHttp() :
	m_hSession(NULL),
	m_hConnection(NULL),
	m_hRequest(NULL)
{
	if (!InitSession())
		exit(-1);
}

CWebHttp::~CWebHttp()
{
	ShutDown();
}

bool CWebHttp::InitSession()
{
#ifdef USE_WINHTTP
	std::wstring wstrAgent = LocalToUnicode(S_AGENT_NAME);
	m_hSession = ::WinHttpOpen(wstrAgent.c_str(),	// NULL - no agent string

		NULL,										// �Ự�������: WINHTTP_ACCESS_TYPE_NO_PROXY (ָʾ���������ʹ�ô������ã� �����ڴ����Ự�󸲸���)��
													// WINHTTP_ACCESS_TYPE_DEFAULT_PROXY (ָʾӦ��ʹ�� WinHTTP ��ע����д洢�����д�������, �˲����������Ա���ƴ������ã����һ������� Internet Explorer �Ĵ�������.
													// ָʾӦ��ʹ�� WinHTTP ��ע����д洢�����д������á��˲����������Ա���ƴ������ã�
													// ���һ������� Internet Explorer �Ĵ������ã����������ķ�����������
													// �� Windows XP �� Windows Server 2003 �У�����ʹ�� proxycfg.exe ʵ�ù��������úͲ�ѯ WinHTTP �������á�
													// �� Windows Vista �� Windows Server 2008 �У�netsh.exe ʵ�ù�������� proxycfg.exe ʵ�ù����ṩ��Ӧ�Ĺ��ܡ�
													// ��������ʹ�� WinHttpSetDefaultProxyConfiguration ����ֱ������ WinHTTP �������ã���ʹ�� WinHttpGetDefaultProxyConfiguration �������в�ѯ��
													// ��Щ������������������Ȼ��Ч���������е�¼�Ự���á�������������ʹ�� WINHTTP_PROXY_INFO �ṹ���ýṹֱ��ӳ�䵽 WinHttpOpen �������������ص�����������)��
													// WINHTTP_ACCESS_TYPE_NAMED_PROXY (ָʾ WinHTTP Ӧ���ڽ�����������������ʹ�ô�������).

		NULL,										// ָ����������������� (L"http://127.0.0.1:61497")
		NULL,										// ָ��һ�� HTTP ��������ѡ�б����еķ���������ͨ����һ������ָ���Ĵ������������·�ɡ�(L"<local>")
		NULL);
#else
	// Initialize the Win32 internet functions
	m_hSession = ::InternetOpen(S_AGENT_NAME,
		INTERNET_OPEN_TYPE_PRECONFIG,		// use registry settings.
		NULL,								// proxy name, NULL indicates use default.
		NULL,								// list of local serers. NULL indicates default.
		0);
#endif

	if (m_hSession == NULL)
	{
		DWORD dwError = ::GetLastError();
		printf("\r\n [CWebHttp::CWebHttp()] open session error! code: [%d] \r\n", dwError);
		return false;
	}

	return true;
}

void CWebHttp::ShutDown()
{
	if (m_hRequest)
	{
#ifdef USE_WINHTTP
		::WinHttpCloseHandle(m_hRequest);
#else
		::InternetCloseHandle(m_hRequest);
#endif
	}


	if (m_hConnection)
	{
#ifdef USE_WINHTTP
		::WinHttpCloseHandle(m_hConnection);
#else
		::InternetCloseHandle(m_hConnection);
#endif
	}


	// close the session
	if (m_hSession)
	{
#ifdef USE_WINHTTP
		::WinHttpCloseHandle(m_hSession);
#else
		::InternetCloseHandle(m_hSession);
#endif
	}
}

void CWebHttp::Clear()
{

}

bool CWebHttp::SetCrackUrl(const char* pUrl)
{
	std::wstring wstrUrl = LocalToUnicode(pUrl);
	
	return m_crachUrl.SetUrl(wstrUrl.c_str());
}

bool CWebHttp::ConnectWeb()
{
	if (m_hSession)
	{
		
#ifdef USE_WINHTTP
		m_hConnection = ::WinHttpConnect(m_hSession, m_crachUrl.GetHostName().c_str(), (INTERNET_PORT)m_crachUrl.GetPort(), 0);
#else
		std::string strHost = UnicodetoLocal(m_crachUrl.GetHostName().c_str());
		m_hConnection = ::InternetConnect(m_hSession, strHost.c_str(), (INTERNET_PORT)m_crachUrl.GetPort(), NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
#endif

		if (m_hConnection == NULL)
		{
			DWORD dwError = ::GetLastError();
			printf("\r\n [CWebHttp::ConnectWeb()] WinHttpConnect error! code: [%d] \r\n", dwError);
			return false;
		}

		return true;
	}

	return false;
}

bool CWebHttp::OpenRequest(const char* pType, const char* pPostData, const char* pHeader)
{
	if (m_hConnection)
	{
		DWORD flags = 0;
#ifdef USE_WINHTTP
		if ((INTERNET_SCHEME)m_crachUrl.GetScheme() == INTERNET_SCHEME_HTTPS)
			flags |= WINHTTP_FLAG_SECURE;

		std::wstring wstrVerb = LocalToUnicode(pType);
		m_hRequest = ::WinHttpOpenRequest(m_hConnection, wstrVerb.c_str(), m_crachUrl.GetPath().c_str(), NULL, NULL, NULL, flags);
#else
		if ((INTERNET_SCHEME)m_crachUrl.GetScheme() == INTERNET_SCHEME_HTTPS)
			flags |= INTERNET_FLAG_SECURE;

		std::string strObjectName = UnicodetoLocal(m_crachUrl.GetPath().c_str());
		m_hRequest = ::HttpOpenRequest(m_hConnection, pType, strObjectName.c_str(), NULL, NULL, NULL, flags, 0);
#endif

		if (m_hRequest == NULL)
		{
			DWORD dwError = ::GetLastError();
			printf("\r\n [CWebHttp::OpenRequest()] WinHttpOpenRequest error! code: [%d] \r\n", dwError);
			return false;
		}

		////////////////////////////////////// POST ��ʽ /////////////////////////////////
		if (strcmp(pType, "POST") == 0)
		{
			// Add request header.
			// wstr Header: "Content-type: application/x-www-form-urlencoded\r\n"
			
			std::wstring wstrHeader = LocalToUnicode(pHeader);
			SIZE_T len = lstrlenW(wstrHeader.c_str());
			BOOL bRet = FALSE;
#ifdef USE_WINHTTP
			bRet = ::WinHttpAddRequestHeaders(m_hRequest, wstrHeader.c_str(), DWORD(len), WINHTTP_ADDREQ_FLAG_ADD);
#else
			bRet = ::HttpAddRequestHeaders(m_hRequest, pHeader, DWORD(len), HTTP_ADDREQ_FLAG_ADD);
#endif
			if (bRet == FALSE)
			{
				DWORD dwError = ::GetLastError();
				printf("\r\n [CWebHttp::OpenRequest()] WinHttpAddRequestHeaders error! code: [%d] \r\n", dwError);
				return false;
			}

			// Send post data.
			// str post date: "value1=10&value2=14"
#ifdef USE_WINHTTP
			len = strlen(pPostData);
			bRet = ::WinHttpSendRequest(m_hRequest, 0, 0, const_cast<void*>((const void*)pPostData), DWORD(len), DWORD(len), 0);
#else
			bRet = ::HttpSendRequest(m_hRequest, 0, 0, const_cast<void*>((const void*)pPostData), DWORD(len));
#endif
			if (bRet == FALSE)
			{
				DWORD dwError = ::GetLastError();
				printf("\r\n [CWebHttp::OpenRequest()] WinHttpSendRequest error! code: [%d] \r\n", dwError);
				return false;
			}

			// End request
#ifdef USE_WINHTTP
			bRet = ::WinHttpReceiveResponse(m_hRequest, 0);
#else
			// if you use HttpSendRequestEx to send request then use HttpEndRequest in here!
			bRet = TRUE;
#endif
			if (bRet == FALSE)
			{
				DWORD dwError = ::GetLastError();
				printf("\r\n [CWebHttp::OpenRequest()] WinHttpReceiveResponse error! code: [%d] \r\n", dwError);
				return false;
			}

			//////////////////////////////////////////////////////////////////////////
			char szBuf[BUF_SIZE] = { 0 };
			DWORD dwSize = 0;

#ifdef USE_WINHTTP
			int contextLengthId = WINHTTP_QUERY_CONTENT_LENGTH;
			int statusCodeId = WINHTTP_QUERY_STATUS_CODE;
			int statusTextId = WINHTTP_QUERY_STATUS_TEXT;
#else
			int contextLengthId = HTTP_QUERY_CONTENT_LENGTH;
			int statusCodeId = HTTP_QUERY_STATUS_CODE;
			int statusTextId = HTTP_QUERY_STATUS_TEXT;
#endif
			
			dwSize = BUF_SIZE;
#ifdef USE_WINHTTP
			bRet = ::WinHttpQueryHeaders(m_hRequest, (DWORD)contextLengthId, 0, szBuf, &dwSize, 0);
#else
			bRet = ::HttpQueryInfo(m_hRequest, (DWORD)contextLengthId, szBuf, &dwSize, 0);
#endif
			if (bRet == FALSE)
			{
				DWORD dwError = ::GetLastError();
				printf("\r\n [CWebHttp::OpenRequest()] WinHttpQueryHeaders WINHTTP_QUERY_CONTENT_LENGTH error! code: [%d] \r\n", dwError);
				return false;
			}

			dwSize = BUF_SIZE;
#ifdef USE_WINHTTP
			bRet = ::WinHttpQueryHeaders(m_hRequest, (DWORD)statusCodeId, 0, szBuf, &dwSize, 0);
#else
			bRet = ::HttpQueryInfo(m_hRequest, (DWORD)statusCodeId, szBuf, &dwSize, 0);
#endif
			if (bRet == FALSE)
			{
				DWORD dwError = ::GetLastError();
				printf("\r\n [CWebHttp::OpenRequest()] WinHttpQueryHeaders WINHTTP_QUERY_STATUS_CODE error! code: [%d] \r\n", dwError);
				return false;
			}

			dwSize = BUF_SIZE;
#ifdef USE_WINHTTP
			bRet = ::WinHttpQueryHeaders(m_hRequest, (DWORD)statusTextId, 0, szBuf, &dwSize, 0);
#else
			bRet = ::HttpQueryInfo(m_hRequest, (DWORD)statusTextId, szBuf, &dwSize, 0);
#endif
			if (bRet == FALSE)
			{
				DWORD dwError = ::GetLastError();
				printf("\r\n [CWebHttp::OpenRequest()] WinHttpQueryHeaders WINHTTP_QUERY_STATUS_TEXT error! code: [%d] \r\n", dwError);
				return false;
			}
		}
		//////////////////////////////////// GET��ʽ //////////////////////////////////////
		else
		{
			// Send a request.
			BOOL bRet = FALSE;
#ifdef USE_WINHTTP
			bRet = ::WinHttpSendRequest(m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
#else
			bRet = ::HttpSendRequest(m_hRequest, 0, 0, NULL, 0);
#endif
			if (bRet == FALSE)
			{
				DWORD dwError = ::GetLastError();
				printf("\r\n [CWebHttp::OpenRequest()] WinHttpSendRequest error! code: [%d] \r\n", dwError);
				return false;
			}

			// End request
#ifdef USE_WINHTTP
			bRet = ::WinHttpReceiveResponse(m_hRequest, 0);
#else
			// if you use HttpSendRequestEx to send request then use HttpEndRequest in here!
			bRet = TRUE;
#endif
			if (bRet == FALSE)
			{
				DWORD dwError = ::GetLastError();
				printf("\r\n [CWebHttp::OpenRequest()] WinHttpReceiveResponse error! code: [%d] \r\n", dwError);
				return false;
			}
		}

		return true;
	}

	return false;
}

std::string CWebHttp::ReadPOSTData()
{
	std::string strData;

	if (m_hRequest)
	{
		BOOL bRet = FALSE;
		DWORD dwSize = 0;
		DWORD dwDownloaded = 0;
		char szBuf[BUF_SIZE] = { 0 };
		for (;;)
		{
			dwSize = BUF_SIZE;
#ifdef USE_WINHTTP
			bRet = ::WinHttpReadData(m_hRequest, (void*)szBuf, dwSize, &dwDownloaded);
#else
			bRet = ::InternetReadFile(m_hRequest, (void*)szBuf, dwSize, &dwDownloaded);
#endif

			if (bRet == FALSE)
			{
				DWORD dwError = ::GetLastError();
				printf("\r\n [CWebHttp::ReadPOSTData()] WinHttpReadData error! code: [%d] \r\n", dwError);
				break;
			}

			if(dwDownloaded <= 0)
				break;

			szBuf[dwSize] = '\0';
			strData += szBuf;
		}
	}

	return strData;
}

std::string CWebHttp::ReadGETData()
{
	std::string strData;

	if (m_hRequest)
	{
		BOOL bRet = FALSE;
		DWORD dwDownloaded = 0;
		DWORD dwSize = 0;
		char* pszOutBuffer = nullptr;

		// Keep checking for data until there is nothing left.
		do 
		{
			// Check for available data.
			dwSize = 0;
#ifdef USE_WINHTTP
			bRet = ::WinHttpQueryDataAvailable(m_hRequest, &dwSize);
#else
			bRet = TRUE;
#endif
			if (bRet == FALSE)
			{
				DWORD dwError = ::GetLastError();
				printf("\r\n [CWebHttp::ReadGETData()] WinHttpQueryDataAvailable error! code: [%d] \r\n", dwError);
				break;
			}

			// Allocate space for the buffer.
			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer)
			{
				printf("\r\n [CWebHttp::ReadGETData()] Out of memory! \r\n");
				break;
			}
			else
			{
				// Read the Data.
				memset(pszOutBuffer, 0, sizeof(char)*(dwSize + 1));

#ifdef USE_WINHTTP
				bRet = ::WinHttpReadData(m_hRequest, (void*)pszOutBuffer, dwSize, &dwDownloaded);
#else
				bRet = ::InternetReadFile(m_hRequest, (void*)szBuf, dwSize, &dwDownloaded);
#endif

				if (bRet == FALSE)
				{
					DWORD dwError = ::GetLastError();
					printf("\r\n [CWebHttp::ReadGETData()] WinHttpReadData error! code: [%d] \r\n", dwError);
					break;
				}

				pszOutBuffer[dwSize] = '\0';
				strData += pszOutBuffer;

				delete[] pszOutBuffer;
				pszOutBuffer = nullptr;

				if (dwDownloaded <= 0)
					break;
			}

		} while (dwSize > 0);
	}
	
	if(!strData.empty())
	{
		std::string::size_type nPos = strData.find("charset=");
		if(nPos != std::string::npos)
		{
			std::string::size_type endPos = strData.find('\"', nPos + 9);
			if (endPos != std::string::npos)
			{
				std::string strCharset = strData.substr(nPos + 9, endPos - (nPos + 9));
				if (strCharset.compare("utf-8") == 0 || strCharset.compare("UTF-8") == 0)
				{
					// ת������
					std::string strRet = UTF_8ToLocal(strData.c_str());
					
					return strRet;
				}
			}
			
		}
	}

	return strData;
}

bool CWebHttp::SaveToFile(const std::string& strData, const char* pFileName)
{
	// ��ȡ��ǰ����Ŀ¼
	char lpFileName[MAX_PATH];
	if(SUCCEEDED(::GetModuleFileName(NULL, lpFileName, MAX_PATH)))
	{
		BOOL bRet = ::PathRemoveFileSpec(lpFileName);
		if (bRet == FALSE)
		{
			printf("\r\n [CWebHttp::SaveToFile()] PathRemoveFileSpec error! code: [%d] \r\n", ::GetLastError());
			return false;
		}

		char lpHtmlFileName[MAX_PATH];
#pragma warning(push)
#pragma warning(disable:4996)
		strcpy(lpHtmlFileName, lpFileName);
		strcat(lpHtmlFileName, "\\");
		strcat(lpHtmlFileName, pFileName);
#pragma warning(pop)
		FILE* fp = _tfopen(lpHtmlFileName, _T("w+"));
		if (fp)
		{
			_ftprintf(fp, strData.c_str());
			fclose(fp);
			return true;
		}
		else
		{
			printf("\r\n [CWebHttp::SaveToFile()] _tfopen error! code: [%d] \r\n", ::GetLastError());
			return false;
		}
	}
	else
	{
		printf("\r\n [CWebHttp::SaveToFile()] GetModuleFileName error! code: [%d] \r\n", ::GetLastError());
		return false;
	}

	return true;
}

/*
 * �����������á������Զ����á������������������Ŀ���ڽ�������ͨѶ��صĿ���ʱ����Ҫʹ�õ����ǣ��±߽�����ν���Щ������Ϣ��ȡ������
 * ����ʹ���Զ����ýű�����ʹ��ʱ��ʹ��WinHttpGetIEProxyConfigForCurrentUser��������ȡ�û��Ĵ������á���������û�ʹ�����Զ����ýű�����ô����Ҫʹ��API
 * WinHttpGetProxyForUrlȥ��ȡĳ��url��Ӧ�Ĵ�����ȡ������֮����Ҫ���ǣ�1���Ƿ���Ҫ���http��https��ftp��socks��2��hostname�Ƿ��ڡ����⡱�У���Ҫ����bypass��"<local>"�Ĵ���
 * ���䣺ʹ���Զ����ýű���Ҳ����ʹ��PAC�ļ�
 *		ע�⣺1��IE�������pac�ļ�·�������Ǳ��ش���·��������WinHttpGetProxyForUrl�����޷���ȷִ�У�
 *		2��WinHttpGetProxyForUrl�ĵڶ�������������http/https��ͷ������·����
 * ����API�ͽ�����WinHttpGetIEProxyConfigForCurrentUser��WinHttpGetProxyForUrl��������chromium�����������ؼ�������ȡchromium��ʵ�ִ��롣
 */
std::wstring CWebHttp::GetProxy(const char *pUrl)
{
	std::wstring wstrRet;

	WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions = { 0 };
	// ����ʹ�� GlobalFree �����ͷŷ� Null �ַ�����Ա������
	/*
		 typedef struct
		{
			BOOL    fAutoDetect;			// ӳ�䵽��������(LAN)���á��Ի����еġ��Զ�������á�ѡ�
			LPWSTR  lpszAutoConfigUrl;		// ӳ�䵽��ʹ���Զ����ýű�����ַ������ֻ��ѡ�д�ѡ��Ż����ó�Ա��
			LPWSTR  lpszProxy;				// ӳ�䵽��ʹ�ô��������...����ַ�Ͷ˿ڣ�����ֻ��ѡ�д�ѡ��Ż����ó�Ա��
			LPWSTR  lpszProxyBypass;		// ӳ�䵽���쳣������λ�ڵ������߼�����ťʱ���ֵĴ����У���ָ���ķ������б�ֻ��ѡ�С��������ص�ַ�Ĵ����������ѡ��Ż����ó�Ա��
		} WINHTTP_CURRENT_USER_IE_PROXY_CONFIG;
	 */
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig = { 0 };

	BOOL bAutoDetect = FALSE;	// ���Զ�������á�������ʱ�򼴱�ѡ����Ҳ�᷵��0��������Ҫ����url�ж�

	// ���� Internet Explorer �Ĵ������ã���Ϊ������û��������������Ϊͨ�� Web ���������� Web��
	// ������Internet Explorer ѡ������С����ӡ�ѡ��ϵġ�������(LAN)���á���ť������ҵ����ô������õ� Internet Explorer �Ի���
	// WinHTTP �ṩ�� WinHttpGetIEProxyConfigForCurrentUser ����������ֱ��Ϊ��ǰ�û��������� Internet Explorer �������ã�
	if (::WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig))
	{
		// Internet Explorer ʹ�� Web �����Զ����֣�Web Proxy Auto-Discovery��WPAD��Э�������Ҵ������á�
		// WPAD ��������� DNS �� DHCP �������ĸ������ṩ PAC �ű��� URL����˿ͻ��˲���Ԥ�������ض��� PAC �ű������뾲̬ IP ��ַ�Ͷ�̬ IP ��ַ֮��Ĺ�ϵ���ơ�
		if (ieProxyConfig.fAutoDetect)
		{
			bAutoDetect = TRUE;
		}

		if (ieProxyConfig.lpszAutoConfigUrl != NULL)
		{
			bAutoDetect = TRUE;
			autoProxyOptions.lpszAutoConfigUrl = ieProxyConfig.lpszAutoConfigUrl;
		}
	}
	else
	{
		printf("\r\n [CWebHttp::GetProxy()] WinHttpGetIEProxyConfigForCurrentUser error! code: [%d] \r\n", ::GetLastError());
		return wstrRet;
	}

	// ���ѡ�е�������ѡ��
	if (bAutoDetect)
	{
		// ָʾ WinHttpGetProxyForUrl ʹ�� WPAD ���Զ����� PAC �ļ��� URL
		if (autoProxyOptions.lpszAutoConfigUrl != NULL)
		{
			autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
		}
		else
		{
			// ��������� lpszAutoConfigUrl�����ʾ Internet Explorer ���ش����Զ����� (PAC) �ļ���ȷ���ض����ӵĴ����������
			// ���ص� PAC �ļ�ͨ������ JavaScript���� JavaScript �������ڽ��ͻ���ָ���ض����������������ȡ����Ŀ���������
			autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
			autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
		}

		autoProxyOptions.fAutoLogonIfChallenged = TRUE;

		HINTERNET hSession = ::WinHttpOpen(0, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
		if (hSession != NULL)
		{
			WINHTTP_PROXY_INFO autoProxyInfo = { 0 };

			std::wstring wstrUrl = LocalToUnicode(pUrl);
			
			/*
			 * ������粻�ṩ WPAD���������ȴ������Ӳ��ܷ��� WinHttpGetProxyForUrl������ GetLastError �᷵�� ERROR_WINHTTP_AUTODETECTION_FAILED��
			 * ���Խ��յ��Ľ���� WinHttpSetOption ������ WINHTTP_OPTION_PROXY ѡ����ʹ�ã��������ض��������Ĵ������á�

			 * @param:
			 * ��һ�� WinHttpGetProxyForUrl ����ָ�� WinHTTP �Ự���
			 * ��һ�� WinHttpGetProxyForUrl ����ָ�� WinHTTP �Ự���, wstrUrl: L"http://example.com/path",
			 * ������������һ�����������ָ������ WinHttpGetProxyForUrl ������Ϊ�� WINHTTP_AUTOPROXY_OPTIONS �ṹ�� �����ڴ˴�ָ���� Internet Explorer �������ü������� PAC �ļ��� URL��
			 * ���һ��������һ�����������ָ�����ս���� WINHTTP_PROXY_INFO �ṹ��
			 */
			BOOL bRet = ::WinHttpGetProxyForUrl(hSession, wstrUrl.c_str(), &autoProxyOptions, &autoProxyInfo);
			if (bRet == FALSE)
			{
				printf("\r\n [CWebHttp::GetProxy()] WinHttpGetProxyForUrl error! code: [%d] \r\n", ::GetLastError());
				goto CATCH_ERROR;
			}

			::WinHttpCloseHandle(hSession);

			if (autoProxyInfo.lpszProxy)
			{
				if (autoProxyInfo.lpszProxyBypass == NULL || CheckPassBy(pUrl, autoProxyInfo.lpszProxyBypass))
				{
					std::wstring wstrProxyAddr = autoProxyInfo.lpszProxy;
					wstrRet = wstrProxyAddr;
				}
				
				// GlobalFree �����ͷ� WINHTTP_PROXY_INFO �ṹ�ķ� Null �ַ�����Ա����
				if (autoProxyInfo.lpszProxy != NULL)
				{
					::GlobalFree(autoProxyInfo.lpszProxy);
				}
				if (autoProxyInfo.lpszProxyBypass != NULL)
				{
					::GlobalFree(autoProxyInfo.lpszProxyBypass);
				}
			}
		}
		else
		{
			printf("\r\n [CWebHttp::GetProxy()] WinHttpOpen error! code: [%d] \r\n", ::GetLastError());
			goto CATCH_ERROR;
		}
	}
	else
	{
		// ��������� lpszProxy�������ֱ�ӽ������������������
		if (ieProxyConfig.lpszProxy != NULL)
		{
			if (ieProxyConfig.lpszProxyBypass == NULL || CheckPassBy(pUrl, ieProxyConfig.lpszProxyBypass))
			{
				std::wstring wstrProxyAddr = ieProxyConfig.lpszProxy;
				wstrRet = wstrProxyAddr;
			}
		}
	}


CATCH_ERROR:
	if (ieProxyConfig.lpszAutoConfigUrl != NULL)
	{
		::GlobalFree(ieProxyConfig.lpszAutoConfigUrl);
	}

	if (ieProxyConfig.lpszProxy != NULL)
	{
		::GlobalFree(ieProxyConfig.lpszProxy);
	}

	if (ieProxyConfig.lpszProxyBypass != NULL)
	{
		::GlobalFree(ieProxyConfig.lpszProxyBypass);
	}

	return wstrRet;
}

bool CWebHttp::CheckPassBy(const char* pUrl, LPCWSTR lpwszByPass)
{
	return true;
}
