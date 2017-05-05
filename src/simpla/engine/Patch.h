//
// Created by salmon on 16-12-8.
//

#ifndef SIMPLA_PATCH_H
#define SIMPLA_PATCH_H

#include <simpla/SIMPLA_config.h>
#include <simpla/data/all.h>
#include <simpla/geometry/GeoObject.h>
#include <simpla/utilities/EntityId.h>
#include <memory>
#include "SPObject.h"
namespace simpla {

namespace engine {
class MeshBlock;
class Chart;
class Patch {
    SP_OBJECT_BASE(Patch)
   public:
    explicit Patch(id_type id = NULL_ID);
    virtual ~Patch();

    SP_DEFAULT_CONSTRUCT(Patch);
    id_type GetId() const;

    void SetChart(std::shared_ptr<Chart>);
    std::shared_ptr<Chart> GetChart() const;

    void SetBlock(std::shared_ptr<MeshBlock> const &);
    std::shared_ptr<MeshBlock> GetBlock() const;
    void Merge(Patch &other);
    std::map<id_type, std::shared_ptr<data::DataBlock>> &GetAllData();

    int Push(id_type const &id, std::shared_ptr<data::DataBlock> const &);
    std::shared_ptr<data::DataBlock> Pop(id_type const &id) const;

    EntityRange const *GetRange(const std::string &g_str = "") const;
    void SetRange(EntityRange const *, const std::string &g_str = "");

    void PushRange(std::shared_ptr<std::map<std::string, nTuple<EntityRange, 4>>> const &r);
    std::shared_ptr<std::map<std::string, nTuple<EntityRange, 4>>> PopRange();

   private:
    struct pimpl_s;
    std::unique_ptr<pimpl_s> m_pimpl_;
};
}  // namespace engine
}  // namespace simpla

#endif  // SIMPLA_PATCH_H
