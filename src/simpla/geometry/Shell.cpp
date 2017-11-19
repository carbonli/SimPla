//
// Created by salmon on 17-11-6.
//

#include "Shell.h"
#include "GeoEntity.h"
namespace simpla {
namespace geometry {
Shell::Shell() = default;
Shell::Shell(Shell const &) = default;
Shell::~Shell() = default;
Shell::Shell(Axis const &axis) : Surface(axis){};
Shell::Shell(std::shared_ptr<const GeoEntity> const &shape) : Surface(shape->GetAxis()), m_shape_(shape){};
void Shell::Deserialize(std::shared_ptr<simpla::data::DataEntry> const &cfg) { base_type::Deserialize(cfg); };
std::shared_ptr<simpla::data::DataEntry> Shell::Serialize() const { return base_type::Serialize(); };
std::shared_ptr<const GeoEntity> Shell::GetShape() const { return m_shape_; }
}  // namespace geometry{
}  // namespace simpla{