
#include "editor/TransformGizmo.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace Editor {

TransformGizmo::TransformGizmo() {}

void TransformGizmo::DrawHandle(ImDrawList* dl, const glm::vec2& pos, bool hovered, bool active, ImU32 col) {
    float sz = m_HandleSize;
    ImU32 c = active ? IM_COL32(255,200,50,255) : hovered ? IM_COL32(100,200,255,255) : col;
    dl->AddRectFilled(ImVec2(pos.x - sz*0.5f, pos.y - sz*0.5f), ImVec2(pos.x + sz*0.5f, pos.y + sz*0.5f), c, 2.f);
    dl->AddRect(ImVec2(pos.x - sz*0.5f, pos.y - sz*0.5f), ImVec2(pos.x + sz*0.5f, pos.y + sz*0.5f), IM_COL32(0,0,0,200), 2.f, 0, 1.f);
}

GizmoHandle TransformGizmo::HitTest(const glm::vec2& mousePos, const glm::vec2& objPos, const glm::vec2& objSize, float rot) {
    // Compute OBB corners
    float hw = objSize.x*0.5f, hh = objSize.y*0.5f;
    float rad = glm::radians(rot);
    float cosR = cosf(rad), sinR = sinf(rad);

    auto rotatePoint = [&](glm::vec2 p) -> glm::vec2 {
        glm::vec2 rel = p - objPos;
        return glm::vec2(rel.x*cosR - rel.y*sinR, rel.x*sinR + rel.y*cosR) + objPos;
    };

    // Check handles
    glm::vec2 handles[8] = {
        {objPos.x - hw, objPos.y - hh}, // TL
        {objPos.x, objPos.y - hh},      // T
        {objPos.x + hw, objPos.y - hh}, // TR
        {objPos.x + hw, objPos.y},      // R
        {objPos.x + hw, objPos.y + hh}, // BR
        {objPos.x, objPos.y + hh},      // B
        {objPos.x - hw, objPos.y + hh}, // BL
        {objPos.x - hw, objPos.y}       // L
    };

    // rotate handles
    for(int i=0;i<8;i++) handles[i] = rotatePoint(handles[i]);

    GizmoHandle types[8] = {GizmoHandle::TopLeft, GizmoHandle::Top, GizmoHandle::TopRight, GizmoHandle::Right,
                            GizmoHandle::BottomRight, GizmoHandle::Bottom, GizmoHandle::BottomLeft, GizmoHandle::Left};

    float hs = m_HandleSize + 4.f;
    for(int i=0;i<8;i++) {
        if(fabsf(mousePos.x - handles[i].x) < hs && fabsf(mousePos.y - handles[i].y) < hs) return types[i];
    }

    // Rotation handle - above top center
    glm::vec2 rotHandle = rotatePoint({objPos.x, objPos.y - hh - m_RotationHandleDist});
    if(fabsf(mousePos.x - rotHandle.x) < hs && fabsf(mousePos.y - rotHandle.y) < hs) return GizmoHandle::Rotation;

    // Inside = move
    // Simple point in OBB test using inverse rotate
    glm::vec2 rel = mousePos - objPos;
    glm::vec2 local = {rel.x*cosR + rel.y*sinR, -rel.x*sinR + rel.y*cosR};
    if(fabsf(local.x) <= hw && fabsf(local.y) <= hh) return GizmoHandle::Center;

    return GizmoHandle::None;
}

