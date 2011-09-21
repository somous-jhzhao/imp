/**
 *  \file CloseBipartitePairContainer.cpp   \brief A list of ParticlePairs.
 *
 *  This file is generated by a script (core/tools/make-container).
 *  Do not edit directly.
 *
 *  Copyright 2007-2011 IMP Inventors. Close rights reserved.
 *
 */

#include "IMP/core/internal/CoreCloseBipartitePairContainer.h"
#include <IMP/core/BoxSweepClosePairsFinder.h>
#include <IMP/core/GridClosePairsFinder.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/core/internal/pair_helpers.h>
#include <IMP/core/internal/close_pairs_helpers.h>
#include <IMP/PairModifier.h>
#include <algorithm>


IMPCORE_BEGIN_INTERNAL_NAMESPACE

IMP_LIST_IMPL(CoreCloseBipartitePairContainer,
              PairFilter,
              pair_filter,
              PairFilter*,
              PairFilters);


CoreCloseBipartitePairContainer
::CoreCloseBipartitePairContainer(SingletonContainer *a,
                              SingletonContainer *b,
                              double distance,
                              double slack):
  P(a->get_model(), "CoreCloseBipartitePairContainer") {
  initialize(a,b, distance, slack,
             core::internal::default_cpf(a->get_number_of_particles()
                                         +b->get_number_of_particles()));
}

CoreCloseBipartitePairContainer
::CoreCloseBipartitePairContainer(SingletonContainer *a,
                              SingletonContainer *b,
                              double distance,
                              core::ClosePairsFinder *cpf,
                              double slack):
  P(a->get_model(), "CoreCloseBipartitePairContainer") {
  initialize(a,b, distance, slack,
             cpf);
}

void CoreCloseBipartitePairContainer::initialize(SingletonContainer *a,
                                             SingletonContainer *b,
                                             double distance,
                                             double slack,
                                             core::ClosePairsFinder *cpf) {
  initialize_active_container(get_model());
  slack_=slack;
  distance_=distance;
  a_=a; b_=b;
  cpf_=cpf;
  cpf_->set_distance(distance_+2*slack_);
  first_call_=true;
  moveda_= cpf_->get_moved_singleton_container(a_, slack_);
  movedb_= cpf_->get_moved_singleton_container(b_, slack_);
}

IMP_ACTIVE_CONTAINER_DEF(CoreCloseBipartitePairContainer, {
    moveda_->set_log_level(l);
    movedb_->set_log_level(l);
  });

ParticlesTemp CoreCloseBipartitePairContainer
::get_state_input_particles() const {
  ParticlesTemp ret(cpf_->get_input_particles(a_->get_particles()));
  ParticlesTemp retb(cpf_->get_input_particles(b_->get_particles()));
  ret.insert(ret.end(), retb.begin(), retb.end());
  if (get_number_of_pair_filters() >0) {
    ParticlesTemp retc;
    for (PairFilterConstIterator it= pair_filters_begin();
         it != pair_filters_end(); ++it) {
      for (unsigned int i=0; i< ret.size(); ++i) {
        ParticlesTemp cur= (*it)->get_input_particles(ret[i]);
        retc.insert(retc.end(), cur.begin(), cur.end());
      }
    }
    ret.insert(ret.end(), retc.begin(), retc.end());
  }
  return ret;
}

ContainersTemp CoreCloseBipartitePairContainer
::get_state_input_containers() const {
  ContainersTemp ret= cpf_->get_input_containers(a_->get_particles());
  ContainersTemp retb= cpf_->get_input_containers(b_->get_particles());
  ret.insert(ret.end(), retb.begin(), retb.end());
  ret.push_back(a_);
  ret.push_back(b_);
  ret.push_back(moveda_);
  ret.push_back(movedb_);
  if (get_number_of_pair_filters() >0) {
    ParticlesTemp ps(cpf_->get_input_particles(a_->get_particles()));
    ParticlesTemp psb(cpf_->get_input_particles(b_->get_particles()));
    ps.insert(ps.end(), psb.begin(), psb.end());
    ContainersTemp retc;
    for (PairFilterConstIterator it= pair_filters_begin();
         it != pair_filters_end(); ++it) {
      for (unsigned int i=0; i< ps.size(); ++i) {
        ContainersTemp cur= (*it)->get_input_containers(ps[i]);
        retc.insert(retc.end(), cur.begin(), cur.end());
      }
    }
    ret.insert(ret.end(), retc.begin(), retc.end());
  }
  return ret;
}

