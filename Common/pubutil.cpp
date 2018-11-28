#include "pubutil.h"

#ifdef _MSC_VER
// Netbios
#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Iphlpapi.lib")
/*
* VerQueryValue
* GetFileVersionInfo
* GetFileVersionInfoSize
*/
#pragma comment(lib, "Version.lib")
#endif

#include "zlib/zlib.h"
#include "md5.h"
#include "xxtea.h"
#include "luaCompile.h"


const int BUFFER_SIZE = 48 * 1024 * 1024;

bool g_64bits = (sizeof(size_t) == 8) ? true : false;


#pragma warning(push)
#pragma warning(disable:4996)

/*
* �������򡪡����������㷨�е�һ�˻��ֺ���
* һ�˻��ֺ������������ػ��ֺ󱻶�λ�Ļ�׼��¼��λ��
* �������� R[s] �� R[t] ���л���
*/
int partition(rectype r[], int s, int t)
{
	int i, j;
	rectype temp;

	i = s;
	j = t;
	temp = r[i];	//��ʼ����tempΪ��׼��¼
	do
	{
		while ((r[j].key >= temp.key) && (i < j))
			j--;	//��������ɨ�裬���ҵ�һ���ؼ���С��temp�ļ�¼

		if (i < j)
			r[i++] = r[j];	//����R[i]��R[j]

		while ((r[i].key <= temp.key) && (i < j))
			i++;	//��������ɨ�裬���ҵ�һ���ؼ��ִ���temp�ļ�¼

		if (i < j)
			r[j--] = r[i];	//����R[i]��R[j]
	} while (i != j);	//i=j, ��һ�λ��ֽ�������׼��¼����������λ��

	r[i] = temp;	//��󽫻�׼��¼temp��λ

	return i;
}

/*
* ���������㷨������R[hs]��R[ht]���п�������
*/
void quick_sort(rectype r[], int hs, int ht)
{
	int i;

	//ֻ��һ�����޼�¼ʱ��������
	if (hs < ht)
	{
		i = partition(r, hs, ht);	//��R[hs]��R[ht]����һ�λ���
		quick_sort(r, hs, i - 1);	//�ݹ鴦��������
		quick_sort(r, i + 1, ht);	//�ݹ鴦��������
	}
}

double get_rag_dis()
{
	point P, Ori, Dest;

	double a = getdist(Ori, P);
	double b = getdist(Dest, P);
	double c = getdist(Dest, Ori);

	double p = (a + b + c) * 0.5;

	double area = sqrt(p * (p - a) * (p - b) * (p - c));

	double res = a;

	if (!check(a, b, c))
		res = min(res, area * 2 / b);

	return res;
}

void Swap(char *a, char *b)
{
	char t = *b;
	*a = *b;
	*b = t;
}

void AllRange(char *pszStr, int k, int m)
{
	if (k == m)
	{
		static int s_i = 1;
		printf("��[%3d]��ȫ����\t[%s]\n", s_i++, pszStr);
	}
	else
	{
		for (int i = k; i <= m; i++)
		{
			if (IsSwap(pszStr, k, i))
			{
				Swap(pszStr + k, pszStr + i);
				AllRange(pszStr, k + i, m);
				Swap(pszStr + k, pszStr + i);
			}
		}
	}
}

void Foo(char *pszStr)
{
	AllRange(pszStr, 0, strlen(pszStr) - 1);
}

bool IsSwap(char *pszStr, int nBegin, int nEnd)
{
	for (int i = nBegin; i < nEnd; i++)
		if (pszStr[i] == pszStr[nEnd])
			return false;

	return true;
}

void Reverse(char *a, char *b)
{
	while (a < b)
		Swap(a++, b--);
}

bool Next_permutation(char a[])
{
	char *pEnd = a + strlen(a);
	if (a == pEnd)
		return false;

	char *p, *q, *pFind;
	pEnd--;
	p = pEnd;
	while (p != a)
	{
		q = p;
		--p;
		if (*p < *q)
		{
			//�Ӻ���ǰ�ұ��滻���ĵ�һ����
			pFind = pEnd;
			while (*pFind <= *p)
				--pFind;

			//�滻
			Swap(pFind, p);
			//�滻������ȫ����ת
			Reverse(q, pFind);
			return true;
		}
	}

	Reverse(p, pEnd);	//���û����һ�����У�ȫ����ת�󷵻�true
	return false;
}

#ifdef LUA_API
void ShowLuaStack(lua_State *L)
{
	printf("\r\nlua stack------------------------------------------\r\n");
	// Lua Stack ���ַ�ʽ
	//for(int i=1; i <= lua_gettop(L); i++)
	for (int i = -1; i >= -lua_gettop(L); i--)
	{
		if (lua_istable(L, i))
			printf("%d: table\r\n", i);
		else if (lua_isnone(L, i))
			printf("%d: none\r\n", i);
		else if (lua_isnil(L, i))
			printf("%d: nil\r\n", i);
		else if (lua_isboolean(L, i))
		{
			if (lua_toboolean(L, i) != 0)
				printf("%d: bool - true\r\n", i);
			else
				printf("%d: bool - false\r\n", i);
		}
		else if (lua_isnumber(L, i))
			printf("%d: %d\r\n", i, lua_tonumber);
		else if (lua_isfunction(L, i))
			printf("%d: function\r\n", i);
		else if (lua_islightuserdata(L, i))
			printf("%d: lightuserdata\r\n", i);
		else if (lua_isuserdata(L, i))
			printf("%d: userdata\r\n", i);
		else if (lua_isthread(L, i))
			printf("%d: thread\r\n", i);
		else if (lua_isstring(L, i))
			printf("%d: %s\r\n", i, lua_tostring);
		else
		{
			printf("%d: unknown type = [%s]\r\n", i, lua_typename(L, lua_type(L, i)));
		}
	}
	printf("\r\nend stack------------------------------------------\r\n");
}

