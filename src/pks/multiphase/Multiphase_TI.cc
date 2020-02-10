/*
  MultiPhase PK

  Copyright 2010-201x held jointly by LANS/LANL, LBNL, and PNNL. 
  Amanzi is released under the three-clause BSD License. 
  The terms of use and "as is" disclaimer for this license are 
  provided in the top-level COPYRIGHT file.

  Authors: Quan Bui (mquanbui@math.umd.edu)
           Konstantin Lipnikov (lipnikov@lanl.gov)

  Multiphase multi-component flow, see Multiphase_PK.cc for more detail.
*/


// TPLs
#include "Teuchos_RCP.hpp"

// Amanzi
#include "PDE_Accumulation.hh"
#include "PDE_AdvectionUpwind.hh"
#include "PDE_DiffusionFV.hh"
#include "PDE_DiffusionFVwithGravity.hh"
#include "Tensor.hh"

// Multiphase
#include "EquationStructure.hh"
#include "Multiphase_PK.hh"
#include "TotalComponentStorage.hh"

namespace Amanzi {
namespace Multiphase {

/* ******************************************************************
* F(u) implementation
****************************************************************** */
void Multiphase_PK::FunctionalResidual(double t_old, double t_new, 
                                       Teuchos::RCP<TreeVector> u_old,
                                       Teuchos::RCP<TreeVector> u_new,
                                       Teuchos::RCP<TreeVector> f) 
{
  double dtp = t_new - t_old;

  // extract pointers to subvectors
  std::vector<Teuchos::RCP<CompositeVector> > up, fp;
  for (int i = 0; i < 3; ++i) {
    up.push_back(u_new->SubVector(i)->Data());
    fp.push_back(f->SubVector(i)->Data());
  }

  // miscalleneous fields
  // -- saturation
  auto& sat_lc = *S_->GetFieldData(saturation_liquid_key_, passwd_)->ViewComponent("cell");

  // -- gas pressure
  S_->GetFieldEvaluator(pressure_gas_key_)->HasFieldChanged(S_.ptr(), passwd_);
  auto pg = S_->GetFieldData(pressure_gas_key_, pressure_gas_key_);

  // -- molar densities
  S_->GetFieldEvaluator(molar_density_gas_key_)->HasFieldChanged(S_.ptr(), passwd_);
  const auto& eta_g = S_->GetFieldData(molar_density_gas_key_);
  const auto& eta_gc = *eta_g->ViewComponent("cell");

  S_->GetFieldEvaluator(molar_density_liquid_key_)->HasFieldChanged(S_.ptr(), passwd_);
  const auto& eta_l = S_->GetFieldData(molar_density_liquid_key_);
  const auto& eta_lc = *eta_l->ViewComponent("cell");

  // -- storage
  S_->GetFieldEvaluator(tws_key_)->HasFieldChanged(S_.ptr(), passwd_);
  S_->GetFieldEvaluator(tcs_key_)->HasFieldChanged(S_.ptr(), passwd_);

  // -- porosity
  const auto& phi = *S_->GetFieldData(porosity_key_)->ViewComponent("cell");

  // -- wrapper for absolute permeability
  Teuchos::RCP<std::vector<WhetStone::Tensor> > Kptr = Teuchos::rcpFromRef(K_);

  // work memory for miscalleneous operator
  auto kr = CreateCVforUpwind(mesh_);
  auto& kr_c = *kr->ViewComponent("cell");
  auto& kr_f = *kr->ViewComponent("face");

  // primary variables
  auto tmp = up[0];
  CompositeVector fone(*tmp), fadd(*tmp), comp(*tmp);
  auto& fone_c = *fone.ViewComponent("cell");
  auto& comp_c = *comp.ViewComponent("cell");

  // start loop over physical equations
  Key key;
  for (int n = 0; n < num_primary_ + 1; ++n) {
    ModifyEvaluators(n);
    auto sol = EquationToSolution(n);
    PopulateBCs(sol.second, true);
  
    // Richards-type operator for all phases
    fone.PutScalar(0.0);

    for (int phase = 0; phase < 2; ++phase) {
      bool bcflag = (phase == 0);
      if ((key = eqns_[n].advection[phase].first) != "") {
        S_->GetFieldEvaluator(key)->HasFieldChanged(S_.ptr(), passwd_);

        // -- upwind cell-centered coefficient
        auto flux = S_->GetFieldData(flux_names_[phase], passwd_);
        kr_c = *S_->GetFieldData(key)->ViewComponent("cell");
        kr->ViewComponent("dirichlet_faces")->PutScalar(0.0);  // FIXME (BC data)
        upwind_->Compute(*flux, *kr, op_bcs_[sol.first]->bc_model(), *kr);

        // -- form operator
        auto& pde = pde_diff_K_;
        pde->Setup(Kptr, kr, Teuchos::null, rho_l_, gravity_);  // FIXME (gravity for gas phase)
        pde->SetBCs(op_bcs_[sol.first], op_bcs_[sol.first]);
        pde->global_operator()->Init();
        pde->UpdateMatrices(Teuchos::null, Teuchos::null);
        pde->ApplyBCs(bcflag, false, false);

        // -- add advection term to the residual
        Key fname = eqns_[n].advection[phase].second;
        S_->GetFieldEvaluator(fname)->HasFieldChanged(S_.ptr(), passwd_);
        auto var = S_->GetFieldData(fname);
        pde->global_operator()->ComputeNegativeResidual(*var, fadd);
        fone.Update(1.0, fadd, 1.0);
      }
    }
 
    // molecular diffusion 
    for (int phase = 0; phase < 2; ++phase) {
      if ((key = eqns_[n].diffusion[phase].first) != "") {
        S_->GetFieldEvaluator(key)->HasFieldChanged(S_.ptr(), passwd_);
        auto flux = *S_->GetFieldData(flux_names_[phase], passwd_);
        kr_c = *S_->GetFieldData(key)->ViewComponent("cell");
        upwind_->Compute(flux, *kr, op_bcs_[sol.first]->bc_model(), *kr);

        // -- form operator
        auto& pde = pde_diff_D_;
        pde->Setup(Teuchos::null, kr, Teuchos::null);
        pde->SetBCs(op_bcs_[sol.first], op_bcs_[sol.first]);
        pde->global_operator()->Init();
        pde->UpdateMatrices(Teuchos::null, Teuchos::null);
        pde->ApplyBCs(false, false, false);

        // -- add diffusion term to the residual
        Key fname = eqns_[n].diffusion[phase].second;
        S_->GetFieldEvaluator(fname)->HasFieldChanged(S_.ptr(), passwd_);
        auto& tmp = *S_->GetFieldData(fname)->ViewComponent("cell");
        int m = std::max(n - 1, tmp.NumVectors() - 1);
        for (int c = 0; c < ncells_owned_; ++c) {
          comp_c[0][c] = tmp[m][c];
        }
        pde->global_operator()->ComputeNegativeResidual(comp, fadd);

        double factor = eqns_[n].diff_factors[phase];
        fone.Update(factor, fadd, 1.0);
      }
    }

    // add storage terms 
    if ((key = eqns_[n].storage) != "") {
      std::string prev_key = "prev_" + key;
      S_->GetFieldEvaluator(key)->HasFieldChanged(S_.ptr(), passwd_);

      const auto& total_c = *S_->GetFieldData(key)->ViewComponent("cell");
      const auto& total_prev_c = *S_->GetFieldData(prev_key)->ViewComponent("cell");

      for (int c = 0; c < ncells_owned_; ++c) {
        double factor = mesh_->cell_volume(c) / dtp;
        fone_c[0][c] += (total_c[0][c] - total_prev_c[0][c]) * factor;
      }
    }

    // copy temporaty vector to residual
    auto& fc = *fp[sol.first]->ViewComponent("cell");
    for (int c = 0; c < ncells_owned_; ++c)
      fc[sol.second][c] = fone_c[0][c];
  }

  // process gas constraints
  int n = num_primary_ + 1;
  key = eqns_[n].constraint.first;
  S_->GetFieldEvaluator(key)->HasFieldChanged(S_.ptr(), passwd_);
  const auto& ncp_fc = *S_->GetFieldData(key)->ViewComponent("cell");

  key = eqns_[n].constraint.second;
  S_->GetFieldEvaluator(key)->HasFieldChanged(S_.ptr(), passwd_);
  const auto& ncp_gc = *S_->GetFieldData(key)->ViewComponent("cell");

  auto& fci = *fp[2]->ViewComponent("cell");
  if (ncp_ == "min") {
    for (int c = 0; c < ncells_owned_; ++c) {
      fci[0][c] = std::min(ncp_fc[0][c], ncp_gc[0][c]);
    }
  } else if (ncp_ == "Fischer-Burmeister") {
    for (int c = 0; c < ncells_owned_; ++c) {
      double a = ncp_fc[0][c];
      double b = ncp_gc[0][c];
      fci[0][c] = std::pow(a * a + b * b, 0.5) - (a + b);
    }
  }
}


/* ******************************************************************
* Update new preconditioner on the interval (tp-dtp, tp].
****************************************************************** */
void Multiphase_PK::UpdatePreconditioner(double tp, Teuchos::RCP<const TreeVector> u, double dtp)
{
  double t_old = tp - dtp;

  // extract pointers to subvectors
  std::vector<Teuchos::RCP<const CompositeVector> > up;
  for (int i = 0; i < 3; ++i) {
    up.push_back(u->SubVector(i)->Data());
  }

  // miscalleneous fields
  // -- saturation
  auto& sat_lc = *S_->GetFieldData(saturation_liquid_key_, passwd_)->ViewComponent("cell");

  // -- porosity
  const auto& phi = *S_->GetFieldData(porosity_key_)->ViewComponent("cell");

  // -- molar densities
  S_->GetFieldEvaluator(molar_density_gas_key_)->HasFieldChanged(S_.ptr(), passwd_);
  const auto& eta_g = S_->GetFieldData(molar_density_gas_key_);
  const auto& eta_gc = *eta_g->ViewComponent("cell");

  S_->GetFieldEvaluator(molar_density_liquid_key_)->HasFieldChanged(S_.ptr(), passwd_);
  const auto& eta_l = S_->GetFieldData(molar_density_liquid_key_);
  const auto& eta_lc = *eta_l->ViewComponent("cell");

  // -- mass density of gas phase 
  auto rho_g = Teuchos::rcp(new CompositeVector(*eta_g));

  // -- gas pressure
  S_->GetFieldEvaluator(pressure_gas_key_)->HasFieldChanged(S_.ptr(), passwd_);
  auto pg = S_->GetFieldData(pressure_gas_key_, pressure_gas_key_);

  // -- wrapper for absolute permeability
  Teuchos::RCP<std::vector<WhetStone::Tensor> > Kptr = Teuchos::rcpFromRef(K_);

  // parameter lists
  auto& adv_list = mp_list_->sublist("operators").sublist("advection operator").sublist("preconditioner");
  auto& ddf_list = mp_list_->sublist("operators").sublist("diffusion operator").sublist("preconditioner");
  auto& mdf_list = mp_list_->sublist("operators").sublist("molecular diffusion operator").sublist("preconditioner");

  // work memory for miscalleneous operator
  Key der_key;
  auto flux_tmp = Teuchos::rcp(new CompositeVector(*S_->GetFieldData(darcy_flux_liquid_key_)));
  auto flux_acc = Teuchos::rcp(new CompositeVector(*flux_tmp));

  auto kr = CreateCVforUpwind(mesh_);
  auto& kr_c = *kr->ViewComponent("cell");
  auto& kr_f = *kr->ViewComponent("face");

  CompositeVector fone(*up[0]);
  auto& fone_c = *fone.ViewComponent("cell");

  // for each operator we linearize (a) functions on which it acts and
  // (b) non-linear coefficients
  Key key;
  for (int row = 0; row < num_primary_ + 1; ++row) {
    ModifyEvaluators(row);
    auto solr = EquationToSolution(row);
    Key keyr = soln_names_[solr.first];
    PopulateBCs(solr.second, false);

    for (int col = 0; col < num_primary_ + 2; ++col) {
      auto solc = EquationToSolution(col);
      Key keyc = soln_names_[solc.first];

      bool bcflag = (row == col);

      // add empty operator to have a well-defined global operator pointer
      auto pde0 = Teuchos::rcp(new Operators::PDE_Accumulation(AmanziMesh::CELL, mesh_));
      auto global_op = pde0->global_operator();
      op_preconditioner_->SetOperatorBlock(row, col, global_op);
      kr_c.PutScalar(0.0);
      pde0->AddAccumulationTerm(*kr, "cell");

      // initialize accumulated flux
      flux_acc->PutScalar(0.0);

      //
      // Richards-type operator for all phases
      //
      for (int phase = 0; phase < 2; ++phase) {
        // -- diffusion operator div[ (K f dg/dv) grad dv ] 
        if ((key = eqns_[row].advection[phase].first) != "") {
          Key fname = eqns_[row].advection[phase].second;
          if (fname == keyc) {
            der_key = "constant_field";  // DAG does not calculate derivative when g(u)=u
          } else {
            der_key = "d" + fname + "_d" + keyc;
            S_->GetFieldEvaluator(fname)->HasFieldDerivativeChanged(S_.ptr(), passwd_, keyc);
          }

          if (S_->HasField(der_key)) {
            auto pde = Teuchos::rcp(new Operators::PDE_DiffusionFV(ddf_list, global_op));

            S_->GetFieldEvaluator(key)->HasFieldChanged(S_.ptr(), passwd_);
            const auto& coef_c = *S_->GetFieldData(key)->ViewComponent("cell");
            const auto& der_c = *S_->GetFieldData(der_key)->ViewComponent("cell");

            for (int c = 0; c < ncells_owned_; ++c) {
              kr_c[0][c] = der_c[0][c] * coef_c[0][c];
            }
            kr->ViewComponent("dirichlet_faces")->PutScalar(0.0);  // FIXME
            auto flux = *S_->GetFieldData(flux_names_[phase], passwd_);
            upwind_->Compute(flux, *kr, op_bcs_[solr.first]->bc_model(), *kr);

            pde->Setup(Kptr, kr, Teuchos::null);
            pde->SetBCs(op_bcs_[solr.first], op_bcs_[solc.first]);
            pde->UpdateMatrices(Teuchos::null, Teuchos::null);
            pde->ApplyBCs(bcflag, false, false);
          }
        }

        // -- advection operator div[ (K f grad dg/dv) dv ]
        if ((key = eqns_[row].advection[phase].first) != "") {
          Key fname = eqns_[row].advection[phase].second;
          Key der_key = "d" + fname + "_d" + keyc;
          S_->GetFieldEvaluator(fname)->HasFieldDerivativeChanged(S_.ptr(), passwd_, keyc);

          if (S_->HasField(der_key)) {
            // --- upwind gas molar mobility times molar fraction 
            S_->GetFieldEvaluator(key)->HasFieldChanged(S_.ptr(), passwd_);
            kr_c = *S_->GetFieldData(key)->ViewComponent("cell");
            kr->ViewComponent("dirichlet_faces")->PutScalar(0.0);  // FIXME
            auto flux = *S_->GetFieldData(flux_names_[phase], passwd_);
            upwind_->Compute(flux, *kr, op_bcs_[solr.first]->bc_model(), *kr);

            // --- calculate advective flux 
            auto der = S_->GetFieldData(der_key);
            pde_diff_K_->Setup(Kptr, kr, Teuchos::null, rho_l_, gravity_);  // FIXME
            pde_diff_K_->SetBCs(op_bcs_[solr.first], op_bcs_[solc.first]);
            pde_diff_K_->UpdateMatrices(Teuchos::null, Teuchos::null);
            pde_diff_K_->UpdateFlux(der.ptr(), flux_tmp.ptr());

            double factor = eqns_[row].adv_factors[phase];
            flux_acc->Update(factor, *flux_tmp, 1.0);
          }
        }

        // -- advection operator div [ (K df/dv grad g) dv ]
        if ((key = eqns_[row].advection[phase].first) != "") {
          Key der_key = "d" + key + "_d" + keyc;
          S_->GetFieldEvaluator(key)->HasFieldDerivativeChanged(S_.ptr(), passwd_, keyc);

          if (S_->HasField(der_key)) {
            // --- upwind derivative
            kr_c = *S_->GetFieldData(der_key)->ViewComponent("cell");
            kr->ViewComponent("dirichlet_faces")->PutScalar(0.0);  // FIXME
            auto flux = *S_->GetFieldData(flux_names_[phase], passwd_);
            upwind_->Compute(flux, *kr, op_bcs_[solr.first]->bc_model(), *kr);

            // --- calculate advective flux 
            Key fname = eqns_[row].advection[phase].second;
            S_->GetFieldEvaluator(fname)->HasFieldChanged(S_.ptr(), passwd_);
            auto var = S_->GetFieldData(fname);
            pde_diff_K_->Setup(Kptr, kr, Teuchos::null, rho_g, gravity_);
            pde_diff_K_->SetBCs(op_bcs_[solr.first], op_bcs_[solc.first]);
            pde_diff_K_->UpdateMatrices(Teuchos::null, Teuchos::null);
            pde_diff_K_->UpdateFlux(var.ptr(), flux_tmp.ptr());

            double factor = eqns_[row].adv_factors[phase];
            flux_acc->Update(factor, *flux_tmp, 1.0);
          }
        }
      }

      //
      // Molecular diffusion
      //
      for (int phase = 0; phase < 2; ++phase) {
        // -- diffusion operator div [ (f dg/dv) grad dv ]
        if ((key = eqns_[row].diffusion[phase].first) != "") {
          Key fname = eqns_[row].diffusion[phase].second;
          if (fname == keyc) {
            der_key = "constant_field";  // DAG does not calculate derivative when g(u)=u
          } else {
            der_key = "d" + fname + "_d" + keyc;
            S_->GetFieldEvaluator(fname)->HasFieldDerivativeChanged(S_.ptr(), passwd_, keyc);
          }

          if (S_->HasField(der_key)) {
            auto pde = Teuchos::rcp(new Operators::PDE_DiffusionFV(mdf_list, global_op));

            S_->GetFieldEvaluator(key)->HasFieldChanged(S_.ptr(), passwd_);
            const auto& coef_c = *S_->GetFieldData(key)->ViewComponent("cell");
            const auto& der_c = *S_->GetFieldData(der_key)->ViewComponent("cell");

            for (int c = 0; c < ncells_owned_; ++c) {
              kr_c[0][c] = der_c[0][c] * coef_c[0][c];
            }
            auto flux = *S_->GetFieldData(flux_names_[phase], passwd_);
            upwind_->Compute(flux, *kr, op_bcs_[solr.first]->bc_model(), *kr);

            pde->Setup(Teuchos::null, kr, Teuchos::null);
            pde->SetBCs(op_bcs_[solr.first], op_bcs_[solc.first]);
            pde->UpdateMatrices(Teuchos::null, Teuchos::null);
            pde->ApplyBCs(bcflag, false, false);

            double factor = eqns_[row].diff_factors[phase];
            if (factor != 1.0) pde->local_op()->Rescale(factor);
          }
        }

        // -- advection operator div[ (f grad dg/dv) dv ]
        if ((key = eqns_[row].diffusion[phase].first) != "") {
          Key fname = eqns_[row].diffusion[phase].second;
          Key der_key = "d" + fname + "_d" + keyc;
          S_->GetFieldEvaluator(fname)->HasFieldDerivativeChanged(S_.ptr(), passwd_, keyc);

          if (S_->HasField(der_key)) {
            // --- calculate diffusion coefficient
            S_->GetFieldEvaluator(key)->HasFieldChanged(S_.ptr(), passwd_);
            kr_c = *S_->GetFieldData(key)->ViewComponent("cell");
            kr->ViewComponent("dirichlet_faces")->PutScalar(0.0);  // FIXME
            auto flux = *S_->GetFieldData(flux_names_[phase], passwd_);
            upwind_->Compute(flux, *kr, op_bcs_[solr.first]->bc_model(), *kr);

            // --- calculate advective flux 
            auto der = S_->GetFieldData(der_key);
            pde_diff_D_->Setup(Teuchos::null, kr, Teuchos::null);
            pde_diff_D_->SetBCs(op_bcs_[solr.first], op_bcs_[solc.first]);
            pde_diff_D_->UpdateMatrices(Teuchos::null, Teuchos::null);
            pde_diff_D_->UpdateFlux(der.ptr(), flux_tmp.ptr());

            double factor = eqns_[row].diff_factors[phase];
            flux_acc->Update(factor, *flux_tmp, 1.0);
          }
        }

        // -- advection operator div [ (df/dv grad g) dv ]
        if ((key = eqns_[row].diffusion[phase].first) != "" && keyc == saturation_liquid_key_) {
          Key der_key = "d" + key + "_d" + keyc;
          S_->GetFieldEvaluator(key)->HasFieldDerivativeChanged(S_.ptr(), passwd_, keyc);

          if (S_->HasField(der_key)) {
            // --- upwind derivative
            kr_c = *S_->GetFieldData(der_key)->ViewComponent("cell");
            kr->ViewComponent("dirichlet_faces")->PutScalar(0.0);  // FIXME
            auto flux = *S_->GetFieldData(flux_names_[phase], passwd_);
            upwind_->Compute(flux, *kr, op_bcs_[solr.first]->bc_model(), *kr);

            // --- calculate advective flux 
            Key fname = eqns_[row].diffusion[phase].second;
            S_->GetFieldEvaluator(fname)->HasFieldChanged(S_.ptr(), passwd_);
            auto var = S_->GetFieldData(fname);
            pde_diff_D_->Setup(Teuchos::null, kr, Teuchos::null);
            pde_diff_D_->SetBCs(op_bcs_[solr.first], op_bcs_[solc.first]);
            pde_diff_D_->UpdateMatrices(Teuchos::null, Teuchos::null);
            pde_diff_D_->UpdateFlux(var.ptr(), flux_tmp.ptr());

            double factor = eqns_[row].diff_factors[phase];
            flux_acc->Update(factor, *flux_tmp, 1.0);
          }
        }
      }

      // populate advection operator
      auto pde1 = Teuchos::rcp(new Operators::PDE_AdvectionUpwind(adv_list, global_op)); 
      pde1->Setup(*flux_acc);
      pde1->SetBCs(op_bcs_[solr.first], op_bcs_[solc.first]);
      pde1->UpdateMatrices(flux_acc.ptr());
      pde1->ApplyBCs(false, false, false);

      // storage term
      if ((key = eqns_[row].storage) != "") {
        Key der_key = "d" + key + "_d" + keyc;
        S_->GetFieldEvaluator(key)->HasFieldDerivativeChanged(S_.ptr(), passwd_, keyc);

        if (S_->HasField(der_key)) {
          auto pde = Teuchos::rcp(new Operators::PDE_Accumulation(AmanziMesh::CELL, global_op)); 
          auto der = S_->GetFieldData(der_key, key);
          pde->AddAccumulationTerm(*der, dtp, "cell");
        }
      }
    }
  }
      
  // process constraint
  int n = num_primary_ + 1;

  for (int i = 0; i < num_primary_ + 2; ++i) {
    auto solc = EquationToSolution(i);
    auto pde = Teuchos::rcp(new Operators::PDE_Accumulation(AmanziMesh::CELL, mesh_)); 
    op_preconditioner_->SetOperatorBlock(n, i, pde->global_operator());
 
    Teuchos::RCP<const Epetra_MultiVector> der_fc, der_gc;

    Key key = eqns_[n].constraint.first;
    S_->GetFieldEvaluator(key)->HasFieldChanged(S_.ptr(), passwd_);
    const auto& ncp_fc = *S_->GetFieldData(key)->ViewComponent("cell");

    Key keyc = soln_names_[solc.first];
    Key derf_key = "d" + key + "_d" + keyc;
    S_->GetFieldEvaluator(key)->HasFieldDerivativeChanged(S_.ptr(), passwd_, keyc);
    if (S_->HasField(derf_key)) der_fc = S_->GetFieldData(derf_key)->ViewComponent("cell");

    key = eqns_[n].constraint.second;
    S_->GetFieldEvaluator(key)->HasFieldChanged(S_.ptr(), passwd_);
    const auto& ncp_gc = *S_->GetFieldData(key)->ViewComponent("cell");

    keyc = soln_names_[solc.first];
    Key derg_key = "d" + key + "_d" + keyc;
    S_->GetFieldEvaluator(key)->HasFieldDerivativeChanged(S_.ptr(), passwd_, keyc);
    if (S_->HasField(derg_key)) der_gc = S_->GetFieldData(derg_key)->ViewComponent("cell");

    // -- identify active set for gas phase
    fone.PutScalar(0.0);
    if (ncp_ == "min") {
      for (int c = 0; c < ncells_owned_; c++) {
        if (ncp_fc[0][c] > ncp_gc[0][c]) {
          if (der_gc.get()) fone_c[0][c] = (*der_gc)[0][c];
        } else {
          if (der_fc.get()) fone_c[0][c] = (*der_fc)[0][c];
        }
      }
    } else if (ncp_ == "Fischer-Burmeister") {
      for (int c = 0; c < ncells_owned_; ++c) {
        double a = ncp_fc[0][c];
        double b = ncp_gc[0][c];

        double da = (der_fc.get()) ? (*der_fc)[0][c] : 0.0;
        double db = (der_gc.get()) ? (*der_gc)[0][c] : 0.0;
        fone_c[0][c] = (a * da + b * db) * std::pow(a * a + b * b, -0.5) - (da + db);
      }
    }

    pde->AddAccumulationTerm(fone, "cell");
  }


  // finalize preconditioner
  if (!op_pc_assembled_) {
    op_preconditioner_->SymbolicAssembleMatrix();
    op_pc_assembled_ = true;
  }
  op_preconditioner_->AssembleMatrix();
  // std::cout << *op_preconditioner_->A() << std::endl; exit(0);
  op_preconditioner_->UpdatePreconditioner();
}


/* ******************************************************************
* Apply preconditioner inv(B) * X.                                                 
****************************************************************** */
int Multiphase_PK::ApplyPreconditioner(Teuchos::RCP<const TreeVector> X, 
                                       Teuchos::RCP<TreeVector> Y)
{
  Y->PutScalar(0.0);
  // *Y = *X; return 0;
  int ierr = op_pc_solver_->ApplyInverse(*X, *Y);
  return ierr;
}


/* ******************************************************************
* This is called when the time integration scheme changes solution
****************************************************************** */
void Multiphase_PK::ChangedSolution()
{
  for (int i = 0; i < 3; ++i ) {
    auto eval = S_->GetFieldEvaluator(soln_names_[i]);
    Teuchos::rcp_dynamic_cast<PrimaryVariableFieldEvaluator>(eval)->SetFieldAsChanged(S_.ptr());
  }
}


/* ******************************************************************
* Monitor l2 norm of residual
****************************************************************** */
double Multiphase_PK::ErrorNorm(Teuchos::RCP<const TreeVector> u,
                                Teuchos::RCP<const TreeVector> du) 
{
  // pressure error
  auto pc = *u->SubVector(0)->Data()->ViewComponent("cell");
  auto dpc = *du->SubVector(0)->Data()->ViewComponent("cell");

  double atm_pressure(1.0e+5);

  double error_p = 0.0;
  for (int c = 0; c < ncells_owned_; c++) {
    double tmp = fabs(dpc[0][c]) / (fabs(pc[0][c] - atm_pressure) + atm_pressure);
    if (tmp > error_p) {
      error_p = tmp;
    } 
  }

  // saturation error
  auto dsc = *du->SubVector(1)->Data()->ViewComponent("cell");

  double error_s = 0.0;
  for (int c = 0; c < ncells_owned_; c++) {
    error_s = std::max(error_s, fabs(dsc[0][c]));
  }

  // concentration error
  auto dxc = *du->SubVector(2)->Data()->ViewComponent("cell");

  double error_x = 0.0;
  for (int c = 0; c < ncells_owned_; c++) {
    error_x = std::max(error_x, fabs(dxc[0][c]));
  }

  return error_p + error_s + error_x;
}

}  // namespace Multiphase
}  // namespace Amanzi

