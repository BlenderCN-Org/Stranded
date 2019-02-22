#include <iostream>
#include <vector>
#include <list>

//#include "../Common/TimeCheck.h"
//
//#pragma comment(lib, "../lib/Common.lib")
//
//static const int si_len = 9000;

// Comparing a signed and unsigned number required the compiler to convert the signed value to unsigned.

int main(int argc, char* argv[])
{
	/*if (argc > 0)
	{

	}

	char cwd[1024];
	_getcwd(cwd, 1024);
	PrintInfo(true, "Program in [%s]", cwd);

#pragma warning(push)
#pragma warning(disable:4996)
	const char* env = std::getenv("PATH");
	if (env && env[0])
	{
		PrintInfo(true, "PATH = [%s]\n", env);
	}
#pragma warning(pop)


	std::vector<int> vecInt;
	std::list<int> listInt;

	for (int i = 0; i < si_len; i++)
	{
		vecInt.push_back(i);
		listInt.push_back(i);
	}
	
	CTimeCheck chk;

	chk.TimeStart();
	chk.QueryPerformanceStart();

	for (std::vector<int>::iterator itr = vecInt.begin(); itr != vecInt.end(); itr++)
	{
		if ((*itr) == 8000)
			break;
	}

	DWORD millisecond = chk.TimeEnd();
	double dLastTime1 = chk.QueryPerformanceEnd();

	std::cout << "vector int: " << dLastTime1 << std::endl;
	std::cout << "vector int1: " << millisecond << std::endl;

	chk.QueryPerformanceStart();

	for (std::list<int>::iterator itr = listInt.begin(); itr != listInt.end(); itr++)
	{
		if ((*itr) == 8000)
			break;
	}

	double dLastTime2 = chk.QueryPerformanceEnd();

	std::cout << "list int: " << dLastTime2 << std::endl;*/

	/*float a = 0.17f;
	int b = (int)a;

	float c = -1.7f;
	int d = (int)c;

	std::cout << "b=" << b << "d=" << d << std::endl;*/
	

	return 0;
}

