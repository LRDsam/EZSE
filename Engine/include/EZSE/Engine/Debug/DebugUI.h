#pragma once

#include <functional>
#include <string>
#include <vector>

namespace ezse {

class World;

class DebugUI {
public:
    using PanelFunction = std::function<void(World&)>;

    // Adds a section to the debug window; draw only draws its content. The name must be unique.
    void AddPanel(std::string name, PanelFunction draw);

    // Whether the debug UI is shown. Off by default.
    void SetVisible(bool visible);
    bool IsVisible() const;

    // Draws every panel; called by Engine::Run between ImGui::NewFrame and ImGui::Render.
    void Draw(World& world);

private:
    struct Panel {
        std::string name;
        PanelFunction draw;
    };

    std::vector<Panel> m_panels;
    bool m_visible = false;
};

} // namespace ezse
