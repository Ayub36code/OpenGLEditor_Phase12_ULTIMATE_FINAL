
#pragma once
#include <glm/glm.hpp>
#include "graphics/Camera.h"
#include "graphics/Framebuffer.h"
#include "media/Image.h"
#include "editor/TransformGizmo.h"
#include "editor/LayerStack.h"
#include "editor/Tools.h"
#include "editor/Selection.h"
#include "editor/Mask.h"
#include "editor/Histogram.h"
#include <memory>

namespace Editor {

class Viewport {
public:
    Viewport(uint32_t width=1280, uint32_t height=720);
    ~Viewport() = default;

    void OnResize(uint32_t w, uint32_t h);
    void OnUpdate(float deltaTime);

    void OnMouseScroll(float yOffset);
    void OnMouseDrag(const glm::vec2& delta);
    void OnMouseButton(int button, bool pressed, const glm::vec2& pos);

    std::shared_ptr<Framebuffer> GetFramebuffer() const { return m_Framebuffer; }
    uint32_t GetColorAttachment() const { return m_Framebuffer->GetColorAttachmentRendererID(0); }
    OrthographicCamera& GetCamera() { return *m_Camera; }

    float GetZoom() const { return m_Zoom; }
    void SetZoom(float zoom);
    glm::vec2 GetPan() const { return m_Pan; }
    void ResetView();

    void SetImage(std::shared_ptr<Image> img);
    std::shared_ptr<Image> GetImage() const { return m_Image; }

    TransformGizmo& GetGizmo() { return m_Gizmo; }
    LayerStack& GetLayerStack() { return m_LayerStack; }
    PaintCanvas& GetPaintCanvas() { return *m_PaintCanvas; }
    ToolManager& GetToolManager() { return *m_ToolManager; }
    SelectionManager& GetSelectionManager() { return *m_SelectionManager; }
    Histogram& GetHistogram() { return m_Histogram; }

    glm::vec2 ScreenToWorld(const glm::vec2& screenPos, const glm::vec2& viewportSize);
    glm::vec2 GetLayerScreenPos(std::shared_ptr<Layer> layer, const glm::vec2& viewportPos, const glm::vec2& viewportSize) const;
    glm::vec2 GetLayerScreenSize(std::shared_ptr<Layer> layer) const;

private:
    void UpdateCamera();
    std::shared_ptr<Framebuffer> m_Framebuffer;
    std::unique_ptr<OrthographicCamera> m_Camera;
    std::shared_ptr<Image> m_Image;
    TransformGizmo m_Gizmo;
    LayerStack m_LayerStack;
    std::unique_ptr<PaintCanvas> m_PaintCanvas;
    std::unique_ptr<ToolManager> m_ToolManager;
    std::unique_ptr<SelectionManager> m_SelectionManager;
    Histogram m_Histogram;
    float m_Zoom=1.f;
    glm::vec2 m_Pan{0.f};
    glm::vec2 m_LastMousePos{0.f};
    bool m_Panning=false;
    std::shared_ptr<Texture2D> m_CheckerTexture;
};

} // namespace
