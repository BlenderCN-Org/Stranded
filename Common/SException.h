/*
 * 异常相关
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

/////////////////////////////异常应用////////////////////////////

// 抛出字符串形式异常
VOID ThrowStringException(LPCTSTR szFmt, ...);

// 抛出断言形式异常
VOID ThrowAssertException(LPCTSTR szFileName, INT nFileLine, LPCTSTR pExpr);

void SendInnerCrashReport(PEXCEPTION_POINTERS pException, HWND hInnerWnd);

#if 1
	// 抛出一个字符串类型异常
	#define SThrow 	ThrowStringException

	// 断言判断
	#define SAssert( bCondition ) \
		do { \
			if( (bCondition) == 0 ) \
					{ \
				ThrowAssertException(_T(__FILE__), __LINE__, _T(#bCondition)); \
			} \
		} while (0);
#else
	// 抛出一个字符串类型异常
	#define SThrow ThrowStringException

	// 断言判断 Release 版不做任何事情
	#define SAssert(bCondition) (VOID)(0)
#endif

#endif