
#pragma once
#include "graphics/Framebuffer.h"
#include "graphics/Camera.h"
#include <memory>

namespace Editor {

// RenderTarget = wrapper around Framebuffer + Camera for viewport rendering
// This is the non-destructive editing pipeline piece: Image -> Texture -> FBO -> Effects -> Composition -> Viewport

class RenderTarget {
public:
    RenderTarget(uint32_t width, uint32_t height);
    void Resize(uint32_t w, uint32_t h);
    void Bind() { m_Framebuffer->Bind(); }
    void Unbind() { m_Framebuffer->Unbind(); }
    std::shared_ptr<Framebuffer> GetFramebuffer() { return m_Framebuffer; }
    uint32_t GetTextureID() const { return m_Framebuffer->GetColorAttachmentRendererID(0); }
    OrthographicCamera& GetCamera() { return *m_Camera; }

private:
    std::shared_ptr<Framebuffer> m_Framebuffer;
    std::unique_ptr<OrthographicCamera> m_Camera;
};

}
