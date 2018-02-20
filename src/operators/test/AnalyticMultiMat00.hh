/*
  Operators

  Copyright 2010-201x held jointly by LANS/LANL, LBNL, and PNNL. 
  Amanzi is released under the three-clause BSD License. 
  The terms of use and "as is" disclaimer for this license are 
  provided in the top-level COPYRIGHT file.

  Author: Konstantin Lipnikov (lipnikov@lanl.gov)

  Linear solution plus constant coefficient.

  u = g_x * x + g_y * y

  for user-provided gradient g = {g_x, g_y}
  
*/

#ifndef AMANZI_OPERATOR_ANALYTICMULTIMAT_00_HH_
#define AMANZI_OPERATOR_ANALYTICMULTIMAT_00_HH_

#include "AnalyticBase.hh"

class AnalyticMultiMat00 : public AnalyticBase {
 public:
  AnalyticMultiMat00(Teuchos::RCP<const Amanzi::AmanziMesh::Mesh> mesh, double gx, double gy) :
      AnalyticBase(mesh),
      gx_(gx),
      gy_(gy) {};
  ~AnalyticMultiMat00() {};

  Amanzi::WhetStone::Tensor Tensor(const Amanzi::AmanziGeometry::Point& p, double t) {
    Amanzi::WhetStone::Tensor K(2, 1);
    K(0, 0) = 1.0;
    return K;
  }

  Amanzi::WhetStone::Tensor Tensor(const Amanzi::AmanziGeometry::Point& p, double t, int mat_id) {
    Amanzi::WhetStone::Tensor K(2, 1);
    if (mat_id == 0){
      K(0, 0) = 1.0;
    }else if (mat_id == 1){
      K(0, 0) = 0.01;
    }
    return K;
  }

  double pressure_exact(const Amanzi::AmanziGeometry::Point& p, double t) { 
    double x = p[0];
    double y = p[1];

    if (x<=y) {
      return 100. * (gx_ * x + gy_ * y);
    }else{
      return gx_ * x + gy_ * y;
    }

  }

  Amanzi::AmanziGeometry::Point velocity_exact(const Amanzi::AmanziGeometry::Point& p, double t) { 
    Amanzi::AmanziGeometry::Point v(2);
    v[0] = -gx_;
    v[1] = -gy_;
    return v;
  }
 
  Amanzi::AmanziGeometry::Point gradient_exact(const Amanzi::AmanziGeometry::Point& p, double t) { 
    Amanzi::AmanziGeometry::Point v(2);
    double x = p[0];
    double y = p[1];

    if (x<=y) {
      v[0] = gx_;
      v[1] = gy_;
      v *= 100.;
    }else{
      v[0] = gx_;
      v[1] = gy_;
    }
    return v;
  }

  double source_exact(const Amanzi::AmanziGeometry::Point& p, double t) { 
    return 0.0;
  }

 private:
  double gx_, gy_;
};



#endif

