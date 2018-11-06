a.h

class CA;

extern CA* a;

/////内容前//////

a.cpp

#include "CA.h"

CA* a = NULL;

///////内容前////////

/////////////////////////////////

b.h

class CB
{
	
};

extern CB g_CB;

b.cpp

CB g_CB;

///////内容前////////

while(!m_map.empty())
{
	map:iterator it = m_map.begin();
	// do something or save it->second;
	
	m_map.erase(it);
	
	// do something...
}

for(iter = m_map.begin(); iter!=m_map.end();)
{
	if((iter->second)>=aa)
	{
		//满足删除条件，删除当前结点，并指向下面一个结点
		m_map.erase(iter++);
	}
	else
	{
		//条件不满足，指向下面一个结点
		iter++;
	}
}

std::string LocalToUTF_8(const char* gb2312)
{
	// gb2312 to utf-16
	int len = ::MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, sizeof(wchar_t)*(len+1));
	::MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	
	// utf-16 to utf-8
	len = ::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, sizeof(char)*(len+1));
	::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	
	if(wstr)
	{
		delete[] wstr;
		wstr = NULL;
	}
	
	std::string re = str;
	if(str)
	{
		delete[] str;
		str = NULL;
	}
	
	return re;
}

std::string UTF_8ToLocal(const char* szUtf_8)
{
	// utf-8 to utf-16
	int len = ::MultiByteToWideChar(CP_UTF8, 0, szUtf_8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, sizeof(wchar_t)*(len+1));
	::MultiByteToWideChar(CP_UTF8, 0, szUtf_8, -1, wstr, len);
	
	// utf-16 to local
	len = ::WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, sizeof(char)*(len+1));
	::WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	
	if(wstr)
	{
		delete[] wstr;
		wstr = NULL;
	}
	
	std::string re = str;
	if(str)
	{
		delete[] str;
		str = NULL;
	}
	
	return re;
}