#include <Windows.h>
#include <dbghelp.h>
#include <strsafe.h>
#include <search.h>
#include <string.h>
#include "debug_agent.h"

int static PvdSymbolSortByNameCallback(const void* a,const void* b)
{
	PSYMBOL_INFO SymInfo1=*(PSYMBOL_INFO*)a;
	PSYMBOL_INFO SymInfo2=*(PSYMBOL_INFO*)b;
	return strncmp(SymInfo1->Name,SymInfo2->Name,SymInfo1->NameLen<SymInfo2->NameLen?SymInfo1->NameLen:SymInfo2->NameLen);
}

int static PvdSymbolSortByAddrCallback(const void* a,const void* b)
{
	PSYMBOL_INFO SymInfo1=*(PSYMBOL_INFO*)a;
	PSYMBOL_INFO SymInfo2=*(PSYMBOL_INFO*)b;
	if(SymInfo1->Address>SymInfo2->Address)
		return 1;
	else if(SymInfo1->Address<SymInfo2->Address)
		return -1;
	return 0;
}

BOOL static PvdSymbolEnumerationPass2Callback(IN PSYMBOL_INFO pSymInfo,IN ULONG SymbolSize,IN PVOID UserContext OPTIONAL)
{
	// In Pass #2, record symbol information.
	PIMAGE_SYMBOL_LIST ImgSym=(PIMAGE_SYMBOL_LIST)UserContext;
	ULONG SymSize=pSymInfo->SizeOfStruct+pSymInfo->NameLen;
	ImgSym->SortedByAddr[ImgSym->AccumulationIndex]=MemAlloc(SymSize);
	ImgSym->SortedByName[ImgSym->AccumulationIndex]=MemAlloc(SymSize);
	if(ImgSym->SortedByAddr[ImgSym->AccumulationIndex] && ImgSym->SortedByName[ImgSym->AccumulationIndex])
	{
		RtlMoveMemory(ImgSym->SortedByAddr[ImgSym->AccumulationIndex],pSymInfo,SymSize);
		RtlMoveMemory(ImgSym->SortedByName[ImgSym->AccumulationIndex],pSymInfo,SymSize);
		ImgSym->AccumulationIndex++;
		return TRUE;
	}
	return FALSE;
}

BOOL static PvdSymbolEnumerationPass1Callback(IN PSYMBOL_INFO pSymInfo,IN ULONG SymbolSize,IN PVOID UserContext OPTIONAL)
{
	// In Pass #1, check the number of symbols.
	PIMAGE_SYMBOL_LIST ImgSym=(PIMAGE_SYMBOL_LIST)UserContext;
	ImgSym->NumberOfSymbols++;
	return TRUE;
}

BOOL PvdCreateDebugImage(OUT PIMAGE_SYMBOL_LIST *ImageSymbol)
{
	BOOL Result=FALSE;
	PIMAGE_SYMBOL_LIST ImgSym=MemAlloc(sizeof(IMAGE_SYMBOL_LIST));
	if(ImgSym)
	{
		ImgSym->ModuleInformation.SizeOfStruct=sizeof(IMAGEHLP_MODULE64);
		AcquireSRWLockExclusive(&PvDebugImageListLock);
		InsertTailList(&PvDebugImageListHead.List,&ImgSym->List);
		ReleaseSRWLockExclusive(&PvDebugImageListLock);
		*ImageSymbol=ImgSym;
		Result=TRUE;
	}
	return Result;
}

void PvdUnloadSymbolForGuestModule(IN ULONG64 ImageBaseGva)
{
	// Search the image and pop it out.
	PIMAGE_SYMBOL_LIST Cur=&PvDebugImageListHead,ImgSym=NULL;
	// Thread-safety consideration.
	AcquireSRWLockExclusive(&PvDebugImageListLock);
	do
	{
		if(Cur->ModuleInformation.BaseOfImage==ImageBaseGva)
		{
			ImgSym=Cur;
			RemoveListEntry(&Cur->List);
			break;
		}
		Cur=(PIMAGE_SYMBOL_LIST)Cur->List.Flink;
	}while(Cur!=&PvDebugImageListHead);
	ReleaseSRWLockExclusive(&PvDebugImageListLock);
	// Unload the symbols...
	if(ImgSym)
	{
		SymUnloadModule64(GetCurrentProcess(),ImgSym->ModuleInformation.BaseOfImage);
		if(ImgSym->SortedByName)
		{
			for(ULONG32 i=0;i<ImgSym->NumberOfSymbols;i++)
				if(ImgSym->SortedByName[i])
					MemFree(ImgSym->SortedByName[i]);
			MemFree(ImgSym->SortedByName);
		}
		if(ImgSym->SortedByAddr)
		{
			for(ULONG32 i=0;i<ImgSym->NumberOfSymbols;i++)
				if(ImgSym->SortedByAddr[i])
					MemFree(ImgSym->SortedByAddr[i]);
			MemFree(ImgSym->SortedByAddr);
		}
		MemFree(ImgSym);
	}
}

