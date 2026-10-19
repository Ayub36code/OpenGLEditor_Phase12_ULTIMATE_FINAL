
#pragma once
#include <string>
#include <glm/glm.hpp>

namespace Editor {

enum class ExportFormat { PNG, JPEG, BMP };

struct ExportSettings {
    ExportFormat Format = ExportFormat::PNG;
    std::string Path = "export.png";
    int Quality = 90; // for JPEG 0-100
    bool IncludeLayers = false;
    bool IncludeAlpha = true;
    glm::ivec2 Size{0,0}; // 0 = original
};

class ExportManager {
public:
    static bool ExportTexture(uint32_t textureID, uint32_t width, uint32_t height, const ExportSettings& settings);
    static bool ExportFramebuffer(std::shared_ptr<class Framebuffer> fbo, const ExportSettings& settings);

    static std::string FormatToExtension(ExportFormat fmt);
    static std::string FormatToString(ExportFormat fmt);
};

} // namespace
