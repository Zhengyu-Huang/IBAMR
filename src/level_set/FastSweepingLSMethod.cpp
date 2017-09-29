// Filename: FastSweepingLSMethod.cpp
// Created on 27 Sep 2017 by Nishant Nangia and Amneet Bhalla
//
// Copyright (c) 2002-2014, Nishant Nangia and Amneet Bhalla
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of The University of North Carolina nor the names of
//      its contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

/////////////////////////////// INCLUDES /////////////////////////////////////

#include "ibamr/FastSweepingLSMethod.h"
#include "CellVariable.h"
#include "HierarchyCellDataOpsReal.h"
#include "IBAMR_config.h"
#include "VariableDatabase.h"
#include "ibamr/namespaces.h"
#include "ibtk/HierarchyGhostCellInterpolation.h"
#include "ibtk/HierarchyMathOps.h"
#include "tbox/RestartManager.h"

// FORTRAN ROUTINES
#if (NDIM == 2)
#define FAST_SWEEP_1ST_ORDER_FC IBAMR_FC_FUNC(fastsweep1storder2d, FASTSWEEP1STORDER2D)
#define FAST_SWEEP_2ND_ORDER_FC IBAMR_FC_FUNC(fastsweep2ndorder2d, FASTSWEEP2NDORDER2D)
#endif

#if (NDIM == 3)
#define FAST_SWEEP_1ST_ORDER_FC IBAMR_FC_FUNC(fastsweep1storder3d, FASTSWEEP1STORDER3D)
#define FAST_SWEEP_2ND_ORDER_FC IBAMR_FC_FUNC(fastsweep2ndorder3d, FASTSWEEP2NDORDER3D)
#endif

extern "C" {
void FAST_SWEEP_1ST_ORDER_FC(double* U,
                             const int& U_gcw,
                             const int& ilower0,
                             const int& iupper0,
                             const int& ilower1,
                             const int& iupper1,
#if (NDIM == 3)
                             const int& ilower2,
                             const int& iupper2,
#endif
                             const int& dlower0,
                             const int& dupper0,
                             const int& dlower1,
                             const int& dupper1,
#if (NDIM == 3)

                             const int& dlower2,
                             const int& dupper2,
#endif
                             const double* dx,
                             const int& patch_touches_bdry,
                             const int& consider_bdry_wall);

void FAST_SWEEP_2ND_ORDER_FC(double* U,
                             const int& U_gcw,
                             const int& ilower0,
                             const int& iupper0,
                             const int& ilower1,
                             const int& iupper1,
#if (NDIM == 3)

                             const int& ilower2,
                             const int& iupper2,
#endif
                             const int& dlower0,
                             const int& dupper0,
                             const int& dlower1,
                             const int& dupper1,
#if (NDIM == 3)

                             const int& dlower2,
                             const int& dupper2,
#endif
                             const double* dx,
                             const int& patch_touches_bdry,
                             const int& consider_bdry_wall);
}

/////////////////////////////// NAMESPACE ////////////////////////////////////

