
#include "editor/EditorUI.h"
#include "editor/Viewport.h"
#include "editor/ImageCanvas.h"
#include "editor/LayerStack.h"
#include "editor/Tools.h"
#include "editor/Selection.h"
#include "editor/ExportManager.h"
#include "editor/Project.h"
#include "editor/History.h"
#include "core/Window.h"
#include "core/Log.h"
#include "graphics/Renderer.h"
#include "media/Image.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>

namespace Editor {

EditorUI::EditorUI(Window& window) : m_Window(window) {}
EditorUI::~EditorUI() {}
void EditorUI::SetupDarkTheme(){ ImGuiStyle& s=ImGui::GetStyle(); s.WindowRounding=4.f; s.FrameRounding=4.f; s.WindowPadding=ImVec2(8,8); ImVec4* c=s.Colors; c[ImGuiCol_Text]=ImVec4(0.9f,0.9f,0.9f,1.f); c[ImGuiCol_WindowBg]=ImVec4(0.13f,0.13f,0.14f,1.f); c[ImGuiCol_ChildBg]=ImVec4(0.11f,0.11f,0.12f,1.f); }
void EditorUI::Init(){ IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGuiIO& io=ImGui::GetIO(); io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard|ImGuiConfigFlags_DockingEnable|ImGuiConfigFlags_ViewportsEnable; SetupDarkTheme(); ImGui_ImplGlfw_InitForOpenGL(m_Window.GetNative(), true); ImGui_ImplOpenGL3_Init("#version 460"); LOG_INFO("Phase9 Project System"); m_ConsoleLogs.push_back("[INFO] Phase9: JSON project save/load, history undo/redo, autosave"); m_ImportPathBuffer.resize(512); m_ResizeBuffer[0]=800; m_ResizeBuffer[1]=600; m_ExportSettings.Path="export.png"; strncpy(m_ProjectPathBuffer, "project.json", sizeof(m_ProjectPathBuffer)); }
void EditorUI::Shutdown(){ ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext(); }
void EditorUI::Begin(){ ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame(); }
void EditorUI::End(){ ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); ImGuiIO& io=ImGui::GetIO(); if(io.ConfigFlags&ImGuiConfigFlags_ViewportsEnable){ GLFWwindow* b=glfwGetCurrentContext(); ImGui::UpdatePlatformWindows(); ImGui::RenderPlatformWindowsDefault(); glfwMakeContextCurrent(b);} }
void EditorUI::Draw(Renderer& r){ ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode); DrawMenuBar(); DrawToolbar(); DrawProjectBrowser(); DrawAssetPanel(); DrawViewport(r); DrawProperties(); DrawLayers(); DrawEffectsPanel(); DrawToolsPanel(); DrawAdvancedPanel(); DrawHistogramPanel(); DrawProjectPanel(); DrawHistoryPanel(); DrawTimeline(); DrawStatusBar(r); DrawConsole(); DrawImageImportPopup(); DrawResizePopup(); DrawExportPopup(); DrawNewProjectPopup(); DrawSaveProjectPopup(); if(ShowDemo) ImGui::ShowDemoWindow(&ShowDemo); }

