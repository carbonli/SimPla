//
// Created by salmon on 17-4-13.
//

#ifndef SIMPLA_APPLICATION_H
#define SIMPLA_APPLICATION_H

#include "simpla/SIMPLA_config.h"

#include <string>
#include "simpla/data/Data.h"
#include "simpla/engine/Model.h"
#include "simpla/engine/SPObject.h"
#include "simpla/engine/Schedule.h"

namespace simpla {
namespace application {
struct SpApp : public engine::SPObject, public data::Serializable {
    SP_OBJECT_HEAD(SpApp, engine::SPObject);
    SP_DEFAULT_CONSTRUCT(SpApp);

   public:
    explicit SpApp(std::string const &s_name = "SpApp");
    ~SpApp() override;

    using data::Serializable::Serialize;
    using data::Serializable::Deserialize;

    void Config(int argc,char ** argv);
    void Serialize(data::DataTable &cfg) const override;
    void Deserialize(const data::DataTable &cfg) override;

    void DoInitialize() override;
    void DoUpdate() override;
    virtual void Run();
    void DoTearDown() override;
    void DoFinalize() override;

    engine::Context &GetContext();
    engine::Context const &GetContext() const;

    void SetSchedule(std::shared_ptr<engine::Schedule> s);
    std::shared_ptr<engine::Schedule> GetSchedule() const;

   private:
    struct pimpl_s;
    std::unique_ptr<pimpl_s> m_pimpl_;
};
}  // namespace application{

#define SP_APP(_app_name, _app_desc)                                                                   \
    struct _APPLICATION_##_app_name : public application::SpApp {                                      \
        typedef _APPLICATION_##_app_name this_type;                                                    \
        static bool is_registered;                                                                     \
        _APPLICATION_##_app_name() {}                                                                  \
        _APPLICATION_##_app_name(this_type const &) = delete;                                          \
        virtual ~_APPLICATION_##_app_name() {}                                                         \
        void Unpack(std::shared_ptr<data::DataTable>);                                                 \
    };                                                                                                 \
    bool _APPLICATION_##_app_name::is_registered =                                                     \
        application::SpApp::RegisterCreator<_APPLICATION_##_app_name>(__STRING(_app_name), _app_desc); \
    void _APPLICATION_##_app_name::Unpack(std::shared_ptr<data::DataTable> options)

#define SP_REGISITER_APP(_app_name, _app_desc)      \
    bool _APPLICATION_##_app_name##_is_registered = \
        application::SpApp::RegisterCreator<_app_name>(__STRING(_app_name), _app_desc);

}  // namespace simpla{
#endif  // SIMPLA_APPLICATION_H
