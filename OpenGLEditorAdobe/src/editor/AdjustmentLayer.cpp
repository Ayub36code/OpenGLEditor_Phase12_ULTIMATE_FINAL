
#include "editor/AdjustmentLayer.h"
#include <glad/gl.h>

namespace Editor {

static const char* vs = R"(#version 460 core
layout(location=0) in vec2 aPos; layout(location=1) in vec2 aTexCoord; out vec2 v_TexCoord; void main(){ v_TexCoord=aTexCoord; gl_Position=vec4(aPos,0,1); })";

AdjustmentLayer::AdjustmentLayer(AdjustmentType type) : m_Type(type) {
    switch(type){
        case AdjustmentType::Levels: m_Name="Levels"; break;
        case AdjustmentType::Curves: m_Name="Curves"; break;
        case AdjustmentType::HueSaturation: m_Name="Hue/Saturation"; break;
        default: m_Name="Adjustment"; break;
    }
}

LevelsAdjustment::LevelsAdjustment() : AdjustmentLayer(AdjustmentType::Levels) {
    const char* fs = R"(#version 460 core
    out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture;
    uniform float u_Shadows=0, u_Midtones=1, u_Highlights=255, u_OutShadows=0, u_OutHighlights=255;
    void main(){
        vec4 c=texture(u_Texture, v_TexCoord);
        // Levels: input levels to output
        float inRange=u_Highlights-u_Shadows;
        vec3 rgb=(c.rgb*255.0 - u_Shadows)/max(inRange,0.001);
        rgb=pow(rgb, vec3(1.0/u_Midtones));
        rgb=rgb*(u_OutHighlights-u_OutShadows)/255.0 + u_OutShadows/255.0;
        outColor=vec4(clamp(rgb,0.0,1.0), c.a);
    })";
    m_Shader=std::make_shared<Shader>("Levels", vs, fs);
}
void LevelsAdjustment::Apply(uint32_t in, std::shared_ptr<Framebuffer> target, const glm::vec4& params){
    target->Bind(); glBindTexture(GL_TEXTURE_2D, in); m_Shader->Bind(); m_Shader->SetInt("u_Texture",0);
    m_Shader->SetFloat("u_Shadows", Shadows); m_Shader->SetFloat("u_Midtones", Midtones); m_Shader->SetFloat("u_Highlights", Highlights);
    m_Shader->SetFloat("u_OutShadows", OutputShadows); m_Shader->SetFloat("u_OutHighlights", OutputHighlights);
    glDrawArrays(GL_TRIANGLES,0,6); target->Unbind();
}
void LevelsAdjustment::DrawUI(){}

CurvesAdjustment::CurvesAdjustment() : AdjustmentLayer(AdjustmentType::Curves) {
    const char* fs = R"(#version 460 core
    out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture; uniform vec2 u_Points[8]; uniform int u_PointCount;
    float curve(float x){
        // Catmull-Rom spline through points
        if(u_PointCount<2) return x;
        for(int i=0;i<u_PointCount-1;i++){
            if(x>=u_Points[i].x && x<=u_Points[i+1].x){
                float t=(x-u_Points[i].x)/(u_Points[i+1].x-u_Points[i].x);
                return mix(u_Points[i].y, u_Points[i+1].y, t);
            }
        }
        return x;
    }
    void main(){ vec4 c=texture(u_Texture, v_TexCoord); c.r=curve(c.r); c.g=curve(c.g); c.b=curve(c.b); outColor=c; })";
    m_Shader=std::make_shared<Shader>("Curves", vs, fs);
}
void CurvesAdjustment::Apply(uint32_t in, std::shared_ptr<Framebuffer> target, const glm::vec4& params){
    target->Bind(); glBindTexture(GL_TEXTURE_2D,in); m_Shader->Bind(); m_Shader->SetInt("u_Texture",0);
    m_Shader->SetInt("u_PointCount", (int)Points.size());
    for(size_t i=0;i<Points.size() && i<8; ++i) m_Shader->SetFloat2(("u_Points["+std::to_string(i)+"]").c_str(), {Points[i].x, Points[i].y});
    glDrawArrays(GL_TRIANGLES,0,6); target->Unbind();
}
void CurvesAdjustment::DrawUI(){}

