#include <pvdef.h>

// Page Structure definitions
// See Chapter 5.3, Volume 2, AMD64 Architecture Programmer's Manual.

typedef union _VIRTUAL_ADDRESS
{
	struct
	{
		ULONG64 PageOffset:12;
		ULONG64 PteIndex:9;
		ULONG64 PdeIndex:9;
		ULONG64 PdpteIndex:9;
		ULONG64 Pml4eIndex:9;
		ULONG64 SignOffset:16;
	}Normal;
	struct
	{
		ULONG64 PageOffset:21;
		ULONG64 PdeIndex:9;
		ULONG64 PdpteIndex:9;
		ULONG64 Pml4eIndex:9;
		ULONG64 SignOffset:16;
	}Large;
	struct
	{
		ULONG64 PageOffset:30;
		ULONG64 PdpteIndex:9;
		ULONG64 Pml4eIndex:9;
		ULONG64 SignOffset:16;
	}Huge;
	ULONG64 Value;
}VIRTUAL_ADDRESS,*PVIRTUAL_ADDRESS;

typedef union _KPML4E
{
	struct
	{
		ULONG64 Present:1;
		ULONG64 Write:1;
		ULONG64 User:1;
		ULONG64 WriteThrough:1;
		ULONG64 CacheDisable:1;
		ULONG64 Accessed:1;
		ULONG64 Ignored:1;
		ULONG64 Reserved0:2;
		ULONG64 Available:3;
		ULONG64 PdpteBase:40;
		ULONG64 Reserved1:11;
		ULONG64 NoExecute:1;
	};
	ULONG64 Value;
}KPML4E,*PKPML4E;

typedef union _KPDPTE
{
	struct
	{
		ULONG64 Present:1;
		ULONG64 Write:1;
		ULONG64 User:1;
		ULONG64 WriteThrough:1;
		ULONG64 CacheDisable:1;
		ULONG64 Accessed:1;
		ULONG64 Dirty:1;
		ULONG64 PageSize:1;
		ULONG64 Global:1;
		ULONG64 Available:3;
		ULONG64 Pat:1;
		ULONG64 Reserved0:17;
		ULONG64 PageBase:22;
		ULONG64 Reserved1:7;
		ULONG64 PageKey:4;
		ULONG64 NoExecute:1;
	}Huge;
	struct
	{
		ULONG64 Present:1;
		ULONG64 Write:1;
		ULONG64 User:1;
		ULONG64 WriteThrough:1;
		ULONG64 CacheDisable:1;
		ULONG64 Accessed:1;
		ULONG64 Ignored0:1;
		ULONG64 PageSize:1;
		ULONG64 Ignored1:1;
		ULONG64 Available:3;
		ULONG64 PdeBase:40;
		ULONG64 Reserved1:11;
		ULONG64 NoExecute:1;
	}Normal;
	ULONG64 Value;
}KPDPTE,*PKPDPTE;

typedef union _KPDE
{
	struct
	{
		ULONG64 Present:1;
		ULONG64 Write:1;
		ULONG64 User:1;
		ULONG64 WriteThrough:1;
		ULONG64 CacheDisable:1;
		ULONG64 Accessed:1;
		ULONG64 Dirty:1;
		ULONG64 PageSize:1;
		ULONG64 Global:1;
		ULONG64 Available:3;
		ULONG64 Pat:1;
		ULONG64 Reserved0:8;
		ULONG64 PageBase:31;
		ULONG64 Reserved1:7;
		ULONG64 PageKey:4;
		ULONG64 NoExecute:1;
	}Large;
	struct
	{
		ULONG64 Present:1;
		ULONG64 Write:1;
		ULONG64 User:1;
		ULONG64 WriteThrough:1;
		ULONG64 CacheDisable:1;
		ULONG64 Accessed:1;
		ULONG64 Ignored0:1;
		ULONG64 PageSize:1;
		ULONG64 Ignored1:1;
		ULONG64 Available:3;
		ULONG64 PteBase:40;
		ULONG64 Reserved1:11;
		ULONG64 NoExecute:1;
	}Normal;
	ULONG64 Value;
}KPDE,*PKPDE;

typedef union _KPTE
{
	struct
	{
		ULONG64 Present:1;
		ULONG64 Write:1;
		ULONG64 User:1;
		ULONG64 WriteThrough:1;
		ULONG64 CacheDisable:1;
		ULONG64 Accessed:1;
		ULONG64 Dirty:1;
		ULONG64 Pat:1;
		ULONG64 Global:1;
		ULONG64 Available:3;
		ULONG64 PageBase:40;
		ULONG64 Reserved1:7;
		ULONG64 PageKey:4;
		ULONG64 NoExecute:1;
	};
	ULONG64 Value;
}KPTE,*PKPTE;