/**
 *  \file core/triplet_predicates.h    \brief Define some predicates.
 *
 *  This file is generated by a script (core/tools/make-container).
 *  Do not edit directly.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_TRIPLET_PREDICATES_H
#define IMPCORE_TRIPLET_PREDICATES_H

#include "core_config.h"
#include <IMP/TripletPredicate.h>
#include <boost/random.hpp>
#include "internal/container_helpers.h"

IMPCORE_BEGIN_NAMESPACE

/** Always return a constant value.
 */
class IMPCOREEXPORT ConstantTripletPredicate: public TripletPredicate {
  int v_;
public:
  ConstantTripletPredicate(int v,
                             std::string name="ConstTripletPredicate%1%");
  IMP_INDEX_TRIPLET_PREDICATE(ConstantTripletPredicate, {
      IMP_UNUSED(m); IMP_UNUSED(pi);
      return v_;
    });
};

/** Return a unique predicate value for each unordered set of ParticleTypes
    (see Typed).
*/

class IMPCOREEXPORT UnorderedTypeTripletPredicate: public TripletPredicate {
public:
  UnorderedTypeTripletPredicate(std::string name
                             ="UnorderedTypeTripletPredicate%1%");
  IMP_INDEX_TRIPLET_PREDICATE(UnorderedTypeTripletPredicate, {
      return internal::get_type_hash(m, pi);
    });
};

/** Return a unique predicate value for each ordered triplet of
    ParticleTypes (see Typed).
*/
class IMPCOREEXPORT OrderedTypeTripletPredicate: public TripletPredicate {
public:
  OrderedTypeTripletPredicate(std::string name
                             ="OrderedTypeTripletPredicate%1%");
  IMP_INDEX_TRIPLET_PREDICATE(OrderedTypeTripletPredicate, {
      return internal::get_ordered_type_hash(m, pi);
    });
  int get_value(const core::ParticleTypes& types) {
    return internal::get_ordered_type_hash(types);
  }
};

/** Return true if all members of the tuple are the same. */
class IMPCOREEXPORT AllSameTripletPredicate: public TripletPredicate {
public:
  AllSameTripletPredicate(std::string name
                             ="AllSameTripletPredicate%1%");
  IMP_INDEX_TRIPLET_PREDICATE(AllSameTripletPredicate, {
      return internal::get_all_same(m, pi);
    });
};

/** Return true with a fixed probability. */
class IMPCOREEXPORT CoinFlipTripletPredicate: public TripletPredicate {
  double p_;
  mutable boost::uniform_real<double> rng_;
public:
  CoinFlipTripletPredicate(double p, std::string name
                             ="CoinFlipTripletPredicate%1%");
  IMP_INDEX_TRIPLET_PREDICATE(CoinFlipTripletPredicate, {
      IMP_UNUSED(m);
      IMP_UNUSED(pi);
      if (rng_(base::random_number_generator)<p_) return 1;
      else return 0;
    });
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_TRIPLET_PREDICATES_H */
