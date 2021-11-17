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

ULONG PvGenerateSeed()
{
	SYSTEMTIME Time;
	USHORT Lo,Hi;
	ULONG Seed;
	GetLocalTime(&Time);
	Lo=Time.wYear+Time.wDayOfWeek+Time.wHour+Time.wSecond;
	Hi=Time.wMonth+Time.wDay+Time.wMinute+Time.wMilliseconds;
	Seed=(Hi<<16)|Lo;
	return Seed;
}

BOOL PvReadGuestStdIn(OUT PVOID Buffer,IN ULONG Size)
{
	DWORD ReadSize;
	return ReadFile(GuestStdInPipe,Buffer,Size,&ReadSize,NULL);
}

BOOL PvWriteGuestStdOut(IN PVOID Buffer,IN ULONG Size)
{
	DWORD WrittenSize;
	return WriteFile(GuestStdOutPipe,Buffer,Size,&WrittenSize,NULL);
}

BOOL PvWriteGuestStdErr(IN PVOID Buffer,IN ULONG Size)
{
	DWORD WrittenSize;
	return WriteFile(GuestStdErrPipe,Buffer,Size,&WrittenSize,NULL);
}

void PvTerminateGuestConsole()
{
	if(GuestStdInPipe!=INVALID_HANDLE_VALUE)CloseHandle(GuestStdInPipe);
	if(GuestStdOutPipe!=INVALID_HANDLE_VALUE)CloseHandle(GuestStdOutPipe);
	if(GuestStdErrPipe!=INVALID_HANDLE_VALUE)CloseHandle(GuestStdErrPipe);
	if(ConsoleProcessCreated)
	{
		TerminateProcess(ConsoleProcess.hProcess,0);
		CloseHandle(ConsoleProcess.hProcess);
		CloseHandle(ConsoleProcess.hThread);
	}
	GuestStdInPipe=GuestStdOutPipe=GuestStdErrPipe=INVALID_HANDLE_VALUE;
	ConsoleProcess.hProcess=ConsoleProcess.hThread=NULL;
	ConsoleProcess.dwProcessId=ConsoleProcess.dwThreadId=0;
}

BOOL PvCreateGuestConsole()
{
	// Create three named pipes for stdin, stdout and stderr for Guest.
	// Generate random numbers so different instances do not conflict.
	ULONG Seed=PvGenerateSeed();
	ULONG PipeNumber=RtlRandomEx(&Seed);
	CHAR StdInPipe[64],StdOutPipe[64],StdErrPipe[64];
	StringCbPrintfA(StdInPipe,sizeof(StdInPipe),"\\\\.\\pipe\\NoirPvCudaGuestStdIn_%u",PipeNumber);
	StringCbPrintfA(StdOutPipe,sizeof(StdOutPipe),"\\\\.\\pipe\\NoirPvCudaGuestStdOut_%u",PipeNumber);
	StringCbPrintfA(StdErrPipe,sizeof(StdErrPipe),"\\\\.\\pipe\\NoirPvCudaGuestStdErr_%u",PipeNumber);
	// Blocking accesses should be waiting forever.
	// Only one instance is allowed.
	GuestStdInPipe=CreateNamedPipeA(StdInPipe,PIPE_ACCESS_INBOUND|FILE_FLAG_FIRST_PIPE_INSTANCE,PIPE_TYPE_BYTE,1,0,4096,0xFFFFFFFF,NULL);
	GuestStdOutPipe=CreateNamedPipeA(StdOutPipe,PIPE_ACCESS_OUTBOUND|FILE_FLAG_FIRST_PIPE_INSTANCE,PIPE_TYPE_BYTE,1,0,4096,0xFFFFFFFF,NULL);
	GuestStdErrPipe=CreateNamedPipeA(StdErrPipe,PIPE_ACCESS_OUTBOUND|FILE_FLAG_FIRST_PIPE_INSTANCE,PIPE_TYPE_BYTE,1,0,4096,0xFFFFFFFF,NULL);
	if(GuestStdInPipe!=INVALID_HANDLE_VALUE && GuestStdOutPipe!=INVALID_HANDLE_VALUE && GuestStdErrPipe!=INVALID_HANDLE_VALUE)
	{
		STARTUPINFOA StartupInfo={0};
		CHAR CmdLine[32];
		StringCbPrintfA(CmdLine,sizeof(CmdLine),".\\pv_console.exe %u",PipeNumber);
		StartupInfo.cb=sizeof(StartupInfo);
		StartupInfo.lpTitle="NoirPvCuda Paravirtualized Guest Console";
		ConsoleProcessCreated=CreateProcessA(NULL,CmdLine,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&StartupInfo,&ConsoleProcess);
		return ConsoleProcessCreated;
	}
	else
	{
		if(GuestStdInPipe!=INVALID_HANDLE_VALUE)CloseHandle(GuestStdInPipe);
		if(GuestStdOutPipe!=INVALID_HANDLE_VALUE)CloseHandle(GuestStdOutPipe);
		if(GuestStdErrPipe!=INVALID_HANDLE_VALUE)CloseHandle(GuestStdErrPipe);
		GuestStdInPipe=GuestStdOutPipe=GuestStdErrPipe=INVALID_HANDLE_VALUE;
		return FALSE;
	}
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