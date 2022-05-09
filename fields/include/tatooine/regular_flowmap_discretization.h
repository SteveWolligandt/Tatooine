#ifndef TATOOINE_REGULAR_FLOWMAP_DISCRETIZATION_H
#define TATOOINE_REGULAR_FLOWMAP_DISCRETIZATION_H
//==============================================================================
#include <tatooine/field.h>
#include <tatooine/interpolation.h>
#include <tatooine/particle.h>
#include <tatooine/rectilinear_grid.h>
#include <tatooine/unstructured_triangular_grid.h>
//==============================================================================
namespace tatooine {
//==============================================================================
/// Samples a flow map by advecting particles from a uniform rectilinear grid.
template <typename Real, std::size_t N>
struct regular_flowmap_discretization {
  using real_type = Real;
  static auto constexpr num_dimensions() { return N; }
  using vec_type = vec<Real, N>;
  using pos_type = vec_type;
  template <std::size_t M, typename... Ts>
  struct grid_type_creator {
    using type = typename grid_type_creator<M - 1, linspace<Real>, Ts...>::type;
  };
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <typename... Ts>
  struct grid_type_creator<0, Ts...> {
    using type = rectilinear_grid<Ts...>;
  };
  //----------------------------------------------------------------------------
  using forward_grid_type = typename grid_type_creator<N>::type;
  using forward_grid_vertex_property_type =
      detail::rectilinear_grid::typed_vertex_property_interface<forward_grid_type, pos_type, true>;
  //----------------------------------------------------------------------------
  template <std::size_t M, template <typename> typename... InterpolationKernels>
  struct forward_grid_sampler_type_creator {
    using type =
        typename forward_grid_sampler_type_creator<M - 1, interpolation::linear,
                                           InterpolationKernels...>::type;
  };
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  template <template <typename> typename... InterpolationKernels>
  struct forward_grid_sampler_type_creator<0, InterpolationKernels...> {
    using type = tatooine::detail::rectilinear_grid::vertex_property_sampler<
        forward_grid_vertex_property_type, InterpolationKernels...>;
  };
  using forward_grid_vertex_property_sampler_type =
      typename forward_grid_sampler_type_creator<N>::type;

  //using forward_grid_vertex_property_sampler_type =
  //    typename forward_grid_vertex_property_type::
  //        inverse_distance_weighting_sampler_type;

  using backward_grid_type = unstructured_simplicial_grid<Real, N, N>;
  using backward_grid_vertex_property_type =
      typename backward_grid_type::template typed_vertex_property_type<
          pos_type>;
  using backward_grid_vertex_property_sampler_type =
      typename backward_grid_type::template vertex_property_sampler_type<
          pos_type>;
  //using backward_grid_vertex_property_sampler_type =
  //    typename backward_grid_type::template inverse_distance_weighting_sampler_type<
  //        pos_type>;
  //============================================================================
 private:
  Real m_t0;
  Real m_t1;
  Real m_tau;

  forward_grid_type                          m_forward_grid;
  forward_grid_vertex_property_type*         m_forward_flowmap_discretization;
  forward_grid_vertex_property_sampler_type  m_forward_sampler;

