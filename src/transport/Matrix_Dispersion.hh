/*
This is the transport component of the Amanzi code. 

Copyright 2010-2013 held jointly by LANS/LANL, LBNL, and PNNL. 
Amanzi is released under the three-clause BSD License. 
The terms of use and "as is" disclaimer for this license are 
provided in the top-level COPYRIGHT file.

Author: Konstantin Lipnikov (lipnikov@lanl.gov)
Usage: 
*/

#ifndef __MATRIX_DISPERSION_HH__
#define __MATRIX_DISPERSION_HH__


#include "Epetra_FECrsMatrix.h"
#include "Epetra_Vector.h"


namespace Amanzi {
namespace AmanziTransport {

class Dispersion_Specs {
 public:
  Dispersion_Specs() {
    method = TRANSPORT_DISPERSIVITY_MODEL_NULL;
    dispersivity_longitudinal = 0.0;
    dispersivity_transverse = 0.0;
  }
  ~Dispersion_Specs() {};

 public:
  int method;
  double dispersivity_longitudinal, dispersivity_transverse;
};


class Matrix_Dispersion {
 public:
  Matrix_Dispersion() {};
  Matrix_Dispersion(Teuchos::RCP<const AmanziMesh::Mesh> mesh) : mesh_(mesh) {};
  ~Matrix_Dispersion() {};

  // primary members
  void Init(Dispersion_Specs& specs);
  void Apply(const Epetra_Vector& v,  Epetra_Vector& av) const;
  void ApplyInverse(const Epetra_Vector& v,  Epetra_Vector& hv) const;

  void CalculateDispersionTensor(const Epetra_Vector& darcy_flux);
  void SymbolicAssembleGlobalMatrix();
  void AssembleGlobalMatrix();
  void AddTimeDerivative(double dT, const Epetra_Vector& phi, const Epetra_Vector& ws);

 private:
  void PopulateHarmonicPoints();
  void ExtractBoundaryConditions(const int component,
                                 std::vector<int>& bc_face_id,
                                 std::vector<double>& bc_face_value);

 private:
  Teuchos::RCP<const AmanziMesh::Mesh> mesh_;
  int dim;

  int ncells_owned, ncells_wghost;
  int nfaces_owned, nfaces_wghost;

  Dispersion_Specs* specs_;

  std::vector<AmanziGeometry::Point> hap_points_;
  std::vector<double> hap_weights_;

  std::vector<WhetStone::Tensor> D;

  Teuchos::RCP<Epetra_FECrsMatrix> Dpp_;
};

}  // namespace AmanziTransport
}  // namespace Amanzi

#endif

