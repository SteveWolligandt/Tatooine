#ifndef TATOOINE_GRID_H
#define TATOOINE_GRID_H

//==============================================================================
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <cassert>
#include <set>
#include <utility>
#include "boundingbox.h"
#include "grid_vertex.h"
#include "grid_vertex_edges.h"
#include "grid_vertex_neighbors.h"
#include "linspace.h"
#include "random.h"
#include "subgrid.h"

//==============================================================================
namespace tatooine {
//==============================================================================

template <typename Real, size_t N>
class grid {
 public:
  using this_t        = grid<Real, N>;
  using linspace_t    = linspace<Real>;
  using linspace_it_t = typename linspace_t::iterator;
  using vertex_t      = grid_vertex<Real, N>;
  using edge_t        = grid_edge<Real, N>;
  using edge_pair     = std::pair<edge_t, edge_t>;
  using edge_seq_t    = std::vector<edge_t>;
  using vertex_seq_t  = std::deque<vertex_t>;

  struct vertex_container;

  //============================================================================
 private:
  std::array<linspace_t, N> m_dimensions;

  //============================================================================
 public:
  constexpr grid(const grid& other) : m_dimensions{other.m_dimensions} {}

  //----------------------------------------------------------------------------
  constexpr grid(grid&& other) noexcept
      : m_dimensions{std::move(other.m_dimensions)} {}

  //----------------------------------------------------------------------------
  template <typename OtherReal, size_t... Is>
  constexpr grid(const grid<OtherReal, N>& other,
                 std::index_sequence<Is...> /*is*/)
      : m_dimensions{other.dimension(Is)...} {}
  template <typename OtherReal>
  constexpr grid(const grid<OtherReal, N>& other)
      : grid(other, std::make_index_sequence<N>{}) {}

  //----------------------------------------------------------------------------
  template <typename... Reals>
  constexpr grid(const linspace<Reals>&... linspaces)
      : m_dimensions{linspace_t{linspaces}...} {
    static_assert(sizeof...(Reals) == N,
                  "number of linspaces does not match number of dimensions");
  }

  //----------------------------------------------------------------------------
  template <typename OtherReal, size_t... Is>
  constexpr grid(const boundingbox<OtherReal, N>& bb,
                 const std::array<size_t, N>&        res,
                 std::index_sequence<Is...> /*is*/)
      : m_dimensions{
            linspace_t{Real(bb.min(Is)), Real(bb.max(Is)), res[Is]}...} {}
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  template <typename OtherReal>
  constexpr grid(const boundingbox<OtherReal, N>& bb,
                 const std::array<size_t, N>&        res)
      : grid{bb, res, std::make_index_sequence<N>{}} {}

  //----------------------------------------------------------------------------
  ~grid() = default;

  //----------------------------------------------------------------------------
  constexpr auto& operator=(const grid& other) {
    m_dimensions = other.m_dimensions;
    return *this;
  }

  //----------------------------------------------------------------------------
  constexpr auto& operator=(grid&& other) noexcept {
    m_dimensions = std::move(other.m_dimensions);
    return *this;
  }

  //----------------------------------------------------------------------------
  template <typename OtherReal>
  constexpr auto& operator=(const grid<OtherReal, N>& other) {
    for (size_t i = 0; i < N; ++i) { m_dimensions[i] = other.dimension(i); }
    return *this;
  }

  //----------------------------------------------------------------------------
  constexpr auto&       dimension(size_t i) { return m_dimensions[i]; }
  constexpr const auto& dimension(size_t i) const { return m_dimensions[i]; }

  //----------------------------------------------------------------------------
  constexpr auto&       dimensions() { return m_dimensions; }
  constexpr const auto& dimensions() const { return m_dimensions; }

  //----------------------------------------------------------------------------
  constexpr auto boundingbox() const {
    return boundingbox(std::make_index_sequence<N>{});
  }

  //----------------------------------------------------------------------------
  template <size_t... Is>
  constexpr auto boundingbox(std::index_sequence<Is...> /*is*/) const {
    static_assert(sizeof...(Is) == N);
    return tatooine::boundingbox{vec{m_dimensions[Is].front()...},
                                 vec{m_dimensions[Is].back()...}};
  }

  //----------------------------------------------------------------------------
  constexpr auto size() const {
    return size(std::make_index_sequence<N>{});
  }