  backward_grid_type                         m_backward_grid;
  backward_grid_vertex_property_type*        m_backward_flowmap_discretization;
  backward_grid_vertex_property_sampler_type m_backward_sampler;
  static constexpr auto default_execution_policy = execution_policy::parallel;
  //============================================================================
 private:
  template <typename Flowmap, typename ExecutionPolicy, std::size_t... Is>
  regular_flowmap_discretization(std::index_sequence<Is...> /*seq*/,
                                 Flowmap&& flowmap, arithmetic auto const t0,
                                 arithmetic auto const tau, pos_type const& min,
                                 pos_type const& max,
                                 ExecutionPolicy execution_policy,
                                 integral auto const... resolution)
      : m_t0{real_type(t0)},
        m_t1{real_type(t0 + tau)},
        m_tau{real_type(tau)},
        m_forward_grid{linspace<Real>{min(Is), max(Is),
                                      static_cast<std::size_t>(resolution)}...},
        m_forward_flowmap_discretization{
            &m_forward_grid.template vertex_property<pos_type>(
                "flowmap_discretization")},
        m_forward_sampler{m_forward_flowmap_discretization->linear_sampler()},
        m_backward_grid{m_forward_grid},
        m_backward_flowmap_discretization{
            &m_backward_grid.template vertex_property<pos_type>(
                "flowmap_discretization")},
        m_backward_sampler{
            m_backward_grid.sampler(*m_backward_flowmap_discretization)} {
    fill(std::forward<Flowmap>(flowmap), execution_policy);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 public:
  template <typename Flowmap, typename ExecutionPolicy>
  regular_flowmap_discretization(Flowmap&&             flowmap,
                                 ExecutionPolicy       execution_policy,
                                 arithmetic auto const t0,
                                 arithmetic auto const tau, pos_type const& min,
                                 pos_type const& max,
                                 integral auto const... resolution)
      : regular_flowmap_discretization{std::make_index_sequence<N>{},
                                       std::forward<Flowmap>(flowmap),
                                       t0,
                                       tau,
                                       min,
                                       max,
                                       execution_policy,
                                       resolution...} {
    static_assert(
        sizeof...(resolution) == N,
        "Number of resolution components does not match number of dimensions.");
    static_assert(
        std::decay_t<Flowmap>::num_dimensions() == N,
        "Number of dimensions of flowmap does not match number of dimensions.");
  }
  //----------------------------------------------------------------------------
  template <typename Flowmap>
  regular_flowmap_discretization(Flowmap&& flowmap, arithmetic auto const t0,
                                 arithmetic auto const tau, pos_type const& min,
                                 pos_type const& max,
                                 integral auto const... resolution)
      : regular_flowmap_discretization{std::make_index_sequence<N>{},
                                       std::forward<Flowmap>(flowmap),
                                       t0,
                                       tau,
                                       min,
                                       max,
                                       default_execution_policy,
                                       resolution...} {
    static_assert(
        sizeof...(resolution) == N,
        "Number of resolution components does not match number of dimensions.");
    static_assert(
        std::decay_t<Flowmap>::num_dimensions() == N,
        "Number of dimensions of flowmap does not match number of dimensions.");
  }
  //----------------------------------------------------------------------------
  template <typename Flowmap, typename ExecutionPolicy>
  auto fill(Flowmap&& flowmap, ExecutionPolicy execution_policy) -> void {
    m_forward_grid.sample_to_vertex_property(
        [&](auto const& x) mutable {
          auto flowmap2 = flowmap;
          if constexpr (requires { flowmap2.use_caching(false); }) {
            flowmap2.use_caching(false);
          }
          auto const map = flowmap2(x, m_t0, m_tau);
          return map;
        },
        "flowmap_discretization", execution_policy);
    m_forward_grid.write("forward.vtr");

    m_forward_grid.vertices().iterate_indices(
        [&](auto const... is) mutable {
          m_backward_grid.vertex_at(m_forward_grid.plain_index(is...)) =
              m_forward_flowmap_discretization->at(is...);
          m_backward_flowmap_discretization->at(m_forward_grid.plain_index(
              is...)) = m_forward_grid.vertex_at(is...);
        },
        execution_policy);
    // m_backward_grid.build_delaunay_mesh();
    m_backward_grid.build_hierarchy();
    m_backward_grid.write("backward.vtu");
  }
  //----------------------------------------------------------------------------
  /// \{
  auto grid(forward_tag const /*tag*/) const -> auto const& {
    return m_forward_grid;
  }
  //----------------------------------------------------------------------------
  auto grid(forward_tag const /*tag*/) -> auto& { return m_forward_grid; }
  //----------------------------------------------------------------------------
  auto grid(backward_tag const /*tag*/) const -> auto const& {
    return m_backward_grid;
  }
  //----------------------------------------------------------------------------
  auto grid(backward_tag const /*tag*/) -> auto& { return m_backward_grid; }
  /// \}
  //----------------------------------------------------------------------------
  /// \{
  auto sampler(forward_tag const /*tag*/) const -> auto const& {
    return m_forward_sampler;
  }
  //----------------------------------------------------------------------------
  auto sampler(forward_tag const /*tag*/) -> auto& {
    return m_forward_sampler;
  }
  //----------------------------------------------------------------------------
  auto sampler(backward_tag const /*tag*/) const -> auto const& {
    return m_backward_sampler;
  }
  //----------------------------------------------------------------------------
  auto sampler(backward_tag const /*tag*/) -> auto& {
    return m_backward_sampler;
  }
  /// \}
  //----------------------------------------------------------------------------
  /// Evaluates flow map in forward direction at time t0 with maximal available
  /// advection time.
  /// \param x position
  /// \returns phi(x, t0, t1 - t0)
  auto sample(pos_type const& x, forward_or_backward_tag auto const tag) const {
    return sampler(tag)(x);
  }
};
//==============================================================================
using regular_flowmap_discretization2 =
    regular_flowmap_discretization<real_number, 2>;
//==============================================================================
}  // namespace tatooine
//==============================================================================
#endif
