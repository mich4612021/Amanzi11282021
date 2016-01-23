/*
  Chemistry PK

  Copyright 2010-201x held jointly by LANS/LANL, LBNL, and PNNL. 
  Amanzi is released under the three-clause BSD License. 
  The terms of use and "as is" disclaimer for this license are 
  provided in the top-level COPYRIGHT file.
*/
 
#ifndef CHEMISTRY_AMANZI_PK_HH_
#define CHEMISTRY_AMANZI_PK_HH_

#include <string>
#include <vector>

// TPLs
#include "Teuchos_RCP.hpp"
#include "Teuchos_ParameterList.hpp"

// Amanzi
#include "beaker.hh"
#include "chemistry_exception.hh"
#include "chemistry_verbosity.hh"
#include "Chemistry_PK.hh"
#include "Chemistry_State.hh"
#include "Mesh.hh"

// forward declarations
class Epetra_MultiVector;
class Epetra_Vector;
class Epetra_SerialDenseVector;

namespace Amanzi {
namespace AmanziChemistry {

// Trilinos based chemistry process kernel for the unstructured mesh
class Amanzi_PK : public Chemistry_PK {
 public:
  Amanzi_PK(const Teuchos::ParameterList& param_list,
            Teuchos::RCP<Chemistry_State> chem_state,
            Teuchos::RCP<State> S,
            Teuchos::RCP<const AmanziMesh::Mesh> mesh);

  ~Amanzi_PK();

  void InitializeChemistry();

  void Advance(const double& delta_time,
               Teuchos::RCP<Epetra_MultiVector> total_component_concentration);
  void CommitState(Teuchos::RCP<Chemistry_State> chem_state, const double& time);

  // modifiers
  void set_max_time_step(const double mts) { this->max_time_step_ = mts; }

  double time_step(void) const {
    return this->max_time_step_;
  }

  int number_aqueous_components(void) const {
    return chemistry_state_->number_of_aqueous_components();
  }

  int number_free_ion(void) const {
    return chemistry_state_->number_of_aqueous_components();
  }

  int number_total_sorbed(void) const {
    return chemistry_state_->number_of_aqueous_components();
  }

  int number_minerals(void) const {
    return chemistry_state_->number_of_minerals();
  }

  int number_ion_exchange_sites(void) const {
    return chemistry_state_->number_of_ion_exchange_sites();
  }

  int number_sorption_sites(void) const {
    return chemistry_state_->number_of_sorption_sites();
  }

  int using_sorption(void) const {
    return chemistry_state_->using_sorption();
  }

  int using_sorption_isotherms(void) const {
    return chemistry_state_->using_sorption_isotherms();
  }

  bool debug(void) const {
    return debug_;
  }

  void set_debug(const bool value) {
    debug_ = value;
  }

  // Ben: the following two routines provide the interface for
  // output of auxillary cellwise data from chemistry
  Teuchos::RCP<Epetra_MultiVector> get_extra_chemistry_output_data();
  void set_chemistry_output_names(std::vector<std::string>* names);

 protected:
  Teuchos::RCP<State> S_;
  Teuchos::RCP<const AmanziMesh::Mesh> mesh_;
  std::string passwd_;

 private:
  bool debug_;
  bool display_free_columns_;
  double max_time_step_;
  // auxilary state for process kernel
  Teuchos::RCP<Chemistry_State> chemistry_state_;

  // parameter list
  Teuchos::ParameterList parameter_list_;

  Beaker* chem_;
  Beaker::BeakerParameters beaker_parameters_;
  Beaker::BeakerComponents beaker_components_;
  Beaker::BeakerComponents beaker_components_copy_;

  double current_time_;
  double saved_time_;

  std::vector<std::string> aux_names_;
  std::vector<int> aux_index_;

  Teuchos::RCP<Epetra_MultiVector> aux_data_;

  void UpdateChemistryStateStorage();

  void XMLParameters();
  void SetupAuxiliaryOutput();
  void SizeBeakerStructures();
  void CopyCellStateToBeakerStructures(
      int cell_id, Teuchos::RCP<Epetra_MultiVector> total_component_concentration);
  void CopyBeakerStructuresToCellState(
      int cell_id, Teuchos::RCP<Epetra_MultiVector> total_component_concentration);
};

}  // namespace AmanziChemistry
}  // namespace Amanzi
#endif
