//
// Created by salmon on 17-3-1.
//

#include "MeshBlock.h"
#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "SPObject.h"
namespace simpla {
namespace engine {

struct MeshBlock::pimpl_s {
    size_type m_level_ = 0;
    Real m_time_ = 0;
    id_type m_GUID_ = NULL_ID;
    index_box_type m_index_box_;
    index_tuple m_ghost_width_{2, 2, 2};
    static boost::uuids::random_generator m_gen_;
    static boost::hash<boost::uuids::uuid> m_hasher_;
};
boost::uuids::random_generator MeshBlock::pimpl_s::m_gen_;
boost::hash<boost::uuids::uuid> MeshBlock::pimpl_s::m_hasher_;
MeshBlock::MeshBlock(index_box_type const &b, size_type level, Real time_now) : m_pimpl_(new pimpl_s) {
    m_pimpl_->m_index_box_ = b;
    m_pimpl_->m_level_ = level;
    m_pimpl_->m_GUID_ = m_pimpl_->m_hasher_(m_pimpl_->m_gen_());
    m_pimpl_->m_time_ = time_now;
};
MeshBlock::MeshBlock(MeshBlock const &other) : m_pimpl_(new pimpl_s) {
    m_pimpl_->m_level_ = other.m_pimpl_->m_level_;
    m_pimpl_->m_GUID_ = other.m_pimpl_->m_GUID_;
    m_pimpl_->m_index_box_ = other.m_pimpl_->m_index_box_;
    m_pimpl_->m_time_ = other.m_pimpl_->m_time_;
}
MeshBlock::~MeshBlock() {}

Real MeshBlock::GetTime() const { return m_pimpl_->m_time_; }
index_tuple MeshBlock::GetGhostWidth() const { return m_pimpl_->m_ghost_width_; };
index_box_type MeshBlock::GetIndexBox(int IFORM, int sub) const { return m_pimpl_->m_index_box_; }
index_box_type MeshBlock::GetOuterIndexBox(int IFORM, int sub) const {
    auto ibox = GetIndexBox(IFORM, sub);
    std::get<0>(ibox) -= GetGhostWidth();
    std::get<1>(ibox) += GetGhostWidth();
    return std::move(ibox);
}
index_box_type MeshBlock::GetInnerIndexBox(int IFORM, int sub) const { return GetIndexBox(IFORM, sub); }

// box_type MeshBlock::GetBoundBox() const {
//    box_type res;
//    res = GetIndexBox();
//    return std::move(res);
//}

index_tuple MeshBlock::GetOffset() const { return std::get<0>(m_pimpl_->m_index_box_); }

size_tuple MeshBlock::GetDimensions() const {
    return std::get<1>(m_pimpl_->m_index_box_) - std::get<0>(m_pimpl_->m_index_box_);
}

id_type MeshBlock::GetGUID() const { return m_pimpl_->m_GUID_; }
size_type MeshBlock::GetLevel() const { return m_pimpl_->m_level_; }

}  // namespace engine {
}  // namespace simpla {