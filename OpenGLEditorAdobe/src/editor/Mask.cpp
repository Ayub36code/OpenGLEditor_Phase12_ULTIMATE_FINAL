
#include "editor/Mask.h"
#include <glad/gl.h>

namespace Editor {

LayerMask::LayerMask(uint32_t w, uint32_t h){
    FramebufferSpecification spec; spec.Width=w; spec.Height=h; spec.Attachments={FramebufferTextureFormat::R8};
    m_FBO=Framebuffer::Create(spec);
    Clear(1.f);

    TextureSpecification tipSpec; tipSpec.Width=64; tipSpec.Height=64; tipSpec.Filter=TextureFilter::Linear;
    m_BrushTip=Texture2D::Create(tipSpec);
    std::vector<unsigned char> data(64*64*4);
    for(int y=0;y<64;y++) for(int x=0;x<64;x++){ float dx=(x-32)/32.f, dy=(y-32)/32.f; float d=sqrtf(dx*dx+dy*dy); float a=1.f-glm::clamp(d,0.f,1.f); data[(y*64+x)*4]=255; data[(y*64+x)*4+1]=255; data[(y*64+x)*4+2]=255; data[(y*64+x)*4+3]=(unsigned char)(a*255); }
    m_BrushTip->SetData(data.data(), data.size());
}

void LayerMask::Resize(uint32_t w, uint32_t h){ if(m_FBO) m_FBO->Resize(w,h); }
void LayerMask::Clear(float value){
    m_FBO->Bind(); glClearColor(value,value,value,1.f); glClear(GL_COLOR_BUFFER_BIT); m_FBO->Unbind();
}
void LayerMask::Paint(const glm::vec2& from, const glm::vec2& to, float brushSize, float opacity, bool erase){
    m_FBO->Bind();
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Draw brush stroke to mask
    m_FBO->Unbind();
}

}
