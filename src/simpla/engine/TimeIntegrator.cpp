//
// Created by salmon on 17-3-17.
//

#include "simpla/SIMPLA_config.h"

#include "TimeIntegrator.h"

#include "simpla/data/Data.h"

#include "Context.h"
#include "TimeIntegrator.h"

namespace simpla {
namespace engine {

TimeIntegrator::TimeIntegrator(std::string const& s_name) : Schedule(s_name){};
TimeIntegrator::~TimeIntegrator() {}

std::shared_ptr<data::DataTable> TimeIntegrator::Serialize() const {
    auto p = Schedule::Serialize();
    p->SetValue("Name", GetName());
    p->SetValue("Type", GetRegisterName());
    p->SetValue("TimeBegin", GetTimeNow());
    p->SetValue("TimeEnd", GetTimeEnd());
    p->SetValue("TimeStep", GetTimeStep());
    p->SetValue("MaxStep", GetMaxStep());
    return p;
}

void TimeIntegrator::Deserialize(const std::shared_ptr<data::DataTable>& cfg) {
    Schedule::Deserialize(cfg);
    SetTimeNow(cfg->GetValue("TimeBegin", 0.0));
    SetTimeEnd(cfg->GetValue("TimeEnd", 1.0));
    SetTimeStep(cfg->GetValue("TimeStep", 0.5));
    SetMaxStep(cfg->GetValue<size_type>("MaxStep", 0));
};
void TimeIntegrator::Synchronize() { Schedule::Synchronize(); }

void TimeIntegrator::NextStep() {
    Advance(m_time_step_);
    Schedule::NextStep();
}

Real TimeIntegrator::Advance(Real time_dt) {
    if (std::abs(time_dt) < std::numeric_limits<Real>::min()) { time_dt = m_time_step_; }
    time_dt = std::min(std::min(time_dt, m_time_step_), m_time_end_ - m_time_now_);
    m_time_now_ += time_dt;
    return m_time_now_;
};

//    if (level >= m_pack_->m_ctx_->GetAtlas().GetNumOfLevels()) { return m_pack_->m_time_; }
//    auto &atlas = m_pack_->m_ctx_->GetAtlas();
//    for (auto const &id : atlas.GetBlockList(level)) {
//        auto mblk = atlas.GetMeshBlock(id);
//        for (auto &v : m_pack_->m_ctx_->GetAllDomains()) {
//            if (!v.second->GetGeoObject()->CheckOverlap(mblk->GetBoundBox())) { continue; }
//            auto res = m_pack_->m_ctx_->GetPatches()->GetTable(std::to_string(id));
//            if (res == nullptr) { res = std::make_shared<data::DataTable>(); }
//            v.second->ConvertPatchFromSAMRAI(mblk, res);
//            LOGGER << " DomainBase [ " << std::setw(10) << std::left << v.second->name() << " ] is applied on "
//                   << mblk->GetIndexBox() << " id= " << id << std::endl;
//            v.second->Run(dt);
//            auto t = v.second->Serialize().second;
//            m_pack_->m_ctx_->GetPatches()->Deserialize(std::to_string(id), t);
//        }
//    }
//    m_pack_->m_time_ += dt;
//    return m_pack_->m_time_;
//    for (auto const &item : atlas.GetLayer(level)) {
//        for (auto &v : m_pack_->m_domains_) {
//            auto b_box = v.second->GetBaseMesh()->inner_bound_box();
//            if (!geometry::check_overlap(item.second->GetBox(), b_box)) { continue; }
//            v.second->Dispatch(m_pack_->m_patches_[item.first]);
//            v.second->Run(dt);
//        }
//    }
//    for (int i = 0; i < m_pack_->m_refine_ratio_; ++i) { Run(dt / m_pack_->m_refine_ratio_, level + 1); }
//    for (auto const &item : atlas.GetLayer(level)) {
//        for (auto &v : m_pack_->m_domains_) {
//            auto b_box = v.second->GetBaseMesh()->GetGeoObject()->GetBoundBox();
//            if (!geometry::check_overlap(item.second->GetBox(), b_box)) { continue; }
//            v.second->Dispatch(m_pack_->m_patches_[item.first]);
//            v.second->Run(dt);
//        }
//    }

}  // namespace engine{
}  // namespace simpla{