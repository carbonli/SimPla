//
// Created by salmon on 17-8-18.
//

#include "DataNode.h"
#include <iomanip>
#include <map>
#include <vector>
#include "DataBase.h"
#include "DataEntity.h"
namespace simpla {
namespace data {

DataNode::DataNode() = default;
DataNode::~DataNode() = default;
std::shared_ptr<DataNode> DataNode::New(std::string const& s) { return data::DataBase::New(s)->Root(); }

std::shared_ptr<DataEntity> DataNode::Get() { return DataEntity::New(); }
std::shared_ptr<DataEntity> DataNode::Get() const { return DataEntity::New(); }
int DataNode::Set(std::shared_ptr<DataEntity> const& v) { return 0; }
int DataNode::Add(std::shared_ptr<DataEntity> const& v) { return AddNode()->Set(v); }
int DataNode::Set(std::shared_ptr<DataNode> const& v) {
    return v->Foreach([&](std::string k, std::shared_ptr<DataNode> node) {
        int count = 0;
        if (node->NodeType() == DataNode::DN_ENTITY) {
            count += GetNode(k, NEW_IF_NOT_EXIST)->Set(node->Get());
        } else {
            count += GetNode(k, NEW_IF_NOT_EXIST)->Set(node);
        }
        return count;
    });
};
int DataNode::Add(std::shared_ptr<DataNode> const& v) {
    return v->Foreach([&](std::string k, std::shared_ptr<DataNode> node) {
        int count = 0;
        if (node->NodeType() == DataNode::DN_ENTITY) {
            count += GetNode(k, NEW_IF_NOT_EXIST)->Add(node->Get());
        } else {
            count += GetNode(k, NEW_IF_NOT_EXIST)->Add(node);
        }
        return count;
    });
};

std::ostream& Print(std::ostream& os, std::shared_ptr<const DataNode> const& entry, int indent) {
    if (entry->NodeType() == DataNode::DN_ARRAY) {
        os << "[ ";
        bool is_first = true;
        bool new_line = entry->GetNumberOfChildren() > 1;
        entry->Foreach([&](auto k, auto v) {
            if (is_first) {
                is_first = false;
            } else {
                os << ", ";
            }
            if (new_line) { os << std::endl << std::setw(indent + 1) << " "; }
            Print(os, v, indent + 1);
            return 1;
        });
        os << " ]";
    } else if (entry->NodeType() == DataNode::DN_TABLE) {
        os << "{ ";
        bool is_first = true;
        bool new_line = entry->GetNumberOfChildren() > 1;
        auto count = entry->Foreach([&](auto k, auto v) {
            if (is_first) {
                is_first = false;
            } else {
                os << ", ";
            }
            if (new_line) { os << std::endl << std::setw(indent + 1) << " "; }
            os << "\"" << k << "\" = ";
            Print(os, v, indent + 1);
            return 1;
        });

        if (new_line) { os << std::endl << std::setw(indent) << " "; }
        os << "}";

    } else if (entry->NodeType() == DataNode::DN_ENTITY) {
        os << *entry->Get();
    }
    return os;
}
std::ostream& operator<<(std::ostream& os, DataNode const& entry) { return Print(os, entry.shared_from_this(), 0); }

static std::regex const sub_group_regex(R"(([^/?#]+)/)", std::regex::optimize);
static std::regex const match_path_regex(R"(^(/?([/\S]+/)*)?([^/]+)?$)", std::regex::optimize);

/**
 * @brief Traverse  a hierarchical table base on URI  example: /ab/c/d/e
 *           if ''' return_if_not_exist ''' return when sub table does not exist
 *           else create a new table
 * @tparam T  table type
 * @tparam FunCheckTable
 * @tparam FunGetTable
 * @tparam FunAddTable
 * @param self
 * @param uri
 * @param check check  if sub obj is a table
 * @param get  return sub table
 * @param add  create a new table
 * @param return_if_not_exist
 * @return
 */
std::pair<std::string, std::shared_ptr<DataNode>> RecursiveFindNode(std::shared_ptr<DataNode> root, std::string uri,
                                                                    int flag) {
    std::pair<std::string, std::shared_ptr<DataNode>> res{"", root};

    if (uri.empty() || uri == ".") { return res; }

    if (uri[0] == '/') {
        root = root->Root();
    } else if (uri.substr(0, 3) == "../") {
        root = root->Parent();
        uri = uri.substr(3);
    }
    std::smatch uri_match_result;

    if (!std::regex_match(uri, uri_match_result, match_path_regex)) {
        RUNTIME_ERROR << "illegal URI: [" << uri << "]" << std::endl;
    }
    std::string path = uri_match_result.str(2);

    if (!path.empty()) {
        std::smatch sub_match_result;
        auto t = root;

        for (auto pos = path.cbegin(), end = path.cend();
             std::regex_search(pos, end, sub_match_result, sub_group_regex); pos = sub_match_result.suffix().first) {
            std::string k = sub_match_result.str(1);
            res.second = t->GetNode(k, flag & (~DataNode::RECURSIVE));
            t = res.second;
            if (res.second == nullptr) {
                res.first = sub_match_result.suffix().str() + uri_match_result[3].str();
                break;
            }
        }
    }
    auto key = uri_match_result.str(3);
    if (!key.empty()) {
        res.first = "";
        res.second = res.second->GetNode(key, flag & (~DataNode::RECURSIVE));
    }
    return res;
};

}  // namespace data {
}  // namespace simpla {
