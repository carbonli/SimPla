//
// Created by salmon on 17-10-23.
//

#ifndef SIMPLA_SPHERE_H
#define SIMPLA_SPHERE_H

#include <simpla/SIMPLA_config.h>
#include "Body.h"
#include "Surface.h"

namespace simpla {
namespace geometry {

struct gSphere : public ParametricBody {
    SP_GEO_ENTITY_HEAD(ParametricBody, gSphere, Sphere)
    explicit gSphere(Real radius) : m_Radius_(radius){};
    SP_PROPERTY(Real, Radius);
    point_type xyz(Real r, Real phi, Real theta) const override {
        Real cos_theta = std::cos(theta);
        return point_type{r * cos_theta * std::cos(phi), r * cos_theta * std::sin(phi), r * std::sin(theta)};
    };
};
struct gSphereSurface : public ParametricSurface {
    SP_GEO_ENTITY_HEAD(ParametricSurface, gSphereSurface, SphereSurface)

    explicit gSphereSurface(Real radius) : m_Radius_(radius){};
    SP_PROPERTY(Real, Radius);
    point_type xyz(Real phi, Real theta) const override {
        Real cos_theta = std::cos(theta);
        return point_type{m_Radius_ * cos_theta * std::cos(phi), m_Radius_ * cos_theta * std::sin(phi),
                          m_Radius_ * std::sin(theta)};
    };
};
}  // namespace geometry
}  // namespace simpla

#endif  // SIMPLA_SPHERE_H