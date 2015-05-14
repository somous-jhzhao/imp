/**
 *  \file RatioVolatilityScore.h   \brief Chi free SAXS score
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/RatioVolatilityScore.h>
#include <boost/random/uniform_real.hpp>
#include <algorithm>

IMPSAXS_BEGIN_NAMESPACE


// TODO: currently assumes uniform sampling of experimental profile
double RatioVolatilityScore::compute_score(const Profile* exp_profile,
                                           const Profile* model_profile,
                                           bool use_offset) const {
  if (model_profile->size() != exp_profile->size()) {
    IMP_THROW("RatioVolatilityScore::compute_score is supported "
              << "only for profiles with the same q values!",
              ValueException);
  }

  double bin_size = PI / dmax_; // default dmax = 400
  unsigned int number_of_bins = exp_profile->size(); //std::floor(exp_profile->get_max_q()/bin_size);
  // number of profile points in each bin
  double number_of_points_in_bin = exp_profile->size()/number_of_bins;

  Vector<double> ratio(number_of_bins, 0.0);
  for (unsigned int i = 0; i < number_of_bins; i++) {
    unsigned int index1 = algebra::get_rounded(i*number_of_points_in_bin);
    unsigned int index2 = algebra::get_rounded((i+1)*number_of_points_in_bin);
    // calculate average intensity in the bin
    double intensity1(0.0), intensity2(0.0);
    for (unsigned int j = index1; j<index2; j++) {
      intensity1 += exp_profile->get_intensity(j);
      intensity2 += model_profile->get_intensity(j);
    }
    intensity1 /= (index2-index1);
    intensity2 /= (index2-index1);
    ratio[i] = intensity1/intensity2;
  }

  double vr = 0;
  for (unsigned int i = 0; i < (number_of_bins-1); i++) {
    vr += 2*std::fabs(ratio[i]-ratio[i+1])/(ratio[i]+ratio[i+1]);
  }
  return 100*vr/number_of_bins;
}


double RatioVolatilityScore::compute_scale_factor(const Profile* exp_profile,
                                                  const Profile* model_profile,
                                                  const double offset) const {
  double sum1 = 0.0, sum2 = 0.0;
  unsigned int profile_size =
    std::min(model_profile->size(), exp_profile->size());
  for (unsigned int k = 0; k < profile_size; k++) {
    double square_error = square(exp_profile->get_error(k));
    double weight_tilda = model_profile->get_weight(k) / square_error;

    sum1 += weight_tilda * model_profile->get_intensity(k) *
            (exp_profile->get_intensity(k) - offset);
    sum2 += weight_tilda * square(model_profile->get_intensity(k));
  }
  return sum1 / sum2;
  //double m1 = exp_profile->mean_intensity();
  //double m2 = model_profile->mean_intensity();
  //return m1 / m2;
}

double RatioVolatilityScore::compute_offset(const Profile* exp_profile,
                                            const Profile* model_profile) const {
  return 0;
}

IMPSAXS_END_NAMESPACE
