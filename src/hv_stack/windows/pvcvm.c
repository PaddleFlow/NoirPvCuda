#include <Windows.h>
#include <dbghelp.h>
#include <strsafe.h>
#include <NoirCvmApi.h>
#include "pvcvm.h"
#include "pvproc.h"

BOOL PvInitializeGuestInternalMapping()
{
	BOOL Result=FALSE;
	HANDLE hFile=CreateFileW(L"page_base.dat",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD FileSize=GetFileSize(hFile,NULL);
		if(FileSize!=INVALID_FILE_SIZE)
		{
			DWORD ReadSize;
			SetFilePointer(hFile,0,NULL,FILE_BEGIN);
			ReadFile(hFile,PvPagingStructures,FileSize,&ReadSize,NULL);
		}
		CloseHandle(hFile);
	}
	return Result;
}

void PvFinalizeVirtualMachine()
{
	NoirDeleteVirtualMachine(PvCvm);
	UnlockPage(PvPagingStructures,PvPagingStructuresSize);
	UnlockPage(PvCriticalRange,PvCriticalRangeSize);
	UnlockPage(PvBasicFreeMemory,PvBasicFreeMemorySize);
	PageFree(PvPagingStructures);
	PageFree(PvCriticalRange);
	PageFree(PvBasicFreeMemory);
	for(ULONG i=0;i<PvAllocatedMemoryExtensions;i++)
	{
		if(PvMemoryExtensionPointerTable[i])
		{
			UnlockPage(PvMemoryExtensionPointerTable[i],PvExtendedFreeMemorySize);
			PageFree(PvMemoryExtensionPointerTable[i]);
		}
	}
	MemFree(PvMemoryExtensionPointerTable);
	PvFinalizeTimerHardware();
	PvFinalizeFileIoHardware();
	PvdUnloadSymbolForGuestModule(PvBootingModuleGva);
}

// Albeit IDT will be initialized by paravirtualized kernel, we should initialize GDT for guest.
void PvInitializeVirtualProcessorDescriptors(IN ULONG32 VpIndex)
{
	ULONG64 PvKernelBlock=PvCriticalRangeGva+(VpIndex<<(PAGE_SHIFT+2))+0x800;
	ULONG64 PvTssBlock=PvCriticalRangeGva+(VpIndex<<(PAGE_SHIFT+2))+0x480;
	PKGDTENTRY64 DataSeg=(PKGDTENTRY64)((ULONG_PTR)PvCriticalRange+(VpIndex<<PAGE_SHIFT)+0x400+KGDT_KERNEL_DATA64);
	PKGDTENTRY64 CodeSeg=(PKGDTENTRY64)((ULONG_PTR)PvCriticalRange+(VpIndex<<PAGE_SHIFT)+0x400+KGDT_KERNEL_CODE64);
	PKGDTENTRY64 FsSeg=(PKGDTENTRY64)((ULONG_PTR)PvCriticalRange+(VpIndex<<PAGE_SHIFT)+0x400+KGDT_USER_TEB32);
	PKGDTENTRY64 GsSeg=(PKGDTENTRY64)((ULONG_PTR)PvCriticalRange+(VpIndex<<PAGE_SHIFT)+0x400+KGDT_KERNEL_PROC64);
	PKGDTENTRY64 TrSeg=(PKGDTENTRY64)((ULONG_PTR)PvCriticalRange+(VpIndex<<PAGE_SHIFT)+0x400+KGDT_KERNEL_TSS64);
	PKGDTENTRY64 UserCodeSeg32=(PKGDTENTRY64)((ULONG_PTR)PvCriticalRange+(VpIndex<<PAGE_SHIFT)+0x400+KGDT_USER_CODE32);
	PKGDTENTRY64 UserDataSeg=(PKGDTENTRY64)((ULONG_PTR)PvCriticalRange+(VpIndex<<PAGE_SHIFT)+0x400+KGDT_USER_DATA);
	PKGDTENTRY64 UserCodeSeg64=(PKGDTENTRY64)((ULONG_PTR)PvCriticalRange+(VpIndex<<PAGE_SHIFT)+0x400+KGDT_USER_CODE64);
	PKTSSENTRY64 Tss=(PKTSSENTRY64)((ULONG_PTR)PvCriticalRange+(VpIndex<<PAGE_SHIFT)+0x480);
	// Initialize Data Segments - for ES, DS, SS
	DataSeg->Limit=0xFFFF;
	DataSeg->BaseLow=0;
	DataSeg->BaseMid1=0;
	DataSeg->BaseMid2=0;
	DataSeg->Attributes=0xCF93;
	// Initialize Code Segments
	CodeSeg->Limit=0;
	CodeSeg->BaseLow=0;
	CodeSeg->BaseMid1=0;
	CodeSeg->BaseMid2=0;
	CodeSeg->Attributes=0x209B;
	// Initialize FS Segment - 32-bit User Thread Block
	FsSeg->Limit=0xFFFF;
	FsSeg->BaseLow=0;
	FsSeg->BaseMid1=0;
	FsSeg->BaseMid2=0;
	FsSeg->Attributes=0xCF93;
	FsSeg->BaseHigh=0;
	FsSeg->Reserved=0;
	// Initialize GS Segment - 64-bit User Thread Block / Kernel Processor Block
	GsSeg->Limit=0xFFFF;
	GsSeg->BaseLow=PvKernelBlock&0xFFFF;
	GsSeg->BaseMid1=(BYTE)((PvKernelBlock&0xFF0000)>>16);
	GsSeg->BaseMid2=(BYTE)((PvKernelBlock&0xFF000000)>>24);
	GsSeg->Attributes=0xCF93;
	GsSeg->BaseHigh=PvKernelBlock>>32;
	GsSeg->Reserved=0;
	// Initialize Task Segment
	TrSeg->Limit=sizeof(KTSSENTRY64)-1;
	TrSeg->BaseLow=PvTssBlock&0xFFFF;
	TrSeg->BaseMid1=(BYTE)((PvTssBlock&0xFF0000)>>16);
	TrSeg->BaseMid2=(BYTE)((PvTssBlock&0xFF000000)>>24);
	TrSeg->Attributes=0x89;
	TrSeg->BaseHigh=PvTssBlock>>32;
	TrSeg->Reserved=0;
	// Initialize 32-Bit User Code Segment
	UserCodeSeg32->Limit=0xFFFF;
	UserCodeSeg32->BaseLow=0;
	UserCodeSeg32->BaseMid1=0;
	UserCodeSeg32->BaseMid2=0;
	UserCodeSeg32->Attributes=0xCFFB;
	// Initialize User Data Segment
	UserDataSeg->Limit=0xFFFF;
	UserDataSeg->BaseLow=0;
	UserDataSeg->BaseMid1=0;
	UserDataSeg->BaseMid2=0;
	UserDataSeg->Attributes=0xCFF3;
	// Initialize 64-Bit User Code Segment
	UserCodeSeg64->Limit=0;
	UserCodeSeg64->BaseLow=0;
	UserCodeSeg64->BaseMid1=0;
	UserCodeSeg64->BaseMid2=0;
	UserCodeSeg64->Attributes=0x20FB;
	// Initialize Task State
	Tss->IoMapBase=sizeof(KTSSENTRY64);
	Tss->Rsp0=PvBootingModuleGva+PAGE_SIZE*500;
}

