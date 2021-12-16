#include <Windows.h>
#include <strsafe.h>
#include <cuda_runtime.h>
#include <NoirCvmApi.h>
#include "virt_cudart.h"

cudaError_t vcudaGetDeviceCount(int* count)
{
	return cudaGetDeviceCount(count);
}

cudaError_t vcudaGetDeviceProperties(struct cudaDeviceProp* prop,int device)
{
	cudaError_t ErrCode=cudaGetDeviceProperties(prop,device);
	StringCbCatA(prop->name,sizeof(prop->name)," (Paravirtualized by NoirVisor)");
	return ErrCode;
}

BOOLEAN PvHandleCudaHypercall(IN ULONG32 VpIndex,IN OUT PNOIR_GPR_STATE GprState,IN OUT PNOIR_CR_STATE CrState,IN PNOIR_CVM_EXIT_CONTEXT ExitContext)
{
	BOOLEAN Result=TRUE;
	switch(GprState->Rcx)
	{
		case NOIR_HYPERCALL_CUDA_GET_DEVICE_COUNT:
		{
			int DevCount;
			GprState->Rax=vcudaGetDeviceCount(&DevCount);
			PvPrintConsoleA("Call to cudaGetDeviceCount is intercepted!\n");
			Result=(PvWriteGuestMemory(CrState->Cr3,GprState->Rdx,&DevCount,sizeof(DevCount),TRUE,1)==0);
			break;
		}
		case NOIR_HYPERCALL_CUDA_GET_DEVICE_PROP:
		{
			struct cudaDeviceProp DevProp;
			GprState->Rax=vcudaGetDeviceProperties(&DevProp,(int)GprState->R8);
			PvPrintConsoleA("Call to cudaGetDeviceProperties is intercepted!\n");
			Result=(PvWriteGuestMemory(CrState->Cr3,GprState->Rdx,&DevProp,sizeof(DevProp),TRUE,1)==0);
			break;
		}
	}
	return Result;
}