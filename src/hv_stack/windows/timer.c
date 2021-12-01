#include <Windows.h>
#include "timer.h"

void static PvTimerHardwareCallback(IN PVOID lpParameter,IN BOOLEAN TimerOrWaitFired)
{
	// Repeat the signal. Please note that it's correct that timer interrupt signal could be lost.
	if(!SetEvent(PvTimerEvent))
		PvPrintConsoleA("Failed to set timer event! Error Code: %u\n",GetLastError());
	// Reset the timer to restart it.
	SetWaitableTimer(PvTimerHandle,&PvExpiryPeriod,0,NULL,NULL,FALSE);
}

void PvFinalizeTimerHardware()
{
	if(PvTimerRegistration)
	{
		UnregisterWait(PvTimerRegistration);
		PvTimerRegistration=NULL;
	}
	if(PvTimerHandle)
	{
		CloseHandle(PvTimerHandle);
		PvTimerHandle=NULL;
	}
	if(PvTimerEvent)
	{
		CloseHandle(PvTimerEvent);
		PvTimerEvent=NULL;
	}
}

BOOL PvInitializeTimerHardware(IN LONG64 InterruptInterval)
{
	BOOL Result=FALSE;
	PvTimerHandle=CreateWaitableTimerW(NULL,FALSE,NULL);
	if(PvTimerHandle)
	{
		PvExpiryPeriod.QuadPart=InterruptInterval*-10000;
		Result=SetWaitableTimer(PvTimerHandle,&PvExpiryPeriod,0,NULL,NULL,FALSE);
		if(Result)Result=RegisterWaitForSingleObject(&PvTimerRegistration,PvTimerHandle,PvTimerHardwareCallback,NULL,INFINITE,WT_EXECUTEDEFAULT);
		if(Result)PvTimerEvent=CreateEventA(NULL,FALSE,FALSE,NULL);
		Result=PvTimerEvent!=NULL;
	}
	return Result;
}