  //----------------------------------------------------------------------------
  template <size_t... Is>
  constexpr auto size(std::index_sequence<Is...> /*is*/) const {
    static_assert(sizeof...(Is) == N);
    return std::array{m_dimensions[Is].size()...};
  }

  //----------------------------------------------------------------------------
  constexpr auto size(size_t i) const { return dimension(i).size(); }

  //----------------------------------------------------------------------------
  template <size_t... Is, typename... Reals,
            typename = std::enable_if_t<(std::is_arithmetic_v<Reals> && ...)>>
  constexpr auto in_domain(std::index_sequence<Is...> /*is*/,
                           Reals... xs) const {
    static_assert(sizeof...(Reals) == N,
                  "number of components does not match number of dimensions");
    static_assert(sizeof...(Is) == N,
                  "number of indices does not match number of dimensions");
    return ((m_dimensions[Is].front() <= xs && xs <= m_dimensions[Is].back()) && ...);
  }

  //----------------------------------------------------------------------------
  template <typename... Reals,
            typename = std::enable_if_t<(std::is_arithmetic_v<Reals> && ...)>>
  constexpr auto in_domain(Reals... xs) const {
    static_assert(sizeof...(Reals) == N,
                  "number of components does not match number of dimensions");
    return in_domain(std::make_index_sequence<N>{},
                     std::forward<Reals>(xs)...);
  }

  //----------------------------------------------------------------------------
  template <size_t... Is>
  constexpr auto in_domain(const std::array<Real, N>& x,
                           std::index_sequence<Is...> /*is*/) const {
    return in_domain(x[Is]...);
  }

  //----------------------------------------------------------------------------
  constexpr auto in_domain(const std::array<Real, N>& x) const {
    return in_domain(x, std::make_index_sequence<N>{});
  }

  //----------------------------------------------------------------------------
  constexpr auto num_points() const {
    return num_points(std::make_index_sequence<N>{});
  }

  //----------------------------------------------------------------------------
  template <size_t... Is>
  constexpr auto num_points(std::index_sequence<Is...> /*is*/) const {
    static_assert(sizeof...(Is) == N);
    return (m_dimensions[Is].size() * ...);
  }

  //----------------------------------------------------------------------------
  template <size_t... Is>
  auto front_vertex(std::index_sequence<Is...> /*is*/) {
    return vertex_t{m_dimensions[Is].begin()...};
  }

  //----------------------------------------------------------------------------
  auto front_vertex() { return front_vertex(std::make_index_sequence<N>()); }

  //----------------------------------------------------------------------------
  template <size_t... Is>
  auto back_vertex(std::index_sequence<Is...> /*is*/) {
    return vertex_t{(--m_dimensions[Is].end())...};
  }

  //----------------------------------------------------------------------------
  auto back_vertex() { return back_vertex(std::make_index_sequence<N>()); }

  //----------------------------------------------------------------------------
  template <typename... Is>
  auto at(Is... is) const {
    static_assert(sizeof...(Is) == N);
    return grid_vertex(*this, is...);
  }

  //----------------------------------------------------------------------------
  constexpr auto num_vertices() const {
    size_t num = 1;
    for (const auto& dim : m_dimensions) { num *= dim.size(); }
    return num;
  }

  //----------------------------------------------------------------------------
 private:
  template <size_t... Is>
  constexpr auto vertex_begin(std::index_sequence<Is...> /*is*/) const {
    return typename vertex_t::iterator{vertex_t(m_dimensions[Is].begin()...)};
  }

 public:
  constexpr auto vertex_begin() const {
    return vertex_begin(std::make_index_sequence<N>{});
  }

 private:
  //----------------------------------------------------------------------------
  template <size_t... Is>
  constexpr auto vertex_end(std::index_sequence<Is...> /*is*/) const {
    return typename vertex_t::iterator{
        vertex_t(m_dimensions[Is].begin()..., m_dimensions.back().end())};
  }

 public:
  //----------------------------------------------------------------------------
  constexpr auto vertex_end() const {
    return vertex_end(std::make_index_sequence<N - 1>());
  }

  //----------------------------------------------------------------------------
  auto vertices() const { return vertex_container{this}; }

  //----------------------------------------------------------------------------
  auto vertices(const vertex_t& v) const {
    return grid_vertex_neighbors<Real, N>(v);
  }

  //----------------------------------------------------------------------------
  auto edges(const vertex_t& v) const {
    return grid_vertex_edges<Real, N>(v);
  }

