#include <Windows.h>
#include <dbghelp.h>
#include <strsafe.h>
#include "debug_agent.h"

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
										{
											MultiByteToWideChar(CP_UTF8,0,CvDebug->Path,DbgDir[i].SizeOfData-24,DatabasePath,Cch);
											PvPrintConsoleA("The PDB File is located in %ws\n",DatabasePath);
										}
										MemFree(CvDebug);
									}
									break;
								}
								default:
								{
									PvPrintConsoleA("Unknown Debug Type %u is detected!\n",DbgDir[i].Type);
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
	return SymInitializeW(GetCurrentProcess(),NULL,FALSE);
}

BOOL PvdFinalize()
{
	return SymCleanup(GetCurrentProcess());
}