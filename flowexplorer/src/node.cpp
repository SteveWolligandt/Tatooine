#include <tatooine/flowexplorer/scene.h>
#include <tatooine/field.h>
#include <tatooine/flowexplorer/ui/node.h>
#include <tatooine/flowexplorer/ui/node_builder.h>
#include <tatooine/flowexplorer/ui/pin.h>
#include <tatooine/flowexplorer/ui/draw_icon.h>
//==============================================================================
namespace tatooine::flowexplorer::ui::base {
//==============================================================================
node::node(flowexplorer::scene& s) : m_title{""}, m_scene{&s} {}
//------------------------------------------------------------------------------
node::node(std::string const& title, flowexplorer::scene& s)
    : m_title{title}, m_scene{&s} {}
//------------------------------------------------------------------------------
auto node::notify_property_changed(bool const notify_self) -> void {
  if (notify_self) {
    on_property_changed();
  }
  if (has_self_pin()) {
    for (auto l : m_self_pin->links()) {
      l->input().node().on_property_changed();
    }
  }
  for (auto& p : m_output_pins) {
    for (auto l : p->links()) {
      l->input().node().on_property_changed();
    }
  }
}
//------------------------------------------------------------------------------
auto node::node_position() const -> ImVec2 {
  ImVec2 pos;
  m_scene->do_in_context(
      [&] { pos = ax::NodeEditor::GetNodePosition(get_id()); });
  return pos;
}
//------------------------------------------------------------------------------
auto node::draw_node() -> void {
  size_t const icon_size = 20 * scene().window().ui_scale_factor();
  namespace ed = ax::NodeEditor;
  node_builder builder;
  builder.begin(get_id());

  ImGui::Dummy(ImVec2(10, 0));
  builder.header();
  ImGui::Checkbox("", &is_active());
  //ImGui::Spring(0);

  auto alpha = ImGui::GetStyle().Alpha;
  if (m_self_pin != nullptr && scene().new_link() &&
      !scene().can_create_new_link(*m_self_pin)) {
    alpha = alpha * 48.0f / 255.0f;
  }

  // draw editable title
  ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
  scene().window().push_bold_font();
  ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0,0,0,0));
  ImGui::PushItemWidth(200);
  ImGui::InputText("##title", &title());
  ImGui::PopItemWidth();
  ImGui::PopStyleColor();
  scene().window().pop_font();
  ImGui::PopStyleVar();

  //ImGui::Spring(1);

  if (m_self_pin != nullptr) {
    //ImGui::BeginVertical("delegates", ImVec2(0, 0));
    //ImGui::Spring(1, 0);
    auto alpha = ImGui::GetStyle().Alpha;
    if (scene().new_link() && !scene().can_create_new_link(*m_self_pin)) {
      alpha = alpha * 48.0f / 255.0f;
    }

    ed::BeginPin(m_self_pin->get_id(), ed::PinKind::Output);
    ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
    ed::PinPivotSize(ImVec2(0, 0));
    //ImGui::BeginHorizontal(m_self_pin->get_id().AsPointer());
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    if (!m_self_pin->title().empty()) {
      ImGui::TextUnformatted(m_self_pin->title().c_str());
      //ImGui::Spring(0);
    }
    m_self_pin->draw(icon_size, alpha);
    //ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
    //ImGui::EndHorizontal();
    ImGui::PopStyleVar();
    ed::EndPin();

    //ImGui::Spring(1, 0);
    //ImGui::EndVertical();
    //ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
  } else {
    //ImGui::Spring(0);
  }
  builder.end_header();

  for (auto& input : input_pins()) {
    auto alpha = ImGui::GetStyle().Alpha;
   if (scene().new_link() && !scene().can_create_new_link(*input)) {
      alpha = alpha * 48.0f / 255.0f;
    }

    builder.input(input->get_id());
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    input->draw(icon_size, alpha);
    //ImGui::Spring(0);
    if (!input->title().empty()) {
      ImGui::TextUnformatted(input->title().c_str());
      //ImGui::Spring(0);
    }
    ImGui::PopStyleVar();
    builder.end_input();
  }
  // ImGui::Spring(0);
  {
    auto alpha = ImGui::GetStyle().Alpha;
    if (scene().new_link()) {
      alpha = alpha * 48.0f / 255.0f;
    }
    builder.middle();
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    ImGui::PushItemWidth(200 * scene().window().ui_scale_factor());
    //ed::Suspend();
    auto const changed = draw_properties();
    //ed::Resume();
    if (changed) {
      notify_property_changed();
    }
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
  }

  for (auto& output : output_pins()) {
    if (output->is_active()) {
      auto alpha = ImGui::GetStyle().Alpha;
      if (scene().new_link() && !scene().can_create_new_link(*output)) {
        alpha = alpha * 48.0f / 255.0f;
      }

      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
      builder.output(output->get_id());
      if (!output->title().empty()) {
        //ImGui::Spring(0);
        ImGui::TextUnformatted(output->title().c_str());
      }
      //ImGui::Spring(0);
      output->draw(icon_size, alpha);
      ImGui::PopStyleVar();
      builder.end_output();
    }
  }
  builder.end();
}
//==============================================================================
}  // namespace tatooine::flowexplorer::ui::base
//==============================================================================
namespace tatooine::flowexplorer {
//==============================================================================
auto insert_registered_element(scene& s, std::string_view const& name)
    -> ui::base::node* {
  iterate_registered_factories(factory) {
    if (auto ptr = factory->f(s, name); ptr) {
      return ptr;
    }
  }
  return nullptr;
}
//==============================================================================
}  // namespace tatooine::flowexplorer
//==============================================================================
