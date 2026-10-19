
#include "editor/ImageCanvas.h"
#include "editor/Viewport.h"
#include "graphics/Renderer.h"
#include "core/Log.h"
#include <glm/glm.hpp>

namespace Editor {

void ImageCanvas::Init() {
    LOG_INFO("ImageCanvas initialized - CanvasSize default 1920x1080");
}

void ImageCanvas::Render(Viewport& viewport) {
    auto fb = viewport.GetFramebuffer();
    auto& cam = viewport.GetCamera();

    fb->Bind();
    glClearColor(m_Settings.ClearColor.r, m_Settings.ClearColor.g, m_Settings.ClearColor.b, m_Settings.ClearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Renderer must be in scene with viewport camera
    // Access global renderer? For now we need to use Renderer singleton-ish pattern via local rendering
    // Since Renderer holds quad VA and shaders, we will render manually here using its shaders if available
    // For Phase3 we do direct OpenGL with same shaders but we rely on Application's Renderer BeginScene already called before canvas
    // So this method assumes Renderer::BeginScene(camera) was called outside

    // The actual quad draws will be done in Application.cpp using Renderer API
    // This class just manages settings

    fb->Unbind();
}

}
