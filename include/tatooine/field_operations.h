#ifndef TATOOINE_FIELD_OPERATIONS_H
#define TATOOINE_FIELD_OPERATIONS_H
//==============================================================================
#include <tatooine/field.h>
#include <tatooine/unary_operation_field.h>
#include <tatooine/binary_operation_field.h>
//==============================================================================
namespace tatooine {
//==============================================================================
template <typename V0, typename Real0,
          typename V1, typename Real1,
          size_t N, size_t TN>
constexpr auto dot(const field<V0, Real0, N, TN>& lhs,
                   const field<V1, Real1, N, TN>& rhs) {
  using RealOut = common_type<Real0, Real1>;
  return make_binary_operation_field<RealOut, N>(
      lhs, rhs,
      [](const typename V0::tensor_t& lhs, const typename V1::tensor_t& rhs) {
        return dot(lhs, rhs);
      });
}
//├──────────────────────────────────────────────────────────────────────────┤
template <typename V0, typename Real0,
          typename V1, typename Real1,
          size_t N, size_t... TensorDims>
constexpr auto operator+(
    const field<V0, Real0, N, TensorDims...>& lhs,
    const field<V1, Real1, N, TensorDims...>& rhs) {
  return make_binary_operation_field<common_type<Real0, Real1>, N,
                                     TensorDims...>(
      lhs, rhs, [](const auto& lhs, const auto& rhs) { return lhs + rhs; });
}
//├──────────────────────────────────────────────────────────────────────────┤
template <typename V0, typename Real0, typename V1,
          typename Real1, size_t N, size_t TM, size_t TN>
constexpr auto operator*(const field<V0, Real0, N, TM, TN>& lhs,
                         const field<V1, Real1, N, TN>&     rhs) {
  return make_binary_operation_field<common_type<Real0, Real1>, N, TM>(
      lhs, rhs, [](const auto& lhs, const auto& rhs) { return lhs * rhs; });
}
//├──────────────────────────────────────────────────────────────────────────┤
template <typename V0, typename Real0,
          typename V1, typename Real1,
          size_t N, size_t... TensorDims>
constexpr auto operator*(const field<V0, Real0, N, TensorDims...>& lhs,
                         const field<V1, Real1, N, TensorDims...>& rhs) {
  return make_binary_operation_field<common_type<Real0, Real1>, N, TensorDims...>(
      lhs, rhs, [](const auto& lhs, const auto& rhs) { return lhs * rhs; });
}
//├──────────────────────────────────────────────────────────────────────────┤
#ifdef __cpp_concpets
template <typename V, arithmetic VReal, size_t N, arithmetic ScalarReal,
          size_t... TensorDims>
#else
template <typename V, typename VReal, size_t N, typename ScalarReal,
          size_t... TensorDims, enable_if<is_arithmetic<VReal, ScalarReal>> = true>
#endif
constexpr auto operator*(const field<V, VReal, N, TensorDims...>& f,
                         const ScalarReal                         scalar) {
  return V{f.as_derived()} | [scalar](auto const& t) { return t * scalar; };
}
//├──────────────────────────────────────────────────────────────────────────┤
#ifdef __cpp_concpets
template <typename V, arithmetic VReal, size_t N, arithmetic ScalarReal,
          size_t... TensorDims>
#else
template <typename V, typename VReal, size_t N, typename ScalarReal,
          size_t... TensorDims, enable_if<is_arithmetic<VReal, ScalarReal>> = true>
#endif
constexpr auto operator*(const ScalarReal                         scalar,
                         const field<V, VReal, N, TensorDims...>& f) {
  return V{f.as_derived()} | [scalar](auto const& t) { return t * scalar; };
}
//------------------------------------------------------------------------------
template <typename V, typename VReal, size_t N, typename ScalarReal,
          size_t... TensorDims,
          enable_if<is_arithmetic<ScalarReal> || is_complex<ScalarReal>> = true>
constexpr auto operator/(field<V, VReal, N, TensorDims...> const& f,
                         ScalarReal const                         scalar) {
  return V{f.as_derived()} | [scalar](auto const& t) { return t / scalar; };
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename V, typename VReal, size_t N, typename ScalarReal,
          size_t... TensorDims,
          enable_if<is_arithmetic<ScalarReal> || is_complex<ScalarReal>> = true>
constexpr auto operator/(ScalarReal const                         scalar,
                         field<V, VReal, N, TensorDims...> const& f) {
  return V{f.as_derived()} | [scalar](auto const& t) { return scalar / t; };
}
//------------------------------------------------------------------------------
// template <typename lhs_tensor_t, typename Real0,
//          typename rhs_tensor_t, typename Real1,
//          size_t... Dims>
// constexpr auto operator-(
//    const base_tensor<lhs_tensor_t, Real0, Dims...>& lhs,
//    const base_tensor<rhs_tensor_t, Real1, Dims...>& rhs) {
//  return binary_operation(std::minus<common_type<Real0, Real1>>{}, lhs,
//                          rhs);
//}
//
//├──────────────────────────────────────────────────────────────────────────┤
///// matrix-vector-multiplication
// template <typename lhs_tensor_t, typename Real0,
//          typename rhs_tensor_t, typename Real1, size_t M, size_t N>
// constexpr auto operator*(const base_tensor<lhs_tensor_t, Real0, M, N>& lhs,
//                         const base_tensor<rhs_tensor_t, Real1, N>& rhs) {
//  tensor<common_type<Real0, Real1>, M> product;
//  for (size_t i = 0; i < M; ++i) {
//    product(i) = dot(lhs.template slice<0>(i), rhs);
//  }
//  return product;
//}
template <typename V, typename VReal, size_t N>
constexpr auto length(vectorfield<V, VReal, N> const& v) {
  return make_unary_operation_field(v, [](auto const& v) { return length(v); });
}
template <typename V, typename VReal, size_t N>
constexpr auto length(vectorfield<V, VReal, N>&& v) {
  return make_unary_operation_field(std::move(v),
                                    [](auto const& v) { return length(v); });
}
//==============================================================================
}  // namespace tatooine
//==============================================================================
#endif
