
#pragma once
#include <glm/glm.hpp>
#include "core/Transform.h"
#include <functional>

namespace Editor {

enum class GizmoHandle {
    None,
    Center, // move
    TopLeft, Top, TopRight,
    Right, BottomRight, Bottom, BottomLeft, Left,
    Rotation
};

enum class GizmoMode { Translate, Scale, Rotate, Universal };

class TransformGizmo {
public:
    TransformGizmo();

    // Call each frame to draw handles in ImGui viewport (using ImDrawList)
    // Returns true if transform changed
    bool DrawAndHandle(const glm::vec2& viewportPos, const glm::vec2& viewportSize,
                       const glm::vec2& objectScreenPos, const glm::vec2& objectScreenSize,
                       float rotation, Transform& transform, float zoom);

    GizmoHandle GetHoveredHandle() const { return m_Hovered; }
    GizmoHandle GetActiveHandle() const { return m_Active; }
    bool IsActive() const { return m_Active != GizmoHandle::None; }

    void SetMode(GizmoMode mode) { m_Mode = mode; }
    GizmoMode GetMode() const { return m_Mode; }

    // Conversion helpers
    static glm::vec2 WorldToScreen(const glm::vec2& worldPos, const glm::mat4& viewProj, const glm::vec2& viewportSize, const glm::vec2& viewportPos);
    static glm::vec2 ScreenToWorldDelta(const glm::vec2& screenDelta, float zoom);

private:
    GizmoHandle HitTest(const glm::vec2& mousePos, const glm::vec2& objPos, const glm::vec2& objSize, float rot);
    void DrawHandle(ImDrawList* dl, const glm::vec2& pos, bool hovered, bool active, ImU32 col);

    GizmoHandle m_Hovered = GizmoHandle::None;
    GizmoHandle m_Active = GizmoHandle::None;
    GizmoMode m_Mode = GizmoMode::Universal;

    glm::vec2 m_DragStartMouse{0};
    glm::vec2 m_DragStartPos{0};
    glm::vec2 m_DragStartScale{1};
    float m_DragStartRotation=0;

    const float m_HandleSize = 8.f;
    const float m_RotationHandleDist = 30.f;
};

} // namespace
