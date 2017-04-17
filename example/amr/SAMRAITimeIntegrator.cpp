//
// Created by salmon on 16-10-24.
//

// Headers for SimPla
#include <simpla/SIMPLA_config.h>
#include <boost/shared_ptr.hpp>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include "simpla/algebra/all.h"
#include "simpla/data/all.h"
#include "simpla/engine/all.h"
#include "simpla/mesh/MeshCommon.h"
#include "simpla/parallel/MPIComm.h"
#include "simpla/toolbox/Log.h"
// Headers for SAMRAI
#include <SAMRAI/SAMRAI_config.h>

#include <SAMRAI/algs/HyperbolicLevelIntegrator.h>
#include <SAMRAI/algs/TimeRefinementIntegrator.h>
#include <SAMRAI/algs/TimeRefinementLevelStrategy.h>

#include <SAMRAI/mesh/BergerRigoutsos.h>
#include <SAMRAI/mesh/CascadePartitioner.h>
#include <SAMRAI/mesh/CascadePartitioner.h>
#include <SAMRAI/mesh/GriddingAlgorithm.h>
#include <SAMRAI/mesh/StandardTagAndInitialize.h>

#include <SAMRAI/hier/BoundaryBox.h>
#include <SAMRAI/hier/BoxContainer.h>
#include <SAMRAI/hier/Index.h>
#include <SAMRAI/hier/PatchDataRestartManager.h>
#include <SAMRAI/hier/PatchHierarchy.h>
#include <SAMRAI/hier/PatchLevel.h>
#include <SAMRAI/hier/VariableDatabase.h>

#include <SAMRAI/geom/CartesianGridGeometry.h>
#include <SAMRAI/geom/CartesianPatchGeometry.h>

#include <SAMRAI/pdat/CellData.h>
#include <SAMRAI/pdat/CellIndex.h>
#include <SAMRAI/pdat/CellIterator.h>
#include <SAMRAI/pdat/CellVariable.h>
#include <SAMRAI/pdat/CellVariable.h>
#include <SAMRAI/pdat/EdgeVariable.h>
#include <SAMRAI/pdat/FaceData.h>
#include <SAMRAI/pdat/FaceIndex.h>
#include <SAMRAI/pdat/FaceVariable.h>
#include <SAMRAI/pdat/NodeVariable.h>

#include <SAMRAI/tbox/BalancedDepthFirstTree.h>
#include <SAMRAI/tbox/Database.h>
#include <SAMRAI/tbox/InputDatabase.h>
#include <SAMRAI/tbox/InputManager.h>
#include <SAMRAI/tbox/MathUtilities.h>
#include <SAMRAI/tbox/PIO.h>
#include <SAMRAI/tbox/PIO.h>
#include <SAMRAI/tbox/RestartManager.h>
#include <SAMRAI/tbox/RestartManager.h>
#include <SAMRAI/tbox/SAMRAIManager.h>
#include <SAMRAI/tbox/SAMRAI_MPI.h>
#include <SAMRAI/tbox/Utilities.h>