void ShowLuaTraceBack(lua_State *L)
{
	int stackSize = lua_gettop(L);
	lua_getglobal(L, "__G__TRACKBACK__");
	bool isExistFunc = lua_isfunction(L, -1);
	lua_pop(L, 1);
	if (!isExistFunc)
		return;

	lua_getglobal(L, "__G__TRACKBACK__");
	lua_pushstring(L, "[DEBUG]");
	lua_pcall(L, 1, 0, 0);	// lua_pcall ���Զ����push��parameters.

	////////////////////////////////���ַ�ʽ//////////////////////////////////////////
	// ��ӡlua����ջ
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	int iError = lua_pcall(L,	// VMachine
		0,	// Argument Count
		1,	// Return value count
		0);
	const char* sz = lua_tostring(L, -1);
	print("\r\n%s\r\n", sz);
	//////////////////////////////////////////////////////////////////////////
}

#endif // LUA_API

/*
 * @sample:
	int **myArray;
	myArray = Make2DArray(5, 10);
	myArray[2][4] = 6;
 *
 * �����û�ָ����С��һ��洢���������������ȫ������(���з����λ����0)������ָ��洢�����׵�ַ������洢�ռ䲻�㣬�򷵻�NULL.
 * void* calloc(size_t _Count, size_t _Size);
 *
 * ������ malloc �� calloc ����Ĵ洢�ռ�_Block��СΪ_Size. ǰmin{s, oldSize} �ֽ����ݲ���.��s>oldSize�������� s-oldSize�ֽڣ����ݲ�ȷ����,
 * ��s<oldSize����ԭ�����������ǲ��ֶ���� oldSize - s �ֽ����ݱ��ͷš���� realloc �����洢�ռ��С�ɹ����򷵻��´洢�����׵�ַ�����򷵻�NULL.
 * void* realloc(void* _Block, size_t _Size);
 */
int ** Make2DArray(int rows, int cols)
{
	int **x, i;

	// get memory for row pointers
	x = (int**)malloc(rows * sizeof(int *));

	// get memory for each row
	for (i = 0; i < rows; i++)
		x[i] = (int*)malloc(cols * sizeof(int));

	return x;
}

std::string GetRegValue(int nKeyType, const std::string& strUrl, const std::string& strKey)
{
	std::string strValue;

	HKEY hKey = NULL;
	HKEY hKeyResult = NULL;
	DWORD dwSize = 0;
	DWORD dwDataType = 0;

	switch (nKeyType)
	{
	case 0:
	{
		hKey = HKEY_CLASSES_ROOT;
		break;
	}
	case 1:
	{
		hKey = HKEY_CURRENT_USER;
		break;
	}
	case 2:
	{
		hKey = HKEY_LOCAL_MACHINE;
		break;
	}
	case 3:
	{
		hKey = HKEY_USERS;
		break;
	}
	case 4:
	{
		hKey = HKEY_PERFORMANCE_DATA;
		break;
	}
	case 5:
	{
		hKey = HKEY_CURRENT_CONFIG;
		break;
	}
	case 6:
	{
		hKey = HKEY_DYN_DATA;
		break;
	}
	case 7:
	{
		hKey = HKEY_CURRENT_USER_LOCAL_SETTINGS;
		break;
	}
	case 8:
	{
		hKey = HKEY_PERFORMANCE_TEXT;
		break;
	}
	case 9:
	{
		hKey = HKEY_PERFORMANCE_NLSTEXT;
		break;
	}
	default:
		return strValue;
	}

	// ��ע���
	if (ERROR_SUCCESS == ::RegOpenKeyEx(hKey, strUrl.c_str(), 0, KEY_QUERY_VALUE, &hKeyResult))
	{
		// ��ȡ����ĳ��� dwSize ������ dwDatatype
		::RegQueryValueEx(hKeyResult, strKey.c_str(), 0, &dwDataType, NULL, &dwSize);
		switch (dwDataType)
		{
		case REG_MULTI_SZ:
		{
			// �����ڴ��С
			BYTE* lpValue = new BYTE[dwSize];
			memset(lpValue, 0, dwSize * sizeof(BYTE));
			// ��ȡע�����ָ���ļ�����Ӧ��ֵ
			LONG lRet = ::RegQueryValueEx(hKeyResult, strKey.c_str(), 0, &dwDataType, lpValue, &dwSize);
			delete[] lpValue;
			break;
		}
		case REG_SZ:
		{
			// �����ڴ��С
			char* lpValue = new char[dwSize];
			memset(lpValue, 0, dwSize * sizeof(char));
			// ��ȡע�����ָ���ļ�����Ӧ��ֵ
			if (ERROR_SUCCESS == ::RegQueryValueEx(hKeyResult, strKey.c_str(), 0, &dwDataType, (LPBYTE)lpValue, &dwSize))
			{
				strValue = lpValue;
			}
			delete[] lpValue;
			break;
		}
		default:
			break;
		}
	}

	// �ر�ע���
	::RegCloseKey(hKeyResult);

	return strValue;
}


