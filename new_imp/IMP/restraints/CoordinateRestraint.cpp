/*
 *  \file CoordianteRestraint.cpp   Optimize based on distance from an
 *                                  absolute position.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "../Model.h"
#include "../Particle.h"
#include "../log.h"
#include "CoordinateRestraint.h"

namespace imp
{

/**
  Constructor - set up the values and indexes for this coordinate restraint.

  \param[in] model Pointer to the model.
  \param[in] p1 Pointer to particle of the restraint.
  \param[in] axis String indicating the axis of absolute reference:
      X_AXIS, Y_AXIS, Z_AXIS, XY_RADIAL, XZ_RADIAL, YZ_RADIAL, XYZ_SPHERE
  \param[in] score_func_params Parameters for creating a score function.
 */

CoordinateRestraint::CoordinateRestraint(Model& model,
                               Particle* p1,
                               const std::string axis,
                               BasicScoreFuncParams* score_func_params)
{
  model_data_ = model.get_model_data();

  particles_.push_back(p1);
  x1_ = p1->float_index(std::string("X"));
  y1_ = p1->float_index(std::string("Y"));
  z1_ = p1->float_index(std::string("Z"));

  axis_ = axis;
  score_func_ = score_func_params->create_score_func();
}

/**
  Destructor
 */

CoordinateRestraint::~CoordinateRestraint ()
{
}

/**
  Calculate the score for this coordinate restraint based on the current
  state of the model.

 \param[in] calc_deriv If true, partial first derivatives should be calculated.
  */

Float CoordinateRestraint::evaluate(bool calc_deriv)
{
  Float score = 0.0, deriv;
  Float x, y, z;
  Float dx = 0.0;
  Float dy = 0.0;
  Float dz = 0.0;
  Float current_distance;

  // get current position of particle
  x = model_data_->get_float(x1_);
  y = model_data_->get_float(y1_);
  z = model_data_->get_float(z1_);

  // restrain the x coordinate
  if (axis_ == "X_AXIS") {
    score = (*score_func_)(x, dx);
  }

  // restrain the y coordinate
  else if (axis_ == "Y_AXIS") {
    score = (*score_func_)(y, dy);
  }

  // restrain the z coordinate
  else if (axis_ == "Z_AXIS") {
    score = (*score_func_)(z, dz);
  }

  // restrain the xy radial distance
  else if (axis_ == "XY_RADIAL") {
    current_distance = sqrt(x * x + y * y);
    score = (*score_func_)(sqrt(x * x + y * y), deriv);
    dx = x / current_distance * deriv;
    dy = y / current_distance * deriv;
  }

  // restrain the xz radial distance
  else if (axis_ == "XZ_RADIAL") {
    current_distance = sqrt(x * x + z * z);
    score = (*score_func_)(current_distance, deriv);
    dx = x / current_distance * deriv;
    dz = z / current_distance * deriv;
  }

  // restrain the yz radial distance
  else if (axis_ == "YZ_RADIAL") {
    current_distance = sqrt(y * y + z * z);
    score = (*score_func_)(current_distance, deriv);
    dy = y / current_distance * deriv;
    dz = z / current_distance * deriv;
  }

  // restrain the xyz spherical distance (Euclidian distance from origin)
  else if (axis_ == "XYZ_SPHERE") {
    current_distance = sqrt(x * x + y * y + z * z);
    score = (*score_func_)(current_distance, deriv);
    dx = x / current_distance * deriv;
    dy = y / current_distance * deriv;
    dz = z / current_distance * deriv;
  }

  // if needed, use the partial derivatives
  if (calc_deriv) {
    model_data_->add_to_deriv(x1_, dx);
    model_data_->add_to_deriv(y1_, dy);
    model_data_->add_to_deriv(z1_, dz);
  }

  IMP_LOG(VERBOSE, axis_ << " score: " << score << "  x: " << x << " y: " << y << " z: " << z << "   dx: " << dx << " dy: " << dy << " dz: " << dz << std::endl);

  return score;
}


/**
  Show the current restraint.

 \param[in] out Stream to send restraint description to.
 */

void CoordinateRestraint::show(std::ostream& out) const
{
  if (is_active()) {
    out << "coordinate restraint (active):" << std::endl;
  } else {
    out << "coordinate restraint (inactive):" << std::endl;
  }

  out << "version: " << version() << "  " << "last_modified_by: " << last_modified_by() << std::endl;
  out << "  x1:" << model_data_->get_float(x1_);
  out << "  y1:" << model_data_->get_float(y1_);
  out << "  z1:" << model_data_->get_float(z1_) << std::endl;

  out << "  dx1:" << model_data_->get_deriv(x1_);
  out << "  dy1:" << model_data_->get_deriv(y1_);
  out << "  dz1:" << model_data_->get_deriv(z1_) << std::endl;

  out << "  axis:" << axis_;
}

}  // namespace imp