  //----------------------------------------------------------------------------
  auto sub(const vertex_t& begin_vertex, const vertex_t& end_vertex) const {
    return subgrid<Real, N>(this, begin_vertex, end_vertex);
  }

  //----------------------------------------------------------------------------
  /// checks if an edge e has vertex v as point
  auto contains(const vertex_t& v, const edge_t& e) {
    return v == e.first || v == e.second;
  }

  //----------------------------------------------------------------------------
  /// checks if an edge sequence seq has vertex v as point
  auto contains(const vertex_t& v, const edge_seq_t& seq) {
    for (const auto& e : seq) {
      if (contains(v, e)) { return true; }
    }
    return false;
  }

  //----------------------------------------------------------------------------
  /// checks if v0 and v1 are direct or diagonal neighbors
  auto are_neighbors(const vertex_t& v0, const vertex_t& v1) {
    auto v0_it = begin(v0.iterators);
    auto v1_it = begin(v1.iterators);
    for (; v0_it != end(v0.iterators); ++v0_it, ++v1_it) {
      if (distance(*v0_it, *v1_it) > 1) { return false; }
    }
    return true;
  }

  //----------------------------------------------------------------------------
  /// checks if v0 and v1 are direct neighbors
  auto are_direct_neighbors(const vertex_t& v0, const vertex_t& v1) {
    bool off   = false;
    auto v0_it = begin(v0.iterators);
    auto v1_it = begin(v1.iterators);
    for (; v0_it != end(v0.iterators); ++v0_it, ++v1_it) {
      auto dist = std::abs(distance(*v0_it, *v1_it));
      if (dist > 1) { return false; }
      if (dist == 1 && !off) { off = true; }
      if (dist == 1 && off) { return false; }
    }
    return true;
  }

  //----------------------------------------------------------------------------
  /// returns set of vertex neighbors of v that are not already in edge
  /// sequence
  auto free_neighbors(const vertex_t& v, const vertex_seq_t& seq) {
    std::set<vertex_t> free_vertices;
    std::copy_if(
        vertices(v).begin(), vertices(v).end(),
        std::inserter(free_vertices, free_vertices.end()),
        [&](const auto& vn) { return boost::find(seq, vn) == seq.end(); });
    return free_vertices;
  }

  //----------------------------------------------------------------------------
  /// returns set of vertex neighbors of v that are not already in edge
  /// sequence
  auto free_neighbors(const vertex_t& v, const edge_seq_t& seq) {
    std::set<vertex_t> free_vertices;
    for (const auto& vn : vertices(v)) {
      if (!contains(vn, seq)) { free_vertices.insert(vn); }
    }
    return free_vertices;
  }

  //----------------------------------------------------------------------------
  template <typename T, typename RandEng>
  auto random_uniform(T low, T hi, RandEng& eng) const {
    if constexpr (std::is_floating_point_v<T>) {
      return std::uniform_real_distribution<T>(low, hi)(eng);
    } else {
      return std::uniform_int_distribution<T>(low, hi)(eng);
    }
  }

  //----------------------------------------------------------------------------
  template <typename T, typename RandEng>
  auto random_normal(T mean, T stddev, RandEng& eng) const {
    return std::normal_distribution<T>(mean, stddev)(eng);
  }

 private:
  //----------------------------------------------------------------------------
  template <size_t... Is, typename RandEng>
  constexpr auto random_vertex(std::index_sequence<Is...> /*is*/,
                               RandEng& eng) const {
    return vertex_t{linspace_it_t{
        &m_dimensions[Is],
        random_uniform<size_t>(0, m_dimensions[Is].size() - 1, eng)}...};
  }

 public:
  //----------------------------------------------------------------------------
  template <typename RandEng>
  vertex_t random_vertex(RandEng& eng) {
    return random_vertex(std::make_index_sequence<N>(), eng);
  }

  //----------------------------------------------------------------------------
  template <typename RandEng>
  auto random_vertex_neighbor_gaussian(const vertex_t& v, const Real _stddev,
                                       RandEng& eng) {
    auto neighbor = v;
    bool ok       = false;
    auto stddev   = _stddev;
    do {
      ok = true;
      for (size_t i = 0; i < N; ++i) {
        auto r = random_normal<Real>(
            0, std::min<Real>(stddev, neighbor[i].linspace().size() / 2),
            eng);
        // stddev -= r;
        neighbor[i].i() += static_cast<size_t>(r);
        if (neighbor[i].i() < 0 ||
            neighbor[i].i() >= neighbor[i].linspace().size()) {
          ok       = false;
          neighbor = v;
          stddev   = _stddev;
          break;
        }
      }
    } while (!ok);
    return neighbor;
  }