std::string ws2s(const std::wstring& ws)
{
	std::string curLocale = setlocale(LC_ALL, "");

	const wchar_t* source = ws.c_str();

	size_t dSize = wcstombs(NULL, source, 0) + 1;

	char* dest = new char[dSize];
	memset(dest, 0, dSize);
	wcstombs(dest, source, dSize);

	std::string result = dest;

	delete[] dest;

	setlocale(LC_ALL, curLocale.c_str());

	return result;
}

std::wstring s2ws(const std::string& s)
{
	std::string curLocale = setlocale(LC_ALL, "");

	const char* source = s.c_str();

	size_t dSize = mbstowcs(NULL, source, 0) + 1;

	wchar_t* dest = new wchar_t[dSize];
	memset(dest, 0, dSize);
	mbstowcs(dest, source, dSize);

	std::wstring result = dest;

	delete[] dest;

	setlocale(LC_ALL, curLocale.c_str());

	return result;
}

//////////////////////////////////////////////////////////////////////////
/*
 * �ȱȽ�ģʽ��ĩβ�ַ���ģʽƥ��
 * �������strlen(pat)��������ʣ�µĴ����ý�������;
 * ���pat�ĵ�һ���ַ���stringƥ�䣬��������pat�����һ���ַ���string�Ķ�Ӧλ�ñȽ�.
 * O(m) - m=strlen(string)
 */
int NFind(char *string, char *pat)
{
	int i, j, start = 0;
	int lasts = strlen(string) - 1;
	int lastp = strlen(pat) - 1;
	int endmatch = lastp;

	for (i = 0; endmatch <= lasts; endmatch++, start++)
	{
		if (string[endmatch] == pat[lastp])
			for (j = 0, i = start; j < lastp && string[i] == pat[i]; i++, j++)
				;
		if (j == lastp)
			return start;		/* successful */
	}

	return -1;
}

/*
 * KMP �㷨
 * Knuth, Morris, Pratt string matching algorithm
 ����: ģʽ�� P=P0P1 ... P(n-1) ��ʧ�亯��Ϊ��
			max{i,0},	������ P0P1 ... Pi = P(j-i)P(j-i+1) ... Pj, i<j;
	f(j) =
			-1,			����

	���磬ģʽ�� P = 'abcabcacab'����ʧ�亯���ǣ�

	j		0	1	2	3	4	5	6	7	8	9
	P		a	b	c	a	b	c	a	c	a	b
	f(j)	-1	-1	-1	0	1	2	3	-1	0	1

	ģʽƥ�����
		�������ƥ������ S(i-j) ... S(i-1) = P0P1 ... P(j-1) �� Si != Pj������������ j!=0,
	Ӧ���� Si �� P(f(j-1)+1) ��ȣ� ��� j==0��Ӧ���� P0 �� S(i+1) ���
 *
 * O(strlen(string))
 *
 * int failure[MAX_PATTERN_SIZE];
 * char string[MAX_STRING_SIZE];
 * char pat[MAX_PATTERN_SIZE];
 */
int PMatch(char * string, char * pat, int failure[])
{
	int i = 0, j = 0;
	int lens = strlen(string);
	int lenp = strlen(pat);

	while (i < lens && j < lenp)
	{
		if (string[i] == pat[j])
		{
			i++;
			j++;
		}
		else if (j == 0)
			i++;
		else
			j = failure[j - 1] + 1;
	}

	/*
	 * �ɴˣ��ɻ�ȡָ��������ģʽ�����ֵ���ʼλ��ָ��.
	 * ����Ƿ��ҵ�ģʽ�������δ�ҵ�����ôģʽ���±�j������ģʽ�����ȣ�����-1��
	 * ����ҵ�ģʽ������ô����ģʽ���������ֵ���ʼλ�õ���i-ģʽ������.
	 */
	return (j == lenp) ? i - lenp : -1;
}

/*
* compute the pattern's failure function
*
			-1,						��� j=0;
	F(j) =	(F(j-1)��m�η�) + 1,	m ���� P (f(j-1)��k�η� + 1) = Pj ����С���� k;
			-1,						���û��������ʽ��k.
	(��ʽ�� F(j)��1�η� = F(j)�� F(j)��m�η� = F( F(j)��m-1�η� ) )
*
* O(strlen(pat))
*/
void Fail(char *pat, int failure[])
{
	int n = strlen(pat);
	failure[0] = -1;
	int i, j;
	for (j = 1; j < n; j++)
	{
		i = failure[j - 1];
		while ((pat[j] != pat[i + 1]) && (i >= 0))
			i = failure[j - 1];

		if (pat[j] == pat[i + 1])
			failure[j] = i + 1;
		else
			failure[j] = -1;
	}
}

//////////////////////////////////////////////////////////////////////////

void GetMemoryAddress(void *ptr, char* pStr)
{
	if (pStr)
		return;

	std::addressof(ptr);
	sprintf(pStr, "%p", pStr);
}




