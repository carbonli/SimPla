//
// Created by salmon on 17-9-18.
//
#include "MPIUpdater.h"
#include <simpla/SIMPLA_config.h>
#include <simpla/utilities/macro.h>
#include <typeinfo>
#include "MPIComm.h"
namespace simpla {
namespace parallel {

struct MPIUpdater::pimpl_s {
    bool m_is_setup_ = false;
    int ndims = 3;
    int mpi_topology_ndims = 0;
    int mpi_dims[3] = {1, 1, 1};
    int mpi_periods[3] = {1, 1, 1};
    int mpi_coords[3] = {0, 0, 0};

    index_box_type m_index_box_{{0, 0, 0}, {1, 1, 1}};
    index_box_type m_halo_box_{{0, 0, 0}, {1, 1, 1}};

    //    index_tuple m_gw_{2, 2, 2};

    //    MPI_Datatype ele_type;
    size_t ele_type;
    int tag = 0;
    int m_direction_ = 0;
    int left = 0, right = 0;
    int m_rank_ = 0;
    int m_coord_[3] = {0, 0, 0};
    bool m_is_perodic_ = true;
};
MPIUpdater::MPIUpdater() : m_pimpl_(new pimpl_s) {
    if (GLOBAL_COMM.size() <= 1) { return; }
    SP_CALL(GLOBAL_COMM.topology(&m_pimpl_->mpi_topology_ndims, m_pimpl_->mpi_dims, m_pimpl_->mpi_periods,
                                 m_pimpl_->mpi_coords));
};
MPIUpdater::~MPIUpdater() { TearDown(); };

void MPIUpdater::SetDirection(int d) { m_pimpl_->m_direction_ = d; }
int MPIUpdater::GetDirection() const { return m_pimpl_->m_direction_; }
void MPIUpdater::SetPeriodic(bool tag) { m_pimpl_->m_is_perodic_ = tag; }
bool MPIUpdater::IsPeriodic() const { return m_pimpl_->m_is_perodic_; }

void MPIUpdater::SetIndexBox(index_box_type const &idx_box) {
    m_pimpl_->m_index_box_ = idx_box;
    m_pimpl_->m_halo_box_ = idx_box;
}
index_box_type MPIUpdater::GetIndexBox() const { return m_pimpl_->m_index_box_; }

void MPIUpdater::SetHaloWidth(index_tuple const &gw) {
    for (int i = 0; i < m_pimpl_->ndims; ++i) {
        if (std::get<1>(m_pimpl_->m_index_box_)[i] - std::get<0>(m_pimpl_->m_index_box_)[i] > 2 * gw[i]) {
            std::get<0>(m_pimpl_->m_halo_box_)[i] = std::get<0>(m_pimpl_->m_index_box_)[i] - gw[i];
            std::get<1>(m_pimpl_->m_halo_box_)[i] = std::get<1>(m_pimpl_->m_index_box_)[i] + gw[i];
        }
    }
}

void MPIUpdater::SetHaloIndexBox(index_box_type const &b) { m_pimpl_->m_halo_box_ = b; }
index_box_type MPIUpdater::GetHaloIndexBox() const { return m_pimpl_->m_halo_box_; }

bool MPIUpdater::isSetUp() const { return m_pimpl_->m_is_setup_; }

void MPIUpdater::SetUp() {
    if (m_pimpl_->m_is_setup_) { return; }

    m_pimpl_->m_is_setup_ = true;
    index_box_type send_box[2];
    index_box_type recv_box[2];

    for (auto &b : send_box) { b = m_pimpl_->m_index_box_; }
    for (auto &b : recv_box) { b = m_pimpl_->m_index_box_; }

    auto d = m_pimpl_->m_direction_;

    for (int i = 0; i < d; ++i) {
        std::get<0>(send_box[0])[i] = std::get<0>(m_pimpl_->m_halo_box_)[i];
        std::get<1>(send_box[0])[i] = std::get<1>(m_pimpl_->m_halo_box_)[i];

        std::get<0>(recv_box[0])[i] = std::get<0>(m_pimpl_->m_halo_box_)[i];
        std::get<1>(recv_box[0])[i] = std::get<1>(m_pimpl_->m_halo_box_)[i];

        std::get<0>(send_box[1])[i] = std::get<0>(m_pimpl_->m_halo_box_)[i];
        std::get<1>(send_box[1])[i] = std::get<1>(m_pimpl_->m_halo_box_)[i];

        std::get<0>(recv_box[1])[i] = std::get<0>(m_pimpl_->m_halo_box_)[i];
        std::get<1>(recv_box[1])[i] = std::get<1>(m_pimpl_->m_halo_box_)[i];
    }

    std::get<0>(send_box[0])[d] = std::get<0>(m_pimpl_->m_index_box_)[d];
    std::get<1>(send_box[0])[d] = std::get<0>(m_pimpl_->m_index_box_)[d] * 2 - std::get<0>(m_pimpl_->m_halo_box_)[d];

    std::get<0>(recv_box[0])[d] = std::get<0>(m_pimpl_->m_halo_box_)[d];
    std::get<1>(recv_box[0])[d] = std::get<0>(m_pimpl_->m_index_box_)[d];

    std::get<0>(send_box[1])[d] = std::get<1>(m_pimpl_->m_index_box_)[d] * 2 - std::get<1>(m_pimpl_->m_halo_box_)[d];
    std::get<1>(send_box[1])[d] = std::get<1>(m_pimpl_->m_index_box_)[d];

    std::get<0>(recv_box[1])[d] = std::get<1>(m_pimpl_->m_index_box_)[d];
    std::get<1>(recv_box[1])[d] = std::get<1>(m_pimpl_->m_halo_box_)[d];

    m_pimpl_->ele_type = value_type_info().hash_code();

    for (int i = 0; i < 2; ++i) {
        GetSendBuffer(i).reset(send_box[i]);
        GetRecvBuffer(i).reset(recv_box[i]);
        GetSendBuffer(i).Clear();
        GetRecvBuffer(i).Clear();
    }

    m_pimpl_->m_rank_ = GLOBAL_COMM.rank();
    GLOBAL_COMM.CartShift(m_pimpl_->m_direction_, 1, &m_pimpl_->left, &m_pimpl_->right);
}
void MPIUpdater::Clear() {
    GetRecvBuffer(0).FillNaN();
    GetRecvBuffer(1).FillNaN();
    GetSendBuffer(0).FillNaN();
    GetSendBuffer(1).FillNaN();
}

void MPIUpdater::TearDown() { m_pimpl_->m_is_setup_ = false; }

void MPIUpdater::SetTag(int tag) { m_pimpl_->tag = tag; }

void MPIUpdater::Push(ArrayBase const &a) {
    GetSendBuffer(0).CopyIn(a);
    GetSendBuffer(1).CopyIn(a);
}
void MPIUpdater::Pop(ArrayBase &a) const {
    a.CopyIn(GetRecvBuffer(0));
    a.CopyIn(GetRecvBuffer(1));
}

void MPIUpdater::SendRecv() {
    if (GLOBAL_COMM.size() > 1 && m_pimpl_->left != m_pimpl_->m_rank_ && m_pimpl_->right != m_pimpl_->m_rank_) {
        GLOBAL_COMM.barrier();
        GLOBAL_COMM.SendRecv(GetSendBuffer(0).pointer(), static_cast<int>(GetSendBuffer(0).size()),  //
                             m_pimpl_->ele_type, m_pimpl_->left, m_pimpl_->tag,                      //
                             GetRecvBuffer(1).pointer(), static_cast<int>(GetRecvBuffer(1).size()),  //
                             m_pimpl_->ele_type, m_pimpl_->right, m_pimpl_->tag                      //
                             );
        GLOBAL_COMM.barrier();
        GLOBAL_COMM.SendRecv(GetSendBuffer(1).pointer(), static_cast<int>(GetSendBuffer(1).size()),  //
                             m_pimpl_->ele_type, m_pimpl_->right, m_pimpl_->tag,                     //
                             GetRecvBuffer(0).pointer(), static_cast<int>(GetRecvBuffer(0).size()),  //
                             m_pimpl_->ele_type, m_pimpl_->left, m_pimpl_->tag                       //
                             );
        GLOBAL_COMM.barrier();
    } else  // if (IsPeriodic())
    {
        index_tuple shift = {0, 0, 0};
        shift[m_pimpl_->m_direction_] = std::get<1>(m_pimpl_->m_index_box_)[m_pimpl_->m_direction_] -
                                        std::get<0>(m_pimpl_->m_index_box_)[m_pimpl_->m_direction_];
        auto s0 = GetSendBuffer(0).DuplicateArray();
        s0->Shift(&shift[0]);
        GetRecvBuffer(1).CopyIn(*s0);
        shift[m_pimpl_->m_direction_] *= -1;
        auto s1 = GetSendBuffer(1).DuplicateArray();
        s1->Shift(&shift[0]);
        GetRecvBuffer(0).CopyIn(*s1);
    }
}

}  // namespace parallel {
}  // namespace simpla {