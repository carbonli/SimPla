//
// Created by salmon on 17-9-3.
//

#ifndef SIMPLA_ENGOBJECT_H
#define SIMPLA_ENGOBJECT_H

#include <simpla/data/Configurable.h>
#include <simpla/data/Serializable.h>
#include <simpla/utilities/Signal.h>
#include "Patch.h"

namespace simpla {
namespace engine {
class EngineObject : public data::Configurable, public data::Serializable {
    struct pimpl_s;
    pimpl_s *m_pimpl_ = nullptr;

   public:
    std::string FancyTypeName() const override { return "EngineObject"; }

    EngineObject();

    EngineObject(EngineObject const &) = delete;
    ~EngineObject() override;

    virtual std::shared_ptr<EngineObject> Copy() const;
    static std::shared_ptr<EngineObject> New(std::string const &);
    static std::shared_ptr<EngineObject> New(std::shared_ptr<data::DataEntry> const &);

    void Deserialize(std::shared_ptr<const data::DataEntry> const &cfg) override;
    std::shared_ptr<data::DataEntry> Serialize() const override;

    void lock();
    void unlock();
    bool try_lock();
    void Tag();
    void Click();
    void ResetTag();
    size_type GetTagCount() const;
    size_type GetClickCount() const;
    virtual bool isModified() const;
    virtual bool isInitialized() const;
    virtual bool isSetUp() const;
    virtual void DoInitialize();  //!< invoke once, before everything,
    virtual void DoSetUp();   //!< invoke after Object all configure opeation , Set/Deserialize, Disable Set/Deserialize
    virtual void DoUpdate();  //!< repeat invoke, Update object after modified
    virtual void DoTearDown();  //!< repeat invoke, enable Set/Deserialize
    virtual void DoFinalize();  //!< invoke once, after everything

    //    virtual void Push(const std::shared_ptr<data::DataEntry> &);
    //    virtual std::shared_ptr<data::DataEntry> Pop() const;

    virtual void Push(const std::shared_ptr<Patch> &);
    virtual std::shared_ptr<Patch> Pop() const;

    design_pattern::Signal<void(EngineObject *)> PreSetUp;
    design_pattern::Signal<void(EngineObject *)> PostSetUp;
    design_pattern::Signal<void(EngineObject *)> PreUpdate;
    design_pattern::Signal<void(EngineObject *)> PostUpdate;
    design_pattern::Signal<void(EngineObject *)> PreTearDown;
    design_pattern::Signal<void(EngineObject *)> PostTearDown;

    void Initialize();
    void SetUp();
    void Update();
    void TearDown();
    void Finalize();
};

}  // namespace engine
}
#endif  // SIMPLA_ENGOBJECT_H