// char pMac[36] = "00-00-00-00-00-00";
int GetMAC( char *pMac )
{
	NCB ncb;
	typedef struct _ASTAT_
	{
		ADAPTER_STATUS adapt;
		NAME_BUFFER NameBuff[30];
	}ASTAT, * PASTAT;
	ASTAT Adapter;

	typedef struct _LANA_ENUM
	{
		// le
		UCHAR length;
		UCHAR lana[MAX_LANA];
	}LANA_ENUM;
	LANA_ENUM lana_enum;

	UCHAR uRetCode;
	memset(&ncb, 0, sizeof(ncb));
	memset(&lana_enum, 0, sizeof(lana_enum));

	ncb.ncb_command = NCBENUM;
	ncb.ncb_buffer = (unsigned char*)&lana_enum;
	ncb.ncb_length = sizeof(LANA_ENUM);

	uRetCode = ::Netbios(&ncb);
	if(uRetCode != NRC_GOODRET)
		return uRetCode;

	for(int lana=0; lana < lana_enum.length; ++lana)
	{
		ncb.ncb_command = NCBRESET;
		ncb.ncb_lana_num = lana_enum.lana[lana];

		uRetCode = ::Netbios(&ncb);
		if(uRetCode == NRC_GOODRET)
			break;
	}
	if(uRetCode != NRC_GOODRET)
		return uRetCode;

	memset(&ncb, 0, sizeof(ncb));
	
	ncb.ncb_command = NCBASTAT;
	ncb.ncb_lana_num = lana_enum.lana[0];
	strcpy((char*)ncb.ncb_callname, "*");
	ncb.ncb_buffer = (unsigned char*)&Adapter;
	ncb.ncb_length = sizeof(Adapter);

	uRetCode = ::Netbios(&ncb);
	if(uRetCode != NRC_GOODRET)
		return uRetCode;

	sprintf(pMac, "%02X-%02X-%02X-%02X-%02X-%02X",
		Adapter.adapt.adapter_address[0],
		Adapter.adapt.adapter_address[1],
		Adapter.adapt.adapter_address[2],
		Adapter.adapt.adapter_address[3],
		Adapter.adapt.adapter_address[4],
		Adapter.adapt.adapter_address[5]
		);
	return 0;
}

// char szInfo[4096] = {0};
void GetHwInformation( char* szInfo )
{
	HKEY hKey;

	// �õ�CPU����
	::RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey);

	DWORD dwMhz = 0;
	DWORD dwSize = sizeof(dwMhz);
	::RegQueryValueEx(hKey, "~MHz", 0, 0, (LPBYTE)&dwMhz, &dwSize);

	char szCpu[256] = {0};
	dwSize = sizeof(szCpu);
	::RegQueryValueEx(hKey, "ProcessorNameString", 0, 0, (LPBYTE)szCpu, &dwSize);

	char szVendor[256] = {0};
	dwSize = sizeof(szVendor);
	::RegQueryValueEx(hKey, "VendorIdentifier", 0, 0, (LPBYTE)szVendor, &dwSize);

	char szIdentifier[256] = {0};
	dwSize = sizeof(szIdentifier);
	::RegQueryValueEx(hKey, "Identifier", 0, 0, (LPBYTE)szIdentifier, &dwSize);

	::RegCloseKey(hKey);

	sprintf(szInfo, "CPU: %dMHz, %s, %s, %s\r\n", (int)dwMhz, szVendor, szCpu, szIdentifier);

	// ����ϵͳ�汾��
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&osvi);

	char szTemp[1024] = {0};
	sprintf(szTemp, "OS: %u, %u, %u, %u\r\n", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber, osvi.dwPlatformId);
	strcat(szInfo, szTemp);

	// �õ��ڴ�����
	MEMORYSTATUS ms;
	::GlobalMemoryStatus(&ms);
	sprintf(szTemp, "mem(F/T): %d%%,", ms.dwMemoryLoad);
	strcat(szInfo, szTemp);

	if(ms.dwAvailPhys >= (1024*1024))
		sprintf(szTemp, "%dM/%dM, ", ms.dwAvailPhys/1024/1024, ms.dwTotalPhys/1024/1024);
	else
		sprintf(szTemp, "%dk/%dM, ", ms.dwAvailPhys/1024, ms.dwTotalPhys/1024/1024);
	strcat(szInfo, szTemp);

	if(ms.dwAvailPageFile >= (1024*1024))
		sprintf(szTemp, "PF: %dM/%dM, ", ms.dwAvailPageFile/1024/1024, ms.dwTotalPageFile/1024/1024);
	else
		sprintf(szTemp, "PF: %dk/%dM, ", ms.dwAvailPageFile/1024, ms.dwTotalPageFile/1024/1024);
	strcat(szInfo, szTemp);

	if(ms.dwAvailVirtual >= (1024*1024))
		sprintf(szTemp, "V: %dM/%dM\r\n", ms.dwAvailVirtual/1024/1024, ms.dwTotalVirtual/1024/1024);
	else
		sprintf(szTemp, "V: %dk/%dM\r\n", ms.dwAvailVirtual/1024, ms.dwTotalVirtual/1024/1024);
	strcat(szInfo, szTemp);
}

