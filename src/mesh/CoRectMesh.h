/**
 *
 * @file corectmesh.h
 * Created by salmon on 15-7-2.
 *
 */

#ifndef SIMPLA_CORECTMESH_H
#define SIMPLA_CORECTMESH_H

#include <vector>
#include <iomanip>

#include "../gtl/macro.h"
#include "../gtl/primitives.h"
#include "../gtl/nTuple.h"
#include "../gtl/nTupleExt.h"
#include "../gtl/PrettyStream.h"
#include "../gtl/type_traits.h"


#include "Mesh.h"
#include "MeshBase.h"
#include "MeshEntityIdCoder.h"

namespace simpla { namespace mesh
{
namespace tags { struct CoRectLinear; }

template<typename ...> class Mesh;

typedef Mesh<tags::CoRectLinear> CoRectMesh;


/**
 * @ingroup mesh
 *
 * @brief Uniform structured mesh
 */
template<>
struct Mesh<tags::CoRectLinear> : public MeshBase, public MeshEntityIdCoder
{
private:
    typedef Mesh<tags::CoRectLinear> this_type;
    typedef MeshBase base_type;
public:
    virtual bool is_a(std::type_info const &info) const { return typeid(this_type) == info || base_type::is_a(info); }

    template<typename _UOTHER_> bool is_a() const { return is_a(typeid(_UOTHER_)); }

    virtual std::string get_class_name() const { return class_name(); }

    static std::string class_name() { return std::string("Mesh<tags::CoRectLinear>"); }

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
 *	0 = id_begin
 *	5 = id_end
 *
 *	1 = id_local_outer_begin
 *	4 = id_local_outer_end
 *
 *	2 = id_local_inner_begin
 *	3 = id_local_inner_end
 *
 *
 */


    point_type m_coords_lower_{{0, 0, 0}};

    point_type m_coords_upper_{{1, 1, 1}};

    vector_type m_dx_{{1, 1, 1}}; //!< width of cell, except m_dx_[i]=0 when m_dims_[i]==1

    index_tuple m_dims_{{10, 10, 10}}, m_shape_{{10, 10, 10}};

    index_tuple m_lower_{{0, 0, 0}}, m_upper_{{10, 10, 10}};

    typedef MeshEntityIdCoder m;

    typedef MeshEntityId id_type;

public:
    static constexpr int ndims = 3;

    Mesh() { }

    Mesh(this_type const &other) = delete;

    virtual  ~Mesh() { }

    void deploy() { }

    virtual std::ostream &print(std::ostream &os, int indent = 1) const
    {

        os
        << std::setw(indent) << " "
        << "Topology = { Type = \"CoRectMesh\", "
        << "Dimensions = " << dims() << " }," << std::endl
        << std::setw(indent) << " " << "Box = " << box() << "," << std::endl;

        return os;
    }


    virtual box_type box() const { return box_type{m_coords_lower_, m_coords_upper_}; }

    virtual MeshEntityRange range(MeshEntityType entityType = VERTEX) const
    {
        MeshEntityRange res(MeshEntityIdCoder::make_range(m_lower_, m_upper_, entityType));

        return std::move(res);
    };

    virtual size_t size(MeshEntityType entityType = VERTEX) const
    {
        return max_hash(entityType);
    }

    virtual size_t max_hash(MeshEntityType entityType = VERTEX) const
    {
        return m::max_hash(m_lower_, m_upper_, entityType);
    }

    virtual size_t hash(MeshEntityId const &s) const
    {
        return static_cast<size_t>(m::hash(s, m_lower_, m_upper_));
    }

    virtual point_type point(MeshEntityId const &s) const { return m::point(s); }

    virtual int get_adjacent_entities(MeshEntityId const &s, MeshEntityType entity_type,
                                      MeshEntityId *p = nullptr) const
    {
        return m::get_adjacent_entities(s, entity_type, entity_type, p);
    }

    virtual std::shared_ptr<MeshBase> refine(box_type const &b, int flag = 0) const
    {
        return std::shared_ptr<MeshBase>();
    }

//================================================================================================
    void dimensions(index_tuple const &dim) { m_dims_ = dim; }

    template<typename X0, typename X1>
    void box(X0 const &x0, X1 const &x1)
    {
        m_coords_lower_ = x0;
        m_coords_upper_ = x1;
    }


    void box(box_type const &b) { std::tie(m_coords_lower_, m_coords_upper_) = b; }


