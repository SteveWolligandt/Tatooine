#ifndef TATOOINE_TENSOR_IO_H
#define TATOOINE_TENSOR_IO_H
//==============================================================================
#include <tatooine/tensor.h>
#include <ostream>
//==============================================================================
namespace tatooine {
//==============================================================================
/// printing vector
template <typename Tensor, typename Real, size_t N>
auto operator<<(std::ostream& out, const base_tensor<Tensor, Real, N>& v)
    -> auto& {
  out << "[ ";
  out << std::scientific;
  for (size_t i = 0; i < N; ++i) {
    if constexpr (!is_complex_v<Real>) {}
    out << v(i) << ' ';
  }
  out << "]";
  out << std::defaultfloat;
  return out;
}

template <typename Tensor, typename Real, size_t M, size_t N>
auto operator<<(std::ostream& out, const base_tensor<Tensor, Real, M, N>& m)
    -> auto& {
  out << std::scientific;
  for (size_t j = 0; j < M; ++j) {
    out << "[ ";
    for (size_t i = 0; i < N; ++i) {
      if constexpr (!is_complex_v<Real>) {
        if (m(j, i) >= 0) { out << ' '; }
      }
      out << m(j, i) << ' ';
    }
    out << "]\n";
  }
  out << std::defaultfloat;
  return out;
}
//==============================================================================
}  // namespace tatooine
//==============================================================================
#endif
