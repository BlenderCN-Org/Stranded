//*************Windows API 文件操作**************************
#include <windows.h>
#include <cstdio>
//*************C++标准库 文件操作****************************
#include <iostream>
#include <fstream>
//*************使用CRT库的宽字符版本 文件操作****************
#include <clocale>
//*************<climits>取整数的最大值、最小值***************
#include <climits>
//*************<cctype>字符、字符串***************
#include <cctype>
//*************<cstring>字符数组操作***************
#include <cstring>
//*************标准C++ string类***************
#include <string>
//*************标准C++ 字符串流std::stringstream***
#include <sstream>
//*************时间操作****************************
#include <ctime>
//************C++STL容器****************************
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <bitset>
#include <queue>
#include <stack>
//************C++STL泛型算法****************************
#include <algorithm>


#include <comutil.h>

//VC2008带有该库
//#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "comsupp.lib")



using namespace std;


//*************Windows API 文件操作(首选)**************************
//创建、打开一个文件
//HANDLE CreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
//写文件
//BOOL WriteFile( HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
//读文件
//BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);

//*************使用CRT(C运行时期库) 文件操作*****************
//创建、打开一个文件
//FILE *fopen(const char *filename, const char *mode);
//关闭文件
//int fclose(FILE *stream);
//向文件输出指定格式的文本
//int fprintf(FILE *stream, const char *format, ...);
//从文件读取指定格式的文本
//int fscanf(FILE *stream, const char *format, ...);

//*************使用CRT库的宽字符版本 文件操作(其次)****************
//FILE *_wfopen(const wchar_t *filename, const wchar_t *mode);
//int fwprintf(FILE *stream, const wchar_t *format, ...);
//int fwscanf(FILE *stream, const wchar_t *format, ...);

//*************使用CRT库的安全版本(高版本VC支持) 文件操作(其次)******************
// errno_t fopen_s(FILE **pFile, const char *filename, const char *mode);
// int fprintf_s(FILE *stream, const char *format, ...);
// int fwprintf_s(FILE *stream, const wchar_t *format, ...);
// int fscanf_s(FILE *stream, const char *format, ...);
// int fwscanf_s(FILE *stream, const wchar_t *format, ...);

//*************使用MFC/ATL 文件操作(首选)*****************
//写入文件
// CFile file;
// if(file.Open(_T("MFC_ATL.txt"), CFile::modeCreate|CFile::modeWrite))
// {
// 	char szLine[256] = "茴香豆的茴";
// 	file.Write(szLine, sizeof(szLine));
// 	file.Close();
// }
// //读取文件
// if(file.Open(_T("MFC_ATL.txt"), CFile::modeRead))
// {
// 	char szLine[256] = "茴香豆";
// 	if(file.Read(szLine, 256) != 0)
// 	{
// 		printf("%s\r\n", szLine);
// 	}
// 	file.Close();
// }

//********字符编码的转换*******************
//int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, CPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
//int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);

//************C++STL容器****************************
//********序列容器*************
//向量vector
//template <class Type, class Allocator=allocator<Type>> class vector
//列表list
//template <class Type, class Allocator=allocator<Type>> class list
//双端队列deque
//template <class Type, class Allocator=allocator<Type>> class deque
//序列容器适配器 队列queue、优先级队列priority_queue
//template <class Type, class Container=deque<Type>> class queue
/*****优先级队列可以指定其使用的容器类型，以及用以比较元素大小的函数对象类(Compare)，
默认情况，采用vector作为实现容器，并将元素按照从大到小降序排列(Compare=less)****/
//template <class Type, class Container=vector<Type>, class Compare=less<typename Container::value_type>> class priority_queue
//序列容器适配器 栈stack
//template <class Type, class Container=deque<Type>> class stack
//********关联容器*************
//映射map、多重映射multimap
//template <class Key, class Type, class Traits=less<Key>, class Allocator=allocator<pair <const Key, Type>>> class map
//template <class Key, class Type, class Traits=less<Key>, class Allocator=allocator<pair <const Key, Type>>> class multimap
//集合set、多重集合multiset
//template <class Key, class Traits=less<Key>, class Allocator=allocator<Key>> class set
//template <class Key, class Compare=less<Key>, class Allocator=allocator<Key>> class multiset
//位集合bitset
//template <size_t N> class bitset;






