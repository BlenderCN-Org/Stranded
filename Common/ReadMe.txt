========================================================================
    静态库：Common 项目概述
========================================================================

应用程序向导已为您创建了此 Common 库项目。

没有为此项目创建源文件。


Common.vcxproj
    这是使用应用程序向导生成的 VC++ 项目的主项目文件，
    其中包含生成该文件的 Visual C++ 
    的版本信息，以及有关使用应用程序向导选择的平台、配置和项目功能的信息。

Common.vcxproj.filters
    这是使用“应用程序向导”生成的 VC++ 项目筛选器文件。 
    它包含有关项目文件与筛选器之间的关联信息。 在 IDE 
    中，通过这种关联，在特定节点下以分组形式显示具有相似扩展名的文件。
    例如，“.cpp”文件与“源文件”筛选器关联。

/////////////////////////////////////////////////////////////////////////////
其他注释：

应用程序向导使用“TODO:”注释来指示应添加或自定义的源代码部分。

/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////向动态链接库添加类的核心代码////////////////////////////////////////////////////////////////////
#ifdef CREATEOPENCLDLL_EXPORTS
#define CREATEOPENCLDLL_API __declspec(dllexport)
#else
#define CREATEOPENCLDLL_API __declspec(dllimport)
#endif

// 为显式链接做准备的导出函数:
extern "C"
{
	CREATEOPENCLDLL_API CreateOpenCLDLL* GetCreateOpenCLDLL(void);
	typedef CreateOpenCLDLL* (*PFNGetCreateOpenCLDLL)(void);
}

// （1）隐式链接
方法1：
	在工程中添加头文件createopencldll.h到新的工程目录。在debug或者release文件中复制buildopencldll.lib到新的工程目录
	在项目中，属性->连接器->输入->附加依赖项中，添加buildopencldll.lib.将buildopencldll.dll复制到新工程的debug或者release目录下。
	测试代码如下：
		#include "createopencldll.h"  
  
		int main()  
		{  
			CreateOpenCLDLL createopencldll;  
			createopencldll.Calcaute();  
			return 0;  
		}

方法2：
	在方法1的基础下，不添加buildopencldll.lib到附加依赖项中，使用代码的方式添加，即在原来的代码下多添加：#pragmacomment(lib, “buildopencldll.lib”)
	代码如下：
	#pragma comment(lib, "buildopencldll.lib")  
  
	#include "createopencldll.h"  
  
	int main()  
	{  
		CreateOpenCLDLL createopencldll;  
		createopencldll.Calcaute();  
		return 0;  
	}

