
#include "editor/Viewport.h"
#include "core/Log.h"

namespace Editor {

Viewport::Viewport(uint32_t width, uint32_t height) {
    FramebufferSpecification spec; spec.Width=width; spec.Height=height;
    spec.Attachments={FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth24Stencil8};
    m_Framebuffer=Framebuffer::Create(spec);
    m_Camera=std::make_unique<OrthographicCamera>(-width/2.f,width/2.f,-height/2.f,height/2.f);
    m_Zoom=1.f; m_Pan={0,0};
    TextureSpecification checkerSpec; checkerSpec.Width=16; checkerSpec.Height=16; checkerSpec.Filter=TextureFilter::Nearest; checkerSpec.Wrap=TextureWrap::Repeat;
    m_CheckerTexture=Texture2D::Create(checkerSpec);
    uint32_t data[256]; for(int y=0;y<16;y++) for(int x=0;x<16;x++){ bool even=((x/4+y/4)%2)==0; data[y*16+x]=even?0xff999999:0xff666666; }
    m_CheckerTexture->SetData(data,sizeof(data));
    m_PaintCanvas=std::make_unique<PaintCanvas>(width,height);
    m_ToolManager=std::make_unique<ToolManager>(*m_PaintCanvas);
    m_SelectionManager=std::make_unique<SelectionManager>(width,height);
    m_LayerStack.AddLayer("Background", width,height, glm::vec4(0,0,0,0));
}
void Viewport::OnResize(uint32_t w,uint32_t h){ if(w==0||h==0) return; m_Framebuffer->Resize(w,h); m_PaintCanvas->Resize(w,h); m_SelectionManager->Resize(w,h); UpdateCamera(); }
void Viewport::OnUpdate(float dt){ (void)dt; }
void Viewport::OnMouseScroll(float yOffset){ float nz=m_Zoom - yOffset*0.1f*m_Zoom; nz=glm::clamp(nz,0.02f,50.f); SetZoom(nz); }
void Viewport::OnMouseDrag(const glm::vec2& delta){ if(!m_Panning) return; m_Pan+=delta*m_Zoom*0.5f; UpdateCamera(); }
void Viewport::OnMouseButton(int button,bool pressed,const glm::vec2& pos){ if(button==2){ m_Panning=pressed; m_LastMousePos=pos; } }
void Viewport::SetZoom(float z){ m_Zoom=glm::clamp(z,0.02f,50.f); UpdateCamera(); }
void Viewport::ResetView(){ m_Zoom=1.f; m_Pan={0,0}; UpdateCamera(); }
void Viewport::UpdateCamera(){ auto spec=m_Framebuffer->GetSpecification(); float w=spec.Width*m_Zoom; float h=spec.Height*m_Zoom; float l=m_Pan.x-w/2.f, r=m_Pan.x+w/2.f, b=m_Pan.y-h/2.f, t=m_Pan.y+h/2.f; m_Camera->SetProjection(l,r,b,t); m_Camera->SetPosition({0,0,0}); }
void Viewport::SetImage(std::shared_ptr<Image> img){ m_Image=img; if(img){ bool found=false; for(auto& l: m_LayerStack.GetLayers()) if(l->ImageData==img) found=true; if(!found) m_LayerStack.AddImageLayer(img); if(!img->GetPixels().empty()) m_Histogram.CalculateFromPixels(img->GetPixels(), img->GetMetadata().Channels); } ResetView(); }
glm::vec2 Viewport::ScreenToWorld(const glm::vec2& sp,const glm::vec2& vs){ return m_Camera->ScreenToWorld(sp,vs); }
glm::vec2 Viewport::GetLayerScreenPos(std::shared_ptr<Layer> layer,const glm::vec2& vpPos,const glm::vec2& vpSize) const { if(!layer) return vpPos+vpSize*0.5f; glm::vec2 wp=layer->Transform.Position; glm::vec2 sc=vpPos+vpSize*0.5f; return sc + (wp - m_Pan)/m_Zoom; }
glm::vec2 Viewport::GetLayerScreenSize(std::shared_ptr<Layer> layer) const { if(!layer) return {0,0}; float w=layer->GetWidth()*layer->Transform.Scale.x; float h=layer->GetHeight()*layer->Transform.Scale.y; return glm::vec2(w,h)/m_Zoom; }

}
