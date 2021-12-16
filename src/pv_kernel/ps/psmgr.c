#include <pvdef.h>
#include <pvbdk.h>
#include <pv_fileio.h>
#include <ps.h>
#include <mm.h>
#include <page.h>
#include <peimage.h>

BOOL static PsInsertVadTree(IN PKVAD VadNode,IN PKVAD NewVad)
{
	// FIXME: Implement AVL Binary-Search Tree.
	if(NewVad->StartingPfn>VadNode->EndingPfn)
	{
		// It is to be inserted to the right.
		if(VadNode->RightChild)
			return PsInsertVadTree(VadNode->RightChild,NewVad);
		else
			VadNode->RightChild=NewVad;
		return TRUE;
	}
	else if(NewVad->EndingPfn<VadNode->StartingPfn)
	{
		// It is to be inserted to the left.
		if(VadNode->LeftChild)
			return PsInsertVadTree(VadNode->LeftChild,NewVad);
		else
			VadNode->LeftChild=NewVad;
		return TRUE;
	}
	// There are overlapping pages. Fail the insertion.
	return FALSE;
}

BOOL PsInsertVadToProcess(IN PKPROCESS Process,IN PKVAD Vad)
{
	// Insert the Binary-Search Tree.
	if(!PsInsertVadTree(&Process->VadRoot,Vad))return FALSE;
	// If the insertion fails, do not insert the view list.
	// Insert the Doubly-Linked List.
	RtlInsertToListTail(&Process->VadRoot.ViewList,&Vad->ViewList);
	return TRUE;
}

