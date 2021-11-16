#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <NoirCvmApi.h>
#include "main.h"

int main(int argc,char* argv[],char* envp[])
{
	NOIR_STATUS st;
	NoirInitializeLibrary();
	PvInitializeConsole();
	PvPrintConsoleA("Welcome to Paravirtualized Cuda!\n");
	st=PvInitializeVirtualMachine(1);
	if(st==NOIR_SUCCESS)
	{
		BOOL b=PvInitializeGuestInternalMapping();
		if(b)PvPrintConsoleA("Guest Internal Mapping is initialized successfully!\n");
		b=PvLoadParavirtualizedKernel();
		if(b)PvPrintConsoleA("Paravirtualized Kernel image is loaded successfully!\n");
		st=PvStartParavirtualizedGuest();
		PvPrintConsoleA("Paravirtualized Guest Startup Status=0x%X\n",st);
		PvFinalizeVirtualMachine();
		PvPrintConsoleA("Guest VM is terminated...\n");
	}
	else
		PvPrintConsoleA("Failed to initialize virtual machine! Status=0x%X\n",st);
	NoirFinalizeLibrary();
	system("pause");
	return 0;
}