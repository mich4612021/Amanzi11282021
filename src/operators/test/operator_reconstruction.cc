/*
  Operators

  Copyright 2010-201x held jointly by LANS/LANL, LBNL, and PNNL. 
  Amanzi is released under the three-clause BSD License. 
  The terms of use and "as is" disclaimer for this license are 
  provided in the top-level COPYRIGHT file.

  Author: Konstantin Lipnikov (lipnikov@lanl.gov)
*/

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <vector>

// TPLs
#include "Epetra_MultiVector.h"
#include "Teuchos_RCP.hpp"
#include "Teuchos_ParameterList.hpp"
#include "Teuchos_ParameterXMLFileReader.hpp"
#include "UnitTest++.h"

// Amanzi
#include "GMVMesh.hh"
#include "MeshFactory.hh"

// Amanzi::Operators
#include "ErrorAnalysis.hh"
#include "OperatorDefs.hh"
#include "ReconstructionCell.hh"


/* *****************************************************************
* Exactness on linear functions in two dimensions
***************************************************************** */
void RunLinear(int icase) {
  using namespace Amanzi;
  using namespace Amanzi::AmanziMesh;
  using namespace Amanzi::AmanziGeometry;
  using namespace Amanzi::Operators;

  auto comm = Amanzi::getDefaultComm();
  int MyPID = comm->MyPID();

  if (MyPID == 0) std::cout << "\nTest: Exactness on linear functions in 2D, weight=" << icase << std::endl;

  // create rectangular mesh
  MeshFactory meshfactory(comm);
  meshfactory.set_preference(Preference({Framework::MSTK, Framework::STK}));

  Teuchos::RCP<const Mesh> mesh = meshfactory.create(0.0, 0.0, 1.0, 1.0, 7, 7);

  // create and initialize cell-based field 
  Teuchos::RCP<Epetra_MultiVector> field = Teuchos::rcp(new Epetra_MultiVector(mesh->cell_map(true), 1));
  Epetra_MultiVector grad_exact(mesh->cell_map(false), 2);

  // create optional picewise linear weight function
  auto weight = Teuchos::rcp(new Epetra_MultiVector(mesh->cell_map(true), 1));
  CompositeVectorSpace cvs;
  cvs.SetMesh(mesh)->SetGhosted(true)->SetComponent("cell", AmanziMesh::CELL, 2);
  auto weight_grad = Teuchos::rcp(new CompositeVector(cvs, true));
  auto& weight_grad_c = *weight_grad->ViewComponent("cell", true);

  int ncells_owned = mesh->num_entities(AmanziMesh::CELL, AmanziMesh::Parallel_type::OWNED);
  int ncells_wghost = mesh->num_entities(AmanziMesh::CELL, AmanziMesh::Parallel_type::ALL);

  for (int c = 0; c < ncells_wghost; c++) {
    const AmanziGeometry::Point& xc = mesh->cell_centroid(c);
    (*field)[0][c] = xc[0] + 2 * xc[1];
    if (c < ncells_owned) {
      grad_exact[0][c] = 1.0;
      grad_exact[1][c] = 2.0;
    }

    (*weight)[0][c] = 1.0;
    if (icase == 2) {
      (*weight)[0][c] = 1.0 + xc[0];
      weight_grad_c[0][c] = 1.0;
      weight_grad_c[1][c] = 2.0;

      (*field)[0][c] = xc[0] + 2 * xc[1] + 5.0 / 588.0 / (1.0 + xc[0]);
    }
  }

  // Compute reconstruction
  Teuchos::ParameterList plist;
  plist.set<std::string>("limiter", "tensorial");
  plist.set<int>("polynomial_order", 1);
  plist.set<bool>("limiter extension for transport", false);

  ReconstructionCell* lifting;
  if (icase == 0) lifting = new ReconstructionCell(mesh);
  else lifting = new ReconstructionCell(mesh, weight, weight_grad);

  lifting->Init(plist);
  lifting->ComputeGradient(field); 

  // calculate gradient error
  double err_int, err_glb, gnorm;
  Epetra_MultiVector& grad_computed = *lifting->gradient()->ViewComponent("cell");

  ComputeGradError(mesh, grad_computed, grad_exact, err_int, err_glb, gnorm);
  CHECK_CLOSE(0.0, err_int + err_glb, 1.0e-12);

  if (MyPID == 0) printf("errors (interior & global): %8.4f %8.4f\n", err_int, err_glb);

  delete lifting;
}


TEST(RECONSTRUCTION_LINEAR_2D) {
  RunLinear(0);
}

TEST(RECONSTRUCTION_LINEAR_2D_WEIGHT) {
  RunLinear(1);
  RunLinear(2);
}


/* *****************************************************************
* Exactness on linear functions in three dimensions
***************************************************************** */
TEST(RECONSTRUCTION_LINEAR_3D) {
  using namespace Amanzi;
  using namespace Amanzi::AmanziMesh;
  using namespace Amanzi::AmanziGeometry;
  using namespace Amanzi::Operators;

  auto comm = Amanzi::getDefaultComm();
  int MyPID = comm->MyPID();

  if (MyPID == 0) std::cout << "\nTest: Exactness on linear functions in 3D." << std::endl;

  // create rectangular mesh
  MeshFactory meshfactory(comm);
  meshfactory.set_preference(Preference({Framework::MSTK, Framework::STK}));

  Teuchos::RCP<const Mesh> mesh = meshfactory.create(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 7, 6, 5);

  // create and initialize cell-based field 
  Teuchos::RCP<Epetra_MultiVector> field = Teuchos::rcp(new Epetra_MultiVector(mesh->cell_map(true), 1));
  Epetra_MultiVector grad_exact(mesh->cell_map(false), 3);

  int ncells_owned = mesh->num_entities(AmanziMesh::CELL, AmanziMesh::Parallel_type::OWNED);
  int ncells_wghost = mesh->num_entities(AmanziMesh::CELL, AmanziMesh::Parallel_type::ALL);

  for (int c = 0; c < ncells_wghost; c++) {
    const AmanziGeometry::Point& xc = mesh->cell_centroid(c);
    (*field)[0][c] = xc[0] + 2 * xc[1] + 3 * xc[2];
    if (c < ncells_owned) {
      grad_exact[0][c] = 1.0;
      grad_exact[1][c] = 2.0;
      grad_exact[2][c] = 3.0;
    }
  }

  // Compute reconstruction
  Teuchos::ParameterList plist;
  plist.set<std::string>("limiter", "tensorial");
  plist.set<int>("polynomial_order", 1);
  plist.set<bool>("limiter extension for transport", false);

  ReconstructionCell lifting(mesh);
  lifting.Init(plist);
  lifting.ComputeGradient(field);

  // calculate gradient error
  double err_int, err_glb, gnorm;
  Epetra_MultiVector& grad_computed = *lifting.gradient()->ViewComponent("cell");

  ComputeGradError(mesh, grad_computed, grad_exact, err_int, err_glb, gnorm);
  CHECK_CLOSE(0.0, err_int + err_glb, 1.0e-12);

  if (MyPID == 0) printf("errors (interior & global): %8.4f %8.4f\n", err_int, err_glb);
}


