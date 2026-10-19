
#include "editor/PenTool.h"
#include "core/Log.h"

namespace Editor {

PenTool::PenTool(){ m_CurrentPath.Name="Path 1"; }

void PenTool::OnMouseDown(const glm::vec2& pos, bool alt, bool ctrl){
    // Check if clicking near existing point for handle editing
    for(size_t i=0;i<m_CurrentPath.Points.size();++i){
        if(glm::distance(m_CurrentPath.Points[i].Position, pos) < 10.f){
            m_SelectedPoint=(int)i; m_IsDraggingHandle=true; return;
        }
    }
    AddPoint(pos);
}

void PenTool::OnMouseDrag(const glm::vec2& pos){
    if(m_IsDraggingHandle && m_SelectedPoint>=0 && m_SelectedPoint < (int)m_CurrentPath.Points.size()){
        auto& pt=m_CurrentPath.Points[m_SelectedPoint];
        glm::vec2 dir=pos - pt.Position;
        pt.HandleOut=dir;
        pt.HandleIn=-dir;
        pt.HasHandleIn=pt.HasHandleOut=true;
    }
}

void PenTool::OnMouseUp(const glm::vec2& pos){ m_IsDraggingHandle=false; }

void PenTool::AddPoint(const glm::vec2& pos){
    BezierPoint pt; pt.Position=pos; pt.Corner=false;
    m_CurrentPath.Points.push_back(pt);
    LOG_INFO("Pen point added at " + std::to_string((int)pos.x) + "," + std::to_string((int)pos.y));
}

void PenTool::ClosePath(){ m_CurrentPath.Closed=true; m_Paths.push_back(m_CurrentPath); m_CurrentPath=VectorPath(); m_CurrentPath.Name="Path " + std::to_string(m_Paths.size()+1); }

void PenTool::DrawPreview(){ /* preview path with ImDrawList */ }

void VectorPath::AddPoint(const glm::vec2& pos){ BezierPoint pt; pt.Position=pos; Points.push_back(pt); }

glm::vec2 VectorPath::Evaluate(float t) const {
    if(Points.empty()) return {0,0};
    if(Points.size()==1) return Points[0].Position;
    // Find segment
    float total=Points.size()- (Closed?0:1);
    float seg=t*total; int idx=(int)seg; float lt=seg-idx;
    if(idx>= (int)Points.size()-1) return Points.back().Position;
    auto& p0=Points[idx]; auto& p1=Points[idx+1];
    // Cubic bezier
    glm::vec2 p0h = p0.HasHandleOut ? p0.Position + p0.HandleOut : p0.Position;
    glm::vec2 p1h = p1.HasHandleIn ? p1.Position + p1.HandleIn : p1.Position;
    float u=1.f-lt;
    return u*u*u*p0.Position + 3*u*u*lt*p0h + 3*u*lt*lt*p1h + lt*lt*lt*p1.Position;
}

std::vector<glm::vec2> VectorPath::Tessellate(int segmentsPerCurve) const {
    std::vector<glm::vec2> result;
    if(Points.size()<2) return result;
    for(size_t i=0;i<Points.size()-1;i++){
        for(int j=0;j<segmentsPerCurve;j++){
            float t=(float)j/segmentsPerCurve;
            float globalT=(i + t)/ (Points.size()-1);
            result.push_back(Evaluate(globalT));
        }
    }
    if(Closed) result.push_back(Points[0].Position);
    return result;
}

}
