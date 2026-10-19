
#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "graphics/Framebuffer.h"
#include "graphics/Texture.h"

namespace Editor {

enum class SelectionType { None, Rectangle, Ellipse, Lasso, MagicWand, Polygonal };

struct Selection {
    SelectionType Type = SelectionType::None;
    std::vector<glm::vec2> Points; // for lasso/polygon
    glm::vec2 Min{0}, Max{0}; // for rect/ellipse
    bool Active = false;
    float Feather = 0.f; // feather radius
    bool Invert = false;

    void Clear() { Points.clear(); Min={0,0}; Max={0,0}; Active=false; Type=SelectionType::None; }
    bool IsEmpty() const { return !Active || (Type==SelectionType::None); }
    glm::vec2 GetSize() const { return Max - Min; }
    glm::vec2 GetCenter() const { return (Min+Max)*0.5f; }
};

class SelectionManager {
public:
    SelectionManager(uint32_t w=1280, uint32_t h=720);
    void Resize(uint32_t w, uint32_t h);

    void BeginSelection(const glm::vec2& pos, SelectionType type);
    void UpdateSelection(const glm::vec2& pos);
    void EndSelection(const glm::vec2& pos);

    void AddLassoPoint(const glm::vec2& pos);
    void Clear() { m_Selection.Clear(); UpdateMask(); }

    const Selection& GetSelection() const { return m_Selection; }
    bool HasSelection() const { return m_Selection.Active; }

    // Magic wand - flood fill based on color tolerance
    void MagicWandSelect(const glm::vec2& pos, uint32_t imageTexture, float tolerance=0.1f);

    // Mask texture for rendering selection overlay
    uint32_t GetMaskTextureID() const { return m_MaskFBO ? m_MaskFBO->GetColorAttachmentRendererID(0) : 0; }
    std::shared_ptr<Framebuffer> GetMaskFBO() { return m_MaskFBO; }

    // Feather and invert
    void SetFeather(float f) { m_Selection.Feather = f; UpdateMask(); }
    void SetInvert(bool inv) { m_Selection.Invert = inv; UpdateMask(); }

    // Check if point inside selection
    bool IsPointInside(const glm::vec2& pos) const;

private:
    void UpdateMask();
    void RenderRectMask();
    void RenderEllipseMask();
    void RenderLassoMask();

    Selection m_Selection;
    std::shared_ptr<Framebuffer> m_MaskFBO;
    std::shared_ptr<class Shader> m_MaskShader;
    uint32_t m_Width, m_Height;
    glm::vec2 m_StartPos{0};
};

} // namespace