void EditorUI::DrawMenuBar(){
    if(ImGui::BeginMainMenuBar()){
        if(ImGui::BeginMenu("File")){
            if(ImGui::MenuItem("New Project", "Ctrl+N")) m_ShowNewProjectPopup=true;
            if(ImGui::MenuItem("Open Project", "Ctrl+O")){
                // Simple file dialog - use buffer
                m_ShowSaveProjectPopup=false;
                // For demo, try load project.json
                if(m_Viewport){
                    ProjectManager::Get().LoadProject("project.json");
                    ProjectManager::Get().ApplyToEditor(*m_Viewport, m_EffectParams);
                    m_ConsoleLogs.push_back("[Project] Loaded project.json");
                }
            }
            if(ImGui::MenuItem("Save Project", "Ctrl+S")){
                if(m_Viewport){
                    ProjectManager::Get().CaptureFromEditor(*m_Viewport, m_EffectParams);
                    ProjectManager::Get().SaveProject(m_ProjectPathBuffer);
                    m_ConsoleLogs.push_back("[Project] Saved");
                }
            }
            if(ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) m_ShowSaveProjectPopup=true;
            ImGui::Separator();
            if(ImGui::MenuItem("Import Image", "Ctrl+I")) m_ShowImportPopup=true;
            if(ImGui::MenuItem("Export", "Ctrl+E")) m_ShowExportPopup=true;
            ImGui::Separator();
            if(ImGui::MenuItem("Exit", "Alt+F4")) glfwSetWindowShouldClose(m_Window.GetNative(), true);
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Edit")){
            if(ImGui::MenuItem("Undo", "Ctrl+Z", false, HistoryManager::Get().CanUndo())) HistoryManager::Get().Undo();
            if(ImGui::MenuItem("Redo", "Ctrl+Y", false, HistoryManager::Get().CanRedo())) HistoryManager::Get().Redo();
            ImGui::Separator();
            if(ImGui::MenuItem("Clear History")) HistoryManager::Get().Clear();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
void EditorUI::DrawToolbar(){
    ImGui::Begin("Toolbar");
    if(ImGui::Button("New", ImVec2(60,24))) m_ShowNewProjectPopup=true;
    ImGui::SameLine(); if(ImGui::Button("Save", ImVec2(60,24)) && m_Viewport){ ProjectManager::Get().CaptureFromEditor(*m_Viewport, m_EffectParams); ProjectManager::Get().SaveProject(m_ProjectPathBuffer); m_ConsoleLogs.push_back("[Project] Saved"); }
    ImGui::SameLine(); if(ImGui::Button("Undo", ImVec2(60,24))) HistoryManager::Get().Undo();
    ImGui::SameLine(); if(ImGui::Button("Redo", ImVec2(60,24))) HistoryManager::Get().Redo();
    ImGui::SameLine(); ImGui::Text("History: %d undo %d redo | Project: %s %s", HistoryManager::Get().GetUndoCount(), HistoryManager::Get().GetRedoCount(), ProjectManager::Get().GetCurrentPath().c_str(), ProjectManager::Get().IsDirty()?"*":"");
    ImGui::End();
}
void EditorUI::DrawProjectBrowser(){ ImGui::Begin("Project Browser"); if(ImGui::TreeNode("Project")){ ImGui::Text("Name: %s", ProjectManager::Get().GetData().Meta.Name.c_str()); ImGui::Text("Size: %dx%d", ProjectManager::Get().GetData().Meta.Width, ProjectManager::Get().GetData().Meta.Height); ImGui::Text("Layers: %zu", m_Viewport?m_Viewport->GetLayerStack().Size():0); ImGui::TreePop(); } ImGui::End(); }
void EditorUI::DrawAssetPanel(){
    ImGui::Begin("Assets");
    if(ImGui::BeginTabBar("Tabs")){
        if(ImGui::BeginTabItem("Images")){
            auto images=ImageManager::Get().GetAll();
            for(size_t i=0;i<images.size();++i){ auto& img=images[i]; bool sel=(m_SelectedImageIndex==(int)i); std::string label=img->GetMetadata().Name; if(ImGui::Selectable(label.c_str(), sel)){ m_SelectedImageIndex=(int)i; ImageManager::Get().SetActive(img); if(m_Viewport) m_Viewport->SetImage(img); } }
            if(ImGui::Button("Import")) m_ShowImportPopup=true;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
void EditorUI::DrawViewport(Renderer& renderer){
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::Begin("Viewport [Phase9 Project]", nullptr, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
    ImVec2 avail=ImGui::GetContentRegionAvail(); m_ViewportWidth=avail.x; m_ViewportHeight=avail.y;
    if(m_Viewport){
        if(m_ViewportWidth>0 && m_ViewportHeight>0) m_Viewport->OnResize((uint32_t)m_ViewportWidth, (uint32_t)m_ViewportHeight);
        ImGuiIO& io=ImGui::GetIO(); ImVec2 vpPos=ImGui::GetCursorScreenPos();
        if(ImGui::IsWindowHovered()){ if(io.MouseWheel!=0.f) m_Viewport->OnMouseScroll(io.MouseWheel); if(ImGui::IsMouseDragging(ImGuiMouseButton_Middle)){ ImVec2 d=io.MouseDelta; m_Viewport->OnMouseDrag({d.x, -d.y}); } }
        uint32_t texID=m_Viewport->GetColorAttachment(); ImGui::Image((ImTextureID)(intptr_t)texID, avail, ImVec2(0,1), ImVec2(1,0));
    }
    ImGui::End(); ImGui::PopStyleVar();
}
void EditorUI::DrawProperties(){ ImGui::Begin("Properties"); if(m_Viewport){ auto active=m_Viewport->GetLayerStack().GetActive(); if(active){ ImGui::Text("Layer: %s", active->Name.c_str()); float p[2]={active->Transform.Position.x, active->Transform.Position.y}; if(ImGui::DragFloat2("Pos", p)){ glm::vec2 old=active->Transform.Position; active->Transform.Position={p[0],p[1]}; } ImGui::SliderFloat("Opacity", &active->Opacity, 0.f,1.f); } } ImGui::End(); }
void EditorUI::DrawLayers(){ ImGui::Begin("Layers"); if(m_Viewport){ auto& layers=m_Viewport->GetLayerStack().GetLayers(); int activeIdx=m_Viewport->GetLayerStack().GetActiveIndex(); for(int i=(int)layers.size()-1;i>=0;--i){ auto& l=layers[i]; bool isActive=(i==activeIdx); ImGui::PushID(i); bool vis=l->Visible; if(ImGui::Checkbox("##vis",&vis)) l->SetVisible(vis); ImGui::SameLine(); if(ImGui::Selectable(l->Name.c_str(), isActive)) m_Viewport->GetLayerStack().SetActiveIndex(i); ImGui::PopID(); } if(ImGui::Button("Add Layer")){ m_Viewport->GetLayerStack().AddLayer(); HistoryManager::Get().Execute(std::make_unique<AddLayerCommand>("Layer")); } } ImGui::End(); }
void EditorUI::DrawEffectsPanel(){ ImGui::Begin("GPU Effects"); ImGui::SliderFloat("Brightness", &m_EffectParams.Brightness, -1.f,1.f); ImGui::SliderFloat("Contrast", &m_EffectParams.Contrast, 0.f,3.f); ImGui::SliderFloat("Blur", &m_EffectParams.BlurRadius, 0.f,20.f); if(ImGui::Button("Reset")) m_EffectParams=EffectParams(); ImGui::End(); }
void EditorUI::DrawToolsPanel(){ ImGui::Begin("Tools"); if(m_Viewport){ const char* tools[]={"Select","Brush","Eraser"}; for(int i=0;i<3;i++) if(ImGui::Button(tools[i], ImVec2(80,24))) m_Viewport->GetToolManager().SetActiveTool((ToolType)i); } ImGui::End(); }
void EditorUI::DrawAdvancedPanel(){ ImGui::Begin("Advanced [Phase8]"); ImGui::Text("Filters, Selection, Histogram - Phase8"); ImGui::End(); }
void EditorUI::DrawHistogramPanel(){ ImGui::Begin("Histogram"); if(m_Viewport){ auto& hist=m_Viewport->GetHistogram(); if(hist.IsValid()){ ImGui::Text("Mean %.1f Std %.1f", hist.GetMean(), hist.GetStdDev()); } else ImGui::TextDisabled("No histogram"); } ImGui::End(); }
void EditorUI::DrawProjectPanel(){
    ImGui::Begin("Project [Phase9]");
    ImGui::Text("Current: %s", ProjectManager::Get().GetCurrentPath().c_str());
    ImGui::Text("Name: %s", ProjectManager::Get().GetData().Meta.Name.c_str());
    ImGui::Text("Dirty: %s", ProjectManager::Get().IsDirty()?"Yes*":"No");
    ImGui::InputText("Path", m_ProjectPathBuffer, sizeof(m_ProjectPathBuffer));
    if(ImGui::Button("Save Project")){
        if(m_Viewport){ ProjectManager::Get().CaptureFromEditor(*m_Viewport, m_EffectParams); ProjectManager::Get().SaveProjectAs(m_ProjectPathBuffer); m_ConsoleLogs.push_back("[Project] Saved to " + std::string(m_ProjectPathBuffer)); }
    }
    ImGui::SameLine(); if(ImGui::Button("Load Project")){
        if(m_Viewport){ if(ProjectManager::Get().LoadProject(m_ProjectPathBuffer)){ ProjectManager::Get().ApplyToEditor(*m_Viewport, m_EffectParams); m_ConsoleLogs.push_back("[Project] Loaded " + std::string(m_ProjectPathBuffer)); } }
    }
    ImGui::Separator();
    ImGui::Text("Layers in project file: %zu", ProjectManager::Get().GetData().Layers.size());
    for(auto& l : ProjectManager::Get().GetData().Layers) ImGui::BulletText("%s - %.0f%%", l.Name.c_str(), l.Opacity*100.f);
    ImGui::Separator();
    if(ImGui::Button("New Project...")) m_ShowNewProjectPopup=true;
    ImGui::SameLine(); if(ImGui::Button("Save As...")) m_ShowSaveProjectPopup=true;
    ImGui::End();
}
void EditorUI::DrawHistoryPanel(){
    ImGui::Begin("History [Phase9]");
    ImGui::Text("Undo: %d Redo: %d", HistoryManager::Get().GetUndoCount(), HistoryManager::Get().GetRedoCount());
    if(ImGui::Button("Undo") && HistoryManager::Get().CanUndo()) HistoryManager::Get().Undo();
    ImGui::SameLine(); if(ImGui::Button("Redo") && HistoryManager::Get().CanRedo()) HistoryManager::Get().Redo();
    ImGui::SameLine(); if(ImGui::Button("Clear")) HistoryManager::Get().Clear();
    ImGui::Separator();
    auto list=HistoryManager::Get().GetHistoryList();
    for(int i=0;i<(int)list.size();++i){
        bool isCurrent=(i==HistoryManager::Get().GetUndoCount()-1);
        if(isCurrent) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f,0.8f,1.f,1.f));
        ImGui::BulletText("%d: %s %s", i, list[i].c_str(), isCurrent?"<--":"");
        if(isCurrent) ImGui::PopStyleColor();
    }
    ImGui::End();
}
void EditorUI::DrawTimeline(){ ImGui::Begin("Timeline"); ImGui::TextDisabled("Phase10"); ImGui::End(); }
void EditorUI::DrawStatusBar(Renderer& r){ ImGui::Begin("Status Bar", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoResize); auto& stats=r.GetStats(); ImGui::Text("FPS %.1f | Project %s | History %d/%d | Phase9 JSON Save/Load + Undo/Redo", stats.FPS, ProjectManager::Get().GetCurrentPath().c_str(), HistoryManager::Get().GetUndoCount(), HistoryManager::Get().GetRedoCount()); ImGui::End(); }
void EditorUI::DrawConsole(){ ImGui::Begin("Console"); if(ImGui::Button("Clear")) m_ConsoleLogs.clear(); ImGui::BeginChild("Log"); for(auto& l: m_ConsoleLogs) ImGui::TextUnformatted(l.c_str()); ImGui::EndChild(); ImGui::End(); }
void EditorUI::DrawImageImportPopup(){ if(!m_ShowImportPopup) return; ImGui::OpenPopup("Import Image"); if(ImGui::BeginPopupModal("Import Image", &m_ShowImportPopup, ImGuiWindowFlags_AlwaysAutoResize)){ ImGui::InputText("Path", m_ImportPathBuffer.data(), m_ImportPathBuffer.size()); if(ImGui::Button("Import")){ std::string p=std::string(m_ImportPathBuffer.c_str()); p=p.c_str(); if(!p.empty()){ auto img=ImageManager::Get().Import(p); if(img){ if(m_Viewport) m_Viewport->SetImage(img); m_ShowImportPopup=false; ImGui::CloseCurrentPopup(); } } } ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowImportPopup=false; ImGui::CloseCurrentPopup(); } ImGui::EndPopup(); } }
void EditorUI::DrawResizePopup(){ if(!m_ShowResizePopup) return; ImGui::OpenPopup("Resize"); if(ImGui::BeginPopupModal("Resize", &m_ShowResizePopup, ImGuiWindowFlags_AlwaysAutoResize)){ ImGui::InputInt2("Size", m_ResizeBuffer); if(ImGui::Button("Resize")){ auto a=ImageManager::Get().GetActive(); if(a) a->Resize(m_ResizeBuffer[0], m_ResizeBuffer[1]); m_ShowResizePopup=false; ImGui::CloseCurrentPopup(); } ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowResizePopup=false; ImGui::CloseCurrentPopup(); } ImGui::EndPopup(); } }
void EditorUI::DrawExportPopup(){
    if(!m_ShowExportPopup) return; ImGui::OpenPopup("Export Image");
    if(ImGui::BeginPopupModal("Export Image", &m_ShowExportPopup, ImGuiWindowFlags_AlwaysAutoResize)){
        static char pathBuf[256]="export.png"; ImGui::InputText("Path", pathBuf, 256); m_ExportSettings.Path=pathBuf;
        const char* fmts[]={"PNG","JPEG","BMP"}; int cur=(int)m_ExportSettings.Format; if(ImGui::Combo("Format", &cur, fmts, 3)) m_ExportSettings.Format=(ExportFormat)cur;
        if(ImGui::Button("Export")){ if(m_Viewport){ ExportManager::ExportFramebuffer(m_Viewport->GetFramebuffer(), m_ExportSettings); } m_ShowExportPopup=false; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowExportPopup=false; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}
void EditorUI::DrawNewProjectPopup(){
    if(!m_ShowNewProjectPopup) return;
    ImGui::OpenPopup("New Project");
    if(ImGui::BeginPopupModal("New Project", &m_ShowNewProjectPopup, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::InputText("Name", m_ProjectNameBuffer, sizeof(m_ProjectNameBuffer));
        ImGui::InputInt2("Size W,H", m_NewProjectSize);
        if(ImGui::Button("Create")){
            ProjectManager::Get().NewProject(m_ProjectNameBuffer, m_NewProjectSize[0], m_NewProjectSize[1]);
            if(m_Viewport){ m_Viewport->GetLayerStack().Clear(); m_Viewport->GetLayerStack().AddLayer("Background", m_NewProjectSize[0], m_NewProjectSize[1]); }
            m_ConsoleLogs.push_back(std::string("[Project] New: ") + m_ProjectNameBuffer);
            m_ShowNewProjectPopup=false; ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowNewProjectPopup=false; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}
void EditorUI::DrawSaveProjectPopup(){
    if(!m_ShowSaveProjectPopup) return;
    ImGui::OpenPopup("Save Project As");
    if(ImGui::BeginPopupModal("Save Project As", &m_ShowSaveProjectPopup, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::InputText("Path", m_ProjectPathBuffer, sizeof(m_ProjectPathBuffer));
        if(ImGui::Button("Save")){
            if(m_Viewport){ ProjectManager::Get().CaptureFromEditor(*m_Viewport, m_EffectParams); ProjectManager::Get().SaveProjectAs(m_ProjectPathBuffer); m_ConsoleLogs.push_back("[Project] Saved As " + std::string(m_ProjectPathBuffer)); }
            m_ShowSaveProjectPopup=false; ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowSaveProjectPopup=false; ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

}
