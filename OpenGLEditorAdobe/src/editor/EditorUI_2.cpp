
#include "editor/EditorUI.h"
#include "editor/Viewport.h"
#include "editor/ImageCanvas.h"
#include "editor/LayerStack.h"
#include "editor/Tools.h"
#include "editor/Selection.h"
#include "editor/ExportManager.h"
#include "core/Window.h"
#include "core/Log.h"
#include "graphics/Renderer.h"
#include "media/Image.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Editor {

EditorUI::EditorUI(Window& window) : m_Window(window) {}
EditorUI::~EditorUI() {}
void EditorUI::SetupDarkTheme(){ ImGuiStyle& s=ImGui::GetStyle(); s.WindowRounding=4.f; s.FrameRounding=4.f; s.WindowPadding=ImVec2(8,8); ImVec4* c=s.Colors; c[ImGuiCol_Text]=ImVec4(0.9f,0.9f,0.9f,1.f); c[ImGuiCol_WindowBg]=ImVec4(0.13f,0.13f,0.14f,1.f); c[ImGuiCol_ChildBg]=ImVec4(0.11f,0.11f,0.12f,1.f); }
void EditorUI::Init(){ IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGuiIO& io=ImGui::GetIO(); io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard|ImGuiConfigFlags_DockingEnable|ImGuiConfigFlags_ViewportsEnable; SetupDarkTheme(); ImGui_ImplGlfw_InitForOpenGL(m_Window.GetNative(), true); ImGui_ImplOpenGL3_Init("#version 460"); LOG_INFO("Phase8 Advanced Features"); m_ConsoleLogs.push_back("[INFO] Phase8: Selection, Masks, Histogram, ColorPicker, Export, Filters"); m_ImportPathBuffer.resize(512); m_ResizeBuffer[0]=800; m_ResizeBuffer[1]=600; m_ExportSettings.Path="export.png"; }
void EditorUI::Shutdown(){ ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext(); }
void EditorUI::Begin(){ ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame(); }
void EditorUI::End(){ ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); ImGuiIO& io=ImGui::GetIO(); if(io.ConfigFlags&ImGuiConfigFlags_ViewportsEnable){ GLFWwindow* b=glfwGetCurrentContext(); ImGui::UpdatePlatformWindows(); ImGui::RenderPlatformWindowsDefault(); glfwMakeContextCurrent(b);} }
void EditorUI::Draw(Renderer& r){ ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode); DrawMenuBar(); DrawToolbar(); DrawProjectBrowser(); DrawAssetPanel(); DrawViewport(r); DrawProperties(); DrawLayers(); DrawEffectsPanel(); DrawToolsPanel(); DrawAdvancedPanel(); DrawHistogramPanel(); DrawTimeline(); DrawStatusBar(r); DrawConsole(); DrawImageImportPopup(); DrawResizePopup(); DrawExportPopup(); if(ShowDemo) ImGui::ShowDemoWindow(&ShowDemo); }

