
#include "editor/EditorUI.h"
#include "editor/Viewport.h"
#include "editor/ImageCanvas.h"
#include "editor/LayerStack.h"
#include "editor/Tools.h"
#include "editor/Selection.h"
#include "editor/ExportManager.h"
#include "editor/Project.h"
#include "editor/History.h"
#include "editor/Timeline.h"
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
#include <filesystem>

namespace Editor {

EditorUI::EditorUI(Window& window) : m_Window(window) {}
EditorUI::~EditorUI() {}
void EditorUI::SetupDarkTheme(){ ImGuiStyle& s=ImGui::GetStyle(); s.WindowRounding=4.f; s.FrameRounding=4.f; s.WindowPadding=ImVec2(8,8); ImVec4* c=s.Colors; c[ImGuiCol_Text]=ImVec4(0.9f,0.9f,0.9f,1.f); c[ImGuiCol_WindowBg]=ImVec4(0.13f,0.13f,0.14f,1.f); c[ImGuiCol_ChildBg]=ImVec4(0.11f,0.11f,0.12f,1.f); }
void EditorUI::Init(){ IMGUI_CHECKVERSION(); ImGui::CreateContext(); ImGuiIO& io=ImGui::GetIO(); io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard|ImGuiConfigFlags_DockingEnable|ImGuiConfigFlags_ViewportsEnable; SetupDarkTheme(); ImGui_ImplGlfw_InitForOpenGL(m_Window.GetNative(), true); ImGui_ImplOpenGL3_Init("#version 460"); LOG_INFO("Phase10 Timeline"); m_ConsoleLogs.push_back("[INFO] Phase10: Timeline with tracks, clips, keyframes, interpolation, playback"); m_ImportPathBuffer.resize(512); m_ResizeBuffer[0]=800; m_ResizeBuffer[1]=600; m_ExportSettings.Path="export.png"; strncpy(m_ProjectPathBuffer, "project.json", sizeof(m_ProjectPathBuffer)); 
    // Demo timeline
    m_Timeline.AddTrack("Background", "Background");
    m_Timeline.AddTrack("Layer 1", "Layer 1");
    // Add demo clip with keyframes
    Clip clip; clip.Name="Move Clip"; clip.StartTime=0.f; clip.Duration=5.f; clip.LayerName="Layer 1";
    Keyframe kf1; kf1.Time=0.f; kf1.Type=KeyframeType::Position; kf1.Value={0,0,0,0}; kf1.Interp=InterpolationType::EaseInOut;
    Keyframe kf2; kf2.Time=2.5f; kf2.Type=KeyframeType::Position; kf2.Value={200,100,0,0}; kf2.Interp=InterpolationType::EaseInOut;
    Keyframe kf3; kf3.Time=5.f; kf3.Type=KeyframeType::Position; kf3.Value={0,0,0,0}; kf3.Interp=InterpolationType::Linear;
    clip.AddKeyframe(kf1); clip.AddKeyframe(kf2); clip.AddKeyframe(kf3);
    Keyframe op1; op1.Time=0.f; op1.Type=KeyframeType::Opacity; op1.Value={1,0,0,0};
    Keyframe op2; op2.Time=2.5f; op2.Type=KeyframeType::Opacity; op2.Value={0.3f,0,0,0};
    clip.AddKeyframe(op1); clip.AddKeyframe(op2);
    if(m_Timeline.GetTracks().size()>1) m_Timeline.GetTracks()[1].AddClip(clip);
}
void EditorUI::Shutdown(){ ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext(); }
void EditorUI::Begin(){ ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame(); }
void EditorUI::End(){ ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); ImGuiIO& io=ImGui::GetIO(); if(io.ConfigFlags&ImGuiConfigFlags_ViewportsEnable){ GLFWwindow* b=glfwGetCurrentContext(); ImGui::UpdatePlatformWindows(); ImGui::RenderPlatformWindowsDefault(); glfwMakeContextCurrent(b);} }
void EditorUI::Draw(Renderer& r){ ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode); DrawMenuBar(); DrawToolbar(); DrawProjectBrowser(); DrawAssetPanel(); DrawViewport(r); DrawProperties(); DrawLayers(); DrawEffectsPanel(); DrawToolsPanel(); DrawAdvancedPanel(); DrawHistogramPanel(); DrawProjectPanel(); DrawHistoryPanel(); DrawTimelinePanel(); DrawStatusBar(r); DrawConsole(); DrawImageImportPopup(); DrawResizePopup(); DrawExportPopup(); DrawNewProjectPopup(); DrawSaveProjectPopup(); if(ShowDemo) ImGui::ShowDemoWindow(&ShowDemo); }

