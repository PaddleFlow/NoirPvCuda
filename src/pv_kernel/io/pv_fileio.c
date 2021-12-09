#include <pvdef.h>
#include <hypercall.h>
#include <pv_fileio.h>

HANDLE PvCreateFileA(IN PSTR FilePath,IN ULONG DesiredAccess,IN ULONG ShareMode,IN ULONG Disposition,IN ULONG Flags)
{
	LARGE_INTEGER Option1,Option2;
	Option1.LowPart=DesiredAccess;
	Option1.HighPart=ShareMode;
	Option2.LowPart=Disposition;
	Option2.HighPart=Flags;
	return (HANDLE)NoirHypercall(NOIR_HYPERCALL_FILE_CREATE,FilePath,Option1.QuadPart,Option2.QuadPart);
}

BOOL PvCloseFile(IN HANDLE FileHandle)
{
	ULONG64 Result=NoirHypercall(NOIR_HYPERCALL_FILE_CLOSE,FileHandle);
	return (BOOL)Result;
}

BOOL PvReadFile(IN HANDLE FileHandle,OUT PVOID Buffer,IN ULONG NumberOfBytes)
{
	ULONG64 Result=NoirHypercall(NOIR_HYPERCALL_FILE_READ_SYNC,FileHandle,Buffer,NumberOfBytes);
	return (BOOL)Result;
}

BOOL PvWriteFile(IN HANDLE FileHandle,IN PVOID Buffer,IN ULONG NumberOfBytes)
{
	ULONG64 Result=NoirHypercall(NOIR_HYPERCALL_FILE_WRITE_SYNC,FileHandle,Buffer,NumberOfBytes);
	return (BOOL)Result;
}

BOOL PvSetFilePointer(IN HANDLE FileHandle,IN LONG64 DistanceToMove,IN ULONG MoveMethod)
{
	ULONG64 Result=NoirHypercall(NOIR_HYPERCALL_FILE_SET_POINTER,FileHandle,DistanceToMove,MoveMethod);
	return (BOOL)Result;
}