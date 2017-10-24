//
// Created by salmon on 17-10-24.
//

#ifndef SIMPLA_REVOLUTIONBODY_H
#define SIMPLA_REVOLUTIONBODY_H
#include <simpla/utilities/Constants.h>
#include "Revolution.h"
#include "SweptBody.h"
namespace simpla {
namespace geometry {
struct Curve;
struct Surface;
struct Revolution : public SweptBody {
    SP_GEO_OBJECT_HEAD(Revolution, SweptBody);

   protected:
    Revolution();
    Revolution(Revolution const &other);
    explicit Revolution(std::shared_ptr<const Surface> const &s, point_type const &origin, vector_type const &axis);

   public:
    ~Revolution() override;

    point_type Value(Real u, Real v, Real w) const override;
};

}  // namespace simpla
}  // namespace geometry
#endif  // SIMPLA_REVOLUTIONBODY_H