NOIR_STATUS PvInitializeVirtualProcessor(IN ULONG32 VpIndex)
{
	// Create the vCPU
	NOIR_STATUS st=NoirCreateVirtualProcessor(PvCvm,VpIndex);
	if(st==NOIR_SUCCESS)
	{
		// Initialize various registers.
		NOIR_CVM_VIRTUAL_PROCESSOR_OPTIONS VpOpt={0};
		NOIR_CR_STATE CrState={0x80050033,PvPagingStructuresBase,0x406F8};
		ULONG64 Dr67[2]={0xFFFF0FF0,0x400};
		NOIR_SR_STATE SrState;
		SEGMENT_REGISTER FgSeg[2];
		SEGMENT_REGISTER DtSeg[2];
		SEGMENT_REGISTER LtSeg[2];
		NOIR_FX_STATE FxState={0};
		ULONG64 RFlags=0x2,Efer=0xD01,Pat=0x0007040600070406,Xcr0=3;
		SrState.Es.Selector=0x18;
		SrState.Es.Attributes=0xC093;
		SrState.Es.Limit=0xFFFFFFFF;
		SrState.Es.Base=0;
		SrState.Cs.Selector=0x10;
		SrState.Cs.Attributes=0x209B;
		SrState.Cs.Limit=0xFFFFFFFF;
		SrState.Cs.Base=0;
		SrState.Ds=SrState.Ss=SrState.Es;
		FgSeg[0].Selector=0x20;
		FgSeg[0].Attributes=0xC093;
		FgSeg[0].Limit=0xFFFFFFFF;
		FgSeg[0].Base=0;
		FgSeg[1].Selector=0x30;
		FgSeg[1].Attributes=0xC093;
		FgSeg[1].Limit=0xFFFFFFFF;
		FgSeg[1].Base=PvCriticalRangeGva+(VpIndex<<PAGE_SHIFT)+0x800;
		DtSeg[0].Limit=0x7F;
		DtSeg[0].Base=PvCriticalRangeGva+(VpIndex<<PAGE_SHIFT)+0x400;
		DtSeg[1].Limit=0xFFF;
		DtSeg[1].Base=PvCriticalRangeGva+(VpIndex<PAGE_SHIFT);
		LtSeg[0].Selector=0x40;
		LtSeg[0].Attributes=0x89;
		LtSeg[0].Limit=sizeof(KTSSENTRY64)-1;
		LtSeg[0].Base=PvCriticalRangeGva+(VpIndex<<(PAGE_SHIFT+2))+0x480;
		LtSeg[1].Selector=0;
		LtSeg[1].Attributes=0;
		LtSeg[1].Limit=0;
		LtSeg[1].Base=0;
		FxState.Fpu.Fcw=0x40;
		FxState.Fpu.Fsw=0x0;
		FxState.Fpu.Ftw=0x0;
		FxState.Fpu.Mxcsr=0x1F80;
		PvInitializeVirtualProcessorDescriptors(VpIndex);
		// Write them into vCPU.
		st=NoirEditVirtualProcessorRegister(PvCvm,VpIndex,NoirCvmFlagsRegister,&RFlags,sizeof(RFlags));
		PvPrintConsoleA("[VM vCPU %u] Flags Register Initialization Status: 0x%X\n",VpIndex,st);
		st=NoirEditVirtualProcessorRegister(PvCvm,VpIndex,NoirCvmControlRegister,&CrState,sizeof(CrState));
		PvPrintConsoleA("[VM vCPU %u] Control Register Initialization Status: 0x%X\n",VpIndex,st);
		st=NoirEditVirtualProcessorRegister(PvCvm,VpIndex,NoirCvmDr67Register,Dr67,sizeof(Dr67));
		PvPrintConsoleA("[VM vCPU %u] The dr6/dr7 Registers Initialization Status: 0x%X\n",VpIndex,st);
		st=NoirEditVirtualProcessorRegister(PvCvm,VpIndex,NoirCvmSegmentRegister,&SrState,sizeof(SrState));
		PvPrintConsoleA("[VM vCPU %u] Segment Register Initialization Status: 0x%X\n",VpIndex,st);
		st=NoirEditVirtualProcessorRegister(PvCvm,VpIndex,NoirCvmFsGsRegister,FgSeg,sizeof(FgSeg));
		PvPrintConsoleA("[VM vCPU %u] The fs/gs Registers Initialization Status: 0x%X\n",VpIndex,st);
		st=NoirEditVirtualProcessorRegister(PvCvm,VpIndex,NoirCvmDescriptorTable,DtSeg,sizeof(DtSeg));
		PvPrintConsoleA("[VM vCPU %u] Descriptor Tables Initialization Status: 0x%X\n",VpIndex,st);
		st=NoirEditVirtualProcessorRegister(PvCvm,VpIndex,NoirCvmTrLdtrRegister,LtSeg,sizeof(LtSeg));
		PvPrintConsoleA("[VM vCPU %u] The tr/ldtr Registers Initialization Status: 0x%X\n",VpIndex,st);
		st=NoirEditVirtualProcessorRegister(PvCvm,VpIndex,NoirCvmFxState,&FxState,sizeof(FxState));
		PvPrintConsoleA("[VM vCPU %u] The x87 FPU State Initialization Status: 0x%X\n",VpIndex,st);
		st=NoirEditVirtualProcessorRegister(PvCvm,VpIndex,NoirCvmXcr0Register,&Xcr0,sizeof(Xcr0));
		PvPrintConsoleA("[VM vCPU %u] Extended CR0 Initialization Status: 0x%X\n",VpIndex,st);
		st=NoirEditVirtualProcessorRegister(PvCvm,VpIndex,NoirCvmEferRegister,&Efer,sizeof(Efer));
		PvPrintConsoleA("[VM vCPU %u] Efer Register Initialization Status: 0x%X\n",VpIndex,st);
		st=NoirEditVirtualProcessorRegister(PvCvm,VpIndex,NoirCvmPatRegister,&Pat,sizeof(Pat));
		PvPrintConsoleA("[VM vCPU %u] PAT Register Initialization Status: 0x%X\n",VpIndex,st);
		// Initialize vCPU Options
		VpOpt.InterceptExceptions=1;
		st=NoirSetVirtualProcessorOptions(PvCvm,0,NoirCvmGuestVpOptions,VpOpt.Value);
		PvPrintConsoleA("[VM vCPU %u] Set-Option Status: 0x%X\n",VpIndex,st);
		st=NoirSetVirtualProcessorOptions(PvCvm,0,NoirCvmExceptionBitmap,0xFFFFFFFF);
		PvPrintConsoleA("[VM vCPU %u] Exception Bitmap Set Status: 0x%X\n",VpIndex,st);
	}
	return st;
}

