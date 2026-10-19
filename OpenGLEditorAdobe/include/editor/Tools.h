
#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include "graphics/Framebuffer.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"

namespace Editor {

enum class ToolType { Select, Brush, Eraser, Text, Rect, Circle, Line, Eyedropper };

struct BrushSettings {
    float Size = 20.f; // px
    float Hardness = 0.8f; // 0..1
    float Opacity = 1.0f;
    float Spacing = 0.1f; // 0..1
    glm::vec4 Color = {1,0,0,1};
    bool Soft = true;
};

struct StrokePoint {
    glm::vec2 Pos;
    float Pressure = 1.f;
};

class PaintCanvas {
public:
    PaintCanvas(uint32_t w=1024, uint32_t h=1024);
    void Resize(uint32_t w, uint32_t h);
    void Clear(const glm::vec4& color = {0,0,0,0});
    void BeginStroke(const glm::vec2& pos);
    void AddPoint(const glm::vec2& pos);
    void EndStroke();

    // Direct paint methods - render to canvas FBO
    void PaintBrush(const glm::vec2& from, const glm::vec2& to, const BrushSettings& settings);
    void PaintEraser(const glm::vec2& from, const glm::vec2& to, const BrushSettings& settings);
    void PaintRect(const glm::vec2& min, const glm::vec2& max, const glm::vec4& color, float thickness=1.f, bool filled=true);
    void PaintCircle(const glm::vec2& center, float radius, const glm::vec4& color, float thickness=1.f, bool filled=true);
    void PaintLine(const glm::vec2& from, const glm::vec2& to, const glm::vec4& color, float thickness=1.f);
    void PaintText(const glm::vec2& pos, const std::string& text, const glm::vec4& color, float size=24.f);

    uint32_t GetTextureID() const { return m_FBO ? m_FBO->GetColorAttachmentRendererID(0) : 0; }
    std::shared_ptr<Framebuffer> GetFBO() { return m_FBO; }
    std::shared_ptr<Texture2D> GetTexture();

    // Composite canvas onto target layer texture
    void CompositeToLayer(std::shared_ptr<class Layer> layer);

private:
    void EnsureShaders();
    std::shared_ptr<Framebuffer> m_FBO;
    std::shared_ptr<Shader> m_BrushShader;
    std::shared_ptr<Shader> m_SolidShader;
    std::shared_ptr<Shader> m_TextShader;
    std::shared_ptr<Texture2D> m_BrushTipTexture;
    std::vector<StrokePoint> m_CurrentStroke;
    bool m_IsPainting = false;
};

class Tool {
public:
    Tool(ToolType type, const std::string& name) : m_Type(type), m_Name(name) {}
    virtual ~Tool() = default;
    virtual void OnMouseDown(const glm::vec2& pos) {}
    virtual void OnMouseDrag(const glm::vec2& from, const glm::vec2& to) {}
    virtual void OnMouseUp(const glm::vec2& pos) {}
    virtual void OnRender() {}

    ToolType GetType() const { return m_Type; }
    const std::string& GetName() const { return m_Name; }
    bool IsActive() const { return m_Active; }
    void SetActive(bool a) { m_Active = a; }

protected:
    ToolType m_Type;
    std::string m_Name;
    bool m_Active = false;
};

class BrushTool : public Tool {
public:
    BrushTool(PaintCanvas& canvas);
    void OnMouseDown(const glm::vec2& pos) override;
    void OnMouseDrag(const glm::vec2& from, const glm::vec2& to) override;
    void OnMouseUp(const glm::vec2& pos) override;
    BrushSettings& GetSettings() { return m_Settings; }
private:
    PaintCanvas& m_Canvas;
    BrushSettings m_Settings;
    glm::vec2 m_LastPos{0};
};

class EraserTool : public Tool {
public:
    EraserTool(PaintCanvas& canvas);
    void OnMouseDown(const glm::vec2& pos) override;
    void OnMouseDrag(const glm::vec2& from, const glm::vec2& to) override;
    void OnMouseUp(const glm::vec2& pos) override;
    BrushSettings& GetSettings() { return m_Settings; }
private:
    PaintCanvas& m_Canvas;
    BrushSettings m_Settings;
    glm::vec2 m_LastPos{0};
};

class ShapeTool : public Tool {
public:
    enum class ShapeType { Rect, Circle, Line };
    ShapeTool(PaintCanvas& canvas, ShapeType shape);
    void OnMouseDown(const glm::vec2& pos) override;
    void OnMouseDrag(const glm::vec2& from, const glm::vec2& to) override;
    void OnMouseUp(const glm::vec2& pos) override;
    void SetShape(ShapeType s) { m_Shape = s; }
    void SetColor(const glm::vec4& c) { m_Color = c; }
    void SetFilled(bool f) { m_Filled = f; }
    void SetThickness(float t) { m_Thickness = t; }
private:
    PaintCanvas& m_Canvas;
    ShapeType m_Shape;
    glm::vec2 m_StartPos{0}, m_EndPos{0};
    glm::vec4 m_Color{1,1,1,1};
    float m_Thickness=2.f;
    bool m_Filled=true;
    bool m_Dragging=false;
};

class TextTool : public Tool {
public:
    TextTool(PaintCanvas& canvas);
    void OnMouseDown(const glm::vec2& pos) override;
    void SetText(const std::string& text) { m_Text = text; }
    void SetColor(const glm::vec4& c) { m_Color = c; }
    void SetSize(float s) { m_Size = s; }
    const std::string& GetText() const { return m_Text; }
private:
    PaintCanvas& m_Canvas;
    std::string m_Text = "Sample Text";
    glm::vec4 m_Color{1,1,1,1};
    float m_Size=32.f;
};

class ToolManager {
public:
    ToolManager(PaintCanvas& canvas);
    void SetActiveTool(ToolType type);
    Tool* GetActiveTool() { return m_ActiveTool; }
    ToolType GetActiveType() const { return m_ActiveType; }

    BrushTool* GetBrushTool() { return m_BrushTool.get(); }
    EraserTool* GetEraserTool() { return m_EraserTool.get(); }
    ShapeTool* GetShapeTool() { return m_ShapeTool.get(); }
    TextTool* GetTextTool() { return m_TextTool.get(); }

    void OnMouseDown(const glm::vec2& pos);
    void OnMouseDrag(const glm::vec2& from, const glm::vec2& to);
    void OnMouseUp(const glm::vec2& pos);

private:
    PaintCanvas& m_Canvas;
    std::unique_ptr<BrushTool> m_BrushTool;
    std::unique_ptr<EraserTool> m_EraserTool;
    std::unique_ptr<ShapeTool> m_ShapeTool;
    std::unique_ptr<TextTool> m_TextTool;
    std::unique_ptr<Tool> m_SelectTool;
    Tool* m_ActiveTool = nullptr;
    ToolType m_ActiveType = ToolType::Select;
};

} // namespace
