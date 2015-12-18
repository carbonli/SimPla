/**
 * @file linear.h
 * @author salmon
 * @date 2015-10-14.
 */

#ifndef SIMPLA_LINEAR_H
#define SIMPLA_LINEAR_H

#include "../../gtl/ntuple.h"

namespace simpla { namespace manifold { namespace policy
{


#define DECLARE_FUNCTION_PREFIX inline static
#define DECLARE_FUNCTION_SUFFIX /*const*/


/**
 * @ingroup interpolate
 * @brief basic linear interpolate
 */
template<typename TMesh>
struct LinearInterpolator
{
private:

    typedef TMesh mesh_type;

    typedef typename TMesh::id_type id_t;

    typedef LinearInterpolator<mesh_type> this_type;

public:

    typedef LinearInterpolator<mesh_type> interpolate_policy;

private:

    template<typename TD, typename TIDX>
    DECLARE_FUNCTION_PREFIX auto gather_impl_(mesh_type const &m, TD const &f,
                                              TIDX const &idx) DECLARE_FUNCTION_SUFFIX -> decltype(
    traits::index(f, std::get<0>(idx)) *
    std::get<1>(idx)[0])
    {

        auto X = (mesh_type::_DI) << 1;
        auto Y = (mesh_type::_DJ) << 1;
        auto Z = (mesh_type::_DK) << 1;

        typename mesh_type::point_type r = std::get<1>(idx);
        typename mesh_type::index_type s = std::get<0>(idx);

        return traits::index(f, ((s + X) + Y) + Z) * (r[0]) * (r[1]) * (r[2]) //
               + traits::index(f, (s + X) + Y) * (r[0]) * (r[1]) * (1.0 - r[2]) //
               + traits::index(f, (s + X) + Z) * (r[0]) * (1.0 - r[1]) * (r[2]) //
               + traits::index(f, (s + X)) * (r[0]) * (1.0 - r[1]) * (1.0 - r[2]) //
               + traits::index(f, (s + Y) + Z) * (1.0 - r[0]) * (r[1]) * (r[2]) //
               + traits::index(f, (s + Y)) * (1.0 - r[0]) * (r[1]) * (1.0 - r[2]) //
               + traits::index(f, s + Z) * (1.0 - r[0]) * (1.0 - r[1]) * (r[2]) //
               + traits::index(f, s) * (1.0 - r[0]) * (1.0 - r[1]) * (1.0 - r[2]);
    }

public:

    template<typename TF, typename TX>
    DECLARE_FUNCTION_PREFIX auto gather(mesh_type const &m, TF const &f,
                                        TX const &r) DECLARE_FUNCTION_SUFFIX//
    ENABLE_IF_DECL_RET_TYPE((traits::iform<TF>::value
                             == VERTEX), (gather_impl_(m, f, m.coordinates_global_to_local(r, 0))))

    template<typename TF>
    DECLARE_FUNCTION_PREFIX auto gather(mesh_type const &m, TF const &f,
                                        typename mesh_type::point_type const &r) DECLARE_FUNCTION_SUFFIX
    ENABLE_IF_DECL_RET_TYPE((traits::iform<TF>::value
                             == EDGE),
                            make_nTuple(
                                    gather_impl_(m, f, m.coordinates_global_to_local(r, 1)),
                                    gather_impl_(m, f, m.coordinates_global_to_local(r, 2)),
                                    gather_impl_(m, f, m.coordinates_global_to_local(r, 4))
                            ))

    template<typename TF>
    DECLARE_FUNCTION_PREFIX auto gather(mesh_type const &m, TF const &f,
                                        typename mesh_type::point_type const &r) DECLARE_FUNCTION_SUFFIX
    ENABLE_IF_DECL_RET_TYPE((traits::iform<TF>::value
                             == FACE),
                            make_nTuple(
                                    gather_impl_(m, f, m.coordinates_global_to_local(r, 6)),
                                    gather_impl_(m, f, m.coordinates_global_to_local(r, 5)),
                                    gather_impl_(m, f, m.coordinates_global_to_local(r, 3))
                            ))

    template<typename TF>
    DECLARE_FUNCTION_PREFIX auto gather(mesh_type const &m, TF const &f,
                                        typename mesh_type::point_type const &x) DECLARE_FUNCTION_SUFFIX
    ENABLE_IF_DECL_RET_TYPE((traits::iform<TF>::value == VOLUME),
                            gather_impl_(m, f, m.coordinates_global_to_local(x, 7)))

private:
    template<typename TF, typename IDX, typename TV>
    DECLARE_FUNCTION_PREFIX void scatter_impl_(mesh_type const &m, TF &f, IDX const &idx,
                                               TV const &v) DECLARE_FUNCTION_SUFFIX
    {

        auto X = (mesh_type::_DI) << 1;
        auto Y = (mesh_type::_DJ) << 1;
        auto Z = (mesh_type::_DK) << 1;

        typename mesh_type::point_type r = std::get<1>(idx);
        typename mesh_type::index_type s = std::get<0>(idx);

        traits::index(f, ((s + X) + Y) + Z) += v * (r[0]) * (r[1]) * (r[2]);
        traits::index(f, (s + X) + Y) += v * (r[0]) * (r[1]) * (1.0 - r[2]);
        traits::index(f, (s + X) + Z) += v * (r[0]) * (1.0 - r[1]) * (r[2]);
        traits::index(f, s + X) += v * (r[0]) * (1.0 - r[1]) * (1.0 - r[2]);
        traits::index(f, (s + Y) + Z) += v * (1.0 - r[0]) * (r[1]) * (r[2]);
        traits::index(f, s + Y) += v * (1.0 - r[0]) * (r[1]) * (1.0 - r[2]);
        traits::index(f, s + Z) += v * (1.0 - r[0]) * (1.0 - r[1]) * (r[2]);
        traits::index(f, s) += v * (1.0 - r[0]) * (1.0 - r[1]) * (1.0 - r[2]);

    }