ULONG64 PvTranslateGpaToHva(IN ULONG64 Gpa)
{
	if(Gpa>=PvCriticalRangeBase && Gpa<PvPagingStructuresBase)
		return (ULONG64)PvCriticalRange+(Gpa-PvCriticalRangeBase);
	else if(Gpa>=PvPagingStructuresBase && Gpa<PvBasicFreeMemoryBase)
		return (ULONG64)PvPagingStructures+(Gpa-PvPagingStructuresBase);
	else if(Gpa>=PvBasicFreeMemoryBase && Gpa<PvExtendedFreeMemoryBase)
		return (ULONG64)PvBasicFreeMemory+(Gpa-PvBasicFreeMemoryBase);
	else
	{
		ULONG64 ExtensionIndex=(Gpa>>28)-1;
		// Beware of overflowing.
		return ExtensionIndex<PvAllocatedMemoryExtensions?(ULONG64)PvMemoryExtensionPointerTable[ExtensionIndex]+(Gpa&0xFFFFFFF):0;
	}
}

// Emulate Page-Table Walking of the Guest for Long Mode Memory Access.
ULONG32 PvTranslateGvaToGpa64(IN ULONG64 GuestCr3,IN ULONG64 Gva,IN ULONG64 Access,OUT PULONG64 Gpa)
{
	AMD64_ADDRESS48 Addr;
	PAMD64_PML4E Pml4eBase=(PAMD64_PML4E)PvTranslateGpaToHva(PAGE_BASE(GuestCr3));
	Addr.Value=Gva;
	// Stage I: Walk the Page Map Level 4.
	if((Pml4eBase[Addr.L4.Pml4eIndex].Value & Access)==Access)
	{
		// Stage II: Walk the Page Directory Pointer Table.
		PAMD64_PDPTE PdpteBase=(PAMD64_PDPTE)PvTranslateGpaToHva(Pml4eBase[Addr.L4.Pml4eIndex].PdpteBase<<PAGE_SHIFT);
		if((PdpteBase[Addr.L4.PdpteIndex].Value & Access)==Access)
		{
			// Check whether it is 1GiB Huge Page.
			if(PdpteBase[Addr.L4.PdpteIndex].NonHuge.HugePage)
			{
				// Reference of 1GiB Huge Page.
				*Gpa=PdpteBase[Addr.L2.PdpteIndex].Huge.PageBase<<PAGE_HUGE_SHIFT;
				*Gpa+=Addr.L2.PageOffset;
				return 0;
			}
			else
			{
				// Reference of Underlying Paging Structures.
				// Stage III: Walk the Page Directory.
				PAMD64_PDE PdeBase=(PAMD64_PDE)PvTranslateGpaToHva(PdpteBase[Addr.L4.PdpteIndex].NonHuge.PdeBase<<PAGE_SHIFT);
				if((PdeBase[Addr.L4.PdeIndex].Value & Access)==Access)
				{
					// Check if whether it is 2MiB Large Page.
					if(PdeBase[Addr.L4.PdeIndex].NonLarge.LargePage)
					{
						// Reference of 2MiB Large Page.
						*Gpa=PdeBase[Addr.L3.PdeIndex].Large.PageBase<<PAGE_LARGE_SHIFT;
						*Gpa+=Addr.L3.PageOffset;
						return 0;
					}
					else
					{
						// Reference of Underlying Paging Structures.
						// Stage IV: Walk the Page Table.
						PAMD64_PTE PteBase=(PAMD64_PTE)PvTranslateGpaToHva(PdeBase[Addr.L4.PdeIndex].NonLarge.PteBase<<PAGE_SHIFT);
						if((PteBase[Addr.L4.PteIndex].Value & Access)==Access)
						{
							*Gpa=PteBase[Addr.L4.PteIndex].PageBase<<PAGE_SHIFT;
							*Gpa+=Addr.L4.PageOffset;
							return 0;
						}
					}
				}
			}
		}
	}
	// FIXME: Return appropriate error code.
	return 1;
}

