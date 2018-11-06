//*************Windows API �ļ�����**************************
#include <windows.h>
#include <cstdio>
//*************C++��׼�� �ļ�����****************************
#include <iostream>
#include <fstream>
//*************ʹ��CRT��Ŀ��ַ��汾 �ļ�����****************
#include <clocale>
//*************<climits>ȡ���������ֵ����Сֵ***************
#include <climits>
//*************<cctype>�ַ����ַ���***************
#include <cctype>
//*************<cstring>�ַ��������***************
#include <cstring>
//*************��׼C++ string��***************
#include <string>
//*************��׼C++ �ַ�����std::stringstream***
#include <sstream>
//*************ʱ�����****************************
#include <ctime>
//************C++STL����****************************
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <bitset>
#include <queue>
#include <stack>
//************C++STL�����㷨****************************
#include <algorithm>


#include <comutil.h>

//VC2008���иÿ�
//#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "comsupp.lib")



using namespace std;


//*************Windows API �ļ�����(��ѡ)**************************
//��������һ���ļ�
//HANDLE CreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
//д�ļ�
//BOOL WriteFile( HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
//���ļ�
//BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);

//*************ʹ��CRT(C����ʱ�ڿ�) �ļ�����*****************
//��������һ���ļ�
//FILE *fopen(const char *filename, const char *mode);
//�ر��ļ�
//int fclose(FILE *stream);
//���ļ����ָ����ʽ���ı�
//int fprintf(FILE *stream, const char *format, ...);
//���ļ���ȡָ����ʽ���ı�
//int fscanf(FILE *stream, const char *format, ...);

//*************ʹ��CRT��Ŀ��ַ��汾 �ļ�����(���)****************
//FILE *_wfopen(const wchar_t *filename, const wchar_t *mode);
//int fwprintf(FILE *stream, const wchar_t *format, ...);
//int fwscanf(FILE *stream, const wchar_t *format, ...);

//*************ʹ��CRT��İ�ȫ�汾(�߰汾VC֧��) �ļ�����(���)******************
// errno_t fopen_s(FILE **pFile, const char *filename, const char *mode);
// int fprintf_s(FILE *stream, const char *format, ...);
// int fwprintf_s(FILE *stream, const wchar_t *format, ...);
// int fscanf_s(FILE *stream, const char *format, ...);
// int fwscanf_s(FILE *stream, const wchar_t *format, ...);

//*************ʹ��MFC/ATL �ļ�����(��ѡ)*****************
//д���ļ�
// CFile file;
// if(file.Open(_T("MFC_ATL.txt"), CFile::modeCreate|CFile::modeWrite))
// {
// 	char szLine[256] = "���㶹����";
// 	file.Write(szLine, sizeof(szLine));
// 	file.Close();
// }
// //��ȡ�ļ�
// if(file.Open(_T("MFC_ATL.txt"), CFile::modeRead))
// {
// 	char szLine[256] = "���㶹";
// 	if(file.Read(szLine, 256) != 0)
// 	{
// 		printf("%s\r\n", szLine);
// 	}
// 	file.Close();
// }

//********�ַ������ת��*******************
//int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, CPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
//int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);

//************C++STL����****************************
//********��������*************
//����vector
//template <class Type, class Allocator=allocator<Type>> class vector
//�б�list
//template <class Type, class Allocator=allocator<Type>> class list
//˫�˶���deque
//template <class Type, class Allocator=allocator<Type>> class deque
//�������������� ����queue�����ȼ�����priority_queue
//template <class Type, class Container=deque<Type>> class queue
/*****���ȼ����п���ָ����ʹ�õ��������ͣ��Լ����ԱȽ�Ԫ�ش�С�ĺ���������(Compare)��
Ĭ�����������vector��Ϊʵ������������Ԫ�ذ��մӴ�С��������(Compare=less)****/
//template <class Type, class Container=vector<Type>, class Compare=less<typename Container::value_type>> class priority_queue
//�������������� ջstack
//template <class Type, class Container=deque<Type>> class stack
//********��������*************
//ӳ��map������ӳ��multimap
//template <class Key, class Type, class Traits=less<Key>, class Allocator=allocator<pair <const Key, Type>>> class map
//template <class Key, class Type, class Traits=less<Key>, class Allocator=allocator<pair <const Key, Type>>> class multimap
//����set�����ؼ���multiset
//template <class Key, class Traits=less<Key>, class Allocator=allocator<Key>> class set
//template <class Key, class Compare=less<Key>, class Allocator=allocator<Key>> class multiset
//λ����bitset
//template <size_t N> class bitset;






