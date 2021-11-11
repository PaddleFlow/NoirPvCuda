// Processor Definitions

#define PAGE_BASE(x)			(x&0xFFFFFFFFFFFFF000)
#define PAGE_SIZE				0x1000
#define PAGE_SHIFT				12
#define PAGE_LARGE_SHIFT		21
#define PAGE_HUGE_SHIFT			30

#define RFLAGS_CF	0
#define RFLAGS_PF	2
#define RFLAGS_AF	4
#define RFLAGS_ZF	6
#define RFLAGS_SF	7
#define RFLAGS_TF	8
#define RFLAGS_IF	9
#define RFLAGS_DF	10
#define RFLAGS_OF	11
#define RFLAGS_NT	14
#define RFLAGS_RF	16
#define RFLAGS_VM	17
#define RFLAGS_AC	18
#define RFLAGS_VIF	19
#define RFLAGS_VIP	20
#define RFLAGS_ID	21

#pragma pack(1)
typedef struct _KGDTENTRY64
{
	USHORT Limit;
	USHORT BaseLow;
	BYTE BaseMid1;
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
	BYTE BaseMid2;
	ULONG32 BaseHigh;
	ULONG32 Reserved;
}KGDTENTRY64,*PKGDTENTRY64;
#pragma pack()

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

#pragma pack(4)
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

typedef union _AMD64_ADDRESS48
{
	struct
	{
		ULONG64 PageOffset:12;
		ULONG64 PteIndex:9;
		ULONG64 PdeIndex:9;
		ULONG64 PdpteIndex:9;
		ULONG64 Pml4eIndex:9;
		ULONG64 SignExtend:16;
	}L4;
	struct
	{
		ULONG64 PageOffset:21;
		ULONG64 PdeIndex:9;
		ULONG64 PdpteIndex:9;
		ULONG64 Pml4eIndex:9;
		ULONG64 SignExtend:16;
	}L3;
	struct
	{
		ULONG64 PageOffset:30;
		ULONG64 PdpteIndex:9;
		ULONG64 Pml4eIndex:9;
		ULONG64 SignExtend:16;
	}L2;
	ULONG64 Value;
}AMD64_ADDRESS48,*PAMD64_ADDRESS48;

typedef union _AMD64_PML4E
{
	struct
	{
		ULONG64 Present:1;
		ULONG64 Write:1;
		ULONG64 User:1;
		ULONG64 Pwt:1;
		ULONG64 Pcd:1;
		ULONG64 Accessed:1;
		ULONG64 Ignored:1;
		ULONG64 Reserved0:2;
		ULONG64 Available:3;
		ULONG64 PdpteBase:40;
		ULONG64 Reserved1:11;
		ULONG64 NoExecute:1;
	};
	ULONG64 Value;
}AMD64_PML4E,*PAMD64_PML4E;

typedef union _AMD64_PDPTE
{
	struct
	{
		ULONG64 Present:1;
		ULONG64 Write:1;
		ULONG64 User:1;
		ULONG64 Pwt:1;
		ULONG64 Pcd:1;
		ULONG64 Accessed:1;
		ULONG64 Ignored0:1;
		ULONG64 HugePage:1;
		ULONG64 Ignored1:1;
		ULONG64 Available:3;
		ULONG64 PdeBase:40;
		ULONG64 Reserved:11;
		ULONG64 NoExecute:1;
	}NonHuge;
	struct
	{
		ULONG64 Present:1;
		ULONG64 Write:1;
		ULONG64 User:1;
		ULONG64 Pwt:1;
		ULONG64 Pcd:1;
		ULONG64 Accessed:1;
		ULONG64 Dirty:1;
		ULONG64 HugePage:1;
		ULONG64 Global:1;
		ULONG64 Available:3;
		ULONG64 Pat:1;
		ULONG64 Reserved0:17;
		ULONG64 PageBase:22;
		ULONG64 Reserved1:7;
		ULONG64 ProtectionKey:4;
		ULONG64 NoExecute:1;
	}Huge;
	ULONG64 Value;
}AMD64_PDPTE,*PAMD64_PDPTE;

typedef union _AMD64_PDE
{
	struct
	{
		ULONG64 Present:1;
		ULONG64 Write:1;
		ULONG64 User:1;
		ULONG64 Pwt:1;
		ULONG64 Pcd:1;
		ULONG64 Accessed:1;
		ULONG64 Ignored0:1;
		ULONG64 LargePage:1;
		ULONG64 Ignored1:1;
		ULONG64 Available:3;
		ULONG64 PteBase:40;
		ULONG64 Reserved:11;
		ULONG64 NoExecute:1;
	}NonLarge;
	struct
	{
		ULONG64 Present:1;
		ULONG64 Write:1;
		ULONG64 User:1;
		ULONG64 Pwt:1;
		ULONG64 Pcd:1;
		ULONG64 Accessed:1;
		ULONG64 Dirty:1;
		ULONG64 LargePage:1;
		ULONG64 Global:1;
		ULONG64 Available:3;
		ULONG64 Pat:1;
		ULONG64 Reserved0:8;
		ULONG64 PageBase:31;
		ULONG64 Reserved1:7;
		ULONG64 ProtectionKey:4;
		ULONG64 NoExecute:1;
	}Large;
	ULONG64 Value;
}AMD64_PDE,*PAMD64_PDE;

typedef union _AMD64_PTE
{
	struct
	{
		ULONG64 Present:1;
		ULONG64 Write:1;
		ULONG64 User:1;
		ULONG64 Pwt:1;
		ULONG64 Pcd:1;
		ULONG64 Accessed:1;
		ULONG64 Dirty:1;
		ULONG64 Pat:1;
		ULONG64 Global:1;
		ULONG64 Available:3;
		ULONG64 PageBase:40;
		ULONG64 Reserved:7;
		ULONG64 ProtectionKey:4;
		ULONG64 NoExecute:1;
	};
	ULONG64 Value;
}AMD64_PTE,*PAMD64_PTE;

typedef union _AMD64_PAGE_FAULT_ERROR_CODE
{
	struct
	{
		ULONG32 Present:1;
		ULONG32 Write:1;
		ULONG32 User:1;
		ULONG32 ReservedBit:1;
		ULONG32 Execute:1;
		ULONG32 ProtectionKey:1;
		ULONG32 ShadowStack:1;
		ULONG32 Reserved:25;
	};
	ULONG32 Value;
}AMD64_PAGE_FAULT_ERROR_CODE,*PAMD64_PAGE_FAULT_ERROR_CODE;