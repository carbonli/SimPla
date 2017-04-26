/**
 * @file em_fluid.h
 * @author salmon
 * @date 2016-01-13.
 */

#ifndef SIMPLA_EM_FLUID_H
#define SIMPLA_EM_FLUID_H

#include "simpla/SIMPLA_config.h"
#include "simpla/algebra/all.h"
#include "simpla/engine/all.h"
#include "simpla/physics/PhysicalConstants.h"

namespace simpla {
using namespace algebra;
using namespace data;
using namespace engine;

template <typename TM>
class EMFluid : public engine::Domain {
    SP_OBJECT_HEAD(EMFluid<TM>, engine::Domain)
    typedef TM mesh_type;

   public:
    DOMAIN_HEAD(EMFluid, engine::Domain)

    std::shared_ptr<data::DataTable> Serialize() const override {
        auto res = std::make_shared<data::DataTable>();
        res->SetValue<std::string>("Type", "EMFluid<" + TM::ClassName() + ">");
        return res;
    };

    void Deserialize(shared_ptr<data::DataTable> const& cfg) override {}

    void Initialize() override;
    void Finalize() override;
    void SetUp() override;
    void TearDown() override;

    void InitializeCondition(Real time_now) override;
    void BoundaryCondition(Real time_now, Real dt) override;
    void Advance(Real time_now, Real dt) override;

    typedef field_type<FACE> TB;
    typedef field_type<EDGE> TE;
    typedef field_type<EDGE> TJ;
    typedef field_type<VERTEX> TRho;
    typedef field_type<VERTEX, 3> TJv;

    field_type<VERTEX> rho0{m_mesh_, "name"_ = "rho"};

    field_type<EDGE> E0{m_mesh_};
    field_type<FACE> B0{m_mesh_};
    field_type<VERTEX, 3> B0v{m_mesh_};
    field_type<VERTEX> BB{m_mesh_, "name"_ = "BB"};
    field_type<VERTEX, 3> Ev{m_mesh_};
    field_type<VERTEX, 3> Bv{m_mesh_};
    field_type<VERTEX, 3> dE{m_mesh_};

    field_type<FACE> B{m_mesh_, "name"_ = "B"};
    field_type<EDGE> E{m_mesh_, "name"_ = "E"};
    field_type<EDGE> J1{m_mesh_, "name"_ = "J1"};

    struct fluid_s {
        Real mass;
        Real charge;
        std::shared_ptr<TRho> rho;
        std::shared_ptr<TJv> J;
    };