  //----------------------------------------------------------------------------
  template <typename RandEng = std ::mt19937_64>
  auto random_vertex_seq_neighbor_gaussian(const vertex_seq_t& seq,
                                           Real stddev, RandEng& eng) {
    return random_vertex_neighbor_gaussian(
        seq[random_uniform<size_t>(0, seq.size() - 1, eng)], stddev, eng);
  }

  //----------------------------------------------------------------------------
  template <typename RandEng = std ::mt19937_64>
  auto random_vertex_sequence(size_t len, const vertex_seq_t& base_seq,
                              Real stddev, RandEng& eng) {
    vertex_seq_t seq;
    do {
      // start vertex
      seq.clear();
      auto v = random_vertex_seq_neighbor_gaussian(base_seq, stddev, eng);
      seq.push_back(v);

      for (size_t i = 0; i < len; ++i) {
        auto neighbors = free_neighbors(v, seq);
        if (neighbors.empty()) { break; }
        v = *next(neighbors.begin(),
                  random_uniform<size_t>(0, neighbors.size() - 1, eng));
        seq.push_back(v);
      }
    } while (seq.size() == len);

    return seq;
  }

  //----------------------------------------------------------------------------
  template <typename RandEng>
  auto random_vertex_sequence(size_t len, const vertex_t& start_v,
                              RandEng& eng) {
    vertex_seq_t seq;
    // start vertex
    do {
      seq.clear();
      seq.push_back(start_v);
      auto v = seq.back();

      for (size_t i = 0; i < len; ++i) {
        auto neighbors = free_neighbors(v, seq);
        if (neighbors.empty()) { break; }
        v = *next(neighbors.begin(),
                  random_uniform<size_t>(0, neighbors.size() - 1, eng));
        seq.push_back(v);
      }
    } while (seq.size() == len);

    return seq;
  }

  //----------------------------------------------------------------------------
  template <typename RandEng>
  auto random_vertex_sequence(size_t len, RandEng& eng) {
    vertex_seq_t seq;
    // start vertex
    do {
      seq.clear();
      seq.push_back(random_vertex(eng));

      for (size_t i = 0; i < len; ++i) {
        auto neighbors = free_neighbors(seq.back(), seq);
        if (neighbors.empty()) { break; }
        seq.push_back(
            *next(neighbors.begin(),
                  random_uniform<size_t>(0, neighbors.size() - 1, eng)));
      }
    } while (seq.size() == len);

    return seq;
  }

  //----------------------------------------------------------------------------
  /// \param min_angle angle in radians
  template <typename RandEng>
  auto random_straight_vertex_sequence(size_t len, Real min_angle,
                                       RandEng& eng) {
    using namespace boost;
    using namespace adaptors;
    vertex_seq_t seq;
    // start vertex
    do {
      seq.clear();
      seq.push_back(random_vertex(eng));

      for (size_t i = 0; i < len; ++i) {
        auto neighbors = free_neighbors(seq.back(), seq);
        if (i > 0) {
          // remove vertices that dont keep the vertex sequence straight
          const auto left_vertex   = prev(seq.end(), 2)->position();
          const auto center_vertex = prev(seq.end())->position();
          const auto last_dir      = normalize(left_vertex - center_vertex);

          auto angle_straight = [&](const auto& right_vertex) {
            return min_angle < angle(last_dir, normalize(right_vertex.position() -
                                                         center_vertex));
          };

          auto filtered_neighbors = neighbors | filtered(angle_straight);
          auto neighbor           = random_elem(filtered_neighbors, eng);
          if (neighbor == end(filtered_neighbors)) { break; }
          seq.push_back(*neighbor);

        } else {
          if (neighbors.empty()) { break; }
          seq.push_back(
              *next(begin(neighbors),
                    random_uniform<size_t>(0, neighbors.size() - 1, eng)));
        }
      }
    } while (seq.size() == len);

    return seq;
  }

