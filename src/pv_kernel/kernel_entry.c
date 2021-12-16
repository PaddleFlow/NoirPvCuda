#include <pvdef.h>
#include <hypercall.h>
#include <pv_fileio.h>
#include <ps.h>
#include <mm.h>
#include <cudart.h>
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
	NoirHypercall(NOIR_HYPERCALL_CODE_SHUTDOWN);
}

void PvTimerInterruptHandler()
{
	CHAR Output[]="Timer Interrupt Occured!\n";
	__outbytestring(ConsoleOutputPort,Output,sizeof(Output));
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
	IdtEntry=(PKIDTENTRY64)IdtR.Base;
	// Initialize Dummy Interrupt Vector.
	*(PULONG64)((ULONG_PTR)&IdtEntry[NOIR_DUMMY_INTERRUPT_VECTOR]+4)=(ULONG64)PvDummyInterrupt;
	IdtEntry[NOIR_DUMMY_INTERRUPT_VECTOR].OffsetLow=(USHORT)PvDummyInterrupt;
	IdtEntry[NOIR_DUMMY_INTERRUPT_VECTOR].Selector=0x10;
	IdtEntry[NOIR_DUMMY_INTERRUPT_VECTOR].IST=0;
	IdtEntry[NOIR_DUMMY_INTERRUPT_VECTOR].Reserved1=0;
	IdtEntry[NOIR_DUMMY_INTERRUPT_VECTOR].Type=0xE;		// 64-Bit Interrupt Gate.
	IdtEntry[NOIR_DUMMY_INTERRUPT_VECTOR].Reserved2=0;
	IdtEntry[NOIR_DUMMY_INTERRUPT_VECTOR].DPL=0;
	IdtEntry[NOIR_DUMMY_INTERRUPT_VECTOR].Present=1;
	IdtEntry[NOIR_DUMMY_INTERRUPT_VECTOR].Reserved=0;
	// Initialize Timer Interrupt Vector.
	*(PULONG64)((ULONG_PTR)&IdtEntry[NOIR_TIMER_INTERRUPT_VECTOR]+4)=(ULONG64)PvTimerInterrupt;
	IdtEntry[NOIR_TIMER_INTERRUPT_VECTOR].OffsetLow=(USHORT)PvTimerInterrupt;
	IdtEntry[NOIR_TIMER_INTERRUPT_VECTOR].Selector=0x10;
	IdtEntry[NOIR_TIMER_INTERRUPT_VECTOR].IST=0;
	IdtEntry[NOIR_TIMER_INTERRUPT_VECTOR].Reserved1=0;
	IdtEntry[NOIR_TIMER_INTERRUPT_VECTOR].Type=0xE;		// 64-Bit Interrupt Gate.
	IdtEntry[NOIR_TIMER_INTERRUPT_VECTOR].Reserved2=0;
	IdtEntry[NOIR_TIMER_INTERRUPT_VECTOR].DPL=0;
	IdtEntry[NOIR_TIMER_INTERRUPT_VECTOR].Present=1;
	IdtEntry[NOIR_TIMER_INTERRUPT_VECTOR].Reserved=0;
}

void PvIdleLoop()
{
	// Enter an infinite loop to schedule tasks.
	while(1)__halt();
}

void PvCudaTest()
{
	int DevCount;
	cudaError_t ErrCode=cudaGetDeviceCount(&DevCount);
	if(ErrCode!=cudaSuccess)
		PvPrintStdOut("Failed to get device count!\n");
	else
	{
		struct cudaDeviceProp DevProp;
		PvPrintStdOut("Number of Device Count: %u\n",DevCount);
		ErrCode=cudaGetDeviceProperties(&DevProp,0);
		PvPrintStdOut("Name of Cuda Device #0 is %s\n",DevProp.name);
	}
}

void PvKernelEntry(IN NOIR_HYPERCALL HypercallFunction)
{
	// HANDLE ImageHandle;
	// PKPROCESS InitialProcess;
	NoirHypercall=HypercallFunction;
	// Setup Interrupt Descriptor Table. Otherwise the processor could not correctly handle interrupts.
	PvSetupInterruptHandlers();
	// Setup System Linkage.
	PvSetupSystemLinkage();
	// Initialize Memory Manager.
	MmInitializeMemoryManager();
	// Initialize User/Kernel Shared Region
	MmInitializeUserKernelSharedRegion();
	// Initialize Process Manager.
	PspInitialzeProcessManager();
	// Use "rep outsb" instruction to output string to paravirtualized console.
	__outbytestring(ConsoleOutputPort,HelloString,sizeof(HelloString));
	// Enable interrupts. Otherwise the processor might be permanently blocked.
	_enable();
	// Test cuda calls.
	PvCudaTest();
	// Create a test process.
	// ImageHandle=PvCreateFileA("/CudaDetector.exe",GENERIC_READ,FILE_SHARE_READ,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL);
	// PvPrintStdOut("Successfully opened image! Handle=0x%p!\n",ImageHandle);
	// PsCreateProcess(&InitialProcess,ImageHandle);
	// Initialization complete, start thread scheduling.
	PvIdleLoop();
	// Never returns.
	// Use a shutdown hypercall to directly exit.
	NoirHypercall(NOIR_HYPERCALL_CODE_SHUTDOWN);
	// Return from this function would trigger a page-fault,
	// unless there is ROP attack or exception handler is set.
}