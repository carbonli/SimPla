//
// Created by salmon on 17-3-2.
//
#include <simpla/data/DataFunction.h>

#include "../DataEntity.h"
#include "../DataNode.h"
#include "../DataTraits.h"

#include "../DataUtilities.h"
#include "LuaObject.h"
namespace simpla {
namespace data {
struct DataNodeLua : public DataNode {
    SP_DEFINE_FANCY_TYPE_NAME(DataNodeLua, DataNode)
    SP_DATA_NODE_HEAD(DataNodeLua);
    SP_DATA_NODE_FUNCTION(DataNodeLua);

   protected:
    explicit DataNodeLua(eNodeType e_type) : DataNode(e_type) {}

   public:
    int Connect(std::string const& authority, std::string const& path, std::string const& query,
                std::string const& fragment) override;
    int Disconnect() override;
    int Parse(std::string const& str) override;
    //    int Flush() override;
    bool isValid() const override;
    //    void Clear() override;

    std::shared_ptr<LuaObject> m_lua_obj_ = nullptr;

    void init();
};
REGISTER_CREATOR(DataNodeLua, lua);

std::shared_ptr<DataNode> DataNodeLua::CreateNode(eNodeType e_type) const {
    std::shared_ptr<DataNode> res = nullptr;
    switch (e_type) {
        case DN_ENTITY:
            res = DataNode::New();
            break;
        case DN_ARRAY:
            res = DataNodeLua::New(DN_ARRAY);
            break;
        case DN_TABLE:
            res = DataNodeLua::New(DN_TABLE);
            break;
        case DN_FUNCTION:
            break;
        case DN_NULL:
        default:
            break;
    }
    res->SetParent(Self());
    return res;
};
struct DataFunctionLua : public DataFunction {
    SP_DEFINE_FANCY_TYPE_NAME(DataFunctionLua, DataFunction);
    std::shared_ptr<LuaObject> m_lua_obj_;

   protected:
    DataFunctionLua() = default;
    explicit DataFunctionLua(std::shared_ptr<LuaObject> const& lobj) : m_lua_obj_(lobj) {}

   public:
    ~DataFunctionLua() override = default;
    DataFunctionLua(DataFunctionLua const& other) = delete;
    DataFunctionLua(DataFunctionLua&& other) = delete;