    vector_type const &dx() const { return m_dx_; }

    index_tuple dims() const { return m_upper_ - m_lower_; }


private:
    Real m_volume_[9];
    Real m_inv_volume_[9];
    Real m_dual_volume_[9];
    Real m_inv_dual_volume_[9];
public:


    virtual Real volume(id_type s) const { return m_volume_[m::node_id(s)]; }

    virtual Real dual_volume(id_type s) const { return m_dual_volume_[m::node_id(s)]; }

    virtual Real inv_volume(id_type s) const { return m_inv_volume_[m::node_id(s)]; }

    virtual Real inv_dual_volume(id_type s) const { return m_inv_dual_volume_[m::node_id(s)]; }

//    int get_vertices(int node_id, id_type s, point_type *p = nullptr) const
//    {
//
//        int num = m::get_adjacent_entities(VERTEX, node_id, s);
//
//        if (p != nullptr)
//        {
//            id_type neighbour[num];
//
//            m::get_adjacent_entities(VERTEX, node_id, s, neighbour);
//
//            for (int i = 0; i < num; ++i)
//            {
//                p[i] = point(neighbour[i]);
//            }
//
//        }
//
//
//        return num;
//    }

/**
 * @name  Coordinate map
 * @{
 *
 *        Topology mesh       geometry mesh
 *                        map
 *              M      ---------->      G
 *              x                       y
 **/
//private:
//
//
//    point_type m_map_orig_ = {0, 0, 0};
//
//    point_type m_map_scale_ = {1, 1, 1};
//
//    point_type m_inv_map_orig_ = {0, 0, 0};
//
//    point_type m_inv_map_scale_ = {1, 1, 1};
//
//
//    point_type inv_map(point_type const &x) const
//    {
//
//        point_type res;
//
//        res[0] = std::fma(x[0], m_inv_map_scale_[0], m_inv_map_orig_[0]);
//
//        res[1] = std::fma(x[1], m_inv_map_scale_[1], m_inv_map_orig_[1]);
//
//        res[2] = std::fma(x[2], m_inv_map_scale_[2], m_inv_map_orig_[2]);
//
//        return std::move(res);
//    }
//
//    point_type map(point_type const &y) const
//    {
//
//        point_type res;
//
//
//        res[0] = std::fma(y[0], m_map_scale_[0], m_map_orig_[0]);
//
//        res[1] = std::fma(y[1], m_map_scale_[1], m_map_orig_[1]);
//
//        res[2] = std::fma(y[2], m_map_scale_[2], m_map_orig_[2]);
//
//        return std::move(res);
//    }
//
//public:
//
//    virtual point_type point(id_type const &s) const { return std::move(map(m::point(s))); }
//
//    virtual point_type coordinates_local_to_global(id_type s, point_type const &x) const
//    {
//        return std::move(map(m::coordinates_local_to_global(s, x)));
//    }
//
//    virtual point_type coordinates_local_to_global(std::tuple<id_type, point_type> const &t) const
//    {
//        return std::move(map(m::coordinates_local_to_global(t)));
//    }
//
//    virtual std::tuple<id_type, point_type> coordinates_global_to_local(point_type const &x, int n_id = 0) const
//    {
//        return std::move(m::coordinates_global_to_local(inv_map(x), n_id));
//    }
//
//    virtual id_type id(point_type const &x, int n_id = 0) const
//    {
//        return std::get<0>(m::coordinates_global_to_local(inv_map(x), n_id));
//    }
//
//
//
//    std::tuple<index_tuple, index_tuple> index_box(std::tuple<point_type, point_type> const &b) const
//    {
//
//        point_type b0, b1, x0, x1;
//
//        std::tie(b0, b1) = local_index_box();
//
//        std::tie(x0, x1) = b;
//
//        if (geometry::box_intersection(b0, b1, &x0, &x1))
//        {
//            return std::make_tuple(m::unpack_index(id(x0)),
//                                   m::unpack_index(id(x1) + (m::_DA << 1)));
//
//        }
//        else
//        {
//            index_tuple i0, i1;
//            i0 = 0;
//            i1 = 0;
//            return std::make_tuple(i0, i1);
//        }
//
//    }


//    struct calculus_policy
//    {
//        template<typename ...Args> static double eval(Args &&...args) { return 1.0; }
//    };
}; // struct  Mesh
}} // namespace simpla // namespace mesh

#endif //SIMPLA_CORECTMESH_H
