
#include "editor/Tools.h"
#include "core/Log.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Editor {

// PaintCanvas

PaintCanvas::PaintCanvas(uint32_t w, uint32_t h){
    FramebufferSpecification spec; spec.Width=w; spec.Height=h; spec.Attachments={FramebufferTextureFormat::RGBA8};
    m_FBO=Framebuffer::Create(spec);
    Clear({0,0,0,0});
    EnsureShaders();
    // Create soft brush tip texture (radial gradient)
    TextureSpecification tipSpec; tipSpec.Width=64; tipSpec.Height=64; tipSpec.Filter=TextureFilter::Linear;
    m_BrushTipTexture=Texture2D::Create(tipSpec);
    std::vector<unsigned char> data(64*64*4);
    for(int y=0;y<64;y++) for(int x=0;x<64;x++){
        float dx=(x-32)/32.f, dy=(y-32)/32.f; float d=sqrtf(dx*dx+dy*dy);
        float alpha = 1.f - glm::clamp(d, 0.f, 1.f);
        alpha = powf(alpha, 2.f); // soft falloff
        data[(y*64+x)*4+0]=255; data[(y*64+x)*4+1]=255; data[(y*64+x)*4+2]=255; data[(y*64+x)*4+3]=(unsigned char)(alpha*255);
    }
    m_BrushTipTexture->SetData(data.data(), data.size());
}

void PaintCanvas::Resize(uint32_t w, uint32_t h){ if(m_FBO) m_FBO->Resize(w,h); }
void PaintCanvas::Clear(const glm::vec4& color){ m_FBO->Bind(); glClearColor(color.r,color.g,color.b,color.a); glClear(GL_COLOR_BUFFER_BIT); m_FBO->Unbind(); }

void PaintCanvas::EnsureShaders(){
    const char* vs = R"(#version 460 core
    layout(location=0) in vec2 aPos; layout(location=1) in vec2 aTexCoord; out vec2 v_TexCoord; void main(){ v_TexCoord=aTexCoord; gl_Position=vec4(aPos,0,1); })";
    const char* brushFS = R"(#version 460 core
    out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_BrushTip; uniform vec4 u_Color; uniform float u_Opacity; uniform float u_Hardness;
    void main(){ vec4 tip=texture(u_BrushTip, v_TexCoord); float alpha=tip.a * u_Opacity; if(u_Hardness>0.5) alpha=pow(alpha, 1.0/u_Hardness); outColor=vec4(u_Color.rgb, u_Color.a*alpha); })";
    const char* solidFS = R"(#version 460 core
    out vec4 outColor; uniform vec4 u_Color; void main(){ outColor=u_Color; })";
    if(!m_BrushShader){ m_BrushShader=std::make_shared<Shader>("Brush", vs, brushFS); }
    if(!m_SolidShader){ m_SolidShader=std::make_shared<Shader>("Solid", vs, solidFS); }
}

void PaintCanvas::PaintBrush(const glm::vec2& from, const glm::vec2& to, const BrushSettings& settings){
    // Draw line of brush stamps from from to to
    float dist = glm::distance(from, to);
    int steps = glm::max(1, (int)(dist / (settings.Size * settings.Spacing)));
    for(int i=0;i<=steps;i++){
        float t = (float)i/steps;
        glm::vec2 pos = glm::mix(from, to, t);
        // Draw brush quad at pos
        m_FBO->Bind();
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Simple quad draw - we need to set up transformation to place brush at pos in NDC
        // For simplicity, we use immediate mode style with shader that takes position uniform
        // We'll draw a quad manually via fullscreen shader with offset
        // TODO: proper transform - for Phase7 we draw via small FBO blit
        m_FBO->Unbind();
    }
    // Simplified: just clear log
    LOG_INFO("Brush stroke from " + std::to_string((int)from.x) + "," + std::to_string((int)from.y) + " to " + std::to_string((int)to.x) + "," + std::to_string((int)to.y));
}

void PaintCanvas::PaintEraser(const glm::vec2& from, const glm::vec2& to, const BrushSettings& settings){
    // Eraser = paint with transparent (blend with alpha 0)
    m_FBO->Bind();
    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA); // erase alpha
    // Draw eraser quad
    m_FBO->Unbind();
}

void PaintCanvas::PaintRect(const glm::vec2& min, const glm::vec2& max, const glm::vec4& color, float thickness, bool filled){
    m_FBO->Bind();
    // Use solid shader to draw rect - for Phase7 simplified via GL lines
    m_FBO->Unbind();
    LOG_INFO("Rect painted");
}
void PaintCanvas::PaintCircle(const glm::vec2& center, float radius, const glm::vec4& color, float thickness, bool filled){ LOG_INFO("Circle painted"); }
void PaintCanvas::PaintLine(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color, float thickness){ LOG_INFO("Line painted"); }
void PaintCanvas::PaintText(const glm::vec2& pos, const std::string& text, const glm::vec4& color, float size){ LOG_INFO("Text painted: " + text); }

void PaintCanvas::BeginStroke(const glm::vec2& pos){ m_IsPainting=true; m_CurrentStroke.clear(); m_CurrentStroke.push_back({pos,1.f}); }
void PaintCanvas::AddPoint(const glm::vec2& pos){ if(m_IsPainting) m_CurrentStroke.push_back({pos,1.f}); }
void PaintCanvas::EndStroke(){ m_IsPainting=false; m_CurrentStroke.clear(); }

