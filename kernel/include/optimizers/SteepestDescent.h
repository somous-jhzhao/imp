/**
 *  \file SteepestDescent.h   \brief Simple steepest descent optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_STEEPEST_DESCENT_H
#define __IMP_STEEPEST_DESCENT_H

#include "../IMP_config.h"
#include "../Optimizer.h"
#include "../internal/kernel_version_info.h"

IMP_BEGIN_NAMESPACE

//! A simple steepest descent optimizer
 /** Follow the gradient based on the partial derivatives. Multiply by the
     current step size. If the score gets worse, reduce the step size.
     If the score gets better, increase the step size if we are sufficiently
     far from a score of zero. If the score reaches the threshold, quit.
     \ingroup optimizer
 */
class IMPDLLEXPORT SteepestDescent : public Optimizer
{
public:
  SteepestDescent();
  virtual ~SteepestDescent();

  IMP_OPTIMIZER(internal::kernel_version_info)

  //! Set the minimum gradient threshold
  void set_threshold(Float t) {threshold_=t;}
  //! Set the step size
  void set_step_size(Float t) {step_size_=t;}

protected:
  Float step_size_;
  Float threshold_;
};

IMP_END_NAMESPACE

#endif  /* __IMP_STEEPEST_DESCENT_H */