//************C++STL�����㷨****************************
//�ۼ���(����������)
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

	//��������operator()����
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
	char		msg[] = TEXT("���㶹����");
	char		szLine[256] = {0};
	BOOL		bResult;


	//***********************************************************
	//*************Windows API �ļ�����**************************
	//д���ļ�
	hFile = CreateFile(TEXT("windows.txt"), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		WriteFile(hFile, msg, sizeof(msg)-1, &nBytes, NULL);
		CloseHandle(hFile);
	}
	//��ȡ�ļ�
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
	//*************C++��׼�� �ļ�����(���)****************************
	//д���ļ�
	ofstream out(TEXT("stdcpp.txt"));
	out << TEXT("���㶹����");
	out.close();

	//��ȡ�ļ�
	ifstream in(TEXT("stdcpp.txt"));
	in.getline(szLine, 256);
	cout << szLine << endl;

	//***********************************************************
	//*************ʹ��CRT(C����ʱ�ڿ�) �ļ�����*****************
	//д���ļ�
	FILE *fp = fopen("crt.txt", "w");
	fprintf(fp, "���㶹����");
	fclose(fp);

	//��ȡ�ļ�
	fp = fopen("crt.txt", "r");
	fscanf(fp, "%s", szLine);
	printf("%s\r\n", szLine);
	fclose(fp);

	//***********************************************************
	//*************ʹ��CRT��Ŀ��ַ��汾 �ļ�����****************
	//����ǰʱ���趨Ϊ"chs"�������ü������ı��뷽ʽ
	setlocale(LC_ALL, "chs");

	//д���ļ�
	FILE *wfp = _wfopen(L"wCRT.txt", L"w,ccs=UNICODE");
	fwprintf(wfp, L"%s", L"���㶹����");
	fclose(wfp);

	//��ȡ�ļ�
	wfp = _wfopen(L"wCRT.txt", L"r,ccs=UNICODE");
	wchar_t wszLine[256];
	fwscanf(wfp, L"%s", wszLine);
	wprintf(L"%s\r\n", wszLine);
	fclose(wfp);

	//***********************************************************
	//*************ʹ��CRT��İ�ȫ�汾 �ļ�����******************
	//��ǿ����У�顢��������С��⡢��ʽ������У��...
	//д���ļ�
// 	FILE *sfp;
// 	fopen_s(&sfp, "sCRT.txt", "w");
// 	fprintf_s(sfp, "���㶹��");
// 	fclose(sfp);
// 
// 	//��ȡ�ļ�
// 	fopen_s(&sfp, "sCRT.txt", "r");
// 	fscanf_s(sfp, "%s", szLine, 256);
// 	printf_s("%s\r\n", szLine);
// 	fclose(sfp);
	//***********************************************************

	//***********************************************************
	//struct�ṹ���������ݳ�Ա���ж���(����)���ڴ�ռ�
	//union���ϵ��������ݳ�Ա��ʼ��ͬһ����ַ. (���һЩ�ֽ���Ϲ���)
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
	//VC�ֽڳ��ȹ̶����������� __intN�ֽڳ��ȹ̶�ΪNλ����N/8�ֽ�
