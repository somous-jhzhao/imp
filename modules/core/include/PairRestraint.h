/**
 *  \file PairRestraint.h
 *  \brief Apply a PairScore to a ParticlePair.
 *
 *  This file is generated by a script (core/tools/make-container).
 *  Do not edit directly.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_PAIR_RESTRAINT_H
#define IMPCORE_PAIR_RESTRAINT_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/Restraint.h>
#include <IMP/Pointer.h>
#include <IMP/PairScore.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Applies a PairScore to a ParticlePair.
/** This restraint stores a ParticlePair.
    \see PairRestraint
 */
class IMPCOREEXPORT PairRestraint : public Restraint
{
  Pointer<PairScore> ss_;
  ParticlePair v_;
public:
  //! Create the restraint.
  /** This function takes the function to apply to the
      stored Pair and the Pair.
   */
  PairRestraint(PairScore *ss,
                     Particle *a, Particle *b,
                     std::string name="PairRestraint %1%");

  IMP_RESTRAINT(PairRestraint, internal::version_info);

  virtual ParticlesList get_interacting_particles() const;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_PAIR_RESTRAINT_H */