BOOL PvdLoadSymbolForGuestModule(IN ULONG64 ImageBaseGva,IN ULONG ImageSize,IN HANDLE FileHandle,IN PCSTR ImageName)
{
	BOOL Result=SymLoadModuleEx(GetCurrentProcess(),FileHandle,ImageName,NULL,ImageBaseGva,ImageSize,NULL,0)==ImageBaseGva;
	if(Result)
	{
		PIMAGE_SYMBOL_LIST ImgSym=NULL;
		Result=PvdCreateDebugImage(&ImgSym);
		if(Result)
		{
			Result=SymGetModuleInfo64(GetCurrentProcess(),ImageBaseGva,&ImgSym->ModuleInformation);
			if(Result)
			{
				// Enumeration Pass #1: Check the number of symbols.
				Result=SymEnumSymbols(GetCurrentProcess(),ImageBaseGva,NULL,PvdSymbolEnumerationPass1Callback,ImgSym);
				if(Result)
				{
					// Allocate arrays for symbols.
					ImgSym->SortedByAddr=MemAlloc(sizeof(PSYMBOL_INFO)*ImgSym->NumberOfSymbols);
					ImgSym->SortedByName=MemAlloc(sizeof(PSYMBOL_INFO)*ImgSym->NumberOfSymbols);
					// Enumeration Pass #2: Record information for symbols.
					if(ImgSym->SortedByAddr && ImgSym->SortedByName)
						Result=SymEnumSymbols(GetCurrentProcess(),ImageBaseGva,NULL,PvdSymbolEnumerationPass2Callback,ImgSym);
					else
						Result=FALSE;
					if(Result)
					{
						// Sort these symbols for binary searching.
						qsort(ImgSym->SortedByAddr,ImgSym->NumberOfSymbols,sizeof(PSYMBOL_INFO),PvdSymbolSortByAddrCallback);
						qsort(ImgSym->SortedByName,ImgSym->NumberOfSymbols,sizeof(PSYMBOL_INFO),PvdSymbolSortByNameCallback);
					}
				}
			}
		}
	}
	return Result;
}

BOOL PvdGetSymbolNameWithLineInfo(IN ULONG64 Address,OUT PSTR Buffer,IN SIZE_T BufferSize)
{
	BOOL Result=FALSE;
	PSYMBOL_INFO SymInfo=MemAlloc(1024);
	if(SymInfo)
	{
		ULONG64 Disp;
		SymInfo->SizeOfStruct=sizeof(SYMBOL_INFO);
		SymInfo->MaxNameLen=1024-sizeof(SYMBOL_INFO)+1;
		Result=SymFromAddr(GetCurrentProcess(),Address,&Disp,SymInfo);
		if(Result)
		{
			IMAGEHLP_LINE64 LineInfo;
			PSTR Remainder;
			SIZE_T RemainderLength;
			StringCbPrintfExA(Buffer,BufferSize-1,&Remainder,&RemainderLength,0,"%s+0x%llX",SymInfo->Name,Disp);
			LineInfo.SizeOfStruct=sizeof(LineInfo);
			if(SymGetLineFromAddr64(GetCurrentProcess(),Address,(PULONG)&Disp,&LineInfo))
				StringCbPrintfA(Remainder,RemainderLength," [%s] @ %u",LineInfo.FileName,LineInfo.LineNumber);
		}
		MemFree(SymInfo);
	}
	return Result;
}

void PvdLocateImageDebugDatabasePath(IN ULONG64 GuestCr3,IN ULONG64 ImageBaseGva,OUT PWSTR DatabasePath,IN ULONG Cch)
{
	IMAGE_DOS_HEADER DosHead;
	ULONG ErrCode=PvReadGuestMemory(GuestCr3,ImageBaseGva,&DosHead,sizeof(DosHead),TRUE,1);
	if(ErrCode==0 && DosHead.e_magic==IMAGE_DOS_SIGNATURE)
	{
		IMAGE_NT_HEADERS64 NtHead;
		ErrCode=PvReadGuestMemory(GuestCr3,ImageBaseGva+DosHead.e_lfanew,&NtHead,sizeof(NtHead),TRUE,1);
		if(ErrCode==0 && NtHead.Signature==IMAGE_NT_SIGNATURE)
		{
			ULONG DbgDirRva=NtHead.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
			if(DbgDirRva)
			{
				PIMAGE_DEBUG_DIRECTORY DbgDir=MemAlloc(NtHead.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size);
				if(DbgDir)
				{
					ULONG Count=NtHead.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size/sizeof(IMAGE_DEBUG_DIRECTORY);
					ErrCode=PvReadGuestMemory(GuestCr3,ImageBaseGva+DbgDirRva,DbgDir,NtHead.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size,TRUE,1);
					if(ErrCode==0)
					{
						for(ULONG i=0;i<Count;i++)
						{
							switch(DbgDir[i].Type)
							{
								case IMAGE_DEBUG_TYPE_CODEVIEW:
								{
									PIMAGE_CODEVIEW_DEBUG_DIRECTORY_ENTRY CvDebug=MemAlloc(DbgDir[i].SizeOfData);
									if(CvDebug)
									{
										PvReadGuestMemory(GuestCr3,ImageBaseGva+DbgDir[i].AddressOfRawData,CvDebug,DbgDir[i].SizeOfData,TRUE,1);
										if(CvDebug->Signature!=IMAGE_CODEVIEW_SIGNATURE)
											PvPrintConsoleA("Debug Signature Mismatch!\n");
										else
											MultiByteToWideChar(CP_UTF8,0,CvDebug->Path,DbgDir[i].SizeOfData-24,DatabasePath,Cch);
										MemFree(CvDebug);
									}
									break;
								}
							}
						}
					}
					MemFree(DbgDir);
				}
			}
		}
	}
}

BOOL PvdInitialize()
{
	InitializeListHead(&PvDebugImageListHead.List);
	SymSetOptions(SYMOPT_LOAD_LINES);
	return SymInitializeW(GetCurrentProcess(),NULL,FALSE);
}

BOOL PvdFinalize()
{
	return SymCleanup(GetCurrentProcess());
}