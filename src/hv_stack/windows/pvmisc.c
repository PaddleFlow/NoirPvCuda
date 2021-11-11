#include <Windows.h>
#include <strsafe.h>
#include <stdarg.h>
#include "pvmisc.h"

void PvInitializeConsole()
{
	StdIn=GetStdHandle(STD_INPUT_HANDLE);
	StdOut=GetStdHandle(STD_OUTPUT_HANDLE);
}

ULONG PvPrintConsoleA(IN PCSTR Format,...)
{
	BOOL Result=FALSE;
	SIZE_T Length=0;
	ULONG ReturnLength=0;
	CHAR Buffer[512];
	va_list ArgList;
	va_start(ArgList,Format);
	StringCbVPrintfA(Buffer,sizeof(Buffer),Format,ArgList);
	va_end(ArgList);
	StringCchLengthA(Buffer,sizeof(Buffer)/sizeof(CHAR),&Length);
	Result=WriteConsoleA(StdOut,Buffer,(DWORD)Length,&ReturnLength,NULL);
	return Result;
}

ULONG PvPrintConsoleW(IN PCWSTR Format,...)
{
	BOOL Result=FALSE;
	SIZE_T Length=0;
	ULONG ReturnLength=0;
	WCHAR Buffer[512];
	va_list ArgList;
	va_start(ArgList,Format);
	StringCbVPrintfW(Buffer,sizeof(Buffer),Format,ArgList);
	va_end(ArgList);
	StringCchLengthW(Buffer,sizeof(Buffer)/sizeof(WCHAR),&Length);
	Result=WriteConsoleW(StdOut,Buffer,(DWORD)Length,&ReturnLength,NULL);
	return Result;
}

PVOID MemAlloc(IN SIZE_T Length)
{
	return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,Length);
}

BOOL MemFree(IN PVOID Memory)
{
	return HeapFree(GetProcessHeap(),0,Memory);
}