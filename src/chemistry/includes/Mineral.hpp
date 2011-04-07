/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */
#ifndef __Mineral_hpp__
#define __Mineral_hpp__

/* Class for mineral reaction, should be written with the mineral as
** the reactant:
**
**  Calcite = 1.0 Ca++ + 1.0 HCO3- -1.0 H+
**
*/

#include <cmath>

#include <vector>

#include "Species.hpp"
#include "SecondarySpecies.hpp"
#include "Verbosity.hpp"

// forward declarations
class Block;

class Mineral : public SecondarySpecies {

 public:
  Mineral();
  Mineral(std::string s);
  Mineral(const SpeciesName name,
          SpeciesId mineral_id,
          std::vector<SpeciesName> species,
          std::vector<double> stoichiometries,
          std::vector<int> species_ids,
          const double h2o_stoich, const double mol_wt,
          const double logK, const double molar_volume,
          const double specific_surface_area);
  ~Mineral();

  // update molalities
  void Update(const std::vector<Species>primary_species);
  // add stoichiometric contribution of complex to total
  void AddContributionToTotal(std::vector<double> &total);
  // add derivative of total with respect to free-ion to dtotal
  void AddContributionToDTotal(const std::vector<Species> primary_species,
                               Block *dtotal);

  void Display(void) const;
  void DisplayResultsHeader(void) const;
  void DisplayResults(void) const;

  double saturation_index(void) const { return std::log10(Q_over_K()); };  // SI = log10(Q/Keq)
  double Q_over_K(void) const { return std::exp(this->lnQK_); };

  double molar_volume(void) const { return this->molar_volume_; }
  void molar_volume(double d) { this->molar_volume_ = d; }

  void UpdateSurfaceAreaFromVolumeFraction(const double total_volume);

  double surface_area(void) const { return this->surface_area_; }
  void set_surface_area(const double d) { this->surface_area_ = d; }

  double specific_surface_area(void) const { return this->specific_surface_area_; }
  void set_specific_surface_area(const double d) { this->specific_surface_area_ = d; }

  double volume_fraction(void) const { return this->volume_fraction_; }
  void set_volume_fraction(const double d) { this->volume_fraction_ = d; }

  void set_verbosity(const Verbosity verbosity) { this->verbosity_ = verbosity; };
  Verbosity verbosity(void) const { return this->verbosity_; };

 protected:

 private:
  Verbosity verbosity_;
  double saturation_index_;
  double molar_volume_;     // [cm^3 / moles] 
  double specific_surface_area_;  // [m^2/g] 
  double surface_area_;      // [m^2 mineral surface / m^3 mineral]
  double volume_fraction_;   // [m^3 mineral / m^3 bulk]

};

#endif // __Mineral_hpp__
