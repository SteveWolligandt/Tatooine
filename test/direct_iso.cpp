#include <tatooine/analytical/fields/numerical/doublegyre.h>
#include <tatooine/direct_iso.h>
#include <tatooine/grid.h>
#include <tatooine/line.h>
#include <tatooine/rendering/perspective_camera.h>
#include <tatooine/spacetime_vectorfield.h>

#include <catch2/catch.hpp>
//==============================================================================
namespace tatooine::test{
//==============================================================================
TEST_CASE("direct_iso_grid_vertex_sampler") {
  auto v = analytical::fields::numerical::doublegyre {};
  auto vst = spacetime_vectorfield{v};
  auto mag_vst = length(vst);

  auto g =
      grid{linspace{0.0, 2.0, 200}, linspace{0.0, 1.0, 100}, linspace{0.0, 10.0, 1000}};
  auto& s       = discretize(mag_vst, g, "s", 0);
  g.write_vtk("discretized_iso_volume.vtk");
  auto iso = 1.05;
  parameterized_line<real_t, 3, interpolation::cubic> l;
  l.push_back(vec3{-2, 4, -2}, 0);
  l.push_back(vec3{-6, 4, 5}, 0.5);
  l.push_back(vec3{-2, 4, 12}, 1);
  parameterized_line<real_t, 3, interpolation::linear> l2;
  l2.push_back(vec3{1, 0, 3}, 0);
  l2.push_back(vec3{1, 0, 7}, 1);

  auto  sampler = s.linear_sampler();
  size_t i = 0;
  auto const ts = linspace{0.0, 1.0, 100};
   for (auto const t : ts)
  {
     rendering::perspective_camera cam{l(t), l2(t), vec3{0, 1, 0},
                                       60,   1920,  1080};

     std::stringstream ss;
     ss << std::setfill('0') << std::setw(2) << i;
     direct_iso(cam, sampler, iso,
                [](auto const& x_iso, auto const& gradient,
                   auto const& view_dir, auto const t) {

                  auto const normal      = normalize(gradient);
                  auto const diffuse     = std::abs(dot(view_dir, normal));
                  auto const reflect_dir = reflect(-view_dir, normal);
                  auto const spec_dot =
                      std::max(std::abs(dot(reflect_dir, view_dir)), 0.0);
                  auto const specular = std::pow(spec_dot, 100);
                  auto const albedo = vec3{1,0,0};
                  return  albedo  * diffuse + specular;
                })
         .vertex_property<vec3>("rendering")
         .write_png("direct_iso." + ss.str() + ".png");
     ++i;
  }
}
//==============================================================================
}  // namespace tatooine::test
//==============================================================================
