#include <Windows.h>

ULONG PvPrintConsoleA(IN PCSTR Format,...);

LARGE_INTEGER PvExpiryPeriod;
HANDLE PvTimerHandle=NULL;
HANDLE PvTimerRegistration=NULL;
HANDLE PvTimerEvent=NULL;