bool TransformGizmo::DrawAndHandle(const glm::vec2& viewportPos, const glm::vec2& viewportSize,
                                   const glm::vec2& objectScreenPos, const glm::vec2& objectScreenSize,
                                   float rotation, Transform& transform, float zoom) {

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    // Alternative: use window draw list - but foreground ensures visibility
    dl = ImGui::GetWindowDrawList();

    ImVec2 mouse = ImGui::GetMousePos();
    glm::vec2 mousePos = {mouse.x, mouse.y};

    bool changed = false;

    // Hit test when not active
    if(m_Active==GizmoHandle::None) {
        m_Hovered = HitTest(mousePos, objectScreenPos, objectScreenSize, rotation);
    }

    // Handle mouse down
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
        if(m_Hovered != GizmoHandle::None) {
            m_Active = m_Hovered;
            m_DragStartMouse = mousePos;
            m_DragStartPos = transform.Position;
            m_DragStartScale = transform.Scale;
            m_DragStartRotation = transform.Rotation;
        }
    }

    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        m_Active = GizmoHandle::None;
    }

    // Handle dragging
    if(m_Active != GizmoHandle::None && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        glm::vec2 delta = mousePos - m_DragStartMouse;
        // Convert screen delta to world delta (divide by zoom inverse? viewport zoom handling)
        // Our objectScreenSize already accounts for zoom, so world delta = screen delta * zoom
        glm::vec2 worldDelta = delta * zoom * 0.5f; // factor tuned for viewport FBO scale

        switch(m_Active) {
            case GizmoHandle::Center: {
                transform.Position = m_DragStartPos + worldDelta;
                changed = true;
                break;
            }
            case GizmoHandle::Right: {
                float scaleX = 1.f + delta.x / (objectScreenSize.x*0.5f);
                transform.Scale.x = glm::max(0.05f, m_DragStartScale.x * scaleX);
                changed = true; break;
            }
            case GizmoHandle::Left: {
                float scaleX = 1.f - delta.x / (objectScreenSize.x*0.5f);
                transform.Scale.x = glm::max(0.05f, m_DragStartScale.x * scaleX);
                changed = true; break;
            }
            case GizmoHandle::Bottom: {
                float scaleY = 1.f + delta.y / (objectScreenSize.y*0.5f);
                transform.Scale.y = glm::max(0.05f, m_DragStartScale.y * scaleY);
                changed = true; break;
            }
            case GizmoHandle::Top: {
                float scaleY = 1.f - delta.y / (objectScreenSize.y*0.5f);
                transform.Scale.y = glm::max(0.05f, m_DragStartScale.y * scaleY);
                changed = true; break;
            }
            case GizmoHandle::TopLeft:
            case GizmoHandle::TopRight:
            case GizmoHandle::BottomRight:
            case GizmoHandle::BottomLeft: {
                float sx = 1.f + (m_Active==GizmoHandle::TopLeft||m_Active==GizmoHandle::BottomLeft ? -delta.x : delta.x) / (objectScreenSize.x*0.5f);
                float sy = 1.f + (m_Active==GizmoHandle::TopLeft||m_Active==GizmoHandle::TopRight ? -delta.y : delta.y) / (objectScreenSize.y*0.5f);
                float avg = (sx+sy)*0.5f;
                if(ImGui::GetIO().KeyShift) { // uniform scale if shift
                    transform.Scale = m_DragStartScale * avg;
                } else {
                    transform.Scale.x = glm::max(0.05f, m_DragStartScale.x * sx);
                    transform.Scale.y = glm::max(0.05f, m_DragStartScale.y * sy);
                }
                changed = true; break;
            }
            case GizmoHandle::Rotation: {
                glm::vec2 center = objectScreenPos;
                glm::vec2 v1 = m_DragStartMouse - center;
                glm::vec2 v2 = mousePos - center;
                float a1 = atan2f(v1.y, v1.x);
                float a2 = atan2f(v2.y, v2.x);
                float dAngle = glm::degrees(a2 - a1);
                transform.Rotation = m_DragStartRotation + dAngle;
                changed = true; break;
            }
            default: break;
        }
    }

    // Draw OBB box
    float hw = objectScreenSize.x*0.5f, hh = objectScreenSize.y*0.5f;
    float rad = glm::radians(rotation);
    float cosR = cosf(rad), sinR = sinf(rad);
    auto rotPt = [&](glm::vec2 p) -> ImVec2 {
        glm::vec2 rel = {p.x - objectScreenPos.x, p.y - objectScreenPos.y};
        glm::vec2 r = {rel.x*cosR - rel.y*sinR, rel.x*sinR + rel.y*cosR};
        return ImVec2(objectScreenPos.x + r.x, objectScreenPos.y + r.y);
    };

    ImVec2 tl = rotPt({objectScreenPos.x - hw, objectScreenPos.y - hh});
    ImVec2 tr = rotPt({objectScreenPos.x + hw, objectScreenPos.y - hh});
    ImVec2 br = rotPt({objectScreenPos.x + hw, objectScreenPos.y + hh});
    ImVec2 bl = rotPt({objectScreenPos.x - hw, objectScreenPos.y + hh});

    dl->AddLine(tl,tr, IM_COL32(100,200,255,255), 1.5f);
    dl->AddLine(tr,br, IM_COL32(100,200,255,255), 1.5f);
    dl->AddLine(br,bl, IM_COL32(100,200,255,255), 1.5f);
    dl->AddLine(bl,tl, IM_COL32(100,200,255,255), 1.5f);

    // Draw handles
    glm::vec2 handlePos[8] = {
        {objectScreenPos.x - hw, objectScreenPos.y - hh},
        {objectScreenPos.x, objectScreenPos.y - hh},
        {objectScreenPos.x + hw, objectScreenPos.y - hh},
        {objectScreenPos.x + hw, objectScreenPos.y},
        {objectScreenPos.x + hw, objectScreenPos.y + hh},
        {objectScreenPos.x, objectScreenPos.y + hh},
        {objectScreenPos.x - hw, objectScreenPos.y + hh},
        {objectScreenPos.x - hw, objectScreenPos.y}
    };
    GizmoHandle types[8] = {GizmoHandle::TopLeft, GizmoHandle::Top, GizmoHandle::TopRight, GizmoHandle::Right,
                            GizmoHandle::BottomRight, GizmoHandle::Bottom, GizmoHandle::BottomLeft, GizmoHandle::Left};
    ImU32 handleCol = IM_COL32(200,200,220,255);
    for(int i=0;i<8;i++) {
        ImVec2 rp = rotPt(handlePos[i]);
        bool hov = (m_Hovered==types[i] || m_Active==types[i]);
        bool act = (m_Active==types[i]);
        DrawHandle(dl, {rp.x, rp.y}, hov, act, handleCol);
    }

    // Rotation handle
    glm::vec2 rotHandleLocal = {objectScreenPos.x, objectScreenPos.y - hh - m_RotationHandleDist};
    ImVec2 rotHandle = rotPt(rotHandleLocal);
    ImVec2 topCenter = rotPt({objectScreenPos.x, objectScreenPos.y - hh});
    dl->AddLine(topCenter, rotHandle, IM_COL32(100,200,255,255), 1.5f);
    DrawHandle(dl, {rotHandle.x, rotHandle.y}, m_Hovered==GizmoHandle::Rotation, m_Active==GizmoHandle::Rotation, IM_COL32(50,255,100,255));
    // Center handle
    DrawHandle(dl, objectScreenPos, m_Hovered==GizmoHandle::Center, m_Active==GizmoHandle::Center, IM_COL32(255,255,255,255));

    return changed;
}

} // namespace
