#include <tatooine/analytical/fields/numerical/abcflow.h>
#include <tatooine/analytical/fields/numerical/doublegyre.h>
#include <tatooine/direct_volume_rendering.h>
#include <tatooine/okubo_weiss_field.h>
#include <tatooine/perspective_camera.h>
#include <tatooine/spacetime_field.h>

#include <catch2/catch.hpp>
//==============================================================================
namespace tatooine::test {
//==============================================================================
TEST_CASE("direct_volume_rendering_doublegyre_magnitude",
          "[direct_volume_rendering][doublegyre][magnitude]") {
  analytical::fields::numerical::doublegyre                  v;
  spacetime_field                                            stv{v};

  boundingbox                bb{vec{0.0, 0.0, 0.0}, vec{2.0, 1.0, 10.0}};
  auto mag = length(stv);
  okubo_weiss_field Q{stv};
  REQUIRE(mag(vec{0.1, 0.1, 0.1}, 0) == length(stv(vec{0.1, 0.1, 0.1}, 0)));
  size_t const width = 1000, height = 1000;
  perspective_camera<double> cam{vec{-1, 2, -3}, vec{0.5, 0.5, 0.0}, 60, width,
                                 height};
  auto mag_grid = direct_volume_rendering(cam, bb, mag, 0, 1, 1.1, 0.01);
  auto Q_grid = direct_volume_rendering(cam, bb, Q, 0, 0.1, 1.1, 0.01);
  write_png("direct_volume_stdg_mag.png",
            mag_grid.vertex_property<double>("rendering"), width, height);
  write_png("direct_volume_stdg_Q.png",
            Q_grid.vertex_property<double>("rendering"), width, height);
}

//==============================================================================
TEST_CASE("direct_volume_rendering_abc_magnitude",
          "[direct_volume_rendering][abc][magnitude]") {
  analytical::fields::numerical::abcflow                     v;
  grid<linspace<double>, linspace<double>, linspace<double>> g{
      linspace{-1.0, 1.0, 200},
      linspace{-1.0, 1.0, 200},
      linspace{-1.0, 1.0, 200}};
  auto& mag =
      g.add_contiguous_vertex_property<double, x_fastest, interpolation::linear,
                                       interpolation::linear,
                                       interpolation::linear>("mag");
  double min = std::numeric_limits<double>::max(),
         max = -std::numeric_limits<double>::max();
  g.loop_over_vertex_indices([&](auto const... is) {
    mag.container().at(is...) = length(v(g.vertex_at(is...), 0));
    min                       = std::min(mag.container().at(is...), min);
    max                       = std::max(mag.container().at(is...), max);
  });
  size_t const width = 1000, height = 1000;
  perspective_camera<double> cam{vec{40, 50, 50}, vec{0.0, 0.0, 0.0}, 60, width,
                                 height};
  std::cerr << "max: " << max << '\n';
  auto rendered_grid = direct_volume_rendering(cam, mag, 1, max, 0.01);
  write_png("direct_volume_abc_mag.png",
            rendered_grid.vertex_property<double>("rendering"), width, height);
}
//==============================================================================
}  // namespace tatooine::test
//==============================================================================