// char szVersion[64] = {0};
DWORD GetGameVersion(char* szVersion, DWORD& dwBuildID, DWORD& dwHightWordBuildID)
{
	TCHAR szFullPath[MAX_PATH] = {0};
	DWORD dwVerInfoSize = 0;
	DWORD dwVerHnd = 0;

	DWORD dwVersion = 0x0;

	::GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));

	dwVerInfoSize = ::GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	if(dwVerInfoSize)
	{
		VS_FIXEDFILEINFO* pFileInfo;
		HANDLE hMem;
		LPVOID lpvMem;
		unsigned int uInfoSize = 0;

		hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpvMem = ::GlobalLock(hMem);

		::GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpvMem);

		::VerQueryValue(lpvMem, (LPTSTR)("\\"), (void**)&pFileInfo, &uInfoSize);

		WORD nProdVer[4];
		nProdVer[0] = HIWORD(pFileInfo->dwProductVersionMS);
		nProdVer[1] = LOWORD(pFileInfo->dwProductVersionMS);
		nProdVer[2] = HIWORD(pFileInfo->dwProductVersionLS);
		nProdVer[3] = LOWORD(pFileInfo->dwProductVersionLS);

		for(int ii = 0; ii < 4; ++ii)
		{
			dwVersion = (dwVersion << 8) + (BYTE)nProdVer[ii];
		}

		WORD wVersionTail = HIWORD(pFileInfo->dwFileVersionMS);
		WORD wVersionTailSe = LOWORD(pFileInfo->dwFileVersionMS);
		if(wVersionTail == 0)
		{
			sprintf(szVersion, "%d.%d.%d.%d", nProdVer[0], nProdVer[1], nProdVer[2], nProdVer[3]);
		}
		else if(wVersionTailSe == 0)
		{
			sprintf(szVersion, "%d.%d.%d.%d.%d", nProdVer[0], nProdVer[1], nProdVer[2], nProdVer[3], wVersionTail);
		}
		else
		{
			if(wVersionTailSe >= 10)
				sprintf(szVersion, "%d.%d.%d.%d.%d.%d.%d", nProdVer[0], nProdVer[1], nProdVer[2], nProdVer[3], wVersionTail, wVersionTailSe/10, wVersionTailSe%10);
			else
				sprintf(szVersion, "%d.%d.%d.%d.%d.%d", nProdVer[0], nProdVer[1], nProdVer[2], nProdVer[3], wVersionTail, wVersionTailSe);
		}

		dwBuildID = LOWORD(pFileInfo->dwFileVersionLS);	// �汾��
		dwHightWordBuildID = HIWORD(pFileInfo->dwFileVersionLS);	// ��ǰ�汾�ı���汾

		::GlobalUnlock(hMem);
		::GlobalFree(hMem);
	}
	else
	{
		dwBuildID = 0x0;
		dwHightWordBuildID = 0x0;
	}

	return dwVersion;
}

void output_debug( char* format,... )
{
	char szTmp[8 * 1024] = {0};

	va_list argptr;

	va_start(argptr, format);
	vsprintf(szTmp, format, argptr);
	va_end(argptr);

	::OutputDebugString(szTmp);
}

void DuffDevice(short& sFrom, short& sTo)
{
	register short *psTo = &sTo, *psFrom = &sFrom;
	register short sCount = 0;
	{
		register short n = (sCount + 7)/8;
		switch(sCount % 8)
		{
		case 0:	do {	*psTo = *psFrom++;
		case 7:			*psTo = *psFrom++;
		case 6:			*psTo = *psFrom++;
		case 5:			*psTo = *psFrom++;
		case 4:			*psTo = *psFrom++;
		case 3:			*psTo = *psFrom++;
		case 2:			*psTo = *psFrom++;
		case 1:			*psTo = *psFrom++;
				} while(--n > 0);
		}
	}
}

void _cdecl GetCPUID(char* sCpuID)
{
	if(!sCpuID)
		return;

	DWORD dwCPUName = 0, dwCPUReserved1 = 0, dwCPUReserved2 = 0, dwCPUID = 0;

	// _emit(__asm _emitһ����)������ֵ��ָ��, һ�ν�֧��һ���ֽ�
	// ����˿ڳ�ʼ��ֵ����������ݷ���CPU��
	// ģ��CPUIDָ�� #define cpuid(func,ax,bx,cx,dx), VC��֧��, Intel CPUID��ָ��������0x0FA2
	// func=1��edx��bit25 sse1��bit26 sse2
#define CPUID __asm _emit 0x0F __asm _emit 0xA2
	__asm
	{
		// ѹջ(��ǰ����еĶ�������Ϣ���ռ�����)
		PUSHAD
		// ��������Intel��CPU�ı�׼ȡCPUID������
		// EAX=1ʱ��CPUIDָ���EAX=CPU�汾��Ϣ��EDX=CPU֧�ֵ�����
		MOV EAX, 1
		CPUID
		// �������������Ϣ����EAX��EBX��ECX��EDX��
		MOV dwCPUName, EAX
		MOV dwCPUReserved1, EBX
		MOV dwCPUReserved2, ECX
		MOV dwCPUID, EDX
		// ��ջ(��ǰ����еĶ�������Ϣ�ͷų���)

		// ���CPU�Ƿ�֧��MMXָ��
		// test edx, 0x800000
		// jz _NO_SUPPORT_MMX;
		// ...
		// _NO_SUPPORT_MMX:
		// ...
		// Instruction of CPUID with eax=80000001h returns edx of bit29 that inel 64 Architecture available if 1
		POPAD
	}
	
	sprintf(sCpuID, "%08X-%08X-%08X", dwCPUID, dwCPUName, dwCPUReserved2);
}

