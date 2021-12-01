// Some definitions about paravirtualization...

#define ConsoleInputPort	0
#define ConsoleOutputPort	1
#define ConsoleErrorPort	2

#define MSR_STAR			0xC0000081
#define MSR_LSTAR			0xC0000082
#define MSR_CSTAR			0xC0000083
#define MSR_SFMASK			0xC0000084
#define MSR_FS_BASE			0xC0000100
#define MSR_GS_BASE			0xC0000101
#define MSR_KGS_BASE		0xC0000102

#define NOIR_DUMMY_INTERRUPT_VECTOR		0x20
#define NOIR_TIMER_INTERRUPT_VECTOR		0x21

#define HelloString			"Hello World from Paravirtualized Kernel!\n"

CHAR DummyString[]="Dummy Interrupt occurred!\n";
ULONG64 DummyStringLength=sizeof(DummyString);

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
typedef struct _KGDTENTRY64
{
	USHORT Limit;
	USHORT BaseLow;
	UCHAR BaseMid1;
	union
	{
		struct
		{
			USHORT Type:4;
			USHORT User:1;
			USHORT DPL:2;
			USHORT Present:1;
			USHORT LimitHi:4;
			USHORT Available:1;
			USHORT Long:1;
			USHORT DefaultBig:1;
			USHORT Granularity:1;
		};
		USHORT Attributes;
	};
	UCHAR BaseMid2;
	ULONG32 BaseHigh;
	ULONG32 Reserved;
}KGDTENTRY64,*PKGDTENTRY64;
#pragma pack()

#pragma pack(1)
typedef struct _KTSSENTRY64
{
	ULONG32 Reserved0;
	ULONG64 Rsp0;
	ULONG64 Rsp1;
	ULONG64 Rsp2;
	ULONG64 Reserved1;
	ULONG64 Ist1;
	ULONG64 Ist2;
	ULONG64 Ist3;
	ULONG64 Ist4;
	ULONG64 Ist5;
	ULONG64 Ist6;
	ULONG64 Ist7;
	ULONG64 Reserved2;
	USHORT Reserved3;
	USHORT IoMapBase;
}KTSSENTRY64,*PKTSSENTRY64;
#pragma pack()

#pragma pack(1)
typedef struct _KDESCRIPTOR
{
	USHORT Limit;
	ULONG_PTR Base;
}KDESCRIPTOR,*PKDESCRIPTOR;
#pragma pack()

// Paravirtualized Kernel Processor Block.
#pragma pack(8)
typedef struct _PVKPB64
{
	ULONG64 IdtBase;
	ULONG64 GdtBase;
	ULONG64 TssBase;
	ULONG64 Self;
	ULONG32 ProcessorId;
	ULONG64 UserRsp;
}PVKPB,*PPVKPB;
#pragma pack()

// Paravirtualized User Thread Environment Block
#pragma pack(8)
typedef struct _PVUTEB64
{
	struct
	{
		ULONG64 ElfImage:1;
		ULONG64 Reserved:63;
	}Flags;
}PVUTEB64,*PPVUTEB64;
#pragma pack()

void PvDummyInterrupt(void);
void PvTimerInterrupt(void);
void PvSystemCall64(void);

PVOID PvSystemCallServiceList[1]={NULL};
UCHAR PvSystemCallArgumentList[1]={0};