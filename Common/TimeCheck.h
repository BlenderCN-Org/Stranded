#ifndef __TIME_CHECK_H__
#define __TIME_CHECK_H__

#include "define.h"

/*
 * RDTSC - 粒度: 纳秒级
 * 在 Intel Pentium (i386) 以上级别的CPU中，有一个称为“时间戳（Time Stamp）”的部件，
 * 它以64位无符号整型数(所能表达最大数字是1.8×10^19)的格式，记录了自CPU上电以来所经过的时钟周期数。
 * 由于目前的CPU主频都非常高，因此这个部件可以达到纳秒级的计时精度。
 * 在Pentium以上的CPU中，提供了一条机器指令RDTSC（Read Time Stamp Counter）来读取这个时间戳的数字，
 * 并将其保存在EDX:EAX寄存器对中。
 * 由于EDX:EAX寄存器对恰好是Win32平台下C++语言保存函数返回值的寄存器，
 * 所以我们可以把这条指令看成是一个普通的函数调用。
 *
 * QueryPerformanceCounter - 粒度: 1~100微秒级
 * 
 * timeGetTime - 粒度: 毫秒级
 *
 * DOS平台下的高精度计时, 控制定时器8253.
 */

inline unsigned __int64 GetCycleCount()
{
	// RDTSC不被C++的内嵌汇编器直接支持，要用_emit伪指令直接嵌入该指令的机器码形式0X0F、0X31
	__asm _emit 0x0F
	__asm _emit 0x31
}

// 程序执行计时类
class CTimeCheck
{
public:
	CTimeCheck();
public:
	void RDTSCStart();
	unsigned __int64 RDTSCStop();
	//////////////////////////////////////////////////////////////////////////
	void QueryPerformanceStart();
	double QueryPerformanceEnd();
	//////////////////////////////////////////////////////////////////////////
	void TimeStart();
	DWORD TimeEnd();
private:
	// 通过QueryPerformanceFrequency获取当前计数器每秒的计数次数
	LARGE_INTEGER m_frequency;
	LARGE_INTEGER m_startCount;
	LARGE_INTEGER m_endCount;
	//////////////////////////////////////////////////////////////////////////
	// 执行RDTSC指令的时间忽略
	unsigned __int64 m_startCycle;
	
	unsigned __int64 m_overhead;
	//////////////////////////////////////////////////////////////////////////
	// 获取当前的时钟间隔值
	DWORD m_dwClockInterval;
	DWORD m_dwTimeStart;
};

#endif