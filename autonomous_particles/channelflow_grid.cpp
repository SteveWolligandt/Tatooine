#include <tatooine/agranovsky_flowmap_discretization.h>
#include <tatooine/analytical/fields/numerical/doublegyre.h>
#include <tatooine/autonomous_particle.h>
#include <tatooine/autonomous_particle_flowmap_discretization.h>
#include <tatooine/chrono.h>
#include <tatooine/netcdf.h>
#include <tatooine/progress_bars.h>
#include <tatooine/rectilinear_grid.h>
#include <tatooine/regular_flowmap_discretization.h>
#include <tatooine/unstructured_triangular_grid.h>
#include <tatooine/vtk_legacy.h>

#include <iomanip>
#include <sstream>

#include "parse_args.h"
//==============================================================================
using namespace tatooine;
//==============================================================================
auto main(int argc, char** argv) -> int {
  std::stringstream report;
  report << "==============================================================="
            "=================\n"
         << "REPORT\n"
         << "==============================================================="
            "=================\n";
  auto args_opt = parse_args(argc, argv);
  if (!args_opt) {
    return 1;
  }
  auto args = *args_opt;
  report << "t0: " << args.t0 << '\n' << "tau: " << args.tau << '\n';

  auto channelflow_154_file =
      hdf5::file{args.velocity_file == std::nullopt
                     ? "/home/vcuser/channel_flow/dino_res_154000.h5"
                     : *args.velocity_file};
  auto discrete_channelflow_domain = nonuniform_rectilinear_grid3{};
  channelflow_154_file.dataset<double>("CartGrid/axis0")
      .read(discrete_channelflow_domain.dimension<0>());
  discrete_channelflow_domain.dimension<0>().pop_back();
  channelflow_154_file.dataset<double>("CartGrid/axis1")
      .read(discrete_channelflow_domain.dimension<1>());
  discrete_channelflow_domain.push_back<1>();
  channelflow_154_file.dataset<double>("CartGrid/axis2")
      .read(discrete_channelflow_domain.dimension<2>());
  discrete_channelflow_domain.push_back<2>();
  report << "size of axis0: "
         << size(discrete_channelflow_domain.dimension<0>()) << '\n';
  report << "size of axis1: "
         << size(discrete_channelflow_domain.dimension<1>()) << '\n';
  report << "size of axis2: "
         << size(discrete_channelflow_domain.dimension<2>()) << '\n';
  discrete_channelflow_domain.write("channelflow_grid.vtk");

  indeterminate_progress_bar([&](auto indicator) {
    indicator.set_text("Allocating data for velocity");
    auto& discrete_velocity = *dynamic_cast<
        typed_vertex_property<nonuniform_rectilinear_grid3, vec3,
                              dynamic_multidim_array<vec3, x_fastest>>*>(
        &discrete_channelflow_domain.vec3_vertex_property("velocity"));
    indicator.set_text("Creating sampler");
    auto       w = discrete_velocity.linear_sampler();
    auto const v = make_infinite<1, 2>(w);
    //----------------------------------------------------------------------------
    // Create memory for measuring
    //----------------------------------------------------------------------------
    indicator.set_text("Loading x-velocity");
    {
      auto dataset    = channelflow_154_file.dataset<double>("velocity/xvel");
      auto data_space = dataset.dataspace();
      data_space.select_hyperslab({0, 0, 0},
                                  {discrete_channelflow_domain.size<0>(),
                                   discrete_channelflow_domain.size<1>() - 1,
                                   discrete_channelflow_domain.size<2>() - 1});
      auto mem_space =
          hdf5::dataspace{discrete_channelflow_domain.size<0>() * 3,
                          discrete_channelflow_domain.size<1>() - 1,
                          discrete_channelflow_domain.size<2>() - 1};
      mem_space.select_hyperslab({0, 0, 0}, {3, 1, 1},
                                 {discrete_channelflow_domain.size<0>(),
                                  discrete_channelflow_domain.size<1>() - 1,
                                  discrete_channelflow_domain.size<2>() - 1});
      dataset.read(mem_space.id(), data_space.id(), H5P_DEFAULT,
                   discrete_velocity.data().front().data_ptr());
    }

    indicator.set_text("Loading y-velocity");
    {
      auto dataset    = channelflow_154_file.dataset<double>("velocity/yvel");
      auto data_space = dataset.dataspace();
      data_space.select_hyperslab({0, 0, 0},
                                  {discrete_channelflow_domain.size<0>(),
                                   discrete_channelflow_domain.size<1>() - 1,
                                   discrete_channelflow_domain.size<2>() - 1});
      auto mem_space = hdf5::dataspace{discrete_channelflow_domain.size(0) * 3,
                                       discrete_channelflow_domain.size(1) - 1,
                                       discrete_channelflow_domain.size(2) - 1};
      mem_space.select_hyperslab({1, 0, 0}, {3, 1, 1},
                                 {discrete_channelflow_domain.size(0),
                                  discrete_channelflow_domain.size(1) - 1,
                                  discrete_channelflow_domain.size(2) - 1});

      dataset.read(mem_space.id(), data_space.id(), H5P_DEFAULT,
                   discrete_velocity.data().front().data_ptr());
    }
    indicator.set_text("Loading z-velocity");
    {
      auto dataset    = channelflow_154_file.dataset<double>("velocity/zvel");
      auto data_space = dataset.dataspace();
      data_space.select_hyperslab({0, 0, 0},
                                  {discrete_channelflow_domain.size<0>(),
                                   discrete_channelflow_domain.size<1>() - 1,
                                   discrete_channelflow_domain.size<2>() - 1});
      auto mem_space = hdf5::dataspace{discrete_channelflow_domain.size(0) * 3,
                                       discrete_channelflow_domain.size(1) - 1,
                                       discrete_channelflow_domain.size(2) - 1};
      mem_space.select_hyperslab({2, 0, 0}, {3, 1, 1},
                                 {discrete_channelflow_domain.size(0),
                                  discrete_channelflow_domain.size(1) - 1,
                                  discrete_channelflow_domain.size(2) - 1});

      dataset.read(mem_space.id(), data_space.id(), H5P_DEFAULT,
                   discrete_velocity.data().front().data_ptr());
    }
    indicator.set_text("Creating slabs for infinite domain");
    repeat_for_infinite<1, 2>(discrete_velocity);

    //----------------------------------------------------------------------------
    // Create memory for measuring
    //----------------------------------------------------------------------------
    indicator.set_text("Creating memory for measuring");
    std::vector<real_t> forward_autonomous_errors, forward_regular_errors,
        forward_agranovsky_errors;
    std::vector<real_t> backward_autonomous_errors, backward_regular_errors,
        backward_agranovsky_errors;
    rectilinear_grid sampler_check_grid{
        linspace{discrete_channelflow_domain.front<0>() + 1e-10,
                 discrete_channelflow_domain.back<0>() - 1e-10,
                 args.output_res_x},
        linspace{discrete_channelflow_domain.front<1>() + 1e-10,
                 discrete_channelflow_domain.back<1>() - 1e-10,
                 args.output_res_y},
        linspace{discrete_channelflow_domain.front<2>() + 1e-10,
                 discrete_channelflow_domain.back<2>() - 1e-10,
                 args.output_res_z}};
    forward_autonomous_errors.reserve(sampler_check_grid.vertices().size());
    forward_regular_errors.reserve(sampler_check_grid.vertices().size());
    forward_agranovsky_errors.reserve(sampler_check_grid.vertices().size());
    backward_autonomous_errors.reserve(sampler_check_grid.vertices().size());
    backward_regular_errors.reserve(sampler_check_grid.vertices().size());
    backward_agranovsky_errors.reserve(sampler_check_grid.vertices().size());
    [[maybe_unused]] auto& numerical_flowmap_forward_prop =
        sampler_check_grid.vec3_vertex_property("numerical_flowmap_forward");
    [[maybe_unused]] auto& numerical_flowmap_backward_prop =
        sampler_check_grid.vec3_vertex_property("numerical_flowmap_backward");

    [[maybe_unused]] auto& autonomous_flowmap_forward_prop =
        sampler_check_grid.vec3_vertex_property("autonomous_flowmap_forward");
    [[maybe_unused]] auto& autonomous_flowmap_backward_prop =
        sampler_check_grid.vec3_vertex_property("autonomous_flowmap_backward");
    [[maybe_unused]] auto& forward_errors_autonomous_prop =
        sampler_check_grid.scalar_vertex_property("forward_error_autonomous");
    [[maybe_unused]] auto& forward_errors_regular_prop =
        sampler_check_grid.scalar_vertex_property("forward_error_regular");
    [[maybe_unused]] auto& forward_errors_agranovsky_prop =
        sampler_check_grid.scalar_vertex_property("forward_error_agranovsky");
    [[maybe_unused]] auto& forward_errors_diff_regular_prop =
        sampler_check_grid.scalar_vertex_property("forward_error_diff_regular");
    [[maybe_unused]] auto& forward_errors_diff_agranovsky_prop =
        sampler_check_grid.scalar_vertex_property(
            "forward_error_diff_agranovsky");
    [[maybe_unused]] auto& backward_errors_autonomous_prop =
        sampler_check_grid.scalar_vertex_property("backward_error_autonomous");
    [[maybe_unused]] auto& backward_errors_regular_prop =
        sampler_check_grid.scalar_vertex_property("backward_error_regular");
    [[maybe_unused]] auto& backward_errors_agranovsky_prop =
        sampler_check_grid.scalar_vertex_property("backward_error_agranovsky");
    [[maybe_unused]] auto& backward_errors_diff_regular_prop =
        sampler_check_grid.scalar_vertex_property(
            "backward_error_diff_regular");
    [[maybe_unused]] auto& backward_errors_diff_agranovsky_prop =
        sampler_check_grid.scalar_vertex_property(
            "backward_error_diff_agranovsky");
    real_t mean_autonomous_forward_error  = std::numeric_limits<real_t>::max(),
           mean_regular_forward_error     = std::numeric_limits<real_t>::max(),
           mean_agranovsky_forward_error  = std::numeric_limits<real_t>::max();
    real_t mean_autonomous_backward_error = std::numeric_limits<real_t>::max(),
           mean_regular_backward_error    = std::numeric_limits<real_t>::max(),
           mean_agranovsky_backward_error = std::numeric_limits<real_t>::max();
    size_t     num_points_ood_forward = 0, num_points_ood_backward = 0;
    std::mutex error_mutex;

    //----------------------------------------------------------------------------
    indicator.set_text("Building numerical flowmap");
    auto phi = flowmap(v);
    phi.use_caching(false);

    sampler_check_grid.vertices().iterate_indices(
        [&](auto const... is) {
          auto copy_phi = phi;
          copy_phi.use_caching(false);
          auto const x = sampler_check_grid.vertex_at(is...);
          numerical_flowmap_forward_prop(is...) =
              copy_phi(x, args.t0, args.tau);
           numerical_flowmap_backward_prop(is...) =
              copy_phi(x, args.t0 + args.tau, -args.tau);
        },
        execution_policy::parallel);
    //  //----------------------------------------------------------------------------
    //  indicator.set_text("Discretizing flow map with autonomous particles");
    //  auto num_particles_after_advection = size_t{};
    //  {
    //    auto autonomous_disc = [&] {
    //      if (args.autonomous_particles_file) {
    //        return autonomous_particle_flowmap_discretization2{
    //            *args.autonomous_particles_file};
    //      } else {
    //        return autonomous_particle_flowmap_discretization2{
    //            phi,
    //            args.t0,
    //            args.tau,
    //            args.tau_step,
    //            rectilinear_grid{linspace{0.0, 2.0, args.width + 1},
    //                             linspace{0.0, 1.0, args.height + 1}},
    //        };
    //      }
    //    }();
    //    indicator.set_text(
    //        "Resampling autonomous particle flow map discretization");
    //    num_particles_after_advection = autonomous_disc.num_particles();
    //    sampler_check_grid.vertices().iterate_indices(
    //        [&](auto const... is) {
    //          auto const x = sampler_check_grid.vertex_at(is...);
    //          // forward
    //          try {
    //            auto const x1 = autonomous_disc.sample_forward(x);
    //            autonomous_flowmap_forward_prop(is...) = x1;
    //
    //            auto const err =
    //                euclidean_distance(x1,
    //                numerical_flowmap_forward_prop(is...));
    //            forward_errors_autonomous_prop(is...) = err;
    //            {
    //              std::lock_guard lock{error_mutex};
    //              forward_autonomous_errors.push_back(err);
    //            }
    //          } catch (std::exception const& e) {
    //            autonomous_flowmap_forward_prop(is...) = vec2::ones() * 0.0
    //            / 0.0; forward_errors_autonomous_prop(is...)  = 0.0 / 0.0;
    //          }
    //          // backward
    //          try {
    //            auto const x0 = autonomous_disc.sample_backward(x);
    //            autonomous_flowmap_backward_prop(is...) = x0;
    //
    //            auto const err = euclidean_distance(
    //                x0, numerical_flowmap_backward_prop(is...));
    //            backward_errors_autonomous_prop(is...) = err;
    //            {
    //              std::lock_guard lock{error_mutex};
    //              backward_autonomous_errors.push_back(err);
    //            }
    //          } catch (std::exception const& e) {
    //            autonomous_flowmap_backward_prop(is...) =
    //                vec2::ones() * 0.0 / 0.0;
    //            backward_errors_autonomous_prop(is...) = 0.0 / 0.0;
    //          }
    //        },
    //        execution_policy::parallel);
    //    //----------------------------------------------------------------------------
    //    indicator.set_text("Writing results");
    //    { sampler_check_grid.write("doublegyre_grid_errors.vtk"); }
    //    //----------------------------------------------------------------------------
    //    //indicator.set_text("Writing Autonomous Particles Results");
    //    //{
    //    //  std::vector<line2> all_advected_discretizations;
    //    //  std::vector<line2> all_initial_discretizations;
    //    //  for (auto const& sampler : autonomous_disc.samplers()) {
    //    //    all_initial_discretizations.push_back(
    //    //        discretize(sampler.ellipse0(), 100));
    //    //    all_advected_discretizations.push_back(
    //    //        discretize(sampler.ellipse1(), 100));
    //    //  }
    //    //  write_vtk(all_initial_discretizations,
    //    "doublegyre_grid_ellipses0.vtk");
    //    //  write_vtk(all_advected_discretizations,
    //    //            "doublegyre_grid_ellipses1.vtk");
    //    //}
    //  }
    //  //----------------------------------------------------------------------------
    //  indicator.set_text("Discretizing flow map regularly");
    //  auto const regularized_height = static_cast<size_t>(
    //      std::ceil(std::sqrt(num_particles_after_advection / 2)));
    //  auto const regularized_width = regularized_height * 2;
    //  {
    //    auto regular_disc = regular_flowmap_discretization<real_t, 2>{
    //        phi,        args.t0,           args.tau,          vec2{0, 0},
    //        vec2{2, 1}, regularized_width, regularized_height};
    //    indicator.set_text("Resampling regular flow map discretization");
    //    sampler_check_grid.vertices().iterate_indices(
    //        [&](auto const... is) {
    //          auto const x = sampler_check_grid.vertex_at(is...);
    //          // forward flowmap
    //          try {
    //            auto const x1 = regular_disc.sample_forward(x);
    //            auto const err =
    //                euclidean_distance(x1,
    //                numerical_flowmap_forward_prop(is...));
    //            forward_errors_regular_prop(is...) = err;
    //            {
    //              std::lock_guard lock{error_mutex};
    //              forward_regular_errors.push_back(err);
    //            }
    //
    //          } catch (std::exception const& e) {
    //            forward_errors_regular_prop(is...) =
    //                std::numeric_limits<real_t>::quiet_NaN();
    //          }
    //          forward_errors_diff_regular_prop(is...) =
    //              forward_errors_regular_prop(is...) -
    //              forward_errors_autonomous_prop(is...);
    //          // backward flowmap
    //          try {
    //            auto const x0  = regular_disc.sample_backward(x);
    //            auto const err = euclidean_distance(
    //                x0, numerical_flowmap_backward_prop(is...));
    //            backward_errors_regular_prop(is...) = err;
    //            {
    //              std::lock_guard lock{error_mutex};
    //              backward_regular_errors.push_back(err);
    //            }
    //
    //          } catch (std::exception const& e) {
    //            backward_errors_regular_prop(is...) = 0.0 / 0.0;
    //          }
    //          backward_errors_diff_regular_prop(is...) =
    //              backward_errors_regular_prop(is...) -
    //              backward_errors_autonomous_prop(is...);
    //        },
    //        execution_policy::parallel);
    //    //----------------------------------------------------------------------------
    //    indicator.set_text("Writing results");
    //    { sampler_check_grid.write("doublegyre_grid_errors.vtk"); }
    //  }
    //  //----------------------------------------------------------------------------
    //  auto const num_agranovksy_steps =
    //      static_cast<size_t>(std::ceil(args.agranovsky_delta_t /
    //      args.tau));
    //  auto const regularized_height_agranovksky =
    //  static_cast<size_t>(std::ceil(
    //      std::sqrt((num_particles_after_advection / 2) /
    //      num_agranovksy_steps)));
    //  auto const regularized_width_agranovksky =
    //      regularized_height_agranovksky * 2;
    //  indicator.set_text("Discretizing flow map with agranovsky sampling");
    //  {
    //    auto agranovsky_disc =
    //        AgranovskyFlowmapDiscretization<2>{phi,
    //                                           args.t0,
    //                                           args.tau,
    //                                           args.agranovsky_delta_t,
    //                                           vec2{0, 0},
    //                                           vec2{2, 1},
    //                                           regularized_width_agranovksky,
    //                                           regularized_height_agranovksky};
    //    {
    //      size_t i = 0;
    //      for (auto const& step : agranovsky_disc.steps()) {
    //        step.backward_grid().write_vtk("agranovsky_backward_" +
    //                                       std::to_string(i++) + ".vtk");
    //      }
    //    }
    //    indicator.set_text("Resampling agranovksy flow map discretization");
    //    sampler_check_grid.vertices().iterate_indices(
    //        [&](auto const... is) {
    //          auto const x = sampler_check_grid.vertex_at(is...);
    //          try {
    //            auto const x1 = agranovsky_disc.sample_forward(x);
    //            auto const err =
    //                euclidean_distance(x1,
    //                numerical_flowmap_forward_prop(is...));
    //            forward_errors_agranovsky_prop(is...) = err;
    //            {
    //              std::lock_guard lock{error_mutex};
    //              forward_agranovsky_errors.push_back(err);
    //            }
    //
    //          } catch (std::exception const& e) {
    //            forward_errors_agranovsky_prop(is...) = 0.0 / 0.0;
    //          }
    //          forward_errors_diff_agranovsky_prop(is...) =
    //              forward_errors_agranovsky_prop(is...) -
    //              forward_errors_autonomous_prop(is...);
    //          try {
    //            auto const x0  = agranovsky_disc.sample_backward(x);
    //            auto const err = euclidean_distance(
    //                x0, numerical_flowmap_backward_prop(is...));
    //            backward_errors_agranovsky_prop(is...) = err;
    //            {
    //              std::lock_guard lock{error_mutex};
    //              backward_agranovsky_errors.push_back(err);
    //            }
    //
    //          } catch (std::exception const& e) {
    //            backward_errors_agranovsky_prop(is...) = 0.0 / 0.0;
    //          }
    //          backward_errors_diff_agranovsky_prop(is...) =
    //              backward_errors_agranovsky_prop(is...) -
    //              backward_errors_autonomous_prop(is...);
    //        },
    //        execution_policy::parallel);
    //----------------------------------------------------------------------------
    indicator.set_text("Writing results");
    { sampler_check_grid.write("channelflow_grid_errors.vtk"); }
    //  }
    //
    //  //----------------------------------------------------------------------------
    //  // Compare forward flow map
    //  //----------------------------------------------------------------------------
    //  mean_autonomous_forward_error =
    //      std::accumulate(begin(forward_autonomous_errors),
    //                      end(forward_autonomous_errors), real_t(0)) /
    //      size(forward_autonomous_errors);
    //  mean_regular_forward_error =
    //      std::accumulate(begin(forward_regular_errors),
    //                      end(forward_regular_errors), real_t(0)) /
    //      size(forward_regular_errors);
    //  mean_agranovsky_forward_error =
    //      std::accumulate(begin(forward_agranovsky_errors),
    //                      end(forward_agranovsky_errors), real_t(0)) /
    //      size(forward_agranovsky_errors);
    //  //----------------------------------------------------------------------------
    //  // Compare backward flow map
    //  //----------------------------------------------------------------------------
    //  mean_autonomous_backward_error =
    //      std::accumulate(begin(backward_autonomous_errors),
    //                      end(backward_autonomous_errors), real_t(0)) /
    //      size(backward_autonomous_errors);
    //  mean_regular_backward_error =
    //      std::accumulate(begin(backward_regular_errors),
    //                      end(backward_regular_errors), real_t(0)) /
    //      size(backward_regular_errors);
    //  mean_agranovsky_backward_error =
    //      std::accumulate(begin(backward_agranovsky_errors),
    //                      end(backward_agranovsky_errors), real_t(0)) /
    //      size(backward_agranovsky_errors);
    //  //----------------------------------------------------------------------------
    //  indicator.mark_as_completed();
    //
    //  report << "number of advected autonomous particles:  \n"
    //         << num_particles_after_advection << '\n'
    //
    //         << "number of regular particles:  \n"
    //         << regularized_width * regularized_height << '\n'
    //
    //         << "number of agranovsky particles:  \n"
    //         << num_agranovksy_steps * regularized_width_agranovksky *
    //                regularized_height_agranovksky
    //         << '\n'
    //
    //         //<< num_points_ood_forward << " / "
    //         //<< sampler_check_grid.vertices().size()
    //         //<< " out of domain in forward direction("
    //         //<< (100 * num_points_ood_forward /
    //         //    (real_t)sampler_check_grid.vertices().size())
    //         //<< "%)\n"
    //
    //         //<< num_points_ood_backward << " / "
    //         //<< sampler_check_grid.vertices().size()
    //         //<< " out of domain in backward direction("
    //         //<< (100 * num_points_ood_backward /
    //         //    (real_t)sampler_check_grid.vertices().size())
    //         //<< "%)\n"
    //
    //         << "mean error forward autonomous particles: " <<
    //         std::scientific
    //         << mean_autonomous_forward_error << '\n'
    //
    //         << "mean error forward regular grid: " << std::scientific
    //         << mean_regular_forward_error << '\n'
    //
    //         << "mean error forward agranovsky grid: " << std::scientific
    //         << mean_agranovsky_forward_error << '\n'
    //
    //         << "mean error backward autonomous particles: " <<
    //         std::scientific
    //         << mean_autonomous_backward_error << '\n'
    //
    //         << "mean error backward regular grid: " << std::scientific
    //         << mean_regular_backward_error << '\n'
    //
    //         << "mean error backward agranovsky grid: " << std::scientific
    //         << mean_agranovsky_backward_error << '\n';
    //
    //  if (mean_regular_forward_error > mean_autonomous_forward_error &&
    //      mean_agranovsky_forward_error > mean_autonomous_forward_error) {
    //    report << "autonomous particles are better in forward direction\n";
    //  } else if (mean_agranovsky_forward_error > mean_regular_forward_error
    //  &&
    //             mean_autonomous_forward_error > mean_regular_forward_error)
    //             {
    //    report << "regular grid is better in forward direction\n";
    //  } else if (mean_regular_forward_error > mean_agranovsky_forward_error
    //  &&
    //             mean_autonomous_forward_error >
    //             mean_agranovsky_forward_error)
    //             {
    //    report << "agranovsky is better in forward direction\n";
    //  }
    //
    //  if (mean_regular_backward_error > mean_autonomous_backward_error &&
    //      mean_agranovsky_backward_error > mean_autonomous_backward_error) {
    //    report << "autonomous particles are better in backward direction\n";
    //  } else if (mean_agranovsky_backward_error >
    //  mean_regular_backward_error
    //  &&
    //             mean_autonomous_backward_error >
    //             mean_regular_backward_error)
    //             {
    //    report << "regular grid is better in backward direction\n";
    //  } else if (mean_regular_backward_error >
    //  mean_agranovsky_backward_error
    //  &&
    //             mean_autonomous_backward_error >
    //                 mean_agranovsky_backward_error) {
    //    report << "agranovsky is better in backward direction\n";
    //  }
  });
  std::cerr << report.str();
}