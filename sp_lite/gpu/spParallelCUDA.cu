//
// Created by salmon on 16-7-25.
//

extern "C" {
#include "spParallelCUDA.h"
}
dim3 sizeType2Dim3(size_type const *v)
{
    dim3 res;
    res.x = (int) v[0];
    res.y = (int) v[1];
    res.z = (int) v[2];
    return res;
}
Real3 real2Real3(Real const *v)
{
    Real3 res;
    res.x = (Real) v[0];
    res.y = (Real) v[1];
    res.z = (Real) v[2];
    return res;
}

int spParallelDeviceInitialize(int argc, char **argv)
{
    int num_of_device = 0;
    SP_PARALLEL_CHECK_RETURN(cudaGetDeviceCount(&num_of_device));
    SP_PARALLEL_CHECK_RETURN(cudaSetDevice(spMPIRank() % num_of_device));
    SP_PARALLEL_CHECK_RETURN(cudaThreadSynchronize()); // Wait for the GPU launched work to complete
    SP_PARALLEL_CHECK_RETURN(cudaGetLastError());
}

int spParallelDeviceFinalize()
{
    SP_PARALLEL_CHECK_RETURN(cudaDeviceReset());
}

int spParallelDeviceAlloc(void **p, size_type s)
{
    SP_PARALLEL_CHECK_RETURN(cudaMalloc(p, s));
    return SP_SUCCESS;
}

int spParallelDeviceFree(void **_P_)
{
    if (*_P_ != NULL)
    {
        SP_PARALLEL_CHECK_RETURN(cudaFree(*_P_));
        *_P_ = NULL;
    }
    return SP_SUCCESS;
};

int spParallelMemcpy(void *dest, void const *src, size_type s)
{
    SP_PARALLEL_CHECK_RETURN(cudaMemcpy(dest, src, s, cudaMemcpyDefault));
    return SP_SUCCESS;
}

int spParallelMemcpyToSymbol(void **dest, void const **src, size_type s)
{
    SP_PARALLEL_CHECK_RETURN(cudaMemcpyToSymbol(dest, src, s, cudaMemcpyDefault));
    return SP_SUCCESS;
}

int spParallelMemset(void *dest, int v, size_type s)
{
    SP_PARALLEL_CHECK_RETURN(cudaMemset(dest, v, s));
    return SP_SUCCESS;
}

int spParallelDeviceSync()
{
    SP_CHECK_RETURN(spParallelGlobalBarrier());
    SP_PARALLEL_CHECK_RETURN(cudaDeviceSynchronize());
    return SP_SUCCESS;
}

int spParallelHostAlloc(void **p, size_type s)
{
    SP_PARALLEL_CHECK_RETURN(cudaHostAlloc(p, s, cudaHostAllocDefault));
    return SP_SUCCESS;
};

int spParallelHostFree(void **p)
{
    if (*p != NULL)
    {
        cudaFreeHost(*p);
        *p = NULL;
    }
    return SP_SUCCESS;
}


__global__
void spParallelDeviceFillIntKernel(int *d, int v, size_type max)
{
    for (size_t s = threadIdx.x + blockIdx.x * blockDim.x; s < max; s += gridDim.x * blockDim.x) { d[s] = v; }
};
int spParallelDeviceFillInt(int *d, int v, size_type s)
{
    LOAD_KERNEL(spParallelDeviceFillIntKernel, 16, 256, d, v, s);

    return SP_SUCCESS;
};

__global__
void spParallelDeviceFillRealKernel(Real *d, Real v, size_type max)
{
    for (size_type s = threadIdx.x + blockIdx.x * blockDim.x; s < max; s += gridDim.x * blockDim.x) { d[s] = v; }
};
int spParallelDeviceFillReal(Real *d, Real v, size_type s)
{
    LOAD_KERNEL(spParallelDeviceFillRealKernel, 16, 256, d, v, s);
    return SP_SUCCESS;
};


__global__
void spParallelAssignKernel(size_type max, size_type const *offset, Real *d, Real const *v)
{

    size_type num_of_thread = blockDim.x * gridDim.x * blockDim.x * gridDim.x * blockDim.x * gridDim.x;

    for (size_type s = (threadIdx.x + blockIdx.x * blockDim.x) +
        (threadIdx.y + blockIdx.y * blockDim.y) * blockDim.x * gridDim.x +
        (threadIdx.x + blockIdx.x * blockDim.x) * blockDim.x * gridDim.x * blockDim.y * gridDim.y;
         s < max; s += num_of_thread) { d[offset[s]] = v[s]; }
};

int spParallelAssign(size_type num_of_point, size_type *offset, Real *d, Real const *v)
{
    LOAD_KERNEL(spParallelAssignKernel, 16, 256, num_of_point, offset, d, v);
    return SP_SUCCESS;
};