#include <SAMRAI/appu/BoundaryUtilityStrategy.h>
#include <SAMRAI/appu/CartesianBoundaryDefines.h>
#include <SAMRAI/appu/CartesianBoundaryDefines.h>
#include <SAMRAI/appu/CartesianBoundaryUtilities2.h>
#include <SAMRAI/appu/CartesianBoundaryUtilities3.h>
#include <SAMRAI/appu/VisItDataWriter.h>
#include <SAMRAI/pdat/NodeDoubleLinearTimeInterpolateOp.h>
#include <SAMRAI/pdat/SideVariable.h>
#include <simpla/data/DataBackend.h>
#include <simpla/engine/TimeIntegrator.h>
#include <simpla/physics/Constants.h>
namespace simpla {

class SAMRAITimeIntegrator;

// static bool Mesh_CartesianGeometry_IS_REGISTERED =
// Chart::RegisterCreator<mesh::CartesianGeometry>("CartesianGeometry");
////    GLOBAL_DOMAIN_FACTORY::RegisterMeshCreator<mesh::CylindricalGeometry>("CartesianGeometry");
// static bool CartesianGeometry_EMFluid_IS_REGISTERED =
//    Worker::RegisterCreator<EMFluid<mesh::CartesianGeometry>>("CartesianGeometry.EMFluid");
//        GLOBAL_WORKER_FACTORY.RegisterCreator<PML<mesh::CartesianGeometry>>("CartesianGeometry.PML");
//
// struct SAMRAIPatchProxy : public data::DataTable {
//   public:
//    SAMRAIPatchProxy(SAMRAI::hier::Patch &patch, boost::shared_ptr<SAMRAI::hier::VariableContext> ctx,
//                     std::map<id_type, std::shared_ptr<data::DataTable>> const &simpla_attrs_,
//                     std::map<id_type, boost::shared_ptr<SAMRAI::hier::Variable>> const &samrai_variables);
//    virtual ~SAMRAIPatchProxy();
//    std::shared_ptr<data::DataBlock> data(id_type const &id, std::shared_ptr<data::DataBlock> const &p = nullptr);
//    std::shared_ptr<data::DataBlock> data(id_type const &id) const;
//
//   private:
//    SAMRAI::hier::Patch &m_samrai_patch_;
//    boost::shared_ptr<SAMRAI::hier::VariableContext> const &m_samrai_ctx_;
//    std::map<id_type, std::shared_ptr<data::DataTable>> const &m_simpla_attrs_;
//    std::map<id_type, boost::shared_ptr<SAMRAI::hier::Variable>> const &m_samrai_variables_;
//};
// SAMRAIPatchProxy::SAMRAIPatchProxy(SAMRAI::hier::Patch &patch, boost::shared_ptr<SAMRAI::hier::VariableContext> ctx,
//                                   std::map<id_type, std::shared_ptr<data::DataTable>> const &simpla_attrs_,
//                                   std::map<id_type, boost::shared_ptr<SAMRAI::hier::Variable>> const &var_map)
//    : m_samrai_patch_(patch), m_samrai_ctx_(ctx), m_simpla_attrs_(simpla_attrs_), m_samrai_variables_(var_map) {
//    auto pgeom = boost::dynamic_pointer_cast<SAMRAI::geom::CartesianPatchGeometry>(patch.getPatchGeometry());
//
//    ASSERT(pgeom != nullptr);
//    const double *dx = pgeom->getDx();
//    const double *xlo = pgeom->getXLower();
//    const double *xhi = pgeom->getXUpper();
//
//    index_type lo[3] = {patch.getBox().lower()[0], patch.getBox().lower()[1], patch.getBox().lower()[2]};
//    index_type hi[3] = {patch.getBox().upper()[0], patch.getBox().upper()[1], patch.getBox().upper()[2]};
//
//    //    std::shared_ptr<simpla::engine::MeshBlock> m = std::make_shared<simpla::engine::MeshBlock>(3, lo, hi, dx,
//    //    xlo);
//    //    m->id(static_cast<id_type>(patch.getBox().getGlobalId().getOwnerRank() * 10000 +
//    //                               patch.getBox().getGlobalId().getLocalId().getValue()));
//    //    this->SetMeshBlock(m);
//};
// SAMRAIPatchProxy::~SAMRAIPatchProxy() {}
//
// namespace detail {
//
// template <typename TV>
// std::shared_ptr<data::DataBlock> create_data_block_t0(data::DataTable const &desc,
//                                                      boost::shared_ptr<SAMRAI::hier::PatchData> pd) {
//    auto p_data = boost::dynamic_pointer_cast<SAMRAI::pdat::NodeData<TV>>(pd);
//    int ndims = p_data->getDim().getValue();
//    int depth = p_data->getDepth();
//    auto outer_lower = p_data->getGhostBox().lower();
//    auto outer_upper = p_data->getGhostBox().upper();
//    auto inner_lower = p_data->getBox().lower();
//    auto inner_upper = p_data->getBox().upper();
//    size_type dims[4] = {static_cast<size_type>(outer_upper[0] - outer_lower[0]),
//                         static_cast<size_type>(outer_upper[1] - outer_lower[1]),
//                         static_cast<size_type>(outer_upper[2] - outer_lower[2]),
//                         static_cast<size_type>(desc.GetValue<int>("DOF"))};
//    index_type lo[4] = {inner_lower[0] - outer_lower[0], inner_lower[1] - outer_lower[1],
//                        inner_lower[2] - outer_lower[2], 0};
//    index_type hi[4] = {inner_upper[0] - outer_lower[0], inner_upper[1] - outer_lower[1],
//                        inner_upper[2] - outer_lower[2], static_cast<index_type>(desc.GetValue<int>("DOF"))};
//    //    auto res = std::make_shared<data::DataEntityWrapper<simpla::Array<TV, 4>>>();
//    // p_data->getPointer(), dims, lo, hi
//    //    res->SetUpDataOnPatch();
//    //    return std::dynamic_pointer_cast<data::DataBlock>(res);
//    return nullptr;
//};
//
// std::shared_ptr<data::DataBlock> create_data_block(data::DataTable const &desc,
//                                                   boost::shared_ptr<SAMRAI::hier::PatchData> pd) {
//    std::shared_ptr<data::DataBlock> res(nullptr);
//    if (desc.value_type_info() == (typeid(float))) {
//        res = create_data_block_t0<float>(desc, pd);
//    } else if (desc.value_type_info() == (typeid(double))) {
//        res = create_data_block_t0<double>(desc, pd);
//    } else if (desc.value_type_info() == (typeid(int))) {
//        res = create_data_block_t0<int>(desc, pd);
//    }
//    //    else if (item->value_type_info() == typeid(long)) { attr_choice_form<long>(item,
//    //    std::forward<Args>(args)...); }
//    else {
//        RUNTIME_ERROR << "Unsupported m_value_ value_type_info" << std::endl;
//    }
//    ASSERT(res != nullptr);
//    return res;
//}
//}  // namespace detail
// std::shared_ptr<data::DataBlock> SAMRAIPatchProxy::data(id_type const &id, std::shared_ptr<data::DataBlock> const &p)
// {
//    UNIMPLEMENTED;
//    return nullptr;
//}
// std::shared_ptr<data::DataBlock> SAMRAIPatchProxy::data(id_type const &id) const {
//    return simpla::detail::create_data_block(*m_simpla_attrs_.at(id),
//                                             m_samrai_patch_.getPatchData(m_samrai_variables_.at(id), m_samrai_ctx_));
//}

template <typename TV, int NDIMS, int IFORM, int DEPTH>
class SAMRAIDataBlock : public data::DataMultiArray<TV, (DEPTH <= 1 ? NDIMS : NDIMS + 1),
                                                    ((IFORM == VERTEX || IFORM == VOLUME) ? 1 : 3)> {
    typedef simpla::Array<TV, (DEPTH <= 1 ? NDIMS : NDIMS + 1)> array_type;

    typedef data::DataMultiArray<TV, (DEPTH <= 1 ? NDIMS : NDIMS + 1), ((IFORM == VERTEX || IFORM == VOLUME) ? 1 : 3)>
        data_block_type;

    typedef SAMRAIDataBlock<TV, NDIMS, IFORM, DEPTH> samrai_data_block;

    SP_OBJECT_HEAD(samrai_data_block, data_block_type);

    typedef std::conditional_t<
        IFORM == VERTEX, SAMRAI::pdat::NodeData<TV>,
        std::conditional_t<IFORM == EDGE, SAMRAI::pdat::EdgeData<TV>,
                           std::conditional_t<IFORM == FACE, SAMRAI::pdat::FaceData<TV>, SAMRAI::pdat::CellData<TV>>>>
        samrai_data_type;

    boost::shared_ptr<samrai_data_type> m_data_;

    SAMRAIDataBlock(boost::shared_ptr<SAMRAI::hier::PatchData> const d)
        : m_data_(boost::dynamic_pointer_cast<samrai_data_type>(d)) {
        for (int i = 0; i < DEPTH; ++i) {
            index_type in_lower[4] = {d->getBox().lower()[0], d->getBox().lower()[1], d->getBox().lower()[2], DEPTH};
            index_type in_upper[4] = {d->getBox().upper()[0], d->getBox().upper()[1], d->getBox().upper()[2], DEPTH};

            index_type out_lower[4] = {d->getGhostBox().lower()[0], d->getGhostBox().lower()[1],
                                       d->getGhostBox().lower()[2], DEPTH};
            index_type out_upper[4] = {d->getGhostBox().upper()[0], d->getGhostBox().upper()[1],
                                       d->getGhostBox().upper()[2], DEPTH};

            array_type(in_lower, in_upper, out_lower, out_upper,
                       std::shared_ptr<value_type>(m_data_->getPointer(i), tags::do_nothing()))
                .swap(this->operator[](i));
        }
    }
    ~SAMRAIDataBlock() {}

    // data::DataBlock
    virtual bool empty() const { return m_data_ != nullptr; }
    virtual std::type_info const &value_type_info() const { return typeid(TV); };
    virtual int GetNDIMS() const { return m_data_->getDim().getValue(); }
    virtual size_type GetDepth() const { return m_data_->getDepth(); }

    virtual index_type const *GetInnerLowerIndex(int depth = 0) const { return nullptr; }
    virtual index_type const *GetInnerUpperIndex(int depth = 0) const { return nullptr; }
    virtual index_type const *GetOuterLowerIndex(int depth = 0) const { return nullptr; }
    virtual index_type const *GetOuterUpperIndex(int depth = 0) const { return nullptr; }

    virtual TV const *GetPointer(int depth = 0) const { return m_data_->getPointer(depth); }
    virtual TV *GetPointer(int depth = 0) { return m_data_->getPointer(depth); }

    virtual void Clear() { m_data_->fillAll(0); }
    virtual void Copy(data::DataBlock const &other) {
        m_data_->copy(dynamic_cast<samrai_data_block const &>(other).m_data_);
    };
};

class SAMRAIHyperbolicPatchStrategyAdapter : public SAMRAI::algs::HyperbolicPatchStrategy {
   public:
    SAMRAIHyperbolicPatchStrategyAdapter(std::shared_ptr<engine::Context> const &ctx,
                                         boost::shared_ptr<SAMRAI::geom::CartesianGridGeometry> const &);

    /**
     * The destructor for SAMRAIWorkerHyperbolic does nothing.
     */
    ~SAMRAIHyperbolicPatchStrategyAdapter();

    ///
    ///  The following routines:
    ///
    ///      registerModelVariables(),
    ///      initializeDataOnPatch(),
    ///      computeStableDtOnPatch(),
    ///      computeFluxesOnPatch(),
    ///      conservativeDifferenceOnPatch(),
    ///      tagGradientDetectorCells(),
    ///      tagRichardsonExtrapolationCells()
    ///
    ///  are concrete implementations of functions declared in the
    ///  algs::HyperbolicPatchStrategy abstract base class.
    ///

    void registerModelVariables(SAMRAI::algs::HyperbolicLevelIntegrator *integrator) final;

    void setupLoadBalancer(SAMRAI::algs::HyperbolicLevelIntegrator *integrator,
                           SAMRAI::mesh::GriddingAlgorithm *gridding_algorithm) final;

    void initializeDataOnPatch(SAMRAI::hier::Patch &patch, const double data_time, const bool initial_time) final;
    double computeStableDtOnPatch(SAMRAI::hier::Patch &patch, const bool initial_time, const double dt_time) final;
    void computeFluxesOnPatch(SAMRAI::hier::Patch &patch, const double time, const double dt) final;

    /**
     * Update linear advection solution variables by performing a conservative
     * difference with the fluxes calculated in computeFluxesOnPatch().
     */
    void conservativeDifferenceOnPatch(SAMRAI::hier::Patch &patch, const double time, const double dt,
                                       bool at_syncronization);

    /**
     * Tag cells for refinement using gradient detector.
     */
    void tagGradientDetectorCells(SAMRAI::hier::Patch &patch, const double regrid_time, const bool initial_error,
                                  const int tag_indexindx, const bool uses_richardson_extrapolation_too);

