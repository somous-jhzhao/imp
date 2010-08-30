/**
 *  \file TripletFilter.cpp   \brief Filter for particle_triplet.
 *
 *  This file is generated by a script (core/tools/make-filter).
 *  Do not edit directly.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/container/InContainerTripletFilter.h"

IMPCONTAINER_BEGIN_NAMESPACE


InContainerTripletFilter
::InContainerTripletFilter(TripletContainer *c,
                             std::string name): TripletFilter(name), c_(c){}

ParticlesTemp InContainerTripletFilter
::get_input_particles(const ParticleTriplet&) const {
  // not quite right, but...
  return ParticlesTemp();
}
ContainersTemp InContainerTripletFilter
::get_input_containers(const ParticleTriplet&) const {
  return ContainersTemp(1,c_);
}

void InContainerTripletFilter::do_show(std::ostream &out) const {
  out << "InContainerTripletFilter on " << *c_ << std::endl;
}

IMPCONTAINER_END_NAMESPACE
