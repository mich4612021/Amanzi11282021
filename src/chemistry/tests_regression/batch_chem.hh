/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */
#ifndef AMANZI_CHEMISTRY_BATCH_CHEM_HH_
#define AMANZI_CHEMISTRY_BATCH_CHEM_HH_

#include <string>
#include <vector>

#include "beaker.hh"
#include "chemistry_verbosity.hh"

struct SimulationParameters {
  std::string description;
  std::string verbosity_name;
  amanzi::chemistry::Verbosity verbosity;
  std::string comparison_model;
  std::string database_type;
  std::string database_file;
  std::string activity_model;
  double porosity;  // [-]
  double saturation;  // [-]
  double volume;  // [m^3]
  double delta_time;  // [s]
  int num_time_steps;  // [-]
  int output_interval;  // [steps]
  double cation_exchange_capacity;
  std::vector<double> mineral_ssa;  // specific surface area []
  std::vector<double> site_density;  // sorption site density []
};

static const std::string kSimulationSection("simulation parameters");
static const std::string kDescriptionParam("description");
static const std::string kVerbosityParam("verbosity");
static const std::string kComparisonModelParam("comparison_model");
static const std::string kDatabaseTypeParam("database_type");
static const std::string kDatabaseFileParam("database_file");
static const std::string kActivityModelParam("activity_model");
static const std::string kPorosityParam("porosity");
static const std::string kSaturationParam("saturation");
static const std::string kVolumeParam("volume");
static const std::string kDeltaTimeParam("delta_time");
static const std::string kNumTimeStepsParam("num_time_steps");
static const std::string kOutputIntervalParam("output_interval");

static const std::string kTotalSection("total");
static const std::string kMineralSection("mineral");
static const std::string kSorbedSection("total_sorbed");
static const std::string kFreeIonSection("free_ion");
static const std::string kIonExchangeSection("ion_exchange");

static const std::string kSiteDensitySection("site_density");
static const std::string kSpecificSurfaceAreaSection("specific_surface_area");
static const std::string kCationExchangeCapacitySection("cation_exchange_capacity");

int CommandLineOptions(int argc, char** argv,
                       std::string* verbosity_name,
                       std::string* input_file_name,
                       std::string* template_file_name,
                       bool* debug_batch_driver);

void WriteTemplateFile(const std::string& file_name);

void ReadInputFile(const std::string& file_name,
                   SimulationParameters* simulation_params,
                   amanzi::chemistry::Beaker::BeakerComponents* components);

void ParseSimulationParameter(const std::string& raw_line,
                              SimulationParameters* params);

void ParseComponentValue(const std::string& raw_line,
                         std::vector<double>* component);
void ParseComponentValue(const std::string& raw_line,
                         double* component);

void ModelSpecificParameters(const std::string model,
                             amanzi::chemistry::Beaker::BeakerParameters* parameters);
void OverrideParameters(const SimulationParameters& simulation_params,
                        amanzi::chemistry::Beaker::BeakerParameters* parameters);

void PrintInput(const SimulationParameters& params,
                const amanzi::chemistry::Beaker::BeakerComponents& components);
void PrintSimulationParameters(const SimulationParameters& params);
void PrintComponents(const amanzi::chemistry::Beaker::BeakerComponents& components);


#endif  /* AMANZI_CHEMISTRY_BATCH_CHEM_HH_ */
