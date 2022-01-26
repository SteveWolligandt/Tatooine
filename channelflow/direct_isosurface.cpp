#include <tatooine/color_scales/magma.h>
#include <tatooine/color_scales/viridis.h>
#include <tatooine/field.h>
#include <tatooine/hdf5.h>
#include <tatooine/line.h>
#include <tatooine/rectilinear_grid.h>
#include <tatooine/rendering/direct_isosurface.h>
#include <tatooine/rendering/perspective_camera.h>

#include <iomanip>
#include <sstream>
//==============================================================================
using namespace tatooine;
//==============================================================================
auto setup_eye_flight(line3& eye, auto const& domain) {
  auto& param = eye.parameterization();

  auto const v0 = eye.push_back(
      vec3{0.6, domain.front(1) - domain.extent(1) / 5, 0.75} * 2 / 3);
  param[v0] = 0;

  auto const v1 = eye.push_back(
      vec3{0.6, domain.back(1) + domain.extent(1) / 5, 0.75} * 2 / 3);
  param[v1] = 1;
}
//------------------------------------------------------------------------------
auto setup_lookat_flight(line3& lookat, auto const& domain) {
  auto&      param = lookat.parameterization();
  auto const v0    = lookat.push_back(vec3{domain.center(0),
                                        domain.front(1) + domain.extent(1) / 6,
                                        domain.center(2)});
  param[v0]        = 0;
  auto const v1    = lookat.push_back(vec3{domain.center(0),
                                        domain.back(1) - domain.extent(1) / 6,
                                        domain.center(2)});
  param[v1]        = 1;
}
//------------------------------------------------------------------------------
auto setup_up_flight(line3& up) {
  auto&      param = up.parameterization();
  auto const v0    = up.push_back(vec3{0, 0, 1});
  param[v0]        = 0;
  auto const v1    = up.push_back(vec3{0, 0, 1});
  param[v1]        = 1;
}
//==============================================================================
auto setup_eye_rotation(line3& eye, auto const& domain) {
  auto& param = eye.parameterization();
  for (auto const t : linspace{0.0, 2 * M_PI, 20}) {
    auto const v =
        eye.push_back(vec3{0.05 + std::cos(t) * 0.5,
                           (domain.front(1) - domain.extent(1) / 5) * 2 / 3,
                           0.05 + std::sin(t) * 0.5});
    param[v] = t / (2 * M_PI);
  }
}
//------------------------------------------------------------------------------
auto setup_lookat_rotation(line3& lookat, auto const& domain) {
  auto& param = lookat.parameterization();
  for (auto const t : linspace{0.0, 2 * M_PI, 20}) {
    auto const v =
        lookat.push_back(vec3{domain.center(0), domain.center(1),
                              // domain.front(1) + domain.extent(1) / 6,
                              domain.center(2)});
    param[v] = t / (2 * M_PI);
  }
}
//------------------------------------------------------------------------------
auto setup_up_rotation(line3& up) {
  auto& param = up.parameterization();
  for (auto const t : linspace{0.0, 2 * M_PI, 20}) {
    // auto const v = up.push_back(vec3{std::sin(t), 0, std::cos(t)});
    auto const v = up.push_back(vec3{0, 0, 1});
    param[v]     = t / (2 * M_PI);
  }
}
//==============================================================================
auto main() -> int {
  auto const path = filesystem::path{"channelflow_velocity_y_154"};
  if (filesystem::exists(path)) {
    filesystem::remove_all(path);
  }
  filesystem::create_directory(path);
  // read full domain axes

  std::cerr << "creating files ...";
  auto channelflow_file =
      hdf5::file{"/home/vcuser/channel_flow/dino_res_154000.h5"};
  std::cerr << "done!\n";

  std::cerr << "loading axes ...";
  auto const axis0 =
      channelflow_file.dataset<double>("CartGrid/axis0").read_as_vector();
  auto const axis1 =
      channelflow_file.dataset<double>("CartGrid/axis1").read_as_vector();
  auto const axis2 =
      channelflow_file.dataset<double>("CartGrid/axis2").read_as_vector();
  std::cerr << "done!\n";

  std::cerr << "creating grids ...";
  auto full_domain = rectilinear_grid{axis0, axis1, axis2};
  full_domain.set_chunk_size_for_lazy_properties(256);
  std::cerr << "full_domain:\n" << full_domain << '\n';
  std::cerr << "done!\n";

  std::cerr << "loading data ...";
  auto& scalar_field = full_domain.insert_vertex_property(
      channelflow_file.dataset<double>("Q_cheng"), "Q_cheng");
  auto& streamwise_velocity = full_domain.insert_vertex_property(
      channelflow_file.dataset<double>("velocity/yvel"), "velocity_y");
  std::cerr << "done!\n";

  std::cerr << "creating samplers ...";
  auto scalar_sampler = scalar_field.linear_sampler();
  auto streamwise_velocity_sampler = streamwise_velocity.linear_sampler();
  std::cerr << "done!\n";

  color_scales::viridis color_scale;

  std::cerr << "creating cameras ...";
  std::size_t const width = 10000, height = 5000;

  auto full_domain_eye    = line3{};
  auto full_domain_lookat = line3{};
  auto full_domain_up     = line3{};

  setup_eye_rotation(full_domain_eye, full_domain);
  setup_lookat_rotation(full_domain_lookat, full_domain);
  setup_up_rotation(full_domain_up);
  // setup_eye_flight(full_domain_eye, full_domain);
  // setup_lookat_flight(full_domain_lookat, full_domain);
  // setup_up_flight(full_domain_up);

  auto full_domain_eye_sampler    = full_domain_eye.linear_sampler();
  auto full_domain_up_sampler     = full_domain_up.linear_sampler();
  auto full_domain_lookat_sampler = full_domain_lookat.linear_sampler();
  std::cerr << "done!\n";

  std::cerr << "calculating min and max scalars ...";
  auto min_scalar = std::numeric_limits<double>::max();
  auto max_scalar = -std::numeric_limits<double>::max();

  full_domain.vertices().iterate_indices([&](auto const... is) {
    min_scalar = std::min(min_scalar, streamwise_velocity(is...));
    max_scalar = std::max(max_scalar, streamwise_velocity(is...));
  });
  auto const medium_scalar = (max_scalar + min_scalar) / 2;
  std::cerr << "done!\n";
  std::cerr << "data range: " << min_scalar << " - " << max_scalar
            << '\n';
  std::size_t       i          = 0;
  std::size_t const num_frames = 100;
  for (auto const t : linspace{0.0, 1.0, num_frames}) {
    std::cerr << "rendering " << i + 1 << " / " << num_frames << "...";
    auto full_domain_cam =
        rendering::perspective_camera{full_domain_eye_sampler(t),
                                      full_domain_lookat_sampler(t),
                                      full_domain_up_sampler(t),
                                      60,
                                      width,
                                      height};
    auto isovalues = std::vector{5e6};
    auto const rendering_grid = rendering::direct_isosurface(
        full_domain_cam, scalar_sampler, isovalues,
        [&](auto const x_iso, auto const isovalue, auto const& gradient,
            auto const& view_dir) {
          auto const normal  = normalize(gradient);
          auto const diffuse = std::abs(dot(view_dir, normal));
          auto const streamwise_velocity = streamwise_velocity_sampler(x_iso);
          auto const t =
              (streamwise_velocity - medium_scalar) / (medium_scalar + max_scalar);
          auto const albedo = color_scale(t);
          auto const col    = albedo * diffuse * 0.8 + albedo * 0.2;
          return vec{col(0), col(1), col(2),
                     std::clamp<double>((t+1)*(t+1)*(t+1), 0.0, 1.0)};
        });
    std::cerr << "done!\n";
    std::cerr << "writing ...";
    std::stringstream str;
    str << std::setw(static_cast<std::size_t>(std::ceil(std::log10(num_frames))))
        << std::setfill('0') << i;
    write_png(path / ("channelflow_velocity_y_154." + str.str() + ".png"),
              rendering_grid.vec3_vertex_property("rendered_isosurface"));
    std::cerr << "done!\n";
    ++i;
  }
}
