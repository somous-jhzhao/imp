/**
 *  \file IMP/QuadScore.h
 *  \brief Define QuadScore.
 *
 *  WARNING This file was generated from NAMEScore.hpp
 *  in tools/maintenance/container_templates/kernel
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_QUAD_SCORE_H
#define IMPKERNEL_QUAD_SCORE_H

#include "kernel_config.h"
#include "base_types.h"
#include "ParticleTuple.h"
#include "DerivativeAccumulator.h"
#include "internal/container_helpers.h"

IMP_BEGIN_NAMESPACE

//! Abstract score function
/** QuadScores will evaluate the score and derivatives
    for the passed particles. Use in conjunction with various
    restraints such as IMP::core::QuadsRestraint or
    IMP::core::QuadRestraint.

    Implementers should check out IMP_QUAD_SCORE().
*/
class IMPEXPORT QuadScore : public base::Object
{
 public:
  typedef ParticleQuad Argument;
  typedef ParticleIndexQuad IndexArgument;
  QuadScore(std::string name="QuadScore %1%");
  //! Compute the score and the derivative if needed.
  virtual double evaluate(const ParticleQuad& vt,
                          DerivativeAccumulator *da) const =0;

  //! Compute the score and the derivative if needed.
  virtual double evaluate_index(Model *m, const ParticleIndexQuad& vt,
                                DerivativeAccumulator *da) const {
    return evaluate(internal::get_particle(m, vt), da);
  }

  /** Implementations
      for these are provided by the IMP_QUAD_SCORE()
      macro.
  */
  virtual double evaluate_indexes(Model *m,
                                  const ParticleIndexQuads &o,
                                  DerivativeAccumulator *da) const {
    double ret=0;
    for (unsigned int i=0; i< o.size(); ++i) {
      ret+= evaluate_index(m, o[i], da);
    }
    return ret;
  }


  //! Compute the score and the derivative if needed.
  virtual double evaluate_if_good_index(Model *m,
                                        const ParticleIndexQuad& vt,
                                        DerivativeAccumulator *da,
                                        double max) const {
    IMP_UNUSED(max);
    return evaluate_index(m, vt, da);
  }

  /** Implementations
      for these are provided by the IMP_QUAD_SCORE()
      macro.
  */
  virtual double evaluate_if_good_indexes(Model *m,
                                          const ParticleIndexQuads &o,
                                          DerivativeAccumulator *da,
                                          double max) const {
    double ret=0;
    for (unsigned int i=0; i< o.size(); ++i) {
      double cur= evaluate_index(m, o[i], da);
      max-=cur;
      ret+=cur;
      if (max<0) break;
    }
    return ret;
  }

  /** Get the set of particles read when applied to the arguments. */
  virtual ParticlesTemp
    get_input_particles(Particle *p) const =0;

  /** Get the set of input containers when this modifier is applied to
      the arguments. */
  virtual ContainersTemp
    get_input_containers(Particle *p) const =0;

  /** Decompose this pair score acting on the pair into a set of
      restraints. The scoring function and derivatives should
      be equal to the current score. The defualt implementation
      just returns this object bound to the pair.*/
  Restraints create_current_decomposition(const ParticleQuad& vt) const;

  IMP_REF_COUNTED_DESTRUCTOR(QuadScore);
};

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_QUAD_SCORE_H */