HueSatAdjustment::HueSatAdjustment() : AdjustmentLayer(AdjustmentType::HueSaturation) {
    const char* fs = R"(#version 460 core
    out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture; uniform float u_Hue=0, u_Sat=0, u_Light=0;
    vec3 rgb2hsv(vec3 c){ vec4 K=vec4(0.0,-1.0/3.0,2.0/3.0,-1.0); vec4 p=mix(vec4(c.bg,K.wz),vec4(c.gb,K.xy),step(c.b,c.g)); vec4 q=mix(vec4(p.xyw,c.r),vec4(c.r,p.yzx),step(p.x,c.r)); float d=q.x-min(q.w,q.y); float e=1.0e-10; return vec3(abs(q.z+(q.w-q.y)/(6.0*d+e)), d/(q.x+e), q.x); }
    vec3 hsv2rgb(vec3 c){ vec4 K=vec4(1.0,2.0/3.0,1.0/3.0,3.0); vec3 p=abs(fract(c.xxx+K.xyz)*6.0-K.www); return c.z*mix(K.xxx,clamp(p-K.xxx,0.0,1.0),c.y); }
    void main(){ vec4 c=texture(u_Texture, v_TexCoord); vec3 hsv=rgb2hsv(c.rgb); hsv.x=fract(hsv.x+u_Hue/360.0); hsv.y=clamp(hsv.y+u_Sat/100.0,0.0,1.0); hsv.z=clamp(hsv.z+u_Light/100.0,0.0,1.0); c.rgb=hsv2rgb(hsv); outColor=c; })";
    m_Shader=std::make_shared<Shader>("HueSat", vs, fs);
}
void HueSatAdjustment::Apply(uint32_t in, std::shared_ptr<Framebuffer> target, const glm::vec4& params){
    target->Bind(); glBindTexture(GL_TEXTURE_2D,in); m_Shader->Bind(); m_Shader->SetInt("u_Texture",0);
    m_Shader->SetFloat("u_Hue", Hue); m_Shader->SetFloat("u_Sat", Saturation); m_Shader->SetFloat("u_Light", Lightness);
    glDrawArrays(GL_TRIANGLES,0,6); target->Unbind();
}
void HueSatAdjustment::DrawUI(){}

AdjustmentStack::AdjustmentStack(uint32_t w, uint32_t h) : m_Width(w), m_Height(h){
    FramebufferSpecification spec; spec.Width=w; spec.Height=h; spec.Attachments={FramebufferTextureFormat::RGBA8};
    m_FBO[0]=Framebuffer::Create(spec); m_FBO[1]=Framebuffer::Create(spec);
    const char* copyFS=R"(#version 460 core out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture; void main(){ outColor=texture(u_Texture, v_TexCoord); })";
    m_CopyShader=std::make_shared<Shader>("CopyAdj", vs, copyFS);
}
void AdjustmentStack::Resize(uint32_t w,uint32_t h){ m_Width=w; m_Height=h; m_FBO[0]->Resize(w,h); m_FBO[1]->Resize(w,h); }
void AdjustmentStack::AddAdjustment(std::shared_ptr<AdjustmentLayer> adj){ m_Adjustments.push_back(adj); }
void AdjustmentStack::RemoveAdjustment(int idx){ if(idx>=0 && idx<(int)m_Adjustments.size()) m_Adjustments.erase(m_Adjustments.begin()+idx); }
void AdjustmentStack::MoveAdjustment(int from,int to){ if(from<0||from>=(int)m_Adjustments.size()||to<0||to>=(int)m_Adjustments.size()) return; auto a=m_Adjustments[from]; m_Adjustments.erase(m_Adjustments.begin()+from); m_Adjustments.insert(m_Adjustments.begin()+to, a); }
void AdjustmentStack::Apply(uint32_t inputTexture, std::shared_ptr<Framebuffer> targetFBO){
    if(m_Adjustments.empty()){
        targetFBO->Bind(); glBindTexture(GL_TEXTURE_2D, inputTexture); m_CopyShader->Bind(); m_CopyShader->SetInt("u_Texture",0); glDrawArrays(GL_TRIANGLES,0,6); targetFBO->Unbind(); return;
    }
    uint32_t curIn=inputTexture;
    std::shared_ptr<Framebuffer> curTarget=m_FBO[0];
    for(size_t i=0;i<m_Adjustments.size();++i){
        if(!m_Adjustments[i]->IsEnabled()) continue;
        bool last=(i==m_Adjustments.size()-1);
        auto dst=last?targetFBO:curTarget;
        m_Adjustments[i]->Apply(curIn, dst, glm::vec4(0));
        curIn=dst->GetColorAttachmentRendererID(0);
        if(!last) curTarget=(curTarget==m_FBO[0])?m_FBO[1]:m_FBO[0];
    }
}

}
