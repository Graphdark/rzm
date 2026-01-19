#include "stdafx.h"

#include "rxdebug.h"

#ifdef _DEBUG 

#ifndef _WINBASE_
//extracted from winbase.h so that
//we don't have to pull in the whole
//header
extern "C"
void
_stdcall
OutputDebugStringA(
    const TCHAR* lpOutputString
    );

extern "C"
void
_stdcall
OutputDebugStringW(
    const unsigned short* lpOutputString
    );
#ifdef UNICODE
#define OutputDebugString  OutputDebugStringW
#else
#define OutputDebugString  OutputDebugStringA
#endif // !UNICODE

#endif //_WINBASE

// determine number of elements in an array (not bytes)
#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

/*inline*/ void _cdecl RxTrace(const TCHAR* lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);
	int nBuf;
	TCHAR szBuffer[512];
	nBuf = _vsntprintf(szBuffer, _countof(szBuffer), lpszFormat, args);
	// was there an error?
	RXASSERT(nBuf >= 0);
    ::OutputDebugString(szBuffer);
	va_end(args);
}

#endif //_DEBUG, entire file
