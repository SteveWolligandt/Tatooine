#ifndef TATOOINE_ABCFLOW_H
#define TATOOINE_ABCFLOW_H

#include <cmath>
#include "field.h"

//==============================================================================
namespace tatooine::analytical {
//==============================================================================
/// \brief The Arnold–Beltrami–Childress (ABC) flow is a three-dimensional
///        incompressible velocity field which is an exact solution of Euler's
///        equation.
template <typename real_t>
struct abcflow : field<abcflow<real_t>, real_t, 3, 3> {
  using this_t   = abcflow<real_t>;
  using parent_t = field<this_t, real_t, 3, 3>;
  using typename parent_t::pos_t;
  using typename parent_t::tensor_t;

  //============================================================================
 private:
  real_t m_a, m_b, m_c;

  //============================================================================
 public:
  constexpr abcflow(const real_t a = 1, const real_t b = 1, const real_t c = 1)
      : m_a{a}, m_b{b}, m_c{c} {}
  constexpr abcflow(const abcflow& other)            = default;
  constexpr abcflow(abcflow&& other)                 = default;
  constexpr abcflow& operator=(const abcflow& other) = default;
  constexpr abcflow& operator=(abcflow&& other)      = default;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  constexpr tensor_t evaluate(const pos_t& x, real_t) const {
    return {m_a * std::sin(x(2)) + m_c * std::cos(x(1)),
            m_b * std::sin(x(0)) + m_a * std::cos(x(2)),
            m_c * std::sin(x(1)) + m_b * std::cos(x(0))};
  }
};

abcflow()->abcflow<double>;

//==============================================================================
}  // namespace tatooine::analytical
//==============================================================================

#endif
