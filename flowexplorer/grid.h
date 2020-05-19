#ifndef TATOOINE_FLOWEXPLORER_GRID_H
#define TATOOINE_FLOWEXPLORER_GRID_H
//==============================================================================
#include <tatooine/grid.h>
#include <yavin/imgui.h>
#include <yavin/indexeddata.h>

#include "line_shader.h"
#include "renderable.h"
//==============================================================================
namespace tatooine::flowexplorer {
//==============================================================================
template <typename Real, size_t N>
struct grid : tatooine::grid<Real, N>, renderable {
  using parent_t = tatooine::grid<Real, N>;
  using gpu_vec  = yavin::vec<float, N>;
  using vbo_t    = yavin::vertexbuffer<gpu_vec>;
  using parent_t::dimension;
  //============================================================================
  yavin::indexeddata<yavin::vec<float, N>> m_gpu_data;
  line_shader                              m_shader;
  int                                      m_linewidth = 1;
  std::array<GLfloat, 4>                   m_color{0.0f, 0.0f, 0.0f, 1.0f};
  //============================================================================
  grid()                       = default;
  grid(const grid&)     = default;
  grid(grid&&) noexcept = default;
  grid& operator=(const grid&) = default;
  grid& operator=(grid&&) noexcept = default;
  //============================================================================
  template <typename... Reals>
  constexpr grid(const linspace<Reals>&... ls) noexcept
      : parent_t{ls...} {
    create_indexed_data();
  }
  //============================================================================
  void render(const yavin::mat4& projection_matrix,
              const yavin::mat4& view_matrix) override {
    set_vbo_data();
    m_shader.bind();
    m_shader.set_color(m_color[0], m_color[1], m_color[2], m_color[3]);
    m_shader.set_projection_matrix(projection_matrix);
    m_shader.set_modelview_matrix(view_matrix);
    yavin::gl::line_width(m_linewidth);
    if (m_color[3] < 1) {
      yavin::enable_blending();
      yavin::blend_func_alpha();
      yavin::disable_depth_test();
    } else {
      yavin::disable_blending();
      yavin::enable_depth_test();
    }
    m_gpu_data.draw_lines();
  }
  //----------------------------------------------------------------------------
  void draw_ui() override {
    draw_ui_preferences();
    draw_ui_render_preferences();
  }
  //----------------------------------------------------------------------------
  void draw_ui_preferences() {
    for (size_t i = 0; i < N; ++i) {
      ImGui::Text("dimension-%i", i);
      //ImGui::DragDouble3("min", this->min().data_ptr(), 0.1);
      //ImGui::DragDouble3("max", this->max().data_ptr(), 0.1);
    }
  }
  //----------------------------------------------------------------------------
  void draw_ui_render_preferences() {
    ImGui::DragInt("line size", &m_linewidth, 1, 1, 10);
    ImGui::ColorEdit4("line color", m_color.data());
  }
  //----------------------------------------------------------------------------
  std::string name() const override { return "Grid"; }
  //============================================================================
  void set_vbo_data() {
    auto vbomap = m_gpu_data.vertex_buffer().map();
    yavin::get<0>(vbomap[0]) =
        gpu_vec{float(this->front(0)), float(this->front(1)), float(this->front(2))};
    yavin::get<0>(vbomap[1]) =
        gpu_vec{float(this->back(0)), float(this->front(1)), float(this->front(2))};
    yavin::get<0>(vbomap[2]) =
        gpu_vec{float(this->front(0)), float(this->back(1)), float(this->front(2))};
    yavin::get<0>(vbomap[3]) =
        gpu_vec{float(this->back(0)), float(this->back(1)), float(this->front(2))};
    yavin::get<0>(vbomap[4]) =
        gpu_vec{float(this->front(0)), float(this->front(1)), float(this->back(2))};
    yavin::get<0>(vbomap[5]) =
        gpu_vec{float(this->back(0)), float(this->front(1)), float(this->back(2))};
    yavin::get<0>(vbomap[6]) =
        gpu_vec{float(this->front(0)), float(this->back(1)), float(this->back(2))};
    yavin::get<0>(vbomap[7]) =
        gpu_vec{float(this->back(0)), float(this->back(1)), float(this->back(2))};
  }
  //----------------------------------------------------------------------------
  void create_indexed_data() {
    m_gpu_data.vertex_buffer().resize(8);
    m_gpu_data.index_buffer().resize(24);
    set_vbo_data();
    m_gpu_data.index_buffer() = {0, 1, 0, 2, 1, 3, 2, 3, 4, 5, 4, 6,
                                 5, 7, 6, 7, 0, 4, 1, 5, 2, 6, 3, 7};
    m_gpu_data.setup_vao();
  }
};
//==============================================================================
// deduction guides
//==============================================================================
template <typename... Reals>
grid(const linspace<Reals>&...) -> grid<promote_t<Reals...>, sizeof...(Reals)>;
//==============================================================================
}  // namespace tatooine::flowexplorer
//==============================================================================
#endif