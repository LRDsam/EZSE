#include "EZSE/Engine/Debug/DebugUI.h"

#include "EZSE/Core/Assert.h"

#include <imgui.h>

#include <utility>

namespace ezse {

void DebugUI::AddPanel(std::string name, PanelFunction draw) {
    EZSE_ASSERT(static_cast<bool>(draw), "PanelFunction cannot be empty");
    for (const auto& panel : m_panels) {
        EZSE_ASSERT(name != panel.name, "A panel with this name already exists");
    }
    m_panels.push_back(Panel{std::move(name), std::move(draw)});
}

void DebugUI::SetVisible(bool visible) {
    m_visible = visible;
}

bool DebugUI::IsVisible() const {
    return m_visible;
}

void DebugUI::Draw(World& world) {
    if (!m_visible) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    for (const auto& panel : m_panels) {
        if (ImGui::CollapsingHeader(panel.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            panel.draw(world);
        }
    }
    ImGui::End();
}

} // namespace ezse
