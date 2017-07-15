/**
 * @file mpi_comm.cpp
 *
 *  Created on: 2014-11-21
 *      Author: salmon
 */
#include "MPIComm.h"
#include <mpi.h>
#include "simpla/SIMPLA_config.h"
#include "simpla/utilities/Log.h"
#include <cassert>
#include <iostream>
#include <vector>
namespace simpla {
namespace parallel {

struct MPIComm::pimpl_s {
    static constexpr int MAX_NUM_OF_DIMS = 3;
    MPI_Comm m_comm_ = MPI_COMM_WORLD;
    size_type m_object_id_count_ = 0;
    int m_topology_ndims_ = 3;
    int m_topology_dims_[3] = {0, 0, 0};
};

MPIComm::MPIComm() : m_pimpl_(new pimpl_s) {}

MPIComm::MPIComm(int argc, char **argv) : MPIComm() { init(argc, argv); }

MPIComm::~MPIComm() { close(); }

int MPIComm::process_num() const { return rank(); }

int MPIComm::num_of_process() const { return size(); }

int MPIComm::rank() const {
    int res = 0;
    if (m_pimpl_->m_comm_ != MPI_COMM_NULL) { MPI_Comm_rank(m_pimpl_->m_comm_, &res); }
    return res;
}

int MPIComm::size() const {
    int res = 1;
    if (m_pimpl_->m_comm_ != MPI_COMM_NULL) { MPI_Comm_size(m_pimpl_->m_comm_, &res); }
    return res;
}

int MPIComm::get_rank(int const *d) const {
    int res = 0;
    MPI_CALL(MPI_Cart_rank(m_pimpl_->m_comm_, (int *)d, &res));
    return res;
}

void MPIComm::init(int argc, char **argv) {
    m_pimpl_->m_object_id_count_ = 0;
    MPI_CALL(MPI_Init(&argc, &argv));
    int m_num_process_;
    MPI_CALL(MPI_Comm_size(MPI_COMM_WORLD, &m_num_process_));
    if (m_num_process_ > 1) {
        int m_topology_coord_[3] = {0, 0, 0};
        MPI_CALL(MPI_Dims_create(m_num_process_, m_pimpl_->m_topology_ndims_, m_pimpl_->m_topology_dims_));
        int periods[m_pimpl_->m_topology_ndims_];
        for (int i = 0; i < m_pimpl_->m_topology_ndims_; ++i) { periods[i] = true; }
        MPI_CALL(MPI_Cart_create(MPI_COMM_WORLD, m_pimpl_->m_topology_ndims_, m_pimpl_->m_topology_dims_, periods,
                                 MPI_ORDER_C, &m_pimpl_->m_comm_));
        logger::set_mpi_comm(rank(), size());

        MPI_CALL(MPI_Cart_coords(m_pimpl_->m_comm_, rank(), m_pimpl_->m_topology_ndims_, m_topology_coord_));

        INFORM << "MPI communicator is initialized! "
                  "[("
               << m_topology_coord_[0] << "," << m_topology_coord_[1] << "," << m_topology_coord_[2] << ")/("
               << m_pimpl_->m_topology_dims_[0] << "," << m_pimpl_->m_topology_dims_[1] << ","
               << m_pimpl_->m_topology_dims_[2] << ")]" << std::endl;
    }
}

size_type MPIComm::generate_object_id() {
    assert(m_pimpl_ != nullptr);

    ++(m_pimpl_->m_object_id_count_);

    return m_pimpl_->m_object_id_count_;
}

void const *MPIComm::comm() const { return reinterpret_cast<void *>(&m_pimpl_->m_comm_); }
//
// MPI_Info MPIComm::info() {
//    assert(m_pack_ != nullptr);
//    return MPI_INFO_NULL;
//}

void MPIComm::barrier() {
    if (m_pimpl_->m_comm_ != MPI_COMM_NULL) { MPI_Barrier(m_pimpl_->m_comm_); }
}

bool MPIComm::is_valid() const { return ((!!m_pimpl_) && m_pimpl_->m_comm_ != MPI_COMM_NULL) && num_of_process() > 1; }

int MPIComm::topology(int *mpi_topo_ndims, int *mpi_topo_dims, int *periods, int *mpi_topo_coord) const {
    *mpi_topo_ndims = 0;
    if (mpi_topo_dims == nullptr || periods == nullptr || mpi_topo_coord == nullptr) { return SP_SUCCESS; }
    if (m_pimpl_->m_comm_ == MPI_COMM_NULL) {
        *mpi_topo_dims = 1;
        *periods = 1;
        *mpi_topo_coord = 0;

    } else {
        int tope_type = MPI_CART;

        MPI_CALL(MPI_Topo_test(m_pimpl_->m_comm_, &tope_type));

        if (tope_type == MPI_CART) {
            MPI_CALL(MPI_Cartdim_get(m_pimpl_->m_comm_, mpi_topo_ndims));

            MPI_CALL(MPI_Cart_get(m_pimpl_->m_comm_, *mpi_topo_ndims, mpi_topo_dims, periods, mpi_topo_coord));
        }
    }
    return SP_SUCCESS;
};

void MPIComm::close() {
    if (m_pimpl_ != nullptr && m_pimpl_->m_comm_ != MPI_COMM_NULL) {
        VERBOSE << "MPI Communicator is closed!" << std::endl;

        MPI_CALL(MPI_Finalize());

        m_pimpl_->m_comm_ = MPI_COMM_NULL;
    }
}

void bcast_string(std::string *filename_) {
    if (GLOBAL_COMM.size() <= 1) return;
    int name_len;
    if (GLOBAL_COMM.process_num() == 0) { name_len = static_cast<int>(filename_->size()); }
    MPI_Bcast(&name_len, 1, MPI_INT, 0, GLOBAL_COMM.m_pimpl_->m_comm_);
    std::vector<char> buffer(static_cast<size_type>(name_len));
    if (GLOBAL_COMM.process_num() == 0) { std::copy(filename_->begin(), filename_->end(), buffer.begin()); }
    MPI_Bcast((&buffer[0]), name_len, MPI_CHAR, 0, GLOBAL_COMM.m_pimpl_->m_comm_);
    buffer.push_back('\0');
    if (GLOBAL_COMM.process_num() != 0) { *filename_ = &buffer[0]; }
}
}
}  // namespace simpla{namespace parallel{
