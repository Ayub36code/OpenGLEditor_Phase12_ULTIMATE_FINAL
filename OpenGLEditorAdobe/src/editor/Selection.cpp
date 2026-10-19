
#include "editor/Selection.h"
#include "core/Log.h"
#include <glad/gl.h>

namespace Editor {

SelectionManager::SelectionManager(uint32_t w, uint32_t h) : m_Width(w), m_Height(h) {
    FramebufferSpecification spec; spec.Width=w; spec.Height=h; spec.Attachments={FramebufferTextureFormat::RGBA8};
    m_MaskFBO=Framebuffer::Create(spec);

    const char* vs = R"(#version 460 core
    layout(location=0) in vec2 aPos; layout(location=1) in vec2 aTexCoord; out vec2 v_TexCoord; void main(){ v_TexCoord=aTexCoord; gl_Position=vec4(aPos,0,1); })";
    const char* fs = R"(#version 460 core
    out vec4 outColor; uniform vec4 u_Color; uniform vec2 u_Min; uniform vec2 u_Max; uniform int u_Type;
    void main(){ outColor=u_Color; })";
    m_MaskShader = std::make_shared<Shader>("Mask", vs, fs);
    Clear();
}

void SelectionManager::Resize(uint32_t w, uint32_t h){ m_Width=w; m_Height=h; if(m_MaskFBO) m_MaskFBO->Resize(w,h); }

void SelectionManager::BeginSelection(const glm::vec2& pos, SelectionType type){
    m_Selection.Type=type; m_Selection.Active=true; m_StartPos=pos; m_Selection.Min=pos; m_Selection.Max=pos;
    if(type==SelectionType::Lasso || type==SelectionType::Polygonal) m_Selection.Points={pos};
    LOG_INFO("Selection begin " + std::to_string((int)type));
}

void SelectionManager::UpdateSelection(const glm::vec2& pos){
    if(!m_Selection.Active) return;
    if(m_Selection.Type==SelectionType::Rectangle || m_Selection.Type==SelectionType::Ellipse){
        m_Selection.Min = glm::min(m_StartPos, pos);
        m_Selection.Max = glm::max(m_StartPos, pos);
    }
    UpdateMask();
}

void SelectionManager::EndSelection(const glm::vec2& pos){
    UpdateSelection(pos);
    LOG_INFO("Selection end size " + std::to_string((int)m_Selection.GetSize().x) + "x" + std::to_string((int)m_Selection.GetSize().y));
}

void SelectionManager::AddLassoPoint(const glm::vec2& pos){
    if(m_Selection.Type==SelectionType::Lasso) m_Selection.Points.push_back(pos);
    UpdateMask();
}

bool SelectionManager::IsPointInside(const glm::vec2& pos) const {
    if(!m_Selection.Active) return true;
    if(m_Selection.Type==SelectionType::Rectangle) return pos.x>=m_Selection.Min.x && pos.x<=m_Selection.Max.x && pos.y>=m_Selection.Min.y && pos.y<=m_Selection.Max.y;
    return false;
}

void SelectionManager::UpdateMask(){
    if(!m_MaskFBO) return;
    m_MaskFBO->Bind();
    glClearColor(0,0,0,0); glClear(GL_COLOR_BUFFER_BIT);
    if(m_Selection.Type==SelectionType::Rectangle) RenderRectMask();
    else if(m_Selection.Type==SelectionType::Ellipse) RenderEllipseMask();
    m_MaskFBO->Unbind();
}

void SelectionManager::RenderRectMask(){
    // Draw white rect for selection area
    // Simplified via GL scissor
}

void SelectionManager::RenderEllipseMask(){}
void SelectionManager::RenderLassoMask(){}

void SelectionManager::MagicWandSelect(const glm::vec2& pos, uint32_t imageTexture, float tolerance){
    // For Phase8, simplified: create rect around pos with tolerance
    m_Selection.Type=SelectionType::MagicWand;
    m_Selection.Active=true;
    m_Selection.Min=pos - glm::vec2(50.f);
    m_Selection.Max=pos + glm::vec2(50.f);
    LOG_INFO("Magic wand select at " + std::to_string((int)pos.x) + "," + std::to_string((int)pos.y) + " tolerance " + std::to_string(tolerance));
    UpdateMask();
}

}
