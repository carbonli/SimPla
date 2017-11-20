//
// Created by salmon on 17-11-6.
//

#include "GeoEntity.h"
#include "Edge.h"
#include "Face.h"
#include "Solid.h"
namespace simpla {
namespace geometry {
GeoEntity::GeoEntity() = default;
GeoEntity::GeoEntity(GeoEntity const &) = default;
GeoEntity::~GeoEntity() = default;
std::string GeoEntity::FancyTypeName() const override { return "GeoEntity"; }
std::shared_ptr<GeoEntity> GeoEntity::Create(std::string const &k) {
    return std::dynamic_pointer_cast<GeoEntity>(data::Serializable::Create(k));
}
std::shared_ptr<GeoEntity> GeoEntity::Create(std::shared_ptr<const simpla::data::DataEntry> const &cfg) {
    return std::dynamic_pointer_cast<GeoEntity>(data::Serializable::Create(cfg));
}
}  // namespace geometry{
}  // namespace simpla{