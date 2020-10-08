#ifndef TATOOINE_AUTONOMOUS_PARTICLES_2D_H
#define TATOOINE_AUTONOMOUS_PARTICLES_2D_H
//==============================================================================
#include <tatooine/autonomous_particle.h>
#include <tatooine/numerical_flowmap.h>
#include <tatooine/field.h>
#include <tatooine/geometry/sphere.h>
#include <tatooine/tensor.h>

//==============================================================================
namespace tatooine {
//==============================================================================
template <fixed_dims_flowmap_c<2> Flowmap>
struct autonomous_particle<Flowmap> {
  //----------------------------------------------------------------------------
  // typedefs
  //----------------------------------------------------------------------------
 public:
  using this_t    = autonomous_particle<Flowmap>;
  using flowmap_t = std::decay_t<Flowmap>;
  static constexpr auto num_dimensions() {
    return flowmap_t::num_dimensions();
  }
  using real_t  = typename flowmap_t::real_t;
  using vec_t   = vec<real_t, num_dimensions()>;
  using mat_t   = mat<real_t, num_dimensions(), num_dimensions()>;
  using diff1_t = mat_t;
  using pos_t   = vec_t;


  //----------------------------------------------------------------------------
  // members
  //----------------------------------------------------------------------------
 private:
  flowmap_t m_phi;
  pos_t     m_x0, m_x1;
  real_t    m_t1;
  mat_t     m_nabla_phi1;
  diff1_t   m_S;

  //----------------------------------------------------------------------------
  // ctors
  //----------------------------------------------------------------------------
 public:
  autonomous_particle(autonomous_particle const&) = default;
  autonomous_particle(autonomous_particle &&) noexcept = default;
  auto operator=(autonomous_particle const&)
    -> autonomous_particle& = default;
  auto operator=(autonomous_particle&&) noexcept
    -> autonomous_particle& = default;
  template <typename = void>
    requires is_numerical_flowmap_v<Flowmap>
  autonomous_particle()
      : autonomous_particle{Flowmap{}, pos_t::zeros(), real_t(0), real_t(0)} {}
  //----------------------------------------------------------------------------
  template <typename V, std::floating_point VReal, real_number RealX0>
  autonomous_particle(vectorfield<V, VReal, 2>const &      v,
                      vec<RealX0, num_dimensions()> const& x0,
                      real_number auto const t0, real_number auto const r0)
      : autonomous_particle{flowmap(v), x0, t0, r0} {}
  //----------------------------------------------------------------------------
  template <real_number RealX0>
  autonomous_particle(flowmap_t phi, tensor<RealX0, num_dimensions()> const& x0,
                      real_number auto const t0, real_number auto const r0)
      : m_phi{std::move(phi)},
        m_x0{x0},
        m_x1{x0},
        m_t1{static_cast<real_t>(t0)},
        m_nabla_phi1{mat_t::eye()},
        m_S{mat_t::eye() * r0} {}
  //----------------------------------------------------------------------------
  autonomous_particle(flowmap_t phi, pos_t const& x0, pos_t const& x1,
                      real_t const t1, mat_t const& nabla_phi1, mat_t const& S)
      : m_phi{std::move(phi)},
        m_x0{x0},
        m_x1{x1},
        m_t1{t1},
        m_nabla_phi1{nabla_phi1},
        m_S{S} {}
  //----------------------------------------------------------------------------
 public:
  //----------------------------------------------------------------------------
  // getter
  //----------------------------------------------------------------------------
  auto x0() -> auto & {
    return m_x0;
  }
  auto x0() const -> auto const& {
    return m_x0;
  }
  auto x0(size_t i) const {
    return m_x0(i);
  }
  auto x1() -> auto & {
    return m_x1;
  }
  auto x1() const -> auto const& {
    return m_x1;
  }
  auto x1(size_t i) const {
    return m_x1(i);
  }
  auto t1() -> auto& {
    return m_t1;
  }
  auto t1() const {
    return m_t1;
  }
  auto nabla_phi1() const -> auto const& {
    return m_nabla_phi1;
  }
  auto S() -> auto& {
    return m_S;
  }
  auto S() const -> auto const& {
    return m_S;
  }
  auto phi() const -> auto const& {
    return m_phi;
  }
  auto phi() -> auto& {
    return m_phi;
  }

