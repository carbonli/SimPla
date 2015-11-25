/**
 * @file block.h
 * @author salmon
 * @date 2015-10-28.
 */

#ifndef SIMPLA_BLOCK_H
#define SIMPLA_BLOCK_H

#include "../../gtl/macro.h"
#include "../../gtl/primitives.h"
#include "../../gtl/ntuple.h"
#include "../../gtl/type_traits.h"
#include "../../gtl/utilities/log.h"
#include "mesh_ids.h"

namespace simpla { namespace mesh
{

struct MeshBlock : public MeshIDs_<4>
{
    static constexpr int ndims = 3;
    enum
    {
        DEFAULT_GHOST_WIDTH = 2
    };
private:

    typedef MeshBlock this_type;
    typedef MeshIDs_<4> m;

public:
    using typename m::id_type;
    using typename m::id_tuple;
    using typename m::index_type;
    using typename m::range_type;
    using typename m::iterator;
    using typename m::index_tuple;
    using typename m::difference_type;

    typedef nTuple<Real, ndims> point_type;
    typedef nTuple<Real, ndims> vector_type;


    /**
 *
 *   -----------------------------5
 *   |                            |
 *   |     ---------------4       |
 *   |     |              |       |
 *   |     |  ********3   |       |
 *   |     |  *       *   |       |
 *   |     |  *       *   |       |
 *   |     |  *       *   |       |
 *   |     |  2********   |       |
 *   |     1---------------       |
 *   0-----------------------------
 *
 *	5-0 = dimensions
 *	4-1 = e-d = ghosts
 *	2-1 = counts
 *
 *	0 = m_min_
 *	5 = m_max_
 *
 *	1 = m_memory_min_
 *	4 = m_memory_max_
 *
 *	2 = m_local_min_
 *	3 = m_local_max_
 *
 *
 */
    index_tuple m_min_;
    index_tuple m_max_;
    index_tuple m_local_min_;
    index_tuple m_local_max_;
    index_tuple m_memory_min_;
    index_tuple m_memory_max_;


    bool m_is_valid_ = false;
public:

    MeshBlock()
    {
        m_min_ = 0;
        m_max_ = 0;
        m_local_min_ = m_min_;
        m_local_max_ = m_max_;
        m_memory_min_ = m_min_;
        m_memory_max_ = m_max_;
    }


    MeshBlock(this_type const &other) :

            m_min_(other.m_min_),

            m_max_(other.m_max_),

            m_local_min_(other.m_local_min_),

            m_local_max_(other.m_local_max_),

            m_memory_min_(other.m_memory_min_),

            m_memory_max_(other.m_memory_max_)
    {

    }

    virtual  ~MeshBlock() { }

    virtual void swap(this_type &other)
    {
        std::swap(m_min_, other.m_min_);
        std::swap(m_max_, other.m_max_);
        std::swap(m_local_min_, other.m_local_min_);
        std::swap(m_local_max_, other.m_local_max_);
        std::swap(m_memory_min_, other.m_memory_min_);
        std::swap(m_memory_max_, other.m_memory_max_);
        deploy();
        other.deploy();

    }

    this_type &operator=(this_type const &other)
    {
        this_type(other).swap(*this);
        return *this;
    }


    template<typename TD>
    void dimensions(TD const &d)
    {
        m_max_ = d;
        m_min_ = 0;
        m_local_max_ = m_max_;
        m_local_min_ = m_min_;
        m_memory_max_ = m_max_;
        m_memory_min_ = m_min_;
    }

    index_tuple dimensions() const
    {
        index_tuple res;

        res = m_max_ - m_min_;

        return std::move(res);
    }

    virtual std::tuple<point_type, point_type> box() const
    {
        return std::make_tuple(m::point(m_min_), m::point(m_max_));
    };

    virtual std::tuple<point_type, point_type> local_box() const
    {
        return std::make_tuple(m::point(m_local_min_), m::point(m_local_max_));
    };

    template<typename T0, typename T1>
    void index_box(T0 const &min, T1 const &max)
    {
        m_min_ = min;
        m_max_ = max;
    };


    auto index_box() const
    DECL_RET_TYPE((std::forward_as_tuple(m_min_, m_max_)))

    auto local_index_box() const
    DECL_RET_TYPE((std::forward_as_tuple(m_local_min_, m_local_max_)))

    auto memory_index_box() const
    DECL_RET_TYPE((std::forward_as_tuple(m_memory_min_, m_memory_max_)))

    bool in_box(index_tuple const &x) const
    {
        return (m_local_min_[1] <= x[1]) && (m_local_min_[2] <= x[2]) && (m_local_min_[0] <= x[0])  //
               && (m_local_max_[1] > x[1]) && (m_local_max_[2] > x[2]) && (m_local_max_[0] > x[0]);

    }

    bool in_box(id_type s) const { return in_box(m::unpack_index(s)); }

    template<int I>
    range_type range() const { return m::template make_range<I>(m_local_min_, m_local_max_); }


    template<int IFORM>
    auto max_hash() const
    DECL_RET_TYPE((m::hash(m::pack_index(m_memory_max_ - 1, m::template sub_index_to_id<IFORM>(3UL)),
                           m_memory_min_, m_memory_max_)))


    size_t hash(id_type const &s) const { return static_cast<size_t>(m::hash(s, m_memory_min_, m_memory_max_)); }


    void decompose(index_tuple const &dist_dimensions, index_tuple const &dist_coord,
                   index_type gw = DEFAULT_GHOST_WIDTH)
    {


        index_tuple b, e;
        b = m_local_min_;
        e = m_local_max_;
        for (int n = 0; n < ndims; ++n)
        {

            m_local_min_[n] = b[n] + (e[n] - b[n]) * dist_coord[n] / dist_dimensions[n];

            m_local_max_[n] = b[n] + (e[n] - b[n]) * (dist_coord[n] + 1) / dist_dimensions[n];


            if (m_local_min_[n] == m_local_max_[n])
            {
                THROW_EXCEPTION_RUNTIME_ERROR("Mesh block decompose failed! Block dimension is smaller than process grid. ");
            }


            if (m_local_max_[n] - m_local_min_[n] > 1 && dist_dimensions[n] > 1)
            {
                m_memory_min_[n] = m_local_min_[n] - gw;
                m_memory_max_[n] = m_local_max_[n] + gw;
            }
        }


    }

    virtual void deploy()
    {
        m_is_valid_ = true;
        m_local_min_ = m_min_;
        m_local_max_ = m_max_;
        m_memory_min_ = m_min_;
        m_memory_max_ = m_max_;

    }

private:

    template<typename T0, typename T1>
    index_type dist_to_box_(id_type const &s, T0 const &imin, T1 const &imax) const
    {
        auto idx = this->unpack_index(s);

        index_type res = std::numeric_limits<index_type>::max();

        for (int i = 0; i < 3; ++i)
        {
            if (imax[i] - imin[i] <= 1)
            {

                continue;
            }
            else
            {
                res = std::min(res, std::min(idx[i] - imin[i], imax[i] - idx[i]));

            }
        }

        return res;


    }

public:

    index_type idx_to_local_boundary(id_type const &s) const
    {
        return dist_to_box_(s, m_local_min_, m_local_max_);
    }

    index_type idx_to_boundary(id_type const &s) const
    {
        return dist_to_box_(s, m_min_, m_max_);

    }


}; // struct MeshBlock
}//namespace mesh
}//namespace simpla
#endif //SIMPLA_BLOCK_H
