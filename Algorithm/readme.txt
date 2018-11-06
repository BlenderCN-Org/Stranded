1、大多数程序的大部分指令执行一次，或者至多只执行几次。如果一个程序的所有指令具有这个性质，我们说程序的运行时间为常量.
2、log N 当程序的运行时间为对数时，程序随着N的增长稍微变慢。通常在求解一个大规模问题的程序中，它把问题变成一些小的子问题，
	每一步都把问题的规模缩小一个几分之几，就会出现这样的运行时间。在我们关注的范围，可以认为这个运行时间小于一个大的常数。
	对数的基底会改变这个常数，但影响不会太大；当N=1000时，如果底数为10，则log N为3，或如果底数为2，则log N为10；当N=1000000
	时，log N只是前值得两倍。当N加倍时，log N只增加常量，只有N增加到N平方时，log N才会加倍.
3、N 当程序的运行时间为线性时，通常对每个输入元素只作了少量处理工作。当N=1000000时，运行时间也为1000000。当N加倍时，运行时间
	也随之加倍。这种情况对于一个必须处理N个输入(或者产生N个输出)的算法是最优的.
4、N log N 当把问题分解成效的子问题，且独立求解子问题，然后把这些子问题的解组合成原问题的解时，就会出现N log N 的运行时间。
	由于没有更好的形容词，我们只能说这种算法的运行时间为N log N。当N=1000000时，N log N 约为20000000。当N加倍时，运行时间
	略多于两倍。

/////////////////////////////////////////////////类成员函数指针//////////////////////////////////////////////////////////////////////

// 类成员函数指针(非普通指针) 32位编译环境 8 字节
//std::cout << "void (C::*)()大小: " << sizeof(void(C::*)()) << std::endl;
// 普通指针 32位编译环境 4字节
//std::cout << "Test<T>*大小: " << sizeof(Test<std::string>*) << std::endl;

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

///////////////////////////////////////////////////头文件定义、map删除////////////////////////////////////////////////////////////////////

