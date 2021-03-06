//
// Created by salmon on 17-11-7.
//
#include <simpla/SIMPLA_config.h>
#include <simpla/application/SPInit.h>
#include <simpla/geometry/Box.h>
#include <simpla/geometry/CutCell.h>
#include <simpla/geometry/GeoEngine.h>
#include <simpla/geometry/GeoObject.h>
#include <simpla/geometry/Sweeping.h>
#include <simpla/geometry/csCartesian.h>
#include <simpla/geometry/csCylindrical.h>
#include <simpla/predefine/device/Tokamak.h>
#include <simpla/utilities/Constants.h>
#include <simpla/utilities/SPDefines.h>

namespace sp = simpla;
namespace sg = simpla::geometry;
using namespace simpla;
int main(int argc, char **argv) {
    sp::logger::set_stdout_level(1000);

    auto tokamak = sp::Tokamak::New("/home/salmon/workspace/SimPla/scripts/gfile/g038300.03900");
    sg::Initialize("OCE");
    auto b = sg::Box::New({{-2, -3, -4}, {2, 2, 2}});

    VERBOSE << "Box " << *b;
    VERBOSE << "Box Copy" << *b->Copy();
    GEO_ENGINE->OpenFile("tokamak.stl");
    auto limiter = sg::MakeRevolution(tokamak->Limiter(), -sp::PI / 4, sp::PI / 4);
    auto boundary = sg::MakeRevolution(tokamak->Boundary(), -sp::PI / 4, sp::PI / 4);

    GEO_ENGINE->Save(limiter, "Limiter");
    GEO_ENGINE->Save(boundary, "Boundary");

    //    auto chart = sg::csCartesian::New();
    //    auto cut_cell = sg::CutCell::New(limiter, chart);
    //
    //    sp::Array<unsigned int> node_tags{{-2, -2, -2}, {2, 2, 2}};
    //    cut_cell->TagCell(&node_tags, nullptr, 0b001);
    GEO_ENGINE->CloseFile();
    sg::Finalize();

    VERBOSE << "DONE";
}