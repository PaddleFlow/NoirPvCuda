#include <pvdef.h>

// Definitions of Hypercalls.

#define NOIR_HYPERCALL_CODE_SHUTDOWN		0x0
#define NOIR_HYPERCALL_MEMORY_EXTENSION		0x1
#define NOIR_HYPERCALL_FILE_CREATE			0x2
#define NOIR_HYPERCALL_FILE_CLOSE			0x3
#define NOIR_HYPERCALL_FILE_READ_SYNC		0x4
#define NOIR_HYPERCALL_FILE_WRITE_SYNC		0x5
#define NOIR_HYPERCALL_FILE_SET_POINTER		0x6
#define NOIR_HYPERCALL_FILE_REMOVE			0x7
#define NOIR_HYPERCALL_FILE_READ_ASYNC		0x8
#define NOIR_HYPERCALL_FILE_WRITE_ASYNC		0x9

#define NOIR_HYPERCALL_CUDA_GET_DEVICE_COUNT		0x10000
#define NOIR_HYPERCALL_CUDA_GET_DEVICE_PROP			0x10001

typedef ULONG64 (__cdecl *NOIR_HYPERCALL)(ULONG32 HypercallIndex,...);

#if defined(_KERNEL_INIT)
NOIR_HYPERCALL NoirHypercall=NULL;
#else
extern NOIR_HYPERCALL NoirHypercall;
#endif