ULONG PvReadGuestMemory(IN ULONG64 GuestCr3,IN ULONG64 GuestAddress,OUT PVOID Buffer,IN ULONG Size,IN BOOLEAN VirtualAddress,IN ULONG64 Access)
{
	ULONG64 Gpa;
	ULONG64 Hva;
	// Do different treatments for guest memory accesses.
	if(VirtualAddress)
	{
		// Access by virtual address.
		// FIXME: Optimize by recognizing large/huge pages.
		ULONG64 PageBase=PAGE_BASE(GuestAddress);
		ULONG32 CopyOffset=PAGE_OFFSET(GuestAddress);
		ULONG32 CopyLength=(PAGE_SIZE-CopyOffset)<Size?PAGE_SIZE-CopyOffset:Size;
		for(ULONG CopiedSize=0;CopiedSize<Size;CopiedSize+=CopyLength)
		{
			// Translate the Guest Page.
			ULONG32 ErrCode=PvTranslateGvaToGpa64(GuestCr3,PageBase+CopyOffset,Access,&Gpa);
			if(ErrCode)return ErrCode;
			Hva=PvTranslateGpaToHva(Gpa);
			if(Hva==0)return 1;
			RtlMoveMemory((PVOID)((ULONG_PTR)Buffer+CopiedSize),(PVOID)Hva,CopyLength);
			// Prepare for next iteration.
			PageBase+=PAGE_SIZE;
			CopyOffset=0;
			CopyLength=(Size-CopiedSize)<PAGE_SIZE?Size-CopiedSize:PAGE_SIZE;
		}
		return 0;
	}
	else
	{
		// Access by physical address.
		// Determine the range. Recurse the call if there is overlapping.
		if(GuestAddress>=PvCriticalRangeBase && GuestAddress<PvPagingStructuresBase)
		{
			ULONG CopyLength=(PvPagingStructuresBase-GuestAddress)<Size?(ULONG)(PvPagingStructuresBase-GuestAddress):Size;
			ULONG RemainderLength=Size-CopyLength;
			RtlMoveMemory(Buffer,(PVOID)((ULONG_PTR)PvCriticalRange+GuestAddress),CopyLength);
			if(RemainderLength)return PvReadGuestMemory(0,PvPagingStructuresBase,(PVOID)((ULONG_PTR)Buffer+CopyLength),RemainderLength,FALSE,0);
		}
		else if(GuestAddress>=PvPagingStructuresBase && GuestAddress<PvBasicFreeMemoryBase)
		{
			ULONG CopyLength=(PvBasicFreeMemoryBase-GuestAddress)<Size?(ULONG)(PvBasicFreeMemoryBase-GuestAddress):Size;
			ULONG RemainderLength=Size-CopyLength;
			RtlMoveMemory(Buffer,(PVOID)((ULONG_PTR)PvPagingStructures+GuestAddress-PvPagingStructuresBase),CopyLength);
			if(RemainderLength)return PvReadGuestMemory(0,PvBasicFreeMemoryBase,(PVOID)((ULONG_PTR)Buffer+CopyLength),RemainderLength,FALSE,0);
		}
		else if(GuestAddress>=PvBasicFreeMemoryBase && GuestAddress<PvExtendedFreeMemoryBase)
		{
			ULONG CopyLength=(PvExtendedFreeMemoryBase-GuestAddress)<Size?(ULONG)(PvExtendedFreeMemoryBase-GuestAddress):Size;
			ULONG RemainderLength=Size-CopyLength;
			RtlMoveMemory(Buffer,(PVOID)((ULONG_PTR)PvBasicFreeMemory+(GuestAddress-PvBasicFreeMemoryBase)),CopyLength);
			if(RemainderLength)return PvReadGuestMemory(0,PvExtendedFreeMemoryBase,(PVOID)((ULONG_PTR)Buffer+CopyLength),RemainderLength,FALSE,0);
		}
		else
		{
			ULONG64 ExtensionIndex=(GuestAddress>>28)-1;
			ULONG64 ExtensionBase=ExtensionIndex<<28;
			ULONG CopyLength=(ExtensionBase+PvExtendedFreeMemorySize-GuestAddress)<Size?(ULONG)(ExtensionBase+PvExtendedFreeMemorySize-GuestAddress):Size;
			ULONG RemainderLength=Size-CopyLength;
			RtlMoveMemory(Buffer,(PVOID)((ULONG_PTR)PvMemoryExtensionPointerTable[ExtensionIndex]+(GuestAddress-ExtensionBase)),CopyLength);
			if(RemainderLength)return PvReadGuestMemory(0,ExtensionBase+PvExtendedFreeMemorySize,(PVOID)((ULONG_PTR)Buffer+CopyLength),RemainderLength,FALSE,0);
		}
		return 0;
	}
	return 0xFFFFFFFF;
}

