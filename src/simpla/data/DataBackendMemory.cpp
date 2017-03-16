//
// Created by salmon on 17-3-6.
//
#include "DataBackendMemory.h"
#include <iomanip>
#include <map>
#include <regex>
#include "DataArray.h"
#include "DataEntity.h"
#include "DataTable.h"
#include "DataUtility.h"

namespace simpla {
namespace data {

struct DataBackendMemory::pimpl_s {
    typedef std::map<std::string, std::shared_ptr<DataEntity>> table_type;
    table_type m_table_;
    static std::pair<DataBackendMemory*, std::string> get_table(DataBackendMemory* self, std::string const& uri,
                                                                bool return_if_not_exist = true);
};

std::pair<DataBackendMemory*, std::string> DataBackendMemory::pimpl_s::get_table(DataBackendMemory* t,
                                                                                 std::string const& uri,
                                                                                 bool return_if_not_exist) {
    return HierarchicalTableForeach(
        t, uri,
        [&](DataBackendMemory* s_t, std::string const& k) -> bool {
            auto res = s_t->m_pimpl_->m_table_.find(k);
            return (res != s_t->m_pimpl_->m_table_.end()) && (res->second->isTable());
        },
        [&](DataBackendMemory* s_t, std::string const& k) {
            return &(std::dynamic_pointer_cast<DataTable>(s_t->m_pimpl_->m_table_.find(k)->second)
                         ->backend()
                         ->cast_as<DataBackendMemory>());
        },
        [&](DataBackendMemory* s_t, std::string const& k) -> DataBackendMemory* {
            if (return_if_not_exist) { return nullptr; }
            return &(
                s_t->m_pimpl_->m_table_.emplace(k, std::make_shared<DataTable>(std::make_shared<DataBackendMemory>()))
                    .first->second->cast_as<DataTable>()
                    .backend()
                    ->cast_as<DataBackendMemory>());

        });
};

DataBackendMemory::DataBackendMemory() : m_pimpl_(new pimpl_s) {}
DataBackendMemory::DataBackendMemory(std::string const& url, std::string const& status) : DataBackendMemory() {
    if (url != "") {
        DataTable d(url);
        d.ForEach([&](std::string const& k, std::shared_ptr<DataEntity> v) { Set(k, v); });
    }
}
DataBackendMemory::DataBackendMemory(const DataBackendMemory& other) : m_pimpl_(new pimpl_s) {
    std::map<std::string, std::shared_ptr<DataEntity>>(other.m_pimpl_->m_table_).swap(m_pimpl_->m_table_);
};
DataBackendMemory::DataBackendMemory(DataBackendMemory&& other) : m_pimpl_(new pimpl_s) {
    std::map<std::string, std::shared_ptr<DataEntity>>(other.m_pimpl_->m_table_).swap(m_pimpl_->m_table_);
};
DataBackendMemory::~DataBackendMemory() {}

std::shared_ptr<DataBackend> DataBackendMemory::CreateNew() const { return std::make_shared<DataBackendMemory>(); }

std::shared_ptr<DataBackend> DataBackendMemory::Duplicate() const { return std::make_shared<DataBackendMemory>(*this); }

void DataBackendMemory::Flush() {}

std::ostream& DataBackendMemory::Print(std::ostream& os, int indent) const { return os; };

bool DataBackendMemory::isNull() const { return m_pimpl_ == nullptr; };
size_type DataBackendMemory::size() const { return m_pimpl_->m_table_.size(); }

std::shared_ptr<DataEntity> DataBackendMemory::Get(std::string const& url) const {
    auto res = m_pimpl_->get_table(const_cast<DataBackendMemory*>(this), url);
    if (res.first != nullptr || res.second != "") {
        auto it = res.first->m_pimpl_->m_table_.find(res.second);
        if (it != res.first->m_pimpl_->m_table_.end()) { return it->second; }
    }

    return nullptr;
};

std::pair<std::shared_ptr<DataEntity>, bool> DataBackendMemory::Set(std::string const& uri,
                                                                    std::shared_ptr<DataEntity> const& v,
                                                                    bool overwrite) {
    auto tab_res = pimpl_s::get_table((this), uri, overwrite);
    if (tab_res.second == "") {
        return std::make_pair(std::make_shared<DataTable>(tab_res.first->shared_from_this()), false);
    } else {
        auto res = tab_res.first->m_pimpl_->m_table_.emplace(tab_res.second, v);
        if (!res.second && overwrite) {
            res.first->second = v;
            return std::make_pair(res.first->second, true);
        } else {
            return std::make_pair(res.first->second, false);
        }
    }
}
std::shared_ptr<DataEntity> DataBackendMemory::Add(std::string const& uri, std::shared_ptr<DataEntity> const& v) {
    auto tab_res = pimpl_s::get_table(const_cast<DataBackendMemory*>(this), uri, false);
    if (tab_res.second == "") { return std::make_shared<DataTable>(tab_res.first->shared_from_this()); }
    auto res = tab_res.first->m_pimpl_->m_table_.emplace(tab_res.second, std::make_shared<DataArrayWrapper<void>>());
    if (res.first->second->isArray() && res.first->second->type() == v->type()) {
    } else if (!res.first->second->isA<DataArrayWrapper<void>>()) {
        auto t_array = std::make_shared<DataArrayWrapper<void>>();
        t_array->Add(res.first->second);
        res.first->second = t_array;
    }
    std::dynamic_pointer_cast<DataArray>(res.first->second)->Add(v);

    return res.first->second;
}
size_type DataBackendMemory::Delete(std::string const& uri) {
    auto res = m_pimpl_->get_table(const_cast<DataBackendMemory*>(this), uri);
    return (res.first != nullptr && res.second != "") ? res.first->m_pimpl_->m_table_.erase(res.second) : 0;
}

size_type DataBackendMemory::ForEach(
    std::function<void(std::string const&, std::shared_ptr<DataEntity>)> const& f) const {
    for (auto const& item : m_pimpl_->m_table_) { f(item.first, item.second); }
}

}  // namespace data {
}  // namespace simpla{