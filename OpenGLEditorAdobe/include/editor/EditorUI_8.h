
#pragma once
#include <string>
#include <vector>
#include "graphics/Effects.h"
#include "editor/ExportManager.h"
#include "editor/Project.h"
#include "editor/Timeline.h"

namespace Editor { class Window; class Renderer; class Viewport; class ImageCanvas;
class EditorUI {
public:
    EditorUI(Window& window); ~EditorUI();
    void Init(); void Shutdown(); void Begin(); void End(); void Draw(Renderer& renderer);
    bool ShowDemo=false; bool ShowGrid=true; bool ShowTimeline=true; bool ShowLayers=true;
    void SetViewport(Viewport* vp){ m_Viewport=vp; } void SetCanvas(ImageCanvas* c){ m_Canvas=c; }
    EffectParams& GetEffectParams() { return m_EffectParams; }
    Timeline& GetTimeline() { return m_Timeline; }
private:
    void DrawMenuBar(); void DrawToolbar(); void DrawProjectBrowser(); void DrawAssetPanel();
    void DrawViewport(Renderer& renderer); void DrawProperties(); void DrawLayers();
    void DrawEffectsPanel(); void DrawToolsPanel(); void DrawAdvancedPanel(); void DrawHistogramPanel();
    void DrawProjectPanel(); void DrawHistoryPanel(); void DrawTimelinePanel();
    void DrawStatusBar(Renderer& renderer); void DrawConsole();
    void DrawImageImportPopup(); void DrawResizePopup(); void DrawExportPopup(); void DrawNewProjectPopup(); void DrawSaveProjectPopup(); void SetupDarkTheme();
    Window& m_Window; Viewport* m_Viewport=nullptr; ImageCanvas* m_Canvas=nullptr;
    std::vector<std::string> m_ConsoleLogs; float m_ViewportWidth=0, m_ViewportHeight=0;
    std::string m_ImportPathBuffer; bool m_ShowImportPopup=false; bool m_ShowResizePopup=false; bool m_ShowExportPopup=false; bool m_ShowNewProjectPopup=false; bool m_ShowSaveProjectPopup=false;
    int m_ResizeBuffer[2]={800,600}; int m_SelectedImageIndex=-1;
    EffectParams m_EffectParams;
    ExportSettings m_ExportSettings;
    Timeline m_Timeline{10.f, 30.f};
    float m_ColorPicker[4]={1,0,0,1};
    char m_ProjectPathBuffer[512]="project.json";
    char m_ProjectNameBuffer[128]="MyProject";
    int m_NewProjectSize[2]={1920,1080};
};
}
