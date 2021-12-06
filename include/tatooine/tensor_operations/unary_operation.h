#ifndef TATOOINE_TENSOR_OPERATIONS_UNARY_OPERATION_H
#define TATOOINE_TENSOR_OPERATIONS_UNARY_OPERATION_H
//==============================================================================
#include <tatooine/base_tensor.h>
//==============================================================================
namespace tatooine {
//==============================================================================
template <typename F, typename Tensor, typename T, std::size_t N>
constexpr auto unary_operation(F&& f, base_tensor<Tensor, T, N> const& t_in) {
  using TOut         = typename std::result_of<decltype(f)(T)>::type;
  auto t_out         = vec<TOut, N>{t_in};
  t_out.unary_operation(std::forward<F>(f));
  return t_out;
}
//------------------------------------------------------------------------------
template <typename F, typename Tensor, typename T, std::size_t M, std::size_t N>
constexpr auto unary_operation(F&&                                 f,
                               base_tensor<Tensor, T, M, N> const& t_in) {
  using TOut = typename std::result_of<decltype(f)(T)>::type;
  auto t_out = mat<TOut, M, N>{t_in};
  t_out.unary_operation(std::forward<F>(f));
  return t_out;
}
//------------------------------------------------------------------------------
template <typename F, typename Tensor, typename T, std::size_t... Dims>
constexpr auto unary_operation(F&&                                    f,
                               base_tensor<Tensor, T, Dims...> const& t_in) {
  using TOut                  = typename std::result_of<decltype(f)(T)>::type;
  tensor<TOut, Dims...> t_out = t_in;
  t_out.unary_operation(std::forward<F>(f));
  return t_out;
}
//==============================================================================
}  // namespace tatooine
//==============================================================================
#endif
