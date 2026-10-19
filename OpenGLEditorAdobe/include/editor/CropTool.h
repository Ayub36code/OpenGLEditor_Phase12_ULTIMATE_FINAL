
#pragma once
#include <glm/glm.hpp>

namespace Editor {

struct CropRect {
    glm::vec2 Min{0,0}; // normalized 0-1
    glm::vec2 Max{1,1};
    bool Active=false;

    glm::vec2 GetSize() const { return Max - Min; }
    void Reset() { Min={0,0}; Max={1,1}; Active=false; }
    bool IsValid() const { return Active && GetSize().x>0.01f && GetSize().y>0.01f; }
};

class CropTool {
public:
    CropTool() = default;

    void Begin(const glm::vec2& imageSize);
    void Update(const glm::vec2& mousePos, const glm::vec2& imagePos, const glm::vec2& imageSize, float zoom);
    void End();

    CropRect& GetRect() { return m_Rect; }
    bool IsActive() const { return m_Active; }
    void SetActive(bool a) { m_Active=a; if(!a) m_Rect.Reset(); }

    // Apply crop to image pixels (CPU)
    bool ApplyCrop(class Image& image);

private:
    CropRect m_Rect;
    bool m_Active=false;
    bool m_Dragging=false;
    glm::vec2 m_StartPos{0};
};

} // namespace
