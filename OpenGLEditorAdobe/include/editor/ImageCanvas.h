
#pragma once
#include "editor/Viewport.h"
#include <glm/glm.hpp>

namespace Editor {

// Canvas that composes image rendering to viewport FBO
// Future: layers, effects pipeline will render here

class ImageCanvas {
public:
    ImageCanvas() = default;
    void Init();
    void Render(Viewport& viewport);

    // Canvas settings
    struct Settings {
        glm::vec4 ClearColor{0.18f,0.18f,0.19f,1.0f};
        bool ShowChecker=true;
        bool ShowGrid=false;
        glm::vec2 CanvasSize{1920,1080}; // default canvas
    };

    Settings& GetSettings() { return m_Settings; }

private:
    Settings m_Settings;
};

} // namespace
