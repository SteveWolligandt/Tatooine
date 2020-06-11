#ifndef TATOOINE_ANALYTICAL_FIELDS_SYMBOLIC_COUNTEREXAMPLE_SADLO_H
#define TATOOINE_ANALYTICAL_FIELDS_SYMBOLIC_COUNTEREXAMPLE_SADLO_H
#include <tatooine/packages.h>
#if TATOOINE_GINAC_AVAILABLE
#include <tatooine/symbolic_field.h>
//==============================================================================
namespace tatooine::analytical::fields::symbolic {
//==============================================================================
template <typename Real>
struct counterexample_sadlo : field<Real, 2, 2> {
  using this_t   = counterexample_sadlo<Real>;
  using parent_t = field<Real, 2, 2>;
  using parent_t::t;
  using parent_t::x;
  using typename parent_t::pos_t;
  using typename parent_t::symtensor_t;
  using typename parent_t::tensor_t;

  counterexample_sadlo() {
    auto r =
        -GiNaC::numeric{1, 80} *
            GiNaC::power(GiNaC::power(x(0), 2) + GiNaC::power(x(1), 2), 2) +
        GiNaC::numeric{81, 80};

    this->set_expr(vec{r * (-GiNaC::numeric{1, 2} * x(0) +
                            GiNaC::numeric{1, 2} * cos(t()) - sin(t())),
                       r * (GiNaC::numeric{1, 2} * x(1) -
                            GiNaC::numeric{1, 2} * sin(t()) + cos(t()))});
  }

  constexpr bool in_domain(const pos_t& x, Real /*t*/) const {
    return length(x) <= 3;
  }
};
//==============================================================================
counterexample_sadlo()->counterexample_sadlo<double>;
//==============================================================================
}  // namespace tatooine::symbolic
//==============================================================================
#endif
#endif