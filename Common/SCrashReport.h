/*
 * 程序崩溃是保存信息并启动CrashReport
 * ref from http://www.codeproject.com/tools/minidump.asp
 * By Vladimir Sedach
 */

#ifndef __S_CRASH_REPORT_H__
#define __S_CRASH_REPORT_H__

#include "define.h"

#pragma optimize("y", off)		// generate stack frame pointers for all functions - same as /Oy - in the project


BOOL CreateSmallDumpInfo(PEXCEPTION_POINTERS pException, PTCHAR szSmallFile, UINT dwLastError);

BOOL CreateBigInfoFile(PEXCEPTION_POINTERS pException, PTCHAR szBigFile, UINT dwLastError);

BOOL CreateDumpHelpFile(PEXCEPTION_POINTERS pException, PTCHAR szDumpFile);

BOOL CatchSEHException(PEXCEPTION_POINTERS pException, bool bRunCrashReport);

#endif