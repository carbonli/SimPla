//
// Created by salmon on 17-2-22.
//
#include "Patch.h"
#include <simpla/geometry/GeoObject.h>
#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <map>
#include "MeshBlock.h"
namespace simpla {
namespace engine {
struct Patch::pimpl_s {
    id_type m_id_ = NULL_ID;
    std::shared_ptr<Chart> m_chart_ = nullptr;
    std::shared_ptr<MeshBlock> m_block_ = nullptr;
    std::map<id_type, std::shared_ptr<data::DataBlock>> m_data_;
    std::shared_ptr<std::map<std::string, nTuple<EntityRange, 4>>> m_range_;
    static boost::uuids::random_generator m_gen_;
    static boost::hash<boost::uuids::uuid> m_hasher_;
};
boost::uuids::random_generator Patch::pimpl_s::m_gen_;
boost::hash<boost::uuids::uuid> Patch::pimpl_s::m_hasher_;
Patch::Patch(id_type id) : m_pimpl_(new pimpl_s) {
    m_pimpl_->m_id_ = id != NULL_ID ? id : m_pimpl_->m_hasher_(m_pimpl_->m_gen_());
}
Patch::~Patch() {}

id_type Patch::GetId() const { return m_pimpl_->m_id_; }

void Patch::SetChart(std::shared_ptr<Chart> c) { m_pimpl_->m_chart_ = c; }
std::shared_ptr<Chart> Patch::GetChart() const { return m_pimpl_->m_chart_; }
void Patch::SetBlock(std::shared_ptr<MeshBlock> const &m) { m_pimpl_->m_block_ = m; }
std::shared_ptr<MeshBlock> Patch::GetBlock() const { return m_pimpl_->m_block_; }

void Patch::Merge(Patch &other) {
    if (other.m_pimpl_->m_chart_ != nullptr) { m_pimpl_->m_chart_ = other.m_pimpl_->m_chart_; }
    if (other.m_pimpl_->m_block_ != nullptr) { m_pimpl_->m_block_ = other.m_pimpl_->m_block_; }
    if (m_pimpl_->m_id_ == NULL_ID) { m_pimpl_->m_id_ = other.m_pimpl_->m_id_; }
    m_pimpl_->m_data_.insert(other.m_pimpl_->m_data_.begin(), other.m_pimpl_->m_data_.end());
}

std::map<id_type, std::shared_ptr<data::DataBlock>> &Patch::GetAllData() { return m_pimpl_->m_data_; };

int Patch::Push(id_type const &id, std::shared_ptr<data::DataBlock> const &d) {
    auto res = m_pimpl_->m_data_.emplace(id, d);
    if (res.first->second == nullptr) { res.first->second = d; }
    return res.first->second != nullptr ? 1 : 0;
}
std::shared_ptr<data::DataBlock> Patch::Pop(id_type const &id) const {
    std::shared_ptr<data::DataBlock> res = nullptr;
    auto it = m_pimpl_->m_data_.find(id);
    if (it != m_pimpl_->m_data_.end()) { res = it->second; }
    return res;
}
EntityRange const *Patch::GetRange(const std::string &g) const {
    EntityRange const *res = nullptr;
    if (m_pimpl_->m_range_ != nullptr) {
        auto it = m_pimpl_->m_range_->find(g);
        if (it != m_pimpl_->m_range_->end()) { res = &(it->second[0]); }
    }
    return res;
}

void Patch::SetRange(EntityRange const *r, const std::string &g) {
    if (r != nullptr) {
        if (m_pimpl_->m_range_ == nullptr) {
            m_pimpl_->m_range_ = std::make_shared<std::map<std::string, nTuple<EntityRange, 4>>>();
        }

        for (int i = 0; i < 4; ++i) { (*m_pimpl_->m_range_)[g][i] = r[i]; }
    };
}

void Patch::PushRange(std::shared_ptr<std::map<std::string, nTuple<EntityRange, 4>>> const &r) {
    m_pimpl_->m_range_ = r;
};
std::shared_ptr<std::map<std::string, nTuple<EntityRange, 4>>> Patch::PopRange() { return m_pimpl_->m_range_; };

}  // namespace engine {
}  // namespace simpla {