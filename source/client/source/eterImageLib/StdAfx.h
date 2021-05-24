
#if !defined(AFX_STDAFX_H__BCF68E23_E7D8_4BF3_A905_AFDBEF92B0F6__INCLUDED_)
#define AFX_STDAFX_H__BCF68E23_E7D8_4BF3_A905_AFDBEF92B0F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#pragma warning(disable:4786)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>

#pragma warning(push, 3)
#include <string>
#include <vector>
#pragma warning(pop)

inline void _TraceForImage(const char* c_szFormat, ...)
{
	va_list args;
	va_start(args, c_szFormat);

	static char szBuf[1024];
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
#ifdef __TEST__
	OutputDebugString(szBuf);
#endif
	va_end(args);
	printf(szBuf);
}

#pragma warning(default:4018)
#endif 
