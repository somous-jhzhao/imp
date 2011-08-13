/**
 *  \file PairContainerSet.cpp
 *  \brief A set of PairContainers.
 *
 *  This file is generated by a script (core/tools/make-container).
 *  Do not edit directly.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/PairContainerSet.h"
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE

PairContainerSet
::PairContainerSet(Model *m, std::string name):
  PairContainer(m, name),
  deps_(new DependenciesScoreState(this), m){
}

namespace {
  Model *my_get_model(const PairContainersTemp &in) {
    if (in.empty()) {
      IMP_THROW("Cannot initialize from empty list of containers.",
                IndexException);
    }
    return in[0]->get_model();
  }
}

PairContainerSet
::PairContainerSet(const PairContainersTemp& in,
                        std::string name):
  PairContainer(my_get_model(in), name),
  deps_(new DependenciesScoreState(this), my_get_model(in)){
  set_pair_containers(in);
}


bool
PairContainerSet
::get_contains_particle_pair(const ParticlePair& vt) const {
  for (PairContainerConstIterator it= pair_containers_begin();
       it != pair_containers_end(); ++it) {
    if ((*it)->get_contains_particle_pair(vt)) return true;
  }
  return false;
}

void PairContainerSet::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << get_number_of_particle_pairs()
      << " containers" << std::endl;
}


ParticleIndexPairs PairContainerSet::get_indexes() const {
  ParticleIndexPairs sum;
  for (PairContainerConstIterator it= pair_containers_begin();
       it != pair_containers_end(); ++it) {
    ParticleIndexPairs cur=(*it)->get_indexes();
    sum.insert(sum.end(), cur.begin(), cur.end());
  }
  return sum;
}

IMP_LIST_IMPL(PairContainerSet,
              PairContainer,
              pair_container,
              PairContainer*,
              PairContainers);


void PairContainerSet::apply(const PairModifier *sm) {
  for (unsigned int i=0; i< get_number_of_pair_containers(); ++i) {
    get_pair_container(i)->apply(sm);
  }
}

void PairContainerSet::apply(const PairDerivativeModifier *sm,
                               DerivativeAccumulator &da) {
  for (unsigned int i=0; i< get_number_of_pair_containers(); ++i) {
    get_pair_container(i)->apply(sm, da);
  }
}

double PairContainerSet::evaluate(const PairScore *s,
                                       DerivativeAccumulator *da) const {
  return template_evaluate(s, da);
}

double PairContainerSet::evaluate_if_good(const PairScore *s,
                                               DerivativeAccumulator *da,
                                               double max) const {
  return template_evaluate_if_good(s, da, max);
}


ParticlesTemp PairContainerSet::get_contained_particles() const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< get_number_of_pair_containers(); ++i) {
    ParticlesTemp cur= get_pair_container(i)->get_contained_particles();
    ret.insert(ret.end(), cur.begin(), cur.end());
  }
  return ret;
}



IMPCONTAINER_END_NAMESPACE
