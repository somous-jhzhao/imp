/**
 *  \file CMMLogOptimizerState.h
 *  \brief A state which writes a series of CMM files.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <fstream>
#include <sstream>

#include "IMP/optimizers/states/CMMLogOptimizerState.h"
#include "IMP/decorators/XYZDecorator.h"

IMP_BEGIN_NAMESPACE

CMMLogOptimizerState::CMMLogOptimizerState(std::string filename,
                                             const Particles &pis) :
    pis_(pis), filename_(filename), file_number_(0), call_number_(0),
    skip_steps_(0)
{
}

void CMMLogOptimizerState::update()
{
  if (skip_steps_ == 0 || (call_number_ % skip_steps_) == 0) {
    write_next_file();
  }
  ++call_number_;
}

void CMMLogOptimizerState::write_next_file()
{
  char buf[1000];
  sprintf(buf, filename_.c_str(), file_number_);
  ++file_number_;
  write(buf);
}


void CMMLogOptimizerState::write(std::string buf) const
{
  std::ofstream out(buf.c_str());
  if (!out) {
    IMP_WARN("Error opening CMM log file " << buf);
  } else {
    IMP_LOG(VERBOSE, "Writing " << pis_.size()
            << " particles to file " << buf << "..." << std::flush);
    std::stringstream filename;
    filename << "optimization_step_" << call_number_;
    write(pis_, filename.str() ,radius_, r_, g_, b_, out);
    //IMP_LOG(TERSE, "done" << std::endl);
  }
}

static Float snap(Float f)
{
  if (f < 0) return 0;
  if (f > 1) return 1;
  return f;
}

void CMMLogOptimizerState::write(const Particles &pis,
                                 const std::string &marker_set_name,
                                 FloatKey radius_key, FloatKey r_key,
                                 FloatKey g_key, FloatKey b_key,
                                 std::ostream &out)
{
  out << "<marker_set name=\"" <<marker_set_name << "\">"<<std::endl;
  for (unsigned int i = 0; i < pis.size(); ++i) {
    try {
      Particle *p = pis[i];
      XYZDecorator xyz = XYZDecorator::cast(p);
      float x = xyz.get_x();
      float y = xyz.get_y();
      float z = xyz.get_z();
      Float rv = 0., gv = 0., bv = 0.;
      if (r_key != FloatKey() && b_key != FloatKey() && g_key != FloatKey()
          && p->has_attribute(r_key) && p->has_attribute(g_key)
          && p->has_attribute(b_key)) {
        rv = snap(p->get_value(r_key));
        gv = snap(p->get_value(g_key));
        bv = snap(p->get_value(b_key));
      }
      Float radius = 5.;
      if (radius_key != FloatKey() && p->has_attribute(radius_key)) {
        radius = p->get_value(radius_key);
      }
      out << "<marker id=\"" << i << "\""
          << " x=\"" << x << "\""
          << " y=\"" << y << "\""
          << " z=\"" << z << "\""
          << " radius=\"" << radius << "\""
          << " r=\"" << rv << "\""
          << " g=\"" << gv << "\""
          << " b=\"" << bv <<  "\"/>" << std::endl;
    }

    catch (InvalidStateException &e) {
      IMP_WARN("Particle " << pis[i] << " does not have "
               << " cartesian coordinates");
    }
  }
  out << "</marker_set>" << std::endl;
}


void CMMLogOptimizerState::show(std::ostream &out) const
{
  out << "Writing CMM files " << filename_ << std::endl;
}

IMP_END_NAMESPACE
