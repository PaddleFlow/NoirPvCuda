#include <Windows.h>
#include <NoirCvmApi.h>
#include "virt_fileio.h"

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