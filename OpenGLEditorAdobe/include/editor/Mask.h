
#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "graphics/Framebuffer.h"
#include "graphics/Texture.h"

namespace Editor {

class LayerMask {
public:
    LayerMask(uint32_t w=512, uint32_t h=512);
    void Resize(uint32_t w, uint32_t h);
    void Clear(float value=1.f); // 1 = fully visible, 0 = hidden

    // Paint to mask
    void Paint(const glm::vec2& from, const glm::vec2& to, float brushSize, float opacity, bool erase=false);

    uint32_t GetTextureID() const { return m_FBO ? m_FBO->GetColorAttachmentRendererID(0) : 0; }
    std::shared_ptr<Framebuffer> GetFBO() { return m_FBO; }

    bool IsEnabled() const { return m_Enabled; }
    void SetEnabled(bool e) { m_Enabled = e; }

private:
    std::shared_ptr<Framebuffer> m_FBO;
    std::shared_ptr<class Shader> m_BrushShader;
    std::shared_ptr<Texture2D> m_BrushTip;
    bool m_Enabled = true;
};

} // namespace