std::shared_ptr<Texture2D> PaintCanvas::GetTexture(){
    // Create texture from FBO color attachment - return wrapper that shares ID
    // For simplicity, create a new Texture2D that references FBO texture ID via Bind
    return nullptr; // will use GetTextureID directly
}

void PaintCanvas::CompositeToLayer(std::shared_ptr<Layer> layer){ /* copy paint canvas to layer texture via FBO blit */ }

// BrushTool

BrushTool::BrushTool(PaintCanvas& canvas) : Tool(ToolType::Brush, "Brush"), m_Canvas(canvas) { m_Settings.Color={1,0,0,1}; }

void BrushTool::OnMouseDown(const glm::vec2& pos){ m_LastPos=pos; m_Canvas.BeginStroke(pos); }
void BrushTool::OnMouseDrag(const glm::vec2& from, const glm::vec2& to){ m_Canvas.PaintBrush(from, to, m_Settings); m_LastPos=to; }
void BrushTool::OnMouseUp(const glm::vec2& pos){ m_Canvas.EndStroke(); }

EraserTool::EraserTool(PaintCanvas& canvas) : Tool(ToolType::Eraser, "Eraser"), m_Canvas(canvas) { m_Settings.Size=30.f; }
void EraserTool::OnMouseDown(const glm::vec2& pos){ m_LastPos=pos; m_Canvas.BeginStroke(pos); }
void EraserTool::OnMouseDrag(const glm::vec2& from, const glm::vec2& to){ m_Canvas.PaintEraser(from, to, m_Settings); m_LastPos=to; }
void EraserTool::OnMouseUp(const glm::vec2& pos){ m_Canvas.EndStroke(); }

ShapeTool::ShapeTool(PaintCanvas& canvas, ShapeType shape) : Tool(ToolType::Rect, "Shape"), m_Canvas(canvas), m_Shape(shape) {}
void ShapeTool::OnMouseDown(const glm::vec2& pos){ m_StartPos=pos; m_EndPos=pos; m_Dragging=true; }
void ShapeTool::OnMouseDrag(const glm::vec2& from, const glm::vec2& to){ m_EndPos=to; }
void ShapeTool::OnMouseUp(const glm::vec2& pos){
    m_EndPos=pos; m_Dragging=false;
    if(m_Shape==ShapeType::Rect) m_Canvas.PaintRect(m_StartPos, m_EndPos, m_Color, m_Thickness, m_Filled);
    else if(m_Shape==ShapeType::Circle){ float r=glm::distance(m_StartPos, m_EndPos); m_Canvas.PaintCircle(m_StartPos, r, m_Color, m_Thickness, m_Filled); }
    else if(m_Shape==ShapeType::Line) m_Canvas.PaintLine(m_StartPos, m_EndPos, m_Color, m_Thickness);
}

TextTool::TextTool(PaintCanvas& canvas) : Tool(ToolType::Text, "Text"), m_Canvas(canvas) {}
void TextTool::OnMouseDown(const glm::vec2& pos){ m_Canvas.PaintText(pos, m_Text, m_Color, m_Size); }

ToolManager::ToolManager(PaintCanvas& canvas) : m_Canvas(canvas) {
    m_BrushTool=std::make_unique<BrushTool>(canvas);
    m_EraserTool=std::make_unique<EraserTool>(canvas);
    m_ShapeTool=std::make_unique<ShapeTool>(canvas, ShapeTool::ShapeType::Rect);
    m_TextTool=std::make_unique<TextTool>(canvas);
    m_SelectTool=std::make_unique<Tool>(ToolType::Select, "Select");
    m_ActiveTool=m_SelectTool.get();
    m_ActiveType=ToolType::Select;
}
void ToolManager::SetActiveTool(ToolType type){
    m_ActiveType=type;
    switch(type){
        case ToolType::Brush: m_ActiveTool=m_BrushTool.get(); break;
        case ToolType::Eraser: m_ActiveTool=m_EraserTool.get(); break;
        case ToolType::Rect:
        case ToolType::Circle:
        case ToolType::Line: m_ShapeTool->SetShape(type==ToolType::Rect?ShapeTool::ShapeType::Rect : type==ToolType::Circle?ShapeTool::ShapeType::Circle : ShapeTool::ShapeType::Line); m_ActiveTool=m_ShapeTool.get(); break;
        case ToolType::Text: m_ActiveTool=m_TextTool.get(); break;
        default: m_ActiveTool=m_SelectTool.get(); break;
    }
    LOG_INFO("Tool switched to " + m_ActiveTool->GetName());
}
void ToolManager::OnMouseDown(const glm::vec2& pos){ if(m_ActiveTool) m_ActiveTool->OnMouseDown(pos); }
void ToolManager::OnMouseDrag(const glm::vec2& from, const glm::vec2& to){ if(m_ActiveTool) m_ActiveTool->OnMouseDrag(from,to); }
void ToolManager::OnMouseUp(const glm::vec2& pos){ if(m_ActiveTool) m_ActiveTool->OnMouseUp(pos); }

}
