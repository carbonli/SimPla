//
// Created by salmon on 16-10-28.
//

#ifndef SIMPLA_DATAENTITY_H
#define SIMPLA_DATAENTITY_H

#include <simpla/SIMPLA_config.h>
#include <simpla/concept/Printable.h>
#include <simpla/engine/SPObjectHead.h>
#include <simpla/toolbox/Log.h>
#include <typeindex>
#include <vector>
#include "DataTraits.h"
namespace simpla {
namespace data {
template <typename, typename Enable = void>
class DataEntityWrapper {};
class DataEntity;
class DataArray;

struct DataEntity : public concept::Printable {
    SP_OBJECT_BASE(DataEntity);

   public:
    DataEntity();
    DataEntity(DataEntity const&) = delete;
    DataEntity(DataEntity&&) = delete;
    virtual ~DataEntity();

    virtual std::ostream& Print(std::ostream& os, int indent = 0) const {
        os << "null";
        return os;
    };
    virtual std::type_info const& type() const { return typeid(void); };
    virtual bool empty() const { return true; }

    virtual bool isEntity() const { return !(isTable() || isArray()); }
    virtual bool isTable() const { return false; }
    virtual bool isArray() const { return false; }
    //    DataTable& asTable() { return cast_as<DataTable>(); }
    //    DataTable const& asTable() const { return cast_as<DataTable>(); }
    //    DataArray& asArray() { return cast_as<DataArray>(); }
    //    DataArray const& asArray() const { return cast_as<DataArray>(); }

    virtual size_type Count() const { return 0; };
    virtual std::shared_ptr<DataEntity> Copy() const {};
    virtual std::shared_ptr<DataArray> MakeArray() const {};
    //    /** as Array */
    //    virtual std::shared_ptr<DataEntity> Get(size_type idx) const { return std::make_shared<DataEntity>(); }
    //    virtual bool Set(size_type idx, std::shared_ptr<DataEntity> const&) { return false; }
    //    virtual bool Add(std::shared_ptr<DataEntity> const&) { return false; }
    //    virtual int Delete(size_type idx) { return 0; }
    //
    //    /** as Table */
    //    virtual std::shared_ptr<DataEntity> Get(std::string const& key) const { return std::make_shared<DataEntity>();
    //    }
    //    virtual bool Set(std::string const& key, std::shared_ptr<DataEntity> const&) { return false; }
    //    virtual bool Add(std::string const& key, std::shared_ptr<DataEntity> const&) { return false; }
    //    virtual size_type Delete(std::string const& key) { return 0; }
    //
    //    template <typename Idx>
    //    std::shared_ptr<DataEntity> operator[](Idx const& idx) const {
    //        return Get(idx);
    //    }

    template <typename U>
    bool operator==(U const& v) const {
        return (type() == typeid(U)) && cast_as<DataEntityWrapper<U>>().equal(v);
    }

    template <typename U>
    U as() const {
        return cast_as<DataEntityWrapper<U>>().value();
    }
    template <typename U>
    U as(U const& u) const {
        try {
            return cast_as<DataEntityWrapper<U>>().value();
        } catch (...) { return u; }
    }
    //
    //    template <typename U>
    //    DataArrayWrapper<U>& asArray() {
    //        return cast_as<DataArrayWrapper<U>>();
    //    }
    //    template <typename U>
    //    DataArrayWrapper<U> const& asArray() const {
    //        return cast_as<DataArrayWrapper<U>>();
    //    }
};
template <typename U>
struct DataEntityWrapper<U, std::enable_if_t<traits::is_light_data<U>::value>> : public DataEntity {
    SP_OBJECT_HEAD(DataEntityWrapper<U>, DataEntity);
    typedef U value_type;

   public:
    DataEntityWrapper() {}
    DataEntityWrapper(value_type const& d) : m_data_(d) {}
    DataEntityWrapper(value_type&& d) : m_data_(d) {}
    virtual ~DataEntityWrapper() {}
    virtual std::type_info const& type() const { return typeid(value_type); }
    virtual std::ostream& Print(std::ostream& os, int indent = 0) const {
        if (typeid(U) == typeid(std::string)) {
            os << "\"" << value() << "\"";
        } else {
            os << value();
        }
        return os;
    }
    virtual bool equal(value_type const& other) const { return m_data_ == other; }
    virtual value_type value() const { return m_data_; }

   private:
    value_type m_data_;
};
template <typename U>
std::shared_ptr<DataEntity> make_data_entity(U const& u, ENABLE_IF(traits::is_light_data<U>::value)) {
    return std::dynamic_pointer_cast<DataEntity>(std::make_shared<DataEntityWrapper<U>>(u));
}

inline std::shared_ptr<DataEntity> make_data_entity(char const* u) {
    return std::dynamic_pointer_cast<DataEntity>(std::make_shared<DataEntityWrapper<std::string>>(u));
}
}  // namespace data {
}  // namespace simpla {
#endif  // SIMPLA_DATAENTITY_H
