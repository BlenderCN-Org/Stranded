#ifndef __WEB_HTTP_H__
#define __WEB_HTTP_H__

#include <windows.h>
#include <Shlwapi.h>
#include <string>

#define USE_WINHTTP
#ifdef USE_WINHTTP
#include <winhttp.h>
#else
// For MFC #include <afxinet.h>
// InternetOpenUrl 必须包含 http://
// InternetConnect 最好不包含 http:// 这样可以获取到http头[如果有头的话]
#include <Wininet.h>
#endif

#include "../Common/pubutil.h"

#pragma comment(lib, "Shlwapi.lib")

class CCrackUrl
{
public:
	CCrackUrl() : m_scheme(0), m_port(0)
	{

	}

	~CCrackUrl() {}

public:
	// "http://www.easy-creator.net/test2/add.asp"
	bool SetUrl(LPCWSTR url)
	{
		URL_COMPONENTS uc = { 0 };
		uc.dwStructSize = sizeof(uc);

		const DWORD BUF_LEN = 256;

		WCHAR host[BUF_LEN];
		uc.lpszHostName = host;
		uc.dwHostNameLength = BUF_LEN;

		WCHAR path[BUF_LEN];
		uc.lpszUrlPath = path;
		uc.dwUrlPathLength = BUF_LEN;

		WCHAR extra[BUF_LEN];
		uc.lpszExtraInfo = extra;
		uc.dwExtraInfoLength = BUF_LEN;

#ifdef USE_WINHTTP
		if (!::WinHttpCrackUrl(url, 0, ICU_ESCAPE, &uc))
		{
			printf("\r\n [CCrackUrl::CCrackUrl()] WinHttpCrackUrl error! code: [%d] \r\n", ::GetLastError());
			return false;
		}
#else
		if (!::InternetCrackUrl(url, 0, ICU_ESCAPE, &uc))
		{
			printf("\r\n [CCrackUrl::CCrackUrl()] InternetCrackUrl error! code: [%d] \r\n", ::GetLastError());
			return false;
		}
#endif

		m_scheme = uc.nScheme;
		m_host = host;
		m_port = uc.nPort;
		m_path = path;

		return true;
	}

	int GetScheme() const
	{
		return m_scheme;
	}

	const std::wstring& GetHostName() const
	{
		return m_host;
	}

	int GetPort() const
	{
		return m_port;
	}

	const std::wstring& GetPath() const
	{
		return m_path;
	}

	static std::string UrlEncode(const char* p)
	{
		if (p == NULL)
			return "";
		
		std::string buf;

		for (;;)
		{
			int ch = (BYTE)(*(p++));
			if(ch == '\0')
				break;

			if (isalnum(ch) || ch == '_' || ch == '-' || ch == '.')
				buf += (char)ch;
			else if (ch == ' ')
				buf += '+';
			else
			{
				char c[16];
				wsprintf(c, "%%%02X", ch);
				buf += c;
			}
		}

		return buf;
	}

private:
	int				m_scheme;
	std::wstring	m_host;
	int				m_port;
	std::wstring	m_path;
};

class CWebHttp
{
public:
	CWebHttp();
	virtual ~CWebHttp();

public:
	bool			SetCrackUrl(const char* pUrl);
	bool			ConnectWeb();
	// "POST"、"GET"
	bool			OpenRequest(const char* pType, const char* pPostData = NULL, const char* pHeader = NULL);
	std::string		ReadPOSTData();
	std::string		ReadGETData();

	bool			SaveToFile(const std::string& strData, const char* pFileName);

	std::wstring	GetProxy(const char *pUrl);
	bool			CheckPassBy(const char* pUrl, LPCWSTR lpwszByPass);

	bool			InitSession();

private:
	void			ShutDown();
	void			Clear();

private:
	// http 连接句柄
	HINTERNET			m_hSession;
	HINTERNET			m_hConnection;
	HINTERNET			m_hRequest;

	CCrackUrl			m_crachUrl;
};

#endif