//************C++STL泛型算法****************************
//累加器(函数对象类)
template <typename T> class Accumulater
{
private:
	int *_count;
	T *_total;
public:
	Accumulater(int *count, T *total)
	{
		_count = count;
		_total = total;
	}

	//必须重载operator()函数
	void operator()(T i)
	{
		(*_count)++;
		(*_total) += i;
	}
};

int main(int argc, char *argv[])
{
	HANDLE		hFile;
	DWORD		nBytes;
	char		msg[] = TEXT("茴香豆的茴");
	char		szLine[256] = {0};
	BOOL		bResult;


	//***********************************************************
	//*************Windows API 文件操作**************************
	//写入文件
	hFile = CreateFile(TEXT("windows.txt"), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, msg, sizeof(msg)-1, &nBytes, NULL);
		CloseHandle(hFile);
	}
	//读取文件
	hFile = CreateFile(TEXT("windows.txt"), GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		bResult = ReadFile(hFile, szLine, sizeof(szLine), &nBytes, NULL);
		if(nBytes != 0)
		{
			printf("%s\r\n", szLine);
		}
		CloseHandle(hFile);
	}
	//***********************************************************
	//*************C++标准库 文件操作(其次)****************************
	//写入文件
	ofstream out(TEXT("stdcpp.txt"));
	out << TEXT("茴香豆的茴");
	out.close();

	//读取文件
	ifstream in(TEXT("stdcpp.txt"));
	in.getline(szLine, 256);
	cout << szLine << endl;

	//***********************************************************
	//*************使用CRT(C运行时期库) 文件操作*****************
	//写入文件
	FILE *fp = fopen("crt.txt", "w");
	fprintf(fp, "茴香豆的茴");
	fclose(fp);

	//读取文件
	fp = fopen("crt.txt", "r");
	fscanf(fp, "%s", szLine);
	printf("%s\r\n", szLine);
	fclose(fp);

	//***********************************************************
	//*************使用CRT库的宽字符版本 文件操作****************
	//将当前时区设定为"chs"，即采用简体中文编码方式
	setlocale(LC_ALL, "chs");

	//写入文件
	FILE *wfp = _wfopen(L"wCRT.txt", L"w,ccs=UNICODE");
	fwprintf(wfp, L"%s", L"茴香豆的茴");
	fclose(wfp);

	//读取文件
	wfp = _wfopen(L"wCRT.txt", L"r,ccs=UNICODE");
	wchar_t wszLine[256];
	fwscanf(wfp, L"%s", wszLine);
	wprintf(L"%s\r\n", wszLine);
	fclose(wfp);

	//***********************************************************
	//*************使用CRT库的安全版本 文件操作******************
	//增强参数校验、缓冲区大小检测、格式化参数校验...
	//写入文件
// 	FILE *sfp;
// 	fopen_s(&sfp, "sCRT.txt", "w");
// 	fprintf_s(sfp, "茴香豆的");
// 	fclose(sfp);
// 
// 	//读取文件
// 	fopen_s(&sfp, "sCRT.txt", "r");
// 	fscanf_s(sfp, "%s", szLine, 256);
// 	printf_s("%s\r\n", szLine);
// 	fclose(sfp);
	//***********************************************************

	//***********************************************************
	//struct结构的所有数据成员具有独立(连续)的内存空间
	//union联合的所有数据成员起始于同一个地址. (完成一些字节组合功能)
	struct RGB
	{
		unsigned char blue;
		unsigned char green;
		unsigned char red;
	};
	union COLOR
	{
		unsigned int value;
		RGB rgb;
	};

	COLOR c = {0xFF8844};
	cout << hex << (int)c.rgb.red << "," << (int)c.rgb.green << "," << (int)c.rgb.blue <<endl;
	cout << &c.value << "," << &c.rgb << endl;
	//***********************************************************

	//***********************************************************
	//VC字节长度固定的整数类型 __intN字节长度固定为N位，即N/8字节
