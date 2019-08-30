#ifndef TATOOINE_DIFF_H
#define TATOOINE_DIFF_H

#include "field.h"
#include "utility.h"
#include "symbolic_field.h"

//==============================================================================
namespace tatooine {
//==============================================================================

template <typename field_t, typename real_t, size_t N, size_t... TensorDims>
struct derived_field : field<derived_field<field_t, real_t, N, TensorDims...>,
                             real_t, N, TensorDims...> {
  using this_t   = derived_field<field_t, real_t, N, TensorDims...>;
  using parent_t = field<this_t, real_t, N, TensorDims...>;
  using parent_t::num_dimensions;
  using typename parent_t::pos_t;
  using typename parent_t::tensor_t;

  static_assert(field_t::tensor_t::num_dimensions() + 1 ==
                tensor_t::num_dimensions());

 private:
  //============================================================================
  const field_t& m_field;
  const real_t   m_eps;

 public:
  derived_field(const field_t& f, real_t eps = 1e-6) : m_field{f}, m_eps{eps} {}
  //============================================================================
  auto evaluate(const pos_t& x, const real_t t) const {
    tensor_t derivative;

    pos_t offset;
    for (size_t i = 0; i < N; ++i) {
      offset(i) = m_eps;
      derivative.template slice<sizeof...(TensorDims) - 1>(i) =
          (m_field(x + offset, t) - m_field(x - offset, t)) / (2 * m_eps);
      offset(i) = 0;
    }

    return derivative;
  }
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template <typename field_t, typename real_t, size_t N, size_t... TensorDims>
auto diff(const field<field_t, real_t, N, TensorDims...>& f) {
  return derived_field<field_t, real_t, N, TensorDims..., N>{f.as_derived()};
}

//------------------------------------------------------------------------------
template <typename real_t, size_t N, size_t... TensorDims>
auto diff(const symbolic::field<real_t, N, TensorDims...>& f) {
  tensor<GiNaC::ex, TensorDims..., N> ex;
  for (size_t i = 0; i < N; ++i) {
    ex.template slice<sizeof...(TensorDims)>(i) =
        diff(f.expr(), symbolic::symbol::x(i));
  }
  return symbolic::field<real_t, N, TensorDims..., N>{std::move(ex)};
}

//==============================================================================
}  // namespace tatooine
//==============================================================================

#endif