  //----------------------------------------------------------------------------
  /// changes one random vertex to another random position on the grid keeping
  /// total edge sequence length; diagonal edge length corresponds straight
  /// edge length
  template <typename RandEng>
  auto mutate_seq(const vertex_seq_t& seq, RandEng& eng) {
    using vertex_set = std::set<vertex_t>;
    enum Op { MOVE, DELETE, INSERT_BEFORE, INSERT_AFTER };
    std::vector<Op> possible_operations;
    auto            mutated_seq = seq;

    unsigned int num_trials = 0;
    do {
      ++num_trials;
      possible_operations.clear();
      // choose random vertex in sequence
      auto v = random_elem(mutated_seq, eng);

      // construct lists of neighbors for random vertex and its possible
      // predecessor and successor
      const auto neighbors = free_neighbors(*v, mutated_seq);

      const std::set<vertex_t> prev_neighbors =
          v != begin(mutated_seq) ? free_neighbors(*prev(v), mutated_seq)
                                  : vertex_set{};

      const vertex_set next_neighbors =
          v != prev(end(mutated_seq)) ? free_neighbors(*next(v), mutated_seq)
                                      : vertex_set{};

      const vertex_set prev_intersection = [&]() {
        if (v != begin(mutated_seq)) {
          vertex_set s;
          boost::range::set_intersection(neighbors, prev_neighbors,
                                         std::inserter(s, end(s)));
          return s;
        }
        return vertex_set{};
      }();
      const vertex_set next_intersection = [&]() {
        if (v != prev(end(mutated_seq))) {
          vertex_set s;
          boost::range::set_intersection(neighbors, next_neighbors,
                                         std::inserter(s, end(s)));
          return s;
        }
        return vertex_set{};
      }();

      // vertex can be moved if it has free neighbors
      vertex_set possible_moves = neighbors;
      if (v != begin(mutated_seq)) {
        vertex_set is;
        boost::set_intersection(possible_moves, prev_neighbors,
                                std::inserter(is, end(is)));
        possible_moves = is;
      }
      if (v != prev(end(mutated_seq))) {
        vertex_set is;
        boost::set_intersection(possible_moves, next_neighbors,
                                std::inserter(is, end(is)));
        possible_moves = is;
      }

      if (mutated_seq.size() > 2) {
        // vertex can be deleted if it is front or back
        if (v == begin(mutated_seq) || v == prev(end(mutated_seq))) {
          possible_operations.push_back(DELETE);
        }

        // delete immediately if previous vertex has next vertex as neighbor
        // this deletes corners
        else if (boost::find(prev_neighbors, *next(v)) != end(prev_neighbors)) {
          for (unsigned int i = 0; i < 3; ++i) {
            possible_operations.push_back(DELETE);
          }
        }
      }

      if (!possible_moves.empty()) { possible_operations.push_back(MOVE); }

      // if vertex is front or predecessor and vertex share neighbors
      if ((v == begin(mutated_seq) &&
           !neighbors.empty()) /* || !prev_intersection.empty() */) {
        possible_operations.push_back(INSERT_BEFORE);
      }

      // if vertex is back or successor and vertex share neighbors
      if ((v == prev(end(mutated_seq)) &&
           !neighbors.empty()) /* || !next_intersection.empty() */) {
        possible_operations.push_back(INSERT_AFTER);
      }

      if (!possible_operations.empty()) {
        switch (*random_elem(possible_operations, eng)) {
          case MOVE: *v = *random_elem(possible_moves, eng); break;

          case DELETE: mutated_seq.erase(v); break;

          case INSERT_BEFORE:
            if (v == begin(mutated_seq)) {
              mutated_seq.insert(v, *random_elem(neighbors, eng));
            } else {
              mutated_seq.insert(v, *random_elem(prev_intersection, eng));
            }
            break;

          case INSERT_AFTER:
            if (v == prev(end(mutated_seq))) {
              mutated_seq.push_back(*random_elem(neighbors, eng));
            } else {
              mutated_seq.insert(next(v), *random_elem(next_intersection, eng));
            }
            break;
        }
      }
    } while (possible_operations.empty() && num_trials < 100);

    return mutated_seq;
  }

