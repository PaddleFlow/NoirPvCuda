#include <pvdef.h>

typedef PVOID HANDLE;

HANDLE PvCreateFileA(IN PSTR FilePath,IN ULONG DesiredAccess,IN ULONG ShareMode,IN ULONG Disposition,IN ULONG Flags);
BOOL PvCloseFile(IN HANDLE FileHandle);
BOOL PvReadFile(IN HANDLE FileHandle,OUT PVOID Buffer,IN ULONG NumberOfBytes);
BOOL PvWriteFile(IN HANDLE FileHandle,IN PVOID Buffer,IN ULONG NumberOfBytes);
BOOL PvSetFilePointer(IN HANDLE FileHandle,IN LONG64 DistanceToMove,IN ULONG MoveMethod);