1�����������Ĵ󲿷�ָ��ִ��һ�Σ���������ִֻ�м��Ρ����һ�����������ָ�����������ʣ�����˵���������ʱ��Ϊ����.
2��log N �����������ʱ��Ϊ����ʱ����������N��������΢������ͨ�������һ�����ģ����ĳ����У�����������һЩС�������⣬
	ÿһ����������Ĺ�ģ��Сһ������֮�����ͻ��������������ʱ�䡣�����ǹ�ע�ķ�Χ��������Ϊ�������ʱ��С��һ����ĳ�����
	�����Ļ��׻�ı������������Ӱ�첻��̫�󣻵�N=1000ʱ���������Ϊ10����log NΪ3�����������Ϊ2����log NΪ10����N=1000000
	ʱ��log Nֻ��ǰֵ����������N�ӱ�ʱ��log Nֻ���ӳ�����ֻ��N���ӵ�Nƽ��ʱ��log N�Ż�ӱ�.
3��N �����������ʱ��Ϊ����ʱ��ͨ����ÿ������Ԫ��ֻ������������������N=1000000ʱ������ʱ��ҲΪ1000000����N�ӱ�ʱ������ʱ��
	Ҳ��֮�ӱ��������������һ�����봦��N������(���߲���N�����)���㷨�����ŵ�.
4��N log N ��������ֽ��Ч�������⣬�Ҷ�����������⣬Ȼ�����Щ������Ľ���ϳ�ԭ����Ľ�ʱ���ͻ����N log N ������ʱ�䡣
	����û�и��õ����ݴʣ�����ֻ��˵�����㷨������ʱ��ΪN log N����N=1000000ʱ��N log N ԼΪ20000000����N�ӱ�ʱ������ʱ��
	�Զ���������

/////////////////////////////////////////////////���Ա����ָ��//////////////////////////////////////////////////////////////////////

// ���Ա����ָ��(����ָͨ��) 32λ���뻷�� 8 �ֽ�
//std::cout << "void (C::*)()��С: " << sizeof(void(C::*)()) << std::endl;
// ��ָͨ�� 32λ���뻷�� 4�ֽ�
//std::cout << "Test<T>*��С: " << sizeof(Test<std::string>*) << std::endl;

///////////////////////////////////////////////SSE////////////////////////////////////////////////////////////////////////

/*
#include "../Common/TimeCheck.h"

#pragma comment(lib, "../lib/Common.lib")

#pragma comment(linker, "/nodefaultlib:libc.lib")
#pragma comment(linker, "/nodefaultlib:libci.lib")
#ifdef _DEBUG
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#else
#pragma comment(linker, "/nodefaultlib:libcmtd.lib")
#endif

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")

__declspec(align(16)) struct matrix4x4
{
	float data[16];
};

__declspec(align(16)) struct vector4
{
	void mul_aligned(vector4& dest, const vector4& sour, const matrix4x4& m4)
	{
		__asm
		{
			MOV ESI, m4;
			MOV EDX, sour;
			MOV EDI, dest;
		}
	}

	float x, y, z, w;
};

__declspec(align(16)) struct quaternion
{
	float x, y, z, w;
};*/

///////////////////////////////////////////////////ͷ�ļ����塢mapɾ��////////////////////////////////////////////////////////////////////

/*
 a.h

class CA;

extern CA* a;

/////����ǰ//////

a.cpp

#include "CA.h"

CA* a = NULL;

///////����ǰ////////

/////////////////////////////////

b.h

class CB
{

};

extern CB g_CB;

b.cpp

CB g_CB;

///////����ǰ////////

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
		//����ɾ��������ɾ����ǰ��㣬��ָ������һ�����
		m_map.erase(iter++);
	}
	else
	{
		//���������㣬ָ������һ�����
		iter++;
	}
}
 */

///////////////////////////////////////////////C++11////////////////////////////////////////////////////////////////////////

#include <time.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <xmmintrin.h>
#include <memory>
#include <array>
#include <forward_list>
#include <unordered_map>
#include <map>
#include <tuple>

#include <stdio.h>
#include <stdlib.h>

void process(std::shared_ptr<int> ptr)
{
	std::cout << "in process use_count: " << ptr.use_count() << std::endl;
}

void print_at_delete(int *p)
{
	std::cout << "deleting ..." << p << '\t' << *p << std::endl;
	delete p;
}

std::unique_ptr<int> clone(int p)
{
	return std::unique_ptr<int>(new int(p));
}

void process_unique_ptr(std::unique_ptr<int> up)
{
	std::cout << "process unique ptr: " << *up << std::endl;
}

