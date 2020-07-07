#include <tatooine/analytical/fields/numerical/autonomous_particles_test.h>
#include <tatooine/analytical/fields/numerical/center.h>
#include <tatooine/analytical/fields/numerical/doublegyre.h>
#include <tatooine/analytical/fields/numerical/saddle.h>
#include <tatooine/autonomous_particle.h>
#include <tatooine/spacetime_field.h>
#include <tatooine/concepts.h>
#include <tatooine/vtk_legacy.h>

#include <catch2/catch.hpp>
//==============================================================================
namespace tatooine::test {
//==============================================================================
using namespace analytical::fields::numerical;
//------------------------------------------------------------------------------
template <typename V, std::floating_point VReal, std::floating_point X0Real>
void autonomous_particles_test_backward_integation_distance(
    std::string const& name, vectorfield<V, VReal, 3> const& v,
    vec<X0Real, 3> const& x0, std::floating_point auto const start_radius,
    real_number auto t0, real_number auto t1, real_number auto tau_step) {
  [[maybe_unused]] double const eps = 1e-5;
  std::vector<parameterized_line<VReal, 3, interpolation::linear>>
                            integral_curves;
  autonomous_particle const p0{v, x0, t0, start_radius};
  auto [particles, ellipsoids] = p0.integrate(tau_step, t1);
  std::vector<line<double, 3>> formula_back;
  for (auto const& particle : particles) {
    auto const     total_integration_length = t0 - particle.t1();
    auto&          integral_curve           = integral_curves.emplace_back();
    decltype(auto) phi                      = particle.phi();
    //auto           stepped_back_integration0 =
    //    phi(particle.x1(), particle.t1(), total_integration_length);
    for (auto const tau : linspace<VReal>{
             total_integration_length, VReal{0},
             static_cast<size_t>(50 * std::abs(total_integration_length))}) {
      auto const x = phi(particle.x1(), particle.t1(), tau);
      integral_curve.push_back(vec{x(0), x(1), particle.t1() + tau},
                               particle.t1() + tau);
    }

    auto& back_calc = formula_back.emplace_back();
    back_calc.push_back(vec{particle.x0(0), particle.x0(1), t0});
    back_calc.push_back(vec{particle.x1(0), particle.x1(1), particle.t1()});

  //  {
  //    INFO("Runge Kutta back step in initial circle")
  //    CAPTURE(particle.x1(), particle.t1(), total_integration_length,
  //            particle.x0(), particle.current_radius(),
  //            stepped_back_integration0);
  //    CHECK(tatooine::distance(stepped_back_integration0, p0.x0()) <=
  //          start_radius);
  //  }
  //
  //  {
  //    INFO("Formula back step in initial circle")
  //    CAPTURE(particle.x1(), particle.t1(), total_integration_length,
  //            particle.x0(), particle.current_radius());
  //    CHECK(tatooine::distance(stepped_back_integration0, p0.x0()) <
  //          start_radius);
  //  }
  //
  //  {
  //    INFO("Runge Kutta back step and formula back step approximately equal")
  //    CAPTURE(particle.x1(), particle.t1(), total_integration_length,
  //            particle.x0(), particle.current_radius(),
  //            stepped_back_integration0);
  //    CHECK(tatooine::distance(stepped_back_integration0, particle.x0()) < eps);
  //  }
  }

  write_vtk(integral_curves, name + "_back_integration.vtk");
  write_vtk(formula_back, name + "_formula_back.vtk");
  write_vtk(ellipsoids, name + "_ellipsoids.vtk");
}
//------------------------------------------------------------------------------
template <typename V, std::floating_point VReal, std::floating_point X0Real>
void autonomous_particles_test_backward_integation_distance(
    std::string const& name, vectorfield<V, VReal, 2> const& v,
    vec<X0Real, 2> const& x0, std::floating_point auto const start_radius,
    real_number auto t0, real_number auto t1, real_number auto tau_step) {
  [[maybe_unused]] double const eps = 1e-5;
  std::vector<parameterized_line<VReal, 3, interpolation::linear>>
                            integral_curves;
  autonomous_particle const p0{v, x0, t0, start_radius};
  auto [particles, ellipses] = p0.integrate(tau_step, t1);
  std::vector<line<double, 3>> formula_back;
  for (auto const& particle : particles) {
    auto const     total_integration_length = t0 - particle.t1();
    auto&          integral_curve           = integral_curves.emplace_back();
    decltype(auto) phi                      = particle.phi();
    //auto           stepped_back_integration0 =
    //    phi(particle.x1(), particle.t1(), total_integration_length);
    for (auto const tau : linspace<VReal>{
             total_integration_length, VReal{0},
             static_cast<size_t>(50 * std::abs(total_integration_length))}) {
      auto const x = phi(particle.x1(), particle.t1(), tau);
      integral_curve.push_back(vec{x(0), x(1), particle.t1() + tau},
                               particle.t1() + tau);
    }

    auto& back_calc = formula_back.emplace_back();
    back_calc.push_back(vec{particle.x0(0), particle.x0(1), t0});
    back_calc.push_back(vec{particle.x1(0), particle.x1(1), particle.t1()});

    //{
    //  INFO("Runge Kutta back step in initial circle")
    //  CAPTURE(particle.x1(), particle.t1(), total_integration_length,
    //          particle.x0(), particle.current_radius(),
    //          stepped_back_integration0);
    //  CHECK(tatooine::distance(stepped_back_integration0, p0.x0()) <=
    //        start_radius);
    //}
    //
    //{
    //  INFO("Formula back step in initial circle")
    //  CAPTURE(particle.x1(), particle.t1(), total_integration_length,
    //          particle.x0(), particle.current_radius());
    //  CHECK(tatooine::distance(stepped_back_integration0, p0.x0()) <
    //        start_radius);
    //}
    //
    //{
    //  INFO("Runge Kutta back step and formula back step approximately equal")
    //  CAPTURE(particle.x1(), particle.t1(), total_integration_length,
    //          particle.x0(), particle.current_radius(),
    //          stepped_back_integration0);
    //  CHECK(tatooine::distance(stepped_back_integration0, particle.x0()) < eps);
    //}
  }

  write_vtk(integral_curves, name + "_back_integration.vtk");
  write_vtk(formula_back, name + "_formula_back.vtk");
  write_vtk(ellipses, name + "_ellipses.vtk");
}
//------------------------------------------------------------------------------
template <typename V, std::floating_point VReal, std::floating_point X0Real>
void autonomous_particles_test_backward_integation_distance(
    std::string const& name, vectorfield<V, VReal, 2> const& v,
    std::vector<vec<X0Real, 2>> const& x0s,
    std::floating_point auto const start_radius, real_number auto t0,
    real_number auto t1, real_number auto tau_step) {
  [[maybe_unused]] double const eps = 1e-10;
  std::vector<parameterized_line<VReal, 3, interpolation::linear>>
      integral_curves;
  for (auto const& x0 : x0s) {
    autonomous_particle const p0{v, x0, t0, start_radius};
    auto [particles, ellipses] = p0.integrate(tau_step, t1);
    for (auto const& particle : particles) {
      auto const total_integration_length = t0 - particle.t1();
      if (total_integration_length <= t0 - t1 + 1e-4) {
        auto& integral_curve = integral_curves.emplace_back();
        for (auto const tau :
             linspace<VReal>{total_integration_length, VReal{0}, 10}) {
          auto const x = particle.phi()(particle.x1(), particle.t1(), tau);
          integral_curve.push_back(vec{x(0), x(1), particle.t1() + tau},
                                   particle.t1() + tau);
        }
      }

      auto const back_integration = particle.phi()(particle.x1(), particle.t1(),
                                                   total_integration_length);

      auto const distance = tatooine::distance(back_integration, particle.x0());
      CAPTURE(particle.x1(), particle.t1(), total_integration_length,
              particle.x0(), back_integration, distance);
      CHECK(distance < eps);
    }
  }

  write_vtk(integral_curves, name + "_backintegration.vtk");
}
//==============================================================================
TEST_CASE("autonomous_particle_dg_backward_integration",
          "[autonomous_particle][dg][2d][2D][doublegyre][backward_integration]") {
  doublegyre v;
  v.set_infinite_domain(true);

  // std::vector<vec<double, 2>> x0s;
  // for (size_t y = 1; y < g.size(1) - 1; ++y) {
  //
  //  for (size_t x = 1; x < g.size(0) - 1; ++x) { x0s.push_back(g(x, y)); }
  // 10
  // autonomous_particles_test_backward_integation_distance(
  //    "dg", v, x0s, g.spacing(0), 0, 10, 0.1);
  autonomous_particles_test_backward_integation_distance("dg", v, vec{1.0, 0.5},
                                                         0.001, 0, 10, 0.1);
}
//------------------------------------------------------------------------------
TEST_CASE("autonomous_particle_stdg_backward_integration",
          "[autonomous_particle][stdg][dg][3d][3D][spacetime][doublegyre][backward_integration]") {
  doublegyre v;
  spacetime_field stv{v};
  v.set_infinite_domain(true);

  autonomous_particles_test_backward_integation_distance(
      "stdg", stv, vec{1.0, 0.5, 0.0}, 0.1, 0, 5, 0.1);
}
//------------------------------------------------------------------------------
TEST_CASE("autonomous_particle_space_time_saddle_backward_integration",
          "[autonomous_particle][saddle][3d][3D][spacetime][backward_integration]") {
  saddle v;
  spacetime_field stv{v};

  autonomous_particles_test_backward_integation_distance(
      "spacetime_saddle", stv, vec{0.0, 0.0, 0.0}, 0.1, 0, 1, 0.1);
}
//==============================================================================
TEST_CASE("autonomous_particle_saddle_backward_integration",
          "[autonomous_particle][saddle][2D][2d][backward_integration]") {
  autonomous_particles_test_backward_integation_distance(
      "saddle", saddle{}, vec{0.0, 0.0}, 0.1, 0, 1.0, 0.1);
}
TEST_CASE("autonomous_particle_center_backward_integration",
          "[autonomous_particle][center][2d][2D][backward_integration]") {
  autonomous_particles_test_backward_integation_distance(
      "center", center{}, vec{0.0,0.0}, 0.01, 0, 20, 0.1);
}
//==============================================================================
TEST_CASE("autonomous_particle_test_field_backward_integration",
          "[autonomous_particle][test_field][2d][2D][backward_integration]") {
  autonomous_particles_test_backward_integation_distance(
      "test_field", autonomous_particles_test{}, vec{0.0, 0.0}, 0.001, 0, 10,
      0.1);
}
//==============================================================================
}  // namespace tatooine::test
//==============================================================================