  //----------------------------------------------------------------------------
  /// picks one random vertex and changes either its left or right side
  template <typename RandEng>
  auto mutate_seq_straight(const vertex_seq_t& seq, Real min_angle,
                           size_t max_size_change, RandEng& eng) {
    using namespace boost;
    using namespace adaptors;

    std::uniform_int_distribution size_change_dist{-static_cast<int>(max_size_change),
                                  static_cast<int>(max_size_change)};
    auto size_change = size_change_dist(eng);
    vertex_seq_t mseq;
    bool         done = false;
    while (!done) {
      mseq      = seq;
      auto v_it = random_elem(mseq, eng);

      const auto coin_side = flip_coin(eng);
      // rearrange left side
      if ((coin_side == HEADS && v_it != begin(mseq)) ||
          (coin_side == TAILS && v_it == prev(end(mseq)))) {
        auto left_length = distance(begin(meq), v_it);
        auto new_left_length =
            static_cast<size_t>(std::max<int>(0, left_size + change));
        // move all preceeding vertices to randomly chosen vertex
        for (auto it = begin(mseq); it != v_it; ++it) { *it = *v_it; }
        // for all preceeding vertices find a new one that keeps the line
        // straight using min_angle
        while (v_it != begin(mseq)) {
          --v_it;
          auto neighbors = free_neighbors(*next(v_it), mseq);
          if (v_it == prev(end(mseq), 2)) {
            auto neighbor_it = random_elem(neighbors, eng);
            if (neighbor_it == end(neighbors)) {
              break;
            }
            *v_it = *neighbor_it;

          } else {
            const auto center_vertex = next(v_it)->position();
            const auto right_vertex  = next(v_it, 2)->position();
            const auto last_dir      = normalize(right_vertex - center_vertex);

            auto angle_straight = [&](const auto& left_vertex) {
              return min_angle <
                     angle(last_dir,
                           normalize(left_vertex.position() - center_vertex));
            };

            auto filtered_neighbors = neighbors | filtered(angle_straight);
            auto neighbor_it        = random_elem(filtered_neighbors, eng);
            if (neighbor_it == end(filtered_neighbors)) {
              break;
            }
            *v_it = *neighbor_it;
          }
        }
        if (v_it == begin(mseq)) {
          done = true;
        }

      } else {
        // move all succeeding vertices to randomly chosen vertex
        for (auto it = next(v_it); it != end(mseq); ++it) { *it = *v_it; }
        // for all preceeding vertices find a new one that keeps the line
        // straight using min_angle
        ++v_it;
        while (v_it != end(mseq)) {
          auto neighbors = free_neighbors(*prev(v_it), mseq);
          if (v_it == next(begin(mseq))) {
            auto neighbor_it = random_elem(neighbors, eng);
            if (neighbor_it == end(neighbors)) {
              break;
            }
            *v_it = *neighbor_it;
          } else {
            const auto center_vertex = prev(v_it)->position();
            const auto left_vertex   = prev(v_it, 2)->position();
            const auto last_dir      = normalize(left_vertex - center_vertex);

            auto angle_straight = [&](const auto& right_vertex) {
              return min_angle <
                     angle(last_dir,
                           normalize(right_vertex.position() - center_vertex));
            };

            auto filtered_neighbors = neighbors | filtered(angle_straight);
            auto neighbor_it        = random_elem(filtered_neighbors, eng);
            if (neighbor_it == end(filtered_neighbors)) {
              break;
            }
            *v_it = *neighbor_it;
          }
          ++v_it;
        }
        if (v_it == end(mseq)) {
          done = true;
        }
      }
    }

    return mseq;
  }

