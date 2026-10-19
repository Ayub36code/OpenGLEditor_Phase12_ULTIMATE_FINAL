
#include "graphics/RenderTarget.h"

namespace Editor {
RenderTarget::RenderTarget(uint32_t w,uint32_t h){
    FramebufferSpecification spec; spec.Width=w; spec.Height=h;
    spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth24Stencil8 };
    m_Framebuffer = Framebuffer::Create(spec);
    m_Camera = std::make_unique<OrthographicCamera>(-w/2.f, w/2.f, -h/2.f, h/2.f);
}
void RenderTarget::Resize(uint32_t w,uint32_t h){
    m_Framebuffer->Resize(w,h);
    m_Camera->SetProjection(-w/2.f, w/2.f, -h/2.f, h/2.f);
}
}