PVOID PsLoadImageToProcess(IN PKPROCESS Process,IN HANDLE ImageFileHandle)
{
	BOOL Result;
	// Load DOS Header.
	IMAGE_DOS_HEADER DosHead;
	PvSetFilePointer(ImageFileHandle,0,FILE_BEGIN);
	Result=PvReadFile(ImageFileHandle,&DosHead,sizeof(DosHead));
	if(Result && DosHead.e_magic==IMAGE_DOS_SIGNATURE)
	{
		// Load NT Header.
		IMAGE_NT_HEADERS64 NtHead;
		PvSetFilePointer(ImageFileHandle,DosHead.e_lfanew,FILE_BEGIN);
		Result=PvReadFile(ImageFileHandle,&NtHead,sizeof(NtHead));
		if(Result && NtHead.Signature==IMAGE_NT_SIGNATURE && NtHead.FileHeader.Machine==IMAGE_FILE_MACHINE_AMD64)
		{
			ULONG SizeInPages=(ULONG)(NtHead.OptionalHeader.SizeOfImage>>PAGE_SHIFT);
			// Describe the image with VAD.
			PKVAD Vad=MmAllocateSystemPool(sizeof(KVAD)+(SizeInPages-1)*sizeof(KPTE_SECTION));
			if(Vad)
			{
				ULONG SizeOfSectionHeaders=sizeof(IMAGE_SECTION_HEADER)*NtHead.FileHeader.NumberOfSections;
				PIMAGE_SECTION_HEADER SectionHeaders=MmAllocateSystemPool(SizeOfSectionHeaders);
				if(SectionHeaders)
				{
					ULONG64 OldCr3=__readcr3();
					// Turn off the Copy-on-Write so we may write freely.
					BOOL WP=MmTurnOffCopyOnWrite();
					// Set the range of VAD.
					Vad->StartingPfn=NtHead.OptionalHeader.ImageBase>>PAGE_SHIFT;
					Vad->EndingPfn=Vad->StartingPfn+SizeInPages-1;
					// Initialize attributes of all pages and allocate physical pages.
					PvReadFile(ImageFileHandle,SectionHeaders,sizeof(IMAGE_SECTION_HEADER)*NtHead.FileHeader.NumberOfSections);
					for(ULONG i=0;i<SizeInPages;i++)
					{
						Vad->SectionPtes[i].Value=0;
						Vad->SectionPtes[i].Read=1;
						Vad->SectionPtes[i].Write=1;
						Vad->SectionPtes[i].PageBase=MmAllocatePhysicalPage()>>PAGE_SHIFT;
					}
					// Initialize attributes per sections.
					for(USHORT i=0;i<NtHead.FileHeader.NumberOfSections;i++)
					{
						// Allocate physical page for it.
						for(ULONG j=SectionHeaders[i].VirtualAddress;j<SectionHeaders[i].VirtualAddress+SectionHeaders[i].SizeOfRawData;j+=PAGE_SIZE)
						{
							PKPTE_SECTION Pte=&Vad->SectionPtes[j>>PAGE_SHIFT];
							Pte->Read=(SectionHeaders[i].Characteristics&IMAGE_SCN_MEM_READ)!=0;
							Pte->Write=(SectionHeaders[i].Characteristics&IMAGE_SCN_MEM_WRITE)!=0;
							Pte->Execute=(SectionHeaders[i].Characteristics&IMAGE_SCN_MEM_EXECUTE)!=0;
							Pte->Swappable=(SectionHeaders[i].Characteristics&IMAGE_SCN_MEM_NOT_PAGED)==0;
							Pte->Shared=(SectionHeaders[i].Characteristics&IMAGE_SCN_MEM_SHARED)!=0;
							Pte->Uncacheable=(SectionHeaders[i].Characteristics&IMAGE_SCN_MEM_NOT_CACHED)==0;
						}
					}
					// Insert the VAD.
					PsInsertVadToProcess(Process,Vad);
					// Map all previously described pages.
					for(ULONG i=0;i<SizeInPages;i++)
					{
						PKPTE_SECTION Pte=&Vad->SectionPtes[i];
						KPAGE_RIGHTS PageRights;
						PageRights.Value=0;
						PageRights.Present=(ULONG32)Pte->Read;
						PageRights.Write=(ULONG32)Pte->Write;
						PageRights.Execute=(ULONG32)Pte->Execute;
						PageRights.User=TRUE;
						PageRights.Caching=Pte->Uncacheable?PAGE_CACHING_UC:PAGE_CACHING_WB;
						MmMapVirtualPage(Process->PagingBase,(PVOID)(NtHead.OptionalHeader.ImageBase+(i<<PAGE_SHIFT)),Pte->PageBase<<PAGE_SHIFT,PageRights);
					}
					// Attach to the new process.
					__writecr3(Process->PagingBase);
					// Copy Headers.
					RtlCopyMemory((PVOID)NtHead.OptionalHeader.ImageBase,&DosHead,sizeof(DosHead));
					RtlCopyMemory((PVOID)(NtHead.OptionalHeader.ImageBase+DosHead.e_lfanew),&NtHead,sizeof(NtHead));
					RtlCopyMemory((PVOID)(NtHead.OptionalHeader.ImageBase+DosHead.e_lfanew+NtHead.FileHeader.SizeOfOptionalHeader),SectionHeaders,SizeOfSectionHeaders);
					// Read sections into memory.
					for(USHORT i=0;i<NtHead.FileHeader.NumberOfSections;i++)
					{
						PvSetFilePointer(ImageFileHandle,SectionHeaders[i].PointerToRawData,FILE_BEGIN);
						PvReadFile(ImageFileHandle,(PVOID)(NtHead.OptionalHeader.ImageBase+SectionHeaders[i].VirtualAddress),SectionHeaders[i].SizeOfRawData);
					}
					// Detach from the new process.
					__writecr3(OldCr3);
					// Resume the state of Copy-on-Write.
					if(WP)MmTurnOnCopyOnWrite();
					MmFreeSystemPool(SectionHeaders);
				}
			}
			return (PVOID)NtHead.OptionalHeader.ImageBase;
		}
	}
	return NULL;
}

void PspCreateThread(IN PKPROCESS Process,OUT PKTHREAD *NewThread,IN ULONG64 StackLimit)
{
	PKTHREAD Thread=MmAllocateSystemPool(sizeof(KTHREAD));
	if(Thread)
	{
		PKVAD Vad=MmAllocateSystemPool(sizeof(KVAD)+sizeof(KPTE_SECTION)*(USER_STACK_PAGES-1));
		Thread->Process=Process;
		RtlInsertToListTail(&Process->ThreadListHead,&Thread->ThreadListEntry);
		// Create thread's stack.
		// FIXME: Use dynamic allocation.
		Thread->StackBase=(PVOID)(PvUserKernelSharedRegionUserVirtualBase-USER_STACK_SIZE);
		if(Vad)
		{
			Vad->StartingPfn=(ULONG64)Thread->StackBase>>PAGE_SHIFT;
			Vad->EndingPfn=Vad->StartingPfn+USER_STACK_PAGES-1;
			for(ULONG i=0;i<USER_STACK_PAGES;i++)
			{
				PKPTE_SECTION Pte=&Vad->SectionPtes[i];
				KPAGE_RIGHTS PageRights;
				Pte->Value=0;
				Pte->Read=Pte->Write=Pte->Swappable=TRUE;
				Pte->PageBase=MmAllocatePhysicalPage()>>PAGE_SHIFT;
				PageRights.Value=0;
				PageRights.Present=PageRights.Write=PageRights.User=TRUE;
				MmMapVirtualPage(Process->PagingBase,(PVOID)((ULONG64)Thread->StackBase+(i<<PAGE_SHIFT)),Pte->PageBase<<PAGE_SHIFT,PageRights);
			}
		}
		// Initialize Context...
		Thread->Context.SegCs=KGDT_USER_CODE64;
		Thread->Context.SegDs=KGDT_USER_DATA;
		Thread->Context.SegEs=KGDT_USER_DATA;
		Thread->Context.SegFs=KGDT_USER_TEB32;
		Thread->Context.SegGs=KGDT_USER_TEB64;
		Thread->Context.SegSs=KGDT_USER_DATA;
		Thread->Context.Eflags=0x202;			// Interrupts must never be masked in user mode.
		Thread->Context.Rsp=(ULONG64)Thread->StackBase+USER_STACK_SIZE-0x10;
		Thread->Context.FxState.Fpu.Fcw=0x40;
		Thread->Context.FxState.Fpu.Fsw=0x0;
		Thread->Context.FxState.Fpu.Ftw=0x0;
		Thread->Context.FxState.Fpu.MxCsr=0x1F80;
	}
	*NewThread=Thread;
}

