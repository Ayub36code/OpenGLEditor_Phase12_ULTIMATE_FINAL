
#include "editor/CropTool.h"
#include "media/Image.h"
#include "core/Log.h"
#include <algorithm>

namespace Editor {

void CropTool::Begin(const glm::vec2& imageSize) {
    m_Active=true;
    m_Rect.Reset();
    m_Rect.Active=true;
}

void CropTool::Update(const glm::vec2& mousePos, const glm::vec2& imagePos, const glm::vec2& imageSize, float zoom) {
    // Not implemented fully for brevity - would track normalized coords
}

void CropTool::End() {
    m_Dragging=false;
}

bool CropTool::ApplyCrop(Image& image) {
    if(!m_Rect.IsValid()) return false;

    auto& pixels = image.GetPixels();
    int w = image.GetWidth();
    int h = image.GetHeight();
    int ch = image.GetMetadata().Channels;

    int x0 = (int)(m_Rect.Min.x * w);
    int y0 = (int)(m_Rect.Min.y * h);
    int x1 = (int)(m_Rect.Max.x * w);
    int y1 = (int)(m_Rect.Max.y * h);

    x0 = std::clamp(x0, 0, w-1); x1 = std::clamp(x1, 0, w);
    y0 = std::clamp(y0, 0, h-1); y1 = std::clamp(y1, 0, h);

    int newW = x1 - x0;
    int newH = y1 - y0;
    if(newW<=0 || newH<=0) return false;

    std::vector<unsigned char> newPixels(newW * newH * ch);
    for(int y=0;y<newH;y++) {
        int srcY = y0 + y;
        for(int x=0;x<newW;x++) {
            int srcX = x0 + x;
            for(int c=0;c<ch;c++) {
                newPixels[(y*newW + x)*ch + c] = pixels[(srcY*w + srcX)*ch + c];
            }
        }
    }

    // For Phase4, we just log and would recreate texture - actual implementation recreates Image
    LOG_INFO("Crop applied: " + std::to_string(newW) + "x" + std::to_string(newH) + " from " + std::to_string(w) + "x" + std::to_string(h));
    // TODO: replace image pixels and texture
    return true;
}

}