    template<typename TF, typename TX, typename TV>
    DECLARE_FUNCTION_PREFIX void scatter_(mesh_type const &m, std::integral_constant<int, VERTEX>, TF &
    f, TX const &x, TV const &u) DECLARE_FUNCTION_SUFFIX
    {
        scatter_impl_(m, f, m.coordinates_global_to_local(x, 0), u);
    }

    template<typename TF, typename TX, typename TV>
    DECLARE_FUNCTION_PREFIX void scatter_(mesh_type const &m, std::integral_constant<int, EDGE>, TF &
    f, TX const &x, TV const &u) DECLARE_FUNCTION_SUFFIX
    {

        scatter_impl_(m, f, m.coordinates_global_to_local(x, 1), u[0]);
        scatter_impl_(m, f, m.coordinates_global_to_local(x, 2), u[1]);
        scatter_impl_(m, f, m.coordinates_global_to_local(x, 4), u[2]);

    }

    template<typename TF, typename TX, typename TV>
    DECLARE_FUNCTION_PREFIX void scatter_(mesh_type const &m, std::integral_constant<int, FACE>, TF &f,
                                          TX const &x, TV const &u) DECLARE_FUNCTION_SUFFIX
    {

        scatter_impl_(m, f, m.coordinates_global_to_local(x, 6), u[0]);
        scatter_impl_(m, f, m.coordinates_global_to_local(x, 5), u[1]);
        scatter_impl_(m, f, m.coordinates_global_to_local(x, 3), u[2]);
    }

    template<typename TF, typename TX, typename TV>
    DECLARE_FUNCTION_PREFIX void scatter_(mesh_type const &m, std::integral_constant<int, VOLUME>,
                                          TF &f, TX const &x, TV const &u) DECLARE_FUNCTION_SUFFIX
    {
        scatter_impl_(m, f, m.coordinates_global_to_local(x, 7), u);
    }

public:
    template<typename TF, typename ...Args>
    DECLARE_FUNCTION_PREFIX void scatter(mesh_type const &m, TF &f, Args &&...args) DECLARE_FUNCTION_SUFFIX
    {
        scatter_(m, traits::iform<TF>(), f, std::forward<Args>(args)...);
    }

private:
    template<typename TI, typename TV>
    DECLARE_FUNCTION_PREFIX TV sample_(mesh_type const &m, std::integral_constant<int, VERTEX>, TI s,
                                       TV const &v) DECLARE_FUNCTION_SUFFIX { return v; }

    template<typename TI, typename TV>
    DECLARE_FUNCTION_PREFIX TV sample_(mesh_type const &m, std::integral_constant<int, VOLUME>, TI s,
                                       TV const &v) DECLARE_FUNCTION_SUFFIX { return v; }

    template<typename TI, typename TV>
    DECLARE_FUNCTION_PREFIX TV sample_(mesh_type const &m, std::integral_constant<int, EDGE>,
                                       TI s, nTuple<TV, 3> const &v) DECLARE_FUNCTION_SUFFIX
    {
        return v[mesh_type::sub_index(s)];
    }

    template<typename TI, typename TV>
    DECLARE_FUNCTION_PREFIX TV sample_(mesh_type const &m, std::integral_constant<int, FACE>,
                                       TI s, nTuple<TV, 3> const &v) DECLARE_FUNCTION_SUFFIX
    {
        return v[mesh_type::sub_index(s)];
    }
//
//    template<int IFORM, typename TI, typename TV>
//    DECLARE_FUNCTION_PREFIX TV sample_(mesh_type const & m,std::integral_constant<int, IFORM>, TI s,
//                                       TV const &v) DECLARE_FUNCTION_SUFFIX { return v; }

public:

//    template<int IFORM, typename TI, typename TV>
//    DECLARE_FUNCTION_PREFIX auto generator(TI const &s, TV const &v) DECLARE_FUNCTION_SUFFIX
//    DECL_RET_TYPE((sample_(mesh_type const & m,std::integral_constant<int, IFORM>(), s, v)))


    template<int IFORM, typename TI, typename TV>
    DECLARE_FUNCTION_PREFIX typename traits::value_type<TV>::type
    sample(mesh_type const &m, TI const &s, TV const &v) DECLARE_FUNCTION_SUFFIX
    {
        return sample_(m, std::integral_constant<int, IFORM>(), s, v);
    }


public:
    typedef typename mesh_type::point_type point_type;
    typedef typename mesh_type::vector_type vector_type;

    LinearInterpolator() { }

    virtual ~LinearInterpolator() { }

    /**
     * A radial basis function (RBF) is a real-valued function whose value depends only
     * on the distance from the origin, so that \f$\phi(\mathbf{x}) = \phi(\|\mathbf{x}\|)\f$;
     * or alternatively on the distance from some other point c, called a center, so that
     * \f$\phi(\mathbf{x}, \mathbf{c}) = \phi(\|\mathbf{x}-\mathbf{c}\|)\f$.
     */
    Real RBF(mesh_type const &m, point_type const &x0, point_type const &x1, vector_type const &a) const
    {
        vector_type r;
        r = (x1 - x0) / a;
        // @NOTE this is not  an exact  RBF
        return (1.0 - std::abs(r[0])) * (1.0 - std::abs(r[1])) * (1.0 - std::abs(r[2]));
    }

    Real RBF(mesh_type const &m, point_type const &x0, point_type const &x1, Real const &a) const
    {

        return (1.0 - m.distance(x1, x0) / a);
    }

};

}}}//namespace simpla
#endif //SIMPLA_LINEAR_H