// 	__int8 nSmall;
// 	__int16 nMedium;
// 	__int32 nLarge;
// 	__int64 nHuge;
// 	LONGLONG lTooLong;

	//***********************************************************
	//*************<climits>ȡ���������ֵ����Сֵ***************
	printf("MB_LEN_MAX: %ld\r\n", MB_LEN_MAX);	//���ֽ��ַ�����������ֽڳ���
	printf("INT_MAX: %X\r\n", INT_MAX);	//int�����ֵ
	printf("UINT_MAX: %X\r\n", UINT_MAX);	//unsigned int�����ֵ
	printf("_I64_MAX: %X\r\n", _I64_MAX);	//__int64�����ֵ

	//***********************************************************
	//*************<cctype>�ַ����ַ���***************
	char src[] = "Hello World!";
	char *cp = src;
	while(*cp)
	{
		//Сд��ĸת�ɴ�д
		if(islower(*cp))
			printf("%c", toupper(*cp));
		else
		{
			//��д��ĸת��Сд
			if(isupper(*cp))
				printf("%c", tolower(*cp));
			//�����ַ�
			else
				printf("@");
		}
		cp++;
	}
	//***********************************************************

	//*************<cstring>�ַ��������***************
	//ͳ��һ�������е�Ԫ����ĸ
	char Vowel[] = "Returns a pointer to the first occurrence in strl of any of the characters that are part of str2, or a null pointer if there are no matches.";
	char key[] = "aeiou";
	char *cpFind;
	int i = 0;
	
	printf("%s\r\n", Vowel);
	printf("Ԫ����ĸ: ");
	//��ȡĿ���ַ����е������ַ���Դ�ַ����е�ָ��λ��
	cpFind = strpbrk(Vowel, key);
	
	while(cpFind != NULL)
	{
		i++;
		printf("%c", *cpFind);
		cpFind = strpbrk(cpFind+1, key);
	}

	printf("\r\n����%d��Ԫ��\r\n", i);
	//***********************************************************

	//*************��׼C++ string��***************
	//��char���͵��ַ���������string����wchar_t���ַ����͵��ַ���������wstring
	//*************string��Ϊbasic_stringģ����ı���***************
	//typedef basic_string<char, char_traits<char>, allocator<char>> string;
	//typedef basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t>> wstring;
	//***********************************************************
	//֧�� + = += [] == != > < >= <=������
	//append ׷�Ӳ�����������operator+=
	//assign ��ֵ������������operator=
	//at ��ȡָ��λ�õ��ַ���������operator[]
	//c_str �����ַ������ָ��
	//capacity ��ȡ�ַ���������
	//clear ����ַ���
	//compare ������һ���ַ������бȽ�
	//empty �ж��ַ����Ƿ�Ϊ��
	//erase ɾ��ָ��λ�õ��ַ�
	//insert ��ָ��λ�ò����µ��ַ�
	//length �����ַ����ĳ���
	//substr ��ȡָ��λ�õ��Ӵ�
	//swap ������һ���ַ������н���
	//find �����Ӵ�
	//find_first_not of ���ҵ�һ����ƥ���λ��
	//find_first_of ���ҵ�һ��ƥ���λ��
	//find_last_not_of �������һ����ƥ���λ��
	//find_last_of �������һ��ƥ���λ��
	//replace �滻ָ��λ�õ��ַ���
	//***********************************************************
	//1������һ���յ�string