BOOL PspMapUserKernelSharedPage(IN PKPROCESS Process)
{
	// There is a shared page between user and kernel.
	PKVAD Vad=MmAllocateSystemPool(sizeof(KVAD));
	if(Vad)
	{
		KPAGE_RIGHTS PageRights;
		// Initialize Basic VAD Entry.
		Vad->StartingPfn=Vad->EndingPfn=PvUserKernelSharedRegionUserVirtualBase>>PAGE_SHIFT;
		// Initialize Section PTE.
		Vad->SectionPtes->Value=0;
		Vad->SectionPtes->Read=TRUE;
		Vad->SectionPtes->PageBase=PvUserKernelSharedRegionPhysicalBase;
		// Map to Process Region.
		// DO NOT GRANT WRITE AND EXECUTE PERMISSION!
		PageRights.Value=0;
		PageRights.Present=TRUE;
		PageRights.User=TRUE;
		PageRights.Caching=PAGE_CACHING_WB;
		MmMapVirtualPage(Process->PagingBase,(PVOID)PvUserKernelSharedRegionUserVirtualBase,PvUserKernelSharedRegionPhysicalBase,PageRights);
		// Insert to VAD Tree.
		if(PsInsertVadToProcess(Process,Vad))return TRUE;
		MmFreeSystemPool(Vad);
	}
	return FALSE;
}

void PspCreateProcess(OUT PKPROCESS *NewProcess)
{
	PKPROCESS Process=MmAllocateSystemPool(sizeof(KPROCESS));
	if(Process)
	{
		Process->RootPage=MmAllocatePageForPagingStructure();
		Process->PagingBase=MmGetPhysicalAddress(Process->RootPage);
		MmInitializeRootPageForProcess(Process->RootPage);
		RtlInitializeListHead(&Process->VadRoot.ViewList);
		RtlInitializeListHead(&Process->ThreadListHead);
		RtlInsertToListTail(&PvIdleProcess.ActiveListEntry,&Process->ActiveListEntry);
	}
	*NewProcess=Process;
}

void PsCreateProcess(OUT PKPROCESS *NewProcess,IN HANDLE ImageFileHandle)
{
	ULONG64 OldCr3=__readcr3();
	PIMAGE_DOS_HEADER DosHead;
	PKTHREAD InitialThread;
	PspCreateProcess(NewProcess);
	PspMapUserKernelSharedPage(*NewProcess);
	// Load image section.
	(*NewProcess)->SectionBase=PsLoadImageToProcess(*NewProcess,ImageFileHandle);
	// Create initial thread.
	PspCreateThread(*NewProcess,&InitialThread,0);
	// Set the instruction pointer of the initial thread.
	// Attach to the target process.
	__writecr3((*NewProcess)->PagingBase);
	DosHead=(*NewProcess)->SectionBase;
	if(DosHead->e_magic==IMAGE_DOS_SIGNATURE)
	{
		PIMAGE_NT_HEADERS64 NtHead=(PIMAGE_NT_HEADERS64)((ULONG64)DosHead+DosHead->e_lfanew);
		if(NtHead->Signature==IMAGE_NT_SIGNATURE)InitialThread->Context.Rip=NtHead->OptionalHeader.AddressOfEntryPoint;
	}
	// Detach from the target process.
	__writecr3(OldCr3);
}

void PspInitialzeProcessManager()
{
	RtlInitializeListHead(&PvIdleProcess.ActiveListEntry);
}