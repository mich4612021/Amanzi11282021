/*
  WhetStone, version 2.0
  Release name: naka-to.

  Copyright 2010-201x held jointly by LANS/LANL, LBNL, and PNNL. 
  Amanzi is released under the three-clause BSD License. 
  The terms of use and "as is" disclaimer for this license are 
  provided in the top-level COPYRIGHT file.

  Author: Konstantin Lipnikov (lipnikov@lanl.gov)

  Miscaleneous discretization methods.
*/

#include <cmath>
#include <vector>

#include "Mesh.hh"
#include "Point.hh"
#include "errors.hh"

#include "mfd3d_diffusion.hh"
#include "Tensor.hh"


namespace Amanzi {
namespace WhetStone {

/* ******************************************************************
* The conventional FV scheme for a general mesh.
****************************************************************** */
int MFD3D_Diffusion::MassMatrixInverseTPFA(int c, const Tensor& K, DenseMatrix& W)
{
  int d = mesh_->space_dimension();

  Entity_ID_List faces;
  std::vector<int> dirs;
  mesh_->cell_get_faces_and_dirs(c, &faces, &dirs);
  int nfaces = faces.size();

  const AmanziGeometry::Point& xc = mesh_->cell_centroid(c);
  AmanziGeometry::Point a(d);

  W.PutScalar(0.0);
  for (int n = 0; n < nfaces; n++) {
    int f = faces[n];
    const AmanziGeometry::Point& xf = mesh_->face_centroid(f);
    const AmanziGeometry::Point& normal = mesh_->face_normal(f);

    a = xf - xc;
    double s = mesh_->face_area(f) * dirs[n] / norm(a);
    double Knn = ((K * a) * normal) * s;
    double dxn = a * normal;
    W(n, n) = Knn / fabs(dxn);
  }
  return WHETSTONE_ELEMENTAL_MATRIX_OK;
}


/* ******************************************************************
* The one-sided transmissibility coefficient. Any change to this 
* routine must be consistent with the above routine.
****************************************************************** */
double MFD3D_Diffusion::Transmissibility(int f, int c, const Tensor& K)
{
  int dir, d = mesh_->space_dimension();

  const AmanziGeometry::Point& xc = mesh_->cell_centroid(c);
  AmanziGeometry::Point a(d);

  const AmanziGeometry::Point& xf = mesh_->face_centroid(f);
  const AmanziGeometry::Point& normal = mesh_->face_normal(f, false, c, &dir);

  a = xf - xc;
  double s = mesh_->face_area(f) * dir / norm(a);
  double Knn = ((K * a) * normal) * s;
  double dxn = a * normal;
  double W = Knn / fabs(dxn);

  return W;
}


/* ******************************************************************
* The debug version of the above FV scheme for a scalar tensor and
* an orthogonal brick element.
****************************************************************** */
int MFD3D_Diffusion::MassMatrixInverseDiagonal(int c, const Tensor& K, DenseMatrix& W)
{
  int d = mesh_->space_dimension();
  double volume = mesh_->cell_volume(c);

  Entity_ID_List faces;
  mesh_->cell_get_faces(c, &faces);
  int nfaces = faces.size();

  W.PutScalar(0.0);
  for (int n = 0; n < nfaces; n++) {
    int f = faces[n];
    double area = mesh_->face_area(f);
    W(n, n) = nfaces * K(0, 0) * area * area / (d * volume);
  }
  return WHETSTONE_ELEMENTAL_MATRIX_OK;
}


/* ******************************************************************
* Second-generation MFD method as inlemented in RC1.
****************************************************************** */
int MFD3D_Diffusion::MassMatrixInverseSO(int c, const Tensor& K, DenseMatrix& W)
{
  int d = mesh_->space_dimension();

  Entity_ID_List faces;
  std::vector<int> fdirs;
  mesh_->cell_get_faces_and_dirs(c, &faces, &fdirs);

  Entity_ID_List nodes, corner_faces;
  mesh_->cell_get_nodes(c, &nodes);
  int nnodes = nodes.size();

  Tensor Kinv(K);
  Kinv.Inverse();

  // collect all corner matrices
  std::vector<Tensor> Mv;
  std::vector<double> cwgt;

  Tensor N(d, 2), NK(d, 2), Mv_tmp(d, 2);

  for (int n = 0; n < nnodes; n++) {
    int v = nodes[n];
    mesh_->node_get_cell_faces(v, c, (ParallelTypeCast)WhetStone::USED, &corner_faces);
    int nfaces = corner_faces.size();
    if (nfaces < d) {
      Errors::Message msg;
      msg << "WhetStone MFD3D_Diffusion: number of faces forming a corner is small.";
      Exceptions::amanzi_throw(msg);
    }

    for (int i = 0; i < d; i++) {
      int f = corner_faces[i];
      N.SetColumn(i, mesh_->face_normal(f));
    }
    double cwgt_tmp = fabs(N.Det());

    N.Inverse();
    NK = N * Kinv;

    N.Transpose();
    Mv_tmp = NK * N;
    Mv.push_back(Mv_tmp);

    for (int i = 0; i < d; i++) {
      int f = corner_faces[i];
      cwgt_tmp /= mesh_->face_area(f);
    }
    cwgt.push_back(cwgt_tmp);
  }

  // rescale corner weights
  double factor = 0.0;
  for (int n = 0; n < nnodes; n++) factor += cwgt[n];
  factor = mesh_->cell_volume(c) / factor;

  for (int n = 0; n < nnodes; n++) cwgt[n] *= factor;

  // assemble corner matrices
  W.PutScalar(0.0);
  for (int n = 0; n < nnodes; n++) {
    int v = nodes[n];
    mesh_->node_get_cell_faces(v, c, (ParallelTypeCast)WhetStone::USED, &corner_faces);

    Tensor& Mv_tmp = Mv[n];
    for (int i = 0; i < d; i++) {
      int k = FindPosition_(corner_faces[i], faces);
      for (int j = i; j < d; j++) {
        int l = FindPosition_(corner_faces[j], faces);
        W(k, l) += Mv_tmp(i, j) * cwgt[n] * fdirs[k] * fdirs[l];
        W(l, k) = W(k, l);
      }
    }
  }
 
  // invert matrix W
  int ierr = W.Inverse();
  if (ierr != 0) {
    Errors::Message msg;
    msg << "WhetStone MFD3D_Diffusion: support operator generated bad elemental mass matrix.";
    Exceptions::amanzi_throw(msg);
  }

  return WHETSTONE_ELEMENTAL_MATRIX_OK;
}

}  // namespace WhetStone
}  // namespace Amanzi



