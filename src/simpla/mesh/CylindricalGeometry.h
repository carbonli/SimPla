//
// Created by salmon on 16-10-9.
//

#ifndef SIMPLA_CYLINDRICALGEOMETRY_H
#define SIMPLA_CYLINDRICALGEOMETRY_H

#include <simpla/SIMPLA_config.h>
#include <simpla/algebra/all.h>
#include <simpla/data/all.h>
#include <simpla/engine/all.h>
#include <simpla/utilities/FancyStream.h>
#include <simpla/utilities/Log.h>
#include <simpla/utilities/macro.h>
#include <simpla/utilities/type_cast.h>
#include <simpla/utilities/type_traits.h>
#include <iomanip>
#include <vector>
#include "Mesh.h"
#include "SMesh.h"

namespace simpla {
namespace mesh {

using namespace simpla::data;

/**
 * @ingroup mesh
 * @brief Uniform structured get_mesh
 */

struct CylindricalSMesh : public SMesh {
    SP_OBJECT_HEAD(CylindricalSMesh, SMesh)
    unsigned int m_phi_axe_ = 2;

   public:
    typedef Real scalar_type;
    explicit CylindricalSMesh(Domain* d) : SMesh(d) {}
    ~CylindricalSMesh() override = default;

    DECLARE_REGISTER_NAME("CylindricalSMesh")

    template <typename V>
    using array_type = Array<V, NDIMS>;

   public:
    using SMesh::point;

    void InitializeData(Real time_now) override;

};  // struct  MeshBase

//    virtual point_type point(EntityId s) const override {
//        return GetChart()->inv_map(
//            point_type{static_cast<double>(s.x), static_cast<double>(s.y), static_cast<double>(s.z)});
//    };
//    virtual point_type point(EntityId id, point_type const &pr) const {

//
//        Real r = pr[0], s = pr[1], t = pr[2];
//
//        Real w0 = (1 - r) * (1 - s) * (1 - t);
//        Real w1 = r * (1 - s) * (1 - t);
//        Real w2 = (1 - r) * s * (1 - t);
//        Real w3 = r * s * (1 - t);
//        Real w4 = (1 - r) * (1 - s) * t;
//        Real w5 = r * (1 - s) * t;
//        Real w6 = (1 - r) * s * t;
//        Real w7 = r * s * t;
//
//        Real x =
//            m_vertices_(id.x /**/, id.y /**/, id.z /**/, 0) * w0 + m_vertices_(id.x + 1, id.y /**/, id.z /**/, 0) *
//            w1 +
//            m_vertices_(id.x /**/, id.y + 1, id.z /**/, 0) * w2 + m_vertices_(id.x + 1, id.y + 1, id.z /**/, 0) * w3
//            +
//            m_vertices_(id.x /**/, id.y /**/, id.z + 1, 0) * w4 + m_vertices_(id.x + 1, id.y /**/, id.z + 1, 0) * w5
//            +
//            m_vertices_(id.x /**/, id.y + 1, id.z + 1, 0) * w6 + m_vertices_(id.x + 1, id.y + 1, id.z + 1, 0) * w7;
//
//        Real y =
//            m_vertices_(id.x /**/, id.y /**/, id.z /**/, 1) * w0 + m_vertices_(id.x + 1, id.y /**/, id.z /**/, 1) *
//            w1 +
//            m_vertices_(id.x /**/, id.y + 1, id.z /**/, 1) * w2 + m_vertices_(id.x + 1, id.y + 1, id.z /**/, 1) * w3
//            +
//            m_vertices_(id.x /**/, id.y /**/, id.z + 1, 1) * w4 + m_vertices_(id.x + 1, id.y /**/, id.z + 1, 1) * w5
//            +
//            m_vertices_(id.x /**/, id.y + 1, id.z + 1, 1) * w6 + m_vertices_(id.x + 1, id.y + 1, id.z + 1, 1) * w7;
//
//        Real z =
//            m_vertices_(id.x /**/, id.y /**/, id.z /**/, 2) * w0 + m_vertices_(id.x + 1, id.y /**/, id.z /**/, 2) *
//            w1 +
//            m_vertices_(id.x /**/, id.y + 1, id.z /**/, 2) * w2 + m_vertices_(id.x + 1, id.y + 1, id.z /**/, 2) * w3
//            +
//            m_vertices_(id.x /**/, id.y /**/, id.z + 1, 2) * w4 + m_vertices_(id.x + 1, id.y /**/, id.z + 1, 2) * w5
//            +
//            m_vertices_(id.x /**/, id.y + 1, id.z + 1, 2) * w6 + m_vertices_(id.x + 1, id.y + 1, id.z + 1, 2) * w7;
//
//        return point_type{x, y, z};
//    }
}  // namespace mesh
}  // namespace simpla

#endif  // SIMPLA_CYLINDRICALGEOMETRY_H
