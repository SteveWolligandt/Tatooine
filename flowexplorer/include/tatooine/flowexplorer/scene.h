#ifndef TATOOINE_FLOWEXPLORER_SCENE_H
#define TATOOINE_FLOWEXPLORER_SCENE_H
//==============================================================================
#include <tatooine/flowexplorer/renderable.h>
#include <tatooine/flowexplorer/ui/link.h>
#include <tatooine/flowexplorer/ui/node.h>
#include <tatooine/flowexplorer/ui/pin.h>
#include <tatooine/rendering/camera_controller.h>

#include <memory>
#include <vector>
//==============================================================================
namespace tatooine::flowexplorer {
//==============================================================================
struct window;
struct scene {
  std::vector<std::unique_ptr<ui::base::node>>   m_nodes;
  std::vector<std::unique_ptr<base::renderable>> m_renderables;
  std::vector<ui::link>                          m_links;
  ax::NodeEditor::EditorContext*       m_node_editor_context = nullptr;
  rendering::camera_controller<float>* m_cam;
  flowexplorer::window*                m_window;
  //============================================================================
  scene(rendering::camera_controller<float>& ctrl, flowexplorer::window* w);
  ~scene();
  //============================================================================
  void render(std::chrono::duration<double> const& dt);
  auto find_node(size_t const id) -> ui::base::node*;
  //----------------------------------------------------------------------------
  auto find_pin(size_t const id) -> ui::pin*;
  //----------------------------------------------------------------------------
  void node_creators();
  //----------------------------------------------------------------------------
  void draw_nodes();
  //----------------------------------------------------------------------------
  void draw_links();
  //----------------------------------------------------------------------------
  void create_link();
  //----------------------------------------------------------------------------
  void remove_link();
  //----------------------------------------------------------------------------
  void draw_node_editor(size_t const pos_x, size_t const pos_y,
                        size_t const width, size_t const height, bool& show);
  //----------------------------------------------------------------------------
  void write(std::filesystem::path const& filepath) const;
  void read(std::filesystem::path const& filepath);
  //----------------------------------------------------------------------------
  auto open_file(std::filesystem::path const& filepath) -> void;
  //----------------------------------------------------------------------------
  template <typename F>
  auto do_in_context(F&& f) const {
    ax::NodeEditor::SetCurrentEditor(m_node_editor_context);
    f();
    ax::NodeEditor::SetCurrentEditor(nullptr);
  }
  void clear() {
    m_nodes.clear();
    m_renderables.clear();
    m_links.clear();
  }
  auto window() const -> auto const& {
    return *m_window;
  }
  auto window() -> auto& {
    return *m_window;
  }
};
//==============================================================================
}  // namespace tatooine::flowexplorer
//==============================================================================
#endif
