// Filename: CCPoissonPETScLevelSolver.cpp
// Created on 30 Aug 2010 by Boyce Griffith
//
// Copyright (c) 2002-2014, Boyce Griffith
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

#include <stddef.h>
#include <string>
#include <vector>

#include "SAMRAI/pdat/CellData.h"
#include "SAMRAI/pdat/CellDataFactory.h"
#include "SAMRAI/pdat/CellVariable.h"
#include "SAMRAI/hier/IntVector.h"

#include "SAMRAI/hier/Patch.h"
#include "SAMRAI/hier/PatchDescriptor.h"
#include "SAMRAI/hier/PatchGeometry.h"
#include "SAMRAI/hier/PatchHierarchy.h"
#include "SAMRAI/hier/PatchLevel.h"
#include "SAMRAI/xfer/RefineSchedule.h"
#include "SAMRAI/solv/SAMRAIVectorReal.h"
#include "SAMRAI/hier/Variable.h"
#include "SAMRAI/hier/VariableContext.h"
#include "SAMRAI/hier/VariableDatabase.h"
#include "ibtk/CCPoissonPETScLevelSolver.h"
#include "ibtk/GeneralSolver.h"
#include "ibtk/IBTK_CHKERRQ.h"
#include "ibtk/PETScLevelSolver.h"
#include "ibtk/PETScMatUtilities.h"
#include "ibtk/PETScVecUtilities.h"
#include "ibtk/PoissonUtilities.h"
#include "ibtk/ibtk_utilities.h"
#include "ibtk/namespaces.h" // IWYU pragma: keep
#include "petscmat.h"
#include "petscsys.h"
#include "petscvec.h"
#include "SAMRAI/tbox/Database.h"

#include "SAMRAI/tbox/SAMRAI_MPI.h"

/////////////////////////////// NAMESPACE ////////////////////////////////////

