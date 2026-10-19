
#include "core/Application.h"
#include "core/Log.h"
#include "core/Time.h"
#include "graphics/OpenGLContext.h"
#include "graphics/Effects.h"
#include "media/Image.h"
#include "editor/Viewport.h"
#include "editor/ImageCanvas.h"

namespace Editor {
Application* Application::s_Instance=nullptr;
Application::Application(){ if(s_Instance) throw std::runtime_error("App exists"); s_Instance=this; Init(); }
Application::~Application(){ Shutdown(); s_Instance=nullptr; }
void Application::Init(){
    WindowProps props; props.Title="OpenGL Editor - [Phase 7 - Brush/Text/Shape Tools]";
    m_Window=std::make_unique<Window>(props);
    OpenGLContext::PrintInfo(); OpenGLContext::SetDebugOutput(true);
    m_Renderer=std::make_unique<Renderer>(); m_Renderer->Init();
    m_EditorUI=std::make_unique<EditorUI>(*m_Window); m_EditorUI->Init();
    m_Window->SetResizeCallback([this](int w,int h){ m_Renderer->OnResize(w,h); });
    LOG_INFO("Phase7 Init - Tools painting to FBO");
}
void Application::Shutdown(){
    if(m_EditorUI){ m_EditorUI->Shutdown(); m_EditorUI.reset(); }
    if(m_Renderer){ m_Renderer->Shutdown(); m_Renderer.reset(); }
    m_Window.reset();
}
void Application::Run(){
    Viewport viewport(1280,720);
    ImageCanvas canvas; canvas.Init();
    m_EditorUI->SetViewport(&viewport); m_EditorUI->SetCanvas(&canvas);
    EffectChain effectChain(1280,720);
    effectChain.AddEffect(std::make_shared<BlurEffect>());
    effectChain.AddEffect(std::make_shared<VignetteEffect>());

    std::string testPath="assets/test/test.png";
    if(std::filesystem::exists(testPath)){ auto img=ImageManager::Get().Import(testPath); if(img) viewport.SetImage(img); }

    while(m_Running && !m_Window->ShouldClose()){
        Time::Update(); viewport.OnUpdate(Time::DeltaTime());
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
            // Paint canvas overlay
            {
                layerFBO->Bind();
                // Render paint canvas texture if has content
                uint32_t paintTex=viewport.GetPaintCanvas().GetTextureID();
                if(paintTex!=0){
                    // Composite paint canvas onto layerFBO with alpha blending
                    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    // Draw paint texture as fullscreen quad
                }
                layerFBO->Unbind();
            }
            effectChain.Resize(spec.Width, spec.Height);
            auto& params=m_EditorUI->GetEffectParams();
            effectChain.Process(layerFBO->GetColorAttachmentRendererID(0), viewport.GetFramebuffer(), params);
        }
        m_Renderer->BeginFrame({0.10f,0.10f,0.11f,1.0f});
        m_Renderer->EndFrame();
        m_EditorUI->Draw(*m_Renderer);
        m_EditorUI->End();
        m_Window->Update();
    }
    m_EditorUI->SetViewport(nullptr); m_EditorUI->SetCanvas(nullptr);
}
}
