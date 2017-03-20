//
// Created by salmon on 16-6-2.
//

#include "Model.h"
#include <simpla/mesh/EntityId.h>
#include "AttributeView.h"
#include "MeshBlock.h"
namespace simpla {
namespace engine {

struct Model::pimpl_s {
    std::map<std::string, std::shared_ptr<geometry::GeoObject>> m_g_obj_;
    box_type m_bound_box_{{0, 0, 0}, {1, 1, 1}};
};

Model::Model() : m_pimpl_(new pimpl_s) {}
Model::~Model() {}
std::ostream& Model::Print(std::ostream& os, int indent) const {
    os << *db() << std::endl;
    return os;
}
void Model::Initialize() {
    LOGGER << "Model is initializing " << std::endl;
    Tag();
    LOGGER << "Model is initialized " << std::endl;
}

bool Model::Update() {
    // TODO: update bound box
    return SPObject::Update();
};
box_type const& Model::bound_box() const { return m_pimpl_->m_bound_box_; };

id_type Model::GetMaterialId(std::string const& k) const { return GetMaterial(k)->GetValue<id_type>("GUID"); }

std::shared_ptr<data::DataTable> Model::GetMaterial(std::string const& s) const { return db()->GetTable(s); }

std::shared_ptr<data::DataTable> Model::SetMaterial(std::string const& s, std::shared_ptr<DataTable> const& other) {
    Click();
    if (db()->Set("/Material/" + s, other, false) > 0) {
        db()->Get("/Material/" + s)->cast_as<DataTable>().SetValue("GUID", std::hash<std::string>{}(s));
    }
}
// id_type Model::GetMaterialId(std::string const& k) const { return GetMaterial(k).GetValue<id_type>("GUID", NULL_ID);
// }
// id_type Model::GetMaterialId(std::string const& k) { return GetMaterial(k).GetValue<id_type>("GUID"); }

// id_type Model::AddObject(id_type id, geometry::GeoObject const &g_obj) {
//    Click();
//    m_pimpl_->m_g_obj_.emplace(id, g_obj);
//}

id_type Model::AddObject(std::string const& key, std::shared_ptr<geometry::GeoObject> const& g_obj) {
    Click();
    m_pimpl_->m_g_obj_.emplace(key, g_obj);
}

std::shared_ptr<geometry::GeoObject> Model::GetObject(std::string const& k) const {
    auto it = m_pimpl_->m_g_obj_.find(k);
    return it == m_pimpl_->m_g_obj_.end() ? nullptr : it->second;
}

size_type Model::DeleteObject(std::string const& key) { return m_pimpl_->m_g_obj_.erase(key); }

}  // namespace engine
}  // namespace simpla{;