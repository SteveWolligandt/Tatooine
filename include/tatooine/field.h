#ifndef TATOOINE_FIELD_H
#define TATOOINE_FIELD_H

#include <vector>

#include "crtp.h"
#include "grid.h"
#include "tensor.h"
#include "type_traits.h"

//==============================================================================
namespace tatooine {
//==============================================================================

struct out_of_domain : std::runtime_error {
  out_of_domain() : std::runtime_error{""} {}
};

//==============================================================================
template <typename Real, size_t... Dims>
struct tensor_type_impl {
  using type = tensor<Real, Dims...>;
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Real, size_t N>
struct tensor_type_impl<Real, N> {
  using type = vec<Real, N>;
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Real, size_t M, size_t N>
struct tensor_type_impl<Real, M, N> {
  using type = mat<Real, M, N>;
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename Real, size_t... Dims>
using tensor_type = typename tensor_type_impl<Real, Dims...>::type;

//==============================================================================
template <typename Derived, typename Real, size_t N, size_t... TensorDims>
struct field : crtp<Derived> {
  using real_t   = Real;
  using this_t   = field<Derived, Real, N, TensorDims...>;
  using parent_t = crtp<Derived>;
  using pos_t    = vec<Real, N>;
  using tensor_t = std::conditional_t<sizeof...(TensorDims) == 0, Real,
                                      tensor_type<Real, TensorDims...>>;
  static constexpr auto num_dimensions() { return N; }
  static constexpr auto num_tensor_dimensions() {
    return sizeof...(TensorDims);
  }
  template <size_t _num_tensor_dims = sizeof...(TensorDims),
            std::enable_if_t<(_num_tensor_dims > 0)>...>
  static constexpr auto tensor_dimension(size_t i) {
    return tensor_t::dimension(i);
  }
  using parent_t::as_derived;

  //============================================================================
  constexpr tensor_t operator()(const pos_t& x, Real t = 0) const {
    return evaluate(x, t);
  }

  //----------------------------------------------------------------------------
  constexpr tensor_t evaluate(const pos_t& x, Real t = 0) const {
    if (!in_domain(x, t)) { throw out_of_domain{}; }
    return as_derived().evaluate(x, t);
  }

