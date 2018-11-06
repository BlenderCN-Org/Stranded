#ifndef __PUBUTIL_H__
#define __PUBUTIL_H__

#include "define.h"
#include <Nb30.h>
#include <IPHlpApi.h>

#ifdef _WIN32

#ifndef _WIN32_WINNT			// Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600		// Change this to the appropriate value to target other versions of Windows.
#endif

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <direct.h>
#include <conio.h>
#include <sys/stat.h>

#else	// _WIN32

#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define _access access
#define _stat stat
#define  _S_IFDIR S_IFDIR
#define  _chdir chdir
#define  _getcwd getcwd

#endif

#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
#include <regex>


extern bool g_64bits;


inline double square(double x)
{
	return x * x;
}

inline double getdist(const point &a, const point &b)
{
	return sqrt(square(a.x - b.x) + square(a.y - b.y) + square(a.z - b.z));
}

inline bool check(double a, double b, double c)
{
	return square(a) + square(b) < square(c);
}

// ��С�˼��
inline bool IsLittleEndian()
{
	int i = 1;
	return (*(char*)&i == 1);
}

int partition(rectype r[], int s, int t);
void quick_sort(rectype r[], int hs, int ht);

double get_rag_dis();

void Swap(char *a, char *b);

//��ת����
void Reverse(char *a, char *b);

//��һ������
bool Next_permutation(char a[]);

//��pszStr�����У�[nBegin, nEnd)���Ƿ����������±�ΪnEnd���������
bool IsSwap(char *pszStr, int nBegin, int nEnd);

//k��ʾ��ǰѡȡ���ڼ�������m��ʾ���ж�����.
void AllRange(char *pszStr, int k, int m);
//����ȫ����
void Foo(char *pszStr);

//#ifdef __cplusplus
//extern "C" void f1(void(*)(int));
//extern "C" typedef void FC(int);
//void f1(FC*);
//#endif

#ifdef LUA_API
void ShowLuaStack(lua_State *L);
void ShowLuaTraceBack(lua_State *L);
#endif // LUA_API

// create a two dimensional rows x cols array
int **Make2DArray(int rows, int cols);

//---------------------------------------------------------------
//function: 
//          GetRegValue ��ȡע�����ָ������ֵ
//Access:
//           public  
//Parameter:
//          [in] int nKeyType - ע���������ͣ�����Ĳ���ֻ������������ֵ��
//                              0:HKEY_CLASSES_ROOT
//                              1:HKEY_CURRENT_USER
//                              2:HKEY_LOCAL_MACHINE
//                              3:HKEY_USERS
//                              4:HKEY_PERFORMANCE_DATA
//                              5:HKEY_CURRENT_CONFIG
//                              6:HKEY_DYN_DATA
//                              7:HKEY_CURRENT_USER_LOCAL_SETTINGS
//                              8:HKEY_PERFORMANCE_TEXT
//                              9:HKEY_PERFORMANCE_NLSTEXT
//          [in] const std::string & strUrl - Ҫ���� �ļ���·��
//          [in] const std::string & strKey - ָ���ļ�
//Returns:
//          std::string - ָ������ֵ
//Remarks:
//
// Sample:
//		std::string strValue = GetRegValue(2, "SOFTWARE\\360Safe\\Liveup", "mid");
//		strValue��
//		"ebd1360403764c9d48c585ef93a6eacbd89ded596f043f78e54eb0adeba7251d"
//---------------------------------------------------------------
std::string GetRegValue(int nKeyType, const std::string& strUrl, const std::string& strKey);

// wstring -> string
std::string ws2s(const std::wstring& ws);

// string->wstring
std::wstring s2ws(const std::string& s);

//////////////////////////////////////////////////////////////////////////
/* match the last character of pattern first, and then match from the beginning. */
int NFind(char *string, char *pat);
// Knuth,Morris,Pratt ģʽƥ���㷨
int PMatch(char * string, char * pat, int failure[]);
// ʵ��ģʽƥ���㷨�����ʧ�亯��
void Fail(char *pat, int failure[]);
//////////////////////////////////////////////////////////////////////////

void GetMemoryAddress(void *ptr, char* pStr);

//��ȡMAC��ַ
int GetMAC(char *pMac);
//ȡ��ϵͳ��Ϣ
void GetHwInformation(char* szInfo);
//��ð汾��Ϣ
DWORD GetGameVersion(char* szVersion, DWORD& dwBuildID, DWORD& dwHightWordBuildID);
//���������Ϣ
void output_debug(char* format,...);

extern "C" {
	// ����豸-ѭ��չ��
	void DuffDevice(short& sFrom, short& sTo);
	// ��ȡCPU��Ϣ
	void _cdecl GetCPUID(char* sCpuID);
}

// UTF-8 ��ʽ����ת��
std::string LocalToUTF_8(const char* gb2312);
std::string UTF_8ToLocal(const char* szUtf_8);

// ���ֽ�ת��Unicode�ַ���
std::wstring LocalToUnicode(const char* gb2312);
std::string UnicodetoLocal(const wchar_t* wUnicode);

// ���д���
void CenterWin(HWND hWnd);

// ������
#if defined(WIN32)
class SLock
{
public:
	SLock() { ::InitializeCriticalSection(&m_lock); }
	~SLock() { ::DeleteCriticalSection(&m_lock); }
	VOID Lock() { ::EnterCriticalSection(&m_lock); }
	VOID Unlock() { ::LeaveCriticalSection(&m_lock); }
private:
	CRITICAL_SECTION m_lock;
};
#elif defined(__LINUX__)
class SLock
{
public:
	SLock() { pthread_mutex_init(&m_mutex, NULL); }
	~SLock() { pthread_mutex_destroy(&m_mutex); }
	VOID Lock() { pthread_mutex_lock(&m_mutex); }
	VOID Unlock() { pthread_mutex_unlock(&m_mutex); }
private:
	pthread_mutex_t m_mutex;
};
#endif

// ���ز��������ָ���
std::size_t NumDigits(int number);



void PrintInfo(bool newLine, const char* format, ...);
bool StartWith(const std::string& str, const std::string& sub);
bool EndWith(const std::string& str, const std::string& sub);
bool EndWith(const char* str, const char* sub);
void MakeStandardPath(std::string& str);
bool IsSkipFile(const std::string& filename);
void MakeRecursive(const char* dir);
bool PathOrFileExists(const std::string& file);
bool IsFile(const std::string& file);
bool IsDir(const std::string& file);
int ReadFile(const std::string& file, char* buffer);
bool SaveFile(const std::string& filename, const char* data, size_t size);
bool ReplaceString(std::string& data, const std::string& src, const std::string& dst);
std::string DoMd5String(const void* data, size_t size);
void ExtractPathName(const std::string& pathname, std::string& path, std::string& name);
void EnumFiles(const std::string& dir, std::vector<std::string>& files);
void RemoveFiles(const std::string& dir);
int SavePackFile(const std::string& filename, const char* data, size_t size, char* compressBuffer);
bool SaveToPatch(const std::string& src, const char* data, int size, char type, std::string& filelist);
bool SaveFileList(const std::string& fullpath, std::string& filelist, char* compressBuffer);
bool LoadFileList(const std::string& fullpath, std::string& filelist, char* buffer, char* compressBuffer);
int EncryptLua(int size, char* buffer);
int CompileLua(const char* name, int size, char* buffer, char* compressBuffer, bool isStripLua);
bool UnpackFile(const std::string& file, char* buffer, char* compressBuffer);

#endif