/*
 * �쳣���
 */

#ifndef __S_EXCEPTION_H__
#define __S_EXCEPTION_H__

#include "define.h"
#include "SCrashReport.h"

#if defined _MSC_VER
/*
 * ref from http://www.codeproject.com/tools/minidump.asp
 * generate stack frame pointers for all functions - same as /Oy - in the project
 */
#pragma optimize("y", off)
#endif

/////////////////////////////�쳣Ӧ��////////////////////////////

// �׳��ַ�����ʽ�쳣
VOID ThrowStringException(LPCTSTR szFmt, ...);

// �׳�������ʽ�쳣
VOID ThrowAssertException(LPCTSTR szFileName, INT nFileLine, LPCTSTR pExpr);

void SendInnerCrashReport(PEXCEPTION_POINTERS pException, HWND hInnerWnd);

#if 1
	// �׳�һ���ַ��������쳣
	#define SThrow 	ThrowStringException

	// �����ж�
	#define SAssert( bCondition ) \
		do { \
			if( (bCondition) == 0 ) \
					{ \
				ThrowAssertException(_T(__FILE__), __LINE__, _T(#bCondition)); \
			} \
		} while (0);
#else
	// �׳�һ���ַ��������쳣
	#define SThrow ThrowStringException

	// �����ж� Release �治���κ�����
	#define SAssert(bCondition) (VOID)(0)
#endif

#endif