//
// Created by salmon on 16-10-10.
//

#include "MeshBlock.h"
#include <simpla/concept/Object.h>
#include <simpla/toolbox/nTuple.h>
#include <simpla/toolbox/nTupleExt.h>
#include <simpla/toolbox/PrettyStream.h>
#include <simpla/toolbox/Log.h>

namespace simpla { namespace mesh
{

MeshBlock::MeshBlock() : m_ndims_(0) {}


MeshBlock::MeshBlock(int ndims, index_type const *lo, index_type const *up, Real const *dx, Real const *xlo) :
        Object(),
        m_ndims_(ndims),
        m_g_box_{{lo[0], lo[1], lo[2]},
                 {up[0], up[1], up[2]}},
        m_global_origin_{xlo[0] - lo[0] * dx[0], xlo[1] - lo[1] * dx[1], xlo[2] - lo[2] * dx[2]},
        m_dx_{dx[0], dx[1], dx[2]},
        m_level_(0)
{
    assert(ndims <= 3);

    if (m_is_deployed_) { return; }

    base_type::deploy();
    ASSERT(toolbox::is_valid(m_g_box_));
    for (int i = 0; i < m_ndims_; ++i)
    {
        if (std::get<1>(m_g_box_)[i] <= std::get<0>(m_g_box_)[i] + 1)
        {
            m_ghost_width_[i] = 0;

            std::get<0>(m_m_box_)[i] = 0;
            std::get<1>(m_m_box_)[i] = 1;

            std::get<0>(m_m_box_)[i] = 0;
            std::get<1>(m_m_box_)[i] = 1;

            std::get<0>(m_inner_box_)[i] = 0;
            std::get<1>(m_inner_box_)[i] = 1;

            std::get<0>(m_outer_box_)[i] = 0;
            std::get<1>(m_outer_box_)[i] = 1;


            m_inv_dx_[i] = 0;

            m_l2g_scale_[i] = 0;
            m_l2g_shift_[i] = m_global_origin_[i];

            m_g2l_scale_[i] = 0;
            m_g2l_shift_[i] = 0;

//            if (i < mpi_topo_ndims && mpi_topo_dims[i] > 1)
//            {
//                RUNTIME_ERROR << " Mesh is not splitable [" << m_g_box_
//                              << ", mpi={" << mpi_topo_dims[0]
//                              << "," << mpi_topo_dims[1]
//                              << "," << mpi_topo_dims[2]
//                              << "}]" << std::endl;
//            }

        } else //if (i < mpi_topo_ndims && mpi_topo_dims[i] > 1)
        {
//            index_type L = std::get<1>(m_g_box_)[i] - std::get<0>(m_g_box_)[i];
//            std::get<1>(m_g_box_)[i] = std::get<0>(m_g_box_)[i] + L * (mpi_topo_coords[i] + 1) / mpi_topo_dims[i];
//            std::get<0>(m_g_box_)[i] += L * mpi_topo_coords[i] / mpi_topo_dims[i];

            m_inv_dx_[i] = static_cast<Real>(1.0) / m_dx_[i];

            m_l2g_scale_[i] = m_dx_[i];
            m_l2g_shift_[i] = m_global_origin_[i];

            m_g2l_scale_[i] = m_inv_dx_[i];
            m_g2l_shift_[i] = -(m_global_origin_[i]) * m_g2l_scale_[i];

        }
    }


    m_inner_box_ = m_g_box_;
    m_outer_box_ = m_g_box_;

    std::get<0>(m_outer_box_) -= m_ghost_width_;
    std::get<1>(m_outer_box_) += m_ghost_width_;

    m_m_box_ = m_outer_box_;
    m_is_deployed_ = true;


}


MeshBlock::MeshBlock(MeshBlock const &other) :
        m_is_deployed_/*    */(other.m_is_deployed_),
        m_space_id_/*       */(other.m_space_id_),
        m_level_/*          */(other.m_level_),
        m_ghost_width_/*    */(other.m_ghost_width_),
        m_g_box_/*          */(other.m_g_box_/*  */),
        m_m_box_/*          */(other.m_m_box_/*  */),
        m_inner_box_/*      */(other.m_inner_box_/*  */),
        m_outer_box_/*      */(other.m_outer_box_/*  */),
        m_dx_/*             */(other.m_dx_/*  */),
        m_global_origin_/*  */(other.m_global_origin_/*  */),
        m_l2g_scale_/*      */(other.m_l2g_scale_),
        m_l2g_shift_/*      */(other.m_l2g_shift_),
        m_g2l_scale_/*      */(other.m_g2l_scale_),
        m_g2l_shift_/*      */(other.m_g2l_shift_) {};

MeshBlock::MeshBlock(MeshBlock &&other) :
        m_is_deployed_/*    */(other.m_is_deployed_),
        m_space_id_/*       */(other.m_space_id_),
        m_level_/*          */(other.m_level_),
        m_ghost_width_/*    */(other.m_ghost_width_),
        m_g_box_/*          */(other.m_g_box_/*  */),
        m_m_box_/*          */(other.m_m_box_/*  */),
        m_inner_box_/*      */(other.m_inner_box_/*  */),
        m_outer_box_/*      */(other.m_outer_box_/*  */),
        m_dx_/*             */(other.m_dx_/*  */),
        m_global_origin_/*  */(other.m_global_origin_/*  */),
        m_l2g_scale_/*      */(other.m_l2g_scale_),
        m_l2g_shift_/*      */(other.m_l2g_shift_),
        m_g2l_scale_/*      */(other.m_g2l_scale_),
        m_g2l_shift_/*      */(other.m_g2l_shift_) {};

MeshBlock::~MeshBlock() {}

std::ostream &MeshBlock::print(std::ostream &os, int indent) const
{

    os << std::setw(indent + 1) << "type = \"" << get_class_name() << "\" ,"
       << " level = " << level() << ",  box = " << m_g_box_;

//#ifndef NDEBUG
//    os
//            << std::setw(indent + 1) << " " << "      lower = " << m_lower_ << "," << std::endl
//            << std::setw(indent + 1) << " " << "      upper = " << m_upper_ << "," << std::endl
//            << std::setw(indent + 1) << " " << "outer lower = " << m_outer_lower_ << "," << std::endl
//            << std::setw(indent + 1) << " " << "outer upper = " << m_outer_upper_ << "," << std::endl
//            << std::setw(indent + 1) << " " << "inner lower = " << m_inner_lower_ << "," << std::endl
//            << std::setw(indent + 1) << " " << "inner upper = " << m_inner_upper_ << "," << std::endl
//            << std::endl;
//#endif
    return os;
}


void MeshBlock::deploy() {}

//std::shared_ptr<MeshBlock>
//MeshBlock::clone() const
//{
//    assert(is_deployed());
//    auto res = std::make_shared<MeshBlock>();
//
//    res->m_is_deployed_/*    */= m_is_deployed_;
//    res->m_space_id_/*       */= m_space_id_;
//    res->m_level_/*          */= m_level_;
//    res->m_ghost_width_/*    */= m_ghost_width_;
//    res->m_g_box_/*          */= m_g_box_/*  */;
//    res->m_m_box_/*          */= m_m_box_/*  */;
//    res->m_inner_box_/*      */= m_inner_box_/*  */;
//    res->m_outer_box_/*      */= m_outer_box_;
//    return res;
//};

std::shared_ptr<MeshBlock>
MeshBlock::create(int inc_level, const index_type *lo, const index_type *hi) const
{
    auto res = std::make_shared<MeshBlock>();
    if (inc_level >= 0)
    {
        std::get<0>(res->m_g_box_)[0] = lo[0] << inc_level;
        std::get<0>(res->m_g_box_)[1] = lo[1] << inc_level;
        std::get<0>(res->m_g_box_)[2] = lo[2] << inc_level;
        std::get<1>(res->m_g_box_)[0] = hi[0] << inc_level;
        std::get<1>(res->m_g_box_)[1] = hi[1] << inc_level;
        std::get<1>(res->m_g_box_)[2] = hi[2] << inc_level;
    } else if (inc_level < 0)
    {
        std::get<0>(res->m_g_box_)[0] = lo[0] >> -inc_level;
        std::get<0>(res->m_g_box_)[1] = lo[1] >> -inc_level;
        std::get<0>(res->m_g_box_)[2] = lo[2] >> -inc_level;
        std::get<1>(res->m_g_box_)[0] = hi[0] >> -inc_level;
        std::get<1>(res->m_g_box_)[1] = hi[1] >> -inc_level;
        std::get<1>(res->m_g_box_)[2] = hi[2] >> -inc_level;
    }
    res->m_level_ += inc_level;
    res->deploy();
    return res;

}

std::shared_ptr<MeshBlock>
MeshBlock::intersection(index_box_type const &other_box, int inc_level)
{
    return create(inc_level, toolbox::intersection(m_inner_box_, other_box));
}




//
///**
// * return the minimum block that contain two blocks
// */
//std::shared_ptr<MeshBlock>
//MeshBlock::union_bounding(const std::shared_ptr<MeshBlock> &other) const
//{
//    assert(is_deployed());
//    auto res = clone();
//
//    res->m_inner_box_ = toolbox::union_bounding(m_inner_box_, other->m_inner_box_);
//
//    if (!toolbox::is_valid(res->m_inner_box_)) { res = nullptr; }
//    else
//    {
//        std::get<0>(res->m_outer_box_) = std::get<0>(res->m_inner_box_) - res->m_ghost_width_;
//        std::get<1>(res->m_outer_box_) = std::get<1>(res->m_inner_box_) + res->m_ghost_width_;
//    }
//    return res;
//
//}

//
//void MeshBlock::foreach(std::function<void(index_type, index_type, index_type)> const &fun) const
//{
//
//#pragma omp parallel for
//    for (index_type i = 0; i < m_inner_count_[0]; ++i)
//        for (index_type j = 0; j < m_inner_count_[1]; ++j)
//            for (index_type k = 0; k < m_inner_count_[2]; ++k)
//            {
//                fun(m_inner_start_[0] + i, m_inner_start_[1] + j, m_inner_start_[2] + k);
//            }
//
//
//}
//
//void MeshBlock::foreach(std::function<void(index_type)> const &fun) const
//{
//#pragma omp parallel for
//    for (index_type i = 0; i < m_inner_count_[0]; ++i)
//        for (index_type j = 0; j < m_inner_count_[1]; ++j)
//            for (index_type k = 0; k < m_inner_count_[2]; ++k)
//            {
//                fun(hash(m_inner_start_[0] + i, m_inner_start_[1] + j, m_inner_start_[2] + k));
//            }
//}
//
//void MeshBlock::foreach(int iform, std::function<void(MeshEntityId const &)> const &fun) const
//{
//    int n = (iform == VERTEX || iform == VOLUME) ? 1 : 3;
//#pragma omp parallel for
//    for (index_type i = 0; i < m_inner_count_[0]; ++i)
//        for (index_type j = 0; j < m_inner_count_[1]; ++j)
//            for (index_type k = 0; k < m_inner_count_[2]; ++k)
//                for (int l = 0; l < n; ++l)
//                {
//                    fun(pack(m_inner_start_[0] + i, m_inner_start_[1] + j, m_inner_start_[2] + k, l));
//                }
//}

//
//std::tuple<data_block::DataSpace, data_block::DataSpace>
//MeshBlock::data_space(MeshEntityType const &t, MeshZoneTag status) const
//{
//    int i_ndims = (t == EDGE || t == FACE) ? (NDIMS + 1) : NDIMS;
//
//    nTuple<size_type, NDIMS + 1> f_dims, f_count;
//    nTuple<size_type, NDIMS + 1> f_start;
//
//    nTuple<size_type, NDIMS + 1> m_dims, m_count;
//    nTuple<size_type, NDIMS + 1> m_start;
//
//    switch (status)
//    {
//        case SP_ES_ALL:
//            f_dims = toolbox::dimensions(m_g_box_);
//            f_start = std::get<0>(m_g_box_);
//            f_count = toolbox::dimensions(m_g_box_);
//
//            m_dims = toolbox::dimensions(m_m_box_);
//            m_start = std::get<0>(m_outer_box_) - std::get<0>(m_m_box_);
//            m_count = toolbox::dimensions(m_outer_box_);;
//            break;
//        case SP_ES_OWNED:
//        default:
//            f_dims = toolbox::dimensions(m_g_box_);;
//            f_start = std::get<0>(m_g_box_);;
//            f_count = toolbox::dimensions(m_inner_box_);
//
//            m_dims = toolbox::dimensions(m_m_box_);;
//            m_start = std::get<0>(m_inner_box_) - std::get<0>(m_m_box_);
//            m_count = toolbox::dimensions(m_inner_box_);
//            break;
//
//    }
//    f_dims[NDIMS] = 3;
//    f_start[NDIMS] = 0;
//    f_count[NDIMS] = 3;
//
//
//    m_dims[NDIMS] = 3;
//    m_start[NDIMS] = 0;
//    m_count[NDIMS] = 3;
//
//    FIXME;
//    data_block::DataSpace f_space(i_ndims, &f_dims[0]);
////    f_space.select_hyperslab(&f_start[0], nullptr, &f_count[0], nullptr);
//
//
//    data_block::DataSpace m_space(i_ndims, &m_dims[0]);
//    m_space.select_hyperslab(&m_start[0], nullptr, &m_count[0], nullptr);
//
//    return std::forward_as_tuple(m_space, f_space);
//
//};


EntityIdRange
MeshBlock::range(MeshEntityType entityType, index_box_type const &b, index_type dof) const
{
    EntityIdRange res;
    res.append(MeshEntityIdCoder::make_range(std::get<0>(b), std::get<1>(b), entityType));
    return res;
}

EntityIdRange
MeshBlock::range(MeshEntityType entityType, box_type const &b, index_type dof) const
{
    index_tuple l, u;
    l = point_to_index(std::get<1>(b));
    u = point_to_index(std::get<1>(b)) + 1;
    return range(entityType, std::make_tuple(l, u), 0);
}

EntityIdRange
MeshBlock::range(MeshEntityType entityType, MeshZoneTag status, index_type dof) const
{
    EntityIdRange res;

    /**
     *   |<-----------------------------     valid   --------------------------------->|
     *   |<- not owned  ->|<-------------------       owned     ---------------------->|
     *   |----------------*----------------*---*---------------------------------------|
     *   |<---- ghost --->|                |   |                                       |
     *   |<------------ shared  ---------->|<--+--------  not shared  ---------------->|
     *   |<------------- DMZ    -------------->|<----------   not DMZ   -------------->|
     *
     */

    index_tuple m_outer_lower_, m_outer_upper_, m_inner_lower_, m_inner_upper_;
    std::tie(m_outer_lower_, m_outer_upper_) = m_outer_box_;
    std::tie(m_inner_lower_, m_inner_upper_) = m_inner_box_;
    size_tuple m_g_dimensions_;
    m_g_dimensions_ = toolbox::dimensions(m_g_box_);
    switch (status)
    {
        case SP_ES_ALL : //all valid
            res.append(MeshEntityIdCoder::make_range(m_outer_lower_, m_outer_upper_, entityType));
            break;
        case SP_ES_OWNED:
            res.append(MeshEntityIdCoder::make_range(m_inner_lower_, m_inner_upper_, entityType));
            break;
        case SP_ES_NON_LOCAL : // = SP_ES_SHARED | SP_ES_OWNED, //              0b000101
        case SP_ES_SHARED : //       = 0x04,                    0b000100 shared by two or more get_mesh grid_dims
            break;
        case SP_ES_NOT_SHARED  : // = 0x08, //                       0b001000 not shared by other get_mesh grid_dims
            break;
        case SP_ES_GHOST : // = SP_ES_SHARED | SP_ES_NOT_OWNED, //              0b000110
            if (m_g_dimensions_[0] > 1)
            {
                res.append(
                        MeshEntityIdCoder::make_range(
                                index_tuple{m_outer_lower_[0], m_outer_lower_[1], m_outer_lower_[2]},
                                index_tuple{m_inner_lower_[0], m_outer_upper_[1], m_outer_upper_[2]}, entityType));
                res.append(
                        MeshEntityIdCoder::make_range(
                                index_tuple{m_inner_upper_[0], m_outer_lower_[1], m_outer_lower_[2]},
                                index_tuple{m_outer_upper_[0], m_outer_upper_[1], m_outer_upper_[2]}, entityType));
            }
            if (m_g_dimensions_[1] > 1)
            {
                res.append(
                        MeshEntityIdCoder::make_range(
                                index_tuple{m_inner_lower_[0], m_outer_lower_[1], m_outer_lower_[2]},
                                index_tuple{m_inner_upper_[0], m_inner_lower_[1], m_outer_upper_[2]}, entityType));
                res.append(
                        MeshEntityIdCoder::make_range(
                                index_tuple{m_inner_lower_[0], m_inner_upper_[1], m_outer_lower_[2]},
                                index_tuple{m_inner_upper_[0], m_outer_upper_[1], m_outer_upper_[2]}, entityType));
            }
            if (m_g_dimensions_[2] > 1)
            {
                res.append(
                        MeshEntityIdCoder::make_range(
                                index_tuple{m_inner_lower_[0], m_inner_lower_[1], m_outer_lower_[2]},
                                index_tuple{m_inner_upper_[0], m_inner_upper_[1], m_inner_lower_[2]}, entityType));
                res.append(
                        MeshEntityIdCoder::make_range(
                                index_tuple{m_inner_lower_[0], m_inner_lower_[1], m_inner_upper_[2]},
                                index_tuple{m_inner_upper_[0], m_inner_upper_[1], m_outer_upper_[2]}, entityType));
            }
            break;
        case SP_ES_DMZ: //  = 0x100,
        case SP_ES_NOT_DMZ: //  = 0x200,
        case SP_ES_LOCAL : // = SP_ES_NOT_SHARED | SP_ES_OWNED, //              0b001001
            res.append(MeshEntityIdCoder::make_range(m_inner_lower_, m_inner_upper_, entityType));
            break;
        case SP_ES_VALID:
            index_tuple l, u;
            l = m_outer_lower_;
            u = m_outer_upper_;
            for (int i = 0; i < 3; ++i)
            {
                if (m_g_dimensions_[i] > 1 && m_ghost_width_[i] != 0)
                {
                    l[i] += 1;
                    u[i] -= 1;
                }
            }
            res.append(MeshEntityIdCoder::make_range(l, u, entityType));
            break;

//        case SP_ES_INTERFACE: //  = 0x010, //                        0b010000 interface(boundary) shared by two get_mesh grid_dims,
//            res.append(m_interface_entities_[entityType]);
            break;
        default:
            UNIMPLEMENTED;
            break;
    }
    return std::move(res);
};


}}