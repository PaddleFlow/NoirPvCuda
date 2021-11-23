/*
  NoirPvCuda PvKernel - Paravirtualized Kernel for Cuda Platform via NoirVisor CVM

  This is the header file defining basic data types for general purpose.
*/

#pragma once

// PvKernel Basic Types.
typedef unsigned __int8		UINT8;
typedef unsigned __int16	UINT16;
typedef unsigned __int32	UINT32;
typedef unsigned __int64	UINT64;

typedef signed __int8		INT8;
typedef signed __int16		INT16;
typedef signed __int32		INT32;
typedef signed __int64		INT64;

#if defined(_M_AMD64)
typedef UINT64	UINTN;
typedef INT64	INTN;
#else
typedef UINT32	UINTN;
typedef INT32	INTN;
#endif

#define FALSE	0
#define TRUE	1

#define NULL	(void*)0

typedef UINT32	BOOL;
typedef UINT8	BOOLEAN;

typedef void*	PVOID;

typedef UINT8*	PUINT8;
typedef UINT16*	PUINT16;
typedef UINT32*	PUINT32;
typedef	UINT64*	PUINT64;
typedef UINTN*	PUINTN;

typedef INT8*	PINT8;
typedef INT16*	PINT16;
typedef INT32*	PINT32;
typedef INT64*	PINT64;
typedef INTN*	PINTN;

typedef union _LARGE_INTEGER
{
	struct
	{
		UINT32 LowPart;
		UINT32 HighPart;
	};
	UINT64 QuadPart;
}LARGE_INTEGER,*PLARGE_INTEGER;

// Compatibility Types with NT DDK.
typedef UINT8	BYTE;
typedef UINT16	USHORT;
typedef UINT32	ULONG32;
typedef UINT64	ULONG64;

typedef INT8	SBYTE;
typedef INT16	SHORT;
typedef INT32	LONG32;
typedef INT64	LONG64;

typedef UINTN	ULONG_PTR;
typedef INTN	LONG_PTR;

typedef BYTE		UCHAR;
typedef ULONG32		ULONG;
typedef ULONG64 	ULONGLONG;
typedef SBYTE		CHAR;
typedef LONG32		LONG;
typedef LONG64		LONGLONG;

typedef BYTE*		PBYTE;
typedef USHORT*		PUSHORT;
typedef ULONG32*	PULONG32;
typedef ULONG64*	PULONG64;
typedef ULONG_PTR*	PULONG_PTR;

typedef	SBYTE*		PSBYTE;
typedef SHORT*		PSHORT;
typedef LONG32*		PLONG32;
typedef LONG64*		PLONG64;
typedef LONG_PTR*	PLONG_PTR;

typedef UCHAR*		PUCHAR;
typedef ULONG*		PULONG;
typedef ULONGLONG*	PULONGLONG;
typedef CHAR*		PCHAR;
typedef LONG*		PLONG;
typedef LONGLONG	PLONGLONG;

// Parameter passing options
#define IN
#define OUT
#define OPTIONAL