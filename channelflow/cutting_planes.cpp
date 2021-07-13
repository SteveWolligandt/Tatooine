#include <tatooine/color_scales/magma.h>
#include <tatooine/color_scales/viridis.h>
#include <tatooine/field.h>
#include <tatooine/grid.h>
#include <tatooine/hdf5.h>
#include <tatooine/line.h>
#include <tatooine/rendering/direct_isosurface.h>
#include <tatooine/rendering/perspective_camera.h>
//==============================================================================
using namespace tatooine;
//==============================================================================
auto main() -> int {
  auto const path = filesystem::path{"channelflow_velocity_y_154"};
  if (filesystem::exists(path)) {
    filesystem::remove_all(path);
  }
  filesystem::create_directory(path);
  // read full domain axes
  std::cerr << "loading axes ...";
  auto const axis0 = hdf5::file{"/home/vcuser/channel_flow/axis0.h5"}
                         .dataset<double>("CartGrid/axis0")
                         .read_as_vector();
  auto const axis1 = hdf5::file{"/home/vcuser/channel_flow/axis1.h5"}
                         .dataset<double>("CartGrid/axis1")
                         .read_as_vector();
  auto const axis2 = hdf5::file{"/home/vcuser/channel_flow/axis2.h5"}
                         .dataset<double>("CartGrid/axis2")
                         .read_as_vector();
  std::cerr << "done!\n";

  std::cerr << "creating grids ...";
  grid full_domain{axis0, axis1, axis2};
  full_domain.set_chunk_size_for_lazy_properties(256);
  std::cerr << "full_domain:\n" << full_domain << '\n';

  auto axis0_Q = axis0;
  axis0_Q.pop_back();
  grid full_domain_Q{axis0_Q, axis1, axis2};
  full_domain_Q.set_chunk_size_for_lazy_properties(256);
  std::cerr << "full_domain_Q:\n" << full_domain_Q << '\n';
  std::cerr << "done!\n";

  std::cerr << "creating files ...";
  hdf5::file channelflow_122_full_file{
      "/home/vcuser/channel_flow/dino_res_122000_full.h5"};
  hdf5::file channelflow_154_full_file{
      "/home/vcuser/channel_flow/dino_res_154000.h5"};
  std::cerr << "done!\n";

  std::cerr << "loading data ...";
  auto& velocity_y_154_full = full_domain.insert_vertex_property(
      channelflow_154_full_file.dataset<double>("velocity/yvel"),
      "velocity_y_154");
  std::cerr << "done!\n";

  std::cerr << "creating samplers ...";
  auto velocity_y_154_full_sampler = velocity_y_154_full.linear_sampler();
  std::cerr << "done!\n";

  auto const basis = mat<real_t, 2, 3>{};
  basis.col(0)     = vec{0, 1, 0};
  basis.col(1)     = vec{0, 0, 1};
  auto d = discretize(stv, basis, vec3{axis0[511], 0, 0}, vec2{axis1.back(), axis2.back()},
                      size(axis1), size(axis2), "512", 0);
  d.write_vtk("cutting_plane_512.vtk", basis);
}