    /**
     * Tag cells for refinement using Richardson extrapolation.
     */
    void tagRichardsonExtrapolationCells(SAMRAI::hier::Patch &patch, const int error_level_number,
                                         const boost::shared_ptr<SAMRAI::hier::VariableContext> &coarsened_fine,
                                         const boost::shared_ptr<SAMRAI::hier::VariableContext> &advanced_coarse,
                                         const double regrid_time, const double deltat, const int error_coarsen_ratio,
                                         const bool initial_error, const int tag_index,
                                         const bool uses_gradient_detector_too);

    //@{
    //! @name Required implementations of HyperbolicPatchStrategy pure virtuals.

    ///
    ///  The following routines:
    ///
    ///      setPhysicalBoundaryConditions()
    ///      getRefineOpStencilWidth(),
    ///      preprocessRefine()
    ///      postprocessRefine()
    ///
    ///  are concrete implementations of functions declared in the
    ///  RefinePatchStrategy abstract base class.  Some are trivial
    ///  because this class doesn't do any pre/postprocessRefine.
    ///

    /**
     * Set the data in ghost cells corresponding to physical boundary
     * conditions.  Specific boundary conditions are determined by
     * information specified in input file and numerical routines.
     */
    void setPhysicalBoundaryConditions(SAMRAI::hier::Patch &patch, const double fill_time,
                                       const SAMRAI::hier::IntVector &ghost_width_to_fill);

    SAMRAI::hier::IntVector getRefineOpStencilWidth(const SAMRAI::tbox::Dimension &dim) const {
        return SAMRAI::hier::IntVector::getZero(dim);
    }

    void preprocessRefine(SAMRAI::hier::Patch &fine, const SAMRAI::hier::Patch &coarse,
                          const SAMRAI::hier::Box &fine_box, const SAMRAI::hier::IntVector &ratio) {
        NULL_USE(fine);
        NULL_USE(coarse);
        NULL_USE(fine_box);
        NULL_USE(ratio);
    }

    void postprocessRefine(SAMRAI::hier::Patch &fine, const SAMRAI::hier::Patch &coarse,
                           const SAMRAI::hier::Box &fine_box, const SAMRAI::hier::IntVector &ratio) {
        NULL_USE(fine);
        NULL_USE(coarse);
        NULL_USE(fine_box);
        NULL_USE(ratio);
    }

    ///
    ///  The following routines:
    ///
    ///      getCoarsenOpStencilWidth(),
    ///      preprocessCoarsen()
    ///      postprocessCoarsen()
    ///
    ///  are concrete implementations of functions declared in the
    ///  CoarsenPatchStrategy abstract base class.  They are trivial
    ///  because this class doesn't do any pre/postprocessCoarsen.
    ///

    SAMRAI::hier::IntVector getCoarsenOpStencilWidth(const SAMRAI::tbox::Dimension &dim) const {
        return SAMRAI::hier::IntVector::getZero(dim);
    }

    void preprocessCoarsen(SAMRAI::hier::Patch &coarse, const SAMRAI::hier::Patch &fine,
                           const SAMRAI::hier::Box &coarse_box, const SAMRAI::hier::IntVector &ratio) {
        NULL_USE(coarse);
        NULL_USE(fine);
        NULL_USE(coarse_box);
        NULL_USE(ratio);
    }

    void postprocessCoarsen(SAMRAI::hier::Patch &coarse, const SAMRAI::hier::Patch &fine,
                            const SAMRAI::hier::Box &coarse_box, const SAMRAI::hier::IntVector &ratio) {
        NULL_USE(coarse);
        NULL_USE(fine);
        NULL_USE(coarse_box);
        NULL_USE(ratio);
    }

    //@}

    /**
     * Register a VisIt data writer so this class will write
     * plot files that may be postprocessed with the VisIt
     * visualization tool.
     */
    void registerVisItDataWriter(boost::shared_ptr<SAMRAI::appu::VisItDataWriter> viz_writer);

    /**Print all data members for SAMRAIWorkerHyperbolic class.     */
    void printClassData(std::ostream &os) const;

    //    void Dispatch(SAMRAI::hier::Patch &patch);

   private:
    std::shared_ptr<engine::Context> m_ctx_;
    /*
     * The object GetName is used for error/warning reporting and also as a
     * string label for restart database entries.
     */
    std::string m_name_;
    SAMRAI::tbox::Dimension d_dim;

    /*
     * We cache pointers to the grid geometry object to set up initial
     * GetDataBlock, SetValue physical boundary conditions, and register plot
     * variables.
     */
    boost::shared_ptr<SAMRAI::geom::CartesianGridGeometry> d_grid_geometry = nullptr;
    boost::shared_ptr<SAMRAI::appu::VisItDataWriter> d_visit_writer = nullptr;

    /*
     * Data items used for nonuniform Load balance, if used.
     */
    boost::shared_ptr<SAMRAI::pdat::CellVariable<double>> d_workload_variable;
    int d_workload_data_id;
    bool d_use_nonuniform_workload;
    std::map<id_type, boost::shared_ptr<SAMRAI::hier::Variable>> m_samrai_variables_;
    SAMRAI::hier::IntVector d_nghosts;
    SAMRAI::hier::IntVector d_fluxghosts;

