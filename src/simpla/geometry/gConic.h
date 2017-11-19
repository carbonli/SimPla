//
// Created by salmon on 17-11-19.
//

#ifndef SIMPLA_GCONIC_H
#define SIMPLA_GCONIC_H
#include "Curve.h"
#include "Surface.h"
namespace simpla {
namespace geometry {
struct gConic : public ParametricCurve2D {
    SP_GEO_ENTITY_ABS_HEAD(ParametricCurve2D, gConic)
};
struct gConicSurface : public ParametricSurface {
    SP_GEO_ENTITY_ABS_HEAD(ParametricSurface, gConicSurface)
};
}  //  namespace geometry{
}  // namespace simpla
#endif  // SIMPLA_GCONIC_H