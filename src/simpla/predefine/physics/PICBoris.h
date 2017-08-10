//
// Created by salmon on 17-8-10.
//

#ifndef SIMPLA_PICBORIS_H
#define SIMPLA_PICBORIS_H

#include <simpla/particle/Particle.h>
#include "simpla/SIMPLA_config.h"

#include "simpla/algebra/Algebra.h"
#include "simpla/engine/Domain.h"
#include "simpla/engine/Model.h"
#include "simpla/physics/PhysicalConstants.h"
namespace simpla {

using namespace data;

template <typename THost>
class Boris {
    SP_ENGINE_POLICY_HEAD(Boris);

    void Serialize(data::DataTable* res) const;
    void Deserialize(std::shared_ptr<data::DataTable> const& cfg);
    void InitialCondition(Real time_now);
    void BoundaryCondition(Real time_now, Real time_dt);
    void Advance(Real time_now, Real dt);

    Field<host_type, Real, VOLUME> ne{m_host_, "name"_ = "ne"};
    Field<host_type, Real, VOLUME, 3> B0v{m_host_, "name"_ = "B0v"};

    Field<host_type, Real, EDGE> E0{m_host_, "name"_ = "E0"};
    Field<host_type, Real, FACE> B0{m_host_, "name"_ = "B0"};
    Field<host_type, Real, VOLUME> BB{m_host_, "name"_ = "BB"};
    Field<host_type, Real, VOLUME, 3> Jv{m_host_, "name"_ = "Jv"};

    Field<host_type, Real, FACE> B{m_host_, "name"_ = "B"};
    Field<host_type, Real, EDGE> E{m_host_, "name"_ = "E"};
    Field<host_type, Real, EDGE> J{m_host_, "name"_ = "J"};

    //    void TagRefinementCells(Real time_now);

    std::map<std::string, std::shared_ptr<Particle<THost>>> m_particle_sp_;
    std::shared_ptr<Particle<THost>> AddSpecies(std::string const& name, std::shared_ptr<data::DataTable> const& d);
    std::map<std::string, std::shared_ptr<Particle<THost>>>& GetSpecies() { return m_particle_sp_; };
};

template <typename TM>
void Boris<TM>::Serialize(data::DataTable* res) const {
    for (auto& item : m_particle_sp_) {
        auto t = std::make_shared<data::DataTable>();
        t->SetValue<double>("mass", item.second->mass / SI_proton_mass);
        t->SetValue<double>("Z", item.second->charge / SI_elementary_charge);
        t->SetValue<double>("ratio", item.second->ratio);

        res->Set("Species/" + item.first, t);
    }
};
template <typename TM>
void Boris<TM>::Deserialize(std::shared_ptr<data::DataTable> const& cfg) {
    if (cfg == nullptr || cfg->GetTable("Species") == nullptr) { return; }
    auto sp = cfg->GetTable("Species");
    sp->Foreach([&](std::string const& k, std::shared_ptr<data::DataEntity> v) {
        if (!v->isTable()) { return; }
        auto t = std::dynamic_pointer_cast<data::DataTable>(v);
        AddSpecies(k, t);
    });
}

template <typename TM>
std::shared_ptr<Particle<TM>> Boris<TM>::AddSpecies(std::string const& name,
                                                    std::shared_ptr<data::DataTable> const& d) {
    auto sp = std::make_shared<Particle<TM>>(m_host_, d);
    sp->db()->SetValue("mass", d->GetValue<double>("mass", d->GetValue<double>("mass", 1)) * SI_proton_mass);
    sp->db()->SetValue("charge", d->GetValue<double>("charge", d->GetValue<double>("Z", 1)) * SI_elementary_charge;
    //    sp->ratio = d->GetValue<double>("ratio", d->GetValue<double>("ratio", 1));

    m_particle_sp_.emplace(name, sp);
    VERBOSE << "Add particle : {\"" << name << "\", mass = " << sp->db()->GetValue<double>("mass") / SI_proton_mass
            << " [m_p], charge = " << sp->db()->GetValue<double>("charge") / SI_elementary_charge << " [q_e] }"
            << std::endl;
    return sp;
}
//
// template <typename TM>
// void Boris<TM>::TagRefinementCells(Real time_now) {
//    m_host_->GetMesh()->TagRefinementCells(m_host_->GetMesh()->GetRange(m_host_->GetName() + "_BOUNDARY_3"));
//}
template <typename TM>
void Boris<TM>::InitialCondition(Real time_now) {}
template <typename TM>
void Boris<TM>::BoundaryCondition(Real time_now, Real time_dt) {}
template <typename TM>
void Boris<TM>::Advance(Real time_now, Real dt) {}

}  // namespace simpla  {

#endif  // SIMPLA_PICBORIS_H
