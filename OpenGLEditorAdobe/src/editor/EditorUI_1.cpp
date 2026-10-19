
#include "editor/EditorUI.h"
#include "editor/Viewport.h"
#include "editor/ImageCanvas.h"
#include "editor/LayerStack.h"
#include "editor/Tools.h"
#include "core/Window.h"
#include "core/Log.h"
#include "core/Time.h"
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
void EditorUI::SetupDarkTheme(){
    ImGuiStyle& s=ImGui::GetStyle(); s.WindowRounding=4.f; s.FrameRounding=4.f; s.WindowPadding=ImVec2(8,8);
    ImVec4* c=s.Colors; c[ImGuiCol_Text]=ImVec4(0.9f,0.9f,0.9f,1.f); c[ImGuiCol_WindowBg]=ImVec4(0.13f,0.13f,0.14f,1.f);
    c[ImGuiCol_ChildBg]=ImVec4(0.11f,0.11f,0.12f,1.f); c[ImGuiCol_TitleBg]=ImVec4(0.10f,0.10f,0.11f,1.f);
    c[ImGuiCol_TitleBgActive]=ImVec4(0.16f,0.16f,0.17f,1.f); c[ImGuiCol_Header]=ImVec4(0.24f,0.24f,0.26f,1.f);
    c[ImGuiCol_Button]=ImVec4(0.24f,0.24f,0.26f,1.f); c[ImGuiCol_ButtonHovered]=ImVec4(0.32f,0.32f,0.34f,1.f);
}
void EditorUI::Init(){
    IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGuiIO& io=ImGui::GetIO();
    io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard|ImGuiConfigFlags_DockingEnable|ImGuiConfigFlags_ViewportsEnable;
    SetupDarkTheme(); ImGui_ImplGlfw_InitForOpenGL(m_Window.GetNative(), true); ImGui_ImplOpenGL3_Init("#version 460");
    LOG_INFO("ImGui Phase7 - Tools"); m_ConsoleLogs.push_back("[INFO] Phase7: Brush/Eraser/Text/Shape tools - painting to layer FBO");
    m_ImportPathBuffer.resize(512); m_ResizeBuffer[0]=800; m_ResizeBuffer[1]=600;
}
void EditorUI::Shutdown(){ ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext(); }
void EditorUI::Begin(){ ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame(); }
void EditorUI::End(){ ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); ImGuiIO& io=ImGui::GetIO(); if(io.ConfigFlags&ImGuiConfigFlags_ViewportsEnable){ GLFWwindow* b=glfwGetCurrentContext(); ImGui::UpdatePlatformWindows(); ImGui::RenderPlatformWindowsDefault(); glfwMakeContextCurrent(b);} }
void EditorUI::Draw(Renderer& r){ ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode); DrawMenuBar(); DrawToolbar(); DrawProjectBrowser(); DrawAssetPanel(); DrawViewport(r); DrawProperties(); DrawLayers(); DrawEffectsPanel(); DrawToolsPanel(); DrawTimeline(); DrawStatusBar(r); DrawConsole(); DrawImageImportPopup(); DrawResizePopup(); if(ShowDemo) ImGui::ShowDemoWindow(&ShowDemo); }

