/*
  WhetStone, version 2.1
  Release name: naka-to.

  Copyright 2010-201x held jointly by LANS/LANL, LBNL, and PNNL. 
  Amanzi is released under the three-clause BSD License. 
  The terms of use and "as is" disclaimer for this license are 
  provided in the top-level COPYRIGHT file.

  Author: Konstantin Lipnikov (lipnikov@lanl.gov)

  The mimetic finite difference method.

  The package uses the formula M = Mc + Ms, where matrix Mc is build from a 
  consistency condition (Mc N = R) and matrix Ms is build from a stability 
  condition (Ms N = 0), to generate mass and stiffness matrices for a variety 
  of physics packages: flow, transport, thermal, and geomechanics. 
  The material properties are imbedded into the the matrix Mc. 

  Notation used below: M (mass), W (inverse of M), A (stiffness).

  NOTE: This class should be never instantiated directly. It is used to
  add additional functionality to the base class (BilinearForm) related to
  MFD and VEM methods, such as various projectors.
*/

#ifndef AMANZI_MFD3D_HH_
#define AMANZI_MFD3D_HH_

#include "Teuchos_RCP.hpp"

#include "Mesh.hh"
#include "Point.hh"

#include "BilinearForm.hh"
#include "DenseMatrix.hh"
#include "InnerProductH1.hh"
#include "InnerProductL2.hh"
#include "Projectors.hh"
#include "Tensor.hh"
#include "WhetStoneDefs.hh"

namespace Amanzi {
namespace WhetStone {

class MFD3D : public virtual BilinearForm,
              public Projectors {
 public:
  explicit MFD3D(const Teuchos::RCP<const AmanziMesh::Mesh>& mesh);
  ~MFD3D() {};

  // access members
  double simplex_functional() { return simplex_functional_; }
  int simplex_num_itrs() { return simplex_num_itrs_; }

  // experimental methods (for stability region analysis; unit test)
  void ModifyStabilityScalingFactor(double factor);

 protected:
  // supporting stability methods (add matrix M += Mstab)
  // use R, Wc, W for the inverse matrix
  int StabilityMonotoneHex(int c, const Tensor& T, DenseMatrix& Mc, DenseMatrix& M);

  int StabilityMMatrix_(int c, DenseMatrix& N, DenseMatrix& M, 
                        int objective = WHETSTONE_SIMPLEX_FUNCTIONAL_SUMALL);

  int SimplexFindFeasibleSolution_(DenseMatrix& T, int m1, int m2, int m3, int* izrow, int* iypos);
  void SimplexPivotElement_(DenseMatrix& T, int kp, int* ip);
  void SimplexExchangeVariables_(DenseMatrix& T, int kp, int ip);

 protected:
  double simplex_functional_;
  int simplex_num_itrs_;
};


// non-member functions
inline int FindPosition(int id, Entity_ID_List list) {
  for (int i = 0; i < list.size(); i++) {
    if (list[i] == id) return i;
  }
  return -1;
}

}  // namespace WhetStone
}  // namespace Amanzi

#endif