    std::map<std::string, std::shared_ptr<fluid_s>> m_fluid_sp_;
    std::shared_ptr<fluid_s> AddSpecies(std::string const& name, data::DataTable const& d);
    std::map<std::string, std::shared_ptr<fluid_s>>& GetSpecies() { return m_fluid_sp_; };
    std::shared_ptr<geometry::GeoObject> m_antenna_;
};

template <typename TM>
bool EMFluid<TM>::is_registered = engine::Domain::RegisterCreator<EMFluid<TM>>();

template <typename TM>
std::shared_ptr<struct EMFluid<TM>::fluid_s> EMFluid<TM>::AddSpecies(std::string const& name,
                                                                     data::DataTable const& d) {
    Real mass;
    Real charge;

    if (d.has("mass")) {
        mass = d.GetValue<double>("mass");
    } else if (d.has("m")) {
        mass = d.GetValue<double>("m") * SI_proton_mass;
    } else {
        mass = SI_proton_mass;
    }

    if (d.has("charge")) {
        charge = d.GetValue<double>("charge");
    } else if (d.has("Z")) {
        charge = d.GetValue<double>("Z") * SI_elementary_charge;
    } else {
        charge = SI_elementary_charge;
    }

    VERBOSE << "Add particle : {\"" << name << "\", mass = " << mass / SI_proton_mass
            << " [m_p], charge = " << charge / SI_elementary_charge << " [q_e] }" << std::endl;
    auto sp = std::make_shared<fluid_s>();
    sp->mass = mass;
    sp->charge = charge;
    sp->rho = std::make_shared<TRho>(m_mesh_, name + "_rho");
    sp->J = std::make_shared<TJv>(m_mesh_, name + "_J");
    m_fluid_sp_.emplace(name, sp);
    return sp;
}

template <typename TM>
void EMFluid<TM>::Initialize() {}

template <typename TM>
void EMFluid<TM>::Finalize() {}

template <typename TM>
void EMFluid<TM>::TearDown() {}

template <typename TM>
void EMFluid<TM>::SetUp() {
    base_type::SetUp();
}

template <typename TM>
void EMFluid<TM>::InitializeCondition(Real time_now) {
    Domain::InitializeCondition(time_now);

    E.Clear();
    B.Clear();
    B0v.Clear();
    //    if (E.isNull()) {}
    //    rho0[0].Foreach([&](index_tuple const& k, Real& v) { v = k[1]; });
    rho0.Assign([&](point_type const& z) -> Real { return z[1] * z[0] * z[2]; });
    //    Ev = map_to<VERTEX>(E);
    //    B0v = map_to<VERTEX>(B0);
    BB = dot(B0v, B0v);
}
template <typename TM>
void EMFluid<TM>::BoundaryCondition(Real time_now, Real dt) {
    //    auto brd = m_mesh_->Boundary();
    //    if (brd == nullptr) { return; }
    //    E(brd) = 0;
    //    B(brd) = 0;
    //    auto antenna = m_mesh_->SubMesh(m_antenna_);
    //    if (antenna != nullptr) {
    //        nTuple<Real, 3> k{1, 1, 1};
    //        Real omega = 1.0;
    //        E(antenna)
    //            .Assign([&](point_type const& x) {
    //                auto amp = std::sin(omega * time_now) * std::cos(dot(k, x));
    //                return nTuple<Real, 3>{amp, 0, 0};
    //            });
    //    }
}
template <typename TM>
void EMFluid<TM>::Advance(Real time_now, Real dt) {
    DEFINE_PHYSICAL_CONST
    B = B - curl(E) * (dt * 0.5);
    //    SetPhysicalBoundaryConditionB(time_now);
    E += (curl(B) * speed_of_light2 - J1 / epsilon0) * dt;
    //    SetPhysicalBoundaryConditionE(time_now);
    if (m_fluid_sp_.size() > 0) {
        field_type<VERTEX, 3> Q{m_mesh_};
        field_type<VERTEX, 3> K{m_mesh_};

        field_type<VERTEX> a{m_mesh_};
        field_type<VERTEX> b{m_mesh_};
        field_type<VERTEX> c{m_mesh_};

        a.Clear();
        b.Clear();
        c.Clear();

        Q = map_to<VERTEX>(E) - Ev;
        dE.Clear();
        K.Clear();
        for (auto& p : m_fluid_sp_) {
            Real ms = p.second->mass;
            Real qs = p.second->charge;
            auto& ns = *p.second->rho;
            auto& Js = *p.second->J;

            Real as = static_cast<Real>((dt * qs) / (2.0 * ms));

            Q -= 0.5 * dt / epsilon0 * Js;

            K = (Ev * qs * ns * 2.0 + cross(Js, B0v)) * as + Js;

            Js = (K + cross(K, B0v) * as + B0v * (dot(K, B0v) * as * as)) / (BB * as * as + 1);

            Q -= 0.5 * dt / epsilon0 * Js;

            a += qs * ns * (as / (BB * as * as + 1));
            b += qs * ns * (as * as / (BB * as * as + 1));
            c += qs * ns * (as * as * as / (BB * as * as + 1));
        }

        a *= 0.5 * dt / epsilon0;
        b *= 0.5 * dt / epsilon0;
        c *= 0.5 * dt / epsilon0;
        a += 1;

        dE = (Q * a - cross(Q, B0v) * b + B0v * (dot(Q, B0v) * (b * b - c * a) / (a + c * BB))) / (b * b * BB + a * a);

        for (auto& p : m_fluid_sp_) {
            Real ms = p.second->mass;
            Real qs = p.second->charge;
            auto& ns = *p.second->rho;
            auto& Js = *p.second->J;

            Real as = static_cast<Real>((dt * qs) / (2.0 * ms));

            K = dE * ns * qs * as;
            Js += (K + cross(K, B0v) * as + B0v * (dot(K, B0v) * as * as)) / (BB * as * as + 1);
        }
        Ev += dE;
        E += map_to<EDGE>(Ev) - E;
    }
    B -= curl(E) * (dt * 0.5);
    //    SetPhysicalBoundaryConditionB(time_now);
}

}  // namespace simpla  {
#endif  // SIMPLA_EM_FLUID_H
