/**
 *  \file TripletContainerSet.h
 *  \brief Store a set of TripletContainers
 *
 *  This file is generated by a script (core/tools/make-container).
 *  Do not edit directly.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_TRIPLET_CONTAINER_SET_H
#define IMPCONTAINER_TRIPLET_CONTAINER_SET_H

#include "container_config.h"
#include <IMP/TripletContainer.h>
#include <IMP/container_macros.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/scoped.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Stores a set of TripletContainers
/** The input sets must be disjoint. This can change if there is
    demand for it.

    \usesconstraint
*/
class IMPCONTAINEREXPORT TripletContainerSet
  : public TripletContainer
{
  IMP_CONTAINER_DEPENDENCIES(TripletContainerSet,
                             {
                               ret.insert(ret.end(),
                                          back_->triplet_containers_begin(),
                                          back_->triplet_containers_end());
                             });
  static TripletContainerSet* get_set(TripletContainer* c) {
    return dynamic_cast<TripletContainerSet*>(c);
  }
 public:
  //! Construct and empty set
  TripletContainerSet(Model *m,
                        std::string name="TripletContainerSet %1%");

  TripletContainerSet(const TripletContainersTemp &pc,
                        std::string name="TripletContainerSet %1%");

  bool get_contains_particle_triplet(const ParticleTriplet&) const;
  void apply(const TripletModifier *sm);
  void apply(const TripletDerivativeModifier *sm,
             DerivativeAccumulator &da);
  double evaluate(const TripletScore *s,
                  DerivativeAccumulator *da) const;
  double evaluate_if_good(const TripletScore *s,
                          DerivativeAccumulator *da,
                          double max) const;
 template <class SM>
  void template_apply(const SM *sm,
                      DerivativeAccumulator &da) {
   for (unsigned int i=0; i< get_number_of_triplet_containers(); ++i) {
     get_triplet_container(i)->apply(sm, da);
   }
 }
  template <class SM>
  void template_apply(const SM *sm) {
    for (unsigned int i=0; i< get_number_of_triplet_containers(); ++i) {
      get_triplet_container(i)->apply(sm);
    }
  }
  template <class SS>
  double template_evaluate(const SS *s,
                           DerivativeAccumulator *da) const {
    double ret=0;
    for (unsigned int i=0; i< get_number_of_triplet_containers(); ++i) {
      ret+=get_triplet_container(i)->evaluate(s, da);
    }
    return ret;
  }
  template <class SS>
    double template_evaluate_if_good(const SS *s,
                                 DerivativeAccumulator *da, double max) const {
    double ret=0;
    for (unsigned int i=0; i< get_number_of_triplet_containers(); ++i) {
      double cur=get_triplet_container(i)->evaluate_if_good(s, da, max);
      ret+=cur;
      max-=cur;
      if (max < 0) break;
    }
    return ret;
  }

  ParticlesTemp get_contained_particles() const;
  bool get_contents_changed() const {
    for (unsigned int i=0; i< get_number_of_triplet_containers(); ++i) {
      if (get_triplet_container(i)->get_contents_changed()) return true;
    }
    return false;
  }
  IMP_OBJECT(TripletContainerSet);

  /** @name Methods to control the nested container

      This container merges a set of nested containers. To add
      or remove nested containers, use the methods below.
  */
  /**@{*/
  IMP_LIST_ACTION(public, TripletContainer, TripletContainers,
                  triplet_container, triplet_containers,
                  TripletContainer*, TripletContainers,
                  {
                    obj->set_was_used(true);
                  },{},
                  );
  /**@}*/
  Restraints create_decomposition(TripletScore *ss) const;
#ifndef IMP_DOXYGEN
  bool get_is_up_to_date() const {
    for (unsigned int i=0;
         i< get_number_of_triplet_containers(); ++i) {
      if (!get_triplet_container(i)->get_is_up_to_date()) return false;
    }
    return true;
  }
  ParticleIndexTriplets get_indexes() const;
  ParticleIndexTriplets get_all_possible_indexes() const;
#endif
};


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_TRIPLET_CONTAINER_SET_H */
