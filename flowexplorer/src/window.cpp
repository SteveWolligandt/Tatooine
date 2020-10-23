#include <tatooine/flowexplorer/window.h>
//==============================================================================
namespace tatooine::flowexplorer {
//==============================================================================
window::window() : m_scene(camera_controller(), this) { start(); }
//============================================================================
void window::on_key_pressed(yavin::key k) {
  first_person_window::on_key_pressed(k);
  if (k == yavin::KEY_F1) {
    m_show_nodes_gui = !m_show_nodes_gui;
  } else if (k == yavin::KEY_F2) {
    camera_controller().use_perspective_camera();
    camera_controller().use_fps_controller();
  } else if (k == yavin::KEY_F3) {
    camera_controller().use_orthographic_camera();
    camera_controller().use_orthographic_controller();
  } else if (k == yavin::KEY_F4) {
    camera_controller().look_at({0, 0, 0}, {0, 0, -1});
  } else if (k == yavin::KEY_F5) {
    m_scene.write("scene.toml");
  } else if (k == yavin::KEY_F6) {
    m_file_browser = std::make_unique<ImGui::FileBrowser>(0);
    m_file_browser->SetTitle("Load File");
    m_file_browser->SetTypeFilters({".toml", ".scene", ".vtk"});
    m_file_browser->Open();
  } else if (k == yavin::KEY_ESCAPE) {
    if (m_file_browser) {
      if (m_file_browser->IsOpened()) {
        m_file_browser->Close();
      }
    }
  }
}
//------------------------------------------------------------------------------
void window::start() {
  render_loop([&](const auto& dt) {
    m_scene.render(dt);
    if (m_show_nodes_gui) {
      m_scene.draw_node_editor(0, 0, this->width() / 3, this->height(),
                               m_show_nodes_gui);
    }
    if (m_file_browser) {
      m_file_browser->Display();
      if (m_file_browser->HasSelected()) {
        m_scene.open_file(m_file_browser->GetSelected());
        m_file_browser->ClearSelected();
        m_file_browser->Close();
      }
    }
  });
}
//==============================================================================
}  // namespace tatooine::flowexplorer
//==============================================================================
