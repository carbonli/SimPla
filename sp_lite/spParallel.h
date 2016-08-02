//
// Created by salmon on 16-7-6.
//

#ifndef SIMPLA_SPPARALLEL_H
#define SIMPLA_SPPARALLEL_H

#include "sp_lite_def.h"
#include "../src/sp_capi.h"

int spParallelInitialize(int argc, char **argv);

int spParallelFinalize();

int spParallelDeviceInitialize(int argc, char **argv);

int spParallelDeviceFinalize();

int spParallelDeviceAlloc(void **, size_type);

int spParallelDeviceFree(void **);

int spParallelHostAlloc(void **, size_type);

int spParallelHostFree(void **);

int spParallelMemcpy(void *, void const *, size_type);

int spParallelMemcpyToSymbol(void *, void const *, size_type);

int spParallelMemset(void *, int v, size_type);

int spParallelDeviceSync();

int spParallelGlobalBarrier();

int spParallelAssign(size_type num_of_point, size_type *offset, Real *d, Real const *v);

int spParallelUpdateNdArrayHalo(void *buffer,
                                const struct spDataType_s *ele_type,
                                int ndims,
                                const size_type *dims,
                                const size_type *start,
                                const size_type *,
                                const size_type *count,
                                const size_type *,
                                int mpi_sync_start_dims);

int spParallelDeviceFillInt(int *d, int v, size_type s);

int spParallelDeviceFillReal(Real *d, Real v, size_type s);


/**
 *  \f[
 *      f\left(v\right)\equiv\frac{1}{\sqrt{\left(2\pi\sigma\right)^{3}}}\exp\left(-\frac{\left(v-u\right)^{2}}{\sigma^{2}}\right)
 *  \f]
 * @param data
 * @param num_of_sample
 * @param u0
 * @param sigma
 * @return
 */
int spRandomNormal3(Real **data, size_type num_of_sample, Real const *u0, Real sigma);

#endif //SIMPLA_SPPARALLEL_H
