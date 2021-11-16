// Main header.

void PvInitializeConsole();
ULONG PvPrintConsoleA(IN PCSTR Format,...);
ULONG PvPrintConsoleW(IN PCWSTR Format,...);
BOOL PvInitializeGuestInternalMapping();
BOOL PvLoadParavirtualizedKernel();
NOIR_STATUS PvStartParavirtualizedGuest();
NOIR_STATUS PvInitializeVirtualMachine(IN ULONG32 VpCount);
void PvFinalizeVirtualMachine();

BOOLEAN DumpMemory=FALSE;