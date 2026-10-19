
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Editor {

class OrthographicCamera {
public:
    OrthographicCamera(float left, float right, float bottom, float top);
    void SetProjection(float left, float right, float bottom, float top);
    void SetPosition(const glm::vec3& pos) { m_Position=pos; Recalculate(); }
    void SetRotation(float rot) { m_Rotation=rot; Recalculate(); }
    void SetZoom(float zoom) { m_Zoom=zoom; RecalculateProjection(); }

    const glm::vec3& GetPosition() const { return m_Position; }
    float GetRotation() const { return m_Rotation; }
    float GetZoom() const { return m_Zoom; }
    const glm::mat4& GetProjectionMatrix() const { return m_Projection; }
    const glm::mat4& GetViewMatrix() const { return m_View; }
    const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjection; }

    // Screen to world for pan/zoom
    glm::vec2 ScreenToWorld(glm::vec2 screenPos, glm::vec2 viewportSize) const;

private:
    void Recalculate();
    void RecalculateProjection();

    glm::mat4 m_Projection{1.0f}, m_View{1.0f}, m_ViewProjection{1.0f};
    glm::vec3 m_Position{0,0,0};
    float m_Rotation=0.f;
    float m_Zoom=1.f;
    float m_Left, m_Right, m_Bottom, m_Top;
};

} // namespace
