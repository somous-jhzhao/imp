/**
 *  \file MinimumQuadScore.cpp  \brief Define QuadScore
 *
 *  This file is generated by a script (core/tools/make-container).
 *  Do not edit directly.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#include <IMP/container/MinimumQuadScore.h>
#include "IMP/algebra/internal/MinimalSet.h"

IMPCONTAINER_BEGIN_NAMESPACE

MinimumQuadScore::MinimumQuadScore(const QuadScoresTemp &scores,
                                           unsigned int n,
                                           std::string name):
  QuadScore(name),
  scores_(scores),
  n_(n)
{
}


namespace {
  typedef algebra::internal::MinimalSet<double,
          QuadScore*, std::less<double> >
  MinimumQuadScoreMS;
  template <class It>
  MinimumQuadScoreMS
  find_minimal_set_MinimumQuadScore(It b, It e, const ParticleQuad& v,
                                          unsigned int n) {
    IMP_LOG(TERSE, "Finding Minimum " << n << " of "
            << std::distance(b,e) << std::endl);
    MinimumQuadScoreMS bestn(n);
    for (It it= b; it != e; ++it) {
      double score= (*it)->evaluate(v, NULL);
      bestn.insert(score, *it);
    }
    return bestn;
  }
}

double MinimumQuadScore::evaluate(const ParticleQuad& v,
                                      DerivativeAccumulator *da) const {
  MinimumQuadScoreMS bestn
    = find_minimal_set_MinimumQuadScore(scores_.begin(),
                                              scores_.end(), v, n_);

  double score=0;
  for (unsigned int i=0; i< bestn.size(); ++i) {
    if (da) {
      bestn[i].second->evaluate(v, da);
    }
    score+= bestn[i].first;
  }
  return score;
}

bool MinimumQuadScore::get_is_changed(const ParticleQuad& v) const {
  for (unsigned int i=0; i< scores_.size(); ++i) {
    if (scores_[i]->get_is_changed(v)) return true;
  }
  return false;
}


ParticlesTemp MinimumQuadScore
::get_input_particles(Particle* p) const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< scores_.size(); ++i) {
    ParticlesTemp c= scores_[i]->get_input_particles(p);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}

ContainersTemp MinimumQuadScore
::get_input_containers(Particle* p) const {
  ContainersTemp ret;
  for (unsigned int i=0; i< scores_.size(); ++i) {
    ContainersTemp c= scores_[i]->get_input_containers(p);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}


void MinimumQuadScore::do_show(std::ostream &out) const
{
  out << "size scores " << scores_.size() << std::endl;
}

IMPCONTAINER_END_NAMESPACE