namespace IBTK
{
/////////////////////////////// STATIC ///////////////////////////////////////

namespace
{
// Number of ghosts cells used for each variable quantity.
static const int CELLG = 1;
}

/////////////////////////////// PUBLIC ///////////////////////////////////////

CCPoissonPETScLevelSolver::CCPoissonPETScLevelSolver(const std::string& object_name,
                                                     boost::shared_ptr<Database> input_db,
                                                     const std::string& default_options_prefix)
    : d_context(NULL), d_dof_index_idx(-1), d_dof_index_var(NULL), d_data_synch_sched(NULL), d_ghost_fill_sched(NULL)
{
    // Configure solver.
    GeneralSolver::init(object_name, /*homogeneous_bc*/ false);
    PETScLevelSolver::init(input_db, default_options_prefix);

    // Construct the DOF index variable/context.
    VariableDatabase* var_db = VariableDatabase::getDatabase();
    d_context = var_db->getContext(object_name + "::CONTEXT");
    d_dof_index_var = boost::make_shared<CellVariable<int> >(DIM, object_name + "::dof_index");
    if (var_db->checkVariableExists(d_dof_index_var->getName()))
    {
        d_dof_index_var = var_db->getVariable(d_dof_index_var->getName());
        d_dof_index_idx = var_db->mapVariableAndContextToIndex(d_dof_index_var, d_context);
        var_db->removePatchDataIndex(d_dof_index_idx);
    }
    d_dof_index_idx = var_db->registerVariableAndContext(d_dof_index_var, d_context, IntVector(DIM, CELLG));
    return;
}

CCPoissonPETScLevelSolver::~CCPoissonPETScLevelSolver()
{
    if (d_is_initialized) deallocateSolverState();
    return;
}

/////////////////////////////// PROTECTED ////////////////////////////////////

void CCPoissonPETScLevelSolver::initializeSolverStateSpecialized(const SAMRAIVectorReal<double>& x,
                                                                 const SAMRAIVectorReal<double>& /*b*/)
{
    // Allocate DOF index data.
    VariableDatabase* var_db = VariableDatabase::getDatabase();
    const int x_idx = x.getComponentDescriptorIndex(0);
    auto x_var = BOOST_CAST<CellVariable<double> >(x.getComponentVariable(0));
    TBOX_ASSERT(x_var);
    const int depth = x_var->getDepth();
    if (d_dof_index_var->getDepth() != depth)
    {
        var_db->removePatchDataIndex(d_dof_index_idx);
        d_dof_index_var =
            boost::make_shared<CellVariable<int> >(d_dof_index_var->getDim(), d_dof_index_var->getName(), depth);
        d_dof_index_idx = var_db->registerVariableAndContext(d_dof_index_var, d_context, IntVector(DIM, CELLG));
    }
    auto level = d_hierarchy->getPatchLevel(d_level_num);
    if (!level->checkAllocated(d_dof_index_idx)) level->allocatePatchData(d_dof_index_idx);

    // Setup PETSc objects.
    int ierr;
    PETScVecUtilities::constructPatchLevelDOFIndices(d_num_dofs_per_proc, d_dof_index_idx, level);
    tbox::SAMRAI_MPI comm(MPI_COMM_WORLD);
    const int mpi_rank = comm.getRank();
    ierr = VecCreateMPI(PETSC_COMM_WORLD, d_num_dofs_per_proc[mpi_rank], PETSC_DETERMINE, &d_petsc_x);
    IBTK_CHKERRQ(ierr);
    ierr = VecCreateMPI(PETSC_COMM_WORLD, d_num_dofs_per_proc[mpi_rank], PETSC_DETERMINE, &d_petsc_b);
    IBTK_CHKERRQ(ierr);
    PETScMatUtilities::constructPatchLevelCCLaplaceOp(d_petsc_mat, d_poisson_spec, d_bc_coefs, d_solution_time,
                                                      d_num_dofs_per_proc, d_dof_index_idx, level);
    d_petsc_pc = d_petsc_mat;
    d_petsc_ksp_ops_flag = SAME_PRECONDITIONER;
    d_data_synch_sched = PETScVecUtilities::constructDataSynchSchedule(x_idx, level);
    d_ghost_fill_sched = PETScVecUtilities::constructGhostFillSchedule(x_idx, level);
    return;
}

void CCPoissonPETScLevelSolver::deallocateSolverStateSpecialized()
{
    // Deallocate DOF index data.
    auto level = d_hierarchy->getPatchLevel(d_level_num);
    if (level->checkAllocated(d_dof_index_idx)) level->deallocatePatchData(d_dof_index_idx);
    return;
}

void CCPoissonPETScLevelSolver::copyToPETScVec(Vec& petsc_x,
                                               SAMRAIVectorReal<double>& x,
                                               boost::shared_ptr<PatchLevel> patch_level)
{
    const int x_idx = x.getComponentDescriptorIndex(0);
    PETScVecUtilities::copyToPatchLevelVec(petsc_x, x_idx, d_dof_index_idx, patch_level);
    return;
}

void CCPoissonPETScLevelSolver::copyFromPETScVec(Vec& petsc_x,
                                                 SAMRAIVectorReal<double>& x,
                                                 boost::shared_ptr<PatchLevel> patch_level)
{
    const int x_idx = x.getComponentDescriptorIndex(0);
    PETScVecUtilities::copyFromPatchLevelVec(petsc_x, x_idx, d_dof_index_idx, patch_level, d_data_synch_sched,
                                             d_ghost_fill_sched);
    return;
}

void CCPoissonPETScLevelSolver::setupKSPVecs(Vec& petsc_x,
                                             Vec& petsc_b,
                                             SAMRAIVectorReal<double>& x,
                                             SAMRAIVectorReal<double>& b,
                                             boost::shared_ptr<PatchLevel> patch_level)
{
    if (!d_initial_guess_nonzero) copyToPETScVec(petsc_x, x, patch_level);
    const int b_idx = b.getComponentDescriptorIndex(0);
    auto b_var = BOOST_CAST<CellVariable<double> >(b.getComponentVariable(0));
    VariableDatabase* var_db = VariableDatabase::getDatabase();
    int b_adj_idx = var_db->registerClonedPatchDataIndex(b_var, b_idx);
    patch_level->allocatePatchData(b_adj_idx);
    for (auto p = patch_level->begin(); p != patch_level->end(); ++p)
    {
        auto patch = *p;
        auto b_data = BOOST_CAST<CellData<double> >(patch->getPatchData(b_idx));
        auto b_adj_data = BOOST_CAST<CellData<double> >(patch->getPatchData(b_adj_idx));
        b_adj_data->copy(*b_data);
        if (!patch->getPatchGeometry()->intersectsPhysicalBoundary()) continue;
        PoissonUtilities::adjustCCBoundaryRhsEntries(patch, *b_adj_data, d_poisson_spec, d_bc_coefs, d_solution_time,
                                                     d_homogeneous_bc);
    }
    PETScVecUtilities::copyToPatchLevelVec(petsc_b, b_adj_idx, d_dof_index_idx, patch_level);
    patch_level->deallocatePatchData(b_adj_idx);
    var_db->removePatchDataIndex(b_adj_idx);
    return;
}

/////////////////////////////// PRIVATE //////////////////////////////////////

/////////////////////////////// NAMESPACE ////////////////////////////////////

} // namespace IBTK

//////////////////////////////////////////////////////////////////////////////
