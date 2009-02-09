/**
 *  \file SAXSProfile.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPSAXS_SAXS_PROFILE_H
#define IMPSAXS_SAXS_PROFILE_H

#include "config.h"
#include "FormFactorTable.h"
#include "Distribution.h"

#include <IMP/Model.h>

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

class RadialDistributionFunction;

/* Basic profile class, can be initialized from the input file
   (experimental or theoretical) or computed from a set of Model
   Particles (theoretical)
*/

class IMPSAXSEXPORT SAXSProfile {
public:
  //! init from file
  SAXSProfile(const String& file_name);

  //! init for theoretical profile
  SAXSProfile(Float qmin, Float qmax, Float delta, FormFactorTable* ff_table);

private:
  class IntensityEntry {
  public:
    IntensityEntry() : q_(0.0), intensity_(0.0), error_(1.0), weight_(1.0) {}
    IntensityEntry(Float q) : q_(q),intensity_(0.0),error_(1.0),weight_(1.0) {}
    IntensityEntry(Float q, Float intensity, Float error)
      : q_(q), intensity_(intensity), error_(error), weight_(1.0) {}

    Float q_;
    Float intensity_;
    Float error_;
    Float weight_;
  };

  friend std::ostream& operator<<(std::ostream& q, const IntensityEntry& e);

  friend std::istream& operator>>(std::istream& q, IntensityEntry& e);

public:
  //! computes theoretical profile
  void calculate_profile(const std::vector<Particle*>& particles) {
    calculate_profile_real(particles);
  }

  //! computes theoretical profile contribution from iter-molecular
  // interactions between the particles
  void calculate_profile(const std::vector<Particle*>& particles1,
                         const std::vector<Particle*>& particles2) {
    calculate_profile_real(particles1, particles2);
  }

  //! reads SAXS profile from file
  void read_SAXS_file(const String& file_name);

  //! print to file
  void write_SAXS_file(const String& file_name);

  //! return sampling resolution
  Float get_delta_q() const { return delta_q_; }

  //! return minimal sampling point
  Float get_min_q() const { return min_q_; }

  //! return maximal sampling point
  Float get_max_q() const { return max_q_; }

  //! return number of entries in SAXS profile
  unsigned int size() const { return profile_.size(); }
  Float get_intensity(unsigned int i) const { return profile_[i].intensity_; }
  Float get_q(unsigned int i) const { return profile_[i].q_; }
  Float get_error(unsigned int i) const { return profile_[i].error_; }
  Float get_weight(unsigned int i) const { return profile_[i].weight_; }
  Float get_pr_resolution() const { return pr_resolution_; }
  Float set_pr_resolution(Float pr_resolution)
    { pr_resolution_ = pr_resolution; return pr_resolution_; }
  Float get_max_pr_distance() const { return max_pr_distance_; }
  Float set_max_pr_distance(Float max_pr_distance)
    { max_pr_distance_ = max_pr_distance; return max_pr_distance_; }

  // parameter for E^2(q), used in faster calculation
  static Float modulation_function_parameter_;

private:
  void init();

  void calculate_profile_reciprocal(const std::vector<Particle*>& particles);

  void calculate_profile_reciprocal(const std::vector<Particle*>& particles1,
                                    const std::vector<Particle*>& particles2);

  void calculate_profile_real(const std::vector<Particle*>& particles);

  void calculate_profile_real(const std::vector<Particle*>& particles1,
                              const std::vector<Particle*>& particles2);

  void radial_distribution_2_profile(const RadialDistributionFunction& r_dist);

  Float sinc(Float value) {
    if(fabs(value) < 1.0e-16) return 1.0;
    return sin(value)/value;
  }

protected:
  std::vector<IntensityEntry> profile_; // the profile
  Float min_q_, max_q_; // minimal and maximal s values  in the profile
  Float delta_q_; // profile sampling resolution
  FormFactorTable* ff_table_; // pointer to form factors table
  Float pr_resolution_;   // resolution of P(r) function (radial distribution)
  Float max_pr_distance_;  // paramter for maximum r value for p(r) function
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_SAXS_PROFILE_H */
