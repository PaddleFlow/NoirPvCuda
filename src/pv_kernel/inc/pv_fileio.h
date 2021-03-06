#include <pvdef.h>

#define DELETE							(0x00010000L)
#define READ_CONTROL					(0x00020000L)
#define WRITE_DAC						(0x00040000L)
#define WRITE_OWNER						(0x00080000L)
#define SYNCHRONIZE						(0x00100000L)

#define STANDARD_RIGHTS_REQUIRED		(0x000F0000L)

#define STANDARD_RIGHTS_READ			(READ_CONTROL)
#define STANDARD_RIGHTS_WRITE			(READ_CONTROL)
#define STANDARD_RIGHTS_EXECUTE			(READ_CONTROL)

#define STANDARD_RIGHTS_ALL				(0x001F0000L)

#define SPECIFIC_RIGHTS_ALL				(0x0000FFFFL)

#define GENERIC_ALL						0x10000000
#define GENERIC_EXECUTE					0x20000000
#define GENERIC_WRITE					0x40000000
#define GENERIC_READ					0x80000000

#define FILE_READ_DATA					0x1
#define FILE_LIST_DIRECTORY				0x1
#define FILE_WRITE_DATA					0x2
#define FILE_ADD_FILE					0x2
#define FILE_ADD_SUBDIRECTORY			0x4
#define FILE_APPEND_DATA				0x4
#define FILE_CREATE_PIPE_INSTANCE		0x4
#define FILE_READ_EA					0x8
#define FILE_WRITE_EA					0x10
#define FILE_EXECUTE					0x20
#define FILE_TRAVERSE					0x20
#define FILE_DELETE_CHILD				0x40
#define FILE_READ_ATTRIBUTES			0x80
#define FILE_WRITE_ATTRIBUTES			0x100

#define FILE_BEGIN 						0
#define FILE_CURRENT					1
#define FILE_END						2

#define CREATE_NEW						1
#define CREATE_ALWAYS					2
#define OPEN_EXISTING					3
#define OPEN_ALWAYS						4
#define TRUNCATE_EXISTING				5

#define FILE_SHARE_READ					0x1
#define FILE_SHARE_WRITE				0x2
#define FILE_SHARE_DELETE				0x4

#define FILE_ATTRIBUTE_READONLY			0x1
#define FILE_ATTRIBUTE_HIDDEN			0x2
#define FILE_ATTRIBUTE_SYSTEM			0x4
#define FILE_ATTRIBUTE_ARCHIVE			0x20
#define FILE_ATTRIBUTE_NORMAL			0x80
#define FILE_ATTRIBUTE_TEMPORARY		0x100
#define FILE_ATTRIBUTE_OFFLINE			0x1000
#define FILE_ATTRIBUTE_ENCRYPTED		0x4000
#define FILE_FLAG_OPEN_NO_RECALL		0x100000
#define FILE_FLAG_OPEN_REPARSE_POINT	0x200000
#define FILE_FLAG_SESSION_AWARE			0x800000
#define FILE_FLAG_POSIX_SEMANTICS		0x1000000
#define FILE_FLAG_BACKUP_SEMANTICS		0x2000000
#define FILE_FLAG_DELETE_ON_CLOSE		0x4000000
#define FILE_FLAG_SEQUENTIAL_SCAN		0x8000000
#define FILE_FLAG_RANDOM_ACCESS			0x10000000
#define FILE_FLAG_NO_BUFFERING			0x20000000
#define FILE_FLAG_OVERLAPPED			0x40000000
#define FILE_FLAG_WRITE_THROUGH			0x80000000

#define INVALID_HANDLE_VALUE			(HANDLE)-1

typedef PVOID HANDLE;

HANDLE PvCreateFileA(IN PSTR FilePath,IN ULONG DesiredAccess,IN ULONG ShareMode,IN ULONG Disposition,IN ULONG Flags);
BOOL PvCloseFile(IN HANDLE FileHandle);
BOOL PvReadFile(IN HANDLE FileHandle,OUT PVOID Buffer,IN ULONG NumberOfBytes);
BOOL PvWriteFile(IN HANDLE FileHandle,IN PVOID Buffer,IN ULONG NumberOfBytes);
BOOL PvSetFilePointer(IN HANDLE FileHandle,IN LONG64 DistanceToMove,IN ULONG MoveMethod);