/*
  Operators

  Copyright 2010-201x held jointly by LANS/LANL, LBNL, and PNNL. 
  Amanzi is released under the three-clause BSD License. 
  The terms of use and "as is" disclaimer for this license are 
  provided in the top-level COPYRIGHT file.

  Author: Konstantin Lipnikov (lipnikov@lanl.gov)

  Base class for testing DG schemes.
  List of problems:

  AnalyticDG0n: polynomial solution of order n, where n=0,1,2, and 3
  AnalyticDG04: sin(3x) six(6y)
*/

#ifndef AMANZI_OPERATOR_ANALYTIC_DG_BASE_HH_
#define AMANZI_OPERATOR_ANALYTIC_DG_BASE_HH_

#include "Teuchos_RCP.hpp"

#include "Mesh.hh"
#include "Point.hh"
#include "Polynomial.hh"
#include "Tensor.hh"

class AnalyticDGBase {
 public:
  AnalyticDGBase(Teuchos::RCP<const Amanzi::AmanziMesh::Mesh> mesh, int order)
    : mesh_(mesh),
      order_(order),
      d_(mesh_->space_dimension()) {};
  ~AnalyticDGBase() {};

  // diffusion tensor
  virtual Amanzi::WhetStone::Tensor Tensor(const Amanzi::AmanziGeometry::Point& p, double t) = 0;

  // analytic solution in conventional Taylor basis
  virtual void TaylorCoefficients(const Amanzi::AmanziGeometry::Point& p, double t,
                                  Amanzi::WhetStone::Polynomial& coefs) = 0;

  // approximate source term
  virtual void SourceTaylor(const Amanzi::AmanziGeometry::Point& p, double t,
                            Amanzi::WhetStone::Polynomial& src) = 0;

  // exact solution value
  double SolutionExact(const Amanzi::AmanziGeometry::Point& p, double t) {
    Amanzi::WhetStone::Polynomial coefs;
    TaylorCoefficients(p, t, coefs);
    return coefs(0, 0);
  }

  // calculate error
  void ComputeCellError(Epetra_MultiVector& p, double t, double& pnorm, double& l2_err, double& inf_err) {
    pnorm = 0.0;
    l2_err = 0.0;
    inf_err = 0.0;

    int ncells = mesh_->num_entities(Amanzi::AmanziMesh::CELL, Amanzi::AmanziMesh::OWNED);
    for (int c = 0; c < ncells; c++) {
      const Amanzi::AmanziGeometry::Point& xc = mesh_->cell_centroid(c);
      double tmp = SolutionExact(xc, t);
      double volume = mesh_->cell_volume(c);

      // std::cout << c << " " << tmp << " " << p[0][c] << std::endl;
      l2_err += std::pow(tmp - p[0][c], 2.0) * volume;
      inf_err = std::max(inf_err, fabs(tmp - p[0][c]));
      pnorm += std::pow(tmp, 2.0) * volume;
    }
#ifdef HAVE_MPI
    double tmp = pnorm;
    mesh_->get_comm()->SumAll(&tmp, &pnorm, 1);
    tmp = l2_err;
    mesh_->get_comm()->SumAll(&tmp, &l2_err, 1);
    tmp = inf_err;
    mesh_->get_comm()->MaxAll(&tmp, &inf_err, 1);
#endif
    pnorm = sqrt(pnorm);
    l2_err = sqrt(l2_err);
  }

 protected:
  Teuchos::RCP<const Amanzi::AmanziMesh::Mesh> mesh_;
  int order_, d_;
};

#endif