// 	__int8 nSmall;
// 	__int16 nMedium;
// 	__int32 nLarge;
// 	__int64 nHuge;
// 	LONGLONG lTooLong;

	//***********************************************************
	//*************<climits>取整数的最大值、最小值***************
	printf("MB_LEN_MAX: %ld\r\n", MB_LEN_MAX);	//多字节字符常量的最大字节长度
	printf("INT_MAX: %X\r\n", INT_MAX);	//int的最大值
	printf("UINT_MAX: %X\r\n", UINT_MAX);	//unsigned int的最大值
	printf("_I64_MAX: %X\r\n", _I64_MAX);	//__int64的最大值

	//***********************************************************
	//*************<cctype>字符、字符串***************
	char src[] = "Hello World!";
	char *cp = src;
	while(*cp)
	{
		//小写字母转成大写
		if(islower(*cp))
			printf("%c", toupper(*cp));
		else
		{
			//大写字母转成小写
			if(isupper(*cp))
				printf("%c", tolower(*cp));
			//其它字符
			else
				printf("@");
		}
		cp++;
	}
	//***********************************************************

	//*************<cstring>字符数组操作***************
	//统计一段文字中的元音字母
	char Vowel[] = "Returns a pointer to the first occurrence in strl of any of the characters that are part of str2, or a null pointer if there are no matches.";
	char key[] = "aeiou";
	char *cpFind;
	int i = 0;
	
	printf("%s\r\n", Vowel);
	printf("元音字母: ");
	//获取目标字符串中的任意字符在源字符串中的指针位置
	cpFind = strpbrk(Vowel, key);
	
	while(cpFind != NULL)
	{
		i++;
		printf("%c", *cpFind);
		cpFind = strpbrk(cpFind+1, key);
	}

	printf("\r\n共有%d个元音\r\n", i);
	//***********************************************************

	//*************标准C++ string类***************
	//用char类型的字符串，采用string；用wchar_t宽字符类型的字符串，采用wstring
	//*************string类为basic_string模板类的别名***************
	//typedef basic_string<char, char_traits<char>, allocator<char>> string;
	//typedef basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t>> wstring;
	//***********************************************************
	//支持 + = += [] == != > < >= <=操作符
	//append 追加操作，类似于operator+=
	//assign 赋值操作，类似于operator=
	//at 获取指定位置的字符，类似于operator[]
	//c_str 返回字符数组的指针
	//capacity 获取字符串的容量
	//clear 清空字符串
	//compare 与另外一个字符串进行比较
	//empty 判断字符串是否为空
	//erase 删除指定位置的字符
	//insert 在指定位置插入新的字符
	//length 返回字符串的长度
	//substr 获取指定位置的子串
	//swap 与另外一个字符串进行交换
	//find 搜索子串
	//find_first_not of 查找第一个不匹配的位置
	//find_first_of 查找第一个匹配的位置
	//find_last_not_of 查找最后一个不匹配的位置
	//find_last_of 查找最后一个匹配的位置
	//replace 替换指定位置的字符串
	//***********************************************************
	//1、构造一个空的string
