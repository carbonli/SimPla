/*
 * update_ghosts.h
 *
 *  Created on: 2014年6月4日
 *      Author: salmon
 */

#ifndef UPDATE_GHOSTS_H_
#define UPDATE_GHOSTS_H_

#include <mpi.h>
#include "message_comm.h"
#include "mpi_datatype.h"
#include "distributed_array.h"

namespace simpla
{

template<int N, typename TV>
void UpdateGhosts(TV* data, DistributedArray<N> const & global_array, MPI_Comm comm = MPI_COMM_NULL)
{
	if (global_array.send_recv_.size() == 0)
		return;

	if (comm == MPI_COMM_NULL)
	{
		comm = GLOBAL_COMM.GetComm();
	}

	MPI_Request request[global_array.send_recv_.size() * 2];

	int count = 0;

	for (auto const & item : global_array.send_recv_)
	{
		auto g_outer_count = global_array.local_.outer_end - global_array.local_.outer_begin;
		auto send_count = item.send_end - item.send_begin;
		auto recv_count = item.recv_end - item.recv_begin;

		MPIDataType<TV> send_type(g_outer_count, send_count, item.send_begin - global_array.local_.outer_begin);
		MPIDataType<TV> recv_type(g_outer_count, recv_count, item.recv_begin - global_array.local_.outer_begin);

		MPI_Isend(data, 1, send_type.type(), item.dest, item.send_tag, comm, &request[count * 2]);
		MPI_Irecv(data, 1, recv_type.type(), item.dest, item.recv_tag, comm, &request[count * 2 + 1]);
		++count;
	}

	MPI_Waitall(global_array.send_recv_.size() * 2, request, MPI_STATUSES_IGNORE);

}

}  // namespace simpla
#endif /* UPDATE_GHOSTS_H_ */
