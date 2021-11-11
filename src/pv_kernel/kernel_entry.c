#include <ntdef.h>
#include <ntimage.h>
#include "kernel_init.h"

void PvKernelEntry(IN NOIR_HYPERCALL HypercallFunction)
{
	NoirHypercall=HypercallFunction;
	// Use "rep outsb" instruction to output string to paravirtualized console.
	__outbytestring(ConsoleOutputPort,HelloString,sizeof(HelloString));
	// Use a shutdown hypercall to directly exit.
	NoirHypercall(NOIR_HYPERCALL_CODE_SHUTDOWN);
	// Return from this function would trigger a triple-fault,
	// unless there is ROP attack or exception handler is set.
}