  //----------------------------------------------------------------------------
  constexpr decltype(auto) in_domain([[maybe_unused]] const pos_t& x,
                                     [[maybe_unused]] Real t = 0) const {
    return as_derived().in_domain(x, t);
  }
};

//==============================================================================
template <typename Field, typename Real, size_t N, typename Op,
          size_t... TensorDims>
struct unary_operation_field
    : field<unary_operation_field<Field, Real, N, Op, TensorDims...>, Real, N,
            TensorDims...> {
 public:
  using this_t   = unary_operation_field<Field, Real, N, Op, TensorDims...>;
  using parent_t = field<this_t, Real, N, TensorDims...>;
  using typename parent_t::pos_t;
  using typename parent_t::tensor_t;

 private:
  Field m_field;
  Op    m_operator;

 public:
  template <typename RealIn, size_t NIn, size_t... TensorDimsIn>
  unary_operation_field(const field<Field, RealIn, NIn, TensorDimsIn...>& f,
                        Op&&                                              op)
      : m_field{f.as_derived()}, m_operator{op} {}
  //============================================================================
  tensor_t evaluate(const pos_t& x, Real t) const {
    return m_operator(m_field(x, t));
  }
  //----------------------------------------------------------------------------
  bool in_domain(const pos_t& x, Real t) const {
    return m_field.in_domain(x, t);
  }
};

//==============================================================================
template <typename LhsField, typename RhsField, typename Op, typename Real,
          size_t N, size_t... TensorDims>
struct binary_operation_field
    : field<binary_operation_field<LhsField, RhsField, Op, Real, N,
                                   TensorDims...>,
            Real, N, TensorDims...> {
 public:
  using this_t =
      binary_operation_field<LhsField, RhsField, Op, Real, N, TensorDims...>;
  using parent_t = field<this_t, Real, N, TensorDims...>;
  using typename parent_t::pos_t;
  using typename parent_t::tensor_t;

 private:
  LhsField m_lhs_field;
  RhsField m_rhs_field;
  Op       m_operator;

 public:
  constexpr binary_operation_field(const LhsField& lhs_field,
                                   const RhsField& rhs_field, const Op& op)
      : m_lhs_field{lhs_field}, m_rhs_field{rhs_field}, m_operator{op} {}

  constexpr binary_operation_field(const LhsField& lhs_field,
                                   const RhsField& rhs_field, Op&& op)
      : m_lhs_field{lhs_field},
        m_rhs_field{rhs_field},
        m_operator{std::move(op)} {}

  //----------------------------------------------------------------------------
  tensor_t evaluate(const pos_t& x, Real t) const {
    return m_operator(m_lhs_field(x, t), m_rhs_field(x, t));
  }
  //----------------------------------------------------------------------------
  bool in_domain(const pos_t& x, Real t) const {
    return m_lhs_field.in_domain(x, t) && m_rhs_field.in_domain(x, t);
  }
};

//==============================================================================
// operations
//==============================================================================

template <typename RealOut, size_t NOut, size_t... TensorDimsOut,
          typename Field, typename Real, size_t N, size_t... TensorDims,
          typename Op>
constexpr auto make_unary_operation_field(
    const field<Field, Real, N, TensorDims...>& f, Op&& op) {
  return unary_operation_field<Field, RealOut, NOut, std::decay_t<Op>,
                               TensorDimsOut...>{f, std::forward<Op>(op)};
}
//------------------------------------------------------------------------------
template <typename Real, size_t N, size_t... TensorDims, typename LhsField,
          typename LhsReal, typename RhsField, typename RhsReal,
          size_t... LhsTensorDims, size_t... RhsTensorDims, typename Op>
constexpr auto make_binary_operation_field(
    const field<LhsField, LhsReal, N, LhsTensorDims...>& lhs,
    const field<RhsField, RhsReal, N, RhsTensorDims...>& rhs, Op&& op) {
  return binary_operation_field<LhsField, RhsField, std::decay_t<Op>, Real, N,
                                TensorDims...>{
      lhs.as_derived(), rhs.as_derived(), std::forward<Op>(op)};
}

//------------------------------------------------------------------------------
template <typename LhsField, typename LhsReal, typename RhsField,
          typename RhsReal, size_t N, size_t D>
constexpr auto dot(const field<LhsField, LhsReal, N, D>& lhs,
                   const field<RhsField, RhsReal, N, D>& rhs) {
  return make_binary_operation_field<promote_t<LhsReal, RhsReal>, N>(
      lhs, rhs, [](const auto& lhs, const auto& rhs) { return dot(lhs, rhs); });
}

//------------------------------------------------------------------------------
template <typename Field, typename Real, size_t N, size_t... TensorDims>
constexpr auto operator-(const field<Field, Real, N, TensorDims...>& f) {
  return make_unary_operation_field<Real, N, TensorDims...>(
      f, [](const auto& v) { return -v; });
}

//------------------------------------------------------------------------------
template <typename Field, typename Real, size_t N, size_t VecDim>
constexpr auto normalize(const field<Field, Real, N, VecDim>& f) {
  return make_unary_operation_field<Real, N, VecDim>(
      f, [](const auto& v) { return normalize(v); });
}

//------------------------------------------------------------------------------
template <typename Field, typename Real, size_t N, size_t VecDim>
constexpr auto length(const field<Field, Real, N, VecDim>& f) {
  return make_unary_operation_field<Real, N>(
      f, [](const auto& v) { return length(v); });
}

//------------------------------------------------------------------------------
template <typename LhsField, typename LhsReal, typename RhsField,
          typename RhsReal, size_t N, size_t... TensorDims>
constexpr auto operator+(
    const field<LhsField, LhsReal, N, TensorDims...>& lhs,
    const field<RhsField, RhsReal, N, TensorDims...>& rhs) {
  return make_binary_operation_field<promote_t<LhsReal, RhsReal>, N,
                                     TensorDims...>(
      lhs, rhs, [](const auto& lhs, const auto& rhs) { return lhs + rhs; });
}

//------------------------------------------------------------------------------
template <typename LhsField, typename LhsReal, typename RhsField,
          typename RhsReal, size_t N, size_t TM, size_t TN>
constexpr auto operator*(const field<LhsField, LhsReal, N, TM, TN>& lhs,
                         const field<RhsField, RhsReal, N, TN>&     rhs) {
  return make_binary_operation_field<promote_t<LhsReal, RhsReal>, N, TM>(
      lhs, rhs, [](const auto& lhs, const auto& rhs) { return lhs * rhs; });
}

//------------------------------------------------------------------------------
template <typename LhsField, typename LhsReal, typename RhsField,
          typename RhsReal, size_t... Dims>
constexpr auto operator*(const field<LhsField, LhsReal, Dims...>& lhs,
                         const field<RhsField, RhsReal, Dims...>& rhs) {
  return make_binary_operation_field<promote_t<LhsReal, RhsReal>, Dims...>(
      lhs, rhs, [](const auto& lhs, const auto& rhs) { return lhs * rhs; });
}

////------------------------------------------------------------------------------
// template <typename tensor_t, typename tensor_Real, typename scalar_Real,
//          size_t... Dims,
//          std::enable_if_t<std::is_arithmetic_v<scalar_Real> ||
//                           is_complex_v<scalar_Real> ||
//                           std::is_same_v<scalar_Real, GiNaC::ex>>...>
// constexpr auto operator*(const base_tensor<tensor_t, tensor_Real, Dims...>&
// t,
//                         const scalar_Real scalar) {
//  return unary_operation(
//      [scalar](const auto& component) { return component * scalar; }, t);
//}
//// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
///-
// template <typename tensor_t, typename tensor_Real, typename scalar_Real,
//          size_t... Dims,
//          std::enable_if_t<std::is_arithmetic_v<scalar_Real> ||
//                           is_complex_v<scalar_Real> ||
//                           std::is_same_v<scalar_Real, GiNaC::ex>>...>
// constexpr auto operator*(
//    const scalar_Real                                  scalar,
//    const base_tensor<tensor_t, tensor_Real, Dims...>& t) {
//  return unary_operation(
//      [scalar](const auto& component) { return component * scalar; }, t);
//}
//
////------------------------------------------------------------------------------
// template <typename tensor_t, typename tensor_Real, typename scalar_Real,
//          size_t... Dims,
//          std::enable_if_t<std::is_arithmetic_v<scalar_Real> ||
//                           is_complex_v<scalar_Real> ||
//                           std::is_same_v<scalar_Real, GiNaC::ex>>...>
// constexpr auto operator/(const base_tensor<tensor_t, tensor_Real, Dims...>&
// t,
//                         const scalar_Real scalar) {
//  return unary_operation(
//      [scalar](const auto& component) { return component / scalar; }, t);
//}
//// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
///-
// template <typename tensor_t, typename tensor_Real, typename scalar_Real,
//          size_t... Dims,
//          std::enable_if_t<std::is_arithmetic_v<scalar_Real> ||
//                           is_complex_v<scalar_Real> ||
//                           std::is_same_v<scalar_Real, GiNaC::ex>>...>
// constexpr auto operator/(
//    const scalar_Real                                  scalar,
//    const base_tensor<tensor_t, tensor_Real, Dims...>& t) {
//  return unary_operation(
//      [scalar](const auto& component) { return scalar / component; }, t);
//}
//
////------------------------------------------------------------------------------
// template <typename lhs_tensor_t, typename LhsReal,
//          typename rhs_tensor_t, typename RhsReal,
//          size_t... Dims>
// constexpr auto operator-(
//    const base_tensor<lhs_tensor_t, LhsReal, Dims...>& lhs,
//    const base_tensor<rhs_tensor_t, RhsReal, Dims...>& rhs) {
//  return binary_operation(std::minus<promote_t<LhsReal, RhsReal>>{}, lhs,
//                          rhs);
//}
//
////------------------------------------------------------------------------------
///// matrix-vector-multiplication
// template <typename lhs_tensor_t, typename LhsReal,
//          typename rhs_tensor_t, typename RhsReal, size_t M, size_t N>
// constexpr auto operator*(const base_tensor<lhs_tensor_t, LhsReal, M, N>& lhs,
//                         const base_tensor<rhs_tensor_t, RhsReal, N>& rhs) {
//  tensor<promote_t<LhsReal, RhsReal>, M> product;
//  for (size_t i = 0; i < M; ++i) {
//    product(i) = dot(lhs.template slice<0>(i), rhs);
//  }
//  return product;
//}

template <typename OutReal, typename Field, typename FieldReal,
          typename GridReal, typename TReal, size_t N, size_t... TensorDims,
          enable_if_arithmetic<FieldReal, GridReal, TReal> = true>
auto sample_to_raw(const field<Field, FieldReal, N, TensorDims...>& f,
                   const grid<GridReal, N>& g, TReal t, size_t padding = 0, OutReal padval = 0) {
  std::vector<OutReal> raw_data;
  raw_data.reserve(g.num_vertices() * Field::tensor_t::num_components());
  for (auto v : g.vertices()) {
    const auto x = v.position();
    if (f.in_domain(x, t)) {
      auto sample = f(x, t);
      for (size_t i = 0; i < Field::tensor_t::num_components(); ++i) {
        raw_data.push_back(sample[i]);
      }
      for (size_t i = 0; i < padding; ++i) { raw_data.push_back(padval); }
    } else {
      for (size_t i = 0; i < Field::tensor_t::num_components(); ++i) {
        raw_data.push_back(0.0 / 0.0);
      }
      for (size_t i = 0; i < padding; ++i) { raw_data.push_back(0.0 / 0.0); }
    }
  }
  return raw_data;
}
//------------------------------------------------------------------------------
template <typename OutReal, typename Field, typename FieldReal,
          typename GridReal, typename TReal, size_t N, size_t... TensorDims>
auto sample_to_raw(const field<Field, FieldReal, N, TensorDims...>& f,
                   const grid<GridReal, N>& g, const linspace<TReal>& ts,
                   size_t padding = 0, OutReal padval = 0) {
  std::vector<OutReal> raw_data;
  raw_data.reserve(g.num_vertices() * Field::tensor_t::num_components() *
                   ts.size());
  for (auto t : ts) {
    for (auto v : g.vertices()) {
      auto sample = f(v.position(), t);
      for (size_t i = 0; i < Field::tensor_t::num_components(); ++i) {
        raw_data.push_back(sample[i]);
      }
      for (size_t i = 0; i < padding; ++i) { raw_data.push_back(padval); }
    }
  }
  return raw_data;
}
//==============================================================================
}  // namespace tatooine
//==============================================================================

#endif