std::string LocalToUTF_8( const char* gb2312 )
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

std::string UTF_8ToLocal( const char* szUtf_8 )
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

std::wstring LocalToUnicode(const char* gb2312)
{
	// gb2312 to Unicode
	int len = ::MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, sizeof(wchar_t)*(len + 1));
	::MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);

	std::wstring re = wstr;
	if (wstr)
	{
		delete[] wstr;
		wstr = NULL;
	}

	return re;
}

std::string UnicodetoLocal(const wchar_t* wUnicode)
{
	// Unicode to Local
	int len = ::WideCharToMultiByte(CP_ACP, 0, wUnicode, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, sizeof(char)*(len + 1));
	::WideCharToMultiByte(CP_ACP, 0, wUnicode, -1, str, len, NULL, NULL);

	std::string re = str;
	if (str)
	{
		delete[] str;
		str = NULL;
	}

	return re;
}

void CenterWin(HWND hWnd)
{
	int x = 0, y = 0;
	int w = 0, h = 0;

	RECT rect;
	::GetWindowRect(hWnd, &rect);

	w = rect.right - rect.left;
	h = rect.bottom - rect.top;

	x = (::GetSystemMetrics(SM_CXMAXIMIZED)) / 2;
	y = (::GetSystemMetrics(SM_CYMAXIMIZED)) / 2;

	::MoveWindow(hWnd, x, y, w, h, TRUE);
}

// ����ʮλ������2����λ������3.
std::size_t NumDigits(int number)
{
	std::size_t digitsSoFar = 1;

	while ((number /= 10) != 0)
		++digitsSoFar;
	
	return digitsSoFar;
}

//////////////////////////////////////////////////////////////////////////

// ����Ŀ¼����Ŀ¼���ǳ����ǰ��
//std::sort(files.begin(), files.end(), compDir);
static bool compDir(const std::string& a, const std::string& b)
{
	std::string pathA, nameA, pathB, nameB;

	ExtractPathName(a, pathA, nameA);
	ExtractPathName(b, pathB, nameB);

	if (pathA == pathB)
		return nameA < nameB;

	return pathA < pathB;
}

//////////////////////////////////////////////////////////////////////////


void PrintInfo(bool newLine, const char* format, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	buffer[sizeof(buffer) - 1] = 0;

	static bool lastNewLine = true;

	if (newLine)
	{
		if (!lastNewLine)
			printf("\n");

		printf("%s\n", buffer);
	}
	else
	{
#ifdef _WIN32
		char line[sizeof(buffer)];
		memset(line, ' ', sizeof(line));
		line[sizeof(line) - 1] = 0;
		printf("\r%s", line);
		printf("\r%s", buffer);
#else
		printf("\033[2K\r%s", buffer);
		fflush(stdout);
#endif
	}

	lastNewLine = newLine;
}

bool StartWith(const std::string& str, const std::string& sub)
{
	if (str.size() < sub.size())
		return false;

	for (size_t i = 0; i < sub.size(); i++)
	{
		if (str.at(i) != sub.at(i))
			return false;
	}

	return true;
}

bool EndWith(const std::string& str, const std::string& sub)
{
	size_t pos = str.rfind(sub);
	if (pos != std::string::npos && pos + sub.size() == str.size())
		return true;

	return false;
}

bool EndWith(const char* str, const char* sub)
{
	size_t str_len = strlen(str);
	size_t sub_len = strlen(sub);
	if (str_len >= sub_len)
	{
		if (memcmp(str + str_len - sub_len, sub, sub_len) == 0)
			return true;
	}

	return false;
}

void MakeStandardPath(std::string& str)
{
	size_t pos;
	while ((pos = str.find('\\')) != std::string::npos)
	{
		str.replace(pos, 1, 1, '/');
	}

	// ɾ���ظ�б��
	while ((pos = str.find("//")) != std::string::npos)
	{
		str.erase(pos, 1);
	}

	// ɾ����β��б��
	if (!str.empty() && str[str.size() - 1] == '/')
		str.erase(str.size() - 1, 1);
}

bool IsSkipFile(const std::string& filename)
{
	const char* re = "desktop\\.ini|Thumbs\\.db|filelist\\.txt|gdb\\.setup|gdbserver|version\\.txt|.*\\.bak|.*\\.ccb|.*\\.luac|.*\\.luac4|.*\\.luac8";

	std::regex skipList(re);

	return std::regex_match(filename, skipList);
}

void MakeRecursive(const char* dir)
{
	std::string str = dir;
	if (str.length() <= 1)
		return;

	for (char* p = &str[1]; *p; p++)
	{
		if (*p == '/' || *p == '\\')
		{
			*p = 0;
#ifdef _WIN32
			_mkdir(&str[0]);
#else
			mode_t processMask = umask(0);
			mkdir(&str[0], S_IRWXU | S_IRWXG | S_IRWXO);
			umask(processMask);
#endif
			*p = '/';
		}
	}
}

bool PathOrFileExists(const std::string& file)
{
	return _access(file.c_str(), 0) == 0 ? true : false;
}

