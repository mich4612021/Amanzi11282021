/*
This is the flow component of the Amanzi code. 
License: BSD
Authors: Konstantin Lipnikov (lipnikov@lanl.gov)
*/

#ifndef __Flow_State_hpp__
#define __Flow_State_hpp__

#include "Epetra_Vector.h"
#include "Epetra_CombineMode.h"
#include "Teuchos_RCP.hpp"

#include "Mesh.hh"
#include "State.hpp"

namespace Amanzi {
namespace AmanziFlow {

enum FlowCreateMode {
CopyPointers,  // copy Teuchos::RCP pointers 
ViewMemory,    // convert to overlap to non-overlap vectors  
CopyMemory     // copy non-overlap vector to overlap vectors 
};


class Flow_State {
 public:
  Flow_State(Teuchos::RCP<State> S);
  Flow_State(State& S);
  Flow_State(Flow_State& S, FlowCreateMode mode = CopyPointers);
  ~Flow_State() {};

  // data management
  void copyMasterCell2GhostCell(Epetra_Vector& v);
  void copyMasterMultiCell2GhostMultiCell(Epetra_MultiVector& v);
  void combineGhostFace2MasterFace(Epetra_Vector& v, Epetra_CombineMode mode = Insert);

  Epetra_Vector* createCellView(const Epetra_Vector& u) const;
  Epetra_Vector* createFaceView(const Epetra_Vector& u) const;

  // access methods
  Teuchos::RCP<Epetra_Vector> get_porosity() { return porosity; }
  Teuchos::RCP<double> get_fluid_density() { return fluid_density; }
  Teuchos::RCP<double> get_fluid_viscosity() { return fluid_viscosity; }
  Teuchos::RCP<Epetra_Vector> get_vertical_permeability() { return vertical_permeability; }
  Teuchos::RCP<Epetra_Vector> get_horizontal_permeability() { return horizontal_permeability; }
  Teuchos::RCP<double*> get_gravity() { return gravity; }
  Teuchos::RCP<Epetra_Vector> get_pressure() { return pressure; }
  Teuchos::RCP<Epetra_Vector> get_darcy_mass_flux() { return darcy_mass_flux; }
  Teuchos::RCP<Epetra_Vector> get_water_saturation() { return water_saturation; }
  Teuchos::RCP<AmanziMesh::Mesh> get_mesh() { return mesh_; }

  Epetra_Vector& ref_porosity() { return *porosity; }
  Epetra_Vector& ref_pressure() { return *pressure; }
  Epetra_Vector& ref_darcy_mass_flux() { return *darcy_mass_flux; }
  Epetra_Vector& ref_vertical_permeability() { return *vertical_permeability; }
  Epetra_Vector& ref_horizontal_permeability() { return *horizontal_permeability; }
  double ref_fluid_density() { return *fluid_density; }
  double ref_fluid_viscosity() { return *fluid_viscosity; }

  // miscaleneous
  double get_time() { return S_->get_time(); }
  double normL2cell(Epetra_Vector& v1, Epetra_Vector& v2);
  double normL2cell(Epetra_Vector& v1);
  
  // debug routines
  void set_fluid_density(double rho);
  void set_fluid_viscosity(double mu);
  void set_pressure_head(double z0, double p0, Epetra_Vector& pressure);

 private:
  State* S_;  

  Teuchos::RCP<double> fluid_density;
  Teuchos::RCP<double> fluid_viscosity;
  Teuchos::RCP<double*> gravity;
  Teuchos::RCP<Epetra_Vector> vertical_permeability;
  Teuchos::RCP<Epetra_Vector> horizontal_permeability;
  Teuchos::RCP<Epetra_Vector> pressure;
  Teuchos::RCP<Epetra_Vector> porosity;
  Teuchos::RCP<Epetra_Vector> darcy_mass_flux;
  Teuchos::RCP<Epetra_Vector> water_saturation;

  Teuchos::RCP<AmanziMesh::Mesh> mesh_;
};

}  // namespace AmanziFlow
}  // namespace Amanzi

#endif
