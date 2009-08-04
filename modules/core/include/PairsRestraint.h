/**
 *  \file PairsRestraint.h
 *  \brief Apply a PairScore to each ParticlePair in a list.
 *
 *  This file is generated by a script (core/tools/make-container).
 *  Do not edit directly.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_PAIRS_RESTRAINT_H
#define IMPCORE_PAIRS_RESTRAINT_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/Restraint.h>
#include <IMP/Pointer.h>
#include <IMP/PairScore.h>
#include <IMP/PairContainer.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Applies a PairScore to each ParticlePair in a list.
/** This restraint stores the used particles in a ParticlePairs.
    The container used can be set so that the list can be shared
    with other containers (or a nonbonded list can be used). By default
    a ListPairContainer is used and the
    {add_, set_, clear_}particle_pair{s} methods can be used.

    \see PairRestraint
 */
class IMPCOREEXPORT PairsRestraint : public Restraint
{
  Pointer<PairScore> ss_;
  Pointer<PairContainer> pc_;
public:
  //! Create the restraint.
  /** \param[in] ss The function to apply to each particle.
      \param[in] ps The list of particles to use in the restraint.
   */
  PairsRestraint(PairScore *ss,
                      const ParticlePairs &ps=ParticlePairs(),
                      std::string name="PairsRestraint %1%");

 //! Create the restraint with a shared container
  /** \param[in] ss The function to apply to each particle.
      \param[in] pc The container containing the stored particles. This
      container is not copied.
   */
  PairsRestraint(PairScore *ss,
                      PairContainer *pc);

  IMP_RESTRAINT(PairsRestraint, internal::version_info);

  /** \note This can only be used if the container is a ListPairContainer
  */
  void add_particle_pairs(const ParticlePairs &ps);
  /** \note This can only be used if the container is a ListPairContainer
   */
  void set_particle_pairs(const ParticlePairs &ps);
  /** \note This can only be used if the container is a ListPairContainer
  */
  void add_particle_pair(ParticlePair v);
  /** \note This can only be used if the container is a ListPairContainer
  */
  void clear_particle_pairs();

  //! Get the container used to store Particles
  PairContainer* get_pair_container() {
    return pc_;
  }
  //! Set the container used to store Particles
  /** \note The passed container is used, not a copy.
   */
  void set_pair_container(PairContainer *pc) {
    pc_=pc;
  }

  virtual ParticlesList get_interacting_particles() const;

  PairScore* get_pair_score() const {
    return ss_;
  }

  void set_pair_score(PairScore* ss) {
    IMP_check(ss, "The PairScore must be non-null",
              IndexException);
    ss_= ss;
  }
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_PAIRS_RESTRAINT_H */
