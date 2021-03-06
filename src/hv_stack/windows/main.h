// Main header.

void NoirInitializeDisassembler();
void PvInitializeConsole();
BOOL PvCreateGuestConsole();
void PvTerminateGuestConsole();
ULONG PvPrintConsoleA(IN PCSTR Format,...);
ULONG PvPrintConsoleW(IN PCWSTR Format,...);
BOOL PvInitializeGuestInternalMapping();
BOOL PvLoadParavirtualizedKernel();
NOIR_STATUS PvStartParavirtualizedGuest();
NOIR_STATUS PvInitializeVirtualMachine(IN ULONG32 VpCount);
void PvFinalizeVirtualMachine();
BOOL PvdInitialize();
BOOL PvdFinalize();

BOOLEAN DumpMemory=FALSE;