// 	string s1;
// 	//2、 使用字符数组构造一个string
// 	string s2("hello");
// 	string s3 = "hello";
// 	//3、从另一个string复制构造本string
// 	string s4(s2);
// 	string s5 = s2;

	string strSrc("Returns a pointer to the first occurrence in strl of any of the characters that are part of str2, or a null pointer if there are no matches.");
	size_t uiCp;
	int j = 0;

	uiCp = strSrc.find_first_of("aeiou");

	while(uiCp != string::npos)
	{
		cout << strSrc.at(uiCp) << " ";
		uiCp = strSrc.find_first_of("aeiou", uiCp+1);
		j++;
	}
	cout << endl << "共有" << j << "个元音" << endl;

	//***********************************************************
	//变体VARIANT表示一个整型
	VARIANT var;
	var.vt = VT_I4;		//指明整型数据. vt成员设定变量的数据类型
	var.lVal = 100;		//赋值. 对应的union成员设定数据内容
	//变体VARIANT表示一个布尔值
	VARIANT var1;
	var1.vt = VT_BOOL;
	var1.boolVal = VARIANT_TRUE;
	//利用VARIANT保存一个字符串
	VARIANT var2;
	var2.vt = VT_BSTR;
	var2.bstrVal = SysAllocString(L"Hello, world!");

	//***********************************************************
	//*************标准C++ 字符串流std::stringstream***
	ostringstream ost;
	ost << "100 120" << endl;
	
	double chinese, math;
	istringstream ins(ost.str());
	ins >> chinese >> math;

	printf("chinese=[%f] math=[%f]\n", chinese, math);

	stringstream strsm;
	strsm << "100 120";

	strsm >> chinese >> math;

	printf("chinese=[%f] math=[%f]\n", chinese, math);

	//***********************************************************
	//随机数 <cstdlib>***************
	srand((unsigned)time(0));	//一般利用当前时间毫秒数作为随机种子，初始化发生器序列

	for(int k=0; k<5; k++)
		printf("%d\r\n", rand());

	//***********************************************************
	//字符编码的转换
	char s1[] = "中文ABC";
	wchar_t s2[] = L"武林外传";

	//使用xtoy转换
	//s2-->p1
	//第一次调用, 计算MBCS字符串字节长度
	int len1 = WideCharToMultiByte(CP_ACP,0, s2/*宽字符串指针*/, sizeof(s2)/*宽字符串长度*/, NULL, 0/*参数0表示计算转换后的字符空间*/, NULL, NULL);

	char *p1 = new char[len1];

	WideCharToMultiByte(CP_ACP, 0, s2/*宽字符串指针*/, sizeof(s2)/*宽字符串长度*/, p1/*转换后的字符串指针*/, len1/*给出空间大小*/, NULL, NULL);

	printf("%s\r\n", p1);

	delete [] p1;

	//s1-->p2
	//第一次调用，计算宽字符串字节长度
	int len2 = MultiByteToWideChar(CP_ACP, 0, s1/*MBCS字符串指针*/, sizeof(s1)/*MBCS字符串长度*/, NULL, 0);

	wchar_t *p2 = new wchar_t[len2];

	MultiByteToWideChar(CP_ACP, 0, s1/*MBCS字符串指针*/, sizeof(s1)/*MBCS字符串长度*/, p2/*转换后的字符串指针*/, len2/*给出空间大小*/);

	printf("%S\r\n", p2);

	delete [] p2;

	//使用BSTR完成转换
	_bstr_t bs1(s1);
	_bstr_t bs2(s2);

	char *bp1 = bs2;

	printf("%s\r\n", bp1);

	wchar_t *bp2 = bs1;

	//宽字符串需要"%S"
	printf("%S\r\n", bp2);

	//***********************************************************
	

	//***********************************************************
	//************C++STL容器****************************
	//STL包括：3组件：容器(container)、指示器(iterator)、算法(algorithm)
	//STL概念：适配器(adapter)、函数对象(function object)
	//std::vector()
	vector<int> v1(3, 0);	//3个元素，初始化为3个0
	
	for (int n=0; n<v1.size(); n++)
	{
		printf("v1[%d]=%d\r\n", n, v1[n]);
	}

	//************C++STL容器 map****************************
	map<string, string> stlMap;
	stlMap[":)"] = "高兴";
	stlMap[":("] = "不高兴";
	stlMap["@_@"] = "惊讶";

	cout << stlMap[":)"] << endl;

	//************C++STL容器 bitset****************************
	//不支持指示器操作，N对应于待处理的位数
	bitset<32> bitsetN;
	bitset<sizeof(int)> bitsetN1;

	//将数值的第7位设置为1
	int iSet = 1;
	iSet |= 128;
	//
	bitsetN = 1;
	cout << bitsetN << endl;
	bitsetN[7] = 1;
	cout << bitsetN << endl;

	//************C++STL容器 queue****************************
	queue < int, vector<int> > qv;
	//************C++STL容器 stack****************************
	stack < int, vector<int> > sv;

	//***********************************************************
	//************C++STL泛型算法****************************
	//vector
	vector<int> records1;
	
	for(int b=0; b<10; b++)
		records1.push_back(b+1);

	int iCount = 0, iTotal = 0;
	Accumulater<int> sum(&iCount, &iTotal);
	//STL算法 for_each()
	for_each(records1.begin()/*指示器*/, records1.end()/*指示器*/, sum/*函数对象*/);

	printf("共统计%d个整数，总和为%d\r\n", iCount, iTotal);

	//也使用于C++数组
	int records2[10];
	for (int a=0; a<10; a++)
		records2[a] = a+1;

	iCount = iTotal = 0;
	for_each(records2, records2+10, sum);

	printf("共统计%d个整数，总和为%d\r\n", iCount, iTotal);

	return 0;
}