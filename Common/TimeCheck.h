#ifndef __TIME_CHECK_H__
#define __TIME_CHECK_H__

#include "define.h"

/*
 * RDTSC - ����: ���뼶
 * �� Intel Pentium (i386) ���ϼ����CPU�У���һ����Ϊ��ʱ�����Time Stamp�����Ĳ�����
 * ����64λ�޷���������(���ܱ�����������1.8��10^19)�ĸ�ʽ����¼����CPU�ϵ�������������ʱ����������
 * ����Ŀǰ��CPU��Ƶ���ǳ��ߣ��������������Դﵽ���뼶�ļ�ʱ���ȡ�
 * ��Pentium���ϵ�CPU�У��ṩ��һ������ָ��RDTSC��Read Time Stamp Counter������ȡ���ʱ��������֣�
 * �����䱣����EDX:EAX�Ĵ������С�
 * ����EDX:EAX�Ĵ�����ǡ����Win32ƽ̨��C++���Ա��溯������ֵ�ļĴ�����
 * �������ǿ��԰�����ָ�����һ����ͨ�ĺ������á�
 *
 * QueryPerformanceCounter - ����: 1~100΢�뼶
 * 
 * timeGetTime - ����: ���뼶
 *
 * DOSƽ̨�µĸ߾��ȼ�ʱ, ���ƶ�ʱ��8253.
 */

inline unsigned __int64 GetCycleCount()
{
	// RDTSC����C++����Ƕ�����ֱ��֧�֣�Ҫ��_emitαָ��ֱ��Ƕ���ָ��Ļ�������ʽ0X0F��0X31
	__asm _emit 0x0F
	__asm _emit 0x31
}

// ����ִ�м�ʱ��
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
	// ͨ��QueryPerformanceFrequency��ȡ��ǰ������ÿ��ļ�������
	LARGE_INTEGER m_frequency;
	LARGE_INTEGER m_startCount;
	LARGE_INTEGER m_endCount;
	//////////////////////////////////////////////////////////////////////////
	// ִ��RDTSCָ���ʱ�����
	unsigned __int64 m_startCycle;
	
	unsigned __int64 m_overhead;
	//////////////////////////////////////////////////////////////////////////
	// ��ȡ��ǰ��ʱ�Ӽ��ֵ
	DWORD m_dwClockInterval;
	DWORD m_dwTimeStart;
};

#endif