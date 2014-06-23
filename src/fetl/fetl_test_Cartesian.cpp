/*
 * fetl_test.cpp
 *
 *  Created on: 2013年12月28日
 *      Author: salmon
 */

//#include "../mesh/mesh_rectangle.h"
//#include "../mesh/octree_forest.h"
//#include "../mesh/geometry_cartesian.h"
//
//#define TMESH Mesh<CartesianGeometry<OcForest>>
#include <gtest/gtest.h>
#include "fetl.h"
#include "fetl_test.h"
//#include "fetl_test1.h"
//#include "fetl_test2.h"
#include "fetl_test3.h"
//#include "fetl_test4.h"

using namespace simpla;

INSTANTIATE_TEST_CASE_P(FETLEuclidean, TestFETL,

testing::Combine(testing::Values(nTuple<3, Real>( { 0.0, 0.0, 0.0, })  //
//        , nTuple<3, Real>( { -1.0, -2.0, -3.0 })

        ),

testing::Values(

nTuple<3, Real>( { 5.0, 2.0, 3.0 })  //
//        , nTuple<3, Real>( { 2.0, 0.0, 0.0 }) //
//        , nTuple<3, Real>( { 0.0, 2.0, 0.0 }) //
//        , nTuple<3, Real>( { 0.0, 0.0, 2.0 }) //
//        , nTuple<3, Real>( { 0.0, 2.0, 2.0 }) //
//        , nTuple<3, Real>( { 2.0, 0.0, 2.0 }) //
//        , nTuple<3, Real>( { 2.0, 2.0, 0.0 }) //

        ),

testing::Values(

nTuple<3, size_t>( { 102, 103, 105 }) //
        , nTuple<3, size_t>( { 127, 1, 1 }) //
        , nTuple<3, size_t>( { 1, 127, 1 }) //
        , nTuple<3, size_t>( { 1, 1, 100 }) //
        , nTuple<3, size_t>( { 1, 100, 200 }) //
        , nTuple<3, size_t>( { 127, 1, 127 }) //
        , nTuple<3, size_t>( { 127, 127, 1 }) //
        , nTuple<3, size_t>( { 102, 106, 100 })   //

        )

        ));