    template <typename... Args>
    static std::shared_ptr<DataFunctionLua> New(Args&&... args) {
        return std::shared_ptr<DataFunctionLua>(new DataFunctionLua(std::forward<Args>(args)...));
    }
    std::shared_ptr<DataEntity> eval(std::initializer_list<std::shared_ptr<DataEntity>> const& args) const override {
        return DataEntity::New();
    };
};
void DataNodeLua::init() {
    if (m_lua_obj_ == nullptr) {
        auto tmp = LuaObject::New();
        m_lua_obj_ = tmp->new_table("_ROOT_");
    }
}

DataNodeLua::DataNodeLua() : DataNode(DN_TABLE) { init(); }
DataNodeLua::~DataNodeLua() { m_lua_obj_.reset(); }
int DataNodeLua::Connect(std::string const& authority, std::string const& path, std::string const& query,
                         std::string const& fragment) {
    if (!path.empty()) {
        auto tmp = LuaObject::New();
        tmp->parse_file(path);
        m_lua_obj_ = tmp->get(query.empty() ? "_ROOT_" : query);
    }
    return SP_SUCCESS;
}
int DataNodeLua::Disconnect() { return 0; }

int DataNodeLua::Parse(std::string const& str) {
    init();
    m_lua_obj_->parse_string(str);
    return SP_SUCCESS;
};

bool DataNodeLua::isValid() const { return m_lua_obj_ != nullptr; }

size_type DataNodeLua::size() const { return m_lua_obj_ != nullptr ? m_lua_obj_->size() : 0; }

std::shared_ptr<DataNode> LuaCreateNode(std::shared_ptr<DataNodeLua> const& parent,
                                        std::shared_ptr<LuaObject> const& tobj) {
    if (parent == nullptr || tobj == nullptr) { return nullptr; }
    std::shared_ptr<DataNode> res = nullptr;

    switch (tobj->type()) {
        case LuaObject::LUA_T_BOOLEAN:
            res = DataNode::New(DataLight::New(tobj->as<bool>()));
            break;
        case LuaObject::LUA_T_INTEGER:
            res = DataNode::New(DataLight::New(tobj->as<int>()));
            break;
        case LuaObject::LUA_T_FLOATING:
            res = DataNode::New(DataLight::New(tobj->as<double>()));
            break;
        case LuaObject::LUA_T_STRING:
            res = DataNode::New(DataLight::New(tobj->as<std::string>()));
            break;
        case LuaObject::LUA_T_TABLE:
            res = parent->CreateNode(DataNode::DN_TABLE);
            std::dynamic_pointer_cast<DataNodeLua>(res)->m_lua_obj_ = tobj;
            break;
        case LuaObject::LUA_T_FUNCTION:
            res = parent->CreateNode(DataNode::DN_FUNCTION);
            std::dynamic_pointer_cast<DataNodeLua>(res)->m_lua_obj_ = tobj;
            break;
        case LuaObject::LUA_T_ARRAY: {
            res = parent->CreateNode(DataNode::DN_ENTITY);

            switch (tobj->get_array_value_type()) {
                case LuaObject::LUA_T_BOOLEAN: {
                    size_type rank = 0;
                    size_type extents[MAX_NDIMS_OF_ARRAY] = {0, 0, 0, 0, 0, 0, 0, 0};
                    tobj->get_value(static_cast<bool*>(nullptr), &rank, extents);
                    auto tmp = DataLightT<bool*>::New(rank, extents);
                    tobj->get_value(tmp->pointer(), &rank, extents);
                    res->SetEntity(tmp);
                } break;
                case LuaObject::LUA_T_INTEGER: {
                    size_type rank = 0;
                    size_type extents[MAX_NDIMS_OF_ARRAY] = {0, 0, 0, 0, 0, 0, 0, 0};
                    tobj->get_value(static_cast<int*>(nullptr), &rank, extents);
                    auto tmp = DataLightT<int*>::New(rank, extents);
                    tobj->get_value(tmp->pointer(), &rank, extents);
                    res->SetEntity(tmp);
                } break;
                case LuaObject::LUA_T_FLOATING: {
                    size_type rank = 0;
                    size_type extents[MAX_NDIMS_OF_ARRAY] = {0, 0, 0, 0, 0, 0, 0, 0};
                    tobj->get_value(static_cast<double*>(nullptr), &rank, extents);
                    auto tmp = DataLightT<double*>::New(rank, extents);
                    tobj->get_value(tmp->pointer(), &rank, extents);
                    res->SetEntity(tmp);
                } break;
                case LuaObject::LUA_T_STRING: {
                    size_type rank = 0;
                    size_type extents[MAX_NDIMS_OF_ARRAY] = {0, 0, 0, 0, 0, 0, 0, 0};
                    tobj->get_value(static_cast<std::string*>(nullptr), &rank, extents);
                    auto tmp = DataLightT<std::string*>::New(rank, extents);
                    tobj->get_value(tmp->pointer(), &rank, extents);
                    res->SetEntity(tmp);
                } break;
                default:
                    break;
            }
        }
        default:
            break;
    }
    return res;
}
size_type DataNodeLua::Foreach(std::function<size_type(std::string, std::shared_ptr<DataNode>)> const& fun) const {
    if (m_lua_obj_ == nullptr) { return 0; }
    size_type count = 0;
    for (auto p : *m_lua_obj_) { count += fun(p.first->as<std::string>(), LuaCreateNode(Self(), p.second)); }
    return count;
}

std::shared_ptr<DataNode> DataNodeLua::Get(std::string const& uri) const {
    if (uri.empty()) { return nullptr; }
    if (uri[0] == SP_URL_SPLIT_CHAR) { return Root()->Get(uri.substr(1)); }

    auto obj = const_cast<this_type*>(this)->shared_from_this();
    std::string k = uri;
    while (obj != nullptr && !k.empty()) {
        auto tail = k.find(SP_URL_SPLIT_CHAR);
        if (auto p = std::dynamic_pointer_cast<this_type>(obj)) {
            obj = LuaCreateNode(Self(), p->m_lua_obj_->get(k.substr(0, tail)));
        } else {
            obj = nullptr;  // obj->Get(k.substr(0, tail));
        }
        if (tail != std::string::npos) {
            k = k.substr(tail + 1);
        } else {
            k = "";
        };
    }
    return obj;
}
size_type LuaSetEntity(std::shared_ptr<LuaObject> const& lobj, std::string const& k,
                       std::shared_ptr<DataNode> const& n) {
    if (lobj == nullptr || n == nullptr || k.empty()) { return 0; }
    size_type count = 0;
    auto entity = n->GetEntity();
#define DEFINE_MULTIMETHOD(_T_)                                              \
    else if (auto p = std::dynamic_pointer_cast<DataLightT<_T_>>(entity)) {  \
        count = lobj->set(k, p->pointer(), 0, nullptr);                      \
    }                                                                        \
    else if (auto p = std::dynamic_pointer_cast<DataLightT<_T_*>>(entity)) { \
        size_type rank = p->rank();                                          \
        size_type extents[MAX_NDIMS_OF_ARRAY];                               \
        p->extents(extents);                                                 \
        count = lobj->set(k, p->pointer(), rank, extents);                   \
    }

    if (auto p = std::dynamic_pointer_cast<DataBlock>(entity)) { FIXME << "LUA backend do not support DataBlock "; }
    DEFINE_MULTIMETHOD(std::string)
    DEFINE_MULTIMETHOD(bool)
    DEFINE_MULTIMETHOD(float)
    DEFINE_MULTIMETHOD(double)
    DEFINE_MULTIMETHOD(int)
    DEFINE_MULTIMETHOD(long)
    DEFINE_MULTIMETHOD(unsigned int)
    DEFINE_MULTIMETHOD(unsigned long)

#undef MULTIMETHOD_T_
    return count;
}
size_type DataNodeLua::Set(std::string const& uri, std::shared_ptr<DataNode> const& entity) {
    if (uri.empty() || entity == nullptr) { return 0; }
    if (uri[0] == SP_URL_SPLIT_CHAR) { return Root()->Set(uri.substr(1), entity); }

    size_type count = 0;
    auto obj = shared_from_this();
    std::string k = uri;
    while (obj != nullptr && !k.empty()) {
        size_type tail = k.find(SP_URL_SPLIT_CHAR);
        if (auto lobj = std::dynamic_pointer_cast<this_type>(obj)) {
            if (tail == std::string::npos) {
                count = LuaSetEntity(lobj->m_lua_obj_, k, entity);  // insert_or_assign
                break;
            } else {
                auto t_table = lobj->m_lua_obj_->get(k.substr(0, tail));
                if (t_table == nullptr) { t_table = lobj->m_lua_obj_->new_table(k.substr(0, tail)); }
                obj = CreateNode(DN_TABLE);
                std::dynamic_pointer_cast<this_type>(obj)->m_lua_obj_ = t_table;
                k = k.substr(tail + 1);
            }
        }
    }
    return count;
    //    if (uri.empty() || entity == nullptr) { return 0; }
    //    if (uri[0] == SP_URL_SPLIT_CHAR) { return Root()->Set(uri.substr(1), entity); }
    //    size_type tail = 0;
    //    auto lobj = m_lua_obj_;
    //    std::string k = uri;
    //    while (lobj != nullptr && tail != std::string::npos) {
    //        tail = k.find(SP_URL_SPLIT_CHAR);
    //
    //        if (tail != std::string::npos) {
    //            auto p = lobj->get(k.substr(0, tail));
    //            lobj = (p != nullptr) ? p : lobj->new_table(k.substr(0, tail));
    //            k = k.substr(tail + 1);
    //        }
    //    }
    //
    //    return LuaSetEntity(lobj, k, entity);
}
size_type DataNodeLua::Add(std::string const& uri, std::shared_ptr<DataNode> const& v) { return 0; }

size_type DataNodeLua::Delete(std::string const& uri) { return 0; /*  m_lua_obj_->erase(uri);*/ }

size_type DataNodeLua::Set(size_type s, std::shared_ptr<DataNode> const& v) { return 0; }

size_type DataNodeLua::Add(size_type s, std::shared_ptr<DataNode> const& v) { return 0; }

size_type DataNodeLua::Delete(size_type s) { return 0; /*  m_lua_obj_->erase(uri);*/ }

std::shared_ptr<DataNode> DataNodeLua::Get(size_type s) const {
    std::shared_ptr<DataNode> res = nullptr;
    auto tobj = m_lua_obj_->get(s);
    if (tobj == nullptr) { return nullptr; }

    return res;
}
//
// int DataNodeLua::SetEntity(std::string const& uri, const std::shared_ptr<DataEntity>& v) {
//    if (v == nullptr) { m_lua_obj_->parse_string(uri); }
//    return 1;
//}
//
// int DataNodeLua::AddEntity(std::string const& key, const std::shared_ptr<DataEntity>& v) {
//    UNIMPLEMENTED;
//    return 0;
//}
// int DataNodeLua::Delete(std::string const& key) {
//    UNIMPLEMENTED;
//    return 0;
//}
// bool DataNodeLua::isNull() const { return m_lua_obj_->is_nil(); }
//
// int DataNodeLua::Foreach(std::function<int(std::string const&, std::shared_ptr<DataEntity>)> const& f)
// const {
//    int counter = 0;
//    if (  m_lua_obj_->is_global()) {
//        UNSUPPORTED;
//    } else {
//        for (auto const& item : m_lua_obj_) {
//            if (item.first.is_string()) {
//                counter += f(item.first.as<std::string>(), this->GetEntity(item.first.as<std::string>()));
//            }
//        };
//    }
//    return counter;
//}
//
// std::shared_ptr<DataEntity> DataNodeLua::Serialize(std::string const& url) {
//    auto obj = m_pack_->m_lua_obj_->get(url);
//    if (obj.is_floating_point()) {
//        return std::make_shared<DataEntityLua<double>>(obj);
//    } else if (obj.is_integer()) {
//        return std::make_shared<DataEntityLua<int>>(obj);
//    } else if (obj.is_string()) {
//        return std::make_shared<DataEntityLua<std::string>>(obj);
//    } else if (obj.is_table()) {
//        auto database = std::make_shared<DataNodeLua>();
//        database->m_pack_->m_lua_obj_ = obj;
//        return std::dynamic_pointer_cast<DataEntity>(std::make_shared<DataTable>(database));
//    } else {
//        RUNTIME_ERROR << "Parse error! url=" << url << ":" << obj.get_typename() << std::endl;
//    }
//};
// std::shared_ptr<DataEntity> DataNodeLua::Serialize(std::string const& url) const {
//    auto obj = m_pack_->m_lua_obj_->get(url);
//    ASSERT(!obj.empty());
//    if (obj.is_floating_point()) {
//        return std::make_shared<DataEntityLua<double>>(obj);
//    } else if (obj.is_integer()) {
//        return std::make_shared<DataEntityLua<int>>(obj);
//    } else if (obj.is_string()) {
//        return std::make_shared<DataEntityLua<std::string>>(obj);
//    } else if (obj.is_table()) {
//        auto database = std::make_shared<DataNodeLua>();
//        database->m_pack_->m_lua_obj_ = obj;
//        return std::dynamic_pointer_cast<DataEntity>(std::make_shared<DataTable>(database));
//    } else {
//        RUNTIME_ERROR << "Parse error! url=" << url << std::endl;
//    }
//}

}  // namespace data{
}  // namespace simpla