bool IsFile(const std::string& file)
{
	struct _stat buf;
	if (_stat(file.c_str(), &buf))
	{
		// �����ڵ��ļ�
		return false;
	}

	if (buf.st_mode & _S_IFDIR)
		return false;

	if (buf.st_size == 0)
		return false;

	return true;
}

bool IsDir(const std::string& file)
{
	struct _stat buf;
	if (_stat(file.c_str(), &buf))
	{
		// �����ڵ�Ŀ¼
		return false;
	}

	if (buf.st_mode & _S_IFDIR)
		return true;

	return false;
}

int ReadFile(const std::string& file, char* buffer)
{
	FILE* f = NULL;
	int size = 0;

	do
	{
		f = fopen(file.c_str(), "rb");
		if (f == NULL)
			break;

		if (fseek(f, 0, SEEK_END) != 0)
			break;

		size = ftell(f);
		if (size <= 0 || size + 1 > BUFFER_SIZE)
		{
			size = 0;
			break;
		}

		if (fseek(f, 0, SEEK_SET) != 0)
		{
			size = 0;
			break;
		}

		if (fread(buffer, 1, size, f) != size)
		{
			size = 0;
			break;
		}

		buffer[size] = 0;
	} while (0);

	if (f)
		fclose(f);

	return size;
}

// �����ļ� 
bool SaveFile(const std::string& filename, const char* data, size_t size)
{
	MakeRecursive(filename.c_str());

	FILE* f = fopen(filename.c_str(), "wb");
	if (f == NULL)
		return false;

	bool ret = fwrite(data, 1, size, f) == size ? true : false;
	fclose(f);

	if (!ret)
	{
		remove(filename.c_str());
		return false;
	}

	return true;
}

bool ReplaceString(std::string& data, const std::string& src, const std::string& dst)
{
	if (StartWith(data, src))
	{
		data.replace(0, src.size(), dst);
		return true;
	}

	return false;
}

std::string DoMd5String(const void* data, size_t size)
{
	md5_state_t ctx;
	md5_init(&ctx);
	md5_append(&ctx, (const md5_byte_t*)data, (int)size);

	unsigned char digest[16];
	md5_finish(&ctx, digest);

	char result[33];
	
	for (unsigned int i = 0; i < 16; i++)
	{
		sprintf(&result[i * 2], "%02x", digest[i]);
	}

	return result;
}

void ExtractPathName(const std::string& pathname, std::string& path, std::string& name)
{
	size_t pos = pathname.find_last_of("\\/");
	if (pos == std::string::npos)
	{
		path == "";
		name = pathname;
	}
	else
	{
		path = pathname.substr(0, pos);
		name = pathname.substr(pos + 1);
	}
}

#ifdef _WIN32
void EnumFiles(const std::string& dir, std::vector<std::string>& files)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind = ::FindFirstFile((dir + "\\*.*").c_str(), &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		PrintInfo(true, "opendir %s Error", dir.c_str());
		return;
	}

	while (::FindNextFile(hFind, &findData))
	{
		if (findData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
			continue;

		// �����Ե㿪ͷ�ĵ��ļ���Ŀ¼ . .. .svn .git
		if (findData.cFileName[0] == '.')
			continue;

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			EnumFiles(dir + "/" + findData.cFileName, files);
		}
		else
		{
			if (g_64bits && !EndWith(findData.cFileName, ".lua"))
				continue;

			if (IsSkipFile(findData.cFileName))
			{
				PrintInfo(true, "skip %s", findData.cFileName);
				continue;
			}

			// ���Կ��ļ�
			if (findData.nFileSizeLow != 0 || findData.nFileSizeHigh != 0)
			{
				PrintInfo(true, "enum file : [%s/%s]", dir.c_str(), findData.cFileName);
				files.push_back(dir + "/" + findData.cFileName);
			}
		}
	}

	::FindClose(hFind);
}

void RemoveFiles(const std::string& dir)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind = ::FindFirstFile((dir + "\\*.*").c_str(), &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		PrintInfo(true, "removefile %s Error", dir.c_str());
		return;
	}

	while (::FindNextFile(hFind, &findData))
	{
		// �����Ե㿪ͷ�ĵ��ļ���Ŀ¼ . .. .svn .git
		if (findData.cFileName[0] == '.')
			continue;

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			RemoveFiles(dir + "/" + findData.cFileName);
		}
		else
		{
			remove((dir + "/" + findData.cFileName).c_str());
		}
	}

	::FindClose(hFind);

	_rmdir(dir.c_str());
}
#else
void EnumFiles(const std::string& dir, std::vector<std::string>& files)
{
	DIR* d = opendir(dir.c_str());
	if (d == NULL)
	{
		PrintInfo(true, "opendir %s Error", dir.c_str());
		return;
	}

	struct dirent *ent;
	while ((ent = readdir(d)) != NULL)
	{
		if (ent->d_name[0] == '.')
			continue;

		if (ent->d_type & DT_DIR)
		{
			EnumFiles(dir + "/" + ent->d_name, files);
		}
		else
		{
			if (g_64bits && !EndWith(ent->d_name, ".lua"))
				continue;

			if (IsSkipFile(ent->d_name))
			{
				PrintInfo(true, "skip %s\n", ent->d_name);
				continue;
			}

			std::string file = dir + "/" + ent->d_name;
			if (IsFile(file))
				files.push_back(dir + "/" + ent->d_name);
		}
	}

	closedir(d);
}

