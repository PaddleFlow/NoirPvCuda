#include <pvdef.h>
#include <hypercall.h>
#include <cudart.h>

cudaError_t cudaGetDeviceCount(int* count)
{
	return NoirHypercall(NOIR_HYPERCALL_CUDA_GET_DEVICE_COUNT,count);
}

cudaError_t cudaGetDeviceProperties(struct cudaDeviceProp *prop,int device)
{
	return NoirHypercall(NOIR_HYPERCALL_CUDA_GET_DEVICE_PROP,prop,device);
}