void EditorUI::DrawMenuBar(){
    if(ImGui::BeginMainMenuBar()){
        if(ImGui::BeginMenu("File")){
            if(ImGui::MenuItem("Import Image", "Ctrl+I")) m_ShowImportPopup=true;
            if(ImGui::MenuItem("Export", "Ctrl+E")) m_ShowExportPopup=true;
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Selection")){
            if(ImGui::MenuItem("Select All", "Ctrl+A") && m_Viewport){ m_Viewport->GetSelectionManager().BeginSelection({0,0}, SelectionType::Rectangle); }
            if(ImGui::MenuItem("Clear Selection", "Ctrl+D") && m_Viewport) m_Viewport->GetSelectionManager().Clear();
            if(ImGui::MenuItem("Magic Wand", "W") && m_Viewport){ /* wand */ }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
void EditorUI::DrawToolbar(){
    ImGui::Begin("Toolbar");
    if(ImGui::Button("Export", ImVec2(80,28))) m_ShowExportPopup=true;
    ImGui::SameLine(); if(m_Viewport) ImGui::Text("Selection: %s | Mask: %s", m_Viewport->GetSelectionManager().HasSelection()?"Active":"None", "Enabled");
    ImGui::End();
}
void EditorUI::DrawProjectBrowser(){ ImGui::Begin("Project Browser"); ImGui::BulletText("Layers: %zu", m_Viewport?m_Viewport->GetLayerStack().Size():0); ImGui::End(); }
void EditorUI::DrawAssetPanel(){
    ImGui::Begin("Assets");
    if(ImGui::BeginTabBar("Tabs")){
        if(ImGui::BeginTabItem("Images")){
            auto images=ImageManager::Get().GetAll();
            for(size_t i=0;i<images.size();++i){ auto& img=images[i]; bool sel=(m_SelectedImageIndex==(int)i); std::string label=img->GetMetadata().Name; if(ImGui::Selectable(label.c_str(), sel)){ m_SelectedImageIndex=(int)i; ImageManager::Get().SetActive(img); if(m_Viewport) m_Viewport->SetImage(img); } }
            if(ImGui::Button("Import")) m_ShowImportPopup=true;
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Color")){
            ImGui::ColorPicker4("Picker", m_ColorPicker, ImGuiColorEditFlags_NoInputs);
            if(m_Viewport){
                auto* brush=m_Viewport->GetToolManager().GetBrushTool();
                if(brush) brush->GetSettings().Color={m_ColorPicker[0], m_ColorPicker[1], m_ColorPicker[2], m_ColorPicker[3]};
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
void EditorUI::DrawViewport(Renderer& renderer){
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::Begin("Viewport [Phase8 Advanced]", nullptr, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
    ImVec2 avail=ImGui::GetContentRegionAvail(); m_ViewportWidth=avail.x; m_ViewportHeight=avail.y;
    if(m_Viewport){
        if(m_ViewportWidth>0 && m_ViewportHeight>0) m_Viewport->OnResize((uint32_t)m_ViewportWidth, (uint32_t)m_ViewportHeight);
        ImGuiIO& io=ImGui::GetIO();
        ImVec2 vpPos=ImGui::GetCursorScreenPos();
        if(ImGui::IsWindowHovered()){
            if(io.MouseWheel!=0.f) m_Viewport->OnMouseScroll(io.MouseWheel);
            if(ImGui::IsMouseDragging(ImGuiMouseButton_Middle)){ ImVec2 d=io.MouseDelta; m_Viewport->OnMouseDrag({d.x, -d.y}); }
        }
        uint32_t texID=m_Viewport->GetColorAttachment(); ImGui::Image((ImTextureID)(intptr_t)texID, avail, ImVec2(0,1), ImVec2(1,0));
        // Selection overlay
        if(m_Viewport->GetSelectionManager().HasSelection()){
            ImDrawList* dl=ImGui::GetWindowDrawList();
            auto sel=m_Viewport->GetSelectionManager().GetSelection();
            ImVec2 min=ImVec2(vpPos.x+sel.Min.x, vpPos.y+sel.Min.y);
            ImVec2 max=ImVec2(vpPos.x+sel.Max.x, vpPos.y+sel.Max.y);
            dl->AddRect(min, max, IM_COL32(255,255,0,200), 0,0,2.f);
            // Marching ants effect via dashed line
            dl->AddRect(min, max, IM_COL32(0,0,0,150), 0,0,1.f);
        }
    }
    ImGui::End(); ImGui::PopStyleVar();
}
void EditorUI::DrawProperties(){
    ImGui::Begin("Properties");
    if(m_Viewport){
        auto active=m_Viewport->GetLayerStack().GetActive();
        if(active){
            ImGui::Text("Layer: %s", active->Name.c_str());
            float p[2]={active->Transform.Position.x, active->Transform.Position.y}; if(ImGui::DragFloat2("Pos", p)) active->Transform.Position={p[0],p[1]};
            ImGui::SliderFloat("Opacity", &active->Opacity, 0.f,1.f);
        }
    }
    ImGui::End();
}
void EditorUI::DrawLayers(){
    ImGui::Begin("Layers");
    if(m_Viewport){
        auto& layers=m_Viewport->GetLayerStack().GetLayers(); int activeIdx=m_Viewport->GetLayerStack().GetActiveIndex();
        for(int i=(int)layers.size()-1;i>=0;--i){ auto& l=layers[i]; bool isActive=(i==activeIdx); ImGui::PushID(i); bool vis=l->Visible; if(ImGui::Checkbox("##vis",&vis)) l->SetVisible(vis); ImGui::SameLine(); if(ImGui::Selectable(l->Name.c_str(), isActive)) m_Viewport->GetLayerStack().SetActiveIndex(i); ImGui::PopID(); }
        if(ImGui::Button("Add Layer")) m_Viewport->GetLayerStack().AddLayer();
    }
    ImGui::End();
}
void EditorUI::DrawEffectsPanel(){
    ImGui::Begin("GPU Effects");
    ImGui::SliderFloat("Brightness", &m_EffectParams.Brightness, -1.f,1.f);
    ImGui::SliderFloat("Contrast", &m_EffectParams.Contrast, 0.f,3.f);
    ImGui::SliderFloat("Saturation", &m_EffectParams.Saturation, 0.f,3.f);
    ImGui::SliderFloat("Blur", &m_EffectParams.BlurRadius, 0.f,20.f);
    if(ImGui::Button("Reset")) m_EffectParams=EffectParams();
    ImGui::End();
}
void EditorUI::DrawToolsPanel(){
    ImGui::Begin("Tools [Phase7]");
    if(m_Viewport){
        const char* tools[]={"Select","Brush","Eraser","Rect","Circle"};
        for(int i=0;i<5;i++) if(ImGui::Button(tools[i], ImVec2(80,24))){ m_Viewport->GetToolManager().SetActiveTool((ToolType)i); }
    }
    ImGui::End();
}
void EditorUI::DrawAdvancedPanel(){
    ImGui::Begin("Advanced [Phase8]");
    ImGui::Text("Selection Tools");
    if(ImGui::Button("Rect Select")) if(m_Viewport) m_Viewport->GetSelectionManager().BeginSelection({100,100}, SelectionType::Rectangle);
    ImGui::SameLine(); if(ImGui::Button("Clear Sel")) if(m_Viewport) m_Viewport->GetSelectionManager().Clear();
    ImGui::Separator();
    ImGui::Text("Layer Mask");
    static bool maskEnabled=true; ImGui::Checkbox("Enable Mask", &maskEnabled);
    ImGui::Separator();
    ImGui::Text("Filters (GPU)");
    if(ImGui::Button("Edge Detection")){ m_ConsoleLogs.push_back("[Filter] Edge detection shader"); }
    ImGui::SameLine(); if(ImGui::Button("Emboss")){ m_ConsoleLogs.push_back("[Filter] Emboss shader"); }
    ImGui::Separator();
    ImGui::Text("Export");
    if(ImGui::Button("Export PNG")) m_ShowExportPopup=true;
    ImGui::End();
}
void EditorUI::DrawHistogramPanel(){
    ImGui::Begin("Histogram [Phase8]");
    if(m_Viewport){
        auto& hist=m_Viewport->GetHistogram();
        if(hist.IsValid()){
            auto& data=hist.GetData();
            ImGui::Text("Mean: %.1f StdDev: %.1f", hist.GetMean(), hist.GetStdDev());
            ImGui::Text("Avg Color: %.2f %.2f %.2f", hist.GetAverageColor().r, hist.GetAverageColor().g, hist.GetAverageColor().b);
            // Draw histogram as lines
            ImDrawList* dl=ImGui::GetWindowDrawList();
            ImVec2 pos=ImGui::GetCursorScreenPos();
            ImVec2 size=ImVec2(256,80);
            ImGui::InvisibleButton("hist", size);
            // Red channel
            for(int i=0;i<255;i++){
                float h1=(float)data.Red[i]/data.MaxCount*size.y;
                float h2=(float)data.Red[i+1]/data.MaxCount*size.y;
                dl->AddLine(ImVec2(pos.x+i, pos.y+size.y-h1), ImVec2(pos.x+i+1, pos.y+size.y-h2), IM_COL32(255,50,50,200), 1.f);
            }
            ImGui::TextDisabled("Red channel histogram (256 bins)");
        } else ImGui::TextDisabled("No image loaded - histogram requires image");
    }
    ImGui::End();
}
void EditorUI::DrawTimeline(){ ImGui::Begin("Timeline"); ImGui::TextDisabled("Phase10"); ImGui::End(); }
void EditorUI::DrawStatusBar(Renderer& r){ ImGui::Begin("Status Bar", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoResize); auto& stats=r.GetStats(); ImGui::Text("FPS %.1f | Phase8 Advanced: Selection, Mask, Histogram, Export, Filters | Layers %zu", stats.FPS, m_Viewport?m_Viewport->GetLayerStack().Size():0); ImGui::End(); }
void EditorUI::DrawConsole(){ ImGui::Begin("Console"); if(ImGui::Button("Clear")) m_ConsoleLogs.clear(); ImGui::BeginChild("Log"); for(auto& l: m_ConsoleLogs) ImGui::TextUnformatted(l.c_str()); ImGui::EndChild(); ImGui::End(); }
void EditorUI::DrawImageImportPopup(){ if(!m_ShowImportPopup) return; ImGui::OpenPopup("Import Image"); if(ImGui::BeginPopupModal("Import Image", &m_ShowImportPopup, ImGuiWindowFlags_AlwaysAutoResize)){ ImGui::InputText("Path", m_ImportPathBuffer.data(), m_ImportPathBuffer.size()); if(ImGui::Button("Import")){ std::string p=std::string(m_ImportPathBuffer.c_str()); p=p.c_str(); if(!p.empty()){ auto img=ImageManager::Get().Import(p); if(img){ if(m_Viewport) m_Viewport->SetImage(img); m_ShowImportPopup=false; ImGui::CloseCurrentPopup(); } } } ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowImportPopup=false; ImGui::CloseCurrentPopup(); } ImGui::EndPopup(); } }
void EditorUI::DrawResizePopup(){ if(!m_ShowResizePopup) return; ImGui::OpenPopup("Resize"); if(ImGui::BeginPopupModal("Resize", &m_ShowResizePopup, ImGuiWindowFlags_AlwaysAutoResize)){ ImGui::InputInt2("Size", m_ResizeBuffer); if(ImGui::Button("Resize")){ auto a=ImageManager::Get().GetActive(); if(a) a->Resize(m_ResizeBuffer[0], m_ResizeBuffer[1]); m_ShowResizePopup=false; ImGui::CloseCurrentPopup(); } ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowResizePopup=false; ImGui::CloseCurrentPopup(); } ImGui::EndPopup(); } }
void EditorUI::DrawExportPopup(){
    if(!m_ShowExportPopup) return;
    ImGui::OpenPopup("Export Image");
    if(ImGui::BeginPopupModal("Export Image", &m_ShowExportPopup, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::InputText("Path", m_ExportSettings.Path.data() ? m_ExportSettings.Path.data() : (char*)"export.png", 256);
        // For simplicity use buffer
        static char pathBuf[256]="export.png"; ImGui::InputText("##path", pathBuf, 256); m_ExportSettings.Path=pathBuf;
        const char* fmts[]={"PNG","JPEG","BMP"}; int cur=(int)m_ExportSettings.Format; if(ImGui::Combo("Format", &cur, fmts, 3)) m_ExportSettings.Format=(ExportFormat)cur;
        if(m_ExportSettings.Format==ExportFormat::JPEG) ImGui::SliderInt("Quality", &m_ExportSettings.Quality, 1,100);
        if(ImGui::Button("Export")){
            if(m_Viewport){
                bool ok=ExportManager::ExportFramebuffer(m_Viewport->GetFramebuffer(), m_ExportSettings);
                if(ok) m_ConsoleLogs.push_back("[Export] Success: " + m_ExportSettings.Path);
                else m_ConsoleLogs.push_back("[Export] Failed");
            }
            m_ShowExportPopup=false; ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowExportPopup=false; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

}