    void Push(SAMRAI::hier::Patch &patch, engine::Patch *p);
    void Pop(SAMRAI::hier::Patch &patch, engine::Patch *p);
};

SAMRAIHyperbolicPatchStrategyAdapter::SAMRAIHyperbolicPatchStrategyAdapter(
    std::shared_ptr<engine::Context> const &ctx,
    boost::shared_ptr<SAMRAI::geom::CartesianGridGeometry> const &grid_geom)
    : SAMRAI::algs::HyperbolicPatchStrategy(),
      d_dim(3),
      d_grid_geometry(grid_geom),
      d_use_nonuniform_workload(false),
      d_nghosts(d_dim, 4),
      d_fluxghosts(d_dim, 1),
      m_ctx_(ctx) {
    TBOX_ASSERT(grid_geom);
}

/**************************************************************************
 *
 * Empty destructor for SAMRAIWorkerAdapter class.
 *
 *************************************************************************
 */

SAMRAIHyperbolicPatchStrategyAdapter::~SAMRAIHyperbolicPatchStrategyAdapter() {}

namespace detail {
template <typename T>
boost::shared_ptr<SAMRAI::hier::Variable> create_samrai_variable_t(int ndims, engine::Attribute const &attr) {
    static int var_depth[4] = {1, 3, 3, 1};
    SAMRAI::tbox::Dimension d_dim(ndims);
    return (attr.GetIFORM() > VOLUME)
               ? nullptr
               : boost::dynamic_pointer_cast<SAMRAI::hier::Variable>(boost::make_shared<SAMRAI::pdat::NodeVariable<T>>(
                     d_dim, attr.GetName(), var_depth[attr.GetIFORM()] * attr.GetDOF()));
}

boost::shared_ptr<SAMRAI::hier::Variable> create_samrai_variable(unsigned int ndims, engine::Attribute const &attr) {
    if (attr.value_type_info() == (typeid(float))) {
        return create_samrai_variable_t<float>(ndims, attr);
    } else if (attr.value_type_info() == (typeid(double))) {
        return create_samrai_variable_t<double>(ndims, attr);
    } else if (attr.value_type_info() == (typeid(int))) {
        return create_samrai_variable_t<int>(ndims, attr);
    }
    //    else if (item->value_type_info() == typeid(long)) { attr_choice_form<long>(item,
    //    std::forward<Args>(args)...); }
    else {
        RUNTIME_ERROR << " attr [" << attr.GetName() << "] is not supported!" << std::endl;
    }
    return nullptr;
}

std::shared_ptr<data::DataBlock> convert(boost::shared_ptr<SAMRAI::hier::PatchData> d) {}
}  // namespace detail{

/**
 * Register conserved variables  and  register plot data with VisIt.
 */

void SAMRAIHyperbolicPatchStrategyAdapter::registerModelVariables(SAMRAI::algs::HyperbolicLevelIntegrator *integrator) {
    ASSERT(integrator != nullptr);
    SAMRAI::hier::VariableDatabase *vardb = SAMRAI::hier::VariableDatabase::getDatabase();

    //    if (!d_visit_writer) {
    //        RUNTIME_ERROR << m_name_
    //                      << ": registerModelVariables() VisIt GetDataBlock writer was not registered."
    //                         "Consequently, no plot  DataBlock will be written."
    //                      << std::endl;
    //    }
    //    SAMRAI::tbox::Dimension d_dim{4};
    SAMRAI::hier::IntVector d_nghosts{d_dim, 4};
    SAMRAI::hier::IntVector d_fluxghosts{d_dim, 1};
    //**************************************************************

    engine::AttributeGroup attr_grp;
    m_ctx_->Register(&attr_grp);
    for (engine::Attribute *v : attr_grp.GetAll()) {
        if (v->GetName() == "" || v->GetName()[0] == '_') continue;
        boost::shared_ptr<SAMRAI::hier::Variable> var = simpla::detail::create_samrai_variable(3, *v);
        m_samrai_variables_[v->GetGUID()] = var;

        /*** FIXME:
        *  1. SAMRAI Visit Writer only support NODE and CELL variable (double,float ,int)
        *  2. SAMRAI   SAMRAI::algs::HyperbolicLevelIntegrator->registerVariable only support double
        **/

        if (v->db()->Check("COORDINATES", true)) {
            VERBOSE << v->GetName() << " is registered as coordinate" << std::endl;
            integrator->registerVariable(var, d_nghosts, SAMRAI::algs::HyperbolicLevelIntegrator::INPUT,
                                         d_grid_geometry, "", "LINEAR_REFINE");

        } else if (v->db()->Check("FLUX", true)) {
            integrator->registerVariable(var, d_fluxghosts, SAMRAI::algs::HyperbolicLevelIntegrator::FLUX,
                                         d_grid_geometry, "CONSERVATIVE_COARSEN", "NO_REFINE");

        } else if (v->db()->Check("INPUT", true)) {
            integrator->registerVariable(var, d_nghosts, SAMRAI::algs::HyperbolicLevelIntegrator::INPUT,
                                         d_grid_geometry, "", "NO_REFINE");
        } else {
            switch (v->GetIFORM()) {
                case EDGE:
                case FACE:
                // integrator->registerVariable(var, d_nghosts,
                // SAMRAI::algs::HyperbolicLevelIntegrator::TIME_DEP,
                //                              d_grid_geometry, "CONSERVATIVE_COARSEN",
                //                              "CONSERVATIVE_LINEAR_REFINE");
                // break;
                case VERTEX:
                case VOLUME:
                default:
                    integrator->registerVariable(var, d_nghosts, SAMRAI::algs::HyperbolicLevelIntegrator::TIME_DEP,
                                                 d_grid_geometry, "", "LINEAR_REFINE");
            }

            //            VERBOSE << (v->db()->GetValue<std::string>("name","unnamed")()) << " --  " <<
            //            visit_variable_type << std::endl;
        }

        std::string visit_variable_type = "";
        if ((v->GetIFORM() == VERTEX || v->GetIFORM() == VOLUME) && (v->GetDOF() == 1)) {
            visit_variable_type = "SCALAR";
        } else if (((v->GetIFORM() == EDGE || v->GetIFORM() == FACE) && (v->GetDOF() == 1)) ||
                   ((v->GetIFORM() == VERTEX || v->GetIFORM() == VOLUME) && (v->GetDOF() == 3))) {
            visit_variable_type = "VECTOR";
        } else if (((v->GetIFORM() == VERTEX || v->GetIFORM() == VOLUME) && v->GetDOF() == 9) ||
                   ((v->GetIFORM() == EDGE || v->GetIFORM() == FACE) && v->GetDOF() == 3)) {
            visit_variable_type = "TENSOR";
        } else {
            WARNING << "Can not register attribute [" << v->GetName() << "] to VisIt writer !" << std::endl;
        }
        if (visit_variable_type != "" && v->db()->Check("COORDINATES")) {
            d_visit_writer->registerNodeCoordinates(
                vardb->mapVariableAndContextToIndex(var, integrator->getPlotContext()));
        } else /*if (v->db()->Check("CHECK")) */ {
            d_visit_writer->registerPlotQuantity(
                v->GetName(), visit_variable_type,
                vardb->mapVariableAndContextToIndex(var, integrator->getPlotContext()));
        }
    }
    //    integrator->printClassData(std::cout);
    //    vardb->printClassData(std::cout);
}

/**
 * Set up parameters for nonuniform load balancing, if used.
 */

void SAMRAIHyperbolicPatchStrategyAdapter::setupLoadBalancer(SAMRAI::algs::HyperbolicLevelIntegrator *integrator,
                                                             SAMRAI::mesh::GriddingAlgorithm *gridding_algorithm) {
    const SAMRAI::hier::IntVector &zero_vec = SAMRAI::hier::IntVector::getZero(d_dim);
    SAMRAI::hier::VariableDatabase *vardb = SAMRAI::hier::VariableDatabase::getDatabase();
    if (d_use_nonuniform_workload && gridding_algorithm) {
        auto load_balancer =
            boost::dynamic_pointer_cast<SAMRAI::mesh::CascadePartitioner>(gridding_algorithm->getLoadBalanceStrategy());
        if (load_balancer) {
            d_workload_variable.reset(new SAMRAI::pdat::CellVariable<double>(d_dim, "workload_variable", 1));
            d_workload_data_id =
                vardb->registerVariableAndContext(d_workload_variable, vardb->getContext("WORKLOAD"), zero_vec);
            load_balancer->setWorkloadPatchDataIndex(d_workload_data_id);
        } else {
            WARNING << m_name_ << ": "
                    << "  Unknown Load balancer used in gridding algorithm."
                    << "  Ignoring request for nonuniform Load balancing." << std::endl;
            d_use_nonuniform_workload = false;
        }
    } else {
        d_use_nonuniform_workload = false;
    }
}

void SAMRAIHyperbolicPatchStrategyAdapter::Push(SAMRAI::hier::Patch &patch, engine::Patch *p) {
    auto mblk = std::make_shared<engine::MeshBlock>(
        index_box_type{{patch.getBox().lower()[0], patch.getBox().lower()[1], patch.getBox().lower()[2]},
                       {patch.getBox().upper()[0], patch.getBox().upper()[1], patch.getBox().upper()[2]}},
        patch.getPatchLevelNumber());

    p->SetBlock(mblk);

    for (auto const &item : m_samrai_variables_) {
        auto pdata = patch.getPatchData(item.second, getDataContext());
        //        p->Push(item.first, d);
    }
}
void SAMRAIHyperbolicPatchStrategyAdapter::Pop(SAMRAI::hier::Patch &patch, engine::Patch *p) {
    //    for (auto const &item : p->GetData()) { patch.setPatchData(item.first, item.second); }
}

/**
 * Set initial data for solution variables on patch interior.
 * This routine is called whenever a new patch is introduced to the
 * AMR patch hierarchy.  Note that the routine does nothing unless
 * we are at the initial time.  In all other cases, conservative
 * interpolation from coarser levels and copies from patches at the
 * same mesh resolution are sufficient to set data.
 *
 */

void SAMRAIHyperbolicPatchStrategyAdapter::initializeDataOnPatch(SAMRAI::hier::Patch &patch, const double data_time,
                                                                 const bool initial_time) {
    if (initial_time) {
        engine::Patch p;
        Push(patch, &p);
        m_ctx_->SetUpDataOnPatch(&p, data_time);
        Pop(patch, &p);
    }

    if (d_use_nonuniform_workload) {
        if (!patch.checkAllocated(d_workload_data_id)) { patch.allocatePatchData(d_workload_data_id); }
        boost::shared_ptr<SAMRAI::pdat::CellData<double>> workload_data(
            boost::dynamic_pointer_cast<SAMRAI::pdat::CellData<double>, SAMRAI::hier::PatchData>(
                patch.getPatchData(d_workload_data_id)));
        TBOX_ASSERT(workload_data);

        const SAMRAI::hier::Box &box = patch.getBox();
        const SAMRAI::hier::BoxId &box_id = box.getBoxId();
        const SAMRAI::hier::LocalId &local_id = box_id.getLocalId();
        double id_val = local_id.getValue() % 2 ? static_cast<double>(local_id.getValue() % 10) : 0.0;
        workload_data->fillAll(1.0 + id_val);
    }
}

/**************************************************************************
 *
 * Compute stable time increment for patch.  Return this m_value_.
 *
 *************************************************************************
 */

double SAMRAIHyperbolicPatchStrategyAdapter::computeStableDtOnPatch(SAMRAI::hier::Patch &patch, const bool initial_time,
                                                                    const double dt_time) {
    auto pgeom = boost::dynamic_pointer_cast<SAMRAI::geom::CartesianPatchGeometry>(patch.getPatchGeometry());
    return pgeom->getDx()[0] / 2.0;
}

/**************************************************************************
 *
 * Compute time integral of numerical fluxes for finite difference
 * at each cell face on patch.  When d_dim == tbox::Dimension(3)), there are two options
 * for the transverse flux correction.  Otherwise, there is only one.
 *
 *************************************************************************
 */

void SAMRAIHyperbolicPatchStrategyAdapter::computeFluxesOnPatch(SAMRAI::hier::Patch &patch, const double time,
                                                                const double dt) {}

/**************************************************************************
 *
 * Update solution variables by performing a conservative
 * difference with the fluxes calculated in computeFluxesOnPatch().
 *
 **************************************************************************/

void SAMRAIHyperbolicPatchStrategyAdapter::conservativeDifferenceOnPatch(SAMRAI::hier::Patch &patch,
                                                                         const double time_now, const double time_dt,
                                                                         bool at_syncronization) {
    engine::Patch p;
    Push(patch, &p);
    m_ctx_->UpdateDataOnPatch(&p, time_now, time_dt);
    Pop(patch, &p);
}

/**************************************************************************
 *
 * Tag cells for refinement using Richardson extrapolation.  Criteria
 * defined in input.
 *
 *************************************************************************
 */

void SAMRAIHyperbolicPatchStrategyAdapter::tagRichardsonExtrapolationCells(
    SAMRAI::hier::Patch &patch, const int error_level_number,
    const boost::shared_ptr<SAMRAI::hier::VariableContext> &coarsened_fine,
    const boost::shared_ptr<SAMRAI::hier::VariableContext> &advanced_coarse, const double regrid_time,
    const double deltat, const int error_coarsen_ratio, const bool initial_error, const int tag_index,
    const bool uses_gradient_detector_too) {}

/**************************************************************************
 *
 * Tag cells for refinement using gradient detector.  Tagging criteria
 * defined in input.
 *
 *************************************************************************
 */

void SAMRAIHyperbolicPatchStrategyAdapter::tagGradientDetectorCells(SAMRAI::hier::Patch &patch,
                                                                    const double regrid_time, const bool initial_error,
                                                                    const int tag_indx,
                                                                    const bool uses_richardson_extrapolation_too) {}

void SAMRAIHyperbolicPatchStrategyAdapter::setPhysicalBoundaryConditions(
    SAMRAI::hier::Patch &patch, const double fill_time, const SAMRAI::hier::IntVector &ghost_width_to_fill) {
    engine::Patch p;
    Push(patch, &p);
    m_ctx_->UpdateDataOnPatch(&p, fill_time);
    Pop(patch, &p);
}

/**************************************************************************
 *
 * Register VisIt SetDataBlock writer to write GetDataBlock to plot files that may
 * be postprocessed by the VisIt tool.
 *
 *************************************************************************
 */

void SAMRAIHyperbolicPatchStrategyAdapter::registerVisItDataWriter(
    boost::shared_ptr<SAMRAI::appu::VisItDataWriter> viz_writer) {
    TBOX_ASSERT(viz_writer);
    d_visit_writer = viz_writer;
}

/**************************************************************************
 * Write SAMRAIWorkerAdapter object state to specified stream.
 **************************************************************************/

void SAMRAIHyperbolicPatchStrategyAdapter::printClassData(std::ostream &os) const {
    os << "\nSAMRAIWorkerAdapter::printClassData..." << std::endl;
    os << "m_name_ = " << m_name_ << std::endl;
    os << "d_grid_geometry = " << d_grid_geometry.get() << std::endl;
    os << std::endl;
}

/**
 * class SAMRAITimeIntegrator
 */
struct SAMRAITimeIntegrator : public engine::TimeIntegrator {
    SP_OBJECT_HEAD(SAMRAITimeIntegrator, engine::TimeIntegrator);

