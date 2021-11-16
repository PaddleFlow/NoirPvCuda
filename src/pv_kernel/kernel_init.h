// Some definitions about paravirtualization...

#define ConsoleInputPort	0
#define ConsoleOutputPort	1
#define ConsoleErrorPort	2

#define HV_X64_MSR_GUEST_OS_ID	0x40000000
#define HV_X64_MSR_HYPERCALL	0x40000001

#define NOIR_HYPERCALL_CODE_SHUTDOWN		0x0

#define NOIR_DUMMY_INTERRUPT_VECTOR		0x20

#define HelloString			"Hello World from Paravirtualized Kernel!\n"

CHAR DummyString[]="Dummy Interrupt occurred!\n";
ULONG64 DummyStringLength=sizeof(DummyString);

typedef ULONG64 (__cdecl *NOIR_HYPERCALL)(ULONG32 HypercallIndex,...);

#pragma pack(1)
typedef struct _KIDTENTRY64
{
	USHORT OffsetLow;
	USHORT Selector;
	struct
	{
		USHORT IST:3;
		USHORT Reserved1:5;
		USHORT Type:4;
		USHORT Reserved2:1;
		USHORT DPL:2;
		USHORT Present:1;
	};
	USHORT OffsetMid;
	ULONG32 OffsetHigh;
	ULONG32 Reserved;
}KIDTENTRY64,*PKIDTENTRY64;
#pragma pack()

#pragma pack(1)
typedef struct _KDESCRIPTOR
{
	USHORT Limit;
	ULONG_PTR Base;
}KDESCRIPTOR,*PKDESCRIPTOR;
#pragma pack()

NOIR_HYPERCALL NoirHypercall;
void PvDummyInterrupt();