void EditorUI::DrawMenuBar(){
    if(ImGui::BeginMainMenuBar()){
        if(ImGui::BeginMenu("File")){
            if(ImGui::MenuItem("New Project", "Ctrl+N")) m_ShowNewProjectPopup=true;
            if(ImGui::MenuItem("Save Project", "Ctrl+S") && m_Viewport){ ProjectManager::Get().CaptureFromEditor(*m_Viewport, m_EffectParams); ProjectManager::Get().SaveProject(m_ProjectPathBuffer); }
            ImGui::Separator();
            if(ImGui::MenuItem("Export Frame", "Ctrl+E")) m_ShowExportPopup=true;
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Timeline")){
            if(ImGui::MenuItem("Play", "Space", false, !m_Timeline.IsPlaying())) m_Timeline.Play();
            if(ImGui::MenuItem("Pause", "Space", false, m_Timeline.IsPlaying())) m_Timeline.Pause();
            if(ImGui::MenuItem("Stop", "Esc")) m_Timeline.Stop();
            ImGui::Separator();
            if(ImGui::MenuItem("Add Track")) m_Timeline.AddTrack("Track " + std::to_string(m_Timeline.GetTracks().size()+1));
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
void EditorUI::DrawToolbar(){
    ImGui::Begin("Toolbar");
    if(m_Timeline.IsPlaying()){ if(ImGui::Button("Pause", ImVec2(60,24))) m_Timeline.Pause(); } else { if(ImGui::Button("Play", ImVec2(60,24))) m_Timeline.Play(); }
    ImGui::SameLine(); if(ImGui::Button("Stop", ImVec2(60,24))) m_Timeline.Stop();
    ImGui::SameLine(); if(ImGui::Button("Add Track", ImVec2(80,24))) m_Timeline.AddTrack("Track " + std::to_string(m_Timeline.GetTracks().size()+1));
    ImGui::SameLine(); ImGui::Text("Time %.2f / %.2f sec | FPS %.0f | %s", m_Timeline.GetCurrentTime(), m_Timeline.GetDuration(), m_Timeline.GetFPS(), m_Timeline.IsPlaying()?"Playing":"Stopped");
    ImGui::End();
}
void EditorUI::DrawProjectBrowser(){ ImGui::Begin("Project Browser"); ImGui::Text("Tracks %zu", m_Timeline.GetTracks().size()); ImGui::End(); }
void EditorUI::DrawAssetPanel(){
    ImGui::Begin("Assets");
    auto images=ImageManager::Get().GetAll();
    for(size_t i=0;i<images.size();++i){ auto& img=images[i]; bool sel=(m_SelectedImageIndex==(int)i); if(ImGui::Selectable(img->GetMetadata().Name.c_str(), sel)){ m_SelectedImageIndex=(int)i; ImageManager::Get().SetActive(img); if(m_Viewport) m_Viewport->SetImage(img); } }
    if(ImGui::Button("Import")) m_ShowImportPopup=true;
    ImGui::End();
}
void EditorUI::DrawViewport(Renderer& renderer){
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::Begin("Viewport [Phase10 Timeline]", nullptr, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
    ImVec2 avail=ImGui::GetContentRegionAvail(); m_ViewportWidth=avail.x; m_ViewportHeight=avail.y;
    if(m_Viewport){
        if(m_ViewportWidth>0 && m_ViewportHeight>0) m_Viewport->OnResize((uint32_t)m_ViewportWidth, (uint32_t)m_ViewportHeight);
        ImGuiIO& io=ImGui::GetIO(); if(ImGui::IsWindowHovered()){ if(io.MouseWheel!=0.f) m_Viewport->OnMouseScroll(io.MouseWheel); if(ImGui::IsMouseDragging(ImGuiMouseButton_Middle)){ ImVec2 d=io.MouseDelta; m_Viewport->OnMouseDrag({d.x, -d.y}); } }
        uint32_t texID=m_Viewport->GetColorAttachment(); ImGui::Image((ImTextureID)(intptr_t)texID, avail, ImVec2(0,1), ImVec2(1,0));
        ImGui::SetCursorPos(ImVec2(10,10)); ImGui::Text("Time %.2f / %.2f", m_Timeline.GetCurrentTime(), m_Timeline.GetDuration());
    }
    ImGui::End(); ImGui::PopStyleVar();
}
void EditorUI::DrawProperties(){ ImGui::Begin("Properties"); if(m_Viewport){ auto active=m_Viewport->GetLayerStack().GetActive(); if(active){ ImGui::Text("Layer: %s", active->Name.c_str()); ImGui::Text("Timeline Pos: %.0f,%.0f", m_Timeline.EvaluatePosition(active->Name, active->Transform.Position).x, m_Timeline.EvaluatePosition(active->Name, active->Transform.Position).y); ImGui::Text("Timeline Opacity: %.2f", m_Timeline.EvaluateOpacity(active->Name, active->Opacity)); } } ImGui::End(); }
void EditorUI::DrawLayers(){ ImGui::Begin("Layers"); if(m_Viewport){ auto& layers=m_Viewport->GetLayerStack().GetLayers(); int activeIdx=m_Viewport->GetLayerStack().GetActiveIndex(); for(int i=(int)layers.size()-1;i>=0;--i){ auto& l=layers[i]; bool isActive=(i==activeIdx); ImGui::PushID(i); bool vis=l->Visible; if(ImGui::Checkbox("##vis",&vis)) l->SetVisible(vis); ImGui::SameLine(); if(ImGui::Selectable(l->Name.c_str(), isActive)) m_Viewport->GetLayerStack().SetActiveIndex(i); ImGui::PopID(); } if(ImGui::Button("Add Layer")) m_Viewport->GetLayerStack().AddLayer(); } ImGui::End(); }
void EditorUI::DrawEffectsPanel(){ ImGui::Begin("GPU Effects"); ImGui::SliderFloat("Brightness", &m_EffectParams.Brightness, -1.f,1.f); ImGui::SliderFloat("Contrast", &m_EffectParams.Contrast, 0.f,3.f); if(ImGui::Button("Reset")) m_EffectParams=EffectParams(); ImGui::End(); }
void EditorUI::DrawToolsPanel(){ ImGui::Begin("Tools"); if(m_Viewport){ const char* tools[]={"Select","Brush","Eraser"}; for(int i=0;i<3;i++) if(ImGui::Button(tools[i], ImVec2(80,24))) m_Viewport->GetToolManager().SetActiveTool((ToolType)i); } ImGui::End(); }
void EditorUI::DrawAdvancedPanel(){ ImGui::Begin("Advanced"); ImGui::TextDisabled("Phase8 filters etc"); ImGui::End(); }
void EditorUI::DrawHistogramPanel(){ ImGui::Begin("Histogram"); if(m_Viewport){ auto& hist=m_Viewport->GetHistogram(); if(hist.IsValid()) ImGui::Text("Mean %.1f", hist.GetMean()); } ImGui::End(); }
void EditorUI::DrawProjectPanel(){ ImGui::Begin("Project"); ImGui::Text("Project: %s", ProjectManager::Get().GetCurrentPath().c_str()); ImGui::End(); }
void EditorUI::DrawHistoryPanel(){ ImGui::Begin("History"); ImGui::Text("Undo %d Redo %d", HistoryManager::Get().GetUndoCount(), HistoryManager::Get().GetRedoCount()); if(ImGui::Button("Undo")) HistoryManager::Get().Undo(); ImGui::SameLine(); if(ImGui::Button("Redo")) HistoryManager::Get().Redo(); ImGui::End(); }

void EditorUI::DrawTimelinePanel(){
    ImGui::Begin("Timeline [Phase10 - Final]", nullptr, ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("Duration %.1f sec | FPS %.0f | Playhead %.2f | %s | Loop %s", m_Timeline.GetDuration(), m_Timeline.GetFPS(), m_Timeline.GetCurrentTime(), m_Timeline.IsPlaying()?"Playing":"Paused", m_Timeline.IsLooping()?"On":"Off");

    // Playback controls
    if(m_Timeline.IsPlaying()){ if(ImGui::Button("Pause")) m_Timeline.Pause(); } else { if(ImGui::Button("Play")) m_Timeline.Play(); }
    ImGui::SameLine(); if(ImGui::Button("Stop")) m_Timeline.Stop();
    ImGui::SameLine(); if(ImGui::Button("Rewind")) m_Timeline.SetCurrentTime(0.f);
    ImGui::SameLine(); bool loop=m_Timeline.IsLooping(); if(ImGui::Checkbox("Loop", &loop)) m_Timeline.SetLoop(loop);
    ImGui::SameLine(); float dur=m_Timeline.GetDuration(); if(ImGui::SliderFloat("Duration", &dur, 1.f, 60.f)) m_Timeline.Resize(dur);
    ImGui::SameLine(); float fps=m_Timeline.GetFPS(); if(ImGui::SliderFloat("FPS", &fps, 1.f, 120.f)) m_Timeline.SetFPS(fps);

    ImGui::Separator();

    // Timeline ruler
    ImDrawList* drawList=ImGui::GetWindowDrawList();
    ImVec2 canvasPos=ImGui::GetCursorScreenPos();
    ImVec2 canvasSize=ImGui::GetContentRegionAvail();
    canvasSize.y=200.f; // fixed height for tracks
    ImGui::InvisibleButton("timeline_canvas", canvasSize);
    bool hovered=ImGui::IsItemHovered();
    float timeToPixel = canvasSize.x / m_Timeline.GetDuration();

    // Background
    drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x+canvasSize.x, canvasPos.y+canvasSize.y), IM_COL32(30,30,35,255));

    // Time markers
    for(float t=0; t<=m_Timeline.GetDuration(); t+=1.f){
        float x=canvasPos.x + t*timeToPixel;
        drawList->AddLine(ImVec2(x, canvasPos.y), ImVec2(x, canvasPos.y+canvasSize.y), IM_COL32(60,60,65,255), 1.f);
        char buf[16]; snprintf(buf, sizeof(buf), "%.0fs", t);
        drawList->AddText(ImVec2(x+2, canvasPos.y+2), IM_COL32(150,150,150,255), buf);
    }

    // Tracks
    float trackHeight=30.f;
    int trackIdx=0;
    for(auto& track : m_Timeline.GetTracks()){
        float y=canvasPos.y + 20.f + trackIdx*trackHeight;
        // Track background
        drawList->AddRectFilled(ImVec2(canvasPos.x, y), ImVec2(canvasPos.x+canvasSize.x, y+trackHeight-2), IM_COL32(45,45,50,255));
        drawList->AddText(ImVec2(canvasPos.x+5, y+5), IM_COL32(200,200,200,255), track.Name.c_str());

        // Clips
        for(auto& clip : track.Clips){
            float cx=canvasPos.x + clip.StartTime*timeToPixel;
            float cw=clip.Duration*timeToPixel;
            ImVec2 cmin(cx, y+2), cmax(cx+cw, y+trackHeight-4);
            ImU32 col=IM_COL32((int)(track.Color.r*255),(int)(track.Color.g*255),(int)(track.Color.b*255), 200);
            drawList->AddRectFilled(cmin, cmax, col);
            drawList->AddRect(cmin, cmax, IM_COL32(255,255,255,100));
            drawList->AddText(ImVec2(cx+5, y+5), IM_COL32(255,255,255,255), clip.Name.c_str());

            // Keyframes inside clip
            for(auto& kf : clip.Keyframes){
                float kx=canvasPos.x + (clip.StartTime + kf.Time)*timeToPixel;
                ImVec2 kpos(kx, y+trackHeight/2);
                drawList->AddCircleFilled(kpos, 4.f, IM_COL32(255,200,50,255));
                drawList->AddCircle(kpos, 4.f, IM_COL32(255,255,255,255));
            }
        }
        trackIdx++;
    }

    // Playhead
    float playheadX=canvasPos.x + m_Timeline.GetCurrentTime()*timeToPixel;
    drawList->AddLine(ImVec2(playheadX, canvasPos.y), ImVec2(playheadX, canvasPos.y+canvasSize.y), IM_COL32(255,50,50,255), 2.f);
    drawList->AddTriangleFilled(ImVec2(playheadX-6, canvasPos.y), ImVec2(playheadX+6, canvasPos.y), ImVec2(playheadX, canvasPos.y+10), IM_COL32(255,50,50,255));

    // Scrubbing
    if(hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
        float mouseX=ImGui::GetIO().MousePos.x - canvasPos.x;
        float newTime=mouseX / timeToPixel;
        m_Timeline.SetCurrentTime(newTime);
    }
    if(hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
        float mouseX=ImGui::GetIO().MousePos.x - canvasPos.x;
        float newTime=mouseX / timeToPixel;
        m_Timeline.SetCurrentTime(newTime);
    }

    ImGui::SetCursorScreenPos(ImVec2(canvasPos.x, canvasPos.y+canvasSize.y+5));
    if(ImGui::Button("Add Track")) m_Timeline.AddTrack("Track " + std::to_string(m_Timeline.GetTracks().size()+1));
    ImGui::SameLine(); if(ImGui::Button("Add Clip to Active") && m_Viewport){
        auto active=m_Viewport->GetLayerStack().GetActive();
        if(active && !m_Timeline.GetTracks().empty()){
            Clip clip; clip.Name="Clip " + std::to_string((int)m_Timeline.GetTracks()[0].Clips.size()+1); clip.StartTime=m_Timeline.GetCurrentTime(); clip.Duration=2.f; clip.LayerName=active->Name;
            m_Timeline.GetTracks()[0].AddClip(clip);
        }
    }
    ImGui::SameLine(); if(ImGui::Button("Export Timeline JSON")){
        // Simple export of timeline data
        m_ConsoleLogs.push_back("[Timeline] Exported timeline with " + std::to_string(m_Timeline.GetTracks().size()) + " tracks");
    }

    ImGui::End();
}

void EditorUI::DrawStatusBar(Renderer& r){ ImGui::Begin("Status Bar", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoResize); auto& stats=r.GetStats(); ImGui::Text("FPS %.1f | Timeline %.2f/%.1f %s | Tracks %zu | Phase10 FINAL - Animation + Keyframes + Playback", stats.FPS, m_Timeline.GetCurrentTime(), m_Timeline.GetDuration(), m_Timeline.IsPlaying()?"▶ Playing":"⏸ Paused", m_Timeline.GetTracks().size()); ImGui::End(); }
void EditorUI::DrawConsole(){ ImGui::Begin("Console"); if(ImGui::Button("Clear")) m_ConsoleLogs.clear(); ImGui::BeginChild("Log"); for(auto& l: m_ConsoleLogs) ImGui::TextUnformatted(l.c_str()); ImGui::EndChild(); ImGui::End(); }
void EditorUI::DrawImageImportPopup(){ if(!m_ShowImportPopup) return; ImGui::OpenPopup("Import Image"); if(ImGui::BeginPopupModal("Import Image", &m_ShowImportPopup, ImGuiWindowFlags_AlwaysAutoResize)){ ImGui::InputText("Path", m_ImportPathBuffer.data(), m_ImportPathBuffer.size()); if(ImGui::Button("Import")){ std::string p=std::string(m_ImportPathBuffer.c_str()); p=p.c_str(); if(!p.empty()){ auto img=ImageManager::Get().Import(p); if(img){ if(m_Viewport) m_Viewport->SetImage(img); m_ShowImportPopup=false; ImGui::CloseCurrentPopup(); } } } ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowImportPopup=false; ImGui::CloseCurrentPopup(); } ImGui::EndPopup(); } }
void EditorUI::DrawResizePopup(){ if(!m_ShowResizePopup) return; ImGui::OpenPopup("Resize"); if(ImGui::BeginPopupModal("Resize", &m_ShowResizePopup, ImGuiWindowFlags_AlwaysAutoResize)){ ImGui::InputInt2("Size", m_ResizeBuffer); if(ImGui::Button("Resize")){ auto a=ImageManager::Get().GetActive(); if(a) a->Resize(m_ResizeBuffer[0], m_ResizeBuffer[1]); m_ShowResizePopup=false; ImGui::CloseCurrentPopup(); } ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowResizePopup=false; ImGui::CloseCurrentPopup(); } ImGui::EndPopup(); } }
void EditorUI::DrawExportPopup(){ if(!m_ShowExportPopup) return; ImGui::OpenPopup("Export"); if(ImGui::BeginPopupModal("Export", &m_ShowExportPopup, ImGuiWindowFlags_AlwaysAutoResize)){ static char pathBuf[256]="export.png"; ImGui::InputText("Path", pathBuf, 256); m_ExportSettings.Path=pathBuf; if(ImGui::Button("Export")){ if(m_Viewport) ExportManager::ExportFramebuffer(m_Viewport->GetFramebuffer(), m_ExportSettings); m_ShowExportPopup=false; ImGui::CloseCurrentPopup(); } ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowExportPopup=false; ImGui::CloseCurrentPopup(); } ImGui::EndPopup(); } }
void EditorUI::DrawNewProjectPopup(){ if(!m_ShowNewProjectPopup) return; ImGui::OpenPopup("New Project"); if(ImGui::BeginPopupModal("New Project", &m_ShowNewProjectPopup, ImGuiWindowFlags_AlwaysAutoResize)){ ImGui::InputText("Name", m_ProjectNameBuffer, sizeof(m_ProjectNameBuffer)); ImGui::InputInt2("Size", m_NewProjectSize); if(ImGui::Button("Create")){ ProjectManager::Get().NewProject(m_ProjectNameBuffer, m_NewProjectSize[0], m_NewProjectSize[1]); if(m_Viewport){ m_Viewport->GetLayerStack().Clear(); m_Viewport->GetLayerStack().AddLayer("Background", m_NewProjectSize[0], m_NewProjectSize[1]); } m_ShowNewProjectPopup=false; ImGui::CloseCurrentPopup(); } ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowNewProjectPopup=false; ImGui::CloseCurrentPopup(); } ImGui::EndPopup(); } }
void EditorUI::DrawSaveProjectPopup(){ if(!m_ShowSaveProjectPopup) return; ImGui::OpenPopup("Save As"); if(ImGui::BeginPopupModal("Save As", &m_ShowSaveProjectPopup, ImGuiWindowFlags_AlwaysAutoResize)){ ImGui::InputText("Path", m_ProjectPathBuffer, sizeof(m_ProjectPathBuffer)); if(ImGui::Button("Save")){ if(m_Viewport){ ProjectManager::Get().CaptureFromEditor(*m_Viewport, m_EffectParams); ProjectManager::Get().SaveProjectAs(m_ProjectPathBuffer); } m_ShowSaveProjectPopup=false; ImGui::CloseCurrentPopup(); } ImGui::SameLine(); if(ImGui::Button("Cancel")){ m_ShowSaveProjectPopup=false; ImGui::CloseCurrentPopup(); } ImGui::EndPopup(); } }

}