// （2）显式链接
	先定义虚基类(IKPerson)，然后到基类(CKChinese)等的实现. 这种方法不需要createopencldll.lib，将createopencldll.dll复制到debug或者release目录。
	// 调用代码
	callclassExportDll.cpp

	#include <windows.h>    
	#include <cstdio>    
	#include "createopencldll.h"  
	int main()  
	{  
		HMODULE hDll = ::LoadLibrary("buildopencldll.dll");  
		if (NULL != hDll)  
		{  
			PFNGetCreateOpenCLDLL pFun = (PFNGetCreateOpenCLDLL)::GetProcAddress(hDll, "GetCreateOpenCLDLL");  
			if (NULL != pFun)  
			{  
				CreateOpenCLDLL* createOpenCLDLL = (*pFun)();  
				if (NULL != createOpenCLDLL)  
				{  
					createOpenCLDLL->Calcaute();  
					delete createOpenCLDLL;  
				}  
			}  
			::FreeLibrary(hDll);  
		}  
		return 0;  
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Rvalue references - 右值引用
左值是一个指向某内存空间的表达式，并且我们可以用&操作符获得该内存空间的地址。右值就是非左值的表达式。
void foo(X& x); // 左值引用重载
void foo(X&& x); // 右值引用重载

X x;	//　x 左值
X foobar();	// foobar() 右值

foo(x); // 参数是左值，调用foo(X&)
foo(foobar()); // 参数是右值，调用foo(X&&)

右值引用允许函数在编译期根据参数是左值还是右值来建立分支。
针对右值引用重载的拷贝构造函数、和赋值运算符，以用来实现move语义：
X& X::operator=(X const & rhs); // classical implementation
X& X::operator=(X&& rhs)
{
  // Move semantics: exchange content between this and rhs
  return *this;
}
如果你实现了void foo(X&);，但是没有实现void foo(X&&);，那么和以前一样foo的参数只能是左值。如果实现了void foo(X const &);，但是没有实现void foo(X&&);，仍和以前一样，foo的参数既可以是左值也可以是右值。
唯一能够区分左值和右值的办法就是实现void foo(X&&);。最后，如果只实现了实现void foo(X&&);，但却没有实现void foo(X&);和void foo(X const &);，那么foo的参数将只能是右值。

右值引用类型既可以被当作左值也可以被当作右值，判断的标准是，如果它有名字，那就是左值，否则就是右值。
x被当作左值
void foo(X&& x)
{
  X anotherX = x; // 调用X(X const & rhs)
}
没有名字的右值引用被当作右值处理
X&& goo();
X x = goo(); // 调用X(X&& rhs)，goo的返回值没有名字

Derived(Derived&& rhs) 
  : Base(std::move(rhs)) // good, calls Base(Base&& rhs)
{
  // Derived-specific stuff
}

返回值优化（return value optimization）: 编译器会在函数返回的地方直接创建对象，而不是在函数中创建后再复制出来。
return value optimization和copy elision

c++11标准里面引入了引用叠加规则：
A& & => A&
A& && => A&
A&& & => A&
A&& && => A&&

模版参数推导规则:
版是接受一个右值引用作为模版参数的函数模版。
template<typename T> void foo(T&&);
1. 当函数foo的实参是一个A类型的左值时，T的类型是A&。再根据引用叠加规则判断，最后参数的实际类型是A&。
2. 当foo的实参是一个A类型的右值时，T的类型是A。根据引用叠加规则可以判断，最后的类型是A&&。
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// string.h implement

/***
*char *strcat(dst, src) - concatenate (append) one string to another
*
*Purpose:
*       Concatenates src onto the end of dest.  Assumes enough
*       space in dest.
*
*Entry:
*       char *dst - string to which "src" is to be appended
*       const char *src - string to be appended to the end of "dst"
*
*Exit:
*       The address of "dst"
*
*Exceptions:
*
*******************************************************************************/

char* __cdecl strcat(char* dst, const char* src)
{
	char *cp = dst;

	// 不能写成while(*cp++) 因为这样的话为*cp为0时还会+1，导致cp执向了\0的后一个。
	while (*cp)
		cp++;		/* find end of dst */

	while (*cp++ = *src++);	/* Copy src to end of dst */

	return(dst);
}

/***
*char *strcpy(dst, src) - copy one string over another
*
*Purpose:
*       Copies the string src into the spot specified by
*       dest; assumes enough room.
*
*Entry:
*       char * dst - string over which "src" is to be copied
*       const char * src - string to be copied over "dst"
*
*Exit:
*       The address of "dst"
*
*Return:
	实现链式表达式
*
*Exceptions:
*******************************************************************************/
char* __cdecl strcpy(char* dst, const char* src)
{

	// assert( (dst!=NULL) && (src!=NULL) );

	char *cp = dst;

	while (*cp++ = *src++)
		;					/* Copy src over dst */

	return( dst );
}

/***
*strcmp - compare two strings, returning less than, equal to, or greater than
*
*Purpose:
*       STRCMP compares two strings and returns an integer
*       to indicate whether the first is less than the second, the two are
*       equal, or whether the first is greater than the second.
*
*       Comparison is done byte by byte on an UNSIGNED basis, which is to
*       say that Null (0) is less than any other character (1-255).
*
*Entry:
*       const char * src - string for left-hand side of comparison
*       const char * dst - string for right-hand side of comparison
*
*Exit:
*       returns -1 if src <  dst
*       returns  0 if src == dst
*       returns +1 if src >  dst
*
*Exceptions:
*
*******************************************************************************/
int _cdecl strcmp(const char *src, const char *dst)
{
	// assert( (dst!=NULL) && (src!=NULL) );

	int ret = 0;

	/*
	 * 使用*（unsigned char *）str1而不是用*str1。这是因为传入的参数为有符号数，
		例如：str的值为1，str2的值为255。
		作为无符号数计算时ret=-254，结果为负值，正确。
		作为有符号数计算时ret=-2，结果为正值，错误。
		ascii char范围为0-127
		有符号字符值的范围是-128-127，无符号字符值的范围是0-255，而字符串的ASCII没有负值，若不转化为无符号数，在减法实现时出现错误。
		怎么才能打印255的字符, 在cmd下，按alt+255 回车，就可以打印
	 *
	 * 如果是de和def比较，*src没有为0，没有比较, 因为:
		! ret
		ret=0才继续比较。
		ret=0-'f'=负数。
		!负数   为假。 0为假，非0为真。
		!0为真。

		最后的dst也可以换成src，因为前面已经做了相减，无论哪个先为'\0'都会退出。
	 */

	while (!(ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
		++src, ++dst;

	if (ret < 0)
		ret = -1;
	else if (ret > 0)
		ret = 1;

	return(ret);
}

size_t __cdecl strlen(const char* str)
{
	const char *eos = str;

	while (*eos++);

	return(eos - str - 1);
}

/***
*char *strncpy(dest, source, count) - copy at most n characters
*
*Purpose:
*       Copies count characters from the source string to the
*       destination.  If count is less than the length of source,
*       NO NULL CHARACTER is put onto the end of the copied string.
*       If count is greater than the length of sources, dest is padded
*       with null characters to length count.
*
*
*Entry:
*       char *dest - pointer to destination
*       char *source - source string for copy
*       unsigned count - max number of characters to copy
*
*Exit:
*       returns dest
*
*Exceptions:
*
* 复制src中的内容（字符，数字、汉字....）到dest，复制多少由num的值决定，返回指向dest的指针。如果遇到null字符（'\0'），
* 且还没有到num个字符时，就用（num - n）（n是遇到null字符前已经有的非null字符个数）个null字符附加到destination。
* 注意：并不是添加到destination的最后，而是紧跟着由source中复制而来的字符后面。
* 例：
char des[] = "Hello,i am!";
char source[] = "abc\0def";
strncpy(des,source,5);
此时，des区域是这样的：a,b,c,\0,\0,逗号,i,空格,a,m,!
在测试时不要用prinf("%s"),因为遇到\0就结束了，
注意：\0,\0并不是添加在!的后面。
说明：
如果n > dest串长度，dest栈空间溢出产生崩溃异常。
否则：
1）src串长度<=dest串长度,(这里的串长度包含串尾NULL字符)
如果n<src串长度，src的前n个字符复制到dest中。但是由于没有NULL字符，所以直接访问dest串会发生栈溢出的异常情况。
如果n = src串长度，与strcpy一致。
如果n >src串长度，src串存放于desk字串的[0,src串长度]，dest串的(src串长度, dest串长度]处存放NULL。
2）src串长度>dest串长度
如果n =dest串长度，则dest串没有NULL字符，会导致输出会有乱码。如果不考虑src串复制完整性，可以将dest最后一字符置为NULL。
综上，一般情况下，使用strncpy时，建议将n置为dest串长度（除非你将多个src串都复制到dest数组，并且从dest尾部反向操作)，
复制完毕后，为保险起见，将dest串最后一字符置NULL，避免发生在第2)种情况下的输出乱码问题。
无论是strcpy还是strncpy，保证src串长度<dest串长度。
*******************************************************************************/
char * __cdecl strncpy(char *dest, const char *source, size_t count)
{
	char *start = dest;

	while (count && (*dest++ = *source++))	/* copy string */
		count--;

	if (count)	/* pad out with zeroes */
		while (--count)
			*dest++ = '\0';

	return(start);
}

/***
*char *strstr(string1, string2) - search for string2 in string1
*
*Purpose:
*       finds the first occurrence of string2 in string1
*
*Entry:
*       char *string1 - string to search in
*       char *string2 - string to search for
*
*Exit:
*       returns a pointer to the first occurrence of string2 in
*       string1, or NULL if string2 does not occur in string1
*
*Uses:
*
* O(m*n) - m=strlen(str1), n=strlen(str2)
*Exceptions:
*
*******************************************************************************/
char* __cdecl strstr(const char *str1, const char *str2)
{
	char *cp = (char *)str1;
	char *s1, *s2;

	if (!*s2)
		return((char *)str1);

	while (*cp)
	{
		s1 = cp;
		s2 = (char *)str2;

		while (*s1 && *s2 && !(*s1 - *s2))
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);
}

/***
*char *strchr(string, c) - search a string for a character
*
*Purpose:
*       Searches a string for a given character, which may be the
*       null character '\0'.
*
*Entry:
*       char *string - string to search in
*       char c - character to search for
*
*Exit:
*       returns pointer to the first occurence of c in string
*       returns NULL if c does not occur in string
*
*Exceptions:
*
*******************************************************************************/
char* __cdecl strchr(const char *string, int ch)
{
	while (*string && *string != (char)ch)
		string++;

	if (*string == (char)ch)
		return((char*)string);

	return(NULL);
}

/***
*char *strrchr(string, ch) - find last occurrence of ch in string
*
*Purpose:
*       Finds the last occurrence of ch in string.  The terminating
*       null character is used as part of the search.
*
*Entry:
*       char *string - string to search in
*       char ch - character to search for
*
*Exit:
*       returns a pointer to the last occurrence of ch in the given
*       string
*       returns NULL if ch does not occurr in the string
*
*Exceptions:
*
*******************************************************************************/
char* __cdecl strrchr(const char *string, int ch)
{
	char *start = (char*)string;

	while (*string++)	/* find end of string */
		;

	while (--string != start && *string != (char)ch)	/* search towards front */
		;

	if (*string == (char)ch)	/* char found ? */
		return((char*)string);

	return(NULL);
}

// 功能：由src所指内存区域复制count个字节到dest所指内存区域。
// 说明：src和dest所指内存区域可以重叠，但复制后dest内容会被更改。函数返回指向dest的指针
void* __cdecl memmove(void *dest, const void *src, size_t count)
{
	assert( (dest != NULL) && (src != NULL) );
	assert( count > 0 );

	char *psrc = (char*)src;	//可以保持src里面的值不变，否则后面的自增会改变里面存放的值
	char *pdest = (char*)dest;

	//检查是否有重叠问题
	if(pdest < psrc)
	{
		// 正向拷贝
		while(count--)
			*pdest++ = *psrc++;
	}
	else if(psrc < pdest)
	{
		// 反向拷贝
		psrc = psrc + count - 1;
		pdest = pdest + count - 1;
		while(count--)
			*pdest-- = *psrc--;
	}

	return dest;
}

//////////////////////////////////////////////////////// Linux 打印调用堆栈 //////////////////////////////////////////////////////////////////////////////////////////////////
#include <execinfo.h>

void print_stacktrace()
{
	int size = 16;
	void* array[16];

	/* int backtrace(void **buffer, int size); */
	// backtrace用来获得当前程序的调用栈，把结果存在buffer中。
	int stack_num = backtrace(array, size);

	/* void backtrace_symbols_fd(void *const *buffer, int size, int fd); */
	// backtrace_symbols_fd把用backtrace获取的调用栈信息写到fd所指定的文件中
	/* char **backtrace_symbols(void *const *buffer, int size); */
	// backtrace_symbols把用backtrace获取的调用栈转换成字符串数组，以字符串数组的形式返回，使用者需要在外面释放返回的字符串数组所占用的内存
	char** stacktrace = backtrace_symbols(array, stack_num);

	for(int i = 0; i < stack_num; ++i)
	{
		printf("[%s]\n", stacktrace[i]);
	}

	free(stacktrace);
}

一、
$ gcc test.cc -o test1
$ ./test1

/* 函数的调用栈, 是16进制的地址, 可以通过反汇编得到每个地址对应的函数. */

/* void * __builtin_return_address (unsigned int level) */
// 来得到当前函数，或者调用它的函数的返回地址，得到这个地址后，通过gdb反汇编，便可得到调用函数相关的信息

二、
$ gcc test.cc -rdynamic -o test2
$ ./test2

-rdynamic: 让链接器把所有的符号都加入到动态符号表, 函数名都是mangle过的，需要demangle才能看到原始的函数.
demangle: 如下命令
	$ c++filt < << "_Z16print_stacktracev"

三、调用位置:
$ addr2line -a 0x4008a7 -e test2 -f
	-f选项可以打出函数名, -C选项也可以demangle -a函数地址
	此时需要　$ gcc test.cc -rdynamic -g -o test3
	-g选项的主要作用是生成调试信息，位置信息就属于调试信息的范畴

#include "malloc .h"

size_t malloc_usable_size((void *__ptr));
// 返回调用malloc后实际分配的可用内存的大小。

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
位域:
	把一个字节中的二进位划分为几个不同的区域，并说明每个区域的位数。每个域有一个域名，允许在程序中按域名进行操作。这样就可以把几个不同的对象用一个字节的二进制位域来表示。
	位域的定义和位域变量的说明位域定义与结构定义相仿，其形式为：
struct 位域结构名
{ 位域列表 };

1) 如果相邻位域字段的类型相同，且其位宽之和小于类型的sizeof大小，则后面的字
	段将紧邻前一个字段存储，直到不能容纳为止；
2) 如果相邻位域字段的类型相同，但其位宽之和大于类型的sizeof大小，则后面的字
	段将从新的存储单元开始，其偏移量为其类型大小的整数倍；
3) 整个结构体的总大小为最宽基本类型成员大小的整数倍。
4) 如果相邻的位域字段的类型不同，则各编译器的具体实现有差异，VC6采取不压缩方
	式，Dev-C++采取压缩方式；
