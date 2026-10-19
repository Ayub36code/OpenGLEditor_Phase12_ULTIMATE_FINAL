
#include "editor/LayerStack.h"
#include "graphics/Renderer.h"
#include "graphics/Camera.h"

namespace Editor {

LayerStack::LayerStack(){}

std::shared_ptr<Layer> LayerStack::AddLayer(const std::string& name, uint32_t w, uint32_t h, const glm::vec4& color){
    std::string n=name.empty()?("Layer " + std::to_string(m_Layers.size()+1)):name;
    auto layer=std::make_shared<Layer>(n,w,h,color);
    layer->Index=(int)m_Layers.size();
    m_Layers.push_back(layer); m_ActiveIndex=(int)m_Layers.size()-1; return layer;
}

std::shared_ptr<Layer> LayerStack::AddImageLayer(std::shared_ptr<Image> img){
    if(!img) return nullptr;
    auto layer=AddLayer(img->GetMetadata().Name, img->GetWidth(), img->GetHeight());
    layer->SetImage(img); return layer;
}

void LayerStack::DeleteLayer(int index){ if(index<0||index>=(int)m_Layers.size()) return; m_Layers.erase(m_Layers.begin()+index); if(m_ActiveIndex>= (int)m_Layers.size()) m_ActiveIndex=(int)m_Layers.size()-1; }
void LayerStack::Clear(){ m_Layers.clear(); m_ActiveIndex=-1; }
std::shared_ptr<Layer> LayerStack::GetActive() const { if(m_ActiveIndex<0||m_ActiveIndex>=(int)m_Layers.size()) return nullptr; return m_Layers[m_ActiveIndex]; }
void LayerStack::SetActiveIndex(int idx){ if(idx>=0 && idx<(int)m_Layers.size()) m_ActiveIndex=idx; }

void LayerStack::Composite(Renderer& renderer, const OrthographicCamera& cam, std::shared_ptr<Framebuffer> targetFBO){
    targetFBO->Bind();
    renderer.BeginFrame({0,0,0,0});
    renderer.BeginScene(cam);
    // Checker background
    renderer.DrawCheckerboard({0,0}, {4000,4000});
    for(auto& layer : m_Layers){
        if(!layer->Visible) continue;
        float w=layer->GetWidth()*layer->TransformData.Scale.x;
        float h=layer->GetHeight()*layer->TransformData.Scale.y;
        auto& pos=layer->TransformData.Position;
        float rot=layer->TransformData.Rotation;
        if(layer->Texture) renderer.DrawQuad(pos, {w,h}, rot, layer->Texture, 1.f, {1,1,1,layer->Opacity});
        else renderer.DrawQuad(pos, {w,h}, glm::vec4(1,1,1,layer->Opacity));
    }
    renderer.EndScene();
    renderer.EndFrame();
    targetFBO->Unbind();
}

}
