//
// Created by salmon on 17-4-5.
//

#include "simpla/SIMPLA_config.h"

#include "simpla/geometry/Chart.h"
#include "simpla/geometry/GeoObject.h"

#include "Attribute.h"
#include "Domain.h"
#include "Mesh.h"
#include "Model.h"
#include "Patch.h"

namespace simpla {
namespace engine {

DomainBase::DomainBase(MeshBase* msh, std::shared_ptr<Model> const& model) : m_mesh_(msh), m_model_(model) {}

DomainBase::~DomainBase() = default;

void DomainBase::Serialize(data::DataTable& cfg) const {
    base_type::Serialize(cfg);
    if (GetGeoBody() != nullptr) { GetGeoBody()->Serialize(cfg.GetTable("Body")); }
}
void DomainBase::Deserialize(const DataTable& cfg) {
    base_type::Deserialize(cfg);
    if (cfg.isTable("Body")) {
        m_geo_body_ = geometry::GeoObject::Create(*cfg.Get("Body"));
    } else if (m_model_ != nullptr) {
        m_geo_body_ = m_model_->GetGeoObject(cfg.GetValue<std::string>("Body", ""));
    }
    Click();
};

void DomainBase::DoUpdate() {}
void DomainBase::DoTearDown() {}
void DomainBase::DoInitialize() {}
void DomainBase::DoFinalize() {}

void DomainBase::InitialCondition(Real time_now) {
    Update();
    if (std::get<0>(GetMesh()->CheckOverlap(GetGeoBody().get())) < EPSILON) { return; }
    PreInitialCondition(this, time_now);
    DoInitialCondition(time_now);
    PostInitialCondition(this, time_now);
}
void DomainBase::BoundaryCondition(Real time_now, Real dt) {
    Update();
    if (std::get<0>(GetMesh()->CheckOverlap(GetGeoBody().get())) < EPSILON) { return; }
    PreBoundaryCondition(this, time_now, dt);
    DoBoundaryCondition(time_now, dt);
    PostBoundaryCondition(this, time_now, dt);
}

void DomainBase::ComputeFluxes(Real time_now, Real dt) {
    Update();
    if (std::get<0>(GetMesh()->CheckOverlap(GetGeoBody().get())) < EPSILON) { return; }
    PreComputeFluxes(this, time_now, dt);
    DoComputeFluxes(time_now, dt);
    PostComputeFluxes(this, time_now, dt);
}
Real DomainBase::ComputeStableDtOnPatch(Real time_now, Real time_dt) const { return time_dt; }

void DomainBase::Advance(Real time_now, Real dt) {
    Update();
    if (std::get<0>(GetMesh()->CheckOverlap(GetGeoBody().get())) < EPSILON) { return; }
    PreAdvance(this, time_now, dt);
    DoAdvance(time_now, dt);
    PostAdvance(this, time_now, dt);
}
void DomainBase::TagRefinementCells(Real time_now) {
    Update();
    if (std::get<0>(GetMesh()->CheckOverlap(GetGeoBody().get())) < EPSILON) { return; }
    PreTagRefinementCells(this, time_now);
    GetMesh()->TagRefinementRange(GetMesh()->GetRange(GetName() + "_BOUNDARY_3"));
    DoTagRefinementCells(time_now);
    PostTagRefinementCells(this, time_now);
}

}  // namespace engine{
}  // namespace simpla{