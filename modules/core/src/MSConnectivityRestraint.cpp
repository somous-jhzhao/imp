/**
 *  \file MSConnectivityRestraint.cpp  \brief Mass Spec Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types. It also handles multiple copies of the same particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <algorithm>

#include <IMP/core/MSConnectivityRestraint.h>

#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/log.h>
#include <IMP/PairScore.h>
#include <IMP/core/PairRestraint.h>
#include <IMP/core/internal/CoreListSingletonContainer.h>

#include <climits>

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/assert.hpp>

#include <limits>

IMPCORE_BEGIN_NAMESPACE


bool MSConnectivityRestraint::ExperimentalTree::is_consistent(
  size_t node_index) const
{
  const Node *node = get_node(node_index);
  const Node::Label &label = node->get_label();
  // the node must have fewer proteins than each of its parents
  for ( size_t i = 0; i < node->get_number_of_parents(); ++i )
  {
    const Node *parent = get_node(node->get_parent(i));
    const Node::Label &parent_label = parent->get_label();
    size_t pi = 0, ci = 0;
    while ( ci < label.size() && pi < parent_label.size() )
    {
      // skip proteins that the parent has but the child does not have
      while ( pi < parent_label.size() &&
              parent_label[pi].first < label[ci].first )
        ++pi;
      if ( pi == parent_label.size() || parent_label[pi].first
              != label[ci].first )
      {
        // this means the child has more proteins than parent,
        // example: parent = "AA", child = "AAB"
        return false;
      }
      if ( parent_label[pi].second < label[ci].second )
      {
        // this means the child has more instances of a given protein,
        // example: parent = "AA", child = "AAA"
        return false;
      }
      ++pi;
      ++ci;
    }
    // last check - child has to have at least one fewer proteins
    // than every parent
    int parent_total = 0, child_total = 0;
    for ( size_t j = 0; j < label.size(); ++j )
      child_total += label[j].second;
    for ( size_t j = 0; j < parent_label.size(); ++j )
      parent_total += parent_label[j].second;
    if ( child_total >= parent_total )
      return false;
  }
  return true;
}


void MSConnectivityRestraint::ExperimentalTree::connect(
  size_t parent, size_t child)
{
  if ( finalized_ )
    IMP_THROW("Cannot add new edges to finalized tree", IMP::ValueException);
  nodes_[parent].children_.push_back(child);
  nodes_[child].parents_.push_back(parent);
}


void MSConnectivityRestraint::ExperimentalTree::finalize()
{
  if ( finalized_ )
    return;
  for ( size_t i=0; i<nodes_.size(); ++i )
  {
    if ( nodes_[i].is_root() )
    {
      if ( root_ == size_t(-1) )
        root_ = i;
      else
        IMP_THROW("Experimental tree has multiple roots", IMP::ValueException);
    }
  }
  if ( find_cycle(root_) )
    IMP_THROW("Experimental tree has a cycle", IMP::ValueException);
  for ( size_t i = 0; i < nodes_.size(); ++i )
    if ( !is_consistent(i) )
    {
      IMP_THROW("Experimental tree is inconsistent: a child has to "
        "have fewer proteins than its parent", IMP::ValueException);
    }
  finalized_ = true;
}


bool MSConnectivityRestraint::ExperimentalTree::find_cycle(size_t node_index)
{
  Node &node = nodes_[node_index];
  if ( node.visited_ )
    return true;
  node.visited_ = true;
  bool cycle_found = false;
  for ( size_t i = 0; i < node.get_number_of_children(); ++i )
    if ( find_cycle(node.get_child(i)) )
    {
      cycle_found = true;
      break;
    }
  node.visited_ = false;
  return cycle_found;
}


size_t MSConnectivityRestraint::ExperimentalTree::add_composite(
  const Ints &components)
{
  if ( finalized_ )
    IMP_THROW("Cannot add new nodes to finalized tree", IMP::ValueException);
  Node node;
  desc_to_label(components, node.label_);
  size_t idx = nodes_.size();
  nodes_.push_back(node);
  return idx;
}


size_t MSConnectivityRestraint::ExperimentalTree::add_composite(
  const Ints &components, size_t parent)
{
  size_t child = add_composite(components);
  connect(parent, child);
  return child;
}


void MSConnectivityRestraint::ExperimentalTree::desc_to_label(
    const Ints &components, Node::Label &label)
{
  label.clear();
  Ints sorted_components(components);
  std::sort(sorted_components.begin(), sorted_components.end());
  for ( size_t i=0; i<sorted_components.size(); ++i )
  {
    size_t id = sorted_components[i];
    if ( label.empty() || label.back().first != id )
      label.push_back(std::make_pair(id, 1));
    else
      label.back().second++;
  }
}



MSConnectivityRestraint::MSConnectivityRestraint(PairScore *ps, double eps):
  Restraint("MSConnectivityRestraint %1%"),
  ps_(ps),
  eps_(eps)
{
}



size_t MSConnectivityRestraint::ParticleMatrix::add_type(const Particles &ps)
{
  protein_by_class_.push_back(std::vector<size_t>());
  for ( size_t i = 0; i < ps.size(); ++i )
  {
    size_t n = particles_.size();
    particles_.push_back(ParticleData(ps[i], current_id_));
    protein_by_class_.back().push_back(n);
  }
  return current_id_++;
}


void MSConnectivityRestraint::ParticleMatrix::create_distance_matrix(
  const PairScore *ps)
{
  size_t n = size();
  dist_matrix_.resize(n*n);
  for ( size_t r = 0; r < n; ++r )
  {
    dist_matrix_[r*n + r] = 0;
    for ( size_t c = r + 1; c < n; ++ c )
    {
      double d = ps->evaluate(ParticlePair(particles_[r].get_particle(),
        particles_[c].get_particle()), 0);
      dist_matrix_[r*n + c] = dist_matrix_[c*n + r] = d;
      min_distance_ = std::min(min_distance_, d);
      max_distance_ = std::max(max_distance_, d);
    }
  }
  order_.clear();
  order_.resize(n);
  for ( size_t i = 0; i < n; ++i )
  {
    for ( size_t j = 0; j < n; ++j )
      if ( j != i )
        order_[i].push_back(j);
    std::sort(order_[i].begin(), order_[i].end(), DistCompare(i, *this));
  }
}


typedef boost::property<boost::edge_weight_t, double> EdgeWeight;
typedef boost::property<boost::vertex_name_t, size_t> VertexId;
typedef boost::adjacency_list<boost::setS, boost::vecS,
        boost::undirectedS, VertexId, EdgeWeight> NNGraph;


class Tuples
{
public:
  Tuples(const std::vector<size_t> &elements, size_t k)
    : current_tuple_(k)
    , indices_(k)
    , elements_(elements)
    , k_(k)
    , n_(elements_.size())
    , empty_(k_ > n_)
  {
    reset();
  }

  std::vector<size_t> const &get_tuple() const
  {
    return current_tuple_;
  }

  bool next();

  void reset();

  bool empty() const
  {
    return empty_;
  }

private:
  void create_current_tuple();

  std::vector<size_t> current_tuple_;
  std::vector<size_t> indices_;
  std::vector<size_t> elements_;
  size_t k_;
  size_t n_;
  bool empty_;
};


bool Tuples::next()
{
  if ( empty_ )
    return false;
  size_t i = k_;
  bool found = false;
  while ( i > 0 )
  {
    --i;
    if ( indices_[i] != i + n_ - k_ )
    {
      found = true;
      break;
    }
  }
  if ( found )
  {
    ++indices_[i];
    for ( size_t j = i + 1; j < k_; ++j )
      indices_[j] = indices_[j - 1] + 1;
    create_current_tuple();
    return true;
  }
  return false;
}


void Tuples::create_current_tuple()
{
  if ( !empty_ )
  {
    for ( size_t i = 0; i < k_; ++i )
      current_tuple_[i] = elements_[indices_[i]];
  }
}


void Tuples::reset()
{
  for ( size_t i = 0; i < k_; ++i )
    indices_[i] = i;
  create_current_tuple();
}


class Assignment
{
public:
  Assignment(std::vector<Tuples> &tuples)
    : tuples_(tuples)
  {
  }

  Tuples const &operator[](size_t i) const
  {
    return tuples_[i];
  }

  bool empty() const
  {
    for ( size_t i = 0; i < tuples_.size(); ++i )
      if ( ! tuples_[i].empty() )
        return false;
    return true;
  }

  size_t size() const
  {
    return tuples_.size();
  }

  bool next();

private:
  std::vector<Tuples> &tuples_;
};


bool Assignment::next()
{
  for ( size_t i = 0; i < tuples_.size(); ++i )
  {
    if ( tuples_[i].empty() )
      continue;
    if ( tuples_[i].next() )
      return true;
    tuples_[i].reset();
  }
  return false;
}

bool is_connected(NNGraph &G)
{
  std::vector<int> components(num_vertices(G));
  return boost::connected_components(G, &components[0]) == 1;
}


typedef std::set< std::pair<size_t, size_t> > EdgeSet;


class MSConnectivityScore
{
public:
  MSConnectivityScore(const MSConnectivityRestraint::ExperimentalTree &tree,
    const PairScore *ps, double eps,
    MSConnectivityRestraint &restraint);
  double score(DerivativeAccumulator *accum) const;
  EdgeSet get_connected_pairs() const;
  Particle *get_particle(size_t p) const
  {
    return restraint_.particle_matrix_.get_particle(p).get_particle();
  }
  void add_edges_to_set(NNGraph &G, EdgeSet &edge_set) const;
  EdgeSet get_all_edges(NNGraph &G) const;

private:

  struct EdgeScoreComparator
  {
    EdgeScoreComparator(const MSConnectivityRestraint &restraint)
      : restraint_(restraint)
    {}

    bool operator()(const std::pair<size_t, size_t> &p1,
        const std::pair<size_t, size_t> &p2)
    {
      double w1 = restraint_.particle_matrix_.get_distance(p1.first, p1.second);
      double w2 = restraint_.particle_matrix_.get_distance(p2.first, p2.second);
      return w1 < w2;
    }

    const MSConnectivityRestraint &restraint_;
  };

  NNGraph create_nn_graph(double threshold) const;
  NNGraph build_subgraph_from_assignment(NNGraph &G,
    Assignment const &assignment) const;
  bool check_assignment(NNGraph &G, size_t node_handle,
    Assignment const &assignment,
    EdgeSet &picked) const;
  bool perform_search(NNGraph &G, EdgeSet &picked) const;
  NNGraph pick_graph(EdgeSet const &picked) const;
  NNGraph find_threshold() const;

  MSConnectivityRestraint &restraint_;
  const PairScore *ps_;
  const MSConnectivityRestraint::ExperimentalTree &tree_;
  double eps_;
};


EdgeSet MSConnectivityScore::get_all_edges(NNGraph &G) const
{
  boost::property_map<NNGraph, boost::vertex_name_t>::type vertex_id =
    boost::get(boost::vertex_name, G);
  EdgeSet result;
  NNGraph::edge_iterator e, end;
  for ( boost::tie(e, end) = edges(G); e != end; ++e )
  {
    size_t src = boost::get(vertex_id, source(*e, G));
    size_t dst = boost::get(vertex_id, target(*e, G));
    if ( src > dst )
      std::swap(src, dst);
    result.insert(std::make_pair(src, dst));
  }
  return result;
}


void MSConnectivityScore::add_edges_to_set(NNGraph &G, EdgeSet &edge_set) const
{
  boost::property_map<NNGraph, boost::vertex_name_t>::type vertex_id =
    boost::get(boost::vertex_name, G);
  NNGraph ng(num_vertices(G));
  std::vector<size_t> vertex_id_to_n(restraint_.particle_matrix_.size(), -1);
  for ( size_t i = 0; i < num_vertices(ng); ++i )
  {
    size_t id = boost::get(vertex_id, i);
    vertex_id_to_n[id] = i;
  }
  for ( EdgeSet::iterator p = edge_set.begin(); p != edge_set.end(); ++p )
  {
    size_t i_from = vertex_id_to_n[(*p).first];
    size_t i_to = vertex_id_to_n[(*p).second];
    add_edge(i_from, i_to, ng);
  }
  std::vector<int> components(num_vertices(ng));
  int ncomp = boost::connected_components(ng, &components[0]);
  if ( ncomp == 1 )
    return;
  std::vector< std::pair<size_t, size_t> > candidates;
  NNGraph::edge_iterator e, end;
  for ( boost::tie(e, end) = edges(G); e != end; ++e )
  {
    size_t src = boost::get(vertex_id, source(*e, G));
    size_t dst = boost::get(vertex_id, target(*e, G));
    if ( src > dst )
      std::swap(src, dst);
    std::pair<size_t, size_t> candidate = std::make_pair(src, dst);
    if ( edge_set.find(candidate) == edge_set.end() )
      candidates.push_back(candidate);
  }
  std::sort(candidates.begin(), candidates.end(),
      EdgeScoreComparator(restraint_));
  size_t idx = 0;
  while ( ncomp > 1 && idx < candidates.size() )
  {
    size_t i_from = vertex_id_to_n[candidates[idx].first];
    size_t i_to = vertex_id_to_n[candidates[idx].second];
    if ( components[i_from] != components[i_to] )
    {
      int old_comp = components[i_to];
      for ( size_t i = 0; i < components.size(); ++i )
        if ( components[i] == old_comp )
          components[i] = components[i_from];
      --ncomp;
      edge_set.insert(candidates[idx]);
    }
    ++idx;
  }
  BOOST_ASSERT(ncomp == 1);
}


MSConnectivityScore::MSConnectivityScore(
  const MSConnectivityRestraint::ExperimentalTree &tree,
  const PairScore *ps, double eps,
  MSConnectivityRestraint &restraint)
    : restraint_(restraint)
    , ps_(ps)
    , tree_(tree)
    , eps_(eps)
{
  restraint_.particle_matrix_.create_distance_matrix(ps);
}


NNGraph MSConnectivityScore::create_nn_graph(double threshold) const
{
  size_t n = restraint_.particle_matrix_.size();
  NNGraph G(n);
  boost::property_map<NNGraph, boost::vertex_name_t>::type vertex_id =
    boost::get(boost::vertex_name, G);
  boost::property_map<NNGraph, boost::edge_weight_t>::type dist =
    boost::get(boost::edge_weight, G);
  for ( size_t i = 0; i < n; ++i )
  {
    boost::put(vertex_id, i, i);
    std::vector<size_t> const &neighbors =
      restraint_.particle_matrix_.get_ordered_neighbors(i);
    for ( size_t j = 0; j < neighbors.size(); ++j )
    {
      double d = restraint_.particle_matrix_.get_distance(i, neighbors[j]);
      if ( d > threshold )
        break;
      NNGraph::edge_descriptor e = boost::add_edge(i, neighbors[j], G).first;
      boost::put(dist, e, d);
    }
  }
  return G;
}


NNGraph MSConnectivityScore::build_subgraph_from_assignment(NNGraph &G,
    Assignment const &assignment) const
{
  size_t num_particles = restraint_.particle_matrix_.size();
  std::vector<size_t> vertices;
  for ( size_t i = 0; i < assignment.size(); ++i )
    if ( not assignment[i].empty() )
    {
      std::vector<size_t> const &conf = assignment[i].get_tuple();
      for ( size_t j = 0; j < conf.size(); ++j )
        vertices.push_back(conf[j]);
    }
  boost::property_map<NNGraph, boost::vertex_name_t>::type vertex_id =
    boost::get(boost::vertex_name, G);
  boost::property_map<NNGraph, boost::edge_weight_t>::type dist =
    boost::get(boost::edge_weight, G);
  NNGraph ng(vertices.size());
  boost::property_map<NNGraph, boost::vertex_name_t>::type new_vertex_id =
    boost::get(boost::vertex_name, ng);
  boost::property_map<NNGraph, boost::edge_weight_t>::type new_dist =
    boost::get(boost::edge_weight, ng);
  for ( size_t i = 0; i < vertices.size(); ++i )
    boost::put(new_vertex_id, i, vertices[i]);
  std::vector<size_t> vertex_id_to_idx(num_particles, -1);
  for ( size_t i = 0; i < vertices.size(); ++i )
    vertex_id_to_idx[vertices[i]] = i;
  NNGraph::edge_iterator e, end;
  for ( boost::tie(e, end) = edges(G); e != end; ++e )
  {
    size_t source_id = boost::get(vertex_id, source(*e, G));
    size_t dest_id = boost::get(vertex_id, target(*e, G));
    size_t p_src = vertex_id_to_idx[source_id];
    size_t p_dst = vertex_id_to_idx[dest_id];
    if ( p_src == size_t(-1) || p_dst == size_t(-1) )
      continue;
    NNGraph::edge_descriptor ed = boost::add_edge(p_src, p_dst, ng).first;
    double d = boost::get(dist, *e);
    boost::put(new_dist, ed, d);
  }
  return ng;
}


bool MSConnectivityScore::check_assignment(NNGraph &G, size_t node_handle,
    Assignment const &assignment,
    EdgeSet &picked) const
{
  MSConnectivityRestraint::ExperimentalTree::Node const *node =
       tree_.get_node(node_handle);
  MSConnectivityRestraint::ExperimentalTree::Node::Label const &lb =
       node->get_label();
  std::vector<Tuples> new_tuples;
  std::vector<size_t> empty_vector;
  for ( size_t i = 0; i < lb.size(); ++i )
  {
    int prot_count = lb[i].second;
    size_t id = lb[i].first;
    while ( new_tuples.size() < id )
      new_tuples.push_back(Tuples(empty_vector, 0));
    if ( prot_count > 0 )
    {
      if ( ! assignment[id].empty() )
      {
        std::vector<size_t> const &configuration =
          assignment[id].get_tuple();
        if ( prot_count > int(configuration.size()) )
        {
          IMP_THROW("Experimental tree is inconsistent", IMP::ValueException);
        }
        new_tuples.push_back(Tuples(configuration, prot_count));
      }
      else
      {
        IMP_THROW("Experimental tree is inconsistent", IMP::ValueException);
      }
    }
    else
      new_tuples.push_back(Tuples(empty_vector, 0));
  }
  while ( new_tuples.size() <
    restraint_.particle_matrix_.get_number_of_classes() )
    new_tuples.push_back(Tuples(empty_vector, 0));
  Assignment new_assignment(new_tuples);
  if ( new_assignment.empty() )
    return false;
  do
  {
    NNGraph ng = build_subgraph_from_assignment(G, new_assignment);
    if ( is_connected(ng) )
    {
      EdgeSet n_picked;
      bool good = true;
      for ( size_t i = 0; i < node->get_number_of_children(); ++i )
      {
        size_t child_handle = node->get_child(i);
        if ( !check_assignment(ng, child_handle, new_assignment,
              n_picked) )
        {
          good = false;
          break;
        }
      }
      if ( good )
      {
        add_edges_to_set(ng, n_picked);
        picked.insert(n_picked.begin(), n_picked.end());
        return true;
      }
    }
  } while ( new_assignment.next() );
  return false;
}


bool MSConnectivityScore::perform_search(NNGraph &G,
  EdgeSet &picked) const
{
  size_t root_handle = tree_.get_root();
  MSConnectivityRestraint::ExperimentalTree::Node const *node =
       tree_.get_node(root_handle);
  MSConnectivityRestraint::ExperimentalTree::Node::Label const &lb =
       node->get_label();
  std::vector<Tuples> tuples;
  std::vector<size_t> empty_vector;
  for ( size_t i = 0; i < lb.size(); ++i )
  {
    int prot_count = lb[i].second;
    size_t id = lb[i].first;
    while ( tuples.size() < id )
      tuples.push_back(Tuples(empty_vector, 0));
    if ( prot_count > 0 )
    {
      tuples.push_back(Tuples(
            restraint_.particle_matrix_.get_all_proteins_in_class(id),
            prot_count));
    }
    else
      tuples.push_back(Tuples(empty_vector, 0));
  }
  while ( tuples.size() < restraint_.particle_matrix_.get_number_of_classes() )
    tuples.push_back(Tuples(empty_vector, 0));
  Assignment assignment(tuples);
  if ( assignment.empty() )
    return false;
  do
  {
    NNGraph ng = build_subgraph_from_assignment(G, assignment);
    if ( is_connected(ng) )
    {
      EdgeSet n_picked;
      bool good = true;
      for ( size_t i = 0; i < node->get_number_of_children(); ++i )
      {
        size_t child_handle = node->get_child(i);
        if ( !check_assignment(ng, child_handle, assignment,
              n_picked) )
        {
          good = false;
          break;
        }
      }
      if ( good )
      {
        add_edges_to_set(ng, n_picked);
        picked.insert(n_picked.begin(), n_picked.end());
        return true;
      }
    }
  } while ( assignment.next() );
  return false;
}


NNGraph MSConnectivityScore::pick_graph(EdgeSet const &picked) const
{
  EdgeSet::const_iterator p;
  std::map<size_t, int> idx_to_vtx;
  int n_vert = 0;
  for ( p = picked.begin(); p != picked.end(); ++p )
  {
    std::map<size_t, int>::iterator q = idx_to_vtx.find(p->first);
    if ( q == idx_to_vtx.end() )
      idx_to_vtx[p->first] = n_vert++;
    q = idx_to_vtx.find(p->second);
    if ( q == idx_to_vtx.end() )
      idx_to_vtx[p->second] = n_vert++;
  }
  NNGraph ng(n_vert);
  boost::property_map<NNGraph, boost::vertex_name_t>::type vertex_id =
    boost::get(boost::vertex_name, ng);
  boost::property_map<NNGraph, boost::edge_weight_t>::type dist =
    boost::get(boost::edge_weight, ng);
  for ( std::map<size_t, int>::iterator q = idx_to_vtx.begin();
      q != idx_to_vtx.end(); ++q )
  {
    boost::put(vertex_id, q->second, q->first);
  }
  for ( p = picked.begin(); p != picked.end(); ++p )
  {
    NNGraph::edge_descriptor e = boost::add_edge(idx_to_vtx[p->first],
        idx_to_vtx[p->second], ng).first;
    double d = restraint_.particle_matrix_.get_distance(p->first, p->second);
    boost::put(dist, e, d);
  }
  return ng;
}


NNGraph MSConnectivityScore::find_threshold() const
{
  double max_dist = 1.1*restraint_.particle_matrix_.max_distance(),
         min_dist = restraint_.particle_matrix_.min_distance();
  NNGraph g = create_nn_graph(min_dist);
  {
    std::set< std::pair<size_t, size_t> > picked;
    if ( perform_search(g, picked) )
    {
      return pick_graph(picked);
    }
    g = create_nn_graph(max_dist);
    if ( !perform_search(g, picked) )
    {
      IMP_THROW("Cannot build a nearest neighbor graph", IMP::ValueException);
    }
  }
  EdgeSet picked;
  while ( max_dist - min_dist > eps_ )
  {
    double dist = 0.5*(max_dist + min_dist);
    g = create_nn_graph(dist);
    EdgeSet tmp_picked;
    if ( perform_search(g, tmp_picked) )
    {
      picked.swap(tmp_picked);
      max_dist = dist;
    }
    else
      min_dist = dist;
  }
  return pick_graph(picked);
}


double MSConnectivityScore::score(DerivativeAccumulator *accum) const
{
  EdgeSet edges = get_connected_pairs();
  double sc = 0;
  for ( EdgeSet::iterator p = edges.begin();
    p != edges.end(); ++p )
  {
    if ( accum )
    {
      sc += ps_->evaluate(ParticlePair(
        restraint_.particle_matrix_.get_particle(p->first).get_particle(),
        restraint_.particle_matrix_.get_particle(p->second).get_particle()),
          accum);
    }
    else
    {
      sc += restraint_.particle_matrix_.get_distance(p->first, p->second);
    }
  }
  return sc;
}


EdgeSet MSConnectivityScore::get_connected_pairs() const
{
  NNGraph g = find_threshold();
  EdgeSet edges = get_all_edges(g);
  return edges;
}


namespace {
  internal::CoreListSingletonContainer* ms_get_list(SingletonContainer *sc) {
    internal::CoreListSingletonContainer *ret
      = dynamic_cast<internal::CoreListSingletonContainer*>(sc);
    if (!ret) {
      IMP_THROW("Can only use the set and add methods when no container"
                << " was passed on construction of MSConnectivityRestraint.",
                ValueException);
    }
    return ret;
  }
}

size_t MSConnectivityRestraint::add_type(const Particles &ps)
{
  if (!sc_&& !ps.empty()) {
    sc_= new internal::CoreListSingletonContainer(ps[0]->get_model(),
                                                  "msconnectivity list");
  }
  ms_get_list(sc_)->add_particles(ps);
  return particle_matrix_.add_type(ps);
}


size_t MSConnectivityRestraint::add_composite(
       const Ints &composite)
{
  return tree_.add_composite(composite);
}


size_t MSConnectivityRestraint::add_composite(
       const Ints &composite, size_t parent)
{
  return tree_.add_composite(composite, parent);
}


double
MSConnectivityRestraint::unprotected_evaluate(
  DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(ps_.get());
  IMP_OBJECT_LOG;
  tree_.finalize();
  MSConnectivityScore mcs(tree_, ps_.get(), eps_,
      *const_cast<MSConnectivityRestraint *>(this));
  return mcs.score(accum);
}


Restraints MSConnectivityRestraint::get_instant_decomposition() const {
  ParticlePairs pp= get_connected_pairs();
  Restraints ret(pp.size());
  for (unsigned int i=0; i< pp.size(); ++i) {
    IMP_NEW(PairRestraint, pr, (ps_, pp[i]));
    std::ostringstream oss;
    oss << get_name() << " " << i;
    pr->set_name(oss.str());
    ret[i]=pr;
  }
  return ret;
}


ParticlePairs MSConnectivityRestraint::get_connected_pairs() const {
  IMP_CHECK_OBJECT(ps_.get());
  tree_.finalize();
  MSConnectivityScore mcs(tree_, ps_.get(), eps_,
      *const_cast<MSConnectivityRestraint *>(this));
  EdgeSet edges = mcs.get_connected_pairs();
  ParticlePairs ret(edges.size());
  unsigned index = 0;
  for ( EdgeSet::iterator p = edges.begin(); p != edges.end(); ++p )
  {
    ret.set(index++, ParticlePair(mcs.get_particle(p->first),
      mcs.get_particle(p->second)));
  }
  return ret;
}


ParticlesTemp MSConnectivityRestraint::get_input_particles() const {
  if (!sc_) return ParticlesTemp();
  ParticlesTemp ret;
  IMP_FOREACH_SINGLETON(sc_, {
      ParticlesTemp cs = ps_->get_input_particles(_1);
      ret.insert(ret.end(), cs.begin(), cs.end());
    });
  return ret;
}

ContainersTemp MSConnectivityRestraint::get_input_containers() const {
  if (!sc_) return ContainersTemp();
  ContainersTemp ret;
  IMP_FOREACH_SINGLETON(sc_, {
      ContainersTemp cs
        = ps_->get_input_containers(_1);
      ret.insert(ret.end(), cs.begin(), cs.end());
    });
  return ret;
}


void MSConnectivityRestraint::do_show(std::ostream& out) const
{
  if (!sc_) {
    out << "container is NULL" << std::endl;
  } else {
    out << "container is " << *sc_ << std::endl;
  }
}

IMPCORE_END_NAMESPACE