void RemoveFiles(const std::string& dir)
{
	DIR* d = opendir(dir.c_str());
	if (d == NULL)
	{
		printInfo(true, "removefile %s Error", dir.c_str());
		return;
	}

	struct dirent *ent;
	while ((ent = readdir(d)) != NULL)
	{
		if (ent->d_name[0] == '.')
			continue;

		if (ent->d_type & DT_DIR)
		{
			removeFiles(dir + "/" + ent->d_name);
		}
		else
		{
			remove((dir + "/" + ent->d_name).c_str());
		}
	}

	closedir(d);
	rmdir(dir.c_str());
}
#endif


// ѹ���󱣴棬��ѹ���ȳ������ֵ(maxRate)ʱ��ѹ��
int SavePackFile(const std::string& filename, const char* data, size_t size, char* compressBuffer)
{
	// ѹ��
	uLong csize = compressBound((uLong)size);
	int status = compress((Bytef*)compressBuffer, &csize, (Bytef*)data, (uLong)size);
	if (status != Z_OK)
		return -1;

	const char* ptr = compressBuffer;

	// ѹ����̫�ͣ���ѹ��
	if (csize + 16 >= size)
	{
		csize = (uLong)size;
		ptr = data;
	}

	// ����
	bool ret = SaveFile(filename, ptr, csize);
	if (!ret)
		return -1;

	return csize;
}

bool SaveToPatch(const std::string& src, const char* data, int size, char type, std::string& filelist)
{
	bool ret = false;

	// ����md5
	std::string md5 = DoMd5String(data, size);
	long csize = size;

	std::string strMd5Dir = "build/md5files";

	std::string dst = strMd5Dir + "/";
	dst.append(md5.c_str(), 2);
	dst.append(1, '/');
	dst.append(md5);
	dst.append(".pack");

	char* compressBuffer = new char[BUFFER_SIZE];

	csize = SavePackFile(dst, data, size, compressBuffer);

	if (csize > 0)
	{
		char buf[2048];
		std::string path;
		std::string name;
		ExtractPathName(src, path, name);
	
		static std::string lastDir;
		if (lastDir != path)
		{
			lastDir = path;

			sprintf(buf, "D, %s, %s, %d, %d\n", path.c_str(), "", 0, 0);
			filelist += buf;
		}

		sprintf(buf, "%c, %s, %s, %d, %d\n", type, name.c_str(), md5.c_str(), size, (int)csize);

		filelist += buf;
		ret = true;
	}

	delete[] compressBuffer;

	return ret;
}

// ѹ������list�ļ���ǰ��4���ֽڱ��泤��
bool SaveFileList(const std::string& fullpath, std::string& filelist, char* compressBuffer)
{
	uLong csize = compressBound((uLong)filelist.size());
	int status = compress((Bytef*)compressBuffer + 4, &csize, (const Bytef*)filelist.c_str(), (uLong)filelist.size());
	if (status != Z_OK)
		return false;

	*(unsigned int*)compressBuffer = (unsigned int)filelist.size();

	//
	bool ret = SaveFile(fullpath, compressBuffer, csize + sizeof(unsigned int));
	return ret;
}

bool LoadFileList(const std::string& fullpath, std::string& filelist, char* buffer, char* compressBuffer)
{
	int csize = ReadFile(fullpath, buffer);
	if (csize <= sizeof(unsigned int))
		return false;

	unsigned int size = *(unsigned int*)buffer;
	csize -= sizeof(unsigned int);

	uLongf uncompressSize = size;
	int status = uncompress((Bytef*)compressBuffer, &uncompressSize, (const unsigned char*)buffer + sizeof(unsigned int), csize);
	if (status != Z_OK || size != uncompressSize)
		return false;

	filelist = std::string(compressBuffer, size);
	return true;
}

int EncryptLua(int size, char* buffer)
{
	char key[] = "666";
	char keysig[] = "chen";
	xxtea_long ret_length = 0;
	unsigned char* result = xxtea_encrypt((unsigned char*)buffer, size, (unsigned char*)key, (xxtea_long)strlen(key), &ret_length);
	memcpy(buffer, keysig, strlen(keysig));
	memcpy(buffer + strlen(keysig), result, ret_length);
	free(result);

	return ret_length + (int)strlen(keysig);
}

int CompileLua(const char* name, int size, char* buffer, char* compressBuffer, bool isStripLua)
{
	size = compileLua(name, buffer, size, compressBuffer, isStripLua);
	if (size <= 0)
		return 0;

	std::swap(buffer, compressBuffer);
	return size;
}

bool UnpackFile(const std::string& file, char* buffer, char* compressBuffer)
{
	int size = ReadFile(file, buffer);
	if (size == 0)
		return false;

	uLongf destLen = BUFFER_SIZE;
	int err = uncompress((Bytef*)compressBuffer, &destLen, (Bytef*)buffer, (uLongf)size);
	if (err != Z_OK)
	{
		err = uncompress((Bytef*)compressBuffer, &destLen, (Bytef*)buffer + 4, (uLongf)size - 4);
	}

	unsigned char* ptr;
	if (err != Z_OK)
		ptr = (unsigned char*)buffer;
	else
	{
		ptr = (unsigned char*)compressBuffer;
		size = destLen;
	}

	//.............................

	return true;
}

#pragma warning(pop)
