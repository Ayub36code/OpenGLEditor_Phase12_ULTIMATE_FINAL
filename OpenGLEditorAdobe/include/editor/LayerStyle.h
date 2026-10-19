
#pragma once
#include "editor/ProFeatures.h"
#include "graphics/Shader.h"
#include "graphics/Framebuffer.h"

namespace Editor {

class LayerStyleRenderer {
public:
    LayerStyleRenderer(uint32_t w=1280, uint32_t h=720);
    void Resize(uint32_t w, uint32_t h);
    void RenderStyle(uint32_t layerTexture, std::shared_ptr<Framebuffer> target, const LayerStyle& style, const glm::vec2& layerPos, const glm::vec2& layerSize);

private:
    void RenderDropShadow(uint32_t layerTex, std::shared_ptr<Framebuffer> target, const DropShadowStyle& style);
    void RenderOuterGlow(uint32_t layerTex, std::shared_ptr<Framebuffer> target, const OuterGlowStyle& style);
    void RenderStroke(uint32_t layerTex, std::shared_ptr<Framebuffer> target, const StrokeStyle& style);

    std::shared_ptr<Shader> m_ShadowShader;
    std::shared_ptr<Shader> m_GlowShader;
    std::shared_ptr<Shader> m_StrokeShader;
    std::shared_ptr<Framebuffer> m_TempFBO[2];
    uint32_t m_Width, m_Height;
};

} // namespace
