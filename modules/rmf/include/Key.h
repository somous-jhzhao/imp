/**
 *  \file IMP/rmf/Key.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_KEY_H
#define IMPRMF_KEY_H

#include "rmf_config.h"
#include "KeyCategory.h"
#include "hdf5_types.h"
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
IMPRMF_BEGIN_INTERNAL_NAMESPACE
class SharedData;

IMPRMF_END_INTERNAL_NAMESPACE
#endif

IMPRMF_BEGIN_NAMESPACE

/** A key referencing a particular piece of data */
template <class TypeTraitsT>
class Key {
  friend class RootHandle;
  friend class NodeHandle;
  friend class internal::SharedData;
  int i_;
  KeyCategory ci_;
  bool pf_;
  Key(KeyCategory category_id,
      int i, bool pf): i_(i), ci_(category_id),
                       pf_(pf) {}
  int get_index() const {
    return i_;
  }
  bool get_is_per_frame() const {
    return pf_;
  }
  KeyCategory get_category() const {
    return ci_;
  }
public:
  typedef TypeTraitsT TypeTraits;
  Key(): i_(-1), ci_(), pf_(false) {}
  void show(std::ostream &out) const {
    out << ci_.get_name() << (pf_?'E':'I') << i_;
  }
  IMP_COMPARISONS_2(Key, ci_, i_);
  IMP_HASHABLE_INLINE(Key, return i_*ci_.get_index());
};
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
template <class TypeTraits>
std::ostream &operator<<(std::ostream &out, Key<TypeTraits> k) {
  out << k;
  return out;
}
#endif

//! Store an arbitrary floating point value
typedef Key<FloatTraits> FloatKey;
IMP_VALUES(FloatKey, FloatKeys);
//! Store an abitrary integer value
typedef Key<IntTraits> IntKey;
IMP_VALUES(IntKey, IntKeys);
//! Store an arbitrary string value
typedef Key<StringTraits> StringKey;
IMP_VALUES(StringKey, StringKeys);
//! Store a zero-based index to something
typedef Key<IndexTraits> IndexKey;
IMP_VALUES(IndexKey, IndexKeys);
//! Store the ID of an existing node in the RMF file
typedef Key<NodeIDTraits> NodeIDKey;
IMP_VALUES(NodeIDKey, NodeIDKeys);
//! Store the name of a data set in the hdf5 file
/** The handling of this data type may change as
    it is currently a bit limited.
*/
typedef Key<DataSetTraits> DataSetKey;
IMP_VALUES(DataSetKey, DataSetKeys);


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_KEY_H */