   public:
    SAMRAITimeIntegrator();
    ~SAMRAITimeIntegrator();

    virtual std::shared_ptr<data::DataTable> Serialize() const;
    virtual void Deserialize(std::shared_ptr<data::DataTable>);

    virtual void Initialize();
    virtual void Finalize();
    virtual void SetUp();
    virtual void TearDown();

    virtual void Synchronize();
    virtual Real Advance(Real time_dt = 0.0);
    virtual bool Done() const;

    virtual void CheckPoint() const;
    virtual void Dump() const;

   private:
    boost::shared_ptr<SAMRAIHyperbolicPatchStrategyAdapter> hyperbolic_patch_strategy;
    boost::shared_ptr<SAMRAI::geom::CartesianGridGeometry> grid_geometry;
    boost::shared_ptr<SAMRAI::hier::PatchHierarchy> patch_hierarchy;
    boost::shared_ptr<SAMRAI::algs::HyperbolicLevelIntegrator> hyp_level_integrator;

    //    boost::shared_ptr<SAMRAI::engine::StandardTagAndInitialize> error_detector;
    //    boost::shared_ptr<SAMRAI::engine::BergerRigoutsos> box_generator;
    //    boost::shared_ptr<SAMRAI::engine::CascadePartitioner> load_balancer;
    //    boost::shared_ptr<SAMRAI::engine::GriddingAlgorithm> gridding_algorithm;
    boost::shared_ptr<SAMRAI::algs::TimeRefinementIntegrator> m_time_refinement_integrator_;
    // VisItDataWriter is only present if HDF is available
    boost::shared_ptr<SAMRAI::appu::VisItDataWriter> visit_data_writer;

    bool write_restart = false;
    int restart_interval = 0;

    std::string restart_write_dirname;

    bool viz_dump_data = false;
    int viz_dump_interval = 1;