5) 如果位域字段之间穿插着非位域字段，则不进行压缩；（不针对所有的编译器）
4 ，5跟编译器有较大的关系，使用时要慎重，尽量避免。

struct test {
    unsigned a:10;
    unsigned b:10;
    unsigned c:6;
    unsigned :2;//this two bytes can't use
    unsigned d:4;
    }data,*pData;
data.a=0x177;
data.b=0x111;
data.c=0x7;
data.d=0x8;
pData=&data;
printf("data.a=%x data.b= %x data.c=%x data.d=%xn",pData->a,pData->b,pData->c,pData->d);//位域可以使用指针
printf("sizeof(data)=%dn",sizeof(data));   //4 bytes ，最常用的情况

struct testLen{
    char a:5;
    char b:5;
    char c:5;
    char d:5;
    char e:5;
    }len;
printf("sizeof(len)=%dn",sizeof(len));     //5bytes 规则2

struct testLen1{
    char a:5;
    char b:2;
    char d:3;
    char c:2;
    char e:7;
    }len1;
printf("sizeof(len1) =%dn",sizeof(len1));    //3bytes 规则1

struct testLen2{
    char a:2;
    char :3;
    char b:7;
    long d:20; //4bytes
    char e:4;
    }len2;
printf("sizeof(len2)=%dn",sizeof(len2));  //12 规则3，4,5，总长为4的整数倍，2+3 占1byte，b占1bye 由于与long对其，2+3+7 占4字节，后面 d 与 e进行了优化 占一个4字节

