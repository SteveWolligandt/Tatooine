#include<tatooine/geometry/sphere.h>
#include<tatooine/chrono.h>
#include<tatooine/rendering/perspective_camera.h>
#include<tatooine/rendering/raytracing/render.h>
//==============================================================================
using namespace tatooine;
//==============================================================================
using mesh_t = triangular_mesh<real_t, 3>;
using cam_t  = rendering::perspective_camera<real_t>;
//==============================================================================
auto main(int const argc, char const** argv) -> int {
  auto const resolution_x = std::size_t(500);
  auto const resolution_y = std::size_t(500);
  auto const fov          = 50.0;
  auto const near         = 0.1;
  auto const far          = 100.0;
  auto const eye          = vec3{0, 1, -5};
  auto const lookat       = vec3::zeros();
  auto const cam =
      cam_t{eye, lookat, fov, near, far, resolution_x, resolution_y};
  auto const mesh = discretize(geometry::sphere3{1}, 3);
  auto const hierarchy_build_duration =
      measure([&] { mesh.build_hierarchy(); });
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(
                   hierarchy_build_duration)
                   .count() << '\n';

  auto const [rendering_duration, image] =
      measure([&] { return rendering::raytracing::render(cam, mesh); });

  write_png(image.vertex_property<vec3>("image"), argv[1]);
}
