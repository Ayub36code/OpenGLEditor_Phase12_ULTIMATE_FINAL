
#include "graphics/Camera.h"

namespace Editor {

OrthographicCamera::OrthographicCamera(float left,float right,float bottom,float top)
    : m_Left(left), m_Right(right), m_Bottom(bottom), m_Top(top) {
    m_Projection = glm::ortho(left,right,bottom,top,-1.f,1.f);
    m_View = glm::mat4(1.f);
    m_ViewProjection = m_Projection * m_View;
}

void OrthographicCamera::SetProjection(float left,float right,float bottom,float top){
    m_Left=left; m_Right=right; m_Bottom=bottom; m_Top=top;
    RecalculateProjection();
}

void OrthographicCamera::RecalculateProjection(){
    float aspect = (m_Right - m_Left) / (m_Top - m_Bottom);
    float zoom = m_Zoom;
    // zoom scales view size: smaller zoom = closer
    float w = (m_Right - m_Left) * zoom * 0.5f;
    float h = (m_Top - m_Bottom) * zoom * 0.5f;
    float cx = m_Position.x; float cy = m_Position.y;
    m_Projection = glm::ortho(cx - w, cx + w, cy - h, cy + h, -1.f, 1.f);
    m_ViewProjection = m_Projection * m_View;
}

void OrthographicCamera::Recalculate(){
    glm::mat4 transform = glm::translate(glm::mat4(1.f), m_Position) *
                          glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation), glm::vec3(0,0,1));
    m_View = glm::inverse(transform);
    m_ViewProjection = m_Projection * m_View;
}

glm::vec2 OrthographicCamera::ScreenToWorld(glm::vec2 screenPos, glm::vec2 viewportSize) const {
    // Normalized device to world: simple orthographic unproject
    glm::vec2 ndc = (screenPos / viewportSize) * 2.f - 1.f;
    ndc.y = -ndc.y;
    glm::mat4 invVP = glm::inverse(m_ViewProjection);
    glm::vec4 world = invVP * glm::vec4(ndc,0,1);
    return glm::vec2(world);
}

}
