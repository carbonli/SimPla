/**
 * @file MeshAtlas.h
 * @author salmon
 * @date 2016-05-19.
 */

#ifndef SIMPLA_MESH_MESHATLAS_H
#define SIMPLA_MESH_MESHATLAS_H

#include "Mesh.h"
#include "MeshBase.h"
#include "../gtl/Log.h"

namespace simpla { namespace mesh
{


/**
 *  manager of mesh blocks
 *  - adjacencies (graph, ?r-tree)
 *  - refine and coarsen
 *  - coordinates map on overlap region
 */
class MeshAtlas
{
    int m_level_ratio_;
    MeshBlockId m_root_;

    std::map<MeshBlockId, std::shared_ptr<MeshBase> > m_mesh_atlas_;
    int m_max_level_ = 1;
public:
    enum MESH_STATUS
    {
        LOCAL = 1, // 001
        ADJACENT = 2, // 010

    };

    template<typename T, typename ...Args>
    std::shared_ptr<T> attribute(Args &&...args) const
    {
//        static_assert(std::is_base_of<MeshAttributeBase, T>::value);

        return std::make_shared<T>(*this, std::forward<Args>(args)...);

    }


    std::vector<MeshBlockId> adjacent_blocks(MeshBlockId const &id, int inc_level = 0, int status_flag = 0);

    std::vector<MeshBlockId> find(int level = 0, int status_flag = 0);

    int count(int level = 0, int status_flag = 0);

    /**return the id of  root block*/
    MeshBlockId root() const { };

    void set(MeshBlockId const &id, std::shared_ptr<MeshBase> ptr)
    {
        m_mesh_atlas_[id].swap(ptr);
    };

    std::shared_ptr<MeshBase> at(MeshBlockId const &id) const
    {
        return m_mesh_atlas_.at(id);
//        if (res != m_mesh_atlas_.end())
//        {
//            return *res;
//        }
//        else
//        {
//            return std::shared_ptr<MeshBase>(nullptr);
//        }
    }

    template<typename TM>
    TM const *at(MeshBlockId const &id) const
    {
        auto res = m_mesh_atlas_.at(id);

        if (!res->base::Object::template is_a<TM>()) { BAD_CAST << ("illegal mesh type conversion!") << std::endl; }

        return std::dynamic_pointer_cast<const TM>(res).get();
    }

    int level_ratio() const
    {
        return m_level_ratio_;
    }

    void level_ratio(int m_level_ratio_)
    {
        MeshAtlas::m_level_ratio_ = m_level_ratio_;
    }

    int max_level() const
    {
        return m_max_level_;
    }


    MeshBlockId add(box_type const &b, int level = 0)
    {
        m_max_level_ = std::max(m_max_level_, level);
//        MeshBlockId uuid = get_id();
//        m_mesh_atlas_.emplace(std::make_pair(uuid, ptr));
        UNIMPLEMENTED;
        return 0;
    };

    void remove(MeshBlockId const &id)
    {
        m_mesh_atlas_.erase(id);
        while (count(m_max_level_ - 1) == 0)
        {
            --m_max_level_;
            if (m_max_level_ == 0)break;
        }
    }


};

}}//namespace simpla{namespace mesh{

#endif //SIMPLA_MESH_MESHATLAS_H
