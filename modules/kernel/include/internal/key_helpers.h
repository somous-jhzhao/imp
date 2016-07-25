/**
 *  \file key_helpers.h    \brief helpers for declaring keys
 *  such as classes KeyData, KeyTable and method get_key_data()
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_KEY_HELPERS_H
#define IMPKERNEL_INTERNAL_KEY_HELPERS_H

#include <IMP/kernel_config.h>
#include <IMP/exception.h>
#include <IMP/check_macros.h>
#include <boost/unordered_map.hpp>
#include <IMP/Vector.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE
/** \internal The data concerning keys (strings and corresponding numbers).
  */
struct IMPKERNELEXPORT KeyData {
  //! map from key string to key number (many:1)
  typedef boost::unordered_map<std::string, int> Map;

  //! reverse map - from key number to one representative of the key
  //! strings that map to it (1:1)
  typedef IMP::Vector<std::string> RMap;

  void show(std::ostream &out = std::cout) const;

  //! ID - the key ID (as in the template of Key)
  KeyData(unsigned int ID);

  void assert_is_initialized() const;

  //! Add key with string "str", return its number
  //! Note is is assumed that str wasn't a key already
  //! and this would override the old "str" key in a way that's not completely expected
  //! (TODO: add checks if str is already in map, to fix rmap or to use the old number?)
  unsigned int add_key(std::string str) {
    unsigned int i = rmap_.size();
    map_[str] = i;
    rmap_.push_back(str);
    return i;
  }

  //! add alias "str" to existing key number i (the alias is kept
  //! in map but not in the reverse map rmap)
  unsigned int add_alias(std::string str, unsigned int i) {
    IMP_INTERNAL_CHECK(rmap_.size() > i, "The aliased key doesn't exist");
    map_[str] = i;
    return i;
  }

  //! return a map from key string to key number (many:1)
  const Map &get_map() const { return map_; }

  //! return a reverse-map from key number to one representative of
  //! the key strings that map to it (1:1)
  const RMap &get_rmap() const { return rmap_; }

 private:
  double heuristic_;
  Map map_;
  RMap rmap_;
};



IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_KEY_HELPERS_H */
