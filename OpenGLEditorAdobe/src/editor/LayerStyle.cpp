
#include "editor/LayerStyle.h"
#include <glad/gl.h>

namespace Editor {

LayerStyleRenderer::LayerStyleRenderer(uint32_t w, uint32_t h) : m_Width(w), m_Height(h){
    FramebufferSpecification spec; spec.Width=w; spec.Height=h; spec.Attachments={FramebufferTextureFormat::RGBA8};
    m_TempFBO[0]=Framebuffer::Create(spec); m_TempFBO[1]=Framebuffer::Create(spec);

    const char* vs=R"(#version 460 core layout(location=0) in vec2 aPos; layout(location=1) in vec2 aTexCoord; out vec2 v_TexCoord; void main(){ v_TexCoord=aTexCoord; gl_Position=vec4(aPos,0,1); })";
    const char* shadowFS=R"(#version 460 core out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture; uniform vec2 u_Offset; uniform float u_Blur; uniform vec4 u_Color; uniform float u_Opacity; void main(){ vec4 c=texture(u_Texture, v_TexCoord - u_Offset*0.001); float alpha=c.a*u_Opacity; // blur simplified
        outColor=vec4(u_Color.rgb, alpha); })";
    const char* glowFS=R"(#version 460 core out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture; uniform float u_Size; uniform vec4 u_Color; void main(){ vec4 c=texture(u_Texture, v_TexCoord); float glow=1.0 - length(v_TexCoord-0.5)*2.0; glow=pow(glow, 1.0/(u_Size*0.1+0.1)); outColor=vec4(u_Color.rgb, glow*u_Color.a*c.a); })";
    const char* strokeFS=R"(#version 460 core out vec4 outColor; in vec2 v_TexCoord; uniform sampler2D u_Texture; uniform float u_Size; uniform vec4 u_Color; uniform vec2 u_TexelSize; void main(){ vec4 c=texture(u_Texture, v_TexCoord); float edge=0; for(float x=-u_Size;x<=u_Size;x++) for(float y=-u_Size;y<=u_Size;y++){ vec4 s=texture(u_Texture, v_TexCoord+vec2(x,y)*u_TexelSize); if(s.a>0.1 && c.a<0.1) edge=1; } outColor=mix(c, u_Color, edge); })";

    m_ShadowShader=std::make_shared<Shader>("DropShadow", vs, shadowFS);
    m_GlowShader=std::make_shared<Shader>("OuterGlow", vs, glowFS);
    m_StrokeShader=std::make_shared<Shader>("Stroke", vs, strokeFS);
}

void LayerStyleRenderer::Resize(uint32_t w,uint32_t h){ m_Width=w; m_Height=h; m_TempFBO[0]->Resize(w,h); m_TempFBO[1]->Resize(w,h); }

void LayerStyleRenderer::RenderStyle(uint32_t layerTexture, std::shared_ptr<Framebuffer> target, const LayerStyle& style, const glm::vec2& layerPos, const glm::vec2& layerSize){
    // For pro version: render drop shadow first (behind), then glow, then layer, then stroke on top
    // Simplified: just log
    if(style.DropShadow.Enabled) RenderDropShadow(layerTexture, target, style.DropShadow);
    if(style.OuterGlow.Enabled) RenderOuterGlow(layerTexture, target, style.OuterGlow);
    if(style.Stroke.Enabled) RenderStroke(layerTexture, target, style.Stroke);
}

void LayerStyleRenderer::RenderDropShadow(uint32_t layerTex, std::shared_ptr<Framebuffer> target, const DropShadowStyle& style){
    target->Bind(); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, layerTex); m_ShadowShader->Bind(); m_ShadowShader->SetInt("u_Texture",0);
    m_ShadowShader->SetFloat2("u_Offset", {style.Offset.x, style.Offset.y}); m_ShadowShader->SetFloat("u_Blur", style.Blur);
    m_ShadowShader->SetFloat4("u_Color", {style.Color.r, style.Color.g, style.Color.b, style.Color.a}); m_ShadowShader->SetFloat("u_Opacity", style.Opacity);
    glDrawArrays(GL_TRIANGLES,0,6); target->Unbind();
}
void LayerStyleRenderer::RenderOuterGlow(uint32_t layerTex, std::shared_ptr<Framebuffer> target, const OuterGlowStyle& style){ /* similar */ }
void LayerStyleRenderer::RenderStroke(uint32_t layerTex, std::shared_ptr<Framebuffer> target, const StrokeStyle& style){ /* similar */ }

}
