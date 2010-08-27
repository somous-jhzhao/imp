/**
 *  \file AllBipartitePairContainer.h
 *  \brief Return all pairs from a SingletonContainer
 *
 *  This file is generated by a script (core/tools/make-container).
 *  Do not edit directly.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_ALL_BIPARTITE_PAIR_CONTAINER_H
#define IMPCONTAINER_ALL_BIPARTITE_PAIR_CONTAINER_H

#include "container_config.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/container/PairContainerSet.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Return all bipartite pairs between two containers
/** \see AllPairContainer

    \usesconstraint
 */
class IMPCONTAINEREXPORT AllBipartitePairContainer : public PairContainer
{
  IMP::internal::OwnerPointer<SingletonContainer> a_, b_;
  friend class AllPairContainer;
  IMP_CONTAINER_DEPENDENCIES(AllBipartitePairContainer,
                             {
                               ret.push_back(back_->a_);
                               ret.push_back(back_->b_);
                             });
  AllBipartitePairContainer(SingletonContainer *a,
                                 SingletonContainer *b, bool);
  template <class F>
    F foreach(F f) const {
    unsigned int sza=a_->get_number_of_particles();
    unsigned int szb=b_->get_number_of_particles();
    for (unsigned int i=0; i< sza; ++i) {
      Particle *a= a_->get_particle(i);
      for (unsigned int j=0; j< szb; ++j) {
        ParticlePair p(a, b_->get_particle(j));
        f(p);
      }
    }
    return f;
  }
public:
  AllBipartitePairContainer(SingletonContainer *a,
                                 SingletonContainer *b);

  static AllBipartitePairContainer *
    create_untracked_container(SingletonContainer *a,
                               SingletonContainer *b) {
    AllBipartitePairContainer *lsc
      = new AllBipartitePairContainer(a,b, false);
    return lsc;
  }
#ifndef IMP_DOXYGEN
  bool get_is_up_to_date() const {
    return a_->get_is_up_to_date()
      && b_->get_is_up_to_date();
  }
#endif
  IMP_PAIR_CONTAINER(AllBipartitePairContainer);
};

IMP_OBJECTS(AllBipartitePairContainer,AllBipartitePairContainers);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_ALL_BIPARTITE_PAIR_CONTAINER_H */
