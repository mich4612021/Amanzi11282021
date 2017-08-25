/* -*-  mode: c++; indent-tabs-mode: nil -*- */
/*
  A region defined by a logical operation on one or two other regions
 
  Operations supported on a single region are only the NOT operation
 
  Operations supported on a pair of regions are UNION, SUBTRACT and
  INTERSECT

  Copyright 2010-2013 held jointly by LANS/LANL, LBNL, and PNNL. 
  Amanzi is released under the three-clause BSD License. 
  The terms of use and "as is" disclaimer for this license are 
  provided in the top-level COPYRIGHT file.

  Authors: Rao Garimella
*/

#include "dbc.hh"
#include "errors.hh"

#include "RegionLogical.hh"

namespace Amanzi {
namespace AmanziGeometry {

//
// RegionLogical:: constructor
// -------------------------------------------------------------
RegionLogical::RegionLogical(const std::string& name, 
                             const Set_ID id,
                             const std::string& operation_str,
                             const std::vector<std::string>& component_regions,
                             const LifeCycleType lifecycle)
  : Region(name, id, false, LOGICAL, 3, lifecycle),
    operation_(NOBOOLEAN)
{
  for (std::vector<std::string>::const_iterator it=component_regions.begin();
         it!=component_regions.end(); ++it) {
    component_regions_.push_back(*it);
  }
       
  
  // Region dimension is set arbitrarily as 3 since the set of
  // entities in the mesh will determine the dimension

  if (operation_str == "complement")
    operation_ = COMPLEMENT;
  else if (operation_str == "union")
    operation_ = UNION;
  else if (operation_str == "intersect")
    operation_ = INTERSECT;
  else if (operation_str == "subtract")
    operation_ = SUBTRACT;
  else {
    Errors::Message mesg("Unknown logical operation type requested on regions");
    amanzi_throw(mesg);
  }
}

  
// -------------------------------------------------------------
// RegionLogical::inside
// -------------------------------------------------------------
bool
RegionLogical::inside(const Point& p) const
{
  Errors::Message mesg("In/out check not implemented for logical regions because the check may not be implemented for one of its component regions");
  Exceptions::amanzi_throw(mesg);
  return false;
}

} // namespace AmanziGeometry
} // namespace Amanzi
