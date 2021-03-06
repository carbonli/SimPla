//
// Created by salmon on 17-11-6.
//

#ifndef SIMPLA_SHAPE_H
#define SIMPLA_SHAPE_H

#include <simpla/data/Configurable.h>
#include <simpla/data/Creatable.h>
#include <simpla/data/Serializable.h>
#include <memory>

namespace simpla {
namespace geometry {
class Edge;
class Face;
class Solid;

/**
 * @brief The general abstract class GeoEntity is understood as a mathematical description of a shape,
 *  e.g. curves, surface ( Bezier,NURBS etc.)
 * - All objects derived from GeoEntity are defined at  DEFAULT Axis.
 *    - Default axis : origin = {0,0,0},  x-axis={1,0,0} ,y-axis ={0,1,0} ,z-axis={0,0,1}
 * - All 2d curve/shape ares defined in  xy-plane, with  normal vector z-axis
 * - z-axis is the main axis of symmetry for cylinder,sphere or circle
 *
 *     @startuml
 *  class GeoEntity{
 *  }
 *
 *   GeoEntity<|--Vertex
 *   GeoEntity<|--Curve
 *   GeoEntity<|--Surface
 *   GeoEntity<|--Body
 *   Body o-- Surface
 *   Surface o-- Curve
 *   Curve <|-- ParametricCurve
 *   Curve <|-- BoundedCurve
 *   ParametricCurve <|-- Line
 *   ParametricCurve <|-- Conic
 *
 *   BoundedCurve <|-- Polyline
 *   BoundedCurve <|-- BezierCurve
 *   BoundedCurve <|-- BSplineCurve
 *   BoundedCurve <|-- TrimmedCurve
 *
 *   Conic <|-- Circle
 *   Conic <|-- Ellipse
 *   Conic <|-- Hyperbola
 *   Conic <|-- Parabola
 *
 *   Surface <|-- ParametricSurface
 *   ParametricSurface <|-- Plane
 *   ParametricSurface <|-- CylindricalSurface
 *   ParametricSurface <|-- SphericalSurface
 *   ParametricSurface <|-- ToroidalSurface
 *
 *   Surface <|-- BoundedSurface
 *   BoundedSurface <|-- BezierSurface
 *   BoundedSurface <|-- BSplineSurface
 *   BoundedSurface <|-- PatchSurface
 *
 *  ParametricBody o-- ParametricSurface
 *
 *   Body <|-- ParametricBody
 *   ParametricBody <|-- Cube
 *   ParametricBody <|-- Ball
 *   ParametricBody <|-- Cylindrical
 *   ParametricBody <|-- Toroidal
 *
 *   Surface <|-- SweptSurface
 *   SweptSurface <|-- SurfaceOfLinearExtrusion
 *   SweptSurface <|-- SurfaceOfRevolution
 *
 *   Vertex  <|-- GetIntersectionCurveSurface
 *   Surface <|-- GetIntersectionSurfaceSolid
 *   Solid   <|-- GetIntersectionSolidSolid
 *     @enduml
 */
struct GeoEntity : public data::Serializable, public data::Configurable, public data::Creatable<GeoEntity> {
   public:
    GeoEntity();
    GeoEntity(GeoEntity const &);
    ~GeoEntity() override;
    std::string FancyTypeName() const override;

    virtual point_type xyz(Real u, Real v, Real w) const = 0;
    virtual GeoEntity *CopyP() const = 0;
    std::shared_ptr<GeoEntity> Copy() const { return std::shared_ptr<GeoEntity>(CopyP()); }
};
#define SP_GEO_ENTITY_ABS_HEAD(_BASE_NAME_, _CLASS_NAME_)                                            \
    SP_SERIALIZABLE_HEAD(_BASE_NAME_, _CLASS_NAME_)                                                  \
   protected:                                                                                        \
    _CLASS_NAME_() = default;                                                                        \
                                                                                                     \
   public:                                                                                           \
    ~_CLASS_NAME_() override = default;                                                              \
    template <typename... Args>                                                                      \
    static std::shared_ptr<this_type> Create(Args &&... args) {                                      \
        return std::dynamic_pointer_cast<this_type>(base_type::Create(std::forward<Args>(args)...)); \
    }

#define SP_GEO_ENTITY_HEAD(_BASE_NAME_, _CLASS_NAME_, _REGISTER_NAME_)                       \
    SP_GEO_ENTITY_ABS_HEAD(_BASE_NAME_, _CLASS_NAME_)                                        \
                                                                                             \
   private:                                                                                  \
    static bool _is_registered;                                                              \
                                                                                             \
   public:                                                                                   \
    static std::string RegisterName() noexcept { return __STRING(_REGISTER_NAME_); }         \
    this_type *CopyP() const override { return new this_type(*this); };                      \
    template <typename... Args>                                                              \
    static std::shared_ptr<_CLASS_NAME_> New(Args &&... args) {                              \
        return std::shared_ptr<_CLASS_NAME_>(new _CLASS_NAME_(std::forward<Args>(args)...)); \
    }

#define SP_GEO_ENTITY_REGISTER(_CLASS_NAME_) \
    bool _CLASS_NAME_::_is_registered =      \
        simpla::Factory<GeoEntity>::RegisterCreator<_CLASS_NAME_>(_CLASS_NAME_::RegisterName());
}  // namespace geometry{
}  // namespace simpla{
#endif  // SIMPLA_SHAPE_H
