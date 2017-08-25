/* -*-  mode: c++; indent-tabs-mode: nil -*- */
/*
  Amanzi is released under the three-clause BSD License. 
  The terms of use and "as is" disclaimer for this license are 
  provided in the top-level COPYRIGHT file.

  Author: Ethan Coon (ecoon@lanl.gov)
*/

//! The interface for a Process Kernel, an equation or system of equations.

#include "boost/algorithm/string.hpp"

#include "Key.hh"
#include "State.hh"
#include "PK_Default.hh"

namespace Amanzi {

PK_Default::PK_Default(const Teuchos::RCP<Teuchos::ParameterList>& pk_tree,
                       const Teuchos::RCP<Teuchos::ParameterList>& global_plist,
                       const Teuchos::RCP<State>& S,
                       const Teuchos::RCP<TreeVector>& solution)
    : S_(S),
      name_(Keys::cleanPListName(pk_tree->name())),
      solution_(solution)
{
  // grab my sublist
  Teuchos::RCP<Teuchos::ParameterList> pks_list = Teuchos::sublist(global_plist, "PKs");
  if (pks_list->isSublist(name_)) {
    plist_ = Teuchos::sublist(pks_list, name_); 
  } else {
    std::stringstream messagestream;
    messagestream << "There is no sublist for PK "<<name_<<"in PKs list\n";
    Errors::Message message(messagestream.str());
    Exceptions::amanzi_throw(message);
  }

  // create a verbose object
  vo_ = Teuchos::rcp(new VerboseObject(name_, *plist_));
};


} // namespace Amanzi