    unsigned int ndims = 3;
};

bool SAMRAITimeIntegrator_IS_REGISTERED =
    engine::Schedule::RegisterCreator<SAMRAITimeIntegrator>("SAMRAI", "SAMRAI Time Integrator");

SAMRAITimeIntegrator::SAMRAITimeIntegrator() : engine::TimeIntegrator(){};
SAMRAITimeIntegrator::~SAMRAITimeIntegrator() {
    SAMRAI::tbox::SAMRAIManager::shutdown();
    SAMRAI::tbox::SAMRAIManager::finalize();
}
void SAMRAITimeIntegrator::Initialize() {
    engine::TimeIntegrator::Initialize();
    /** Setup SAMRAI::tbox::MPI.      */
    SAMRAI::tbox::SAMRAI_MPI::init(GLOBAL_COMM.comm());
    SAMRAI::tbox::SAMRAIManager::initialize();
    /** Setup SAMRAI, enable logging, and process command line.     */
    SAMRAI::tbox::SAMRAIManager::startup();

    //    data::DataTable(std::make_shared<DataBackendSAMRAI>()).swap(*db());
    //    const SAMRAI::tbox::SAMRAI_MPI & mpi(SAMRAI::tbox::SAMRAI_MPI::getSAMRAIWorld());
}
void SAMRAITimeIntegrator::Synchronize() { engine::TimeIntegrator::Synchronize(); }
std::shared_ptr<data::DataTable> SAMRAITimeIntegrator::Serialize() const { return engine::TimeIntegrator::Serialize(); }
void SAMRAITimeIntegrator::Deserialize(std::shared_ptr<data::DataTable> cfg) {
    engine::TimeIntegrator::Deserialize(cfg);
}
void SAMRAITimeIntegrator::TearDown() { engine::TimeIntegrator::TearDown(); }
void SAMRAITimeIntegrator::SetUp() {
    engine::TimeIntegrator::SetUp();
    /** test.3d.input */
    /**
    // Refer to geom::CartesianGridGeometry and its base classes for input
    CartesianGeometry{
       domain_boxes	= [ (0,0,0) , (14,9,9) ]
       x_lo = 0.e0 , 0.e0 , 0.e0    // lower end of computational domain.
       x_up = 30.e0 , 20.e0 , 20.e0 // upper end of computational domain.
    }

    // Refer to hier::PatchHierarchy for input
    PatchHierarchy {
       max_levels = 3        // Maximum number of levels in hierarchy.
    // Note: For the following regridding information, data is required for each
    //       potential in the patch hierarchy; i.e., levels 0 thru max_levels-1.
    //       If more data values than needed are given, only the number required
    //       will be read in.  If fewer values are given, an error will result.
    //
    // Specify coarsening ratios for each level 1 through max_levels-1
       ratio_to_coarser {             // vector ratio to next coarser level
          level_1 = 2 , 2 , 2
          level_2 = 2 , 2 , 2
          level_3 = 2 , 2 , 2
       }
       largest_patch_size {
          level_0 = 40 , 40 , 40  // largest patch allowed in hierarchy
          // all finer levels will use same values as level_0...
       }
       smallest_patch_size {
          level_0 = 9 , 9 , 9
          // all finer levels will use same values as level_0...
       }
    }
    // Refer to mesh::GriddingAlgorithm for input
    GriddingAlgorithm{}

    // Refer to mesh::BergerRigoutsos for input
    BergerRigoutsos {
       sort_output_nodes = TRUE // Makes results repeatable.
       efficiency_tolerance   = 0.85e0    // min % of tag cells in new patch level
       combine_efficiency     = 0.95e0    // chop box if sum of volumes of smaller
                                          // boxes < efficiency * vol of large box
    }
    // Refer to mesh::StandardTagAndInitialize for input
    StandardTagAndInitialize {tagging_method = "GRADIENT_DETECTOR"}
    // Refer to algs::HyperbolicLevelIntegrator for input
    HyperbolicLevelIntegrator{
       cfl                       = 0.9e0    // max cfl factor used in problem
       cfl_init                  = 0.9e0    // initial cfl factor
       lag_dt_computation        = TRUE
       use_ghosts_to_compute_dt  = TRUE
    }

    // Refer to algs::TimeRefinementIntegrator for input
    TimeRefinementIntegrator{
       start_time           = 0.e0     // initial simulation time
       end_time             = 100.e0   // final simulation time
       grow_dt              = 1.1e0    // growth factor for timesteps
       max_integrator_steps = 10       // max number of simulation timesteps
    }

    // Refer to mesh::TreeLoadBalancer for input
    LoadBalancer {
       // using default TreeLoadBalancer configuration
    }
     */
    auto ctx = GetContext();
    auto const &atlas = ctx->GetAtlas();
    auto bound_box = ctx->GetModel().GetBoundBox();
    ndims = static_cast<unsigned int>(ctx->GetModel().GetNDims());
    bool use_refined_timestepping = true;  // m_samrai_db_->GetValue<bool>("use_refined_timestepping", true);

    SAMRAI::tbox::Dimension dim(static_cast<unsigned short>(ndims));

    //    samrai_db = simpla::detail::convert_database(db(), name());
    /**
     * Create major algorithm and data objects which comprise application.
     * Each object will be initialized either from input data or restart
     * files, or a combination of both.  Refer to each class constructor
     * for details.  For more information on the composition of objects
     * for this application, see comments at top of file.
     */

    auto CartesianGridGeometry = boost::make_shared<SAMRAI::tbox::MemoryDatabase>("CartesianGeometry");

    nTuple<int, 3> i_low, i_up;
    std::tie(i_low, i_up) = atlas.GetIndexBox();
    SAMRAI::tbox::DatabaseBox box{SAMRAI::tbox::Dimension(3), &i_low[0], &i_up[0]};
    CartesianGridGeometry->putDatabaseBox("domain_boxes_0", box);
    nTuple<int, 3> periodic_dimension;
    periodic_dimension = atlas.GetPeriodicDimension();
    nTuple<double, 3> x_low, x_up;
    std::tie(x_low, x_up) = bound_box;

    CartesianGridGeometry->putIntegerArray("periodic_dimension", &periodic_dimension[0], ndims);
    CartesianGridGeometry->putDoubleArray("x_lo", &x_low[0], ndims);
    CartesianGridGeometry->putDoubleArray("x_up", &x_up[0], ndims);

    grid_geometry.reset(new SAMRAI::geom::CartesianGridGeometry(dim, "CartesianGeometry", CartesianGridGeometry));

    //---------------------------------

    auto PatchHierarchy = boost::make_shared<SAMRAI::tbox::MemoryDatabase>("PatchHierarchy");

    // Maximum number of levels in hierarchy.
    PatchHierarchy->putInteger("max_levels", static_cast<int>(atlas.GetMaxLevel()));

    auto ratio_to_coarser = PatchHierarchy->putDatabase("ratio_to_coarser");

    for (int i = 0, n = static_cast<int>(atlas.GetMaxLevel()); i < n; ++i) {
        nTuple<int, 3> level;
        level = atlas.GetRefineRatio(i);
        ratio_to_coarser->putIntegerArray("level_" + std::to_string(i), &level[0], ndims);
    }

    auto largest_patch_size = PatchHierarchy->putDatabase("largest_patch_size");
    auto smallest_patch_size = PatchHierarchy->putDatabase("smallest_patch_size");

    nTuple<int, 3> level_largest, level_smallest;
    level_smallest = atlas.GetSmallestDimensions();
    level_largest = atlas.GetLargestDimensions();

    smallest_patch_size->putIntegerArray("level_0", &level_smallest[0], ndims);
    largest_patch_size->putIntegerArray("level_0", &level_largest[0], ndims);

    patch_hierarchy.reset(new SAMRAI::hier::PatchHierarchy("PatchHierarchy", grid_geometry, PatchHierarchy));

    auto HyperbolicLevelIntegrator = boost::make_shared<SAMRAI::tbox::MemoryDatabase>("HyperbolicLevelIntegrator");

    // Refer to algs::HyperbolicLevelIntegrator for input
    // max cfl factor used in problem
    HyperbolicLevelIntegrator->putDouble("cfl", engine::TimeIntegrator::GetCFL());
    HyperbolicLevelIntegrator->putDouble("cfl_init", 0.9);
    HyperbolicLevelIntegrator->putBool("lag_dt_computation", true);
    HyperbolicLevelIntegrator->putBool("use_ghosts_to_compute_dt", true);

    /***
     *  create hyp_level_integrator and error_detector
     */
    hyperbolic_patch_strategy.reset(new SAMRAIHyperbolicPatchStrategyAdapter(GetContext(), grid_geometry));

    hyp_level_integrator.reset(new SAMRAI::algs::HyperbolicLevelIntegrator(
        "SAMRAILevelIntegrator", HyperbolicLevelIntegrator, hyperbolic_patch_strategy.get(), use_refined_timestepping));

    auto StandardTagAndInitialize = boost::make_shared<SAMRAI::tbox::MemoryDatabase>("StandardTagAndInitialize");
    // Refer to mesh::StandardTagAndInitialize for input
    StandardTagAndInitialize->putString("tagging_method", "GRADIENT_DETECTOR");

    auto error_detector = boost::make_shared<SAMRAI::mesh::StandardTagAndInitialize>(
        "StandardTagAndInitialize", hyp_level_integrator.get(), StandardTagAndInitialize);
    /*********************************************************************
     *  create grid_algorithm
     */
    auto BergerRigoutsos = boost::make_shared<SAMRAI::tbox::MemoryDatabase>("BergerRigoutsos");

    BergerRigoutsos->putBool("sort_output_nodes", true);       // Makes results repeatable.
    BergerRigoutsos->putDouble("efficiency_tolerance", 0.85);  // min % of GetTag cells in new patch level,
    BergerRigoutsos->putDouble("combine_efficiency", 0.95);    //  chop box if  sum of volumes of   smaller
    // boxes <  efficiency * vol of large box

    auto box_generator = boost::make_shared<SAMRAI::mesh::BergerRigoutsos>(dim, BergerRigoutsos);

    box_generator->useDuplicateMPI(SAMRAI::tbox::SAMRAI_MPI::getSAMRAIWorld());

    auto LoadBalancer = boost::make_shared<SAMRAI::tbox::MemoryDatabase>("LoadBalancer");
    auto load_balancer = boost::make_shared<SAMRAI::mesh::CascadePartitioner>(dim, "LoadBalancer", LoadBalancer);

    load_balancer->setSAMRAI_MPI(SAMRAI::tbox::SAMRAI_MPI::getSAMRAIWorld());
    load_balancer->printStatistics(std::cout);

    auto GriddingAlgorithm = boost::make_shared<SAMRAI::tbox::MemoryDatabase>("GriddingAlgorithm");

    auto gridding_algorithm = boost::make_shared<SAMRAI::mesh::GriddingAlgorithm>(
        patch_hierarchy, "GriddingAlgorithm", GriddingAlgorithm, error_detector, box_generator, load_balancer);

    // Refer to algs::TimeRefinementIntegrator for input
    auto TimeRefinementIntegrator = boost::make_shared<SAMRAI::tbox::MemoryDatabase>("TimeRefinementIntegrator");

    TimeRefinementIntegrator->putDouble("start_time", engine::TimeIntegrator::GetTime());   // initial simulation time
    TimeRefinementIntegrator->putDouble("end_time", engine::TimeIntegrator::GetTimeEnd());  // final simulation time
    TimeRefinementIntegrator->putDouble("grow_dt", 1.1);  // growth factor for timesteps
    TimeRefinementIntegrator->putInteger("max_integrator_steps", 100);

    m_time_refinement_integrator_.reset(
        new SAMRAI::algs::TimeRefinementIntegrator("TimeRefinementIntegrator", TimeRefinementIntegrator,
                                                   patch_hierarchy, hyp_level_integrator, gridding_algorithm));

    visit_data_writer.reset(new SAMRAI::appu::VisItDataWriter(dim, "SimPLA VisIt Writer", GetOutputURL(), 1));

    hyperbolic_patch_strategy->registerVisItDataWriter(visit_data_writer);

    m_time_refinement_integrator_->initializeHierarchy();

    grid_geometry->printClassData(std::cout);
    hyp_level_integrator->printClassData(std::cout);
    //    m_time_refinement_integrator_->printClassData(std::cout);
    MESSAGE << "==================  Context is initialized!  =================" << std::endl;
};
void SAMRAITimeIntegrator::Finalize() {
    visit_data_writer.reset();
    m_time_refinement_integrator_.reset();
    hyp_level_integrator.reset();
    hyperbolic_patch_strategy.reset();
    engine::TimeIntegrator::Finalize();
}
Real SAMRAITimeIntegrator::Advance(Real dt) {
    ASSERT(m_time_refinement_integrator_ != nullptr);

    // SetTime(m_time_refinement_integrator_->getIntegratorTime());
    Real loop_time = GetTime();
    Real loop_time_end = std::min(loop_time + dt, GetTimeEnd());
    Real loop_dt = dt;
    while ((loop_time < loop_time_end) && (loop_dt > 0)) {  //&& m_time_refinement_integrator_->stepsRemaining() > 0
        Real dt_new = m_time_refinement_integrator_->advanceHierarchy(loop_dt, false);
        loop_dt = std::min(dt_new, loop_time_end - loop_time);
        loop_time += loop_dt;
    }

    SetTime(loop_time_end);
    return loop_time_end;
}
void SAMRAITimeIntegrator::CheckPoint() const {
    if (visit_data_writer != nullptr) {
        VERBOSE << "Check Point at Step " << m_time_refinement_integrator_->getIntegratorStep() << std::endl;
        visit_data_writer->writePlotData(patch_hierarchy, m_time_refinement_integrator_->getIntegratorStep(),
                                         m_time_refinement_integrator_->getIntegratorTime());
    }
}
void SAMRAITimeIntegrator::Dump() const {
    //    if (visit_data_writer != nullptr) {
    //        VERBOSE << "Dump : Step = " << GetNumberOfStep() << std::end;
    //        visit_data_writer->writePlotData(patch_hierarchy, m_time_refinement_integrator_->getIntegratorStep(),
    //                                         m_time_refinement_integrator_->getIntegratorTime());
    //    }
}
bool SAMRAITimeIntegrator::Done() const {
    // m_time_refinement_integrator_ != nullptr ? !m_time_refinement_integrator_->stepsRemaining():;
    return engine::TimeIntegrator::Done();
}
}  // namespace simpla

