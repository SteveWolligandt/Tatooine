#ifndef TATOOINE_DETAIL_POINTSET_RADIAL_BASIS_FUNCTIONS_SAMPLER_H
#define TATOOINE_DETAIL_POINTSET_RADIAL_BASIS_FUNCTIONS_SAMPLER_H
//==============================================================================
#include <tatooine/concepts.h>
#include <tatooine/tags.h>
//==============================================================================
namespace tatooine::detail::pointset {
//==============================================================================
template <floating_point Real, std::size_t NumDimensions, typename T,
          invocable<Real> Kernel>
struct radial_basis_functions_sampler
    : field<radial_basis_functions_sampler<Real, NumDimensions, T, Kernel>,
            Real, NumDimensions, T> {
  using this_type =
      radial_basis_functions_sampler<Real, NumDimensions, T, Kernel>;
  using parent_type = field<this_type, Real, NumDimensions, T>;
  using typename parent_type::pos_type;
  using typename parent_type::real_type;
  using typename parent_type::tensor_type;
  using pointset_type = tatooine::pointset<Real, NumDimensions>;
  using vertex_handle = typename pointset_type::vertex_handle;
  using vertex_property_type =
      typename pointset_type::template typed_vertex_property_type<T>;
  //==========================================================================
  pointset_type const&        m_pointset;
  vertex_property_type const& m_property;
  Kernel                      m_kernel;
  tensor<Real>                m_weights;
  //==========================================================================
  radial_basis_functions_sampler(pointset_type const&          ps,
                                 vertex_property_type const&   property,
                                 convertible_to<Kernel> auto&& kernel)
      : radial_basis_functions_sampler{ps, property,
                                       std::forward<decltype(kernel)>(kernel),
                                       execution_policy::sequential} {}
  //----------------------------------------------------------------------------
  radial_basis_functions_sampler(pointset_type const&          ps,
                                 vertex_property_type const&   property,
                                 convertible_to<Kernel> auto&& kernel,
                                 execution_policy::parallel_t /*pol*/)
      : m_pointset{ps},
        m_property{property},
        m_kernel{std::forward<decltype(kernel)>(kernel)} {
    auto const N = m_pointset.vertices().size();
    // construct lower part of symmetric matrix A
    auto A = tensor<real_type>::zeros(N, N);
#pragma omp parallel for collapse(2)
    for (std::size_t c = 0; c < N; ++c) {
      for (std::size_t r = c + 1; r < N; ++r) {
        A(r, c) = m_kernel(squared_euclidean_distance(m_pointset.vertex_at(c),
                                                      m_pointset.vertex_at(r)));
      }
    }
    m_weights = [N] {
      if constexpr (arithmetic<T>) {
        return tensor<T>::zeros(N);
      } else if constexpr (static_tensor<T>) {
        return tensor<tensor_value_type<T>>::zeros(N, T::num_components());
      }
    }();

#pragma omp parallel for
    for (std::size_t i = 0; i < N; ++i) {
      if constexpr (arithmetic<T>) {
        m_weights(i) = m_property[i];
      } else if constexpr (static_tensor<T>) {
        for (std::size_t j = 0; j < T::num_components(); ++j) {
          m_weights(i, j) = m_property[i].data()[j];
        }
      }
    }
    // do not copy by moving A and m_weights into solver
    m_weights = *solve_symmetric_lapack(std::move(A), std::move(m_weights),
                                        lapack::Uplo::Lower);
  }
  //----------------------------------------------------------------------------
  radial_basis_functions_sampler(pointset_type const&          ps,
                                 vertex_property_type const&   property,
                                 convertible_to<Kernel> auto&& kernel,
                                 execution_policy::sequential_t /*pol*/)
      : m_pointset{ps},
        m_property{property},
        m_kernel{std::forward<decltype(kernel)>(kernel)} {
    auto const N = m_pointset.vertices().size();
    // construct lower part of symmetric matrix A
    auto A = tensor<real_type>::zeros(N, N);
    for (std::size_t c = 0; c < N; ++c) {
      for (std::size_t r = c + 1; r < N; ++r) {
        A(r, c) = m_kernel(squared_euclidean_distance(m_pointset.vertex_at(c),
                                                      m_pointset.vertex_at(r)));
      }
    }
    m_weights = [N] {
      if constexpr (arithmetic<T>) {
        return tensor<T>::zeros(N);
      } else if constexpr (static_tensor<T>) {
        return tensor<tensor_value_type<T>>::zeros(N, T::num_components());
      }
    }();

    for (std::size_t i = 0; i < N; ++i) {
      if constexpr (arithmetic<T>) {
        m_weights(i) = m_property[i];
      } else if constexpr (static_tensor<T>) {
        for (std::size_t j = 0; j < T::num_components(); ++j) {
          m_weights(i, j) = m_property[i].data()[j];
        }
      }
    }
    // do not copy by moving A and m_weights into solver
    m_weights = *solve_symmetric_lapack(std::move(A), std::move(m_weights),
                                        lapack::Uplo::Lower);
  }
  //--------------------------------------------------------------------------
  radial_basis_functions_sampler(radial_basis_functions_sampler const&) =
      default;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  radial_basis_functions_sampler(radial_basis_functions_sampler&&) noexcept =
      default;
  //--------------------------------------------------------------------------
  auto operator=(radial_basis_functions_sampler const&)
      -> radial_basis_functions_sampler& = default;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  auto operator=(radial_basis_functions_sampler&&) noexcept
      -> radial_basis_functions_sampler& = default;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  ~radial_basis_functions_sampler() = default;
  //==========================================================================
  [[nodiscard]] auto evaluate(pos_type const& q, real_type const /*t*/) const
      -> tensor_type {
    auto acc = T{};
    for (auto const v : m_pointset.vertices()) {
      auto const sqr_dist = squared_euclidean_distance(q, m_pointset[v]);
      if (sqr_dist == 0) {
        return m_property[v];
      }
      if constexpr (arithmetic<T>) {
        acc += m_weights(v.index()) * m_kernel(sqr_dist);
      } else if constexpr (static_tensor<T>) {
        for (std::size_t j = 0; j < T::num_components(); ++j) {
          acc.data()[j] += m_weights(v.index(), j) * m_kernel(sqr_dist);
        }
      }
    }
    return acc;
  }
};
//==============================================================================
template <floating_point Real, std::size_t NumDimensions, typename T,
          invocable<Real> Kernel>
radial_basis_functions_sampler(
    tatooine::pointset<Real, NumDimensions> const& ps,
    typed_vector_property<
        typename tatooine::pointset<Real, NumDimensions>::vertex_handle, T>,
    Kernel&& kernel) -> radial_basis_functions_sampler<Real, NumDimensions, T,
                                                       std::decay_t<Kernel>>;
//==============================================================================
}  // namespace tatooine::detail::pointset
//==============================================================================
#endif
