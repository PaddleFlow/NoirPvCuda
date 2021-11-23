#include <pvdef.h>
#include <hypercall.h>
#include "kernel_init.h"

void PvDummyPrinter()
{
	// Invoked by dummy interrupt.
	CHAR Input[256];
	ULONG Length=0;
	__outbytestring(ConsoleOutputPort,DummyString,DummyStringLength);
	__inbytestring(ConsoleInputPort,Input,sizeof(Input));
	for(ULONG i=0;i<254;i++)
	{
		if(Input[i]=='\0')
		{
			Input[i]='\n';
			Length=i+1;
			break;
		}
	}
	Input[254]='\n';
	Input[255]='\0';
	__outbytestring(ConsoleOutputPort,Input,Length);
}

void PvSetupSystemLinkage()
{
	// User selector: 0x53. Kernel selector: 0x10.
	__writemsr(MSR_STAR,0x0053001000000000);
	// Specify the system call handler address.
	__writemsr(MSR_LSTAR,(ULONG64)PvSystemCall64);
	// Compatibility-Mode system call is unsupported.
	__writemsr(MSR_CSTAR,0);
	// Mask TF, IF, DF and NT bits in RFlags during system call.
	__writemsr(MSR_SFMASK,0x4700);
	// FIXME: Setup Kernel GS MSR for swapgs instruction purpose.
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
	// Setup System Linkage.
	PvSetupSystemLinkage();
	// Use "rep outsb" instruction to output string to paravirtualized console.
	__outbytestring(ConsoleOutputPort,HelloString,sizeof(HelloString));
	// Enable interrupts. Otherwise the processor might be permanently blocked.
	_enable();
	// Initialization complete, start thread scheduling.
	__halt();
	// Use a shutdown hypercall to directly exit.
	NoirHypercall(NOIR_HYPERCALL_CODE_SHUTDOWN);
	// Return from this function would trigger a page-fault,
	// unless there is ROP attack or exception handler is set.
}