void print_list(std::initializer_list<int> il)
{
	for (auto it = il.begin(); it != il.end(); it++)
	{
		std::cout << *it << '\t';
		//*it = 100;	// wrong. initializer_list element is read-only.
	}
	std::cout << std::endl;
}

class TestData
{
public:
	TestData(std::string name, int age, double salary) : name(name), age(age), salary(salary)
	{}
private:
	std::string name;
	int age;
	double salary;
};

class CTestClass
{
public:
	CTestClass() = default;
	CTestClass(const int i, const char c) : m_iMember(i), m_cMember(c) {}
	CTestClass(const int i) : CTestClass(i, 0) { m_cMember = 'T'; }
public:
	int m_iMember;
	char m_cMember;
};

int main(int argc, char* argv[])
{
	const time_t t = time(NULL);
#pragma warning(push)
#pragma warning(disable:4996)
	std::cerr << "Started on " << ctime(&t) << "\n";
#pragma warning(pop)
	
	int step = 2;
	int i = -step;
	std::vector<int> seq(10);
	std::generate(seq.begin(), seq.end(),
		[i, step]() mutable
		{
			return (i += step);
		});

	auto f1 =
		[](int x, int y) -> int
		{
			return (x + y);
		};

	

	//////////////////////////////shared_ptr �����÷�////////////////////////////////////////////
	std::shared_ptr<std::string> p1 = std::make_shared<std::string>("");
	if (p1 && p1->empty())
		*p1 = "hello";

	auto p2 = std::make_shared<std::string>("world");
	std::cout << *p1 << ' ' << *p2 << std::endl;

	std::cout << "p1 cnt: " << p1.use_count() << "\t p2 cnt: " << p2.use_count() << std::endl;

	auto p3 = p2;
	std::cout << "p1 cnt: " << p1.use_count() << "\t p2 cnt: " << p2.use_count() << "\t p3 cnt: " << p3.use_count() << std::endl;

	std::cout << *p1 << ' ' << *p2 << ' ' << *p3 << std::endl;

	p2 = p1;
	std::cout << "p1 cnt: " << p1.use_count() << "\t p2 cnt: " << p2.use_count() << "\t p3 cnt: " << p3.use_count() << std::endl;

	std::cout << *p1 << ' ' << *p2 << ' ' << *p3 << std::endl;

	///////////////////////////////////////shared_ptr �� new///////////////////////////////////
	std::shared_ptr<int> p4(new int(1024));
	std::cout << *p4 << std::endl;

	std::shared_ptr<int> p5(new int(1024));
	process(p5);
	int v5 = *p5;
	std::cout << "v5: " << v5 << std::endl;

	// �ڶ���process p6ʱ��shared_ptr�����ü���Ϊ1�����뿪process��������ʱ�����ͷŶ�Ӧ���ڴ棬��ʱp6��Ϊ������ָ�롣
	// һ����һ��new���ʽ���ص�ָ�뽻��shared_ptr����֮�󣬾Ͳ�Ҫ��ͨ����ָͨ���������ڴ棡
	//int *p6 = new int(1024);
	//process(std::shared_ptr<int>(p6));
	//int v6 = *p6;
	//std::cout << "v6: " << v6 << std::endl;

	// shared_ptr����ͨ��reset��������ָ����һ�����󣬴�ʱԭ��������ü�����һ��
	p1.reset(new std::string("cpp11"));

	std::cout << "p1 cnt: " << p1.use_count() << "\t p2 cnt: " << p2.use_count() << "\t p3 cnt: " << p3.use_count() << std::endl;

	std::cout << *p1 << ' ' << *p2 << ' ' << *p3 << std::endl;

	// ���Զ���һ��deleter������������shared_ptr�ͷŶ���ʱ���á�
	int *p7 = new int(1024);
	std::shared_ptr<int> p8(p7, print_at_delete);
	p8 = std::make_shared<int>(1025);

	// unique_ptr������ָ��Ķ���������������ʾ���Ƕ�ռ�ġ����ԣ������Զ�unique_ptr���п�������ֵ�Ȳ��������ǿ���ͨ��release������unique_ptr֮��ת�ƿ���Ȩ��
	std::unique_ptr<int> up1(new int(1024));
	std::cout << "up1: " << *up1 << std::endl;
	std::unique_ptr<int> up2(up1.release());
	std::cout << "up2: " << *up2 << std::endl;

	std::unique_ptr<int> up4(new int(1025));
	up4.reset(up2.release());
	std::cout << "up4: " << *up4 << std::endl;
		
	// �������ڿ��������ƣ������������������unique_ptr������Ϊ�����ķ���ֵ�Ͳ���ʹ�ã���ʱ��ȻҲ�������Ŀ������ڣ����ǲ��ǲ����еġ�
	auto up5 = clone(1024);
	std::cout << "up: " << *up5 << std::endl;
	process_unique_ptr(std::move(up5));
		
	// unique_ptrͬ����������deleter����shared_ptr��ͬ���ǣ�����Ҫ��ģ�������ָ��deleter�����͡�����������decltype�����������Ȼд�������鷳��
	int *p9 = new int(1024);
	std::unique_ptr<int, decltype(print_at_delete) *> up6(p9, print_at_delete);
	std::unique_ptr<int> up7(new int(1025));
	up6.reset(up7.release());
	
	// weak_ptrһ���shared_ptr���ʹ�á�������ָ��shared_ptr��ָ��Ķ��󣬵���ȴ�����Ӷ�������ü�����
	// �������п��ܳ���weak_ptr��ָ��Ķ���ʵ�����Ѿ����ͷ��˵��������ˣ�weak_ptr��һ��lock����������ȡ��һ��ָ������shared_ptr��
	auto p10 = std::make_shared<int>(1024);
	std::weak_ptr<int> wp1(p10);
	std::cout << "p10 use_count: " << p10.use_count() << std::endl;
	//p10.reset(new in(1025));	// this will cause wp1.lock() return a false obj.
	std::shared_ptr<int> p11 = wp1.lock();
	if (p11)
		std::cout << "wp1: " << *p11 << " use count: " << p11.use_count() << std::endl;

	// ԭ����begin��end���ص�iterator�Ƿ��ǳ���ȡ���ڶ�Ӧ���������ͣ�������ʱ����ʹ�������ǳ������ͣ�����Ҳϣ�����һ��const_iterator��
	// �Ա��ⲻ��Ҫ���޸���Ϊ��C++11�±�׼���ṩ��cbegin��cend�����������������ͣ����̶�����const_iterator��
	std::vector<int> c1 = { 0, 1, 2, 3, 4 };
	auto it1_1 = c1.begin();
	auto it1_2 = c1.cbegin();
	*it1_1 = 4;
	//*it1_2 = 5;	// wrong, const iterator's value can't be changed via this iterator.
	std::cout << *it1_1 << '\t' << *it1_2 << std::endl;

	// C++11�±�׼�ṩ��begin��end���������Զ���ͨ����ʹ�ã����ͷָ���βָ�롣
	int c2[] = { 0, 1, 2, 3, 4 };
	auto it2_1 = std::begin(c2);
	auto it2_2 = std::end(c2);
	while (it2_1 != it2_2)
	{
		std::cout << *it2_1 << '\t';
		it2_1++;
	}
	std::cout << std::endl;

	// C++11����ʹ��һ��{}��Χ�ĳ�ʼֵ�б������и��ơ�����Ⱥ�����Ǹ���������ô��ô��ֵ������������
	std::vector<int> c3;
	c3 = { 0, 1, 2, 3, 4 };
	for (auto it3_1 = c3.begin(); it3_1 != c3.end(); it3_1++)
		std::cout << *it3_1 << '\t';
	std::cout << std::endl;

	// C++11�±�׼��������initializer_list���ͣ���ʵ��֮ǰ���ܳ�ʼ������ƪ�����У�ʹ��vector v = {0, 1, 2, 3, 4}���ֳ�ʼ����ʽʱ��
	// ����ʽ��ʹ����initializer_list��ÿ���ڳ����г���һ����{}��Χ��������ʱ���ͻ��Զ�����һ��initializer_list����
	// ���⣬initializer_list����һ�����þ�������Ϊ�������βΣ������ĺ������Է���Ĵ�����{}��Χ�Ĳ������б�
	print_list({ 0, 1, 2, 3, 4 });
	print_list({ 0, 1, 2, 3, 4, 5 });

	// C++11��׼���ṩ�˶�����������array���������ͨ�������ȫ������ʹ�á�array�Ƕ������飬���Բ�֧��������롢ɾ���ȸı�������С�Ĳ��������ǿ��Զ�Ԫ�ؽ��и�ֵ�ı���ֵ��
	std::array<int, 5> c4 = { 0, 1, 2, 3, 4 };
	c4[3] = 100;	// can't insert since the array size is fixed.
	for (auto it4_1 = c4.begin(); it4_1 != c4.end(); it4_1++)
	{
		std::cout << *it4_1 << '\t';
	}
	std::cout << std::endl;

	// C++11��׼���������µ�����forward_list���ṩ��һ�����ٵġ���ȫ�ĵ�������ʵ�֡���Ϊ�ǵ�����������Ҳ��û��rbegin��rendһ��ĺ���֧���ˡ�
	// ͬ������Ϊ���������Ե�ʣ��޷����ʵ�����Ԫ�ص�ǰ��������û���ṩinsert����������Ӧ�ṩ��һ��insert_after�����������ڸ���Ԫ��֮�����ڵ㡣erase_after��emplace_afterͬ��
	std::forward_list<int> c5 = { 3, 4 };
	c5.push_front(2);
	c5.push_front(1);
	auto it5_1 = c5.before_begin();
	c5.insert_after(it5_1, 0);
	for (auto it5_2 = c5.begin(); it5_2 != c5.end(); it5_2++)
		std::cout << *it5_2 << '\t';
	std::cout << std::endl;

	// �±�׼���ṩ�˷ǳ�Ա�汾��swap�������˲�����array�������ύ��Ԫ�ص�ֵ����������������ֻ�����������ڲ��ṹ����������Ԫ��ֵ�Ŀ�������������������������Ƿǳ�Ѹ�ٵġ�
	// ������ˣ���swap array��ԭ��array�ϵĵ���������Ȼָ��ԭ��Ԫ�أ�ֻ��Ԫ�ص�ֵ���ˣ���swap��array����֮��ԭ�������ϵĵ�������ָ��Է������ϵ�Ԫ�أ���ָ���Ԫ�ص�ֵȴ���ֲ��䡣
	std::vector<int> c6 = { 0, 1, 2, 3, 4 };
	std::vector<int> c7 = { 5, 6, 7, 8, 9 };
	auto it6_1 = c6.begin();
	auto it7_1 = c7.begin();
	std::swap(c6, c7);
	for (auto it6_2 = c6.begin(); it6_2 != c6.end(); it6_2++)
		std::cout << *it6_2 << '\t';
	std::cout << std::endl;

	for (auto it7_2 = c7.begin(); it7_2 != c7.end(); it7_2++)
		std::cout << *it7_2 << '\t';
	std::cout << std::endl;

	std::cout << (it6_1 == c7.begin()) << '\t' << (it7_1 == c6.begin()) << std::endl;

	std::array<int, 5> c8 = { 0, 1, 2, 3, 4 };
	std::array<int, 5> c9 = { 5, 6, 7, 8, 9 };
	auto it8_1 = c8.begin();
	auto it9_1 = c9.begin();
	std::swap(c8, c9);
	std::cout << (it8_1 == c8.begin()) << '\t' << (it9_1 == c9.begin()) << std::endl;

	// emplace������ʹ�ý��ܵĲ�������һ����Ӧ�����е�Ԫ�أ������������С���һ�㣬ʹ����ͨ��insert��push�������������ġ�
	std::vector<TestData> c10;
	c10.emplace_back("yubo", 26, 100000.0);
	// c10.push_back("laowang", 56, 10.5);	// wrong. no 3 params push_back
	c10.push_back(TestData("laowang", 56, 10.5));
	std::cout << c10.size() << std::endl;

	// һ��ɱ䳤������Ԥ�ȶ����һ�����ڴ�������Ա��ں�������Ԫ��ʱ������ÿ�ζ������ڴ档������size��capacity֮�֡�size�ǵ�ǰ�����д���Ԫ�صĸ�����
	// ��capacity�����ڲ����������ڴ������£���ǰ�ɴ��Ԫ�ص������Ŀ����shrink_to_fit�ͱ�ʾ��capacity�еĶ��ಿ���˻أ�ʹ��ص�size��С�����ǣ���������ľ���Ч��Ҫ�����ڱ�������ʵ�֡���
	std::vector<int> c11;
	for (int i = 0; i < 24; i++)
		c11.push_back(i);
	std::cout << c11.size() << '\t' << c11.capacity() << std::endl;
	c11.shrink_to_fit();
	std::cout << c11.size() << '\t' << c11.capacity() << std::endl;

	// C++11�±�׼�������˶�map��set�ȹ�������������汾������unorderer_map\ / unordered_set��
	// �������������ʹ�ü�ֵ�ıȽϲ�������֯Ԫ��˳�򣬶���ʹ�ù�ϣ��������ĳЩԪ��˳����Ҫ������£�Ч�ʸ��ߡ�
	std::unordered_map<std::string, int> c12;
	std::map<std::string, int> c13;
	std::string string_keys[5] = { "aaa", "bbb", "ccc", "ddd", "eee" };
	for (int i = 0; i < 5; i++)
	{
		c12[string_keys[i]] = i;
		c13[string_keys[i]] = i;
	}

	std::cout << "normal map:\n";
	for (auto it13 = c13.begin(); it13 != c13.end(); it13++)
		std::cout << it13->first << ":" << it13->second << '\t';
	std::cout << std::endl;

	std::cout << "unordered map:\n";
	for (auto it12 = c12.begin(); it12 != c12.end(); it12++)
		std::cout << it12->first << ":" << it12->second << '\t';
	std::cout << std::endl;

	// ��Ϥpython�ĳ���ԱӦ�ö�tuple����İ����C++11��Ҳ��������һ���ݽṹ�����ڷ���Ľ���ͬ���͵�ֵ���������
	// ����ͨ�����·�ʽ����ȡtuple�е�Ԫ�ء�tuple�ĳ��ȵȣ�
	//tuple<int, string, vector<int>> c14 = {1, "tuple", {0, 1, 2, 3, 4}};	// wrong. must explicit initialize
	std::tuple<int, std::string, std::vector<int>> c14{ 1, "tuple", {0, 1, 2, 3, 4} };
	std::get<0>(c14) = 2;
	typedef decltype(c14) ctype;
	std::size_t sz = std::tuple_size<ctype>::value;
	std::cout << std::get<0>(c14) << '\t' << std::get<1>(c14) << '\t' << std::get<2>(c14)[0] << '\t' << sz << std::endl;

	// �±�׼�У�sizeof����ֱ�����ڻ�ȡClass::member�Ĵ�С��������ͨ��Class��ʵ����
	std::cout << sizeof(CTestClass::m_iMember) << "\t" << sizeof(CTestClass::m_cMember) << std::endl;

	// �±�׼�У�����ͨ��=defaultǿ�Ʊ���������һ��Ĭ��constructor��
	//CTestClass tc;	// may cause error if no default construct.
	//std::cout << tc.m_iMember << '\t' << (short)tc.m_cMember << std::endl;

	// delegate constructor
	// �±�׼�У������ڳ�ʼ���б��н�һ��constructor��ʼ���Ĺ���ί�и���һ��constructor��
	CTestClass tc2(2);
	std::cout << tc2.m_iMember << '\t' << tc2.m_cMember << std::endl;

	// �±�׼�У�allocator.construct����ʹ������Ĺ��캯����
	//std::allocator<CTestClass> alloc;
	//auto p = alloc.allocate(10);
	//alloc.construct(p, 10);
	//std::cout << p->m_iMember << '\t' << p->m_cMember << std::endl;

	//////////////////////////////////////////////////////////////////////////

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �ֽڶ��� /Zp���뿪��
#pragma pack(push, 1)
/// ���߷ֿ�
#pragma pack(push)	// ����ǰpack����ѹջ����
#pragma pack(2)		// Ĭ����8 �ṹ���Աƫ���� offsetof(item) = min(n, sizeof(item))

#pragma pack(pop)	// �ָ���ǰ��pack����

////////////////////////////////////////////////////const ����//////////////////////////////////////////////////////////////////////
class Widget
{
public:
	Widget();
	Widget(const Widget& rhs);
	Widget& operator=(const Widget& rhs);
private:
	static const int NumTurns = 5;	// ��������ʽ
	int scores[NumTurns];
};

const Widget operator* (const Widget& lhs, const Widget& rhs);

class CTextBlock
{
public:
	std::size_t length() const;
private:
	char* pText;
	mutable std::size_t textLength;
	mutable bool lengthIsValid;
};

std::size_t CTextBlock::length() const
{
	if (!lengthIsValid)
	{
		textLength = std::strlen(pText);
		lengthIsValid = true;
	}
	return textLength;
}

class TextBlock
{
public:
	// �������ͷ�������, ��Ա����constness����
	const char& operator[](std::size_t position) const
	{
		return text[position];
	}

	// ���� const �� non-const ��Ա��������ʵ�ʵȼ۵�ʵ��
	char& operator[](std::size_t position)	// ֻ���� const op[]����������ظ�
	{
		return
			const_cast<char&>(							// �� op[] ����ֵ�� const ת��
				static_cast<const TextBlock&>(*this)	// Ϊ *this ���� const (�����ظ��ݹ����������ȫת��)
					[position]							// ���� const op[]
		);
	}
private:
	std::string text;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////