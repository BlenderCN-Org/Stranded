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

		NULL,										// 会话对象参数: WINHTTP_ACCESS_TYPE_NO_PROXY (指示不会检索或使用代理设置， 可以在创建会话后覆盖它)；
													// WINHTTP_ACCESS_TYPE_DEFAULT_PROXY (指示应该使用 WinHTTP 在注册表中存储的所有代理设置, 此参数允许管理员控制代理设置，而且还独立于 Internet Explorer 的代理设置.
													// 指示应该使用 WinHTTP 在注册表中存储的所有代理设置。此参数允许管理员控制代理设置，
													// 而且还独立于 Internet Explorer 的代理设置，因此是理想的服务器方案。
													// 在 Windows XP 和 Windows Server 2003 中，可以使用 proxycfg.exe 实用工具来设置和查询 WinHTTP 代理设置。
													// 在 Windows Vista 和 Windows Server 2008 中，netsh.exe 实用工具已替代 proxycfg.exe 实用工具提供相应的功能。
													// 您还可以使用 WinHttpSetDefaultProxyConfiguration 函数直接设置 WinHTTP 代理设置，并使用 WinHttpGetDefaultProxyConfiguration 函数进行查询。
													// 这些设置在重新启动后仍然有效，并由所有登录会话共用。这两个函数均使用 WINHTTP_PROXY_INFO 结构，该结构直接映射到 WinHttpOpen 函数中与代理相关的三个参数。)；
													// WINHTTP_ACCESS_TYPE_NAMED_PROXY (指示 WinHTTP 应该在接下来的两个参数中使用代理设置).

		NULL,										// 指定代理服务器的名称 (L"http://127.0.0.1:61497")
		NULL,										// 指定一个 HTTP 服务器可选列表，其中的服务器不能通过上一个参数指定的代理服务器进行路由。(L"<local>")
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

		////////////////////////////////////// POST 方式 /////////////////////////////////
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
		//////////////////////////////////// GET方式 //////////////////////////////////////
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
					// 转换编码
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
	// 获取当前程序目录
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
 * “局域网设置”里有自动配置、代理服务器的设置项目，在进行网络通讯相关的开发时，需要使用到它们，下边介绍如何将这些设置信息读取出来。
 * 当“使用自动配置脚本”不使用时，使用WinHttpGetIEProxyConfigForCurrentUser函数来获取用户的代理配置。如果发现用户使用了自动配置脚本，那么就需要使用API
 * WinHttpGetProxyForUrl去获取某个url对应的代理。获取到代理之后需要考虑：1、是否需要拆分http、https、ftp、socks；2、hostname是否在“例外”中，需要考虑bypass有"<local>"的处理。
 * 补充：使用自动配置脚本（也就是使用PAC文件
 *		注意：1、IE设置里的pac文件路径不能是本地磁盘路径，否则WinHttpGetProxyForUrl函数无法正确执行；
 *		2、WinHttpGetProxyForUrl的第二个参数必须是http/https开头的完整路径。
 * 核心API就仅仅是WinHttpGetIEProxyConfigForCurrentUser和WinHttpGetProxyForUrl，可以在chromium中搜索这两关键函数获取chromium的实现代码。
 */
std::wstring CWebHttp::GetProxy(const char *pUrl)
{
	std::wstring wstrRet;

	WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions = { 0 };
	// 必须使用 GlobalFree 函数释放非 Null 字符串成员变量。
	/*
		 typedef struct
		{
			BOOL    fAutoDetect;			// 映射到“局域网(LAN)设置”对话框中的“自动检测设置”选项。
			LPWSTR  lpszAutoConfigUrl;		// 映射到“使用自动配置脚本”地址，而且只有选中此选项才会填充该成员。
			LPWSTR  lpszProxy;				// 映射到“使用代理服务器...”地址和端口，而且只有选中此选项才会填充该成员。
			LPWSTR  lpszProxyBypass;		// 映射到“异常”区域（位于单击“高级”按钮时出现的窗口中）中指定的服务器列表，只有选中“跳过本地地址的代理服务器”选项才会填充该成员。
		} WINHTTP_CURRENT_USER_IE_PROXY_CONFIG;
	 */
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig = { 0 };

	BOOL bAutoDetect = FALSE;	// “自动检测设置”，但有时候即便选择上也会返回0，所以需要根据url判断

	// 检索 Internet Explorer 的代理设置，因为大多数用户的浏览器已配置为通过 Web 代理来访问 Web。
	// 单击“Internet Explorer 选项”窗口中“连接”选项卡上的“局域网(LAN)设置”按钮，便可找到配置代理设置的 Internet Explorer 对话框。
	// WinHTTP 提供了 WinHttpGetIEProxyConfigForCurrentUser 函数，可以直接为当前用户检索下列 Internet Explorer 代理设置：
	if (::WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig))
	{
		// Internet Explorer 使用 Web 代理自动发现（Web Proxy Auto-Discovery，WPAD）协议来查找代理设置。
		// WPAD 根据网络的 DNS 或 DHCP 服务器的更新来提供 PAC 脚本的 URL，因此客户端不必预先配置特定的 PAC 脚本。这与静态 IP 地址和动态 IP 地址之间的关系类似。
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

	// 如果选中的是其他选项
	if (bAutoDetect)
	{
		// 指示 WinHttpGetProxyForUrl 使用 WPAD 来自动查找 PAC 文件的 URL
		if (autoProxyOptions.lpszAutoConfigUrl != NULL)
		{
			autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
		}
		else
		{
			// 如果填充的是 lpszAutoConfigUrl，则表示 Internet Explorer 下载代理自动配置 (PAC) 文件来确定特定连接的代理服务器。
			// 下载的 PAC 文件通常包含 JavaScript，而 JavaScript 可能用于将客户端指向特定代理服务器，具体取决于目标服务器。
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
			 * 如果网络不提供 WPAD，则可能需等待几秒钟才能返回 WinHttpGetProxyForUrl，而且 GetLastError 会返回 ERROR_WINHTTP_AUTODETECTION_FAILED。
			 * 可以将收到的结果与 WinHttpSetOption 函数和 WINHTTP_OPTION_PROXY 选项结合使用，以设置特定请求对象的代理设置。

			 * @param:
			 * 第一个 WinHttpGetProxyForUrl 参数指定 WinHTTP 会话句柄
			 * 第一个 WinHttpGetProxyForUrl 参数指定 WinHTTP 会话句柄, wstrUrl: L"http://example.com/path",
			 * 第三个参数是一个输入参数，指定控制 WinHttpGetProxyForUrl 函数行为的 WINHTTP_AUTOPROXY_OPTIONS 结构。 您将在此处指定从 Internet Explorer 代理设置检索到的 PAC 文件的 URL。
			 * 最后一个参数是一个输出参数，指定接收结果的 WINHTTP_PROXY_INFO 结构。
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
				
				// GlobalFree 函数释放 WINHTTP_PROXY_INFO 结构的非 Null 字符串成员变量
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
		// 如果填充的是 lpszProxy，则可以直接将它用作代理服务器。
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