void CoreCloseBipartitePairContainer::do_before_evaluate() {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(a_);
  IMP_CHECK_OBJECT(b_);
  IMP_CHECK_OBJECT(cpf_);
  core::internal::ListLikePairContainer::do_before_evaluate();
  if (first_call_) {
    ParticleIndexPairs t= cpf_->get_close_pairs(get_model(),
                                                a_->get_indexes(),
                                                b_->get_indexes());
    core::internal::filter_close_pairs(this, t);
    moveda_->reset();
    movedb_->reset();
    update_list(t);
    first_call_=false;
  } else {
    if (moveda_->get_number_of_particles() != 0
        || movedb_->get_number_of_particles() != 0) {
      if ((moveda_->get_number_of_particles()
           + movedb_->get_number_of_particles())
          < a_->get_number_of_particles()*.1
          + b_->get_number_of_particles()*.1) {
        ParticleIndexPairs ret0= cpf_->get_close_pairs(get_model(),
                                                       moveda_->get_indexes(),
                                                       movedb_->get_indexes());
        ParticleIndexPairs ret1= cpf_->get_close_pairs(get_model(),
                                                       moveda_->get_indexes(),
                                                       b_->get_indexes());
        ParticleIndexPairs ret2= cpf_->get_close_pairs(get_model(),
                                                       a_->get_indexes(),
                                                       movedb_->get_indexes());
        ParticleIndexPairs ret; ret.reserve(ret0.size()
                                           + ret1.size()+ret2.size());
        ret.insert(ret.end(), ret0.begin(), ret0.end());
        ret.insert(ret.end(), ret1.begin(), ret1.end());
        ret.insert(ret.end(), ret2.begin(), ret2.end());
        core::internal::filter_close_pairs(this, ret);
        add_to_list(ret);
        moveda_->reset_moved();
        movedb_->reset_moved();
      } else {
        ParticleIndexPairs ret= cpf_->get_close_pairs(get_model(),
                                                      a_->get_indexes(),
                                                      b_->get_indexes());
        core::internal::filter_close_pairs(this, ret);
        update_list(ret);
        moveda_->reset();
        movedb_->reset();
      }
    }
  }
}


void CoreCloseBipartitePairContainer::do_after_evaluate() {
  core::internal::ListLikePairContainer::do_after_evaluate();
}


void CoreCloseBipartitePairContainer::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << "container "
      << *a_ << " and " << *b_ << std::endl;
}


ParticlesTemp CoreCloseBipartitePairContainer::get_contained_particles() const {
  ParticlesTemp ret =cpf_->get_input_particles(a_->get_particles());
  ParticlesTemp ret0 =cpf_->get_input_particles(b_->get_particles());
  ret.insert(ret.end(), ret0.begin(), ret0.end());
  return ret;
}

ParticleIndexPairs
CoreCloseBipartitePairContainer::get_all_possible_indexes() const {
  ParticleIndexes pis= a_->get_all_possible_indexes();
  ParticleIndexes pjs= b_->get_all_possible_indexes();
  ParticleIndexPairs ret; ret.resize(pis.size()*pjs.size());
  for (unsigned int i=0; i< pis.size(); ++i) {
    for (unsigned int j=0; j< pjs.size(); ++j) {
      ret.push_back(ParticleIndexPair(pis[i], pjs[j]));
    }
  }
  return ret;
}

IMPCORE_END_INTERNAL_NAMESPACE