namespace IBAMR
{
/////////////////////////////// STATIC ///////////////////////////////////////

/////////////////////////////// PUBLIC ///////////////////////////////////////

FastSweepingLSMethod::FastSweepingLSMethod(const std::string& object_name,
                                           Pointer<Database> db,
                                           bool register_for_restart)
    : LSInitStrategy(object_name, register_for_restart), d_ls_order(FIRST_ORDER_LS)
{
    // Some default values.
    d_ls_order = FIRST_ORDER_LS;
    d_max_its = 100;
    d_sweep_abs_tol = 1e-5;
    d_enable_logging = false;
    d_consider_phys_bdry_wall = false;

    if (d_registered_for_restart) getFromRestart();
    if (!db.isNull()) getFromInput(db);

    return;
} // FastSweepingLSMethod

FastSweepingLSMethod::~FastSweepingLSMethod()
{
    if (d_registered_for_restart)
    {
        RestartManager::getManager()->unregisterRestartItem(d_object_name);
    }
    d_registered_for_restart = false;

} // ~FastSweepingLSMethod

void
FastSweepingLSMethod::registerInterfaceNeighborhoodLocatingFcn(LocateInterfaceNeighborhoodFcnPtr callback_fcn,
                                                               void* ctx)
{
    d_locate_interface_fcns.push_back(callback_fcn);
    d_locate_interface_fcns_ctx.push_back(ctx);

    return;
} // registerInterfaceNeighborhoodLocatingFcn

void
FastSweepingLSMethod::initializeLSData(int D_idx,
                                       Pointer<HierarchyMathOps> hier_math_ops,
                                       double time,
                                       bool initial_time)
{
    VariableDatabase<NDIM>* var_db = VariableDatabase<NDIM>::getDatabase();
    Pointer<Variable<NDIM> > data_var;
    var_db->mapIndexToVariable(D_idx, data_var);
    Pointer<CellVariable<NDIM, double> > D_var = data_var;
#if !defined(NDEBUG)
    TBOX_ASSERT(!D_var.isNull());
#endif

    Pointer<PatchHierarchy<NDIM> > hierarchy = hier_math_ops->getPatchHierarchy();
    const int coarsest_ln = 0;
    const int finest_ln = hierarchy->getFinestLevelNumber();

    // Create a temporary variable to hold previous iteration values.
    const int D_iter_idx = var_db->registerClonedPatchDataIndex(D_var, D_idx);
    for (int ln = coarsest_ln; ln <= finest_ln; ++ln)
    {
        hierarchy->getPatchLevel(ln)->allocatePatchData(D_iter_idx, time);
    }

    // First, fill cells with some large positive/negative values
    // away from the interface and actual distance value near the interface.
    for (unsigned k = 0; k < d_locate_interface_fcns.size(); ++k)
    {
        (*d_locate_interface_fcns[k])(D_idx, hierarchy, time, initial_time, d_locate_interface_fcns_ctx[k]);
    }

    // Set hierarchy objects.
    typedef HierarchyGhostCellInterpolation::InterpolationTransactionComponent InterpolationTransactionComponent;
    InterpolationTransactionComponent D_transaction(
        D_idx, "CONSERVATIVE_LINEAR_REFINE", true, "CONSERVATIVE_COARSEN", "LINEAR", false, d_bc_coef);
    Pointer<HierarchyGhostCellInterpolation> fill_op = new HierarchyGhostCellInterpolation();
    HierarchyCellDataOpsReal<NDIM, double> hier_cc_data_ops(hierarchy, coarsest_ln, finest_ln);

    // Carry out iterations
    double diff_L2_norm = 1.0e12;
    int outer_iter = 0;
    const int cc_wgt_idx = hier_math_ops->getCellWeightPatchDescriptorIndex();
    while (diff_L2_norm > d_sweep_abs_tol && outer_iter < d_max_its)
    {
        hier_cc_data_ops.copyData(D_iter_idx, D_idx);

        fill_op->initializeOperatorState(D_transaction, hierarchy);
        fill_op->fillData(time);
        fastSweep(hier_math_ops, D_idx);

        hier_cc_data_ops.axmy(D_iter_idx, 1.0, D_iter_idx, D_idx);
        diff_L2_norm = hier_cc_data_ops.L2Norm(D_iter_idx, cc_wgt_idx);

        outer_iter += 1;

        if (d_enable_logging)
        {
            pout << d_object_name << "::initializeLSData(): After iteration # " << outer_iter << std::endl;
            pout << d_object_name << "::initializeLSData(): L2-norm between successive iterations = " << diff_L2_norm
                 << std::endl;
        }

        if (diff_L2_norm <= d_sweep_abs_tol && d_enable_logging)
        {
            pout << d_object_name << "::initializeLSData(): Fast sweeping algorithm converged" << std::endl;
        }
    }

    if (outer_iter >= d_max_its)
    {
        if (d_enable_logging)
        {
            pout << d_object_name << "::initializeLSData(): Reached maximum allowable outer iterations" << std::endl;
            pout << d_object_name << "::initializeLSData(): Fast sweeping algorithm likely diverged" << std::endl;
            pout << d_object_name << "::initializeLSData(): ||distance_new - distance_old||_2 = " << diff_L2_norm
                 << std::endl;
        }
        TBOX_ERROR("FastSweeping method not converged. Exiting.....\n");
    }

    return;
} // initializeLSData

/////////////////////////////// PRIVATE //////////////////////////////////////

void
FastSweepingLSMethod::fastSweep(Pointer<HierarchyMathOps> hier_math_ops, int dist_idx) const
{
    Pointer<PatchHierarchy<NDIM> > hierarchy = hier_math_ops->getPatchHierarchy();
    const int coarsest_ln = 0;
    const int finest_ln = hierarchy->getFinestLevelNumber();

    for (int ln = coarsest_ln; ln <= finest_ln; ++ln)
    {
        Pointer<PatchLevel<NDIM> > level = hierarchy->getPatchLevel(ln);
        const BoxArray<NDIM>& domain_boxes = level->getPhysicalDomain();
#if !defined(NDEBUG)
        TBOX_ASSERT(domain_boxes.size() == 1);
#endif

        for (PatchLevel<NDIM>::Iterator p(level); p; p++)
        {
            Pointer<Patch<NDIM> > patch = level->getPatch(p());
            Pointer<CellData<NDIM, double> > dist_data = patch->getPatchData(dist_idx);
            const bool patch_touches_bdry =
                level->patchTouchesRegularBoundary(p()) || level->patchTouchesPeriodicBoundary(p());
            fastSweep(dist_data, patch, domain_boxes[0], patch_touches_bdry);
        }
    }
    return;

} // fastSweep

void
FastSweepingLSMethod::fastSweep(Pointer<CellData<NDIM, double> > dist_data,
                                const Pointer<Patch<NDIM> > patch,
                                const Box<NDIM>& domain_box,
                                bool patch_touches_bdry) const
{
    double* const D = dist_data->getPointer(0);
    const int D_ghosts = (dist_data->getGhostCellWidth()).max();

#if !defined(NDEBUG)
    TBOX_ASSERT(dist_data->getDepth() == 1);
    if (d_ls_order == FIRST_ORDER_LS) TBOX_ASSERT(D_ghosts >= 1);
    if (d_ls_order == SECOND_ORDER_LS) TBOX_ASSERT(D_ghosts >= 2);
#endif

    const Box<NDIM>& patch_box = patch->getBox();
    const Pointer<CartesianPatchGeometry<NDIM> > pgeom = patch->getPatchGeometry();
    const double* const dx = pgeom->getDx();

    if (d_ls_order == FIRST_ORDER_LS)
    {
        FAST_SWEEP_1ST_ORDER_FC(D,
                                D_ghosts,
                                patch_box.lower(0),
                                patch_box.upper(0),
                                patch_box.lower(1),
                                patch_box.upper(1),
#if (NDIM == 3)
                                patch_box.lower(2),
                                patch_box.upper(2),
#endif
                                domain_box.lower(0),
                                domain_box.upper(0),
                                domain_box.lower(1),
                                domain_box.upper(1),
#if (NDIM == 3)
                                domain_box.lower(2),
                                domain_box.upper(2),
#endif
                                dx,
                                patch_touches_bdry,
                                d_consider_phys_bdry_wall);
    }
    else if (d_ls_order == SECOND_ORDER_LS)
    {
        FAST_SWEEP_2ND_ORDER_FC(D,
                                D_ghosts,
                                patch_box.lower(0),
                                patch_box.upper(0),
                                patch_box.lower(1),
                                patch_box.upper(1),
#if (NDIM == 3)
                                patch_box.lower(2),
                                patch_box.upper(2),
#endif
                                domain_box.lower(0),
                                domain_box.upper(0),
                                domain_box.lower(1),
                                domain_box.upper(1),
#if (NDIM == 3)
                                domain_box.lower(2),
                                domain_box.upper(2),
#endif
                                dx,
                                patch_touches_bdry,
                                d_consider_phys_bdry_wall);
    }
    else
    {
        TBOX_ERROR("FastSweepingLSMethod does not support " << enum_to_string(d_ls_order) << std::endl);
    }

    return;
} // fastSweep

void
FastSweepingLSMethod::getFromInput(Pointer<Database> input_db)
{
    d_max_its = input_db->getIntegerWithDefault("max_iterations", d_max_its);
    d_max_its = input_db->getIntegerWithDefault("max_its", d_max_its);

    d_sweep_abs_tol = input_db->getDoubleWithDefault("sweep_abs_tol", d_sweep_abs_tol);
    d_sweep_abs_tol = input_db->getDoubleWithDefault("abs_tol", d_sweep_abs_tol);

    d_enable_logging = input_db->getBoolWithDefault("enable_logging", d_enable_logging);
    d_consider_phys_bdry_wall = input_db->getBoolWithDefault("physical_bdry_wall", d_consider_phys_bdry_wall);

    return;
} // getFromInput

/////////////////////////////// NAMESPACE ////////////////////////////////////

} // namespace IBAMR

//////////////////////////////////////////////////////////////////////////////