struct testLen3{
    char a:2;
    char :3;
    char b:7;
    long d:30;
    char e:4;
    }len3;
printf("sizeof(len3)=%dn",sizeof(len3));//12 规则3，4，5，总长为4的整数倍，2+3 占1byte，b占1bye 由于与long对其，2+3+7 占4字节，后面 d占一个4字节，为了保证与long对其e独占一个4字节
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 模板特化
#define MAXNAME 128

struct job
{
	char name[MAXNAME];
	int salary;
};

// 函数模板
template<class T> void swap(T &a, T &b)
{
	T temp;
	temp = a;
	a = b;
	b = temp;
}

// 显示实例化，只需声明
template void swap<int>(int &a, int &b);

// explicite specialization. 模板特化，必须定义
template<> void swap<job>(job &a, job &b)
{
	int salary;
	salary = a.salary;
	a.salary = b.salary;
	b.salary = salary;
}

// 类模板
template <class T> class Array
{
private:
	T* ar;
	int l;
};

template class Array<int>;		// explicit instantiation. 显示实例化(也可以隐式实例化，由编译器推断)

// explicit specialization.   显式具体化，类定义体可以不同于类模板Array
template<> class Array<job>
{
private:
	job* ar;
	int l;
};
///////////////////////////////////////////////////////////////C++11 异步///////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <vector>
#include <numeric>
#include <future>

