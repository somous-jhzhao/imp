/**
 *  \file MinimumQuadRestraint.h
 *  \brief Score based on the minimum score over a set of Quads
 *
 *  This file is generated by a script (core/tools/make-container).
 *  Do not edit directly.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_MAXIMUM_QUAD_RESTRAINT_H
#define IMPCONTAINER_MAXIMUM_QUAD_RESTRAINT_H

#include "container_config.h"
#include <IMP/Restraint.h>
#include <IMP/QuadScore.h>
#include <IMP/QuadContainer.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Score based on the maximum quad over a set of Quads
/** The score is evaluated for each of the Quads in the container
    and the value of the maximum n scores is used. That is,
    if n is 1, the value of the restraint is the value of the maximum
    score over the container.
 */
class IMPCONTAINEREXPORT MaximumQuadRestraint
: public Restraint
{
  IMP::internal::OwnerPointer<QuadScore> f_;
  IMP::internal::OwnerPointer<QuadContainer> c_;
  unsigned int n_;
public:
  /** n is the number of maximum scores to use.
   */
  MaximumQuadRestraint(QuadScore *f,
                                 QuadContainer *c,
                                 unsigned int n=1,
                                 std::string name
                                 ="MaximumQuadRestraint %1%");

  IMP_RESTRAINT(MaximumQuadRestraint);

  //! Set the number of lowest scores to use.
  void set_n(unsigned int n) { n_=n;}
};

IMP_OBJECTS(MaximumQuadRestraint,MaximumQuadRestraints);


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_MAXIMUM_QUAD_RESTRAINT_H */
