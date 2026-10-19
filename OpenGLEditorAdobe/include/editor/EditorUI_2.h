
#pragma once
#include <string>
#include <vector>
#include "graphics/Effects.h"
#include "editor/ExportManager.h"

namespace Editor { class Window; class Renderer; class Viewport; class ImageCanvas;
class EditorUI {
public:
    EditorUI(Window& window); ~EditorUI();
    void Init(); void Shutdown(); void Begin(); void End(); void Draw(Renderer& renderer);
    bool ShowDemo=false; bool ShowGrid=true; bool ShowTimeline=true; bool ShowLayers=true;
    void SetViewport(Viewport* vp){ m_Viewport=vp; } void SetCanvas(ImageCanvas* c){ m_Canvas=c; }
    EffectParams& GetEffectParams() { return m_EffectParams; }
private:
    void DrawMenuBar(); void DrawToolbar(); void DrawProjectBrowser(); void DrawAssetPanel();
    void DrawViewport(Renderer& renderer); void DrawProperties(); void DrawLayers();
    void DrawEffectsPanel(); void DrawToolsPanel(); void DrawAdvancedPanel(); void DrawHistogramPanel();
    void DrawTimeline(); void DrawStatusBar(Renderer& renderer); void DrawConsole();
    void DrawImageImportPopup(); void DrawResizePopup(); void DrawExportPopup(); void SetupDarkTheme();
    Window& m_Window; Viewport* m_Viewport=nullptr; ImageCanvas* m_Canvas=nullptr;
    std::vector<std::string> m_ConsoleLogs; float m_ViewportWidth=0, m_ViewportHeight=0;
    std::string m_ImportPathBuffer; bool m_ShowImportPopup=false; bool m_ShowResizePopup=false; bool m_ShowExportPopup=false;
    int m_ResizeBuffer[2]={800,600}; int m_SelectedImageIndex=-1;
    EffectParams m_EffectParams;
    ExportSettings m_ExportSettings;
    float m_ColorPicker[4]={1,0,0,1};
};
}
