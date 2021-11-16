#include <ntdef.h>
#include <ntimage.h>
#include "kernel_init.h"

void PvDummyPrinter()
{
	// Invoked by dummy interrupt.
	CHAR Input[256];
	__outbytestring(ConsoleOutputPort,DummyString,DummyStringLength);
	__inbytestring(ConsoleInputPort,Input,sizeof(Input));
	__outbytestring(ConsoleOutputPort,Input,sizeof(Input));
}

void PvSetupInterruptHandlers()
{
	PKIDTENTRY64 IdtEntry;
	KDESCRIPTOR IdtR;
	__sidt(&IdtR);
	IdtEntry=(PKIDTENTRY64)(IdtR.Base+0x200);
	// Initialize Dummy Interrupt Vector.
	*(PULONG64)((ULONG_PTR)IdtEntry+4)=(ULONG64)PvDummyInterrupt;
	IdtEntry->OffsetLow=(USHORT)PvDummyInterrupt;
	IdtEntry->Selector=0x10;
	IdtEntry->IST=0;
	IdtEntry->Reserved1=0;
	IdtEntry->Type=0xE;		// 64-Bit Interrupt Gate.
	IdtEntry->Reserved2=0;
	IdtEntry->DPL=0;
	IdtEntry->Present=1;
	IdtEntry->Reserved=0;
}

void PvKernelEntry(IN NOIR_HYPERCALL HypercallFunction)
{
	NoirHypercall=HypercallFunction;
	// Setup Interrupt Descriptor Table. Otherwise the processor could not correctly handle interrupts.
	PvSetupInterruptHandlers();
	// Use "rep outsb" instruction to output string to paravirtualized console.
	__outbytestring(ConsoleOutputPort,HelloString,sizeof(HelloString));
	// Enable interrupts. Otherwise the processor might be permanently blocked.
	_enable();
	__halt();
	// Use a shutdown hypercall to directly exit.
	NoirHypercall(NOIR_HYPERCALL_CODE_SHUTDOWN);
	// Return from this function would trigger a page-fault,
	// unless there is ROP attack or exception handler is set.
}