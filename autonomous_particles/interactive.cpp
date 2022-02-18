#include <tatooine/agranovsky_flowmap_discretization.h>
#include <tatooine/analytical/fields/doublegyre.h>
#include <tatooine/analytical/fields/saddle.h>
#include <tatooine/autonomous_particle_flowmap_discretization.h>
#include <tatooine/rendering/interactive.h>
#include <tatooine/rendering/interactive/shaders.h>
//==============================================================================
using namespace tatooine;
//==============================================================================
using autonomous_particle_flowmap_type =
    AutonomousParticleFlowmapDiscretization<
        2, AutonomousParticle<2>::split_behaviors::five_splits>;
//==============================================================================
struct vis {
  using pass_through =
      rendering::interactive::shaders::colored_pass_through_2d_without_matrices;
  struct hoverable_shader : gl::shader {
    //------------------------------------------------------------------------------
    static constexpr std::string_view vertex_shader =
        "#version 330 core\n"
        "layout (location = 0) in vec2 position;\n"
        "layout (location = 1) in int hovered;\n"
        "flat out int frag_hovered;\n"
        "uniform mat4 view_matrix;\n"
        "uniform mat4 projection_matrix;\n"
        "void main() {\n"
        "  frag_hovered = hovered;\n"
        "  gl_Position = projection_matrix *\n"
        "                view_matrix *\n"
        "                vec4(position, 0, 1);\n"
        "}\n";
    //------------------------------------------------------------------------------
    static constexpr std::string_view fragment_shader =
        "#version 330 core\n"
        "uniform vec4 color;\n"
        "out vec4 out_color;\n"
        "flat in int frag_hovered;\n"
        "void main() {\n"
        "  if (frag_hovered == 1) {\n"
        "    discard;\n"
        "  }\n"
        "  out_color = vec4(frag_hovered, 0, 0, 1);\n"
        "}\n";
    //------------------------------------------------------------------------------
    static auto get() -> auto& {
      static auto s = hoverable_shader{};
      return s;
    }
    //------------------------------------------------------------------------------
   private:
    //------------------------------------------------------------------------------
    hoverable_shader() {
      add_stage<gl::vertexshader>(gl::shadersource{vertex_shader});
      add_stage<gl::fragmentshader>(gl::shadersource{fragment_shader});
      create();
      set_color(0, 0, 0);
      set_projection_matrix(Mat4<GLfloat>::eye());
      set_view_matrix(Mat4<GLfloat>::eye());
    }
    //------------------------------------------------------------------------------
   public:
    //------------------------------------------------------------------------------
    auto set_color(GLfloat const r, GLfloat const g, GLfloat const b,
                   GLfloat const a = 1) -> void {
      set_uniform("color", r, g, b, a);
    }
    //------------------------------------------------------------------------------
    auto set_projection_matrix(Mat4<GLfloat> const& P) -> void {
      set_uniform_mat4("projection_matrix", P.data().data());
    }
    //------------------------------------------------------------------------------
    auto set_view_matrix(Mat4<GLfloat> const& MV) -> void {
      set_uniform_mat4("view_matrix", MV.data().data());
    }
  };
  struct only_hovered_shader : gl::shader {
    //------------------------------------------------------------------------------
    static constexpr std::string_view vertex_shader =
        "#version 330 core\n"
        "layout (location = 0) in vec2 position;\n"
        "layout (location = 1) in int hovered;\n"
        "flat out int frag_hovered;\n"
        "uniform mat4 view_matrix;\n"
        "uniform mat4 projection_matrix;\n"
        "void main() {\n"
        "  frag_hovered = hovered;\n"
        "  gl_Position = projection_matrix *\n"
        "                view_matrix *\n"
        "                vec4(position, 0, 1);\n"
        "}\n";
    //------------------------------------------------------------------------------
    static constexpr std::string_view fragment_shader =
        "#version 330 core\n"
        "uniform vec4 color;\n"
        "out vec4 out_color;\n"
        "flat in int frag_hovered;\n"
        "void main() {\n"
        "  if (frag_hovered == 0) {\n"
        "    discard;\n"
        "  }\n"
        "  out_color = vec4(frag_hovered, 0, 0, 1);\n"
        "}\n";
    //------------------------------------------------------------------------------
    static auto get() -> auto& {
      static auto s = only_hovered_shader{};
      return s;
    }
    //------------------------------------------------------------------------------
   private:
    //------------------------------------------------------------------------------
    only_hovered_shader() {
      add_stage<gl::vertexshader>(gl::shadersource{vertex_shader});
      add_stage<gl::fragmentshader>(gl::shadersource{fragment_shader});
      create();
      set_color(0, 0, 0);
      set_projection_matrix(Mat4<GLfloat>::eye());
      set_view_matrix(Mat4<GLfloat>::eye());
    }
    //------------------------------------------------------------------------------
   public:
    //------------------------------------------------------------------------------
    auto set_color(GLfloat const r, GLfloat const g, GLfloat const b,
                   GLfloat const a = 1) -> void {
      set_uniform("color", r, g, b, a);
    }
    //------------------------------------------------------------------------------
    auto set_projection_matrix(Mat4<GLfloat> const& P) -> void {
      set_uniform_mat4("projection_matrix", P.data().data());
    }
    //------------------------------------------------------------------------------
    auto set_view_matrix(Mat4<GLfloat> const& V) -> void {
      set_uniform_mat4("view_matrix", V.data().data());
    }
  };
  //----------------------------------------------------------------------------
 private:
  //----------------------------------------------------------------------------
  uniform_rectilinear_grid<double, 2>     grid;
  double                                  agranovsky_error           = 0;
  double                                  autonomous_particles_error = 0;
  vec2d                                   cursor_pos;
  vec2d                                   current_point;
  std::vector<bool>                       hovered;
  int                                     point_size = 20;
  rendering::orthographic_camera<GLfloat> cam;
  std::vector<autonomous_particle2::sampler_type> const& samplers;
  std::vector<vec2>                                      locals;
  gl::indexeddata<Vec2<GLfloat>, int>                    locals_gpu;
  bool                                                   mouse_down = false;
  std::vector<std::size_t>                               hovered_indices;
  // static auto constexpr minimap_range = 10;
  float minimap_range = 0.25f;