NOIR_STATUS PvPrintGeneralPurposeRegisters(IN ULONG32 VpIndex)
{
	ULONG64 GprState[16];
	NOIR_STATUS st=NoirViewVirtualProcessorRegister(PvCvm,VpIndex,NoirCvmGeneralPurposeRegister,&GprState,sizeof(GprState));
	if(st==NOIR_SUCCESS)
	{
		PSTR GprNames[16]={"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};
		PvPrintConsoleA("Register\tValue\n");
		for(ULONG i=0;i<16;i++)PvPrintConsoleA("%s\t\t0x%016llX\n",GprNames[i],GprState[i]);
	}
	return st;
}

BOOLEAN PvHandleIoAccess(IN ULONG32 VpIndex,IN PNOIR_CVM_EXIT_CONTEXT ExitContext)
{
	BOOLEAN Result=FALSE;
	switch(ExitContext->Io.Port)
	{
		case ConsoleInputPort:
		{
			if(ExitContext->Io.Access.IoType)
			{
				if(ExitContext->Io.Access.String)
				{
					NOIR_CR_STATE CrState;
					NOIR_STATUS st=NoirViewVirtualProcessorRegister(PvCvm,0,NoirCvmControlRegister,&CrState,sizeof(CrState));
					if(st==NOIR_SUCCESS)
					{
						NOIR_GPR_STATE GprState;
						// Output a character/string by memory reference.
						USHORT Increment=ExitContext->Io.Access.OperandSize;
						ULONG64 StringGva=0;
						NoirViewVirtualProcessorRegister(PvCvm,0,NoirCvmGeneralPurposeRegister,&GprState,sizeof(GprState));
						Increment*=_bittest64(&ExitContext->Rflags,RFLAGS_DF)?-1:1;
						RtlMoveMemory(&StringGva,&ExitContext->Io.Rdi,ExitContext->Io.Access.AddressWidth);
						Result=TRUE;
						do
						{
							// Resolve Address Translation.
							ULONG64 StringGpa;
							ULONG ErrorCode=PvTranslateGvaToGpa64(CrState.Cr3,StringGva,0,&StringGpa);
							if(ErrorCode==0)
							{
								ULONG64 StringHva=PvTranslateGpaToHva(StringGpa);
								if(StringHva==0)
									Result=FALSE;
								else
									RtlMoveMemory((PVOID)StringHva,&DummyBuffer[DummyPointer],ExitContext->Io.Access.OperandSize);
							}
							else
							{
								Result=FALSE;
								break;
							}
							// Perform Increment.
							if(ExitContext->Io.Access.Repeat)ExitContext->Io.Rcx--;
							StringGva+=Increment;
							DummyPointer+=Increment;
						}while(ExitContext->Io.Access.Repeat && ExitContext->Io.Rcx);
						GprState.Rdi=StringGva;
						GprState.Rcx=ExitContext->Io.Rcx;
						NoirEditVirtualProcessorRegister(PvCvm,0,NoirCvmGeneralPurposeRegister,&GprState,sizeof(GprState));
					}
				}
				else
				{
					// Input a character by register reference.
					RtlMoveMemory(&ExitContext->Io.Rax,&DummyBuffer[DummyPointer],ExitContext->Io.Access.OperandSize);
					DummyPointer+=ExitContext->Io.Access.OperandSize;
				}
			}
		}
		case ConsoleOutputPort:
		{
			if(ExitContext->Io.Access.IoType==0)
			{
				if(ExitContext->Io.Access.String)
				{
					NOIR_CR_STATE CrState;
					NOIR_STATUS st=NoirViewVirtualProcessorRegister(PvCvm,0,NoirCvmControlRegister,&CrState,sizeof(CrState));
					if(st==NOIR_SUCCESS)
					{
						PSTR StringPool=MemAlloc(ExitContext->Io.Rcx);
						if(StringPool)
						{
							NOIR_GPR_STATE GprState;
							// Output a character/string by memory reference.
							USHORT Increment=ExitContext->Io.Access.OperandSize;
							ULONG64 StringGva=0;
							ULONG64 Index=0;
							NoirViewVirtualProcessorRegister(PvCvm,0,NoirCvmGeneralPurposeRegister,&GprState,sizeof(GprState));
							Increment*=_bittest64(&ExitContext->Rflags,RFLAGS_DF)?-1:1;
							RtlMoveMemory(&StringGva,&ExitContext->Io.Rsi,ExitContext->Io.Access.AddressWidth);
							Result=TRUE;
							do
							{
								// Resolve Address Translation.
								ULONG64 StringGpa;
								ULONG ErrorCode=PvTranslateGvaToGpa64(CrState.Cr3,StringGva,1,&StringGpa);
								if(ErrorCode==0)
								{
									ULONG64 StringHva=PvTranslateGpaToHva(StringGpa);
									if(StringHva==0)
										Result=FALSE;
									else
										RtlMoveMemory(&StringPool[Index],(PVOID)StringHva,ExitContext->Io.Access.OperandSize);
								}
								else
								{
									Result=FALSE;
									break;
								}
								// Perform Increment.
								if(ExitContext->Io.Access.Repeat)ExitContext->Io.Rcx--;
								StringGva+=Increment;
								Index+=Increment;
							}while(ExitContext->Io.Access.Repeat && ExitContext->Io.Rcx);
							GprState.Rsi=StringGva;
							GprState.Rcx=ExitContext->Io.Rcx;
							NoirEditVirtualProcessorRegister(PvCvm,0,NoirCvmGeneralPurposeRegister,&GprState,sizeof(GprState));
							PvWriteGuestStdOut(StringPool,(ULONG)(Index-1));
							MemFree(StringPool);
						}
					}
				}
				else
				{
					// Output a character by register reference.
					BYTE Character[8]={0};
					RtlMoveMemory(Character,&ExitContext->Io.Rax,ExitContext->Io.Access.OperandSize);
					PvPrintConsoleA(Character);
					Result=TRUE;
				}
			}
			break;
		}
		case ConsoleErrorPort:
		{
			break;
		}
		default:
		{
			break;
		}
	}
	return Result;
}

BOOLEAN PvHandleHypercall(IN ULONG32 VpIndex,IN PNOIR_CVM_EXIT_CONTEXT ExitContext)
{
	BOOLEAN Result=FALSE;
	NOIR_GPR_STATE GprState;
	NOIR_STATUS st=NoirViewVirtualProcessorRegister(PvCvm,0,NoirCvmGeneralPurposeRegister,&GprState,sizeof(GprState));
	if(st==NOIR_SUCCESS)
	{
		switch(GprState.Rcx)
		{
			case NOIR_HYPERCALL_CODE_SHUTDOWN:
			{
				PvPrintConsoleA("Shutdown Hypercall is invoked! Guest will now shut down!\n");
				break;
			}
			case NOIR_HYPERCALL_MEMORY_EXTENSION:
			{
				PvPrintConsoleA("Memory Extension is requested!\n");
				break;
			}
			case NOIR_HYPERCALL_FILE_CREATE:
			{
				CHAR FilePath[MAX_PATH];
				LARGE_INTEGER Option1,Option2;
				Option1.QuadPart=GprState.R8;
				Option2.QuadPart=GprState.R9;
				// Resolve GVA here.
				GprState.Rax=(ULONG64)CreateFileA(FilePath,Option1.LowPart,Option1.HighPart,NULL,Option2.LowPart,Option2.HighPart,NULL);
				Result=TRUE;
				break;
			}
			case NOIR_HYPERCALL_FILE_CLOSE:
			{
				GprState.Rax=(ULONG64)CloseHandle((HANDLE)GprState.Rcx);
				Result=TRUE;
				break;
			}
			case NOIR_HYPERCALL_FILE_READ:
			{
				Result=TRUE;
				break;
			}
			case NOIR_HYPERCALL_FILE_WRITE:
			{
				Result=TRUE;
				break;
			}
			case NOIR_HYPERCALL_FILE_SET_POINTER:
			{
				PLARGE_INTEGER NewPointer=(PLARGE_INTEGER)&GprState.R8;
				GprState.Rax=(ULONG64)SetFilePointer((HANDLE)GprState.Rdx,NewPointer->LowPart,&NewPointer->HighPart,(DWORD)GprState.R9);
				Result=TRUE;
				break;
			}
		}
	}
	return Result;
}

NOIR_STATUS PvStartParavirtualizedGuest()
{
	NOIR_STATUS st=NoirEditVirtualProcessorRegister(PvCvm,0,NoirCvmInstructionPointer,&PvParavirtualizedKernelEntryPoint,sizeof(PvParavirtualizedKernelEntryPoint));
	if(st==NOIR_SUCCESS)
	{
		NOIR_GPR_STATE GprState={0};
		OVERLAPPED StdInOverlapped={0};
		BOOLEAN ContinueExecution=TRUE;
		StdInOverlapped.hEvent=PvStdInEvent;
		GprState.Rsp=(ULONG64)PvBootingModuleGva+512*PAGE_SIZE-0x20;
		GprState.Rcx=GprState.Rsp-8*PAGE_SIZE+0x20;
		*(PULONG32)((ULONG_PTR)PvBasicFreeMemory+504*PAGE_SIZE)=0xC3D9010F;
		st=NoirEditVirtualProcessorRegister(PvCvm,0,NoirCvmGeneralPurposeRegister,&GprState,sizeof(GprState));
		PvPrintConsoleA("General-Purpose Register is initialized successfully!\n");
		while(ContinueExecution)
		{
			NOIR_CVM_EXIT_CONTEXT ExitContext;
			st=NoirRunVirtualProcessor(PvCvm,0,&ExitContext);
			if(st!=NOIR_SUCCESS)
			{
				PvPrintConsoleA("Failed to run virtual processor! Status=0x%X\n",st);
				break;
			}
			switch(ExitContext.InterceptCode)
			{
				case CvInvalidState:
				{
					PvPrintConsoleA("Fatal: The guest vCPU state is invalid!\n");
					ContinueExecution=FALSE;
					break;
				}
				case CvShutdownCondition:
				{
					ULONG_PTR IdtBase=(ULONG_PTR)PvCriticalRange;
					PvPrintConsoleA("Fatal: Triple-Fault is intercepted! Guest VM will be terminated!\n");
					ContinueExecution=FALSE;
					break;
				}
				case CvHltInstruction:
				{
					if(_bittest64(&ExitContext.Rflags,RFLAGS_IF))
					{
						ULONG64 NextRip=ExitContext.Rip+ExitContext.VpState.InstructionLength;
						// Wait for external hardwares to send interrupts.
						PvReadGuestStdInAsync(DummyBuffer,sizeof(DummyBuffer),&StdInOverlapped);
						ULONG WaitResult=WaitForMultipleObjects(3,PvHardwareEventGroup,FALSE,INFINITE);
						PvPrintConsoleA("vCPU is halted, waiting for external events...\n");
						switch(WaitResult)
						{
							case WAIT_OBJECT_0:
							{
								// There is something in stdin.
								NoirSetEventInjection(PvCvm,0,TRUE,NOIR_DUMMY_INTERRUPT_VECTOR,NoirEventTypeExternalInterrupt,1,FALSE,0);
								// StdIn Event requires manual resetting.
								ResetEvent(PvStdInEvent);
								break;
							}
							case WAIT_OBJECT_0+1:
							{
								// Timer interrupt is to be injected.
								NoirSetEventInjection(PvCvm,0,TRUE,NOIR_TIMER_INTERRUPT_VECTOR,NoirEventTypeExternalInterrupt,1,FALSE,0);
								break;
							}
							case WAIT_OBJECT_0+2:
							{
								// File I/O interrupt is to be injected.
								// File I/O Event requires manual resetting.
								ResetEvent(PvFileEvent);
								break;
							}
							default:
							{
								PvPrintConsoleA("Fatal: Unknown event wait result: 0x%X!\n",WaitResult);
								ContinueExecution=FALSE;
								break;
							}
						}
						NoirEditVirtualProcessorRegister(PvCvm,0,NoirCvmInstructionPointer,&NextRip,sizeof(NextRip));
					}
					else
					{
						PvPrintConsoleA("Fatal: Processor entered halted state without enabling interrupts!\n");
						ContinueExecution=FALSE;
					}
					break;
				}
				case CvIoInstruction:
				{
					if(PvHandleIoAccess(0,&ExitContext))
					{
						ULONG64 NextRip=ExitContext.Rip+ExitContext.VpState.InstructionLength;
						NoirEditVirtualProcessorRegister(PvCvm,0,NoirCvmInstructionPointer,&NextRip,sizeof(NextRip));
					}
					else
					{
						PvPrintConsoleA("Fatal: Failed to translate GVA in I/O Handler! Port=0x%X\n",ExitContext.Io.Port);
						ContinueExecution=FALSE;
					}
					break;
				}
				case CvHypercall:
				{
					if(PvHandleHypercall(0,&ExitContext))
					{
						ULONG64 NextRip=ExitContext.Rip+ExitContext.VpState.InstructionLength;
						NoirEditVirtualProcessorRegister(PvCvm,0,NoirCvmInstructionPointer,&NextRip,sizeof(NextRip));
					}
					else
					{
						PvPrintConsoleA("Hypercall specified an end-of-execution!\n");
						ContinueExecution=FALSE;
					}
					break;
				}
				case CvException:
				{
					CHAR Cmd[100];
					PvPrintConsoleA("Exception Vector %u is intercepted!\n",ExitContext.Exception.Vector);
					PvPrintConsoleA("Error Code is valid: 0x%X\n",ExitContext.Exception.ErrorCode);
					if(ExitContext.Exception.Vector==14)PvPrintConsoleA("#PF Linear Address: 0x%p\n",ExitContext.Exception.PageFaultAddress);
					PvPrintGeneralPurposeRegisters(0);
					PvPrintConsoleA("rflags\t\t0x%016llX\n",ExitContext.Rflags);
					PvPrintConsoleA("rip\t\t0x%016llX\n",ExitContext.Rip);
					StringCbGetsA(Cmd,sizeof(Cmd));
					ContinueExecution=FALSE;
					break;
				}
				default:
				{
					PvPrintConsoleA("Unknown interception occured! Code=%u (0x%X)\n",ExitContext.InterceptCode,ExitContext.InterceptCode);
					break;
				}
			}
		}
	}
	return st;
}

BOOL PvLoadParavirtualizedKernel()
{
	BOOL bRet=FALSE;
	HANDLE hFile=CreateFileW(L"pv_kernel.exe",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		IMAGE_DOS_HEADER DosHead;
		DWORD dwRead;
		SetFilePointer(hFile,0,NULL,FILE_BEGIN);
		// Load DOS header.
		ReadFile(hFile,&DosHead,sizeof(DosHead),&dwRead,NULL);
		if(DosHead.e_magic==IMAGE_DOS_SIGNATURE)
		{
			IMAGE_NT_HEADERS64 NtHead;
			SetFilePointer(hFile,DosHead.e_lfanew,NULL,FILE_BEGIN);
			// Load NT header.
			ReadFile(hFile,&NtHead,sizeof(NtHead),&dwRead,NULL);
			if(NtHead.Signature==IMAGE_NT_SIGNATURE && NtHead.FileHeader.Machine==IMAGE_FILE_MACHINE_AMD64)
			{
				PIMAGE_SECTION_HEADER SectionHeaders=(PIMAGE_SECTION_HEADER)((ULONG_PTR)PvBasicFreeMemory+DosHead.e_lfanew+sizeof(NtHead));
				PvPrintConsoleA("Size of the Image: %u bytes\n",NtHead.OptionalHeader.SizeOfImage);
				if(NtHead.OptionalHeader.SizeOfImage>=504*PAGE_SIZE)PvPrintConsoleA("Image is too big!\n");
				// Move the loaded headers to guest memory.
				RtlMoveMemory(PvBasicFreeMemory,&DosHead,sizeof(DosHead));
				RtlMoveMemory((PVOID)((ULONG_PTR)PvBasicFreeMemory+DosHead.e_lfanew),&NtHead,sizeof(NtHead));
				// Load Section headers.
				ReadFile(hFile,SectionHeaders,NtHead.FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER),&dwRead,NULL);
				for(USHORT i=0;i<NtHead.FileHeader.NumberOfSections;i++)
				{
					PVOID SectionBase=(PVOID)((ULONG_PTR)PvBasicFreeMemory+SectionHeaders[i].VirtualAddress);
					SetFilePointer(hFile,SectionHeaders[i].PointerToRawData,NULL,FILE_BEGIN);
					// Load section into guest memory.
					ReadFile(hFile,SectionBase,SectionHeaders[i].SizeOfRawData,&dwRead,NULL);
				}
				// Perform relocations.
				if(NtHead.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress)
				{
					PIMAGE_BASE_RELOCATION RelocBlock=(PIMAGE_BASE_RELOCATION)((ULONG_PTR)PvBasicFreeMemory+NtHead.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
					do
					{
						ULONG NumberOfRelocations=(RelocBlock->SizeOfBlock-sizeof(IMAGE_BASE_RELOCATION))>>1;
						PUSHORT RelocData=(PUSHORT)((ULONG_PTR)RelocBlock+sizeof(IMAGE_BASE_RELOCATION));
						PvPrintConsoleA("Relocating Page 0x%p...\n",PvBootingModuleGva+RelocBlock->VirtualAddress);
						for(ULONG i=0;i<NumberOfRelocations;i++)
						{
							if((RelocData[i]>>12)==IMAGE_REL_BASED_DIR64)
							{
								USHORT MiniOffset=RelocData[i]&0xFFF;
								PULONG64 RelocAddress=(PULONG64)((ULONG_PTR)PvBasicFreeMemory+RelocBlock->VirtualAddress+MiniOffset);
								*RelocAddress+=PvBootingModuleGva-NtHead.OptionalHeader.ImageBase;
							}
						}
						RelocBlock=(PIMAGE_BASE_RELOCATION)((ULONG_PTR)RelocBlock+RelocBlock->SizeOfBlock);
					}while(RelocBlock->VirtualAddress);
				}
				PvParavirtualizedKernelEntryPoint+=NtHead.OptionalHeader.AddressOfEntryPoint;
				// Finally, load debug image of the Guest kernel booting module.
				if(PvdLoadSymbolForGuestModule(PvBootingModuleGva,0,hFile,"pv_kernel.exe")==FALSE)
					PvPrintConsoleA("Failed to load symbols for Guest Kernel Booting Module!\n");
				bRet=TRUE;
			}
			else
			{
				if(NtHead.Signature!=IMAGE_NT_SIGNATURE)PvPrintConsoleA("NT Signature of PV-Kernel image mismatch!\n");
				if(NtHead.FileHeader.Machine!=IMAGE_FILE_MACHINE_AMD64)PvPrintConsoleA("PV-Kernel is not an AMD64-based image!\n");
			}
		}
		else
			PvPrintConsoleA("DOS Signature of PV-Kernel Image mismatch!\n");
		CloseHandle(hFile);
	}
	return bRet;
}

NOIR_STATUS PvInitializeVirtualMachine(IN ULONG32 VpCount)
{
	// Create the virtual machine
	NOIR_STATUS st=NoirCreateVirtualMachine(&PvCvm);
	if(st==NOIR_SUCCESS)
	{
		// Allocate Resident Memory for Guest Physical Memories.
		st=NOIR_INSUFFICIENT_RESOURCES;
		PvCriticalRange=PageAlloc(PvCriticalRangeSize);
		PvPagingStructures=PageAlloc(PvPagingStructuresSize);
		PvBasicFreeMemory=PageAlloc(PvBasicFreeMemorySize);
		PvMemoryExtensionPointerTable=MemAlloc(sizeof(PVOID)*PvLimitOfMemoryExtensions);
		if(PvCriticalRange && PvPagingStructures && PvBasicFreeMemory && PvMemoryExtensionPointerTable)
		{
			BOOL b1=LockPage(PvCriticalRange,PvCriticalRangeSize);
			BOOL b2=LockPage(PvPagingStructures,PvPagingStructuresSize);
			BOOL b3=LockPage(PvBasicFreeMemory,PvBasicFreeMemorySize);
			if(b1 && b2 && b3)
			{
				NOIR_ADDRESS_MAPPING MapInfo;
				// Initialize Attributes....
				MapInfo.Attributes.Present=1;
				MapInfo.Attributes.Write=1;
				MapInfo.Attributes.Execute=1;
				MapInfo.Attributes.User=1;
				MapInfo.Attributes.Caching=NoirMemoryTypeWriteBack;
				MapInfo.Attributes.PageSize=0;
				MapInfo.Attributes.Reserved=0;
				// Map the critical range.
				MapInfo.GPA=PvCriticalRangeBase;
				MapInfo.HVA=(ULONG64)PvCriticalRange;
				MapInfo.NumberOfPages=PvCriticalRangePages;
				st=NoirSetAddressMapping(PvCvm,&MapInfo);
				PvPrintConsoleA("Critical Range Mapping Status=0x%X\n",st);
				// Map the paging structures.
				MapInfo.GPA=PvPagingStructuresBase;
				MapInfo.HVA=(ULONG64)PvPagingStructures;
				st=NoirSetAddressMapping(PvCvm,&MapInfo);
				PvPrintConsoleA("Guest Internal Paging Structure Mapping Status=0x%X\n",st);
				// Map the basic free memory.
				MapInfo.GPA=PvBasicFreeMemoryBase;
				MapInfo.HVA=(ULONG64)PvBasicFreeMemory;
				st=NoirSetAddressMapping(PvCvm,&MapInfo);
				PvPrintConsoleA("Basic Free Memory Paging Structure Mapping Status=0x%X\n",st);
				// Initialize vCPUs.
				for(ULONG32 i=0;i<VpCount;i++)
				{
					st=PvInitializeVirtualProcessor(i);
					PvPrintConsoleA("vCPU %u Initialization Status: 0x%X\n",i,st);
				}
				// Initialize Virtual Hardware...
				PvInitializeTimerHardware(233);
				PvInitializeFileIoHardware();
				PvHardwareEventGroup[0]=PvStdInEvent;
				PvHardwareEventGroup[1]=PvTimerEvent;
				PvHardwareEventGroup[2]=PvFileEvent;
			}
			else
			{
				if(!b1)PvPrintConsoleA("Failed to lock Critical Range!\n");
				if(!b2)PvPrintConsoleA("Failed to lock Guest Paging Structure Range!\n");
				if(!b3)PvPrintConsoleA("Failed to lock Basic Free Memory!\n");
			}
		}
	}
	return st;
}