#include <Windows.h>

ULONG PvPrintConsoleA(IN PCSTR Format,...);

extern ULONG64 volatile PvTimerSignal;
extern CVM_HANDLE PvCvm;

LARGE_INTEGER PvExpiryPeriod;
HANDLE PvTimerHandle=NULL;
HANDLE PvTimerRegistration=NULL;
HANDLE PvTimerEvent=NULL;