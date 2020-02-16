/*
  This is the multiphase component of the Amanzi code. 

  Copyright 2010-201x held jointly by LANS/LANL, LBNL, and PNNL. 
  Amanzi is released under the three-clause BSD License. 
  The terms of use and "as is" disclaimer for this license are 
  provided in the top-level COPYRIGHT file.

  Authors: Quan Bui (mquanbui@math.umd.edu)

  We use this class to set up simple water retention model 
  for decoupled multiphase flow
*/

#ifndef AMANZI_MULTIPHASE_WRM_SIMPLE_HH_
#define AMANZI_MULTIPHASE_WRM_SIMPLE_HH_

#include "Teuchos_ParameterList.hpp"

#include "Factory.hh"

#include "WRMmp.hh"

namespace Amanzi {
namespace Multiphase {

class WRMmp_Simple : public WRMmp {
 public:
  WRMmp_Simple(Teuchos::ParameterList& plist);
  ~WRMmp_Simple() {};
  
  // required methods from the base class
  double k_relative(double Sw, const std::string& phase);
  double capillaryPressure(double saturation);
  double dPc_dS(double saturation);
  double dKdS(double Sw, const std::string& phase);

  void Init_(double S_rw, double S_rn, double coef);

 private:
  double S_rw_, S_rn_, coef_, exponent_;

  static Utils::RegisteredFactory<WRMmp, WRMmp_Simple> factory_;
};

}  // namespace Flow
}  // namespace Amanzi
 
#endif