/*
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

	

	//////////////////////////////shared_ptr 基本用法////////////////////////////////////////////
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

	///////////////////////////////////////shared_ptr 和 new///////////////////////////////////
	std::shared_ptr<int> p4(new int(1024));
	std::cout << *p4 << std::endl;

	std::shared_ptr<int> p5(new int(1024));
	process(p5);
	int v5 = *p5;
	std::cout << "v5: " << v5 << std::endl;

	// 第二次process p6时，shared_ptr的引用计数为1，当离开process的作用域时，会释放对应的内存，此时p6成为了悬挂指针。
	// 一旦将一个new表达式返回的指针交由shared_ptr管理之后，就不要再通过普通指针访问这块内存！
	//int *p6 = new int(1024);
	//process(std::shared_ptr<int>(p6));
	//int v6 = *p6;
	//std::cout << "v6: " << v6 << std::endl;

	// shared_ptr可以通过reset方法重置指向另一个对象，此时原对象的引用计数减一。
	p1.reset(new std::string("cpp11"));

	std::cout << "p1 cnt: " << p1.use_count() << "\t p2 cnt: " << p2.use_count() << "\t p3 cnt: " << p3.use_count() << std::endl;

	std::cout << *p1 << ' ' << *p2 << ' ' << *p3 << std::endl;

	// 可以定制一个deleter函数，用于在shared_ptr释放对象时调用。
	int *p7 = new int(1024);
	std::shared_ptr<int> p8(p7, print_at_delete);
	p8 = std::make_shared<int>(1025);

	// unique_ptr对于所指向的对象，正如其名字所示，是独占的。所以，不可以对unique_ptr进行拷贝、赋值等操作，但是可以通过release函数在unique_ptr之间转移控制权。
	std::unique_ptr<int> up1(new int(1024));
	std::cout << "up1: " << *up1 << std::endl;
	std::unique_ptr<int> up2(up1.release());
	std::cout << "up2: " << *up2 << std::endl;

	std::unique_ptr<int> up4(new int(1025));
	up4.reset(up2.release());
	std::cout << "up4: " << *up4 << std::endl;
		
	// 上述对于拷贝的限制，有两个特殊情况，即unique_ptr可以作为函数的返回值和参数使用，这时虽然也有隐含的拷贝存在，但是并非不可行的。
	auto up5 = clone(1024);
	std::cout << "up: " << *up5 << std::endl;
	process_unique_ptr(std::move(up5));
		
	// unique_ptr同样可以设置deleter，和shared_ptr不同的是，它需要在模板参数中指定deleter的类型。好在我们有decltype这个利器，不然写起来好麻烦。
	int *p9 = new int(1024);
	std::unique_ptr<int, decltype(print_at_delete) *> up6(p9, print_at_delete);
	std::unique_ptr<int> up7(new int(1025));
	up6.reset(up7.release());
	
	// weak_ptr一般和shared_ptr配合使用。它可以指向shared_ptr所指向的对象，但是却不增加对象的引用计数。
	// 这样就有可能出现weak_ptr所指向的对象实际上已经被释放了的情况。因此，weak_ptr有一个lock函数，尝试取回一个指向对象的shared_ptr。
	auto p10 = std::make_shared<int>(1024);
	std::weak_ptr<int> wp1(p10);
	std::cout << "p10 use_count: " << p10.use_count() << std::endl;
	//p10.reset(new in(1025));	// this will cause wp1.lock() return a false obj.
	std::shared_ptr<int> p11 = wp1.lock();
	if (p11)
		std::cout << "wp1: " << *p11 << " use count: " << p11.use_count() << std::endl;

	// 原来的begin和end返回的iterator是否是常量取决于对应的容器类型，但是有时，即使容器不是常量类型，我们也希望获得一个const_iterator，
	// 以避免不必要的修改行为。C++11新标准中提供了cbegin和cend函数，无论容器类型，都固定返回const_iterator。
	std::vector<int> c1 = { 0, 1, 2, 3, 4 };
	auto it1_1 = c1.begin();
	auto it1_2 = c1.cbegin();
	*it1_1 = 4;
	//*it1_2 = 5;	// wrong, const iterator's value can't be changed via this iterator.
	std::cout << *it1_1 << '\t' << *it1_2 << std::endl;

	// C++11新标准提供了begin和end函数，可以对普通数组使用，获得头指针和尾指针。
	int c2[] = { 0, 1, 2, 3, 4 };
	auto it2_1 = std::begin(c2);
	auto it2_2 = std::end(c2);
	while (it2_1 != it2_2)
	{
		std::cout << *it2_1 << '\t';
		it2_1++;
	}
	std::cout << std::endl;

	// C++11允许使用一个{}包围的初始值列表来进行复制。如果等号左侧是个容器，那么怎么赋值由容器决定。
	std::vector<int> c3;
	c3 = { 0, 1, 2, 3, 4 };
	for (auto it3_1 = c3.begin(); it3_1 != c3.end(); it3_1++)
		std::cout << *it3_1 << '\t';
	std::cout << std::endl;

	// C++11新标准中新增了initializer_list类型，其实在之前介绍初始化的那篇文章中，使用vector v = {0, 1, 2, 3, 4}这种初始化形式时，
	// 就隐式的使用了initializer_list：每当在程序中出现一段以{}包围的字面量时，就会自动构造一个initializer_list对象。
	// 另外，initializer_list的另一个作用就在于作为函数的形参，这样的函数可以方便的传入以{}包围的不定长列表：
	print_list({ 0, 1, 2, 3, 4 });
	print_list({ 0, 1, 2, 3, 4, 5 });

	// C++11标准中提供了定长数组容器array，相比于普通数组更安全、更易使用。array是定长数组，所以不支持诸如插入、删除等改变容器大小的操作，但是可以对元素进行赋值改变其值。
	std::array<int, 5> c4 = { 0, 1, 2, 3, 4 };
	c4[3] = 100;	// can't insert since the array size is fixed.
	for (auto it4_1 = c4.begin(); it4_1 != c4.end(); it4_1++)
	{
		std::cout << *it4_1 << '\t';
	}
	std::cout << std::endl;

	// C++11标准中增加了新的容器forward_list，提供了一个快速的、安全的单向链表实现。因为是单向链表，所以也就没有rbegin、rend一类的函数支持了。
	// 同样是因为单向链表的缘故，无法访问到给定元素的前驱，所以没有提供insert函数，而对应提供了一个insert_after函数，用于在给定元素之后插入节点。erase_after、emplace_after同理。
	std::forward_list<int> c5 = { 3, 4 };
	c5.push_front(2);
	c5.push_front(1);
	auto it5_1 = c5.before_begin();
	c5.insert_after(it5_1, 0);
	for (auto it5_2 = c5.begin(); it5_2 != c5.end(); it5_2++)
		std::cout << *it5_2 << '\t';
	std::cout << std::endl;

	// 新标准中提供了非成员版本的swap操作，此操作对array容器，会交换元素的值；对其他容器，则只交换容器的内部结构，并不进行元素值的拷贝操作，所以在这种情况下是非常迅速的。
	// 正因如此，当swap array后，原来array上的迭代器还依然指向原有元素，只是元素的值变了；而swap非array容器之后，原来容器上的迭代器将指向对方容器上的元素，而指向的元素的值却保持不变。
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

	// emplace操作将使用接受的参数构造一个对应容器中的元素，并插入容器中。这一点，使用普通的insert、push操作是做不到的。
	std::vector<TestData> c10;
	c10.emplace_back("yubo", 26, 100000.0);
	// c10.push_back("laowang", 56, 10.5);	// wrong. no 3 params push_back
	c10.push_back(TestData("laowang", 56, 10.5));
	std::cout << c10.size() << std::endl;

	// 一般可变长容器会预先多分配一部分内存出来，以备在后续增加元素时，不用每次都申请内存。所以有size和capacity之分。size是当前容器中存有元素的个数，
	// 而capacity则是在不重新申请内存的情况下，当前可存放元素的最大数目。而shrink_to_fit就表示将capacity中的多余部分退回，使其回到size大小。但是，这个函数的具体效果要依赖于编译器的实现……
	std::vector<int> c11;
	for (int i = 0; i < 24; i++)
		c11.push_back(i);
	std::cout << c11.size() << '\t' << c11.capacity() << std::endl;
	c11.shrink_to_fit();
	std::cout << c11.size() << '\t' << c11.capacity() << std::endl;

	// C++11新标准中引入了对map、set等关联容器的无序版本，叫做unorderer_map\ / unordered_set。
	// 无序关联容器不使用键值的比较操作来组织元素顺序，而是使用哈希。这样在某些元素顺序不重要的情况下，效率更高。
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

	// 熟悉python的程序员应该对tuple都不陌生，C++11中也引入了这一数据结构，用于方便的将不同类型的值组合起来。
	// 可以通过如下方式，获取tuple中的元素、tuple的长度等：
	//tuple<int, string, vector<int>> c14 = {1, "tuple", {0, 1, 2, 3, 4}};	// wrong. must explicit initialize
	std::tuple<int, std::string, std::vector<int>> c14{ 1, "tuple", {0, 1, 2, 3, 4} };
	std::get<0>(c14) = 2;
	typedef decltype(c14) ctype;
	std::size_t sz = std::tuple_size<ctype>::value;
	std::cout << std::get<0>(c14) << '\t' << std::get<1>(c14) << '\t' << std::get<2>(c14)[0] << '\t' << sz << std::endl;

	// 新标准中，sizeof可以直接用于获取Class::member的大小，而不用通过Class的实例。
	std::cout << sizeof(CTestClass::m_iMember) << "\t" << sizeof(CTestClass::m_cMember) << std::endl;

	// 新标准中，可以通过=default强制编译器生成一个默认constructor。
	//CTestClass tc;	// may cause error if no default construct.
	//std::cout << tc.m_iMember << '\t' << (short)tc.m_cMember << std::endl;

	// delegate constructor
	// 新标准中，可以在初始化列表中将一个constructor初始化的工作委托给另一个constructor。
	CTestClass tc2(2);
	std::cout << tc2.m_iMember << '\t' << tc2.m_cMember << std::endl;

	// 新标准中，allocator.construct可以使用任意的构造函数。
	//std::allocator<CTestClass> alloc;
	//auto p = alloc.allocate(10);
	//alloc.construct(p, 10);
	//std::cout << p->m_iMember << '\t' << p->m_cMember << std::endl;

	//////////////////////////////////////////////////////////////////////////

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 字节对齐 /Zp编译开关
#pragma pack(push, 1)
/// 或者分开
#pragma pack(push)	// 将当前pack设置压栈保存
#pragma pack(2)		// 默认是8 结构体成员偏移量 offsetof(item) = min(n, sizeof(item))

#pragma pack(pop)	// 恢复先前的pack设置

////////////////////////////////////////////////////const 规则//////////////////////////////////////////////////////////////////////
class Widget
{
public:
	Widget();
	Widget(const Widget& rhs);
	Widget& operator=(const Widget& rhs);
private:
	static const int NumTurns = 5;	// 常量声明式
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
	// 内置类型返回引用, 成员函数constness重载
	const char& operator[](std::size_t position) const
	{
		return text[position];
	}

	// 这里 const 和 non-const 成员函数有着实质等价的实现
	char& operator[](std::size_t position)	// 只调用 const op[]，避免代码重复
	{
		return
			const_cast<char&>(							// 将 op[] 返回值的 const 转除
				static_cast<const TextBlock&>(*this)	// 为 *this 加上 const (避免重复递归调用自身，安全转型)
					[position]							// 调用 const op[]
		);
	}
private:
	std::string text;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////