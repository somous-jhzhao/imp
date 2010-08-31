/**
 *  \file internal/singleton_helpers.h
 *  \brief A container for particles.
 *
 *  This file is generated by a script (core/tools/make-container).
 *  Do not edit directly.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_SINGLETON_HELPERS_H
#define IMPCORE_INTERNAL_SINGLETON_HELPERS_H

#include "../core_config.h"
#include <IMP/SingletonContainer.h>
#include <IMP/SingletonModifier.h>
#include <IMP/internal/container_helpers.h>
#include <algorithm>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

class IMPCOREEXPORT ListLikeSingletonContainer: public SingletonContainer {
private:
  Particles data_;
  mutable Particles index_;
  void update_index() const {
    if (index_.size()==data_.size()) return;
    unsigned int osize=index_.size();
    index_.insert(index_.end(), data_.begin()+osize, data_.end());
    std::sort(index_.begin()+osize, index_.end());
    std::inplace_merge(index_.begin(), index_.begin()+osize, index_.end());
  }
protected:
  ListLikeSingletonContainer *get_added() const {
    if (get_has_added_and_removed_containers()) {
      return dynamic_cast<ListLikeSingletonContainer*>
        (get_added_container());
    } else {
      return NULL;
    }
  }
  ListLikeSingletonContainer *get_removed() const {
    return dynamic_cast<ListLikeSingletonContainer*>
      (get_removed_container());
  }
  ListLikeSingletonContainer(){}
  void update_list(ParticlesTemp &cur) {
    IMP_IF_CHECK(USAGE) {
      for (unsigned int i=0; i< cur.size(); ++i) {
        IMP_USAGE_CHECK(
         IMP::internal::is_valid(cur[i]),
         "Passed Particle cannot be NULL (or None)");
      }
    }
    if (get_added()) {
      std::sort(cur.begin(), cur.end());
      ParticlesTemp added, removed;
      std::set_difference(cur.begin(), cur.end(),
                          data_.begin(), data_.end(),
                          std::back_inserter(added));
      std::set_difference(data_.begin(), data_.end(),
                          cur.begin(), cur.end(),
                          std::back_inserter(removed));
      get_added()->data_=added;
      get_removed()->data_=removed;
    }
    swap(data_, cur);
  }
  void add_to_list(ParticlesTemp &cur) {
    std::sort(cur.begin(), cur.end());
    ParticlesTemp added;
    std::set_difference(cur.begin(), cur.end(),
                        data_.begin(), data_.end(),
                        std::back_inserter(added));
    data_.insert(data_.end(), added.begin(), added.end());
    if (get_added()) {
      ListLikeSingletonContainer* ac=get_added();
      ac->data_.insert(ac->data_.end(), added.begin(), added.end());
    }
  }
  void remove_from_list(ParticlesTemp &cur) {
    index_.clear();
    std::sort(cur.begin(), cur.end());
    ParticlesTemp newlist;
    std::set_difference(data_.begin(), data_.end(),
                        cur.begin(), cur.end(),
                        std::back_inserter(newlist));
    swap(data_, newlist);
    if (get_has_added_and_removed_containers()) {
      ListLikeSingletonContainer* ac=get_removed();
      ac->data_.insert(ac->data_.end(), cur.begin(), cur.end());
    }
  }
  void add_to_list(Particle* cur) {
    data_.push_back(cur);
    if (get_added()) {
      ListLikeSingletonContainer* ac=get_added();
      ac->data_.push_back(cur);
    }
  }
  ListLikeSingletonContainer(Model *m, std::string name):
    SingletonContainer(m,name){
  }
  template <class F>
    F foreach(F f) const {
    return std::for_each(data_.begin(), data_.end(), f);
  }
public:
  IMP_SINGLETON_CONTAINER(ListLikeSingletonContainer);
  typedef Particles::const_iterator ParticleIterator;
  ParticleIterator particles_begin() const {
    return data_.begin();
  }
  ParticleIterator particles_end() const {
    return data_.end();
  }
  ObjectsTemp get_input_objects() const;
  void do_after_evaluate() {
    if (get_added()) {
      get_added()->data_.clear();
      get_removed()->data_.clear();
    }
  }
  void do_before_evaluate() {
    std::remove_if(data_.begin(), data_.end(),
         IMP::internal::IsInactive());
  }
  bool get_is_up_to_date() const {return true;}
  bool get_provides_access() const {return true;}
  const ParticlesTemp& get_access() const {
    IMP_INTERNAL_CHECK(get_is_up_to_date(),
                       "Container is out of date");
    return data_;
  }
};


IMPCORE_END_INTERNAL_NAMESPACE

#define IMP_LISTLIKE_SINGLETON_CONTAINER(Name)               \
  IMP_OBJECT(Name)


#endif  /* IMPCORE_INTERNAL_SINGLETON_HELPERS_H */
