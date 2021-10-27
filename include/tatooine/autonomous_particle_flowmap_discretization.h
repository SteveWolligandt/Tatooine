#ifndef TATOOINE_AUTONOMOUS_PARTICLE_FLOWMAP_DISCRETIZATION_H
#define TATOOINE_AUTONOMOUS_PARTICLE_FLOWMAP_DISCRETIZATION_H
//==============================================================================
#include <tatooine/autonomous_particle.h>
#include <tatooine/uniform_tree_hierarchy.h>
#include <tatooine/unstructured_simplex_grid.h>
#include <tatooine/staggered_flowmap_discretization.h>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/copy.hpp>
//==============================================================================
namespace tatooine {
//==============================================================================
template <typename Derived, typename Real, size_t NumDimensions>
struct autonomous_particle_sampler_hierarchy
    : base_uniform_tree_hierarchy<
          Real, NumDimensions,
          autonomous_particle_sampler_hierarchy<Derived, Real, NumDimensions>> {
  static_assert(
      constrained_delaunay_available(NumDimensions),
      "autonomous_particle_sampler_hierarchy needs constrained delaunay");
  using ellipse_t           = autonomous_particle_sampler<Real, NumDimensions>;
  using ellipse_container_t = std::vector<ellipse_t>;
  using this_t =
      autonomous_particle_sampler_hierarchy<Derived, Real, NumDimensions>;
  using parent_t = base_uniform_tree_hierarchy<Real, NumDimensions, this_t>;
  using real_t   = typename parent_t::real_t;
  using parent_t::center;
  using parent_t::children;
  using parent_t::is_at_max_depth;
  using parent_t::is_inside;
  using parent_t::is_simplex_inside;
  using parent_t::is_splitted;
  using parent_t::max;
  using parent_t::min;
  using parent_t::split_and_distribute;
  using typename parent_t::vec_t;
  using pos_t = vec_t;

  //============================================================================
 private:
  ellipse_container_t const* m_ellipses = nullptr;
  std::vector<size_t>        m_indices;
  //============================================================================
 public:
  autonomous_particle_sampler_hierarchy() = default;
  autonomous_particle_sampler_hierarchy(
      autonomous_particle_sampler_hierarchy const&) = default;
  autonomous_particle_sampler_hierarchy(
      autonomous_particle_sampler_hierarchy&&) noexcept = default;
  auto operator=(autonomous_particle_sampler_hierarchy const&)
      -> autonomous_particle_sampler_hierarchy& = default;
  auto operator=(autonomous_particle_sampler_hierarchy&&) noexcept
      -> autonomous_particle_sampler_hierarchy&    = default;
  virtual ~autonomous_particle_sampler_hierarchy() = default;
  //----------------------------------------------------------------------------
  explicit autonomous_particle_sampler_hierarchy(
      ellipse_container_t const& ellipses,
      size_t const               max_depth = parent_t::default_max_depth)
      : parent_t{pos_t::ones() * -std::numeric_limits<real_t>::max(),
                 pos_t::ones() * std::numeric_limits<real_t>::max(), max_depth},
        m_ellipses{&ellipses} {
    for (size_t i = 0; i < size(ellipses); ++i) {
      insert_ellipse(i);
    }
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  autonomous_particle_sampler_hierarchy(
      vec_t const& min, vec_t const& max, ellipse_container_t const& ellipses,
      size_t const max_depth = parent_t::default_max_depth)
      : parent_t{min, max, 1, max_depth}, m_ellipses{&ellipses} {
    for (size_t i = 0; i < size(ellipses); ++i) {
      insert_ellipse(i);
    }
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 private:
  autonomous_particle_sampler_hierarchy(vec_t const& min, vec_t const& max,
                                        size_t const               level,
                                        size_t const               max_depth,
                                        ellipse_container_t const& ellipses)
      : parent_t{min, max, level, max_depth}, m_ellipses{&ellipses} {}
  //============================================================================
 public:
  auto ellipses() const -> auto const& { return *m_ellipses; }
  auto ellipse_indices_in_node() const -> auto const& { return m_indices; }
  auto ellipse(size_t const i) const -> auto const& {
    return static_cast<Derived const*>(this)->ellipse(i);
  }
  auto constexpr holds_ellipses() const { return !m_indices.empty(); }
  //----------------------------------------------------------------------------
  auto insert_ellipse(size_t const i) -> bool {
    auto const& e = ellipse(i);
    if (!this->is_rectangle_inside(e.center() + e.S() * vec_t{-1, -1},
                                   e.center() + e.S() * vec_t{1, -1},
                                   e.center() + e.S() * vec_t{1, 1},
                                   e.center() + e.S() * vec_t{-1, 1})) {
      return false;
    }
    if (holds_ellipses()) {
      if (is_at_max_depth()) {
        m_indices.push_back(i);
      } else {
        split_and_distribute();
        distribute_ellipse(i);
      }
    } else {
      if (is_splitted()) {
        distribute_ellipse(i);
      } else {
        m_indices.push_back(i);
      }
    }
    return true;
  }
  //----------------------------------------------------------------------------
  auto distribute() {
    if (!m_indices.empty()) {
      distribute_ellipse(m_indices.front());
      m_indices.clear();
    }
  }
  //------------------------------------------------------------------------------
  auto construct(vec_t const& min, vec_t const& max, size_t const level,
                 size_t const max_depth) const {
    return std::unique_ptr<this_t>{
        new this_t{min, max, level, max_depth, ellipses()}};
  }
  //----------------------------------------------------------------------------
  auto distribute_ellipse(size_t const i) {
    for (auto& child : children()) {
      child->insert_ellipse(i);
    }
  }
  //----------------------------------------------------------------------------
  auto collect_nearby_ellipses(pos_t const&                pos,
                               std::unordered_set<size_t>& cells) const
      -> void {
    if (is_inside(pos)) {
      if (is_splitted()) {
        for (auto const& child : children()) {
          child->collect_nearby_ellipses(pos, cells);
        }
      } else {
        if (!m_indices.empty()) {
          std::copy(begin(m_indices), end(m_indices),
                    std::inserter(cells, end(cells)));
        }
      }
    }
  }
  //----------------------------------------------------------------------------
  auto nearby_ellipses(pos_t const& pos) const {
    std::unordered_set<size_t> indices;
    collect_nearby_ellipses(pos, indices);
    return indices;
  }
};
//==============================================================================
template <typename Real, size_t NumDimensions>
struct forward_autonomous_particle_sampler_hierarchy
    : autonomous_particle_sampler_hierarchy<
          forward_autonomous_particle_sampler_hierarchy<Real, NumDimensions>,
          Real, NumDimensions> {
  using parent_t = autonomous_particle_sampler_hierarchy<
      forward_autonomous_particle_sampler_hierarchy<Real, NumDimensions>, Real,
      NumDimensions>;
  using parent_t::parent_t;
  auto ellipse(size_t const i) const -> auto const& {
    return this->ellipses().at(i).ellipse1();
  }
};
//==============================================================================
template <typename Real, size_t NumDimensions>
struct backward_autonomous_particle_sampler_hierarchy
    : autonomous_particle_sampler_hierarchy<
          backward_autonomous_particle_sampler_hierarchy<Real, NumDimensions>,
          Real, NumDimensions> {
  using parent_t = autonomous_particle_sampler_hierarchy<
      backward_autonomous_particle_sampler_hierarchy<Real, NumDimensions>, Real,
      NumDimensions>;
  using parent_t::parent_t;
  auto ellipse(size_t const i) const -> auto const& {
    return this->ellipses().at(i).ellipse1();
  }
};
//==============================================================================
template <typename Real, size_t NumDimensions>
struct autonomous_particle_flowmap_discretization {
  using real_t              = Real;
  using vec_t               = vec<Real, NumDimensions>;
  using pos_t               = vec_t;
  using sampler_t           = autonomous_particle_sampler<Real, NumDimensions>;
  using sampler_container_t = std::vector<sampler_t>;
  using mesh_t              = unstructured_simplex_grid<Real, NumDimensions>;
  using mesh_prop_t =
      typename mesh_t::template vertex_property_t<sampler_t const*>;
  static constexpr auto num_dimensions() { return NumDimensions; }
  //============================================================================
 private:
  sampler_container_t m_samplers;
  //mesh_t              m_mesh0;
  //mesh_t              m_mesh1;
  //mesh_prop_t*        m_mesh0_samplers;
  //mesh_prop_t*        m_mesh1_samplers;
  //std::unique_ptr<
  //    forward_autonomous_particle_sampler_hierarchy<Real, NumDimensions>>
  //    m_hierarchy0;
  //std::unique_ptr<
  //    backward_autonomous_particle_sampler_hierarchy<Real, NumDimensions>>
  //    m_hierarchy1;
  //============================================================================
 public:
  template <typename Flowmap>
  autonomous_particle_flowmap_discretization(
      Flowmap&& flowmap, arithmetic auto const t0, arithmetic auto const tau,
      arithmetic auto const                                tau_step,
      uniform_rectilinear_grid<Real, NumDimensions> const& g) {
    static_assert(
        std::decay_t<Flowmap>::num_dimensions() == NumDimensions,
        "Number of dimensions of flowmap does not match number of dimensions.");
    auto initial_particle_distribution = g.copy_without_properties();
    std::vector<autonomous_particle<Real, NumDimensions>> particles;
    for (size_t i = 0; i < NumDimensions; ++i) {
      auto const spacing = initial_particle_distribution.dimension(i).spacing();
      initial_particle_distribution.dimension(i).pop_front();
      initial_particle_distribution.dimension(i).front() -= spacing / 2;
      initial_particle_distribution.dimension(i).back() -= spacing / 2;
    }
    initial_particle_distribution.vertices().iterate_indices(
        [&](auto const... is) {
          particles.emplace_back(
              initial_particle_distribution.vertex_at(is...), t0,
              initial_particle_distribution.dimension(0).spacing() / 2);
        });
    auto const small_particle_size =
        (std::sqrt(2 * initial_particle_distribution.dimension(0).spacing() *
                   initial_particle_distribution.dimension(0).spacing()) -
         initial_particle_distribution.dimension(0).spacing()) /
        2;

    std::cerr << small_particle_size << '\n';
    for (size_t i = 0; i < NumDimensions; ++i) {
      auto const spacing = initial_particle_distribution.dimension(i).spacing();
      initial_particle_distribution.dimension(i).pop_front();
      initial_particle_distribution.dimension(i).front() -= spacing / 2;
      initial_particle_distribution.dimension(i).back() -= spacing / 2;
    }
    initial_particle_distribution.vertices().iterate_indices(
        [&](auto const... is) {
          particles.emplace_back(
              initial_particle_distribution.vertex_at(is...), t0,
              small_particle_size);
        });
    fill(std::forward<Flowmap>(flowmap), particles, t0 + tau, tau_step);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename Flowmap>
  autonomous_particle_flowmap_discretization(
      Flowmap&& flowmap, arithmetic auto const t1,
      arithmetic auto const tau_step,
      std::vector<autonomous_particle<Real, NumDimensions>> const&
          initial_particles) {
    static_assert(
        std::decay_t<Flowmap>::num_dimensions() == NumDimensions,
        "Number of dimensions of flowmap does not match number of dimensions.");
    fill(std::forward<Flowmap>(flowmap), initial_particles, t1, tau_step);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename Flowmap>
  autonomous_particle_flowmap_discretization(
      Flowmap&& flowmap, arithmetic auto const t1,
      arithmetic auto const                           tau_step,
      autonomous_particle<Real, NumDimensions> const& initial_particle) {
    static_assert(
        std::decay_t<Flowmap>::num_dimensions() == NumDimensions,
        "Number of dimensions of flowmap does not match number of dimensions.");
    fill(std::forward<Flowmap>(flowmap), std::vector{initial_particle}, t1,
         tau_step);
  }
  //============================================================================
  auto samplers() const -> auto const& { return m_samplers; }
  //auto mesh0()    const -> auto const& { return m_mesh0; }
  //auto mesh1()    const -> auto const& { return m_mesh1; }
  //============================================================================
 private:
  template <typename Flowmap>
  auto fill(Flowmap&& flowmap, range auto const& initial_particles,
            arithmetic auto const t1, arithmetic auto const tau_step) {
    //auto const advected_particles =
        autonomous_particle<Real, NumDimensions>::advect_with_3_splits(
            std::forward<Flowmap>(flowmap), tau_step, t1, initial_particles);
    //m_samplers.reserve(size(advected_particles));
    //using boost::copy;
    //using boost::adaptors::transformed;
    //auto constexpr sampler = [](auto const& p) { return p.sampler(); };
    //copy(advected_particles | transformed(sampler),
    //     std::back_inserter(m_samplers));
    //m_hierarchy0 = std::make_unique<
    //    forward_autonomous_particle_sampler_hierarchy<Real, NumDimensions>>(
    //    vec2{-10, -10}, vec2{10, 10}, m_samplers, 10);
    //m_hierarchy1 = std::make_unique<
    //    backward_autonomous_particle_sampler_hierarchy<Real, NumDimensions>>(
    //    vec2{-10, -10}, vec2{10, 10}, m_samplers, 10);
    //
    //m_mesh0_samplers =
    //    &m_mesh0.template vertex_property<sampler_t const*>("samplers");
    //m_mesh1_samplers =
    //    &m_mesh1.template vertex_property<sampler_t const*>("samplers");
    //
    //size_t const num_vertices = 15;
    //auto const   ts           = linspace{0.0, 2 * M_PI, num_vertices + 1};
    //std::vector<std::pair<typename mesh_t::vertex_handle,
    //                      typename mesh_t::vertex_handle>>
    //                                            constraints0, constraints1;
    //std::vector<typename mesh_t::vertex_handle> vertices_of_ellipse0(
    //    num_vertices);
    //std::vector<typename mesh_t::vertex_handle> vertices_of_ellipse1(
    //    num_vertices);
    //for (auto const& sampler : samplers()) {
    //  {
    //    auto const v = m_mesh0.insert_vertex(sampler.ellipse0().center());
    //    m_mesh0_samplers->at(v) = &sampler;
    //  }
    //
    //  {
    //    auto const v = m_mesh1.insert_vertex(sampler.ellipse1().center());
    //    m_mesh1_samplers->at(v) = &sampler;
    //  }
    //
    //  auto vit0 = vertices_of_ellipse0.begin();
    //  auto vit1 = vertices_of_ellipse1.begin();
    //  for (auto t_it = begin(ts); t_it != prev(end(ts));
    //       ++t_it, ++vit0, ++vit1) {
    //    auto const t = *t_it;
    //    auto const y = vec{std::cos(t), std::sin(t)};
    //    {
    //      auto const v = m_mesh0.insert_vertex(sampler.ellipse0().center() +
    //                                           sampler.ellipse0().S() * y);
    //      *vit0        = v;
    //      m_mesh0_samplers->at(v) = &sampler;
    //    }
    //
    //    {
    //      auto const v = m_mesh1.insert_vertex(sampler.ellipse1().center() +
    //                                           sampler.ellipse1().S() * y);
    //      *vit1        = v;
    //      m_mesh1_samplers->at(v) = &sampler;
    //    }
    //  }
    //  for (size_t i = 0; i < num_vertices - 1; ++i) {
    //    constraints0.emplace_back(vertices_of_ellipse0[i],
    //                              vertices_of_ellipse0[i + 1]);
    //  }
    //  constraints0.emplace_back(vertices_of_ellipse1[num_vertices - 1],
    //                            vertices_of_ellipse1[0]);
    //  for (size_t i = 0; i < num_vertices - 1; ++i) {
    //    constraints1.emplace_back(vertices_of_ellipse1[i],
    //                              vertices_of_ellipse1[i + 1]);
    //  }
    //  constraints1.emplace_back(vertices_of_ellipse0[num_vertices - 1],
    //                            vertices_of_ellipse0[0]);
    //}
    //
    //m_mesh0.build_delaunay_mesh(constraints0);
    //m_mesh0.build_hierarchy();
    //
    //m_mesh1.build_delaunay_mesh(constraints1);
    //m_mesh1.build_hierarchy();
  }
  //----------------------------------------------------------------------------
 private:
  template <typename Tag, size_t... VertexSeq>
  [[nodiscard]] auto sample(pos_t const& x, Tag const tag,
                            std::index_sequence<VertexSeq...> /*seq*/) const {
    auto shortest_distance = std::numeric_limits<real_t>::infinity();
    sampler_t const* nearest_sampler   = nullptr;
    for (auto const& sampler : m_samplers) {
      if (auto const dist =
              sampler.ellipse(tag).squared_euclidean_distance_to_center(x);
          dist < shortest_distance) {
        shortest_distance = dist;
        nearest_sampler   = &sampler;
      }
    }
    return nearest_sampler->sample(x, tag);
  }

 public:
  //----------------------------------------------------------------------------
  [[nodiscard]] auto sample_forward(pos_t const& x) const {
    return sample(x, tag::forward,
                  std::make_index_sequence<NumDimensions + 1>{});
  }
  //----------------------------------------------------------------------------
  auto operator()(pos_t const& x, tag::forward_t /*tag*/) const {
    return sample_forward(x);
  }
  //----------------------------------------------------------------------------
  auto sample_backward(pos_t const& x) const {
    return sample(x, tag::backward,
                  std::make_index_sequence<NumDimensions + 1>{});
  }
  //----------------------------------------------------------------------------
  auto operator()(pos_t const& x, tag::backward_t /*tag*/) const {
    return sample_backward(x);
  }
};
//==============================================================================
template <size_t NumDimensions>
using AutonomousParticleFlowmapDiscretization =
    autonomous_particle_flowmap_discretization<real_t, NumDimensions>;
using autonomous_particle_flowmap_discretization_2 =
    AutonomousParticleFlowmapDiscretization<2>;
using autonomous_particle_flowmap_discretization_3 =
    AutonomousParticleFlowmapDiscretization<3>;
//==============================================================================
template <typename Real, size_t NumDimensions>
using staggered_autonomous_particle_flowmap_discretization =
    staggered_flowmap_discretization<
        autonomous_particle_flowmap_discretization<Real, NumDimensions>>;
//------------------------------------------------------------------------------
template <size_t NumDimensions>
using StaggeredAutonomousParticleFlowmapDiscretization =
    staggered_autonomous_particle_flowmap_discretization<real_t, NumDimensions>;
using staggered_autonomous_particle_flowmap_discretization_2 =
    StaggeredAutonomousParticleFlowmapDiscretization<2>;
using staggered_autonomous_particle_flowmap_discretization_3 =
    StaggeredAutonomousParticleFlowmapDiscretization<3>;
//==============================================================================
}  // namespace tatooine
//==============================================================================
#endif
