#include "TimeCheck.h"
#include "pubutil.h"

#ifdef _MSC_VER
#pragma comment(lib, "Winmm.lib")
#endif

CTimeCheck::CTimeCheck()
{
	//////////////////////////////////////////////////////////////////////////
	// ����Ӳ��֧�ֵĸ߾��ȼ�������Ƶ��
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

// ���뼶�ļ�ʱ���ȣ���1GHz��CPU��ÿ��ʱ�����ھ���һ���룩
// һ�������൱��1/(CPU��ƵHz��)��
unsigned __int64 CTimeCheck::RDTSCStop()
{
	// ��CPU�ϵ����������� = RDTSC������������ / CPU��Ƶ���ʣ�Hz��
	// ��ȷ������������ ����ʱ�� = ����ֵ/CPUƵ��
	return GetCycleCount() - m_startCycle;
}

void CTimeCheck::QueryPerformanceStart()
{
	::QueryPerformanceCounter(&m_startCount);
}

double CTimeCheck::QueryPerformanceEnd()
{
	::QueryPerformanceCounter(&m_endCount);
	// ��λ����
	return (double)(m_endCount.QuadPart - m_startCount.QuadPart) / (double)m_frequency.QuadPart;
}

void CTimeCheck::TimeStart()
{
	// ms
	if(m_dwClockInterval / 10000 > 1)
		::timeBeginPeriod(1);	// ���õ�ǰϵͳʱ�Ӽ��Ϊ1ms

	m_dwTimeStart = ::timeGetTime();
}

DWORD CTimeCheck::TimeEnd()
{
	// 1����
	if(m_dwClockInterval / 10000 > 1)
		::timeEndPeriod(1);	// �ָ���ԭƵ��

	// ����
	return (::timeGetTime() - m_dwTimeStart);
}
