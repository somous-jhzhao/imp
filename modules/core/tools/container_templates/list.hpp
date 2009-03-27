/**
 *  \file ListGroupnameContainer.h    \brief Store a list of Classnames
 *
 *  This file is generated by a script (core/tools/make-container).
 *  Do not edit directly.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_LIST_GROUPNAME_CONTAINER_H
#define IMPCORE_LIST_GROUPNAME_CONTAINER_H

#include "config.h"
#include "internal/version_info.h"
#include <IMP/GroupnameContainer.h>
#include <IMP/container_macros.h>

IMPCORE_BEGIN_NAMESPACE

//! Store a list of Classnames
/** \note The indexes can change when particles are inserted
    as the list is maintained in sorted order.
 */
class IMPCOREEXPORT ListGroupnameContainer : public GroupnameContainer
{
public:
  //! construct and pass an initial set of classnames
  ListGroupnameContainer(const Classnames &ps= Classnames());
 /** @name Methods to control the contained objects

     This container stores a list of Classname objects. To manipulate
     the list use these methods.
  */
  /**@{*/
  IMP_LIST(public, Classname, classname, Value);
  /**@}*/

  //! log n time
  virtual bool get_contains_classname(Value vt) const;

  IMP::VersionInfo get_version_info() const {
    return internal::version_info;
  }

  virtual void show(std::ostream &out = std::cout) const;

  // for some reason swig gets this wrong
  //IMP_REF_COUNTED_DESTRUCTOR(ListGroupnameContainer)
  ~ListGroupnameContainer(){}
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_LIST_GROUPNAME_CONTAINER_H */