// 	string s1;
// 	//2�� ʹ���ַ����鹹��һ��string
// 	string s2("hello");
// 	string s3 = "hello";
// 	//3������һ��string���ƹ��챾string
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
	cout << endl << "����" << j << "��Ԫ��" << endl;

	//***********************************************************
	//����VARIANT��ʾһ������
	VARIANT var;
	var.vt = VT_I4;		//ָ����������. vt��Ա�趨��������������
	var.lVal = 100;		//��ֵ. ��Ӧ��union��Ա�趨��������
	//����VARIANT��ʾһ������ֵ
	VARIANT var1;
	var1.vt = VT_BOOL;
	var1.boolVal = VARIANT_TRUE;
	//����VARIANT����һ���ַ���
	VARIANT var2;
	var2.vt = VT_BSTR;
	var2.bstrVal = SysAllocString(L"Hello, world!");

	//***********************************************************
	//*************��׼C++ �ַ�����std::stringstream***
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
	//����� <cstdlib>***************
	srand((unsigned)time(0));	//һ�����õ�ǰʱ���������Ϊ������ӣ���ʼ������������

	for(int k=0; k<5; k++)
		printf("%d\r\n", rand());

	//***********************************************************
	//�ַ������ת��
	char s1[] = "����ABC";
	wchar_t s2[] = L"�����⴫";

	//ʹ��xtoyת��
	//s2-->p1
	//��һ�ε���, ����MBCS�ַ����ֽڳ���
	int len1 = WideCharToMultiByte(CP_ACP,0, s2/*���ַ���ָ��*/, sizeof(s2)/*���ַ�������*/, NULL, 0/*����0��ʾ����ת������ַ��ռ�*/, NULL, NULL);

	char *p1 = new char[len1];

	WideCharToMultiByte(CP_ACP, 0, s2/*���ַ���ָ��*/, sizeof(s2)/*���ַ�������*/, p1/*ת������ַ���ָ��*/, len1/*�����ռ��С*/, NULL, NULL);

	printf("%s\r\n", p1);

	delete [] p1;

	//s1-->p2
	//��һ�ε��ã�������ַ����ֽڳ���
	int len2 = MultiByteToWideChar(CP_ACP, 0, s1/*MBCS�ַ���ָ��*/, sizeof(s1)/*MBCS�ַ�������*/, NULL, 0);

	wchar_t *p2 = new wchar_t[len2];

	MultiByteToWideChar(CP_ACP, 0, s1/*MBCS�ַ���ָ��*/, sizeof(s1)/*MBCS�ַ�������*/, p2/*ת������ַ���ָ��*/, len2/*�����ռ��С*/);

	printf("%S\r\n", p2);

	delete [] p2;

	//ʹ��BSTR���ת��
	_bstr_t bs1(s1);
	_bstr_t bs2(s2);

	char *bp1 = bs2;

	printf("%s\r\n", bp1);

	wchar_t *bp2 = bs1;

	//���ַ�����Ҫ"%S"
	printf("%S\r\n", bp2);

	//***********************************************************
	

	//***********************************************************
	//************C++STL����****************************
	//STL������3���������(container)��ָʾ��(iterator)���㷨(algorithm)
	//STL���������(adapter)����������(function object)
	//std::vector()
	vector<int> v1(3, 0);	//3��Ԫ�أ���ʼ��Ϊ3��0
	
	for (int n=0; n<v1.size(); n++)
	{
		printf("v1[%d]=%d\r\n", n, v1[n]);
	}

	//************C++STL���� map****************************
	map<string, string> stlMap;
	stlMap[":)"] = "����";
	stlMap[":("] = "������";
	stlMap["@_@"] = "����";

	cout << stlMap[":)"] << endl;

	//************C++STL���� bitset****************************
	//��֧��ָʾ��������N��Ӧ�ڴ������λ��
	bitset<32> bitsetN;
	bitset<sizeof(int)> bitsetN1;

	//����ֵ�ĵ�7λ����Ϊ1
	int iSet = 1;
	iSet |= 128;
	//
	bitsetN = 1;
	cout << bitsetN << endl;
	bitsetN[7] = 1;
	cout << bitsetN << endl;

	//************C++STL���� queue****************************
	queue < int, vector<int> > qv;
	//************C++STL���� stack****************************
	stack < int, vector<int> > sv;

	//***********************************************************
	//************C++STL�����㷨****************************
	//vector
	vector<int> records1;
	
	for(int b=0; b<10; b++)
		records1.push_back(b+1);

	int iCount = 0, iTotal = 0;
	Accumulater<int> sum(&iCount, &iTotal);
	//STL�㷨 for_each()
	for_each(records1.begin()/*ָʾ��*/, records1.end()/*ָʾ��*/, sum/*��������*/);

	printf("��ͳ��%d���������ܺ�Ϊ%d\r\n", iCount, iTotal);

	//Ҳʹ����C++����
	int records2[10];
	for (int a=0; a<10; a++)
		records2[a] = a+1;

	iCount = iTotal = 0;
	for_each(records2, records2+10, sum);

	printf("��ͳ��%d���������ܺ�Ϊ%d\r\n", iCount, iTotal);

	return 0;
}