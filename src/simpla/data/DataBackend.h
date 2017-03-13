//
// Created by salmon on 17-3-6.
//

#ifndef SIMPLA_DATABACKEND_H
#define SIMPLA_DATABACKEND_H

#include <simpla/SIMPLA_config.h>
#include <simpla/concept/Printable.h>
#include <simpla/design_pattern/Factory.h>
#include <simpla/design_pattern/SingletonHolder.h>
#include <simpla/engine/SPObjectHead.h>
#include <simpla/toolbox/Log.h>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <vector>
namespace simpla {
namespace data {
class DataEntity;
class DataTable;
class DataBackend : public concept::Printable, public std::enable_shared_from_this<DataBackend> {
    SP_OBJECT_BASE(DataBackend);

   public:
    DataBackend(){};
    virtual ~DataBackend(){};

    virtual void Connect(std::string const& path, std::string const& param = ""){};
    virtual void Disconnect(){};
    virtual void Flush() = 0;
    virtual std::ostream& Print(std::ostream& os, int indent = 0) const { return os; };

    /**
     * @brief create a new backend with same scheme
     * @return
     */
    virtual std::shared_ptr<DataBackend> CreateNew() const = 0;
    /**
     * @brief create a copy of this backend;
     * @return
     */
    virtual std::shared_ptr<DataBackend> Duplicate() const = 0;

    /**
     * @brief Get entities that are selected by the '''uri''',
     * @return if nothing is selected return nullptr
     */
    virtual std::shared_ptr<DataEntity> Get(std::string const& uri) const = 0;

    /**
     * @brief  put v to uri,
     * @return
     */
    virtual void Set(std::string const& uri, std::shared_ptr<DataEntity> const& v) = 0;

    /**
     * @brief  add v to uri,
     *          if uri does not exist then create an array
     *          else if uri is not an array then throw runtim error
     * @return
     */
    virtual void Add(std::string const& uri, std::shared_ptr<DataEntity> const&) = 0;
    /**
     * @brief  delete entities selected by uri
     * @return  the number of deleted entities
     */
    virtual size_type Delete(std::string const& uri) = 0;

    /**
     * @brief Get the number of entities in this table
     * @return
     */
    virtual size_type size() const = 0;
    /**
     * @brief '''for_each''' entities in this table
     * @return
     */
    virtual size_type Accept(std::function<void(std::string const&, std::shared_ptr<DataEntity>)> const&) const = 0;

};  // class DataBackend {

class DataBackendFactory : public design_pattern::Factory<std::string, DataBackend>, public concept::Printable {
    typedef design_pattern::Factory<std::string, DataBackend> base_type;

   public:
    DataBackendFactory();
    virtual ~DataBackendFactory();
    std::vector<std::string> RegisteredBackend() const;
    std::shared_ptr<DataBackend> Create(std::string const& uri, std::string const& ext_param = "");

    void RegisterDefault();
};

#define GLOBAL_DATA_BACKEND_FACTORY SingletonHolder<DataBackendFactory>::instance()
}  // namespace data {
}  // namespace simpla{
#endif  // SIMPLA_DATABACKEND_H