template <typename TIter> int AsynchronousSum(TIter front, TIter begin, TIter end)
{
	auto len = end - begin;
	if (len < 1000)
	{
		char out[64];
		sprintf(out, "Calculate from [%d] to [%d].\n", begin - front, end - front);
		std::cout << out << std::endl;
		return std::accumulate(begin, end, 0);
	}

	TIter mid = begin + len / 2;

	auto handleRight = std::async(std::launch::async, AsynchronousSum<TIter>, front, mid, end);
	auto handleLeft = std::async(std::launch::async, AsynchronousSum<TIter>, front, begin, mid);

	return handleLeft.get() + handleRight.get();
}

std::vector<int> v(20000, 1);
std::cout << "The sum is " << AsynchronousSum< std::vector<int>::iterator >(v.begin(), v.begin(), v.end()) << std::endl;
//////////////////////////////////////////////////////////////////C++多态继承////////////////////////////////////////////////////////////////////////////////////////
Walk()是非虚函数，被静态绑定所限制，所以pp、ps是什么类型就决定了调用的版本。
通过对象调用是静态绑定，一定要指定参数值，因为静态绑定这个函数不从base class继承缺省参数值。动态绑定却可以从base class继承参数值。
virtual函数是动态绑定，缺省参数值是静态绑定, 

class Shape
{
public:
    enum ShapeColor{Red, Green, Blue};                //形状颜色
 
    virtual void Draw(ShapeColor color = Red) const = 0;
};
 
class Circle : public Shape
{
public:
    virtual void Draw(ShapeColor color) const
    {
        cout << "I am Circle::Draw. ";
        cout << "My color = " << color << endl;
    }
};
 
class Rectangle : public Shape
{
public:
    virtual void Draw(ShapeColor color = Green) const            //缺省的参数值被更改了
    {
        cout << "I am Rectangle::Draw. ";
        cout << "My color = " << color << endl;
    }
};

Circle cr;
Circle *ps = &cr;    //这还是静态绑定，静态类型Circle *，编译不通过
ps->Draw();

Shape *ps = &cr;    //动态绑定
ps->Draw();
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////