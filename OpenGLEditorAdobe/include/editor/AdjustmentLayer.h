
#pragma once
#include "editor/ProFeatures.h"
#include "graphics/Framebuffer.h"
#include "graphics/Shader.h"
#include <memory>

namespace Editor {

class AdjustmentStack {
public:
    AdjustmentStack(uint32_t w=1280, uint32_t h=720);
    void Resize(uint32_t w, uint32_t h);
    void AddAdjustment(std::shared_ptr<AdjustmentLayer> adj);
    void RemoveAdjustment(int index);
    void MoveAdjustment(int from, int to);
    std::vector<std::shared_ptr<AdjustmentLayer>>& GetAdjustments() { return m_Adjustments; }

    // Apply all enabled adjustments
    void Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO);

private:
    std::vector<std::shared_ptr<AdjustmentLayer>> m_Adjustments;
    std::shared_ptr<Framebuffer> m_FBO[2];
    std::shared_ptr<Shader> m_CopyShader;
    uint32_t m_Width, m_Height;
};

} // namespace
