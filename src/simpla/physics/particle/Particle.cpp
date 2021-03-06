//
// Created by salmon on 17-8-10.
//

#include "Particle.h"
#include "ParticleData.h"
#include "simpla/algebra/EntityId.h"
#include "simpla/algebra/nTuple.h"
#include "simpla/engine/Domain.h"

namespace simpla {

class ParticleEngineBase;
struct ParticlePool : public data::DataEntity {};

struct ParticleBase::pimpl_s {
    static constexpr int MAX_NUMBER_OF_PARTICLE_ATTRIBUTES = 10;
    engine::DomainBase const* m_domain_;
    size_type m_num_pic_ = 100;
    size_type m_max_size_ = 0;
    int m_num_of_attr_ = 3;
    std::shared_ptr<ParticleData> m_data_block_ = nullptr;
    id_type* m_tag_;
    Real* m_data_[MAX_NUMBER_OF_PARTICLE_ATTRIBUTES];
};

ParticleBase::ParticleBase() : m_pimpl_(new pimpl_s) { Initialize(); }

ParticleBase::~ParticleBase() {
    Finalize();
    delete m_pimpl_;
}

void ParticleBase::DoInitialize() {
    m_pimpl_ = new pimpl_s;
    int dof = db()->GetValue<int>("DOF", 6);
    SetDOF(1, &dof);
}
void ParticleBase::DoFinalize() {
    delete m_pimpl_;
    m_pimpl_ = nullptr;
}
std::shared_ptr<simpla::data::DataEntry> ParticleBase::Serialize() const { return base_type::Serialize(); }
void ParticleBase::Deserialize(std::shared_ptr<data::DataEntry> const& cfg) { base_type::Deserialize(cfg); }
void ParticleBase::Push(std::shared_ptr<data::DataEntry> const& dblk) {
    SetUp();
    //    base_type::Push(dblk);
    Update();
}
std::shared_ptr<data::DataEntry> ParticleBase::Pop() const {
    //    auto res = engine::Attribute::Pop();
    return nullptr;
}
void ParticleBase::SetNumberOfAttributes(int n) { m_pimpl_->m_num_of_attr_ = n; }
int ParticleBase::GetNumberOfAttributes() const { return m_pimpl_->m_num_of_attr_; }

void ParticleBase::SetNumberOfPIC(size_type n) { m_pimpl_->m_num_pic_ = n; }
size_type ParticleBase::GetNumberOfPIC() { return m_pimpl_->m_num_pic_; }

size_type ParticleBase::GetMaxSize() const { return m_pimpl_->m_max_size_; }
std::shared_ptr<ParticleBase::Bucket> ParticleBase::GetBucket(id_type s) { return nullptr; }
std::shared_ptr<ParticleBase::Bucket> ParticleBase::GetBucket(id_type s) const { return nullptr; }
std::shared_ptr<ParticleBase::Bucket> ParticleBase::AddBucket(id_type s, size_type num) { return nullptr; }
void ParticleBase::RemoveBucket(id_type s) {}
size_type ParticleBase::Count(id_type s) const {
    size_type res = 0;
    if (s == NULL_ID) {
    } else {
        for (auto bucket = GetBucket(s); bucket != nullptr; bucket = bucket->next) { res += bucket->count; }
    }
    return res;
}
//*********************************************************************************************************************
enum { SP_RAND_UNIFORM = 0x1, SP_RAND_NORMAL = 0x10 };

int ParticleInitialLoad(Real**, size_type num, int n_dof, int const* dist_types, size_type random_seed_offset);
void ParticleUpdateTag(size_type num, id_type* tag, Real** r);
void ParticleSort(size_type num, int num_of_attr, id_type const* tag_in, id_type* tag_out, Real** in, Real** out);
//*********************************************************************************************************************

void ParticleBase::Sort() {
    ParticleUpdateTag(GetMaxSize(), m_pimpl_->m_tag_, m_pimpl_->m_data_);

    ParticleSort(GetMaxSize(), GetNumberOfAttributes(), m_pimpl_->m_tag_, m_pimpl_->m_tag_, m_pimpl_->m_data_,
                 m_pimpl_->m_data_);
}
void ParticleBase::DeepSort() {}

void ParticleBase::InitialLoad(int const* rnd_dist_type, size_type rnd_offset) {
    int dist_type[GetNumberOfAttributes()];
    int ndims = m_pimpl_->m_domain_->GetNDIMS();
    ASSERT(GetNumberOfAttributes() >= 2 * ndims);

    if (rnd_dist_type == nullptr) {
        for (int i = 0; i < m_pimpl_->m_domain_->GetNDIMS(); ++i) { dist_type[i] = SP_RAND_UNIFORM; }
        for (int i = ndims; i < 2 * ndims; ++i) { dist_type[i] = SP_RAND_NORMAL; }

    } else {
        for (int i = 0; i < 2 * ndims; ++i) { dist_type[i] = rnd_dist_type[i]; }
    }
    ParticleInitialLoad(m_pimpl_->m_data_, m_pimpl_->m_max_size_, 2 * ndims, dist_type, rnd_offset);
}

}  // namespace simpla {
