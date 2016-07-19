/*




 */



#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

// TPLs
#include "Teuchos_RCP.hpp"
#include "Teuchos_ParameterList.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"
#include "UnitTest++.h"

// Amanzi
#include "GMVMesh.hh"
#include "MeshFactory.hh"
#include "MeshAudit.hh"
#include "State.hh"

#include "ObservableLineSegmentAqueous.hh"


TEST(OBSERVABLE_LINE_SEGMENT){


  using namespace Amanzi;
  using namespace Amanzi::AmanziMesh;
  using namespace Amanzi::AmanziGeometry;


  Epetra_MpiComm comm(MPI_COMM_WORLD);
  int MyPID = comm.MyPID();

  if (MyPID == 0) std::cout << "Test: Obsevable Line Segment" << std::endl;


  /* read parameter list */
  std::string xmlFileName = "test/observable_line_segment.xml";
  Teuchos::RCP<Teuchos::ParameterList> plist = Teuchos::getParametersFromXmlFile(xmlFileName);

  // create an SIMPLE mesh framework
  Teuchos::ParameterList regions_list = plist->get<Teuchos::ParameterList>("regions");
  Teuchos::RCP<Amanzi::AmanziGeometry::GeometricModel> gm =
      Teuchos::rcp(new Amanzi::AmanziGeometry::GeometricModel(3, regions_list, &comm));


  Teuchos::ParameterList well_list = regions_list.get<Teuchos::ParameterList>("Well3012").get<Teuchos::ParameterList>("region: line segment");

  Teuchos::Array<double> xyz0 = well_list.get<Teuchos::Array<double> >("end coordinate"); 
  Teuchos::Array<double> xyz1 = well_list.get<Teuchos::Array<double> >("opposite end coordinate"); 

  Teuchos::ParameterList well_list_2 = regions_list.get<Teuchos::ParameterList>("Well3013").get<Teuchos::ParameterList>("region: line segment");

  Teuchos::Array<double> xyz0_2 = well_list_2.get<Teuchos::Array<double> >("end coordinate"); 
  Teuchos::Array<double> xyz1_2 = well_list_2.get<Teuchos::Array<double> >("opposite end coordinate"); 


  FrameworkPreference pref;
  pref.clear();
  pref.push_back(MSTK);
  pref.push_back(STKMESH);

  MeshFactory meshfactory(&comm);
  meshfactory.preference(pref);
  Teuchos::RCP<Mesh> mesh = meshfactory(0.0, 0.0, 0.0, 10.0, 10.0, 10.0, 5, 5, 5, gm);

  /* create a simple state and populate it */
  Amanzi::VerboseObject::hide_line_prefix = false;

  Teuchos::ParameterList state_list = plist->sublist("state");
  Teuchos::RCP<State> S = Teuchos::rcp(new State(state_list));
  S->RegisterDomainMesh(Teuchos::rcp_const_cast<Mesh>(mesh));

  S->RequireField("test_field", "test_field")->SetMesh(mesh)->SetGhosted(true)
    ->SetComponent("cell", AmanziMesh::CELL, 1);

  S->Setup();
  S->InitializeFields();
  S->InitializeEvaluators();

  Epetra_MultiVector& test =  *S->GetFieldData("test_field", "test_field")->ViewComponent("cell");

  int ncells_owned = mesh->num_entities(AmanziMesh::CELL, AmanziMesh::OWNED);

  double A=3., B=1., C=5., D=0.2;

  for (int c=0; c<ncells_owned; c++){
    const AmanziGeometry::Point& xc = mesh->cell_centroid(c);
    test[0][c] = A*xc[0] + B*xc[1] + C*xc[2] + D;
  }

  Teuchos::ParameterList obs_plist, units_plist;

  std::string var="test_field";
  std::string func="observation data: point";
  std::string region="Well3012";
  obs_plist.set<std::string>("interpolation", "linear");
  obs_plist.set<std::string>("weighting", "none");
  obs_plist.set<std::string>("region", region);
  obs_plist.set<std::string>("variable", var);
  obs_plist.set<std::string>("functional", func);

  /******************************* Region  Well3012 *************/   
  Teuchos::RCP<ObservableLineSegment> observe = 
    Teuchos::rcp(new ObservableLineSegmentAqueous(var, region, func, obs_plist, units_plist, mesh));

  double value, volume;
  observe -> ComputeRegionSize();
  observe -> ComputeObservation(*S, &value, &volume);

  Teuchos::Array<double> xyzc(3);
  double len = 0.;

  for (int i=0;i<3;i++){
    xyzc[i] = 0.5*(xyz0[i] + xyz1[i]);
    len += (xyz0[i] - xyz1[i])*(xyz0[i] - xyz1[i]);
  }
  len = sqrt(len);
  double exact_val = A*xyzc[0] + B*xyzc[1] + C*xyzc[2] + D;

  /******************************* Region  Well3013 *************/   
  /*** One cell region ****/

  region="Well3013";

  Teuchos::RCP<ObservableLineSegment> observe2 = 
    Teuchos::rcp(new ObservableLineSegmentAqueous(var, region, func, obs_plist, units_plist, mesh));

  double value2, volume2;
  observe2 -> ComputeRegionSize();
  observe2 -> ComputeObservation(*S, &value2, &volume2);
  double len2 = 0.;
  for (int i=0;i<3;i++){
    xyzc[i] = 0.5*(xyz0_2[i] + xyz1_2[i]);
    len2 += (xyz0_2[i] - xyz1_2[i])*(xyz0_2[i] - xyz1_2[i]);
    //std::cout<<xyzc[i]<<" "<<xyz0_2[i]<<" "<<xyz1_2[i]<<"\n";
  }
  len2 = sqrt(len2);
  double exact_val2 = A*xyzc[0] + B*xyzc[1] + C*xyzc[2] + D;

  // std::cout<<value2<<" "<<volume2<<"\n";
  // std::cout<<exact_val2*len2<<" "<<len2<<"\n";

  CHECK((std::abs(exact_val*len - value) < 1e-10) && (std::abs(exact_val2*len2 - value2) < 1e-10));

}