  //----------------------------------------------------------------------------
  /// changes one random vertex to another random position on the grid keeping
  /// total edge sequence length; diagonal edge length corresponds straight
  /// edge length
  template <typename RandEng>
  auto mutate_seq(const edge_seq_t& seq, RandEng& eng) {
    using namespace boost;
    auto               mutated_seq = seq;
    edge_t *           e0 = nullptr, *e1 = nullptr;
    vertex_t           v = *vertex_begin();
    std::set<vertex_t> neighbors;

    do {
      // pick a random edge e
      e0 = &mutated_seq[random_uniform<size_t>(0, mutated_seq.size() - 1, eng)];

      // pick a random vertex v of e
      v = random_uniform<size_t>(1, 2, eng) == 1 ? e0->first : e0->second;
      neighbors = free_neighbors(v, mutated_seq);

      // search for second edge that also uses v
      e1 = nullptr;
      for (auto& e : mutated_seq) {
        if (e != *e0 && contains(v, e)) {
          e1 = &e;
          break;
        }
      }

      // intersection of neighbors of other vertex of e0
      const auto&        v0 = v == e0->first ? e0->second : e0->first;
      std::set<vertex_t> neighbors_of_v0 = free_neighbors(v0, mutated_seq);

      std::set<vertex_t> neighbor_intersection0;

      set_intersection(
          neighbors, neighbors_of_v0,
          std::inserter(neighbor_intersection0, neighbor_intersection0.end()));
      neighbors = std::move(neighbor_intersection0);

      if (e1) {
        // intersection of neighbors of other vertex of e1
        const auto&        v1 = v == e1->first ? e1->second : e1->first;
        std::set<vertex_t> neighbors_of_v1 = free_neighbors(v1, mutated_seq);

        std::set<vertex_t> neighbor_intersection1;

        set_intersection(neighbors, neighbors_of_v1,
                         std::inserter(neighbor_intersection1,
                                       neighbor_intersection1.end()));
        neighbors = std::move(neighbor_intersection1);
      }
    } while (neighbors.empty());

    // pick a random neighbor vertex vn of v that is not used by any other
    // edge of sequence
    auto vn_i        = random_uniform<size_t>(0, neighbors.size() - 1, eng);
    auto neighbor_it = neighbors.begin();
    while (vn_i != 0) {
      --vn_i;
      neighbor_it++;
    }
    auto new_vertex = *neighbor_it;

    if (e0->first == v) {
      e0->first = new_vertex;
    } else if (e0->second == v) {
      e0->second = new_vertex;
    }

    if (e1) {
      if (e1->first == v) {
        e1->first = new_vertex;
      } else if (e1->second == v) {
        e1->second = new_vertex;
      }
    }

    return mutated_seq;
  }

  //----------------------------------------------------------------------------
  /// randomly changes order of edge sequence
  template <typename RandEng>
  auto permute_edge_seq(const edge_seq_t& edge_seq, RandEng& eng) {
    auto   permuted_seq = edge_seq;
    size_t i0 = random_uniform<size_t>(0, permuted_seq.size() - 1, eng);
    size_t i1 = 0;
    do {
      i1 = random_uniform<size_t>(0, permuted_seq.size() - 1, eng);
    } while (i0 == i1);
    std::swap(permuted_seq[i0], permuted_seq[i1]);
    return permuted_seq;
  }

  //----------------------------------------------------------------------------
  auto to_edge_seq(const vertex_seq_t& vertex_seq) const {
    edge_seq_t edge_seq;
    for (auto it = vertex_seq.begin(); it != prev(vertex_seq.end()); ++it) {
      edge_seq.emplace_back(*it, *next(it));
    }
    return edge_seq;
  }

  //----------------------------------------------------------------------------
  template <typename OtherReal, size_t... Is>
  auto add_dimension(const linspace<OtherReal>& additional_dimension,
                     std::index_sequence<Is...> /*is*/) const {
    return grid<Real, N + 1>{m_dimensions[Is]..., additional_dimension};
  }

  //----------------------------------------------------------------------------
  template <typename OtherReal>
  auto add_dimension(const linspace<OtherReal>& additional_dimension) const {
    return add_dimension(additional_dimension, std::make_index_sequence<N>{});
  }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <typename... Reals>
grid(const linspace<Reals>&...)
    ->grid<promote_t<Reals...>, sizeof...(Reals)>;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Real, size_t N, size_t... Is>
grid(const boundingbox<Real, N>& bb, const std::array<size_t, N>& res,
     std::index_sequence<Is...>)
    ->grid<Real, N>;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Real, size_t N>
grid(const boundingbox<Real, N>& bb, const std::array<size_t, N>& res)
    ->grid<Real, N>;

//==============================================================================
template <typename Real, size_t N>
struct grid<Real, N>::vertex_container {
  const grid* g;
  auto        begin() const { return g->vertex_begin(); }
  auto        end() const { return g->vertex_end(); }
  auto        size() const { return g->num_vertices(); }
};

//------------------------------------------------------------------------------
template <typename Real, size_t N>
auto operator+(const grid<Real, N>&  grid,
               const linspace<Real>& additional_dimension) {
  return grid.add_dimension(additional_dimension);
}

//------------------------------------------------------------------------------
template <typename Real, size_t N>
auto operator+(const linspace<Real>& additional_dimension,
               const grid<Real, N>&  grid) {
  return grid.add_dimension(additional_dimension);
}

//==============================================================================
}  // namespace tatooine
//==============================================================================

#endif
