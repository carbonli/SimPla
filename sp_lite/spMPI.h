//
// Created by salmon on 16-9-12.
//

#ifndef SIMPLA_SPMPI_H
#define SIMPLA_SPMPI_H

#include <mpi.h>
#include "sp_config.h"

#define SP_MPI_UPDATER_HEAD            \
        MPI_Comm comm;                 \
        int num_of_neighbour;          \
        int send_count[6];             \
        int recv_count[6];             \
        int type_tag;                  \
        MPI_Aint send_displs[6];       \
        MPI_Aint recv_displs[6];       \
        void *send_buffer[6];          \
        void *recv_buffer[6];          \

struct spMPIUpdater_s
{
    SP_MPI_UPDATER_HEAD
};
typedef struct spMPIUpdater_s spMPIUpdater;

int spMPIInitialize(int argc, char **argv);

int spMPIFinalize();

MPI_Comm spMPIComm();

size_type spMPIGenerateObjectId();

int spMPIBarrier();

int spMPIRank();

int spMPISize();

int spMPITopology(int *mpi_topo_ndims, int *mpi_topo_dims, int *periods, int *mpi_topo_coord);

int spMPIPrefixSum(size_type *offset, size_type *p_count);

int spMPIUpdaterCreate(spMPIUpdater **updater, size_type size, int type_tag);

int spMPIUpdaterDestroy(spMPIUpdater **updater);

struct spMPIHaloUpdater_s;

typedef struct spMPIHaloUpdater_s spMPIHaloUpdater;

int spMPIHaloUpdaterCreate(spMPIHaloUpdater **updater, int type_tag);


int spMPIHaloUpdaterSetup(spMPIHaloUpdater *updater, int mpi_sync_start_dims, int ndims,
                          const size_type *shape, const size_type *start, const size_type *stride,
                          const size_type *count, const size_type *block);


int spMPIHaloUpdaterDestroy(spMPIHaloUpdater **updater);

int spMPIHaloUpdate(spMPIHaloUpdater *updater, void *);

int spMPIHaloUpdateAll(spMPIHaloUpdater *updater, int num_of_buffer, void **buffers);


struct spMPIBucketUpdater_s;

typedef struct spMPIBucketUpdater_s spMPIBucketUpdater;

int spMPIBucketUpdaterCreate(spMPIBucketUpdater **updater, int data_type_tag);

int spMPIBucketUpdaterDestroy(spMPIBucketUpdater **updater);

int spMPIBucketUpdaterSetup(spMPIBucketUpdater *updater, const size_type *bucket_start,
                            const size_type *buck_count, size_type const *sorted_index);


int spMPIBucketUpdate(spMPIBucketUpdater *updater, void *buffer);

int spMPIBucketUpdateAll(spMPIBucketUpdater *updater, int num_of_buffer, void **buffers);


#endif //SIMPLA_SPMPI_H
