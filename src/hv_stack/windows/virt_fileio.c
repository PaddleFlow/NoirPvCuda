#include <Windows.h>
#include <strsafe.h>
#include <NoirCvmApi.h>
#include "pvproc.h"
#include "virt_fileio.h"

HANDLE PvIoCreateFile(IN PSTR FilePath,IN ULONG DesiredAccess,IN ULONG ShareMode,IN ULONG Disposition,IN ULONG Attributes)
{
	// Translate file path then create file.
	CHAR TargetFilePath[MAX_PATH];
	PSTR TargetFilePathEnd;
	HANDLE FileHandle;
	StringCbCopyExA(&TargetFilePath[1],sizeof(TargetFilePath)-1,FilePath,&TargetFilePathEnd,NULL,0);
	TargetFilePath[0]='.';
	// FIXME: More accurate translation. Prevent directory retracing.
	for(PSTR s=&TargetFilePath[1];s<TargetFilePathEnd;s++)
		if(*s=='/')*s='\\';
	PvPrintConsoleA("File Path: %s\n",TargetFilePath);
	FileHandle=CreateFileA(FilePath,DesiredAccess,ShareMode,NULL,Disposition,Attributes,NULL);
	return FileHandle;
}

BOOL PvIoReadFileSynchronous(IN ULONG64 GuestCr3,IN HANDLE FileHandle,IN ULONG64 BufferGva,IN ULONG NumberOfBytes)
{
	ULONG RemainderLength=NumberOfBytes;
	ULONG64 GuestBuffer=BufferGva;
	do
	{
		ULONG64 BufferHva;
		ULONG32 ErrCode=PvTranslateGvaToHva64(GuestCr3,GuestBuffer,1,&BufferHva);
		if(ErrCode)
			return FALSE;
		else
		{
			ULONG SizeToRead=(ULONG)((PAGE_BASE(GuestBuffer)+PAGE_SIZE-GuestBuffer)>RemainderLength?RemainderLength:(PAGE_BASE(GuestBuffer)+PAGE_SIZE-GuestBuffer));
			ULONG SizeRead;
			BOOL Result=ReadFile(FileHandle,(PVOID)BufferHva,SizeToRead,&SizeRead,NULL);
			if(!Result)return FALSE;
			RemainderLength-=SizeRead;
			GuestBuffer+=SizeRead;
		}
	}while(RemainderLength);
	return TRUE;
}

void PvFinalizeFileIoHardware()
{
	if(PvFileEvent)
	{
		CloseHandle(PvFileEvent);
		PvFileEvent=NULL;
	}
}

BOOL PvInitializeFileIoHardware()
{
	// The event must be manually reset by interrupt controller.
	PvFileEvent=CreateEventA(NULL,TRUE,FALSE,NULL);
	return PvFileEvent!=NULL;
}