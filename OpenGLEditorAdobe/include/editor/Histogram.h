
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <memory>

namespace Editor {

struct HistogramData {
    std::vector<int> Red{256,0};
    std::vector<int> Green{256,0};
    std::vector<int> Blue{256,0};
    std::vector<int> Luminance{256,0};
    int MaxCount = 1;
};

class Histogram {
public:
    Histogram() = default;
    void CalculateFromPixels(const std::vector<unsigned char>& pixels, int channels);
    void CalculateFromTexture(uint32_t textureID, uint32_t width, uint32_t height);

    const HistogramData& GetData() const { return m_Data; }
    bool IsValid() const { return m_Valid; }

    // Stats
    float GetMean() const;
    float GetStdDev() const;
    glm::vec3 GetAverageColor() const { return m_AvgColor; }

private:
    HistogramData m_Data;
    bool m_Valid = false;
    glm::vec3 m_AvgColor{0};
    float m_Mean = 0, m_StdDev = 0;
};

} // namespace
