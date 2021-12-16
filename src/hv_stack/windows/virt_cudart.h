#include <Windows.h>
#include <cuda_runtime.h>

#define NOIR_HYPERCALL_CUDA_GET_DEVICE_COUNT		0x10000
#define NOIR_HYPERCALL_CUDA_GET_DEVICE_PROP			0x10001

ULONG PvPrintConsoleA(IN PCSTR Format,...);
ULONG PvPrintConsoleW(IN PCWSTR Format,...);
PVOID MemAlloc(IN SIZE_T Length);
BOOL MemFree(IN PVOID Memory);

ULONG PvReadGuestMemory(IN ULONG64 GuestCr3,IN ULONG64 GuestAddress,OUT PVOID Buffer,IN ULONG Size,IN BOOLEAN VirtualAddress,IN ULONG64 Access);
ULONG PvWriteGuestMemory(IN ULONG64 GuestCr3,IN ULONG64 GuestAddress,OUT PVOID Buffer,IN ULONG Size,IN BOOLEAN VirtualAddress,IN ULONG64 Access);