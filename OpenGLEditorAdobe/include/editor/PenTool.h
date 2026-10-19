
#pragma once
#include "editor/ProFeatures.h"
#include "graphics/Shader.h"
#include <vector>

namespace Editor {

class PenTool {
public:
    PenTool();
    void OnMouseDown(const glm::vec2& pos, bool alt=false, bool ctrl=false);
    void OnMouseDrag(const glm::vec2& pos);
    void OnMouseUp(const glm::vec2& pos);

    void AddPoint(const glm::vec2& pos);
    void ClosePath();
    void Clear() { m_CurrentPath.Points.clear(); m_CurrentPath.Closed=false; }

    VectorPath& GetPath() { return m_CurrentPath; }
    std::vector<VectorPath>& GetAllPaths() { return m_Paths; }

    void SetFillColor(const glm::vec4& c) { m_CurrentPath.FillColor=c; }
    void SetStrokeColor(const glm::vec4& c) { m_CurrentPath.StrokeColor=c; }

    void DrawPreview();

private:
    VectorPath m_CurrentPath;
    std::vector<VectorPath> m_Paths;
    bool m_IsDraggingHandle=false;
    int m_SelectedPoint=-1;
};

} // namespace
