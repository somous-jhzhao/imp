/**
 *  \file ListPairContainer.cpp   \brief A list of ParticlePairs.
 *
 *  This file is generated by a script (core/tools/make-container).
 *  Do not edit directly.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/ListPairContainer.h"
#include <algorithm>


IMPCORE_BEGIN_NAMESPACE

ListPairContainer::ListPairContainer(const ParticlePairs &ps){
  sorted_=false;
  set_particle_pairs(ps);
  set_is_editing(false);
}

IMP_LIST_IMPL(ListPairContainer, ParticlePair,
              particle_pair, ParticlePair,ParticlePairs,, {
                if (sorted_) std::sort(particle_pairs_begin(),
                                       particle_pairs_end());
              },);


void ListPairContainer::set_is_editing(bool tf) {
  if (tf== !sorted_) return;
  else {
    sorted_=!tf;
    if (sorted_) {
      std::sort(particle_pairs_begin(), particle_pairs_end());
    }
  }
}


bool
ListPairContainer::get_contains_particle_pair(ParticlePair vt) const {
  IMP_CHECK_OBJECT(this);
  return std::binary_search(particle_pairs_begin(), particle_pairs_end(), vt);
}

void ListPairContainer::show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << "ListPairContainer with " << get_number_of_particle_pairs()
      << " particle_pairs." << std::endl;
}

void ListPairContainer::apply(const PairModifier *sm) {

  std::for_each(ListPairContainer::particle_pairs_begin(),
                ListPairContainer::particle_pairs_end(),
                PairFunctor(sm));
}

void ListPairContainer::apply(const PairModifier *sm,
                               DerivativeAccumulator *da) {

  std::for_each(ListPairContainer::particle_pairs_begin(),
                ListPairContainer::particle_pairs_end(),
                PairFunctor(sm, da));
}

double ListPairContainer::evaluate(const PairScore *s,
                                    DerivativeAccumulator *da) const {
  double score=0;
  for (unsigned int i=0;
       i< ListPairContainer::get_number_of_particle_pairs();
       ++i) {
    score+= IMP::internal::ContainerTraits<ParticlePair>
      ::evaluate(s, ListPairContainer::get_particle_pair(i), da);
  }
  return score;
}


ParticlePairs ListPairContainer::get_particle_pairs() const {
  return ParticlePairs(ListPairContainer::particle_pairs_begin(),
                    ListPairContainer::particle_pairs_end());
}

IMPCORE_END_NAMESPACE
