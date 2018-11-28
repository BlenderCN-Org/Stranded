#include "TimeCheck.h"
#include "pubutil.h"

#ifdef _MSC_VER
#pragma comment(lib, "Winmm.lib")
#endif

CTimeCheck::CTimeCheck()
{
	//////////////////////////////////////////////////////////////////////////
	// 返回硬件支持的高精度计数器的频率
	::QueryPerformanceFrequency(&m_frequency);
	//////////////////////////////////////////////////////////////////////////
	DWORD dwAdjustment = 0;
	m_dwClockInterval = 0;
	BOOL bAdjustmentDisabled = FALSE;
	// "The systemclock interval is %d ms\n", clockInterval / 10000
	::GetSystemTimeAdjustment(&dwAdjustment, &m_dwClockInterval, &bAdjustmentDisabled);
	m_dwTimeStart = 0;

	//10^6 MHz https://docs.microsoft.com/zh-cn/windows/desktop/SysInfo/getting-hardware-information
	GetRegValue(2, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "~MHz");
}

void CTimeCheck::RDTSCStart()
{
	m_startCycle = GetCycleCount();
}

// 纳秒级的计时精度（在1GHz的CPU上每个时钟周期就是一纳秒）
// 一个计数相当于1/(CPU主频Hz数)秒
unsigned __int64 CTimeCheck::RDTSCStop()
{
	// 自CPU上电以来的秒数 = RDTSC读出的周期数 / CPU主频速率（Hz）
	// 精确到纳秒数量级 消耗时间 = 返回值/CPU频率
	return GetCycleCount() - m_startCycle;
}

void CTimeCheck::QueryPerformanceStart()
{
	::QueryPerformanceCounter(&m_startCount);
}

double CTimeCheck::QueryPerformanceEnd()
{
	::QueryPerformanceCounter(&m_endCount);
	// 单位是秒
	return (double)(m_endCount.QuadPart - m_startCount.QuadPart) / (double)m_frequency.QuadPart;
}

void CTimeCheck::TimeStart()
{
	// ms
	if(m_dwClockInterval / 10000 > 1)
		::timeBeginPeriod(1);	// 设置当前系统时钟间隔为1ms

	m_dwTimeStart = ::timeGetTime();
}

DWORD CTimeCheck::TimeEnd()
{
	// 1毫秒
	if(m_dwClockInterval / 10000 > 1)
		::timeEndPeriod(1);	// 恢复到原频率

	// 毫秒
	return (::timeGetTime() - m_dwTimeStart);
}