  //----------------------------------------------------------------------------
 public:
  //----------------------------------------------------------------------------
  vis(uniform_rectilinear_grid<double, 2> const& g, auto const& samplers)
      : grid{g},
        hovered(size(samplers), false),
        cam{Vec3<GLfloat>{0, 0, 0},
            Vec3<GLfloat>{0, 0, -1},
            -minimap_range,
            minimap_range,
            -minimap_range,
            minimap_range,
            -1,
            1,
            Vec4<std::size_t>{10, 10, 500, 500}},
        samplers{samplers},
        locals(size(samplers), vec2::zeros()) {
    locals_gpu.vertexbuffer().resize(size(samplers));
    locals_gpu.indexbuffer().resize(size(samplers));
    for (std::size_t i = 0; i < size(samplers); ++i) {
      locals_gpu.indexbuffer()[i] = i;
    }
  }
  //----------------------------------------------------------------------------
  auto properties() {
    ImGui::Text("Vis");
    if (ImGui::DragFloat("scale", &minimap_range, 0.01f, 0.0f, FLT_MAX,
                         "%.06f")) {
      cam = {Vec3<GLfloat>{0, 0, 0},
             Vec3<GLfloat>{0, 0, -1},
             -minimap_range,
             minimap_range,
             -minimap_range,
             minimap_range,
             -1,
             1,
             Vec4<std::size_t>{10, 10, 500, 500}};
    }
    ImGui::Text("Agran error: %e", agranovsky_error);
    ImGui::Text("Auton error: %e ", autonomous_particles_error);
    ImGui::Text("error diff:  %e ",
                agranovsky_error - autonomous_particles_error);
  }
  //----------------------------------------------------------------------------
  auto render(auto const& renderable, rendering::camera auto const& cam) {
    hoverable_shader::get().set_projection_matrix(cam.projection_matrix());
    hoverable_shader::get().set_view_matrix(cam.view_matrix());
    hoverable_shader::get().bind();
    gl::point_size(point_size);
    {
      auto p = gl::indexeddata<Vec2<GLfloat>, int>{};
      p.vertexbuffer().resize(1);
      p.indexbuffer().resize(1);
      {
        auto data = p.vertexbuffer().map();
        data[0]   = {Vec2<GLfloat>{current_point}, 0};
      }
      {
        auto data = p.indexbuffer().map();
        data[0]   = 0;
      }
      p.draw_points();
    }

    auto& ellipse_shader = rendering::interactive::renderer<
        geometry::ellipse<double>>::shader::get();
    auto& ellipse_geometry = rendering::interactive::renderer<
        geometry::ellipse<double>>::geometry::get();
    ellipse_shader.set_projection_matrix(cam.projection_matrix());
    ellipse_shader.bind();
    ellipse_shader.set_color(1, 0, 0);
    gl::line_width(3);
    for (std::size_t i = 0; i < size(hovered); ++i) {
      if (hovered[i]) {
        ellipse_shader.set_model_view_matrix(
            cam.view_matrix() *
            rendering::interactive::renderer<geometry::ellipse<double>>::
                construct_model_matrix(samplers[i].ellipse(backward).S(),
                                       samplers[i].center(backward)));
        ellipse_geometry.draw_line_loop();
      }
    }
  }
  //----------------------------------------------------------------------------
  auto late_render() {
    cam.set_gl_viewport();
    pass_through::get().bind();
    {
      auto outline = gl::indexeddata<Vec2<GLfloat>>{};
      outline.vertexbuffer().resize(4);
      outline.indexbuffer().resize(4);
      {
        auto data = outline.vertexbuffer().map();
        data[0]   = Vec2<GLfloat>{-0.999, -0.999};
        data[1]   = Vec2<GLfloat>{0.999, -0.999};
        data[2]   = Vec2<GLfloat>{0.999, 0.999};
        data[3]   = Vec2<GLfloat>{-0.999, 0.999};
      }
      {
        auto data = outline.indexbuffer().map();
        data[0]   = 0;
        data[1]   = 1;
        data[2]   = 2;
        data[3]   = 3;
      }
      gl::line_width(3);
      outline.draw_line_loop();
    }
    {
      auto axes = gl::indexeddata<Vec2<GLfloat>>{};
      axes.vertexbuffer().resize(4);
      axes.indexbuffer().resize(4);
      {
        auto data = axes.vertexbuffer().map();
        data[0]   = Vec2<GLfloat>{0, -1};
        data[1]   = Vec2<GLfloat>{0, 1};
        data[2]   = Vec2<GLfloat>{-1, 0};
        data[3]   = Vec2<GLfloat>{1, 0};
      }
      {
        auto data = axes.indexbuffer().map();
        data[0]   = 0;
        data[1]   = 1;
        data[2]   = 2;
        data[3]   = 3;
      }
      gl::line_width(1);
      axes.draw_lines();
    }

    only_hovered_shader::get().set_projection_matrix(cam.projection_matrix());
    only_hovered_shader::get().set_view_matrix(cam.view_matrix());
    only_hovered_shader::get().bind();
    gl::point_size(10);
    locals_gpu.draw_points();
    hoverable_shader::get().set_projection_matrix(cam.projection_matrix());
    hoverable_shader::get().set_view_matrix(cam.view_matrix());
    hoverable_shader::get().bind();
    gl::point_size(5);
    locals_gpu.draw_points();
  }
  //----------------------------------------------------------------------------
  auto update_points(rendering::camera auto const& cam) {
    current_point =
        vec2{cam.unproject(vec2f{cursor_pos.x(), cursor_pos.y()}).xy()};

    auto map = locals_gpu.vertexbuffer().wmap();
    for_loop(
        [&](auto const i) {
          auto const& s = samplers[i];
          locals[i]     = s.local_pos(current_point, backward);
          // locals[i] = *inv(s.ellipse(backward).S()) * (current_point -
          // s.ellipse(backward).center());
          map[i] = {vec2f{locals[i]}, hovered[i] ? 1 : 0};
        },
        execution_policy::parallel, samplers.size());
    agranovsky_error =
        grid.scalar_vertex_property("flowmap_error_agranovksy_backward")
            .linear_sampler()(current_point);
    autonomous_particles_error =
        grid.scalar_vertex_property(
                "flowmap_error_autonomous_particles_backward")
            .linear_sampler()(current_point);
  }
  //----------------------------------------------------------------------------
  auto on_cursor_moved(double const cursor_x, double const cursor_y,
                       rendering::camera auto const& cam) {
    cursor_pos = {cursor_x, cursor_y};
    if (mouse_down) {
      update_points(cam);
    }

    for (std::size_t i = 0; i < size(locals); ++i) {
      auto const proj  = this->cam.project(vec2f{locals[i]}).xy();
      auto const proj2 = vec2{cam.unproject(vec2f{cursor_pos}).xy()};
      hovered[i]       = euclidean_distance(proj, cursor_pos) < 10 ||
                   samplers[i].is_inside(proj2, backward);
      locals_gpu.vertexbuffer()[i] = {vec2f{locals[i]}, hovered[i] ? 1 : 0};
    }
  }
  //----------------------------------------------------------------------------
  auto on_button_pressed(gl::button b, rendering::camera auto const& cam) {
    if (b == gl::button::left) {
      mouse_down = true;
      update_points(cam);
    }
  }
  auto on_button_released(gl::button b) {
    if (b == gl::button::left) {
      mouse_down = false;
    }
  }
};
//------------------------------------------------------------------------------
auto doit(auto& g, auto const& v, auto const& initial_particles,
          auto& uuid_generator, auto const t0, auto const t_end) {
  auto const tau = t_end - t0;
  auto&      flowmap_numerical_backward_prop =
      g.vec2_vertex_property("flowmap_numerical_backward");
  auto& flowmap_autonomous_particles_backward_prop =
      g.vec2_vertex_property("flowmap_autonomous_particles_backward");
  auto& flowmap_agranovsky_backward_prop =
      g.vec2_vertex_property("flowmap_agranovksy_backward");
  auto& flowmap_error_autonomous_particles_backward_prop =
      g.scalar_vertex_property("flowmap_error_autonomous_particles_backward");
  auto& flowmap_error_agranovksy_backward_prop =
      g.scalar_vertex_property("flowmap_error_agranovksy_backward");
  auto& flowmap_error_diff_backward_prop =
      g.scalar_vertex_property("flowmap_error_diff_backward");
  discretize(v, g, "velocity", execution_policy::parallel);
  auto phi = flowmap(v);

  auto flowmap_autonomous_particles = autonomous_particle_flowmap_type{
      phi, t_end, 0.01, initial_particles, uuid_generator};
  auto const num_particles_after_advection =
      flowmap_autonomous_particles.num_particles();

  auto const agranovsky_delta_t = 0.1;
  auto const num_agranovksy_steps =
      static_cast<std::size_t>(std::ceil(agranovsky_delta_t / t_end));
  auto const regularized_height_agranovksky =
      static_cast<std::size_t>(std::ceil(
          std::sqrt((num_particles_after_advection) / num_agranovksy_steps)));
  auto const regularized_width_agranovksky = regularized_height_agranovksky;
  std::cout << "num_particles_after_advection: "
            << num_particles_after_advection << '\n';
  std::cout << "num_agranovksy_steps: " << num_agranovksy_steps << '\n';
  std::cout << "regularized_width_agranovksky: "
            << regularized_width_agranovksky << '\n';
  std::cout << "regularized_height_agranovksky: "
            << regularized_height_agranovksky << '\n';

  auto flowmap_agranovsky =
      agranovsky_flowmap_discretization2{phi,
                                         t0,
                                         tau,
                                         agranovsky_delta_t,
                                         g.front(),
                                         g.back(),
                                         regularized_width_agranovksky,
                                         regularized_height_agranovksky};
  std::cout << "measuring numerical flowmap...\n";
  g.vertices().iterate_indices(
      [&](auto const... is) {
        auto       copy_phi                    = phi;
        auto const x                           = g.vertex_at(is...);
        flowmap_numerical_backward_prop(is...) = copy_phi(x, t_end, -tau);
      },
      execution_policy::parallel);
  std::cout << "measuring numerical flowmap done\n";
  std::cout << "measuring autonomous particles flowmap...\n";
  g.vertices().iterate_indices(
      [&](auto const... is) {
        auto       copy_phi = phi;
        auto const x        = g.vertex_at(is...);
        flowmap_autonomous_particles_backward_prop(is...) =
            flowmap_autonomous_particles.sample_backward(x);
      },
      execution_policy::parallel);
  std::cout << "measuring autonomous particles flowmap done\n";
  std::cout << "measuring agranovsky flowmap...\n";
  g.vertices().iterate_indices(
      [&](auto const... is) {
        auto       copy_phi = phi;
        auto const x        = g.vertex_at(is...);
        try {
          flowmap_agranovsky_backward_prop(is...) =
              flowmap_agranovsky.sample_backward(x);
        } catch (...) {
          flowmap_agranovsky_backward_prop(is...) = vec2{0.0 / 0.0, 0.0 / 0.0};
        }
      },
      execution_policy::parallel);
  std::cout << "measuring agranovsky flowmap done\n";
  std::cout << "measuring errors...\n";
  g.vertices().iterate_indices(
      [&](auto const... is) {
        auto       copy_phi = phi;
        auto const x        = g.vertex_at(is...);
        flowmap_error_autonomous_particles_backward_prop(is...) =
            euclidean_distance(
                flowmap_numerical_backward_prop(is...),
                flowmap_autonomous_particles_backward_prop(is...));
        flowmap_error_agranovksy_backward_prop(is...) =
            euclidean_distance(flowmap_numerical_backward_prop(is...),
                               flowmap_agranovsky_backward_prop(is...));
        flowmap_error_diff_backward_prop(is...) =
            flowmap_error_agranovksy_backward_prop(is...) -
            flowmap_error_autonomous_particles_backward_prop(is...);
      },
      execution_policy::parallel);
  std::cout << "measuring errors done\n";

  rendering::interactive::pre_setup();
  auto m                  = vis{g, flowmap_autonomous_particles.samplers()};
  auto advected_particles = std::vector<geometry::ellipse<real_number>>{};
  std::ranges::copy(flowmap_autonomous_particles.samplers() |
                        std::views::transform(
                            [](auto const& s) { return s.ellipse(backward); }),
                    std::back_inserter(advected_particles));
  rendering::interactive::render(/*initial_particles, */ advected_particles, m,
                                 g);
};
//==============================================================================
auto main() -> int {
  auto                        uuid_generator = std::atomic_uint64_t{};
  [[maybe_unused]] auto const r              = 0.01;
  [[maybe_unused]] auto const t0             = 0;
  [[maybe_unused]] auto const t_end          = 1;

  auto  rand    = random::uniform{0.0, 1.0};
  auto  ps      = pointset2{};
  auto& prop_ps = ps.scalar_vertex_property("prop");
  for (std::size_t i = 0; i < 1000; ++i) {
    auto v     = ps.insert_vertex(rand() * 2, rand());
    prop_ps[v] = rand();
  }

  auto dg = analytical::fields::numerical::doublegyre{};
  auto g  = rectilinear_grid{linspace{0.0, 2.0, 401}, linspace{0.0, 1.0, 201}};
  auto const sampler_radius = 0.01;
  auto       prop_ps_sampler_mls =
      ps.moving_least_squares_sampler(prop_ps, sampler_radius);
  discretize(prop_ps_sampler_mls, g, "moving_least_squares",
             execution_policy::sequential);
  auto prop_ps_sampler_inv_dist =
      ps.inverse_distance_weighting_sampler(prop_ps, sampler_radius);
  discretize(prop_ps_sampler_inv_dist, g, "inverse_distance_weighting",
             execution_policy::parallel);
  rendering::interactive::render(ps, g);

  //auto const eps                  = 1e-3;
  //auto const initial_particles_dg = autonomous_particle2::particles_from_grid(
  //    t0,
  //    rectilinear_grid{linspace{0.0 + eps, 2.0 - eps, 21},
  //                     linspace{0.0 + eps, 1.0 - eps, 11}},
  //    uuid_generator);
  //// auto const initial_particles_dg = std::vector<autonomous_particle2>{
  ////     {vec2{1 - r, 0.5 - r}, t0, r, uuid_generator},
  ////     {vec2{1 + r, 0.5 - r}, t0, r, uuid_generator},
  ////     {vec2{1 - r, 0.5 + r}, t0, r, uuid_generator},
  ////     {vec2{1 + r, 0.5 + r}, t0, r, uuid_generator}};
  // doit(g, dg, initial_particles_dg, uuid_generator, t0, t_end);

  //============================================================================
   //auto s  = analytical::fields::numerical::saddle{};
   //auto rs = analytical::fields::numerical::rotated_saddle{};
   //auto constexpr cos = gcem::cos(M_PI / 4);
   //auto constexpr sin = gcem::sin(M_PI / 4);
   //auto const initial_particles_saddle =
   //   std::vector<autonomous_particle2>{{vec2{r, r}, t0, r, uuid_generator},
   //                                     {vec2{r, -r}, t0, r, uuid_generator},
   //                                     {vec2{-r, r}, t0, r, uuid_generator},
   //                                     {vec2{-r, -r}, t0, r,
   //                                     uuid_generator}};
   //auto const initial_rotated_particles_saddle =
   //   std::vector<autonomous_particle2>{
   //       {vec2{cos * r - sin * r, sin * r + cos * r}, t0, r, uuid_generator},
   //       {vec2{cos * -r - sin * r, sin * -r + cos * r}, t0, r,
   //       uuid_generator}, {vec2{cos * r - sin * -r, sin * r + cos * -r}, t0,
   //       r, uuid_generator}, {vec2{cos * -r - sin * -r, sin * -r + cos * -r},
   //       t0, r,
   //        uuid_generator}};
   //
   //doit(g, rs, initial_particles_saddle, uuid_generator, t0, t_end);
}
