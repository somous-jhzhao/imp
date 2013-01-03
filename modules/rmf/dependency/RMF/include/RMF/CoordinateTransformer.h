/**
 *  \file RMF/CoordinateTransformer.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_COORDINATE_TRANSFORMER_H
#define RMF_COORDINATE_TRANSFORMER_H

#include <RMF/config.h>
#include "internal/Transform.h"
#include "infrastructure_macros.h"

namespace RMF {
class ReferenceFrameConst;
/** Transform coordinates into the global reference frame from
    a nested one.
 */
class RMFEXPORT CoordinateTransformer {
  internal::Transform transform_;
public:
  /** Create one with the default reference frame, so the points aren't
      transformed at all.*/
  CoordinateTransformer() {
  };
  /** Compose an outer CoordinateTransformer with the nested ReferenceFrame.*/
  CoordinateTransformer(CoordinateTransformer base,
                        ReferenceFrameConst   rb);
  RMF_SHOWABLE(CoordinateTransformer, transform_);
  /** Return the global coordinates for a given set of internal coordinates.
   */
  Floats get_global_coordinates(const Floats& internal) const {
    return transform_.get_transformed(internal);
  }
};

typedef vector<CoordinateTransformer> CoordinateTransformers;

} /* namespace RMF */

#endif /* RMF_COORDINATE_TRANSFORMER_H */