void EditorUI::DrawMenuBar(){
    if(ImGui::BeginMainMenuBar()){
        if(ImGui::BeginMenu("File")){ if(ImGui::MenuItem("Import Image", "Ctrl+I")) m_ShowImportPopup=true; if(ImGui::MenuItem("Exit", "Alt+F4")) glfwSetWindowShouldClose(m_Window.GetNative(), true); ImGui::EndMenu(); }
        if(ImGui::BeginMenu("Tools")){
            if(ImGui::MenuItem("Select", "V")) if(m_Viewport) m_Viewport->GetToolManager().SetActiveTool(ToolType::Select);
            if(ImGui::MenuItem("Brush", "B")) if(m_Viewport) m_Viewport->GetToolManager().SetActiveTool(ToolType::Brush);
            if(ImGui::MenuItem("Eraser", "E")) if(m_Viewport) m_Viewport->GetToolManager().SetActiveTool(ToolType::Eraser);
            if(ImGui::MenuItem("Text", "T")) if(m_Viewport) m_Viewport->GetToolManager().SetActiveTool(ToolType::Text);
            if(ImGui::MenuItem("Rect", "R")) if(m_Viewport) m_Viewport->GetToolManager().SetActiveTool(ToolType::Rect);
            if(ImGui::MenuItem("Circle", "O")) if(m_Viewport) m_Viewport->GetToolManager().SetActiveTool(ToolType::Circle);
            if(ImGui::MenuItem("Line", "L")) if(m_Viewport) m_Viewport->GetToolManager().SetActiveTool(ToolType::Line);
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Effects")){ if(ImGui::MenuItem("Reset FX")) m_EffectParams=EffectParams(); ImGui::EndMenu(); }
        ImGui::EndMainMenuBar();
    }
}
void EditorUI::DrawToolbar(){
    ImGui::Begin("Toolbar");
    const char* tools[]={"V Select","B Brush","E Eraser","T Text","R Rect","O Circle","L Line"};
    ToolType types[]={ToolType::Select, ToolType::Brush, ToolType::Eraser, ToolType::Text, ToolType::Rect, ToolType::Circle, ToolType::Line};
    for(int i=0;i<7;i++){
        bool active = m_Viewport && m_Viewport->GetToolManager().GetActiveType()==types[i];
        if(active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f,0.6f,1.f,1.f));
        if(ImGui::Button(tools[i], ImVec2(85,28)) && m_Viewport) m_Viewport->GetToolManager().SetActiveTool(types[i]);
        if(active) ImGui::PopStyleColor();
        ImGui::SameLine();
    }
    ImGui::NewLine(); ImGui::Separator();
    if(m_Viewport){
        auto* brush = m_Viewport->GetToolManager().GetBrushTool();
        if(brush){
            ImGui::Text("Brush:"); ImGui::SameLine(); ImGui::SliderFloat("Size", &brush->GetSettings().Size, 1.f, 200.f);
            ImGui::SameLine(); ImGui::ColorEdit4("Color", &brush->GetSettings().Color.r, ImGuiColorEditFlags_NoInputs);
        }
    }
    ImGui::End();
}
void EditorUI::DrawProjectBrowser(){ ImGui::Begin("Project Browser"); ImGui::BulletText("Layers: %zu", m_Viewport?m_Viewport->GetLayerStack().Size():0); ImGui::End(); }
void EditorUI::DrawAssetPanel(){
    ImGui::Begin("Assets / Media");
    if(ImGui::BeginTabBar("AssetTabs")){
        if(ImGui::BeginTabItem("Images")){
            auto images=ImageManager::Get().GetAll();
            for(size_t i=0;i<images.size();++i){ auto& img=images[i]; bool sel=(m_SelectedImageIndex==(int)i); std::string label=img->GetMetadata().Name + " " + std::to_string(img->GetWidth())+"x"+std::to_string(img->GetHeight()); if(ImGui::Selectable(label.c_str(), sel)){ m_SelectedImageIndex=(int)i; ImageManager::Get().SetActive(img); if(m_Viewport) m_Viewport->SetImage(img); } }
            if(ImGui::Button("Import Image")) m_ShowImportPopup=true;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
void EditorUI::DrawViewport(Renderer& renderer){
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::Begin("Viewport - Canvas [Phase7 Tools]", nullptr, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
    ImVec2 avail=ImGui::GetContentRegionAvail(); m_ViewportWidth=avail.x; m_ViewportHeight=avail.y;
    if(m_Viewport){
        if(m_ViewportWidth>0 && m_ViewportHeight>0) m_Viewport->OnResize((uint32_t)m_ViewportWidth, (uint32_t)m_ViewportHeight);
        ImGuiIO& io=ImGui::GetIO();
        ImVec2 vpPos=ImGui::GetCursorScreenPos();
        bool hovered=ImGui::IsWindowHovered();
        if(hovered){
            if(io.MouseWheel!=0.f && !io.KeyCtrl) m_Viewport->OnMouseScroll(io.MouseWheel);
            if(ImGui::IsMouseDragging(ImGuiMouseButton_Middle)){ ImVec2 d=io.MouseDelta; m_Viewport->OnMouseDrag({d.x, -d.y}); }
            // Tool handling - left mouse paints when not panning
            if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().KeyAlt){
                glm::vec2 mousePos={io.MousePos.x - vpPos.x, io.MousePos.y - vpPos.y};
                m_Viewport->GetToolManager().OnMouseDown(mousePos);
            }
            if(ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
                glm::vec2 from={io.MousePos.x - io.MouseDelta.x - vpPos.x, io.MousePos.y - io.MouseDelta.y - vpPos.y};
                glm::vec2 to={io.MousePos.x - vpPos.x, io.MousePos.y - vpPos.y};
                m_Viewport->GetToolManager().OnMouseDrag(from, to);
            }
            if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
                glm::vec2 pos={io.MousePos.x - vpPos.x, io.MousePos.y - vpPos.y};
                m_Viewport->GetToolManager().OnMouseUp(pos);
            }
        }
        uint32_t texID=m_Viewport->GetColorAttachment(); ImGui::Image((ImTextureID)(intptr_t)texID, avail, ImVec2(0,1), ImVec2(1,0));
        auto activeLayer=m_Viewport->GetLayerStack().GetActive();
        if(activeLayer && !activeLayer->Locked && m_Viewport->GetToolManager().GetActiveType()==ToolType::Select){
            glm::vec2 screenPos=m_Viewport->GetLayerScreenPos(activeLayer, {vpPos.x, vpPos.y}, {avail.x, avail.y});
            glm::vec2 screenSize=m_Viewport->GetLayerScreenSize(activeLayer);
            m_Viewport->GetGizmo().DrawAndHandle({vpPos.x, vpPos.y}, {avail.x, avail.y}, screenPos, screenSize, activeLayer->Transform.Rotation, activeLayer->Transform, m_Viewport->GetZoom());
        }
        ImGui::SetCursorPos(ImVec2(10,10)); ImGui::Text("Tool: %s | PaintCanvas FBO %d | Layers %zu", m_Viewport->GetToolManager().GetActiveTool()?m_Viewport->GetToolManager().GetActiveTool()->GetName().c_str():"None", m_Viewport->GetPaintCanvas().GetTextureID(), m_Viewport->GetLayerStack().Size());
    }
    ImGui::End(); ImGui::PopStyleVar();
}
void EditorUI::DrawProperties(){
    ImGui::Begin("Properties / Inspector");
    if(m_Viewport){
        auto active=m_Viewport->GetLayerStack().GetActive();
        if(active){
            ImGui::Text("Layer: %s", active->Name.c_str());
            float p[2]={active->Transform.Position.x, active->Transform.Position.y}; if(ImGui::DragFloat2("Position", p)) active->Transform.Position={p[0],p[1]};
            float s[2]={active->Transform.Scale.x, active->Transform.Scale.y}; if(ImGui::DragFloat2("Scale", s,0.01f,0.01f,10.f)) active->Transform.Scale={s[0],s[1]};
            if(ImGui::SliderFloat("Rotation", &active->Transform.Rotation, -180.f, 180.f)) {}
            ImGui::Separator(); if(ImGui::SliderFloat("Opacity", &active->Opacity, 0.f,1.f)) active->SetOpacity(active->Opacity);
        }
    }
    ImGui::End();
}
void EditorUI::DrawLayers(){
    ImGui::Begin("Layers [Phase7]");
    if(m_Viewport){
        auto& layers=m_Viewport->GetLayerStack().GetLayers(); int activeIdx=m_Viewport->GetLayerStack().GetActiveIndex();
        for(int i=(int)layers.size()-1;i>=0;--i){ auto& layer=layers[i]; bool isActive=(i==activeIdx); ImGui::PushID(i); bool vis=layer->Visible; if(ImGui::Checkbox("##vis", &vis)) layer->SetVisible(vis); ImGui::SameLine(); std::string label=layer->Name + " " + std::to_string((int)(layer->Opacity*100))+"%"; if(ImGui::Selectable(label.c_str(), isActive)) m_Viewport->GetLayerStack().SetActiveIndex(i); ImGui::PopID(); }
        if(ImGui::Button("Add Layer")) m_Viewport->GetLayerStack().AddLayer();
        ImGui::SameLine(); if(ImGui::Button("Delete") && activeIdx>=0) m_Viewport->GetLayerStack().DeleteLayer(activeIdx);
    }
    ImGui::End();
}
void EditorUI::DrawEffectsPanel(){
    ImGui::Begin("GPU Effects [Phase6]");
    ImGui::SliderFloat("Brightness", &m_EffectParams.Brightness, -1.f,1.f);
    ImGui::SliderFloat("Contrast", &m_EffectParams.Contrast, 0.f,3.f);
    ImGui::SliderFloat("Saturation", &m_EffectParams.Saturation, 0.f,3.f);
    ImGui::SliderFloat("Blur", &m_EffectParams.BlurRadius, 0.f,20.f);
    ImGui::Checkbox("Grayscale", &m_EffectParams.Grayscale); ImGui::SameLine(); ImGui::Checkbox("Invert", &m_EffectParams.Invert);
    if(ImGui::Button("Reset FX")) m_EffectParams=EffectParams();
    ImGui::End();
}
void EditorUI::DrawToolsPanel(){
    ImGui::Begin("Tools [Phase7]");
    if(m_Viewport){
        auto type=m_Viewport->GetToolManager().GetActiveType();
        ImGui::Text("Active: %d", (int)type);
        if(type==ToolType::Brush || type==ToolType::Eraser){
            auto* brush = type==ToolType::Brush ? m_Viewport->GetToolManager().GetBrushTool() : (BrushTool*)m_Viewport->GetToolManager().GetEraserTool();
            if(brush){
                ImGui::SliderFloat("Size", &brush->GetSettings().Size, 1.f, 200.f);
                ImGui::SliderFloat("Hardness", &brush->GetSettings().Hardness, 0.f,1.f);
                ImGui::SliderFloat("Opacity", &brush->GetSettings().Opacity, 0.f,1.f);
                ImGui::ColorEdit4("Color", &brush->GetSettings().Color.r);
            }
        }
        if(type==ToolType::Text){
            auto* textTool=m_Viewport->GetToolManager().GetTextTool();
            static char buf[256]="Sample Text"; ImGui::InputText("Text", buf, 256); textTool->SetText(buf);
            static float col[4]={1,1,1,1}; if(ImGui::ColorEdit4("Color", col)) textTool->SetColor({col[0],col[1],col[2],col[3]});
        }
        if(type==ToolType::Rect || type==ToolType::Circle || type==ToolType::Line){
            auto* shapeTool=m_Viewport->GetToolManager().GetShapeTool();
            static float col[4]={1,0,0,1}; static float thick=2.f; static bool filled=true;
            ImGui::ColorEdit4("Color", col); ImGui::SliderFloat("Thickness", &thick, 1.f,20.f); ImGui::Checkbox("Filled", &filled);
            shapeTool->SetColor({col[0],col[1],col[2],col[3]}); shapeTool->SetThickness(thick); shapeTool->SetFilled(filled);
        }
        ImGui::Separator();
        ImGui::TextDisabled("Painting via FBO - GPU texture updated");
        ImGui::TextDisabled("Brush tip: radial gradient texture, soft falloff");
        ImGui::TextDisabled("Shortcuts: B=Brush, E=Eraser, T=Text, R=Rect, O=Circle, L=Line");
    }
    ImGui::End();
}
void EditorUI::DrawTimeline(){ ImGui::Begin("Timeline"); ImGui::TextDisabled("Phase10"); ImGui::End(); }
void EditorUI::DrawStatusBar(Renderer& r){ ImGui::Begin("Status Bar", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoResize); auto& stats=r.GetStats(); ImGui::Text("FPS %.1f | DC %d | Tool %s | Phase7 Painting to FBO", stats.FPS, stats.DrawCalls, m_Viewport?m_Viewport->GetToolManager().GetActiveTool()->GetName().c_str():"None"); ImGui::End(); }
void EditorUI::DrawConsole(){ ImGui::Begin("Console / Log"); if(ImGui::Button("Clear")) m_ConsoleLogs.clear(); ImGui::BeginChild("LogScroll"); for(auto& l: m_ConsoleLogs) ImGui::TextUnformatted(l.c_str()); if(ImGui::GetScrollY()>=ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.f); ImGui::EndChild(); ImGui::End(); }
void EditorUI::DrawImageImportPopup(){ if(!m_ShowImportPopup) return; ImGui::OpenPopup("Import Image"); if(ImGui::BeginPopupModal("Import Image", &m_ShowImportPopup, ImGuiWindowFlags_AlwaysAutoResize)){ ImGui::Text("Enter image path (PNG/JPEG/BMP):"); ImGui::InputText("##path", m_ImportPathBuffer.data(), m_ImportPathBuffer.size()); if(ImGui::Button("Import")){ std::string p=std::string(m_ImportPathBuffer.c_str()); p=p.c_str(); if(!p.empty()){ auto img=ImageManager::Get().Import(p); if(img){ m_ConsoleLogs.push_back("[Import] "+p); if(m_Viewport) m_Viewport->SetImage(img); m_ShowImportPopup=false; ImGui::CloseCurrentPopup(); } else m_ConsoleLogs.push_back("[Error] Failed: "+p); } } ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowImportPopup=false; ImGui::CloseCurrentPopup(); } ImGui::EndPopup(); } }
void EditorUI::DrawResizePopup(){ if(!m_ShowResizePopup) return; ImGui::OpenPopup("Resize Image"); if(ImGui::BeginPopupModal("Resize Image", &m_ShowResizePopup, ImGuiWindowFlags_AlwaysAutoResize)){ ImGui::InputInt2("New Size", m_ResizeBuffer); if(ImGui::Button("Resize")){ auto active=ImageManager::Get().GetActive(); if(active && m_ResizeBuffer[0]>0 && m_ResizeBuffer[1]>0) if(active->Resize(m_ResizeBuffer[0], m_ResizeBuffer[1])) m_ConsoleLogs.push_back("[Resize] to "+std::to_string(m_ResizeBuffer[0])+"x"+std::to_string(m_ResizeBuffer[1])); m_ShowResizePopup=false; ImGui::CloseCurrentPopup(); } ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowResizePopup=false; ImGui::CloseCurrentPopup(); } ImGui::EndPopup(); } }

}