//
// namespace data {
// class DataBackendSAMRAI : public data::DataBackend {
// SP_OBJECT_HEAD(DataBackendSAMRAI, data::DataBackend);
//
// public:
//    DataBackendSAMRAI();
//    DataBackendSAMRAI(DataBackendSAMRAI const &);
//    DataBackendSAMRAI(DataBackendSAMRAI &&);
//
//    DataBackendSAMRAI(std::string const &uri, std::string const &status = "");
//    virtual ~DataBackendSAMRAI();
//
//    virtual std::ostream &Print(std::ostream &os, int indent = 0) const;
//    virtual std::shared_ptr<data::DataBackend> Duplicate() const;
//    virtual std::shared_ptr<data::DataBackend> CreateNew() const;
//    virtual bool isNull() const;
//    virtual void Flush();
//
//    virtual std::shared_ptr<data::DataEntity> Get(std::string const &URI) const;
//    virtual void Set(std::string const &uri, std::shared_ptr<data::DataEntity> const &v, bool overwrite = true);
//    virtual void Add(std::string const &uri, std::shared_ptr<data::DataEntity> const &v);
//    virtual void Delete(std::string const &URI);
//    virtual size_type size() const;
//    virtual size_type Foreach(
//            std::function<void(std::string const &, std::shared_ptr<data::DataEntity>)> const &) const;
//
//    boost::shared_ptr<SAMRAI::tbox::Database> samrai_db();
//
// private:
//    boost::shared_ptr<SAMRAI::tbox::Database> m_samrai_db_ = nullptr;
//    static std::regex sub_group_regex;
//    static std::regex match_path;
//
//    typedef boost::shared_ptr<SAMRAI::tbox::Database> table_type;
//
//    static std::shared_ptr<DataBackendSAMRAI> CreateBackend(boost::shared_ptr<SAMRAI::tbox::Database> const &db) {
//        auto res = std::make_shared<DataBackendSAMRAI>();
//        res->m_samrai_db_ = db;
//        return res;
//    };
//
//    static std::shared_ptr<data::DataEntity> get_data_from_samrai(boost::shared_ptr<SAMRAI::tbox::Database> const
//    &lobj,
//                                                                  std::string const &key);
//    static void add_data_to_samrai(boost::shared_ptr<SAMRAI::tbox::Database> &lobj, std::string const &uri,
//                                   std::shared_ptr<data::DataEntity> const &v);
//    static void set_data_to_samrai(boost::shared_ptr<SAMRAI::tbox::Database> &lobj, std::string const &uri,
//                                   std::shared_ptr<data::DataEntity> const &v);
//
//    static std::pair<table_type, std::string> get_table(table_type self, std::string const &uri,
//                                                        bool return_if_not_exist = true);
//};
//
// std::pair<DataBackendSAMRAI::table_type, std::string> DataBackendSAMRAI::get_table(table_type t, std::string const
// &uri,
//                                                                                   bool return_if_not_exist) {
//    return data::HierarchicalTableForeach(
//            t, uri, [&](table_type s_t, std::string const &k) { return s_t->isDatabase(k); },
//            [&](table_type s_t, std::string const &k) { return s_t->getDatabase(k); },
//            [&](table_type s_t, std::string const &k) {
//                return return_if_not_exist ? static_cast<table_type>(nullptr) : s_t->putDatabase(k);
//            });
//};
//
// DataBackendSAMRAI::DataBackendSAMRAI() : m_samrai_db_(boost::make_shared<SAMRAI::tbox::MemoryDatabase>("")) { ; }
// DataBackendSAMRAI::DataBackendSAMRAI(DataBackendSAMRAI const &other) : m_samrai_db_(other.m_samrai_db_){};
// DataBackendSAMRAI::DataBackendSAMRAI(DataBackendSAMRAI &&other) : m_samrai_db_(other.m_samrai_db_){};
// DataBackendSAMRAI::DataBackendSAMRAI(std::string const &uri, std::string const &status)
//        : m_samrai_db_(boost::make_shared<SAMRAI::tbox::MemoryDatabase>(uri)) {}
//
// DataBackendSAMRAI::~DataBackendSAMRAI() {
//    if (m_samrai_db_ != nullptr) { m_samrai_db_->close(); }
//}
// std::ostream &DataBackendSAMRAI::Print(std::ostream &os, int indent) const {
//    m_samrai_db_->printClassData(os);
//    return os;
//}
//
// boost::shared_ptr<SAMRAI::tbox::Database> DataBackendSAMRAI::samrai_db() { return m_samrai_db_; }
// std::shared_ptr<data::DataBackend> DataBackendSAMRAI::Duplicate() const {
//    return std::make_shared<DataBackendSAMRAI>(*this);
//}
// std::shared_ptr<data::DataBackend> DataBackendSAMRAI::CreateNew() const {
//    return std::make_shared<DataBackendSAMRAI>();
//}
//
// void DataBackendSAMRAI::Flush() { UNSUPPORTED; }
// bool DataBackendSAMRAI::isNull() const { return m_samrai_db_ == nullptr; }
// size_type DataBackendSAMRAI::size() const { return m_samrai_db_->getAllKeys().size(); }
//
//// namespace detail {
// void DataBackendSAMRAI::set_data_to_samrai(boost::shared_ptr<SAMRAI::tbox::Database> &dest, std::string const &uri,
//                                           std::shared_ptr<data::DataEntity> const &src) {
//    if (src->isTable()) {
//        auto sub_db = uri == "" ? dest : dest->putDatabase(uri);
//        src->cast_as<data::DataTable>().Foreach([&](std::string const &k, std::shared_ptr<data::DataEntity> const &v)
//        {
//            set_data_to_samrai(sub_db, k, v);
//        });
//    } else if (uri == "") {
//        return;
//    } else if (src->isNull()) {
//        dest->putDatabase(uri);
//    } else if (src->isBlock()) {
//    } else if (src->isA(typeid(data::DataEntityWrapper<index_box_type>))) {
//    } else if (src->isArray()) {
//        if (src->value_type_info() == typeid(bool)) {
//            auto &varray = src->cast_as<data::DataEntityWrapper<bool *>>().get();
//            bool d[varray.size()];
//            size_type num = varray.size();
//            for (int i = 0; i < num; ++i) { d[i] = varray[i]; }
//            dest->putBoolArray(uri, d, num);
//        } else if (src->value_type_info() == typeid(std::string)) {
//            auto &varray = src->cast_as<data::DataEntityWrapper<std::string *>>().get();
//            dest->putStringArray(uri, &varray[0], varray.size());
//        } else if (src->value_type_info() == typeid(double)) {
//            auto &varray = src->cast_as<data::DataEntityWrapper<double *>>().get();
//            dest->putDoubleArray(uri, &varray[0], varray.size());
//        } else if (src->value_type_info() == typeid(int)) {
//            auto &varray = src->cast_as<data::DataEntityWrapper<int *>>().get();
//            dest->putIntegerArray(uri, &varray[0], varray.size());
//        } else if (src->cast_as<data::DataArray>().Get(0)->isArray() && src->cast_as<data::DataArray>().size() >= 3 &&
//                   src->cast_as<data::DataArray>().Get(0)->value_type_info() == typeid(int)) {
//            nTuple<int, 3> i_lo = data::data_cast<nTuple<int, 3>>(*src->cast_as<data::DataArray>().Get(0));
//            nTuple<int, 3> i_up = data::data_cast<nTuple<int, 3>>(*src->cast_as<data::DataArray>().Get(1));
//
//            SAMRAI::tbox::Dimension dim(3);
//            dest->putDatabaseBox(uri, SAMRAI::tbox::DatabaseBox(dim, &(i_lo[0]), &(i_up[0])));
//        }
//    } else if (src->isLight()) {
//        if (src->value_type_info() == typeid(bool)) {
//            dest->putBool(uri, data::data_cast<bool>(*src));
//        } else if (src->value_type_info() == typeid(std::string)) {
//            dest->putString(uri, data::data_cast<std::string>(*src));
//        } else if (src->value_type_info() == typeid(double)) {
//            dest->putDouble(uri, data::data_cast<double>(*src));
//        } else if (src->value_type_info() == typeid(int)) {
//            dest->putInteger(uri, data::data_cast<int>(*src));
//        }
//    } else {
//        WARNING << " Unknown value_type_info " << *src << " " << std::endl;
//    }
//}
// void DataBackendSAMRAI::add_data_to_samrai(boost::shared_ptr<SAMRAI::tbox::Database> &lobj, std::string const &uri,
//                                           std::shared_ptr<data::DataEntity> const &v) {
//    UNSUPPORTED;
//}
// std::shared_ptr<data::DataEntity> DataBackendSAMRAI::get_data_from_samrai(
//        boost::shared_ptr<SAMRAI::tbox::Database> const &lobj, std::string const &key) {
//    if (!lobj->keyExists(key)) { return nullptr; };
//    std::shared_ptr<data::DataEntity> res = nullptr;
//    switch (lobj->getArrayType(key)) {
//        case SAMRAI::tbox::Database::SAMRAI_BOOL:
//            res = data::make_data_entity(lobj->getBool(key));
//            break;
//        case SAMRAI::tbox::Database::SAMRAI_DOUBLE:
//            res = data::make_data_entity(lobj->getDouble(key));
//            break;
//        case SAMRAI::tbox::Database::SAMRAI_INT:
//            res = data::make_data_entity(lobj->getInteger(key));
//            break;
//        default:
//            break;
//    }
//
//    return res;
//}
//
// std::shared_ptr<data::DataEntity> DataBackendSAMRAI::Get(std::string const &uri) const {
//    auto res = get_table(m_samrai_db_, uri, true);
//    return (res.first == nullptr || res.second == "") ? std::make_shared<data::DataEntity>()
//                                                      : get_data_from_samrai(res.first, res.second);
//}
//
// void DataBackendSAMRAI::Set(std::string const &uri, std::shared_ptr<data::DataEntity> const &v, bool overwrite) {
//    auto res = get_table(m_samrai_db_, uri, false);
//    if (res.first != nullptr && res.second != "") { set_data_to_samrai(res.first, res.second, v); }
//}
//
// void DataBackendSAMRAI::Add(std::string const &uri, std::shared_ptr<data::DataEntity> const &v) {
//    auto res = get_table(m_samrai_db_, uri, false);
//    if (res.second != "") { add_data_to_samrai(res.first, res.second, v); }
//}
//
// void DataBackendSAMRAI::Delete(std::string const &uri) {
//    auto res = get_table(m_samrai_db_, uri, true);
//    res.first->putDatabase(res.second);
//}
//
// size_type DataBackendSAMRAI::Foreach(
//        std::function<void(std::string const &, std::shared_ptr<data::DataEntity>)> const &fun) const {
//    auto keys = m_samrai_db_->getAllKeys();
//    for (auto const &k : keys) { fun(k, get_data_from_samrai(m_samrai_db_, k)); }
//    return 0;
//}
//}  // namespace data{
