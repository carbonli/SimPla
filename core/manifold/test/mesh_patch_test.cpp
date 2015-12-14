/**
 * @file mesh_layout_test.cpp
 * @author salmon
 * @date 2015-12-13.
 */

#include <gtest/gtest.h>
#include "../mesh/mesh_patch.h"
#include "../pre_define/predefine.h"

#include "../../field/field.h"
#include "../../gtl/utilities/log.h"

using namespace simpla;

TEST(MeshMultiBlock, RectMesh)
{
    typedef manifold::CartesianManifold mesh_type;

    mesh_type m;

    auto l_box = m.box();

    traits::field_t<Real, mesh_type, VERTEX> f0{m};
    traits::field_t<Real, mesh_type, EDGE> f1a{m};
    traits::field_t<Real, mesh_type, EDGE> f1b{m};

    f0.clear();
    f1a.clear();
    f1b.clear();

    auto it = m.insert_patch(l_box);

    m.time_integral(1.0, f1a, f1b * f0);

//    m.erase_patch(std::get<0>(it));


}