//
// Created by salmon on 16-10-28.
//

#ifndef SIMPLA_LUADATABASE_H
#define SIMPLA_LUADATABASE_H

#include <simpla/SIMPLA_config.h>
#include <memory>
#include <ostream>
#include <string>
#include "DataBackend.h"

namespace simpla {
namespace data {

class DataEntity;

class DataBackendLua : public DataBackend {
   public:
    DataBackendLua();
    DataBackendLua(DataBackendLua const&);
    DataBackendLua(std::string const& url, std::string const& status = "");
    virtual ~DataBackendLua();
    virtual std::ostream& Print(std::ostream& os, int indent = 0) const;

/** Interface DataBackend*/
    virtual std::unique_ptr<DataBackend> Copy() const ;
    virtual void Initialize() ;
    virtual void Finalize() ;
    virtual bool IsNull() const ;  //!< is not initialized
    virtual void Flush() ;

    virtual std::shared_ptr<DataEntity> Get(std::string const& URI) const ;
    virtual std::shared_ptr<DataEntity> Get(id_type key) const ;
    virtual bool Set(std::string const& URI, std::shared_ptr<DataEntity> const&) ;
    virtual bool Set(id_type key, std::shared_ptr<DataEntity> const&) ;
    virtual bool Add(std::string const& URI, std::shared_ptr<DataEntity> const&) ;
    virtual bool Add(id_type key, std::shared_ptr<DataEntity> const&) ;
    virtual size_type Delete(std::string const& URI) ;
    virtual size_type Delete(id_type key) ;
    virtual void DeleteAll() ;
    virtual size_type Count(std::string const& uri = "") const ;
    virtual size_type Accept(std::function<void(std::string const&, std::shared_ptr<DataEntity>)> const&) const ;
    virtual size_type Accept(std::function<void(id_type, std::shared_ptr<DataEntity>)> const&) const ;

    //   private:
    struct pimpl_s;
    pimpl_s* m_pimpl_ = nullptr;
};
}  // { namespace data {
}  // namespace simpla
#endif  // SIMPLA_LUADATABASE_H
