//
// Created by salmon on 16-10-28.
//

#ifndef SIMPLA_LUADATABASE_H
#define SIMPLA_LUADATABASE_H
#include <memory>
#include <ostream>
#include <string>
#include "../DataBackend.h"
#include "simpla/SIMPLA_config.h"

namespace simpla {
namespace data {

class DataEntity;

class DataBackendLua : public DataBackend {
    SP_OBJECT_HEAD(DataBackendLua, DataBackend)
   public:
    DataBackendLua();
    DataBackendLua(this_type const& other);
    ~DataBackendLua() override;

    std::ostream& Print(std::ostream& os, int indent) const override;

    void Parser(std::string const& s) override;

    void Connect(std::string const& authority, std::string const& path, std::string const& query = "",
                 std::string const& fragment = "") override;
    void Disconnect() override;
    std::shared_ptr<DataBackend> Duplicate() const override;
    std::shared_ptr<DataBackend> CreateNew() const override;
    bool isNull() const;
    void Flush() override;

    std::shared_ptr<DataEntity> Get(std::string const& URI) const override;
    std::shared_ptr<DataEntity> Get(int key) const;
    void Set(std::string const& URI, const std::shared_ptr<DataEntity>&) override;
    void Add(std::string const& URI, const std::shared_ptr<DataEntity>&) override;
    size_type Delete(std::string const& URI) override;
    size_type size() const override;
    size_type Foreach(std::function<void(std::string const&, std::shared_ptr<DataEntity>)> const&) const override;

   private:
    struct pimpl_s;
    std::unique_ptr<pimpl_s> m_pimpl_;
};
}  // { namespace data {
}  // namespace simpla
#endif  // SIMPLA_LUADATABASE_H
