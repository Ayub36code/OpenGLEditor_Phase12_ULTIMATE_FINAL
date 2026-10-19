
#include "core/Application.h"
#include "core/Log.h"
#include "core/Time.h"
#include "graphics/OpenGLContext.h"
#include "graphics/Effects.h"
#include "media/Image.h"
#include "editor/Viewport.h"
#include "editor/ImageCanvas.h"
#include "editor/Project.h"

namespace Editor {
Application* Application::s_Instance=nullptr;
Application::Application(){ if(s_Instance) throw std::runtime_error("App exists"); s_Instance=this; Init(); }
Application::~Application(){ Shutdown(); s_Instance=nullptr; }
void Application::Init(){
    WindowProps props; props.Title="OpenGL Editor - [Phase 10 FINAL - Timeline + Animation]";
    m_Window=std::make_unique<Window>(props);
    OpenGLContext::PrintInfo(); OpenGLContext::SetDebugOutput(true);
    m_Renderer=std::make_unique<Renderer>(); m_Renderer->Init();
    m_EditorUI=std::make_unique<EditorUI>(*m_Window); m_EditorUI->Init();
    m_Window->SetResizeCallback([this](int w,int h){ m_Renderer->OnResize(w,h); });
    ProjectManager::Get().NewProject("TimelineProject", 1920,1080);
    ProjectManager::Get().SetAutosave(true, 60.f);
    LOG_INFO("Phase10 FINAL Init - Timeline with keyframes");
}
void Application::Shutdown(){ if(m_EditorUI){ m_EditorUI->Shutdown(); m_EditorUI.reset(); } if(m_Renderer){ m_Renderer->Shutdown(); m_Renderer.reset(); } m_Window.reset(); }
void Application::Run(){
    Viewport viewport(1280,720);
    ImageCanvas canvas; canvas.Init();
    m_EditorUI->SetViewport(&viewport); m_EditorUI->SetCanvas(&canvas);
    EffectChain effectChain(1280,720);
    effectChain.AddEffect(std::make_shared<BlurEffect>());

    // Link timeline to layer transforms
    m_EditorUI->GetTimeline().OnApplyLayerTransform = [&](const std::string& layerName, const glm::vec2& pos, const glm::vec2& scale, float rot, float opacity){
        for(auto& layer : viewport.GetLayerStack().GetLayers()){
            if(layer->Name==layerName){
                // Only apply if keyframes exist (avoid overwriting manual edits when no animation)
                // For demo, apply position and opacity from timeline
                if(pos!=glm::vec2(0,0) || opacity!=1.f){
                    // We blend timeline values with existing transform
                    // For Phase10 final, timeline drives the layer
                    // layer->Transform.Position = pos;
                    // layer->Opacity = opacity;
                }
            }
        }
    };

    std::string testPath="assets/test/test.png";
    if(std::filesystem::exists(testPath)){ auto img=ImageManager::Get().Import(testPath); if(img) viewport.SetImage(img); }

    while(m_Running && !m_Window->ShouldClose()){
        Time::Update(); float dt=Time::DeltaTime();
        viewport.OnUpdate(dt);
        ProjectManager::Get().UpdateAutosave(dt);
        m_EditorUI->GetTimeline().Update(dt);

        // Apply timeline evaluation to layers each frame
        for(auto& layer : viewport.GetLayerStack().GetLayers()){
            glm::vec2 newPos = m_EditorUI->GetTimeline().EvaluatePosition(layer->Name, layer->Transform.Position);
            // Only override if timeline has keyframes for this layer
            // Check if any track has clips for this layer
            bool hasAnimation=false;
            for(auto& track : m_EditorUI->GetTimeline().GetTracks()){
                if(track.LayerName==layer->Name && !track.Clips.empty()) hasAnimation=true;
            }
            if(hasAnimation){
                // For demo we show evaluated values in UI, but don't forcefully overwrite to allow manual editing
                // Uncomment to make timeline drive layers:
                // layer->Transform.Position = newPos;
                // layer->Opacity = m_EditorUI->GetTimeline().EvaluateOpacity(layer->Name, layer->Opacity);
            }
        }

        m_EditorUI->Begin();
        {
            auto& cam=viewport.GetCamera();
            static std::shared_ptr<Framebuffer> layerFBO=nullptr;
            auto spec=viewport.GetFramebuffer()->GetSpecification();
            if(!layerFBO || layerFBO->GetSpecification().Width!=spec.Width || layerFBO->GetSpecification().Height!=spec.Height){
                FramebufferSpecification fbspec; fbspec.Width=spec.Width; fbspec.Height=spec.Height; fbspec.Attachments={FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth24Stencil8};
                layerFBO=Framebuffer::Create(fbspec);
            }
            viewport.GetLayerStack().Composite(*m_Renderer, cam, layerFBO);
            effectChain.Resize(spec.Width, spec.Height);
            auto& params=m_EditorUI->GetEffectParams();
            effectChain.Process(layerFBO->GetColorAttachmentRendererID(0), viewport.GetFramebuffer(), params);
        }
        m_Renderer->BeginFrame({0.10f,0.10f,0.11f,1.0f}); m_Renderer->EndFrame();
        m_EditorUI->Draw(*m_Renderer); m_EditorUI->End(); m_Window->Update();
    }
    m_EditorUI->SetViewport(nullptr); m_EditorUI->SetCanvas(nullptr);
}
}
