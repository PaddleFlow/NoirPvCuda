#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <strsafe.h>
#include "main.h"

DWORD WINAPI GuestStdOutThreadRoutine(IN LPVOID lpParameter)
{
	// Listen on stdout and output to console.
	while(1)
	{
		BYTE StdOutBuff[4096];
		ULONG ReadSize;
		ReadFile(GuestStdOutPipe,StdOutBuff,sizeof(StdOutBuff),&ReadSize,NULL);
		WriteConsoleA(StdOut,StdOutBuff,ReadSize,NULL,NULL);
	}
	ExitThread(1);
	return 1;
}

DWORD WINAPI GuestStdErrThreadRoutine(IN LPVOID lpParameter)
{
	// Listen on stderr and output to console.
	while(1)
	{
		BYTE StdErrBuff[4096];
		ULONG ReadSize;
		ReadFile(GuestStdErrPipe,StdErrBuff,sizeof(StdErrBuff),&ReadSize,NULL);
		WriteConsoleA(StdErr,StdErrBuff,ReadSize,NULL,NULL);
	}
	ExitThread(1);
	return 1;
}

int main(int argc,char* argv[],char* envp[])
{
	// Check if argument count matches.
	CHAR StdInPipe[64],StdOutPipe[64],StdErrPipe[64];
	if(argc<2)
	{
		puts("Missing argument for Pipe Name!");
		return 1;
	}
	else if(argc>2)
	{
		for(int i=2;i<argc;i++)
			printf("Excessive Argument: %s\n",argv[i]);
		return 1;
	}
	// Initialize Console...
	StdIn=GetStdHandle(STD_INPUT_HANDLE);
	StdOut=GetStdHandle(STD_OUTPUT_HANDLE);
	StdErr=GetStdHandle(STD_ERROR_HANDLE);
	// Generate Names...
	StringCbPrintfA(StdInPipe,sizeof(StdInPipe),"\\\\.\\pipe\\NoirPvCudaGuestStdIn_%s",argv[1]);
	StringCbPrintfA(StdOutPipe,sizeof(StdOutPipe),"\\\\.\\pipe\\NoirPvCudaGuestStdOut_%s",argv[1]);
	StringCbPrintfA(StdErrPipe,sizeof(StdErrPipe),"\\\\.\\pipe\\NoirPvCudaGuestStdErr_%s",argv[1]);
	GuestStdInPipe=CreateFileA(StdInPipe,GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,NULL);
	GuestStdOutPipe=CreateFileA(StdOutPipe,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,NULL);
	GuestStdErrPipe=CreateFileA(StdErrPipe,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,NULL);
	if(GuestStdInPipe!=INVALID_HANDLE_VALUE && GuestStdOutPipe!=INVALID_HANDLE_VALUE && GuestStdErrPipe!=INVALID_HANDLE_VALUE)
	{
		// Use multithreading to listen to stdin, stdout and stderr.
		DWORD StdOutThreadId,StdErrThreadId;
		HANDLE StdOutThread=CreateThread(NULL,0x100000,GuestStdOutThreadRoutine,NULL,0,&StdOutThreadId);
		HANDLE StdErrThread=CreateThread(NULL,0x100000,GuestStdErrThreadRoutine,NULL,0,&StdErrThreadId);
		if(StdOutThread && StdErrThreadId)
		{
			printf("StdOut Thread Id: %u\n",StdOutThreadId);
			printf("StdErr Thread Id: %u\n",StdErrThreadId);
			// Main thread will be listening on stdin.
			while(1)
			{
				BYTE StdInBuff[4096];
				SIZE_T Length;
				ULONG WrittenLength;
				StringCbGetsExA(StdInBuff,sizeof(StdInBuff),NULL,&Length,0);
				WriteFile(GuestStdInPipe,StdInBuff,(ULONG)(sizeof(StdInBuff)-Length),&WrittenLength,NULL);
			}
		}
		else
		{
			if(StdOutThread)TerminateThread(StdOutThread,0);
			if(StdErrThread)TerminateThread(StdErrThread,0);
			puts("Failed to create threads for stdout/stderr!");
			goto InitializationFailed;
		}
	}
	else
	{
InitializationFailed:
		if(GuestStdInPipe!=INVALID_HANDLE_VALUE)CloseHandle(GuestStdInPipe);
		if(GuestStdOutPipe!=INVALID_HANDLE_VALUE)CloseHandle(GuestStdOutPipe);
		if(GuestStdErrPipe!=INVALID_HANDLE_VALUE)CloseHandle(GuestStdErrPipe);
		GuestStdInPipe=GuestStdOutPipe=GuestStdErrPipe=INVALID_HANDLE_VALUE;
		puts("Initialization failed!");
		system("pause");
		return 1;
	}
	return 0;
}