  //----------------------------------------------------------------------------
  // methods
  //----------------------------------------------------------------------------
 public:
  auto integrate(real_t tau_step, real_t const max_t, bool const& stop = false) const {
    std::array particles{std::vector<this_t>{*this}, std::vector<this_t>{}};
    size_t     active = 0;
    real_t tau  = 0;
    while (tau < max_t && ! stop) {
      if (tau + tau_step > max_t) {
        tau_step = max_t - tau;
      }
      tau += tau_step;
      particles[1 - active].clear();
      for (auto const& particle : particles[active]) {
        if (stop) {
          break;
        }
        auto new_particles = particle.step_until_split5(tau_step);
        std::move(begin(new_particles), end(new_particles),
                  std::back_inserter(particles[1 - active]));
      }
      active = 1 - active;
    } 
    return particles[active];
  }
 private:
  //----------------------------------------------------------------------------
  auto calc_B() const{
    auto const [Q, lambdas] = eigenvectors_sym(m_S);
    auto const sigma        = diag(lambdas);
    return Q * sigma;
  }
  auto calc_H(real_t const tau, mat_t const& B) const {
    // n stands for negative offset, p for positive offset
    auto const o_p0 = B * vec_t{ 1,  0};
    auto const o_n0 = B * vec_t{-1,  0};
    auto const o_0p = B * vec_t{ 0,  1};
    auto const o_0n = B * vec_t{ 0, -1};

    // integrate ghost particles
    auto const p_p0 = m_phi(m_x1 + o_p0, m_t1, tau);
    auto const p_n0 = m_phi(m_x1 + o_n0, m_t1, tau);
    auto const p_0p = m_phi(m_x1 + o_0p, m_t1, tau);
    auto const p_0n = m_phi(m_x1 + o_0n, m_t1, tau);

    mat_t H;
    H.col(0) = p_p0 - p_n0;
    H.col(1) = p_0p - p_0n;
    H /= 2;
    return H;
  }
  //----------------------------------------------------------------------------
  auto step_until_split2(real_t tau) const -> std::vector<this_t>  {
    auto const [Q, lambdas]    = eigenvectors_sym(m_S);
    auto const sigma           = diag(lambdas);
    auto const B               = Q * sigma;
    auto const H               = calc_H(tau, B);
    auto const nabla_phi2      = H * inv(sigma) * transposed(Q);
    auto const advected_center = m_phi(m_x1, m_t1, tau);
    auto const HHt             = H * transposed(H);
    auto const [eigvecs_HHt, eigvals_HHt] = eigenvectors_sym(HHt);
    auto const   cond     = eigvals_HHt(1) / eigvals_HHt(0);
    mat_t const  fmg2fmg1 = nabla_phi2 * m_nabla_phi1;
    real_t const t2       = m_t1 + tau;

    static auto const objective_cond = 2;
    if (cond >= objective_cond) {
      vec const   new_eigvals{std::sqrt(eigvals_HHt(1)) / 2,
                            std::sqrt(eigvals_HHt(1)) / 2};
      vec_t const offset2 = eigvecs_HHt.col(1) * std::sqrt(eigvals_HHt(1)) / 2;
      vec_t const offset0 = inv(fmg2fmg1) * offset2;
      auto const  new_S =
          eigvecs_HHt * diag(new_eigvals) * transposed(eigvecs_HHt);
      return {{m_phi, m_x0 - offset0, advected_center - offset2, t2, fmg2fmg1, new_S},
              {m_phi, m_x0 + offset0, advected_center + offset2, t2, fmg2fmg1, new_S}};
    } else {
      vec const foo{std::sqrt(eigvals_HHt(0)), std::sqrt(eigvals_HHt(1))};
      return {{m_phi, m_x0, advected_center, t2, fmg2fmg1,
               eigvecs_HHt * diag(foo) * transposed(eigvecs_HHt)}};
    }
  }
  //----------------------------------------------------------------------------
  auto step_until_split3(real_t tau) const -> std::vector<this_t> {
    auto const [Q, lambdas]    = eigenvectors_sym(m_S);
    auto const sigma           = diag(lambdas);
    auto const B               = Q * sigma;
    auto const H               = calc_H(tau, B);
    auto const nabla_phi2      = H * inv(sigma) * transposed(Q);
    auto const advected_center = m_phi(m_x1, m_t1, tau);
    auto const HHt             = H * transposed(H);
    auto const [eigvecs_HHt, eigvals_HHt] = eigenvectors_sym(HHt);
    auto const   cond     = eigvals_HHt(1) / eigvals_HHt(0);
    mat_t const  fmg2fmg1 = nabla_phi2 * m_nabla_phi1;
    real_t const t2       = m_t1 + tau;

    auto objective_cond = 4;
    if (cond >= objective_cond) {
      vec const   new_eigvals_inner{std::sqrt(eigvals_HHt(0)),
                                  std::sqrt(eigvals_HHt(1)) / 2};
      auto const  new_eigvals_outer = new_eigvals_inner / 2;
      vec_t const offset2 =
          std::sqrt(eigvals_HHt(1)) * eigvecs_HHt.col(1) * 3 / 4;
      vec_t const offset0 = inv(fmg2fmg1) * offset2;
      auto const  new_S_inner =
          eigvecs_HHt * diag(new_eigvals_inner) * transposed(eigvecs_HHt);
      auto const new_S_outer =
          eigvecs_HHt * diag(new_eigvals_outer) * transposed(eigvecs_HHt);
      return {
          {m_phi, m_x0 - offset0, advected_center - offset2, t2, fmg2fmg1, new_S_outer},
          {m_phi, m_x0, advected_center, t2, fmg2fmg1, new_S_inner},
          {m_phi, m_x0 + offset0, advected_center + offset2, t2, fmg2fmg1, new_S_outer}};
    } else {
      vec const foo{std::sqrt(eigvals_HHt(0)), std::sqrt(eigvals_HHt(1))};
      return {{m_phi, m_x0, advected_center, t2, fmg2fmg1,
               eigvecs_HHt * diag(foo) * transposed(eigvecs_HHt)}};
    }
  }
  //----------------------------------------------------------------------------
  auto step_until_split5(real_t tau) const -> std::vector<this_t> {
    auto const [Q, lambdas]               = eigenvectors_sym(m_S);
    auto const sigma                      = diag(lambdas);
    auto const B                          = Q * sigma;
    auto const H                          = calc_H(tau, B);
    auto const nabla_phi2                 = H * inv(sigma) * transposed(Q);
    auto const advected_center            = m_phi(m_x1, m_t1, tau);
    auto const HHt                        = H * transposed(H);
    auto const [eigvecs_HHt, eigvals_HHt] = eigenvectors_sym(HHt);
    auto const cond                       = eigvals_HHt(1) / eigvals_HHt(0);
    auto const fmg2fmg1                   = nabla_phi2 * m_nabla_phi1;
    auto const t2                         = m_t1 + tau;
    auto const relative_1                 = std::sqrt(eigvals_HHt(0));

    static auto const sqrt5          = std::sqrt(5);
    auto              objective_cond = 6 + 2 * sqrt5;
    if (cond >= objective_cond) {
      static auto const middle_radius = (sqrt5 + 3) / (2 * sqrt5 + 2);
      static auto const outer_radius  = 1 / (sqrt5 + 1);
      // inner circle
      vec const  new_eigvals_inner{relative_1, relative_1};
      auto const new_S_inner =
          eigvecs_HHt * diag(new_eigvals_inner) * transposed(eigvecs_HHt);

      // middle circles
      auto const new_eigval_mid = relative_1 * middle_radius;
      vec const  new_eigvals_mid{new_eigval_mid, new_eigval_mid};

      vec_t const offset2_mid =
          eigvecs_HHt.col(1) * relative_1 * (1 + middle_radius);
      vec_t const offset0_mid = inv(fmg2fmg1) * offset2_mid;
      auto const  new_S_mid =
          eigvecs_HHt * diag(new_eigvals_mid) * transposed(eigvecs_HHt);

      // outer cirlces
      auto const  new_eigval_outer = relative_1 * outer_radius;
      vec const   new_eigvals_outer{new_eigval_outer, new_eigval_outer};
      vec_t const offset2_outer = eigvecs_HHt.col(1) * relative_1 *
                                  (1 + 2 * middle_radius + outer_radius);
      vec_t const offset0_outer = inv(fmg2fmg1) * offset2_outer;
      auto const  new_S_outer =
          eigvecs_HHt * diag(new_eigvals_outer) * transposed(eigvecs_HHt);

      return {{m_phi, m_x0, advected_center, t2, fmg2fmg1, new_S_inner},
              {m_phi, m_x0 - offset0_mid, advected_center - offset2_mid, t2, fmg2fmg1,
               new_S_mid},
              {m_phi, m_x0 + offset0_mid, advected_center + offset2_mid, t2, fmg2fmg1,
               new_S_mid},
              {m_phi, m_x0 - offset0_outer, advected_center - offset2_outer, t2, fmg2fmg1,
               new_S_outer},
              {m_phi, m_x0 + offset0_outer, advected_center + offset2_outer, t2, fmg2fmg1,
               new_S_outer}};
    } else {
      vec const new_eig_vals{std::sqrt(eigvals_HHt(0)),
                             std::sqrt(eigvals_HHt(1))};
      return {{m_phi, m_x0, advected_center, t2, fmg2fmg1,
               eigvecs_HHt * diag(new_eig_vals) * transposed(eigvecs_HHt)}};
    }
  }
};
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// deduction guides
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <typename V, typename VReal, real_number RealX0, size_t N>
autonomous_particle(const vectorfield<V, VReal, 2>& v, vec<RealX0, N> const&,
                    real_number auto const, real_number auto const)
    -> autonomous_particle<decltype(flowmap(v))>;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template <fixed_dims_flowmap_c<2> Flowmap, real_number RealX0, size_t N>
autonomous_particle(const Flowmap& flowmap, vec<RealX0, N> const&,
                    real_number auto const, real_number auto const)
    -> autonomous_particle<Flowmap>;
//==============================================================================
}  // namespace tatooine
//==============================================================================
#endif
