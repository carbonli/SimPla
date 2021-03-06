//
// Created by salmon on 17-8-20.
//

#ifndef SIMPLA_SCENARIO_H
#define SIMPLA_SCENARIO_H

#include "Attribute.h"
#include "EngineObject.h"
#include "simpla/geometry/GeoObject.h"
namespace simpla {
namespace engine {
class MeshBase;
class DomainBase;
class Atlas;

class Scenario : public EngineObject {
    SP_ENABLE_NEW_HEAD(EngineObject, Scenario)
   public:
    void Deserialize(std::shared_ptr<const simpla::data::DataEntry> const &cfg) override;
    std::shared_ptr<simpla::data::DataEntry> Serialize() const override;

    virtual void TagRefinementCells(Real time_now);
    virtual void Synchronize(int level);
    virtual void NextStep();
    virtual void Run();
    virtual bool Done() const;
    virtual void CheckPoint(size_type step_num) const;
    virtual void Dump() const;

    void DoInitialize() override;
    void DoSetUp() override;
    void DoUpdate() override;
    void DoTearDown() override;
    void DoFinalize() override;

    virtual Real GetTime() const { return 0.0; }

    void SetStepNumber(size_type s);
    size_type GetStepNumber() const;

    void SetAtlas(std::shared_ptr<Atlas> const &);
    std::shared_ptr<const Atlas> GetAtlas() const;
    std::shared_ptr<Atlas> GetAtlas();

    std::shared_ptr<DomainBase> SetDomain(std::string const &k, std::shared_ptr<DomainBase> const &d);
    std::shared_ptr<DomainBase> GetDomain(std::string const &k) const;

    box_type FitBoundingBox() const;

    template <typename TDomain>
    std::shared_ptr<TDomain> NewDomain(std::string const &k,
                                       std::shared_ptr<const geometry::GeoObject> const &g = nullptr) {
        auto res = TDomain::New();
        res->SetBoundary(g);
        SetDomain(k, res);
        return std::dynamic_pointer_cast<TDomain>(GetDomain(k));
    };
    std::shared_ptr<DomainBase> NewDomain(std::string const &s_type, std::string const &k,
                                          std::shared_ptr<const geometry::GeoObject> const &g = nullptr);

    std::map<std::string, std::shared_ptr<DomainBase>> &GetDomains();
    std::map<std::string, std::shared_ptr<DomainBase>> const &GetDomains() const;

    size_type DeletePatch(id_type);
    id_type SetPatch(id_type id, const std::shared_ptr<Patch> &p);
    std::shared_ptr<Patch> GetPatch(id_type id) const;

    //    std::map<std::string, std::shared_ptr<data::DataEntry>> const &GetAttributes() const;
    //    std::map<std::string, std::shared_ptr<data::DataEntry>> &GetAttributes();

    std::shared_ptr<Attribute> GetAttribute(std::string const &key);
    std::shared_ptr<Attribute> GetAttribute(std::string const &key) const;
    template <typename... Args>
    size_type ConfigureAttribute(std::string const &name, Args &&... args) {
        size_type success = 0;
        if (auto attr = GetAttribute(name)) { success = attr->SetProperties(std::forward<Args>(args)...); }
        return success;
    }

    template <typename U>
    size_type ConfigureAttribute(std::string const &name, std::string const &key, U const &u) {
        size_type success = 0;
        if (auto attr = GetAttribute(name)) { success = attr->SetProperty<U>(key, u); }
        return success;
    }

    //    Range<EntityId> GetRange(std::string const &k) const;
   private:
    struct pimpl_s;
    pimpl_s *m_pimpl_ = nullptr;
};

}  // namespace engine
}  // namespace simpla

#endif  // SIMPLA_SCENARIO_H
