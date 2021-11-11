// Some definitions about paravirtualization...

#define ConsoleInputPort	0
#define ConsoleOutputPort	1
#define ConsoleErrorPort	2

#define HV_X64_MSR_GUEST_OS_ID	0x40000000
#define HV_X64_MSR_HYPERCALL	0x40000001

#define NOIR_HYPERCALL_CODE_SHUTDOWN		0x0

#define HelloString			"Hello World from Paravirtualized Kernel!\n"

typedef ULONG64 (__cdecl *NOIR_HYPERCALL)(ULONG32 HypercallIndex,...);

NOIR_HYPERCALL NoirHypercall;